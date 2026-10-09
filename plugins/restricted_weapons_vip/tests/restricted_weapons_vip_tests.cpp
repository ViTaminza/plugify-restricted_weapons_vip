// restricted_weapons_vip unit tests — the engine-free part: the slot/weapon guard and the
// strstr weapon test of OnWeaponRestricted, with the production groups.ini value as the sample
// list.
//
// Run by ../../test.sh (inside the Steam Runtime image, cwd = build/tests).

#include <cstdio>

#include "weapon_allow.hpp"

namespace {

int g_checks = 0;
int g_failures = 0;

void Check(bool actual, bool expected, const char* what) {
	++g_checks;
	if (actual != expected) {
		++g_failures;
		std::printf("FAIL: %s (expected %d, got %d)\n", what, expected ? 1 : 0, actual ? 1 : 0);
	}
}

// The pair of calls OnWeaponRestricted makes: guard first, then the strstr test.
bool Allowed(int iSlot, const char* szWeapon, const char* szAllowed) {
	if (!restricted_weapons_vip::ShouldAskVip(iSlot, szWeapon)) {
		return false;
	}
	return restricted_weapons_vip::IsWeaponAllowed(szAllowed, szWeapon);
}

} // namespace

int main() {
	std::printf("restricted_weapons_vip tests\n");

	// The value this module's feature carries in the production groups.ini
	// (addons/configs/vip/groups.ini: "restricted_weapons" "weapon_awp;weapon_ssg08;weapon_scar20").
	constexpr const char* kList = "weapon_awp;weapon_ssg08;weapon_scar20";

	// Weapons of the list are unblocked...
	Check(Allowed(3, "weapon_awp", kList), true, "awp in list");
	Check(Allowed(3, "weapon_ssg08", kList), true, "ssg08 in list");
	Check(Allowed(3, "weapon_scar20", kList), true, "scar20 in list");
	// ...and one that is not (the core still blocks it) stays blocked.
	Check(Allowed(3, "weapon_ak47", kList), false, "ak47 not in list");
	Check(Allowed(3, "weapon_m4a1", kList), false, "m4a1 not in list");

	// strstr semantics, kept verbatim from the original: any substring matches, so a weapon name
	// that is a fragment of a listed one is unblocked too (the classnames the core passes are
	// full names, so this cannot happen in practice).
	Check(Allowed(3, "weapon_aw", kList), true, "substring matches (parity)");
	Check(Allowed(3, "weapon_", kList), true, "prefix matches (parity)");

	// A player without VIP, or without the feature configured in his group: empty list -> no
	// match.
	Check(Allowed(3, "weapon_awp", ""), false, "empty list");
	// A semicolon-separated list without spaces, exactly like the original's file format.
	Check(Allowed(3, "weapon_awp", "weapon_awp"), true, "single-entry list");

	// The guard: slots outside [0, 64) and empty/missing classnames are rejected before the list
	// is even queried (the original's first line, in the same order).
	Check(Allowed(-1, "weapon_awp", kList), false, "slot -1 (console)");
	Check(Allowed(64, "weapon_awp", kList), false, "slot 64");
	Check(Allowed(0, "", kList), false, "empty weapon");
	Check(Allowed(0, nullptr, kList), false, "null weapon");
	Check(Allowed(63, "weapon_awp", kList), true, "last valid slot");
	Check(Allowed(0, "weapon_awp", kList), true, "first valid slot");
	Check(Allowed(0, "weapon_awp", nullptr), false, "null list");

	std::printf("%d checks, %d failures\n", g_checks, g_failures);
	return g_failures == 0 ? 0 : 1;
}
