#pragma once

namespace QuickLoot
{
	class MenuVisibilityManager
	{
	public:
		MenuVisibilityManager() = delete;
		~MenuVisibilityManager() = delete;
		MenuVisibilityManager(MenuVisibilityManager&&) = delete;
		MenuVisibilityManager(const MenuVisibilityManager&) = delete;
		MenuVisibilityManager& operator=(MenuVisibilityManager&&) = delete;
		MenuVisibilityManager& operator=(const MenuVisibilityManager&) = delete;

		static void DisableLootMenu(const std::string& modName);
		static void EnableLootMenu(const std::string& modName);

		static void OnCameraStateChanged(RE::CameraState state);
		static void OnCombatStateChanged(RE::ACTOR_COMBAT_STATE state);
		static void OnContainerChanged(RE::FormID container);
		static void OnCrosshairRefChanged(const RE::ObjectRefHandle& ref);
		static void OnGrabStateChanged();
		static void OnLifeStateChanged(RE::Actor& actor);
		static void OnLockChanged(RE::TESObjectREFR& container);
		static void OnMenuOpenClose(bool opening, const RE::BSFixedString& menuName);
		static void OnSneakStateChanged();

		static void SetForcedContainer(RE::ObjectRefHandle container);
		static bool IsForcedContainer(const RE::ObjectRefHandle& container);

		// Fork: a consumer that draws its own window asked to see empty containers too (EX3 SetShowEmpty), or
		// locked and empty ones (EX2 SetShowLockedAndEmpty, both flags at once).
		static void SetShowLockedAndEmpty(const std::string& plugin, bool show);
		static void SetShowEmpty(const std::string& plugin, bool show);
		static bool ShowLocked() { return _showLocked; }
		static bool ShowEmpty() { return _showEmpty; }

		static inline bool LOG_EVENTS = false;

	private:
		static inline RE::ObjectRefHandle _focusedRef{};
		static inline RE::ObjectRefHandle _currentContainer{};
		static inline RE::ObjectRefHandle _forcedContainer{};

		static inline std::set<std::string> _disablingMods{};

		static inline std::mutex _showLock{};
		static inline std::set<std::string> _showLockedPlugins{};
		static inline std::set<std::string> _showEmptyPlugins{};
		static inline std::atomic<bool> _showLocked{ false };
		static inline std::atomic<bool> _showEmpty{ false };

		// Adds or removes the plugin and updates the flag; true when the flag changed.
		static bool SetShow(std::set<std::string>& plugins, std::atomic<bool>& flag, const std::string& plugin, bool show);

		static RE::TESObjectREFRPtr GetContainerObject(RE::ObjectRefHandle ref);
		static bool IsValidCameraState(RE::CameraState state);
		static const char* GetMenuNameSafe(const RE::IMenu* menu);
		static const char* FindBlockingMenu();
		static bool IsContainerBlacklisted(const RE::TESObjectREFRPtr& container);
		static bool CanOpen(const RE::TESObjectREFRPtr& container);
		static void RefreshOpenState();
		static void RefreshInventory();
	};
}
