#pragma once

/*
	Header File for QuickLoot IE integration.
	Before using any other functions, call QuickLootAPI::Init and pass in your own plugin name.

	Unlike the rest of the project, this file continues to be available under the MIT license.
	This means you can use the QuickLoot API in projects that are licensed under terms not compatible with GPL3.
*/

namespace QuickLoot::API
{
	struct ItemStack
	{
		// This is a pointer to the inventory entry
		RE::InventoryEntryData* entry;
		// This is set if the inventory entry is for an item the NPC dropped on the floor.
		RE::ObjectRefHandle dropRef;
	};

	enum class QuickLootAction : uint8_t
	{
		kNone,

		kDisable,
		kEnable,

		kUse,
		kTake,
		kTakeAll,
		kTransfer,

		kScrollUp,
		kScrollDown,
		kPrevPage,
		kNextPage,

		// Fork: the target's perk options (Feed, Harvest Blood, ...) while More is held.
		kPerkOption1,
		kPerkOption2,
		kPerkOption3,
	};

	namespace Events
	{
		enum class HandleResult : uint8_t
		{
			kContinue = 0,
			kStop = 1
		};

		struct TakingItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
			// Set this to HandleResult::kStop to prevent the item from being taken.
			HandleResult result = HandleResult::kContinue;
		};

