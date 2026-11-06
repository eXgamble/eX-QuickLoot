#include "PerkOptions.h"

namespace QuickLoot::Behaviors
{
	namespace
	{
		class ChoiceVisitor final : public RE::PerkEntryVisitor
		{
		public:
			ChoiceVisitor(RE::Actor* a_owner, RE::TESObjectREFR* a_target) :
				_args{ a_owner, a_target } {}

			RE::BSContainer::ForEachResult Visit(RE::BGSPerkEntry* a_entry) override
			{
				if (!a_entry || a_entry->GetType() != RE::PERK_ENTRY_TYPE::kEntryPoint) {
					return RE::BSContainer::ForEachResult::kContinue;
				}

				const auto entry = static_cast<RE::BGSEntryPointPerkEntry*>(a_entry);
				const auto data = entry->functionData;
				if (!data || data->GetType() != RE::BGSEntryPointFunctionData::ENTRY_POINT_FUNCTION_DATA::kActivateChoice) {
					return RE::BSContainer::ForEachResult::kContinue;
				}

				// Entries that run on their own with no button (Dawnguard's vampire-sleep script
				// runs on every activation) aren't options.
				const auto choice = static_cast<RE::BGSEntryPointFunctionDataActivateChoice*>(data);
				if (choice->RunsImmediately() || choice->label.empty()) {
					return RE::BSContainer::ForEachResult::kContinue;
				}

				// Activate entries have one condition list per tab: 0 runs on the perk owner,
				// 1 on the target reference (verified in game: matches the popup's buttons).
				const auto& tabs = entry->conditions;
				for (std::uint32_t i = 0; i < tabs.size() && i < 2; ++i) {
					if (!tabs[i].IsTrue(_args[i], _args[1 - i])) {
						return RE::BSContainer::ForEachResult::kContinue;
					}
				}

				options.push_back({ choice->label.c_str(), entry->perk, choice->ReplacesDefault(), choice->RunsImmediately() });
				return RE::BSContainer::ForEachResult::kContinue;
			}

			std::vector<PerkOption> options;

		private:
			RE::TESObjectREFR* _args[2];
		};

		bool IsActivateChoiceBox(const RE::MessageBoxData* a_data)
		{
			if (!a_data || !a_data->callback) {
				return false;
			}
			static REL::Relocation<std::uintptr_t> vtbl{ RE::ActivateChoiceMenuCallbackFunctor::VTABLE[0] };
			return *reinterpret_cast<const std::uintptr_t*>(a_data->callback.get()) == vtbl.address();
		}

		void LogBox(const RE::MessageBoxData* a_data)
		{
			logger::info("[PerkOptions] popup '{}' choiceBox={} buttons={}", a_data->bodyText.c_str(), IsActivateChoiceBox(a_data), a_data->buttonText.size());
			for (std::uint32_t i = 0; i < a_data->buttonText.size(); ++i) {
				logger::info("[PerkOptions]   button {} '{}'", i, a_data->buttonText[i].c_str());
			}
		}

		// Activates the target as the player, then answers the choice popup it queued with the
		// button picked by a_pick (-1 leaves it). Returns false when no choice popup came.
		template <class Pick>
		bool ActivateAndAnswer(RE::TESObjectREFR* a_target, Pick a_pick)
		{
			const auto player = RE::PlayerCharacter::GetSingleton();
			if (!a_target || !player) {
				return false;
			}

			// The choice popup comes only from the player's own activation of the crosshair
			// ref; ActivateRef from code skips perk choices and opens the container directly.
			const auto crosshair = RE::CrosshairPickData::GetSingleton();
			const auto picked = crosshair ? crosshair->GetActiveTarget().get() : nullptr;
			if (picked.get() != a_target) {
				logger::warn("[PerkOptions] target {:08X} isn't the crosshair ref ({:08X})", a_target->GetFormID(), picked ? picked->GetFormID() : 0);
				return false;
			}

			const auto before = RE::MessageBoxMenu::GetCurrentMessageBoxData();
			player->ActivatePickRef();
			const auto data = RE::MessageBoxMenu::GetCurrentMessageBoxData();

			if (!data || data == before) {
				logger::info("[PerkOptions] activation queued no popup");
				return false;
			}

			LogBox(data);
			if (!IsActivateChoiceBox(data)) {
				logger::warn("[PerkOptions] newest popup isn't an activate choice; left alone");
				return false;
			}

			const int index = a_pick(*data);
			if (index < 0) {
				logger::warn("[PerkOptions] no matching button; popup left open");
				return false;
			}

			logger::info("[PerkOptions] answering with button {} (menu open now: {})", index, RE::UI::GetSingleton()->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME));
			RE::MessageBoxMenu::SelectOption(index);
			return true;
		}

