#pragma once

#include "ModifierKeyFrameworkAPI.h"

namespace QuickLoot::Integrations
{
	// Fork: Modifier Key Framework (MKF 1.9.0+, API 3). While the loot window shows, the game's Activate
	// press never reaches MKF, so the window lists MKF's modifier actions for the target on its More bar
	// and runs them through MKF itself.
	class ModifierKeyFramework
	{
	public:
		ModifierKeyFramework() = delete;

		// nullptr without MKF, or with an MKF too old to list and run its actions. More is off then.
		static const MKF::API* Get()
		{
			static const MKF::API* api = [] {
				const auto module = GetModuleHandleW(L"ModifierKeyFramework.dll");
				const auto get = module ? reinterpret_cast<MKF::GetAPI>(GetProcAddress(module, "MKF_GetAPI")) : nullptr;
				const MKF::API* found = get ? get() : nullptr;
				if (!found) {
					logger::info("Modifier Key Framework not found: More is off (it needs MKF 1.9.0 or newer)");
					return static_cast<const MKF::API*>(nullptr);
				}
				if (found->version < 3) {
					logger::info("Modifier Key Framework API {} is older than 3: More is off (it needs MKF 1.9.0 or newer)", found->version);
					return static_cast<const MKF::API*>(nullptr);
				}
				logger::info("Modifier Key Framework API {}: its modifier actions go on the More bar", found->version);
				return found;
			}();
			return api;
		}
	};
}