		struct TakeItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
		};

		struct SelectItemEvent
		{
			RE::Actor* actor;
			RE::ObjectRefHandle container;
			const ItemStack* stack;
		};

		struct OpeningLootMenuEvent
		{
			RE::ObjectRefHandle container;
			// Set this to HandleResult::kStop to prevent the loot menu from opening.
			HandleResult result = HandleResult::kContinue;
		};

		struct OpenLootMenuEvent
		{
			RE::ObjectRefHandle container;
		};

		struct CloseLootMenuEvent
		{
			RE::ObjectRefHandle container;
		};

		struct InvalidateLootMenuEvent
		{
			RE::ObjectRefHandle container;
			const RE::BSTArray<ItemStack>& inventory;
		};

		struct ModifyInventoryEvent
		{
			RE::ObjectRefHandle container;
			// Modify this array to change what is displayed in the item list.
			// Each ItemStack owns its InventoryEntryData object.
			// - If you remove entries, make sure to delete their InventoryEntryData objects to avoid leaking memory.
			// - If you add entries, allocate the InventoryEntryData objects with the new operator. QuickLoot will delete them once they are no longer needed.
			RE::BSTArray<ItemStack>& inventory;
		};

		struct PopulateInfoBarEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if the container is empty.
			const ItemStack* stack;
			// Populate this array with text you want to display in the info bar. Some HTML is supported.
			RE::BSTArray<RE::BSString> result;
		};

		struct ButtonDefinition
		{
			RE::BSString label;
			// For a list of valid values, see https://github.com/MissCorruption/QuickLootIE/blob/main/src/Input/ButtonArtIndex.h
			uint16_t buttonArtIndex;
		};

		struct ButtonDefinition2
		{
			RE::BSString label;
			// For a list of valid values, see https://github.com/MissCorruption/QuickLootIE/blob/main/src/Input/ButtonArtIndex.h
			uint16_t buttonArtIndex;
			// Whether the button is supposed to be red.
			bool stealing;
			// The associated QuickLoot action for default buttons.
			// This is only provided so you can identify them.
			// Changing this field doesn't have any effect.
			QuickLootAction action = QuickLootAction::kNone;
		};

		struct PopulateButtonBarEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if the container is empty.
			const ItemStack* stack;
			// Populate this array with buttons you want to add.
			RE::BSTArray<ButtonDefinition> result;
		};

		struct ModifyButtonBarEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack. This is null if the container is empty.
			const ItemStack* stack;
			// Modify this array as you please.
			RE::BSTArray<ButtonDefinition2>& buttons;
		};

		struct ModifyItemDataEvent
		{
			RE::ObjectRefHandle container;
			// The selected item stack.
			const ItemStack* stack;
			// This is the data object passed to the swf for display.
			RE::GFxValue& data;
		};

		struct InputActionEvent
		{
			RE::ObjectRefHandle container;
			// The action to perform.
			QuickLootAction action;
			// Set this to HandleResult::kStop to cancel the action.
			HandleResult result = HandleResult::kContinue;
		};

		// Fork (eX-QuickLoot): what the loot window shows, sent after every change. In headless mode
		// this is the only picture of the window: draw it yourself from here.
		// Sent on the game's main thread (an SKSE task) in headless mode, on the UI thread otherwise.
		// The pointers and arrays are only valid during the call.
		struct ViewChangedEvent
		{
			// Empty when the window just closed.
			RE::ObjectRefHandle container;
			// The container's display name (empty when closed).
			const char* title;
			// The items in display order (sorted as the window sorts them).
			const RE::BSTArray<ItemStack>& items;
			// Index into items, -1 when there are none.
			int32_t selectedIndex;
			// The button bar exactly as the window would draw it, after ModifyButtonBar handlers.
			const RE::BSTArray<ButtonDefinition2>& buttons;
			// Taking from this container is stealing.
			bool stealing;
		};

		// Fork (eX-QuickLoot, EX2): the same picture plus what More holds, so a window of your own can show
		// the options before More is held and fade between them and the buttons. Sent with ViewChangedEvent.
		struct ViewChangedEvent2
		{
			const ViewChangedEvent& view;
			// What holding More puts on the bar, up to three: Modifier Key Framework's actions for the target
			// first, then its perk options (Feed, Harvest Blood, ...), each with the key that runs it while
			// More is held on the device in use. Empty when the target has none (then there's no More button).
			const RE::BSTArray<ButtonDefinition2>& moreOptions;
			// More is held right now: view.buttons are the options instead of Take, Transfer and More.
			bool moreHeld;
			// The container is locked: -1 when it isn't, else its RE::LOCK_LEVEL (0 Novice ... 4 Master,
			// 5 Requires Key). Only sent for a locked one after SetShowLockedAndEmpty(true): then items is
			// empty (what's inside stays hidden) and the only button is Unlock on Take's key, which runs the
			// game's own activation (lock picking, or the key).
			int32_t lockLevel;
		};

		template <typename TEvent>
		using EventHandler = void (*)(TEvent* e);

		using TakingItemHandler = EventHandler<TakingItemEvent>;
		using TakeItemHandler = EventHandler<TakeItemEvent>;
		using SelectItemHandler = EventHandler<SelectItemEvent>;
		using OpeningLootMenuHandler = EventHandler<OpeningLootMenuEvent>;
		using OpenLootMenuHandler = EventHandler<OpenLootMenuEvent>;
		using CloseLootMenuHandler = EventHandler<CloseLootMenuEvent>;
		using InvalidateLootMenuHandler = EventHandler<InvalidateLootMenuEvent>;
		using ModifyInventoryHandler = EventHandler<ModifyInventoryEvent>;
		using PopulateInfoBarHandler = EventHandler<PopulateInfoBarEvent>;
		using PopulateButtonBarHandler = EventHandler<PopulateButtonBarEvent>;
		using ModifyButtonBarHandler = EventHandler<ModifyButtonBarEvent>;
		using ModifyItemDataHandler = EventHandler<ModifyItemDataEvent>;
		using InputActionHandler = EventHandler<InputActionEvent>;
		using ViewChangedHandler = EventHandler<ViewChangedEvent>;
		using ViewChanged2Handler = EventHandler<ViewChangedEvent2>;
	}

	using namespace Events;

	enum class ApiVersion
	{
		kV20, kV21,

		kLatest = kV21
	};

	class QuickLootAPI
	{
	public:
		QuickLootAPI() = delete;
		~QuickLootAPI() = delete;
		QuickLootAPI(QuickLootAPI const&) = delete;
		QuickLootAPI(QuickLootAPI const&&) = delete;
		QuickLootAPI operator=(QuickLootAPI&) = delete;
		QuickLootAPI operator=(QuickLootAPI&&) = delete;

		static constexpr const char* SERVER_PLUGIN_NAME = "QuickLootIE";

		template<typename TInterface>
		static TInterface* LoadInterface(const char* procName)
		{
			using GetInterfaceProc = TInterface* (*)();

			const auto dllHandle = GetModuleHandleA(SERVER_PLUGIN_NAME);
			if (!dllHandle) {
				return nullptr;
			}

			const auto getInterfaceProc = reinterpret_cast<GetInterfaceProc>(GetProcAddress(dllHandle, procName));
			if (!getInterfaceProc) {
				return nullptr;
			}

			return getInterfaceProc();
		}

		// Call this before any other API function and pass your own plugin name.
		static bool Init(const char* plugin, ApiVersion minVersion = ApiVersion::kLatest)
		{
			_plugin = plugin;
			_interfaceV20 = LoadInterface<InterfaceV20>("GetQuickLootInterfaceV20");
			_interfaceV21 = LoadInterface<InterfaceV21>("GetQuickLootInterfaceV21");
			_interfaceEX1 = LoadInterface<InterfaceEX1>("GetQuickLootInterfaceEX1");
			_interfaceEX2 = LoadInterface<InterfaceEX2>("GetQuickLootInterfaceEX2");
			_interfaceEX3 = LoadInterface<InterfaceEX3>("GetQuickLootInterfaceEX3");

			return IsReady(minVersion);
		}

		// Fork (EX2): also open on locked containers (no rows, an Unlock button, ViewChangedEvent2::lockLevel)
		// and stay open on empty ones, so a window of your own can show a note line instead. Off by default;
		// QuickLoot's own empty-container setting still applies to the Flash window when nobody asks.
		static void SetShowLockedAndEmpty(bool show)
		{
			if (_interfaceEX2) {
				_interfaceEX2->SetShowLockedAndEmpty(_plugin, show);
			}
		}

		// Fork (EX3): stay open on empty containers only (the note line), while locked ones keep QuickLoot's
		// usual behaviour (no window; the game's activation prompt handles the lock). False when the fork is
		// older than EX3: then nothing changed.
		static bool SetShowEmpty(bool show)
		{
			if (!_interfaceEX3) {
				return false;
			}
			_interfaceEX3->SetShowEmpty(_plugin, show);
			return true;
		}

		// Fork: true when the fork is new enough to send ViewChangedEvent2 (More's options and held state).
		static bool HasViewChanged2()
		{
			return _interfaceEX2;
		}

		// Fork (EX2): sent right after each ViewChangedEvent.
		static void RegisterViewChanged2Handler(ViewChanged2Handler handler)
		{
			if (_interfaceEX2) {
				_interfaceEX2->RegisterViewChanged2Handler(_plugin, handler);
			}
		}

		// Fork (eX-QuickLoot): true when the fork's own additions (headless mode) are there.
		// Upstream QuickLoot IE doesn't have them.
		static bool IsForkReady()
		{
			return _interfaceEX1;
		}

		// Fork: while any plugin asks for it, QuickLoot draws no window of its own (and needs no swf,
		// so no SkyUI). Everything else keeps working: which container is looked at, the keys, taking
		// items, perk options, and every event. Draw the window yourself from ViewChangedEvent, and
		// scroll or take through PerformInputAction. Best set once, at kPostLoad or kDataLoaded.
		static void SetHeadless(bool headless)
		{
			if (_interfaceEX1) {
				_interfaceEX1->SetHeadless(_plugin, headless);
			}
		}

		static void RegisterViewChangedHandler(ViewChangedHandler handler)
		{
			if (_interfaceEX1) {
				_interfaceEX1->RegisterViewChangedHandler(_plugin, handler);
			}
		}

		static bool IsReady(ApiVersion minVersion = ApiVersion::kLatest)
		{
			switch (minVersion) {
			case ApiVersion::kV20:
				return _interfaceV20;

			case ApiVersion::kV21:
				return _interfaceV21;

			default:
				return false;
			}
		}

		static void DisableLootMenu()
		{
			if (_interfaceV20) {
				_interfaceV20->DisableLootMenu(_plugin);
			}
		}

		static void EnableLootMenu()
		{
			if (_interfaceV20) {
				_interfaceV20->EnableLootMenu(_plugin);
			}
		}

		static void RegisterTakingItemHandler(TakingItemHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterTakingItemHandler(_plugin, handler);
			}
		}

		static void RegisterTakeItemHandler(TakeItemHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterTakeItemHandler(_plugin, handler);
			}
		}

		static void RegisterSelectItemHandler(SelectItemHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterSelectItemHandler(_plugin, handler);
			}
		}

		static void RegisterOpeningLootMenuHandler(OpeningLootMenuHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterOpeningLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterOpenLootMenuHandler(OpenLootMenuHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterOpenLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterCloseLootMenuHandler(CloseLootMenuHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterCloseLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterInvalidateLootMenuHandler(InvalidateLootMenuHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterInvalidateLootMenuHandler(_plugin, handler);
			}
		}

		static void RegisterModifyInventoryHandler(ModifyInventoryHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterModifyInventoryHandler(_plugin, handler);
			}
		}

		static void RegisterPopulateInfoBarHandler(PopulateInfoBarHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterPopulateInfoBarHandler(_plugin, handler);
			}
		}

		[[deprecated("This only exists for backwards compatibility. Use RegisterModifyButtonBarHandler instead.")]]
		static void RegisterPopulateButtonBarHandler(PopulateButtonBarHandler handler)
		{
			if (_interfaceV20) {
				_interfaceV20->RegisterPopulateButtonBarHandler(_plugin, handler);
			}
		}

		static void ForceCurrentContainer(RE::ObjectRefHandle container)
		{
			if (_interfaceV20) {
				_interfaceV20->ForceCurrentContainer(_plugin, container);
			}
		}

		static void ClearForcedContainer()
		{
			if (_interfaceV20) {
				_interfaceV20->ClearForcedContainer(_plugin);
			}
		}

		static void CloseLootMenu()
		{
			if (_interfaceV20) {
				_interfaceV20->CloseLootMenu(_plugin);
			}
		}

		static void RefreshLootMenu()
		{
			if (_interfaceV20) {
				_interfaceV20->RefreshLootMenu(_plugin);
			}
		}

		static void RegisterModifyButtonBarHandler(ModifyButtonBarHandler handler)
		{
			if (_interfaceV21) {
				_interfaceV21->RegisterModifyButtonBarHandler(_plugin, handler);
			}
		}

		static void RegisterModifyItemDataHandler(ModifyItemDataHandler handler)
		{
			if (_interfaceV21) {
				_interfaceV21->RegisterModifyItemDataHandler(_plugin, handler);
			}
		}

		static void RegisterInputActionHandler(InputActionHandler handler)
		{
			if (_interfaceV21) {
				_interfaceV21->RegisterInputActionHandler(_plugin, handler);
			}
		}

		static void PerformInputAction(QuickLootAction action)
		{
			if (_interfaceV21) {
				_interfaceV21->PerformInputAction(_plugin, action);
			}
		}

	private:
		friend class APIServer;

		// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
		struct InterfaceV20
		{
			virtual void DisableLootMenu(const char* plugin);
			virtual void EnableLootMenu(const char* plugin);

			virtual void RegisterTakingItemHandler(const char* plugin, TakingItemHandler handler);
			virtual void RegisterTakeItemHandler(const char* plugin, TakeItemHandler handler);
			virtual void RegisterSelectItemHandler(const char* plugin, SelectItemHandler handler);
			virtual void RegisterOpeningLootMenuHandler(const char* plugin, OpeningLootMenuHandler handler);
			virtual void RegisterOpenLootMenuHandler(const char* plugin, OpenLootMenuHandler handler);
			virtual void RegisterCloseLootMenuHandler(const char* plugin, CloseLootMenuHandler handler);
			virtual void RegisterInvalidateLootMenuHandler(const char* plugin, InvalidateLootMenuHandler handler);

			virtual void RegisterModifyInventoryHandler(const char* plugin, ModifyInventoryHandler handler);
			virtual void RegisterPopulateInfoBarHandler(const char* plugin, PopulateInfoBarHandler handler);
			virtual void RegisterPopulateButtonBarHandler(const char* plugin, PopulateButtonBarHandler handler);

			virtual void ForceCurrentContainer(const char* plugin, RE::ObjectRefHandle container);
			virtual void ClearForcedContainer(const char* plugin);
			virtual void CloseLootMenu(const char* plugin);
			virtual void RefreshLootMenu(const char* plugin);
		};

		struct InterfaceV21 : public InterfaceV20
		{
			virtual void RegisterModifyButtonBarHandler(const char* plugin, ModifyButtonBarHandler handler);
			virtual void RegisterModifyItemDataHandler(const char* plugin, ModifyItemDataHandler handler);

			virtual void RegisterInputActionHandler(const char* plugin, InputActionHandler handler);
			virtual void PerformInputAction(const char* plugin, QuickLootAction action);
		};

		// Fork (eX-QuickLoot) additions, kept apart from upstream's version numbers.
		struct InterfaceEX1 : public InterfaceV21
		{
			virtual void SetHeadless(const char* plugin, bool headless);
			virtual void RegisterViewChangedHandler(const char* plugin, ViewChangedHandler handler);
		};

		struct InterfaceEX2 : public InterfaceEX1
		{
			virtual void RegisterViewChanged2Handler(const char* plugin, ViewChanged2Handler handler);
			virtual void SetShowLockedAndEmpty(const char* plugin, bool show);
		};

		struct InterfaceEX3 : public InterfaceEX2
		{
			virtual void SetShowEmpty(const char* plugin, bool show);
		};

		static inline const char* _plugin;
		static inline InterfaceV20* _interfaceV20;
		static inline InterfaceV21* _interfaceV21;
		static inline InterfaceEX1* _interfaceEX1;
		static inline InterfaceEX2* _interfaceEX2;
		static inline InterfaceEX3* _interfaceEX3;
	};
}
