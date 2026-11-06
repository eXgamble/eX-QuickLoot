#include "LootMenuManager.h"

#include "Behaviors/ActivationPrompt.h"
#include "Behaviors/ContainerAnimator.h"
#include "Input/InputManager.h"
#include "Integrations/APIServer.h"
#include "Integrations/BetterThirdPersonSelection.h"
#include "Integrations/NPCsNamesDistributor.h"
#include "LootMenu.h"
#include "MenuVisibilityManager.h"

namespace QuickLoot
{
	bool LootMenuManager::IsShowing()
	{
		return static_cast<bool>(_currentContainer);
	}

	void LootMenuManager::RequestShow(const RE::ObjectRefHandle& container)
	{
		if (!container || container == _currentContainer) {
			return;
		}

		if (!_flashAvailable && !IsHeadless()) {
			return;  // no window can show: the swf didn't load and nobody draws one for us
		}

		if (API::APIServer::DispatchOpeningLootMenuEvent(container) != API::HandleResult::kContinue) {
			logger::info("Opening was canceled by API subscriber");
			RequestHide();
			return;
		}

		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		if (!RE::PlayerCamera::GetSingleton()->IsInThirdPerson() ||
			!Integrations::BetterThirdPersonSelection::Is3DWidgetEnabled()) {
			Behaviors::ActivationPrompt::Block();
		}

		if (const auto actor = skyrim_cast<RE::Actor*>(container.get().get())) {
			Integrations::NPCsNamesDistributor::RevealName(actor);
		}

		_currentContainer = container;
		const auto index = container == _lastContainer ? _lastSelectedIndex : 0;

		// This may trigger another call to RequestShow, so make sure it happens after _currentContainer is set.
		Input::InputManager::BlockConflictingInputs();

		EnsureOpen();

		QueueLootMenuTask([=](LootMenu& menu) {
			menu.Show(container, index);
		});
	}

	void LootMenuManager::RequestHide()
	{
		if (!IsShowing()) {
			return;
		}

		Behaviors::ActivationPrompt::Unblock();
		Behaviors::ContainerAnimator::CloseContainer(_currentContainer);

		_currentContainer.reset();

		// This may trigger another call to RequestHide, so make sure it happens after _currentContainer is reset.
		Input::InputManager::UnblockConflictingInputs();

		{
			// kHide destroys the instance, so any pending messages would be run when the
			// next loot menu instance is created. Here, we discard them instead.
			std::scoped_lock lock{ _lock };
			_taskQueue.clear();
		}

		if (IsHeadless()) {
			// No menu to destroy: the headless window just closes (and tells the API it did).
			QueueLootMenuTask([](LootMenu& menu) {
				menu.Hide();
			});
			return;
		}

		RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	}

	bool LootMenuManager::IsHeadless()
	{
		return _headless;
	}

	void LootMenuManager::SetHeadless(const std::string& plugin, bool headless)
	{
		bool changed;
		{
			std::scoped_lock lock{ _lock };
			if (headless) {
				_headlessPlugins.insert(plugin);
			} else {
				_headlessPlugins.erase(plugin);
			}
			changed = _headless != !_headlessPlugins.empty();
		}

		logger::info("Plugin {} {} headless mode", plugin, headless ? "asked for" : "let go of");

		if (!changed) {
			return;
		}

		// Close whatever is open in the old mode first; the next container looked at opens in the new one.
		RequestHide();
		_headless = !_headless;
		logger::info("Headless mode {}", _headless ? "on: QuickLoot draws no window" : "off");
	}

	void LootMenuManager::RequestRefresh(RefreshFlags flags)
	{
		QueueLootMenuTask([=](LootMenu& menu) {
			menu.QueueRefresh(flags);
		});
	}

	void LootMenuManager::OnInputAction(Input::QuickLootAction action)
	{
		//logger::trace("Input action {}", static_cast<int>(action));

		const auto plugin = SKSE::PluginDeclaration::GetSingleton();

		switch (action) {
		case Input::QuickLootAction::kDisable:
			MenuVisibilityManager::DisableLootMenu(std::string(plugin->GetName()));
			break;

		case Input::QuickLootAction::kEnable:
			MenuVisibilityManager::EnableLootMenu(std::string(plugin->GetName()));
			break;

		default:
			QueueLootMenuTask([=](LootMenu& menu) {
				menu.OnInputAction(action);
			});
			break;
		}
	}

	void LootMenuManager::EnsureOpen()
	{
		if (IsHeadless()) {
			return;  // the headless window is made by its first task
		}

		if (!RE::UI::GetSingleton()->IsMenuOpen(LootMenu::MENU_NAME)) {
			RE::UIMessageQueue::GetSingleton()->AddMessage(LootMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
		}
	}

	void LootMenuManager::SaveLastSelectedIndex(const RE::ObjectRefHandle& container, int selectedIndex)
	{
		if (container.get()) {
			_lastContainer = container;
			_lastSelectedIndex = selectedIndex;
		}
	}

	void LootMenuManager::ProcessPendingTasks(LootMenu& menu)
	{
		std::vector<LootMenuTask> tasks;

		{
			// Swap out the queue so queueing new tasks within another task
			// cannot reallocate the vector mid-iteration.
			std::scoped_lock lock{ _lock };
			tasks.swap(_taskQueue);
		}

		for (auto& task : tasks) {
			task(menu);
		}
	}

	void LootMenuManager::QueueLootMenuTask(LootMenuTask task)
	{
		if (!task) {
			return;
		}

		{
			std::scoped_lock lock{ _lock };
			_taskQueue.push_back(std::move(task));
		}

		if (IsHeadless()) {
			ScheduleHeadlessPump();
		}
	}

	void LootMenuManager::ScheduleHeadlessPump()
	{
		// One SKSE task runs everything queued so far, then refreshes once. Event-driven: nothing
		// runs while nothing is queued. Tasks queued while it runs schedule the next one.
		if (_pumpQueued.exchange(true)) {
			return;
		}

		SKSE::GetTaskInterface()->AddTask([] {
			_pumpQueued = false;

			if (!_headlessMenu) {
				_headlessMenu = std::make_unique<LootMenu>(LootMenu::Headless{});
			}

			ProcessPendingTasks(*_headlessMenu);
			_headlessMenu->Refresh();
		});
	}
}
