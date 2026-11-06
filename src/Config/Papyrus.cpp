#include "Papyrus.h"

#include "LootMenu.h"
#include "UserSettings.h"
#include "Util/ScriptObject.h"

#include <fstream>

namespace QuickLoot::Config
{
	void Papyrus::Install()
	{
		SKSE::GetPapyrusInterface()->Register(RegisterFunctions);

		logger::info("Installed {}", typeid(Papyrus).name());
	}

	bool Papyrus::RegisterFunctions(RE::BSScript::IVirtualMachine* vm)
	{
#define RegisterScriptFunction(name) vm->RegisterFunction(#name, "QuickLootIENative", name);

		RegisterScriptFunction(GetDllVersion);
		RegisterScriptFunction(GetSwfVersion);
		RegisterScriptFunction(SetFrameworkQuest);
		RegisterScriptFunction(LogWithPlugin);
		RegisterScriptFunction(UpdateVariables);
		RegisterScriptFunction(FormatSortOptionsList);
		RegisterScriptFunction(RemoveSortOptionPriority);
		RegisterScriptFunction(InsertSortOptionPriority);
		RegisterScriptFunction(GetSortingPreset);
		RegisterScriptFunction(GetSortingPresets);
		RegisterScriptFunction(AddPresetsToArray);

#undef RegisterScriptFunction
		return true;
	};

