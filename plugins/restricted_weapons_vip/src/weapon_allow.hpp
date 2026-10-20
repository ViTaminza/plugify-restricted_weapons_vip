// restricted_weapons_vip — the engine-free part of the module: the guard and the weapon test of
// the original's OnWeaponRestricted.
//
// Kept in a header so tests/restricted_weapons_vip_tests.cpp can exercise the strstr semantics
// without the engine or the cores.

#pragma once

#include <cstring>

namespace restricted_weapons_vip {

// The guard of the original's OnWeaponRestricted: a slot outside 0..63, a null or empty weapon
// string, or a missing vip API all mean "do not ask" and return false (the missing vip API is the
// wrapper's "vip plugin not loaded" check in the port).
inline bool ShouldAskVip(int iSlot, const char* szWeapon) {
	return iSlot >= 0 && iSlot < 64 && szWeapon != nullptr && szWeapon[0] != '\0';
}

// The original asked the vip core for the player's "restricted_weapons" value and returned true
// when the weapon classname occurred anywhere in it. That value is a semicolon-separated
// weapon list like "weapon_awp;weapon_ssg08;weapon_scar20" (the vip core's groups.ini). The original
// matched with plain strstr, i.e. any substring occurrence unblocks the weapon; kept verbatim.
inline bool IsWeaponAllowed(const char* szAllowed, const char* szWeapon) {
	return szAllowed != nullptr && szAllowed[0] != '\0' && szWeapon != nullptr &&
		   std::strstr(szAllowed, szWeapon) != nullptr;
}

} // namespace restricted_weapons_vip
