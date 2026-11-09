#pragma once

#include "Util/ScriptObject.h"

#include <cctype>
#include <charconv>
#include <unordered_map>

namespace QuickLoot::Config
{
	static RE::TESQuest* MCMQuest{ nullptr };

	inline bool QLIE_ShowInCombat;
	inline bool QLIE_ShowWhenEmpty;
	inline bool QLIE_ShowWhenSneaking;
	inline bool QLIE_ShowWhenUnlocked;
	inline bool QLIE_ShowInThirdPerson;
	inline bool QLIE_ShowWhenMounted;
	inline bool QLIE_EnableForContainers;
	inline bool QLIE_EnableForCorpses;
	inline bool QLIE_EnableForAnimals;
	inline bool QLIE_EnableForDragons;
	inline bool QLIE_BreakInvisibility;
	inline bool QLIE_PlayScrollSound;

	inline int QLIE_WindowOffsetX;
	inline int QLIE_WindowOffsetY;
	inline float QLIE_WindowScale;
	inline int QLIE_WindowAnchor;
	inline int QLIE_WindowMinLines;
	inline int QLIE_WindowMaxLines;
	inline float QLIE_WindowOpacityNormal;
	inline float QLIE_WindowOpacityEmpty;

	inline bool QLIE_ShowIconItem;
	inline bool QLIE_ShowIconBest;
	inline bool QLIE_ShowIconRead;
	inline bool QLIE_ShowIconStolen;
	inline bool QLIE_ShowIconEnchanted;
	inline bool QLIE_ShowIconEnchantedKnown;
	inline bool QLIE_ShowIconEnchantedSpecial;

	inline std::vector<std::string> QLIE_InfoColumns;

	inline std::vector<std::string> QLIE_SortRulesActive;

	inline int QLIE_KeybindingDisable;
	inline int QLIE_KeybindingEnable;

	inline int QLIE_KeybindingDisableModifier;
	inline int QLIE_KeybindingEnableModifier;

	inline int QLIE_KeybindingDisableGamepad;
	inline int QLIE_KeybindingEnableGamepad;

	inline int QLIE_KeybindingDisableGamepadModifier;
	inline int QLIE_KeybindingEnableGamepadModifier;

	inline bool QLIE_ShowIconArtifactNew;
	inline bool QLIE_ShowIconArtifactCarried;
	inline bool QLIE_ShowIconArtifactDisplayed;

	inline bool QLIE_ShowIconCompletionistNeeded;
	inline bool QLIE_ShowIconCompletionistCollected;
	inline bool QLIE_ShowIconCompletionistDisplayed;
	inline bool QLIE_ShowIconCompletionistOccupied;
	inline bool QLIE_ShowIconCompletionistDisplayable;

	class Papyrus
	{
	public:
		Papyrus() = delete;
		~Papyrus() = delete;
		Papyrus(Papyrus const&) = delete;
		Papyrus(Papyrus const&&) = delete;
		Papyrus& operator=(Papyrus&) = delete;
		Papyrus& operator=(Papyrus&&) = delete;

		static void Install();
		static void SetFrameworkQuest(RE::StaticFunctionTag*, RE::TESQuest* quest);
		static void LogWithPlugin(RE::StaticFunctionTag*, std::string message);
		static void UpdateVariables(RE::StaticFunctionTag* = nullptr);

		static std::string GetDllVersion(RE::StaticFunctionTag*);
		static std::string GetSwfVersion(RE::StaticFunctionTag*);

		static std::vector<std::string> FormatSortOptionsList(RE::StaticFunctionTag*, std::vector<std::string> options, std::vector<std::string> userList);
		static std::vector<std::string> RemoveSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, int elementPos);
		static std::vector<std::string> InsertSortOptionPriority(RE::StaticFunctionTag*, std::vector<std::string> userList, std::string newElement, int elementPos);

		static std::vector<std::string> GetSortingPresets(RE::StaticFunctionTag*);
		static std::vector<std::string> GetSortingPreset(RE::StaticFunctionTag*, int presetChoice);
		static std::vector<std::string> AddPresetsToArray(RE::StaticFunctionTag*, std::vector<std::string> userList, std::vector<std::string> presetList);

		// Fork: the settings without SkyUI. Keys are the MCM property names without "QLIE_"
		// (ShowInCombat = false), case-insensitive; sections and order don't matter.
		static constexpr auto INI_PATH = "Data\\SKSE\\Plugins\\QuickLootIE.ini";
		using IniValues = std::unordered_map<std::string, std::string>;
		static IniValues ReadIni();

	private:
		static bool RegisterFunctions(RE::BSScript::IVirtualMachine* vm);

		template <class Source>
		static void LoadAll(Source& source);

		static const std::string* FindIniValue(const IniValues& ini, const std::string& propertyName);
		static bool ParseIniBool(const std::string& text, bool& out);

