#pragma once

#include "Input/Input.h"
#include "LootMenu.h"

namespace QuickLoot
{
	// Interface for interacting with the loot menu outside of UI threads.
	class LootMenuManager
	{
	public:
		LootMenuManager() = delete;
		~LootMenuManager() = delete;
		LootMenuManager(LootMenuManager&&) = delete;
		LootMenuManager(const LootMenuManager&) = delete;
		LootMenuManager& operator=(LootMenuManager&&) = delete;
		LootMenuManager& operator=(const LootMenuManager&) = delete;

		static bool IsShowing();
		static void RequestShow(const RE::ObjectRefHandle& container);
		static void RequestHide();
		static void RequestRefresh(RefreshFlags flags);

		static void OnInputAction(Input::QuickLootAction action);

		// Fork: headless mode (no Flash window; an API consumer draws its own). On while any plugin asks.
		static bool IsHeadless();
		static void SetHeadless(const std::string& plugin, bool headless);
		// False when LootMenuIE.swf can't load (no SkyUI): then only headless mode opens anything.
		static void SetFlashAvailable(bool available) { _flashAvailable = available; }

		// These should only ever be called from the loot menu itself.
		static void ProcessPendingTasks(LootMenu& menu);
		static void SaveLastSelectedIndex(const RE::ObjectRefHandle& container, int selectedIndex);

	private:
		using LootMenuTask = std::function<void(LootMenu& menu)>;

		static inline std::recursive_mutex _lock{};
		static inline std::vector<LootMenuTask> _taskQueue{};
		static inline RE::ObjectRefHandle _currentContainer{};
		static inline RE::ObjectRefHandle _lastContainer{};
		static inline int _lastSelectedIndex = 0;

		static inline std::set<std::string> _headlessPlugins{};
		static inline std::atomic_bool _headless = false;
		static inline bool _flashAvailable = true;
		// The headless window: a LootMenu that never enters the UI, driven by SKSE tasks.
		// Only touched from those tasks (the game's main thread).
		static inline std::unique_ptr<LootMenu> _headlessMenu{};
		static inline std::atomic_bool _pumpQueued = false;

		static void EnsureOpen();
		static void ScheduleHeadlessPump();
		static void QueueLootMenuTask(LootMenuTask task);
	};
}
