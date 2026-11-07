#pragma once

#include "Input.h"

namespace QuickLoot::Input
{
	class InputManager
	{
	public:
		InputManager() = delete;
		~InputManager() = delete;
		InputManager(InputManager&&) = delete;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		static void Install();
		static void UpdateMappings();

		static void UpdateModifierStates();

		static void BlockConflictingInputs();
		static void UnblockConflictingInputs();

		static void HandleButtonEvent(const RE::ButtonEvent* event);
		static void HandleThumbstickEvent(const RE::ThumbstickEvent* event);

		static std::vector<Keybinding> GetButtonBarKeybindings();
		// Fork: the More slots' keybindings on the device in use, whether More is held or not.
		static std::vector<Keybinding> GetMoreSlotKeybindings();

		// Fork: the More key (MKF's modifier key: Left Shift / LB unless MKF's ini says otherwise).
		// Held over a target with perk options, the bar swaps to those options on the slot keys.
		static void SetPerkOptionCount(int count);
		static DeviceKey GetMoreKey();
		static bool IsMoreLive() { return _moreLive.load(); }

	private:
		static inline std::atomic<int> _perkOptionCount{ 0 };
		static inline std::atomic<bool> _moreLive{ false };
		static inline DeviceKey _moreKeyKeyboard = DeviceKey::Get(KeyboardKey::kLeftShift);
		static inline DeviceKey _moreKeyGamepad = DeviceKey::Get(GamepadInput::kLeftShoulder);
		// The game controls of More slots 2 and 3: MKF's own (sSlot2Control / sSlot3Control in its ini).
		static inline std::string _slot2Control = "Ready Weapon";
		static inline std::string _slot3Control = "Jump";
		// Game mappings taken while More is live (Jump on a slot key), with their old group flags.
		static inline std::vector<std::pair<UserEventMapping*, std::uint32_t>> _liveTaken{};

		static void ReadMoreKeys();
		static void SetMoreLive(bool live);

		static inline std::vector<Keybinding> _keybindings{};
		static inline std::set<DeviceKey> _allInputKeys{};
		static inline std::set<DeviceKey> _allModifierKeys{};
		static inline std::set<DeviceKey> _allHoldKeys{};
		static inline std::set<DeviceKey> _heldModifierKeys{};
		static inline const RE::Setting* _grabDelaySetting = nullptr;
		static inline bool _triggerOnActivateRelease = false;

		static void ReloadKeybindings();
		static Keybinding* FindConflictingKeybinding(const UserEventMapping&, DeviceType);
		static Keybinding* FindSatisfiedKeybinding(DeviceKey key, float holdTime);

		static bool IsKeyPressed(DeviceKey key);
		static RE::BSInputDevice* GetInputDevice(DeviceType deviceType);

		static void SendFakeButtonEvent(DeviceType device, int idCode, float value, float heldDownSecs);

		static bool HandleGrab(const RE::ButtonEvent* event, const Keybinding* keybinding);
		static bool TryGrab();
		static void TriggerKeybinding(const Keybinding* keybinding);
		static void HandleHold(const RE::ButtonEvent* event, Keybinding* keybinding);

		static void WalkMappings(const std::function<void(UserEventMapping&, DeviceType)>& functor, bool allContexts = false);
	};
}