		// Same as the script version below, but from QuickLootIE.ini; a missing or bad value keeps the default.
		template <typename T>
		static void LoadSetting(const IniValues& ini, T& variable, const std::string& propertyName, const T& defaultValue)
		{
			variable = defaultValue;

			const auto* text = FindIniValue(ini, propertyName);
			if (!text) {
				return;
			}

			bool ok = true;

			if constexpr (std::is_same_v<T, bool>) {
				ok = ParseIniBool(*text, variable);
			} else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
				const auto* end = text->data() + text->size();
				const auto [ptr, ec] = std::from_chars(text->data(), end, variable);
				ok = ec == std::errc{} && ptr == end;
			} else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				variable.clear();
				std::string_view rest = *text;
				while (!rest.empty()) {
					const auto comma = rest.find(',');
					auto part = rest.substr(0, comma);
					rest = comma == std::string_view::npos ? std::string_view{} : rest.substr(comma + 1);
					while (!part.empty() && std::isspace(static_cast<unsigned char>(part.front()))) part.remove_prefix(1);
					while (!part.empty() && std::isspace(static_cast<unsigned char>(part.back()))) part.remove_suffix(1);
					if (!part.empty()) {
						variable.emplace_back(part);
					}
				}
			}

			if (!ok) {
				variable = defaultValue;
				logger::warn("{}: \"{}\" in QuickLootIE.ini isn't a valid value, keeping the default", propertyName, *text);
				return;
			}

			logger::trace("{}: {} (ini)", propertyName, *text);
		}

		static std::vector<std::string> ConvertScriptArrayToVector(ScriptArrayPtr scriptArrayPtr)
		{
			std::vector<std::string> result;

			if (!scriptArrayPtr) {
				return result;
			}

			std::size_t arraySize = scriptArrayPtr->size();
			result.reserve(arraySize);

			for (RE::BSScript::Array::size_type i = 0; i < arraySize; ++i) {
				const auto& variable = (*scriptArrayPtr)[i].GetString();
				result.push_back(variable.data());
			}

			return result;
		}

		template <size_t N>
		static std::vector<std::string> ConvertArrayToVector(const std::array<const char*, N>& arr)
		{
			std::vector<std::string> vec;
			vec.reserve(N);

			for (size_t i = 0; i < N; ++i) {
				vec.push_back(arr[i]);
			}

			return vec;
		}

		template <typename T>
		static void LoadSetting(Util::ScriptObject& script, T& variable, const std::string& propertyName, const T& defaultValue)
		{
			const auto* prop = script.GetProperty(propertyName);

			if (!prop) {
				variable = defaultValue;
				logger::trace("{}: not found", propertyName);
				return;
			}

			if constexpr (std::is_same_v<T, bool>) {
				variable = prop->GetBool();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, std::string>) {
				variable = prop->GetString();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, int>) {
				variable = prop->GetSInt();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, float>) {
				variable = prop->GetFloat();
				logger::trace("{}: {}", propertyName, variable);
				return;
			}

			if constexpr (std::is_same_v<T, std::vector<std::string>>) {
				variable = ConvertScriptArrayToVector(prop->GetArray());
				logger::trace("{}: {} strings", propertyName, variable.size());
				return;
			}

			//logger::trace("{}: unsupported type {}", propertyName, typeid(T).name());
		}
	};
}

//These are just examples, we should localise these for the MCM e.g "The Goblin" would be "$qlie_preset_goblin"
constexpr std::array SortingPresets = {
	"Select Preset...",  //Mandatory Entry
	"Default Preset",
	"The Goblin",
	//Add more as you see fit.
};

constexpr std::array SortingPresets_Default = {
	"$qlie_SortRule_Gold",
	"$qlie_SortRule_Gems",
	"$qlie_SortRule_SoulGems",
	"$qlie_SortRule_Lockpicks",
	"$qlie_SortRule_OresIngots",
	"$qlie_SortRule_Potions",
	"$qlie_SortRule_FoodDrinks",
	"$qlie_SortRule_Books",
	"$qlie_SortRule_Notes",
	"$qlie_SortRule_Scrolls",
	"$qlie_SortRule_Weapons",
	"$qlie_SortRule_ArrowsBolts",
	"$qlie_SortRule_Armors",
	"$qlie_SortRule_Clothes",
	"$qlie_SortRule_Jewelry",
	"$qlie_SortRule_Weightless",
	"$qlie_SortRule_ByWeight",
	"$qlie_SortRule_ByValue",
	"$qlie_SortRule_ByV/W",
	"$qlie_SortRule_ByName",
	"$qlie_SortRule_ArtifactNeeded",
	"$qlie_SortRule_CompletionistNeeded",
};

constexpr std::array SortingPresets_Goblin = {
	"$qlie_SortRule_Gold",

	"$qlie_SortRule_CompletionistNeeded",
	"$qlie_SortRule_ArtifactNeeded",

	"$qlie_SortRule_Jewelry",
	"$qlie_SortRule_Gems",
	"$qlie_SortRule_SoulGems",

	"$qlie_SortRule_Weightless",

	"$qlie_SortRule_ByName",
};
