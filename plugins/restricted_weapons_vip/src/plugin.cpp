// restricted_weapons_vip — Plugify port of the Pisex RestrictedWeapons module
// "[Restricted Weapons] VIP"
// (ref/src/cs2-restricted_weapons/VIP_Module/restricted_weapons_vip.cpp, 153 lines;
// addons/RestrictedWeapons/restricted_weapons_vip.so on the production server).
//
// Behaviour kept identical:
//   * registers the vip feature "restricted_weapons" (VIP_BOOL, TOGGLABLE, toggle callback
//     OnToggle) - the per-group value is the semicolon-separated list of weapons the group may
//     use despite the core's restriction ("weapon_awp;weapon_ssg08;weapon_scar20" in the
//     production groups.ini);
//   * hooks the RestrictedWeapons core (HookOnWeaponRestricted): when a restriction would block
//     the weapon a player is about to pick up, the module answers true ("let it through") when
//     the weapon's classname occurs in the player's "restricted_weapons" value - the original's
//     plain strstr over VIP_GetClientFeatureString. The core ORs the answers of all modules and
//     applies its own unblock_type rule; for a non-VIP player the value is empty, so there is
//     nothing to match;
//   * VIP_OnClientLoaded keeps the original's callback registered (see port notes).
//
// Port notes (differences to the Metamod original):
//   * the original's `const char* szWeapons[64]` was written by OnToggle and OnClientLoaded but
//     read nowhere (the restriction test re-queried the vip core every time), and it stored
//     pointers obtained from the core's shared string buffer; across the plugify boundary such a
//     pointer has no defined lifetime, so the port drops the array and the stores. Observable
//     behaviour is unchanged: neither callback did anything but return a value (OnToggle's
//     return false = "do not veto the toggle", which the port keeps);
//   * the interfaces become the consumer wrappers rw_api.hpp / vip_api.hpp; the manifest
//     declares both cores as dependencies, so they are loaded before this plugin (the original
//     fetched them with MetaFactory in AllPluginsLoaded and unloaded itself when one was
//     missing);
//   * no config and no phrases of its own: the feature value lives in the vip core's groups.ini
//     (menu label in vip.phrases.txt), the restriction limits in the RestrictedWeapons.ini of
//     the core.

#include <pisex/pisex.hpp>

#include <plg/plugin.hpp>
#include <plugin_export.h>

#include <rw_api.hpp>
#include <vip_api.hpp>

#include <chrono>
#include <exception>
#include <string>
#include <string_view>

#include "weapon_allow.hpp"

