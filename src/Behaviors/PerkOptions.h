#pragma once

namespace QuickLoot::Behaviors
{
	// The game's own "activate choice" perk options on a reference (Namira's Ring "Feed",
	// the Essence Extractor's "Harvest Blood", mods' own options). Vanilla shows them in a
	// popup when the player activates; this runs one directly instead.
	struct PerkOption
	{
		std::string  label;
		RE::BGSPerk* perk = nullptr;
		bool         replacesDefault = false;
		bool         runsImmediately = false;
	};

	class PerkOptions
	{
	public:
		PerkOptions() = delete;

		static void Install();

		// Options whose conditions pass for the player on this target. Read-only.
		static std::vector<PerkOption> List(RE::TESObjectREFR* a_target);

		// Activates the target as the player and answers the game's choice popup with the
		// option of this label. Call from an SKSE task.
		static bool Run(RE::TESObjectREFR* a_target, std::string_view a_label);

		// Activates the target and answers the popup with the default choice (Search), so the
		// container menu opens. Call from an SKSE task.
		static bool Search(RE::TESObjectREFR* a_target);

		// Test natives, script "eXQuickLootSpike" (dev only).
		static bool RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm);
	};
}
