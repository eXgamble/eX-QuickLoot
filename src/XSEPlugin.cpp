#include <spdlog/sinks/basic_file_sink.h>

#include "LootMenu.h"
#include "LootMenuManager.h"
#include "SanityChecks.h"
#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/LockpickActivation.h"
#include "Behaviors/PerkOptions.h"
#include "Config/Papyrus.h"
#include "Config/SystemSettings.h"
#include "Config/UserSettings.h"
#include "Input/InputManager.h"
#include "Input/InputObserver.h"
#include "Integrations/APIServer.h"
#include "Integrations/Artifacts.h"
#include "Integrations/BetterThirdPersonSelection.h"
#include "Integrations/Completionist.h"
#include "Integrations/DismemberingFramework.h"
#include "Integrations/ModifierKeyFramework.h"
#include "Integrations/NPCsNamesDistributor.h"
#include "Observers/CameraStateObserver.h"
#include "Observers/CombatStateObserver.h"
#include "Observers/ContainerObserver.h"
#include "Observers/CrosshairRefObserver.h"
#include "Observers/GrabStateObserver.h"
#include "Observers/LifeStateObserver.h"
#include "Observers/LockChangedObserver.h"
#include "Observers/MenuObserver.h"
#include "Observers/SneakStateObserver.h"
#include "Util/Profiler.h"

// Fork: the debugHeadless consumer, so headless mode can be tested without another mod.
static void LogView(QuickLoot::API::ViewChangedEvent* e)
{
	const auto container = e->container.get();
	if (!container) {
		logger::info("[View] closed");
		return;
	}

	std::string buttons;
	for (const auto& button : e->buttons) {
		buttons += std::format("[{}] {}{}  ", button.buttonArtIndex, button.label.c_str(), button.stealing ? " (red)" : "");
	}

	std::string selected = "-";
	if (e->selectedIndex >= 0 && static_cast<uint32_t>(e->selectedIndex) < e->items.size()) {
		const auto entry = e->items[e->selectedIndex].entry;
		selected = entry ? entry->GetDisplayName() : "?";
	}

	logger::info("[View] {:08X} '{}' items {} selected {} ({}){} | {}", container->GetFormID(), e->title, e->items.size(),
		e->selectedIndex, selected, e->stealing ? " stealing" : "", buttons);
}

// Fork: More requires Modifier Key Framework 1.9.0+ (API 3). Without it the loot window still works
// (Take, Transfer), and the player is told once per game launch, after the first load or new game.
// The box waits until the load is really over: the loading screen has closed and no fade or other box is
// up. kPostLoadGame arrives while the main menu is still up; a box raised then is lost (measured 2026-11-08).
class MKFWarning final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	static MKFWarning* Get()
	{
		static MKFWarning singleton;
		return &singleton;
	}

	void Arm()
	{
		if (_armed || REL::Module::IsVR() || QuickLoot::Integrations::ModifierKeyFramework::Get()) {
			return;
		}
		_armed = true;
		RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(this);
	}

	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
	{
		if (!event || _shown) {
			return RE::BSEventNotifyControl::kContinue;
		}
		logger::debug("[MKF warning] {} {}", event->menuName.c_str(), event->opening ? "opened" : "closed");
		if (!event->opening) {
			if (event->menuName == RE::LoadingMenu::MENU_NAME) {
				_loadDone = true;
			}
			TryShow(event->menuName);
		}
		return RE::BSEventNotifyControl::kContinue;
	}

private:
	// closing: the menu whose close event this is (it may still count as open while its event is sent).
	void TryShow(const RE::BSFixedString& closing)
	{
		const auto ui = RE::UI::GetSingleton();
		if (_shown || !_loadDone || !ui) {
			return;
		}
		for (const auto& name : { RE::LoadingMenu::MENU_NAME, RE::FaderMenu::MENU_NAME, RE::MessageBoxMenu::MENU_NAME, RE::MainMenu::MENU_NAME }) {
			if (closing != name && ui->IsMenuOpen(name)) {
				return;
			}
		}
		_shown = true;
		logger::info("Modifier Key Framework missing: telling the player More is off");
		SKSE::GetTaskInterface()->AddTask([] {
			RE::UI::GetSingleton()->RemoveEventSink<RE::MenuOpenCloseEvent>(Get());
			RE::DebugMessageBox("eX QuickLoot: the More button needs Modifier Key Framework 1.9.0 or newer. "
								"Looting still works, but More is off until it's installed.");
		});
	}

	bool _armed = false;
	bool _loadDone = false;
	bool _shown = false;
};