namespace {

// The original's unqualified enum spellings (VIP_BOOL, TOGGLABLE, ...).
using namespace vip;

// GetLogTag() of the original.
constexpr const char* kLogTag = "restricted_weapons_vip";

// VIP_RegisterFeature("restricted_weapons", VIP_BOOL, TOGGLABLE, nullptr, OnToggle) of the
// original.
constexpr const char* kFeature = "restricted_weapons";

// The original's g_pRWApi / g_pVIPApi.
restricted_weapons::RWApi* g_pRWApi = &restricted_weapons::RWApi::Get();
vip::IVIPApi* g_pVIPApi = &vip::IVIPApi::Get();

void Log(std::string_view message, plg::Severity severity) {
	if (plg::Log) {
		plg::Log(std::string("[") + kLogTag + "] " + std::string(message), severity,
				 plg::source_location::current());
	}
}

// OnWeaponRestricted() of the original, including its comment: VIP_GetClientFeatureString
// returns a pointer into the core's shared buffer that the next call overwrites, so the list is
// read here and used immediately.
bool OnWeaponRestricted(int iSlot, const char* szWeapon) {
	if (!restricted_weapons_vip::ShouldAskVip(iSlot, szWeapon)) {
		return false;
	}
	// The wrapper answers "" when the vip plugin is not loaded (the original returned early on
	// the null interface).
	const char* szAllowed = g_pVIPApi->VIP_GetClientFeatureString(iSlot, kFeature);
	const bool bAllowed = restricted_weapons_vip::IsWeaponAllowed(szAllowed, szWeapon);
	// TEMPORARY diagnostics (capped): what the VIP module saw for this buy.
	static int s_diagBudget = 40;
	if (s_diagBudget > 0) {
		--s_diagBudget;
		Log("[vip-diag] slot " + std::to_string(iSlot) + " " + szWeapon + " feature='" +
				(szAllowed != nullptr ? szAllowed : "") + "' allowed=" + (bAllowed ? "1" : "0"),
			plg::Severity::Info);
	}
	return bAllowed;
}

// OnToggle() of the original (the Item_togglable_callback of VIP_RegisterFeature). The original
// stored the feature string in the write-only szWeapons array and returned false; the store is
// dropped (see the file header), the return value - "never veto a toggle" - is kept.
bool OnToggle(int /*iSlot*/, const char* /*szFeature*/, VIP_ToggleState /*eOldStatus*/,
			  VIP_ToggleState& /*eNewStatus*/) {
	return false;
}

// OnClientLoaded() of the original. The original only stored the feature string in the
// write-only szWeapons array; nothing observable is left to do.
void OnClientLoaded(int /*iSlot*/, bool /*bIsVIP*/) {
}

class RestrictedWeaponsVipPlugin final : public plg::Plugin {
public:
	plg::PluginResult OnPluginStart() override {
		try {
			// Never Failed: a plugin that fails OnPluginStart stops every Plugify plugin.
			pisex::Init("restricted_weapons_vip");

			// AllPluginsLoaded() of the original, in its order. Both cores are declared
			// dependencies, so they are loaded before this plugin.
			g_pRWApi->HookOnWeaponRestricted(OnWeaponRestricted);
			Log(std::string("RestrictedWeapons listener registered: ") +
					(__RestrictedWeapons_OnWeaponRestricted_Register != nullptr ? "yes" : "NO"),
				plg::Severity::Info);
			g_pVIPApi->VIP_RegisterFeature(kFeature, VIP_BOOL, TOGGLABLE, nullptr, OnToggle);
			g_pVIPApi->VIP_OnClientLoaded(OnClientLoaded);

			Log("ready", plg::Severity::Info);
		} catch (const std::exception& ex) {
			Log(std::string("OnPluginStart failed: ") + ex.what(), plg::Severity::Error);
		} catch (...) {
			Log("OnPluginStart failed with an unknown exception", plg::Severity::Error);
		}
		return {};
	}

	plg::PluginResult OnPluginUpdate(std::chrono::milliseconds) override {
		try {
			pisex::Poll();
		} catch (...) {
			// the compat layer never lets a poll failure escape; nothing else to do here
		}
		return {};
	}

	plg::PluginResult OnPluginEnd() override {
		try {
			// The original had no unload paths (listeners lived until the module unloaded);
			// these are the wrappers' plugify pairs.
			g_pRWApi->UnhookOnWeaponRestricted();
			// The feature's toggle callback is a trampoline inside this library: re-register the
			// feature with no callbacks so the vip core's entry holds nulls (its menu guards every
			// callback and treats a missing one as "toggles without a veto") and a menu toggle after
			// this unload cannot call into the unloaded code. The feature itself stays registered,
			// like the original's - the vip core exposes no feature unregister.
			g_pVIPApi->VIP_RegisterFeature(kFeature, VIP_BOOL, TOGGLABLE, nullptr, nullptr);
			g_pVIPApi->VIP_OnClientLoadedUnregister();
			pisex::Shutdown();
		} catch (...) {
			// nothing left to do at unload
		}
		return {};
	}
};

RestrictedWeaponsVipPlugin g_plugin;

} // namespace

PLUGIFY_PLUGIN(PLUGIN_API, &g_plugin)
