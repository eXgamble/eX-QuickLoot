#include "SanityChecks.h"

#include "LootMenu.h"
#include "Util/FormUtil.h"

// No popups: a Windows message box here blocks the game's main thread during startup.
// The loot window just stays off and the reason goes to the log.
void ShowFatalError(const char* message)
{
	logger::error("{}", message);
}

bool QuickLoot::SanityChecks::PerformChecks()
{
	return ValidateEsp() & ValidateSwf();
}

bool QuickLoot::SanityChecks::ValidateEsp()
{
	if (!Util::FormUtil::FormExists("QuickLootIE.esp", 0x001)) {
		logger::error("QuickLootIE.esp is not loaded");
		return false;
	}

	return true;
}

bool QuickLoot::SanityChecks::ValidateSwf()
{
	if (std::filesystem::exists(LEGACY_SWF)) {
		logger::warn("LootMenu.swf present");

		// The presence of LootMenu.swf does not prevent QuickLoot from working,
		// so it shouldn't fail the sanity check.
	}

	switch (const int version = LootMenu::GetSwfVersion()) {
	case -2:
		if (!std::filesystem::exists(CURRENT_SWF)) {
			logger::error("LootMenuIE.swf missing");

			ShowFatalError(
				"LootMenuIE.swf is missing. "
				"This file is required by eX QuickLoot. "
				"Please make sure the mod is installed correctly."
				"\n\nExit game now? (Recommend yes)");

			return false;
		}

		logger::error("Failed to load swf");

		ShowFatalError(
			"LootMenuIE.swf exists but failed to load. "
			"If you are using a reskin, make sure all of its dependencies are installed."
			"\n\nExit game now? (Recommend yes)");

		return false;

	case -1:
		logger::error("Incompatible swf");

		ShowFatalError(
			"The installed version of LootMenuIE.swf is not compatible with eX QuickLoot."
			"\n\nExit game now? (Recommend yes)");

		return false;

	default:
		if (version < LootMenu::CURRENT_MENU_VERSION) {
			logger::warn("LootMenuIE.swf reports version {}, but current loot menu version is {}", version, LootMenu::CURRENT_MENU_VERSION);
		}
		break;
	}

	return true;
}