static void WarnIfNoMKF()
{
	MKFWarning::Get()->Arm();
}

static void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		WarnIfNoMKF();
		break;

	case SKSE::MessagingInterface::kDataLoaded:
		{
			PROFILE_SCOPE_NAMED("SKSE Message (kDataLoaded)");
			logger::info("--------------------------------[ kDataLoaded start ]--------------------------------");

			QuickLoot::Config::SystemSettings::Update();

			// Perk options don't need the loot window, so they work even if its swf fails.
			QuickLoot::Behaviors::PerkOptions::Install();

			if (!QuickLoot::SanityChecks::ValidateEsp()) {
				logger::error("Sanity checks failed. Disabling QuickLootIE.");
				return;
			}

			// Fork: without a working swf (it needs SkyUI) only the Flash window is off. Headless mode,
			// where another mod draws the window through the API, still works.
			if (QuickLoot::SanityChecks::ValidateSwf()) {
				QuickLoot::LootMenu::Register();
			} else {
				QuickLoot::LootMenuManager::SetFlashAvailable(false);
				logger::warn("LootMenuIE.swf can't load: QuickLoot draws no window, only headless mode (API) works");
			}

			QuickLoot::Config::Papyrus::Install();

			QuickLoot::Input::InputManager::Install();
			QuickLoot::Input::InputObserver::Install();

			QuickLoot::Observers::CameraStateObserver::Install();
			QuickLoot::Observers::CombatStateObserver::Install();
			QuickLoot::Observers::ContainerObserver::Install();
			QuickLoot::Observers::CrosshairRefObserver::Install();
			QuickLoot::Observers::GrabStateObserver::Install();
			QuickLoot::Observers::LifeStateObserver::Install();
			QuickLoot::Observers::LockChangedObserver::Install();
			QuickLoot::Observers::MenuObserver::Install();
			QuickLoot::Observers::SneakStateObserver::Install();

			QuickLoot::Behaviors::ActivationPrompt::Install();
			QuickLoot::Behaviors::LockpickActivation::Install();

			QuickLoot::Integrations::Artifacts::Init();
			QuickLoot::Integrations::BetterThirdPersonSelection::Init();
			QuickLoot::Integrations::Completionist::Init();
			QuickLoot::Integrations::DismemberingFramework::Init();
			QuickLoot::Integrations::NPCsNamesDistributor::Init();

			// Fork: settings and key bindings from QuickLootIE.ini right away, so the window works without
			// SkyUI. When SkyUI's settings menu registers (each load), its values replace these.
			QuickLoot::Config::UserSettings::Update();

			if (QuickLoot::Config::SystemSettings::DebugHeadless()) {
				constexpr auto debugPlugin = "QuickLootIE debugHeadless";
				const auto api = QuickLoot::API::APIServer::GetInterfaceEX1();
				api->SetHeadless(debugPlugin, true);
				api->RegisterViewChangedHandler(debugPlugin, LogView);
			}

			logger::info("--------------------------------[ kDataLoaded end ]--------------------------------");
			break;
		}

	default:
		break;
	}
}

static void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();

	*path /= std::format("{}.log"sv, plugin->GetName());
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S.%e] [%l] [%t] %v");
}

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLog();

	const auto plugin = SKSE::PluginDeclaration::GetSingleton();

	logger::info("Starting {} {} on Skyrim {}", plugin->GetName(), plugin->GetVersion().string("."), REL::Module::get().version().string("."));

	QuickLoot::Config::SystemSettings::Update(true);
	QuickLoot::Util::Profiler::Init();

	PROFILE_SCOPE_NAMED("Plugin Startup");

	SKSE::Init(skse, {
		.log = false,
		.trampoline = true,
		.trampolineSize = 1 << 8,
	});

	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}

#ifdef IS_CMAKE_BUILD

// xmake automatically generates this boilerplate code, but for cmake builds we need to include it manually.

#	include "Plugin.h"

extern "C" [[maybe_unused]] __declspec(dllexport) constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v{};
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	return v;
}();

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Query(SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

#endif
