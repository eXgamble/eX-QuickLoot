#pragma once

namespace QuickLoot
{
	class SanityChecks
	{
	public:
		SanityChecks() = delete;
		~SanityChecks() = delete;
		SanityChecks(SanityChecks&&) = delete;
		SanityChecks(const SanityChecks&) = delete;
		SanityChecks& operator=(SanityChecks&&) = delete;
		SanityChecks& operator=(const SanityChecks&) = delete;

		static bool PerformChecks();

		// Fork: checked apart, so a swf that can't load (no SkyUI) leaves headless mode working.
		static bool ValidateEsp();
		static bool ValidateSwf();

	private:
		static constexpr auto LEGACY_SWF = "Data\\Interface\\LootMenu.swf";
		static constexpr auto CURRENT_SWF = "Data\\Interface\\LootMenuIE.swf";
	};
}