		class MenuLogger final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
		{
		public:
			static MenuLogger* GetSingleton()
			{
				static MenuLogger singleton;
				return &singleton;
			}

			RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
			{
				if (a_event && a_event->menuName == RE::MessageBoxMenu::MENU_NAME) {
					logger::info("[PerkOptions] MessageBoxMenu {}", a_event->opening ? "OPENED" : "closed");
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		// --- test natives (eXQuickLootSpike) ---

		void ListOptions(RE::StaticFunctionTag*, RE::TESObjectREFR* a_target)
		{
			if (!a_target) {
				return;
			}
			const auto handle = a_target->CreateRefHandle();
			SKSE::GetTaskInterface()->AddTask([handle] {
				const auto target = handle.get();
				if (!target) {
					return;
				}
				const auto options = PerkOptions::List(target.get());
				logger::info("[PerkOptions] {} option(s) on {:08X} '{}'", options.size(), target->GetFormID(), target->GetDisplayFullName());
				for (std::size_t i = 0; i < options.size(); ++i) {
					const auto& o = options[i];
					logger::info("[PerkOptions]   {} '{}' perk {:08X} replacesDefault={} runsImmediately={}", i, o.label, o.perk ? o.perk->GetFormID() : 0, o.replacesDefault, o.runsImmediately);
				}
			});
		}

		void RunIndex(RE::StaticFunctionTag*, RE::TESObjectREFR* a_target, std::int32_t a_index)
		{
			if (!a_target) {
				return;
			}
			const auto handle = a_target->CreateRefHandle();
			SKSE::GetTaskInterface()->AddTask([handle, a_index] {
				const auto target = handle.get();
				if (!target) {
					return;
				}
				const auto options = PerkOptions::List(target.get());
				if (a_index < 0 || a_index >= static_cast<std::int32_t>(options.size())) {
					logger::warn("[PerkOptions] RunIndex {}: only {} option(s)", a_index, options.size());
					return;
				}
				logger::info("[PerkOptions] RunIndex {} '{}' -> {}", a_index, options[a_index].label, PerkOptions::Run(target.get(), options[a_index].label));
			});
		}

		void SearchTarget(RE::StaticFunctionTag*, RE::TESObjectREFR* a_target)
		{
			if (!a_target) {
				return;
			}
			const auto handle = a_target->CreateRefHandle();
			SKSE::GetTaskInterface()->AddTask([handle] {
				if (const auto target = handle.get()) {
					logger::info("[PerkOptions] Search -> {}", PerkOptions::Search(target.get()));
				}
			});
		}
	}

	void PerkOptions::Install()
	{
		if (const auto ui = RE::UI::GetSingleton()) {
			ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuLogger::GetSingleton());
		}
		SKSE::GetPapyrusInterface()->Register(RegisterFunctions);
		logger::info("Installed PerkOptions");
	}

	std::vector<PerkOption> PerkOptions::List(RE::TESObjectREFR* a_target)
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!a_target || !player) {
			return {};
		}
		ChoiceVisitor visitor(player, a_target);
		player->ForEachPerkEntry(RE::BGSEntryPoint::ENTRY_POINT::kActivate, visitor);
		return std::move(visitor.options);
	}

	bool PerkOptions::Run(RE::TESObjectREFR* a_target, std::string_view a_label)
	{
		return ActivateAndAnswer(a_target, [a_label](const RE::MessageBoxData& a_data) {
			for (std::uint32_t i = 0; i < a_data.buttonText.size(); ++i) {
				if (a_label == a_data.buttonText[i].c_str()) {
					return static_cast<int>(i);
				}
			}
			return -1;
		});
	}

	bool PerkOptions::Search(RE::TESObjectREFR* a_target)
	{
		const auto options = List(a_target);
		return ActivateAndAnswer(a_target, [&options](const RE::MessageBoxData& a_data) {
			// The default choice is the one button that isn't a perk option.
			for (std::uint32_t i = 0; i < a_data.buttonText.size(); ++i) {
				const std::string_view text = a_data.buttonText[i].c_str();
				const bool isOption = std::ranges::any_of(options, [&](const PerkOption& o) { return o.label == text; });
				if (!isOption) {
					return static_cast<int>(i);
				}
			}
			return -1;
		});
	}

	bool PerkOptions::RegisterFunctions(RE::BSScript::IVirtualMachine* a_vm)
	{
		constexpr auto script = "eXQuickLootSpike";
		a_vm->RegisterFunction("ListOptions", script, ListOptions);
		a_vm->RegisterFunction("RunIndex", script, RunIndex);
		a_vm->RegisterFunction("SearchTarget", script, SearchTarget);
		return true;
	}
}
