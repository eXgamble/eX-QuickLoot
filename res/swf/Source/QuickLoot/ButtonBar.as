class QuickLoot.ButtonBar extends gfx.controls.ButtonBar
{
	/* PRIVATE VARIABLES */

	// Fork: how many renderers the data uses; the ones after it are spares, kept hidden.
	private var _shownCount: Number = 0;


	/* INITIALIZATION */

	// @override ButtonBar
	public function ButtonBar()
	{
		super();
	}


	/* PRIVATE FUNCTIONS */

	// Fork: members are written with "this." on purpose. The swf is rebuilt by importing this file with
	// JPEXS FFDec, whose compiler treats an unqualified inherited member (renderers, _dataProvider) as a
	// plain variable, which is undefined at run time (the bar vanished).

	// @override gfx.core.UIComponent
	private function draw(): Void
	{
		// Fork (SKY-200): match the buttons to the data on every draw. Upstream skipped this while a new
		// button was still getting ready, so a second change in those frames (the window opening with
		// More held) kept the old buttons, unlabelled. Spare buttons are hidden instead of removed, so a
		// bar that shrinks and grows again (More held, then let go) reuses ready buttons instead of
		// waiting frames for new ones (the bar showed only "Take" meanwhile).
		var l: Number = this._dataProvider.length;
		while (this.renderers.length < l) {
			this.renderers.push(this.createRenderer(this.renderers.length));
		}
		this._shownCount = l;

		this.populateData();

		if (this.drawLayout() && this._selectedIndex != -1) {
			this.selectItem(this._selectedIndex);
		}
	}


	/**
	 * @override gfx.controls.ButtonBar
	 * implements a horizontal, centered layout
	 */
	private function drawLayout(): Boolean
	{
		// Spares stay hidden.
		for (var s: Number = this._shownCount; s < this.renderers.length; ++s) {
			this.renderers[s]._visible = false;
		}

		// If a shown renderer is not yet ready, invalidate to force a redraw.
		for (var c: Number = 0; c < this._shownCount; ++c) {
			if (!this.renderers[c].initialized) {
				this.reflowing = true;
				this.invalidate();
				return false;
			}
		}
		this.reflowing = false;

		var totalW: Number = 0;
		for (var i: Number = 0; i < this._shownCount; ++i) {
			var renderer: MovieClip = this.renderers[i];
			// Manually size the renderer
			if (this._autoSize == "none" && this._buttonWidth > 0) {
				renderer.width = this._buttonWidth;
			}

			totalW += renderer.width;
		}

		totalW += this._spacing * (this._shownCount - 1);

		var calcY: Function = function(a_height: Number): Number {
			return Math.max((this.height - a_height) / 2, 0);
		};

		var midpoint: Number = this.width / 2;
		var w: Number = midpoint - totalW / 2;
		for (var i: Number = 0; i < this._shownCount; ++i) {
			var renderer: MovieClip = this.renderers[i];
			renderer._x = w;
			renderer._y = calcY(renderer.height);
			renderer._visible = true;
			w += renderer.width + this._spacing;
		}

		return true;
	}


	// @override gfx.controls.ButtonBar
	private function createRenderer(a_index: Number): MovieClip
	{
		var renderer: MovieClip = super.createRenderer(a_index);
		renderer._visible = false;	// avoid visible snapping into place
		return renderer;
	}
}