	void Papyrus::SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* quest)
	{
		if (!quest) {
			logger::info("No quest passed to registration function");
			return;
		};

		MCMQuest = quest;

		logger::info("MCM quest pointer set successfully");

		UserSettings::Update();
	};

	Papyrus::IniValues Papyrus::ReadIni()
	{
		IniValues values{};

		std::ifstream ifs(INI_PATH);
		if (!ifs.is_open()) {
			logger::info("No {}, using the defaults", INI_PATH);
			return values;
		}

		const auto trim = [](std::string_view text) {
			while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
			while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
			return text;
		};

		std::string line;
		while (std::getline(ifs, line)) {
			auto text = trim(line);
			if (text.empty() || text.front() == ';' || text.front() == '#' || text.front() == '[') {
				continue;
			}

			const auto equals = text.find('=');
			if (equals == std::string_view::npos) {
				continue;
			}

			auto value = text.substr(equals + 1);
			if (const auto comment = value.find_first_of(";#"); comment != std::string_view::npos) {
				value = value.substr(0, comment);
			}

			std::string key{ trim(text.substr(0, equals)) };
			std::ranges::transform(key, key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			values[key] = std::string{ trim(value) };
		}

		logger::info("Read {} values from {}", values.size(), INI_PATH);
		return values;
	}

	const std::string* Papyrus::FindIniValue(const IniValues& ini, const std::string& propertyName)
	{
		std::string key = propertyName.starts_with("QLIE_") ? propertyName.substr(5) : propertyName;
		std::ranges::transform(key, key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		const auto it = ini.find(key);
		return it == ini.end() ? nullptr : &it->second;
	}

	bool Papyrus::ParseIniBool(const std::string& text, bool& out)
	{
		std::string lower = text;
		std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (lower == "1" || lower == "true" || lower == "yes" || lower == "on") {
			out = true;
			return true;
		}

		if (lower == "0" || lower == "false" || lower == "no" || lower == "off") {
			out = false;
			return true;
		}

		return false;
	}

	void Papyrus::UpdateVariables(RE::StaticFunctionTag*)
	{
		PROFILE_SCOPE;

		// SkyUI's settings menu wins when it's there. Without SkyUI its quest script never binds, so the
		// settings come from QuickLootIE.ini over the defaults (also what applies before the menu registers).
		if (auto script = Util::ScriptObject::FromForm(MCMQuest, "QuickLootIEMCM"); script.IsValid()) {
			logger::info("Settings from the MCM");
			LoadAll(script);
			return;
		}

		logger::info("Settings from {}", INI_PATH);
		const auto ini = ReadIni();
		LoadAll(ini);
	}

	template <class Source>
	void Papyrus::LoadAll(Source& source)
	{
#define LoadSettingsVar(name, ...) LoadSetting(source, name, #name, __VA_ARGS__)

		// General > Behavior Settings
		LoadSettingsVar(QLIE_ShowInCombat, true);
		LoadSettingsVar(QLIE_ShowWhenEmpty, false);
		LoadSettingsVar(QLIE_ShowWhenSneaking, true);
		LoadSettingsVar(QLIE_ShowWhenUnlocked, true);
		LoadSettingsVar(QLIE_ShowInThirdPerson, true);
		LoadSettingsVar(QLIE_ShowWhenMounted, false);
		LoadSettingsVar(QLIE_EnableForContainers, true);
		LoadSettingsVar(QLIE_EnableForCorpses, true);
		LoadSettingsVar(QLIE_EnableForAnimals, true);
		LoadSettingsVar(QLIE_EnableForDragons, true);
		LoadSettingsVar(QLIE_BreakInvisibility, true);
		LoadSettingsVar(QLIE_PlayScrollSound, true);

		// Display > Window Settings
		LoadSettingsVar(QLIE_WindowOffsetX, 100);
		LoadSettingsVar(QLIE_WindowOffsetY, -200);
		LoadSettingsVar(QLIE_WindowScale, 1.0f);
		LoadSettingsVar(QLIE_WindowAnchor, 0);
		LoadSettingsVar(QLIE_WindowMinLines, 0);
		LoadSettingsVar(QLIE_WindowMaxLines, 7);
		LoadSettingsVar(QLIE_WindowOpacityNormal, 1.0f);
		LoadSettingsVar(QLIE_WindowOpacityEmpty, 0.3f);

		// Display > Icon Settings
		LoadSettingsVar(QLIE_ShowIconItem, true);
		LoadSettingsVar(QLIE_ShowIconBest, false); // disabled by default due to performance concerns
		LoadSettingsVar(QLIE_ShowIconRead, true);
		LoadSettingsVar(QLIE_ShowIconStolen, true);
		LoadSettingsVar(QLIE_ShowIconEnchanted, true);
		LoadSettingsVar(QLIE_ShowIconEnchantedKnown, true);
		LoadSettingsVar(QLIE_ShowIconEnchantedSpecial, true);

		// Display > Info Columns
		LoadSettingsVar(QLIE_InfoColumns, { "value", "weight", "valuePerWeight" });

		// Sorting
		LoadSettingsVar(QLIE_SortRulesActive, {});

		// Controls
		LoadSettingsVar(QLIE_KeybindingUse, 18);
		LoadSettingsVar(QLIE_KeybindingTake, 18);
		LoadSettingsVar(QLIE_KeybindingTakeAll, 19);
		LoadSettingsVar(QLIE_KeybindingTransfer, 16);
		LoadSettingsVar(QLIE_KeybindingDisable, -1);
		LoadSettingsVar(QLIE_KeybindingEnable, -1);

		LoadSettingsVar(QLIE_KeybindingUseModifier, 42);
		LoadSettingsVar(QLIE_KeybindingTakeModifier, -1);
		LoadSettingsVar(QLIE_KeybindingTakeAllModifier, -1);
		LoadSettingsVar(QLIE_KeybindingTransferModifier, -1);
		LoadSettingsVar(QLIE_KeybindingDisableModifier, -1);
		LoadSettingsVar(QLIE_KeybindingEnableModifier, -1);

		LoadSettingsVar(QLIE_KeybindingUseGamepad, 279);
		LoadSettingsVar(QLIE_KeybindingTakeGamepad, 276);
		LoadSettingsVar(QLIE_KeybindingTakeAllGamepad, 278);
		LoadSettingsVar(QLIE_KeybindingTransferGamepad, 271);
		LoadSettingsVar(QLIE_KeybindingDisableGamepad, -1);
		LoadSettingsVar(QLIE_KeybindingEnableGamepad, -1);

		LoadSettingsVar(QLIE_KeybindingUseGamepadModifier, -1);
		LoadSettingsVar(QLIE_KeybindingTakeGamepadModifier, -1);
		LoadSettingsVar(QLIE_KeybindingTakeAllGamepadModifier, -1);
		LoadSettingsVar(QLIE_KeybindingTransferGamepadModifier, -1);
		LoadSettingsVar(QLIE_KeybindingDisableGamepadModifier, -1);
		LoadSettingsVar(QLIE_KeybindingEnableGamepadModifier, -1);

		// Compatibility > LOTD Icons
		LoadSettingsVar(QLIE_ShowIconArtifactNew, true);
		LoadSettingsVar(QLIE_ShowIconArtifactCarried, true);
		LoadSettingsVar(QLIE_ShowIconArtifactDisplayed, true);

		// Compatibility > Completionist Icons
		LoadSettingsVar(QLIE_ShowIconCompletionistNeeded, true);
		LoadSettingsVar(QLIE_ShowIconCompletionistCollected, true);
		LoadSettingsVar(QLIE_ShowIconCompletionistDisplayable, true);
		LoadSettingsVar(QLIE_ShowIconCompletionistDisplayed, true);
		LoadSettingsVar(QLIE_ShowIconCompletionistOccupied, true);

#undef LoadSettingsVar
	}

	void Papyrus::LogWithPlugin(RE::StaticFunctionTag*, std::string message)
	{
		logger::info("! {}", message);
	}

	std::string Papyrus::GetDllVersion(RE::StaticFunctionTag*)
	{
		const auto plugin = SKSE::PluginDeclaration::GetSingleton();
		return plugin->GetVersion().string(".");
	}

	std::string Papyrus::GetSwfVersion(RE::StaticFunctionTag*)
	{
		return std::to_string(LootMenu::GetSwfVersion());
	}

	std::vector<std::string> Papyrus::FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList)
	{
		std::erase_if(options, [&userList](const std::string& option) {
			return std::ranges::find(userList, option) != userList.end();
		});

		return options;
	}

	std::vector<std::string> Papyrus::RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int elementPos)
	{
		if (elementPos >= 0 && elementPos < static_cast<int>(userList.size())) {
			userList.erase(userList.begin() + elementPos);
		}

		return userList;
	}

	std::vector<std::string> Papyrus::InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int elementPos)
	{
		if (elementPos >= 0 && elementPos <= static_cast<int>(userList.size())) {
			userList.insert(userList.begin() + elementPos, newElement);
		}

		return userList;
	}

	std::vector<std::string> Papyrus::AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList)
	{
		userList.insert(userList.end(), presetList.begin(), presetList.end());
		return userList;
	}

	std::vector<std::string> Papyrus::GetSortingPresets(RE::StaticFunctionTag*)
	{
		return ConvertArrayToVector(SortingPresets);
	}

	std::vector<std::string> Papyrus::GetSortingPreset(RE::StaticFunctionTag*, int presetChoice)
	{
		switch (presetChoice) {
		case 1:
			return ConvertArrayToVector(SortingPresets_Default);
		case 2:
			return ConvertArrayToVector(SortingPresets_Goblin);
		default:
			return ConvertArrayToVector(SortingPresets_Default);
		}
	}
}
