// restricted_weapons_vip unit tests — the engine-free, binding-free part of the module:
// the slot/weapon guard and the strstr weapon test of OnWeaponRestricted, with the groups.ini
// sample value as the list. No pisexcompat, no s2sdk, no plugify binding globals.
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

	// The value this module's feature carries for a vip group
	// (addons/configs/vip/groups.ini).
	constexpr const char* kList = "weapon_awp;weapon_ssg08;weapon_scar20";

	// Weapons of the list are unblocked...
	Check(Allowed(3, "weapon_awp", kList), true, "awp in list");
	Check(Allowed(3, "weapon_ssg08", kList), true, "ssg08 in list");
	Check(Allowed(3, "weapon_scar20", kList), true, "scar20 in list");
	// ...at any position of the list (only, middle, last entry).
	Check(Allowed(3, "weapon_awp", "weapon_awp"), true, "only entry of the list");
	Check(Allowed(3, "weapon_ssg08", "weapon_awp;weapon_ssg08;weapon_scar20"), true,
		  "middle entry of the list");
	Check(Allowed(3, "weapon_scar20", "weapon_awp;weapon_scar20"), true, "last entry of the list");
	// ...and one that is not (the core still blocks it) stays blocked.
	Check(Allowed(3, "weapon_ak47", kList), false, "ak47 not in list");
	Check(Allowed(3, "weapon_m4a1", kList), false, "m4a1 not in list");
	Check(Allowed(3, "weapon_awp", "weapon_ssg08;weapon_scar20"), false, "other weapons listed");

	// strstr semantics, kept verbatim from the original: any substring matches, so a weapon name
	// that is a fragment of a listed one is unblocked too (the classnames the core passes are
	// full names, so this cannot happen in practice).
	Check(Allowed(3, "weapon_aw", kList), true, "substring matches (parity)");
	Check(Allowed(3, "weapon_", kList), true, "prefix matches (parity)");
	Check(Allowed(3, "awp", kList), true, "bare name matches (parity)");
	Check(Allowed(3, "weapon_ssg", kList), true, "fragment of a listed name (parity)");
	// The match is case-sensitive (plain strstr, no lowering): the core passes lower-case
	// classnames and groups.ini is written in the same case.
	Check(Allowed(3, "Weapon_AWP", kList), false, "case-sensitive match");

	// A player without VIP, or without the feature configured in his group: empty list -> no
	// match.
	Check(Allowed(3, "weapon_awp", ""), false, "empty list");
	Check(Allowed(3, "weapon_awp", nullptr), false, "null list");
	// A semicolon-separated list without spaces, exactly like the original's file format. An
	// entry with a stray space still matches its weapon by strstr (the core never emits one).
	Check(Allowed(3, "weapon_awp", "weapon_awp"), true, "single-entry list");
	Check(Allowed(3, "weapon_ssg08", "weapon_awp; weapon_ssg08"), true, "space after semicolon");
	// Separators only: nothing to match.
	Check(Allowed(3, "weapon_awp", ";;;"), false, "separators only");
	// A list that only carries another group's weapons.
	Check(Allowed(3, "weapon_awp", "weapon_scar20;weapon_g3sg1"), false, "no matching entry");

	// The guard: slots outside [0, 64) and empty/missing classnames are rejected before the list
	// is even queried (the original's first line, in the same order).
	Check(Allowed(-1, "weapon_awp", kList), false, "slot -1 (console)");
	Check(Allowed(-2, "weapon_awp", kList), false, "slot -2");
	Check(Allowed(64, "weapon_awp", kList), false, "slot 64");
	Check(Allowed(65, "weapon_awp", kList), false, "slot 65");
	Check(Allowed(64, "weapon_awp", nullptr), false, "slot 64 rejects before the list is read");
	Check(Allowed(0, "", kList), false, "empty weapon");
	Check(Allowed(0, nullptr, kList), false, "null weapon");
	Check(Allowed(0, "", ""), false, "empty weapon and empty list");
	Check(Allowed(63, "weapon_awp", kList), true, "last valid slot");
	Check(Allowed(0, "weapon_awp", kList), true, "first valid slot");
	Check(Allowed(32, "weapon_awp", kList), true, "middle valid slot");

	// The two halves are independent: IsWeaponAllowed's strstr is non-empty-list only, while the
	// empty weapon is rejected by the guard - the exact split of the original.
	Check(restricted_weapons_vip::IsWeaponAllowed(kList, ""), true,
		  "empty needle matches by strstr (guard rejects it)");
	Check(restricted_weapons_vip::IsWeaponAllowed(nullptr, "weapon_awp"), false,
		  "null list direct");
	Check(restricted_weapons_vip::IsWeaponAllowed(kList, nullptr), false, "null weapon direct");
	Check(restricted_weapons_vip::ShouldAskVip(0, "weapon_awp"), true, "guard accepts slot 0");
	Check(restricted_weapons_vip::ShouldAskVip(63, "weapon_awp"), true, "guard accepts slot 63");
	Check(restricted_weapons_vip::ShouldAskVip(64, "weapon_awp"), false, "guard rejects slot 64");
	Check(restricted_weapons_vip::ShouldAskVip(-1, "weapon_awp"), false, "guard rejects slot -1");
	Check(restricted_weapons_vip::ShouldAskVip(0, nullptr), false, "guard rejects null weapon");
	Check(restricted_weapons_vip::ShouldAskVip(0, ""), false, "guard rejects empty weapon");

	std::printf("%d checks, %d failures\n", g_checks, g_failures);
	return g_failures == 0 ? 0 : 1;
}
