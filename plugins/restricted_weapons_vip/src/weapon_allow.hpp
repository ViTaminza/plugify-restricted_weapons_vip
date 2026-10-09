// restricted_weapons_vip — the engine-free part of the module: the guard and the weapon test of
// the original's OnWeaponRestricted.
//
// Kept in a header so tests/restricted_weapons_vip_tests.cpp can exercise the strstr semantics
// without the engine or the cores.

#pragma once

#include <cstring>

namespace restricted_weapons_vip {

// The guard of the original's OnWeaponRestricted:
//   if (iSlot < 0 || iSlot >= 64 || !szWeapon || !szWeapon[0] || !g_pVIPApi) return false;
// (the "!g_pVIPApi" part is the wrapper's "vip plugin not loaded" check in the port).
inline bool ShouldAskVip(int iSlot, const char* szWeapon) {
	return iSlot >= 0 && iSlot < 64 && szWeapon != nullptr && szWeapon[0] != '\0';
}

// The original's return line:
//   const char* szAllowed = g_pVIPApi->VIP_GetClientFeatureString(iSlot, "restricted_weapons");
//   return szAllowed && szAllowed[0] && strstr(szAllowed, szWeapon);
// szAllowed is the "restricted_weapons" value of the player's vip group - a semicolon-separated
// weapon list like "weapon_awp;weapon_ssg08;weapon_scar20" (production groups.ini). The original
// matched with plain strstr, i.e. any substring occurrence unblocks the weapon; kept verbatim.
inline bool IsWeaponAllowed(const char* szAllowed, const char* szWeapon) {
	return szAllowed != nullptr && szAllowed[0] != '\0' && szWeapon != nullptr &&
		   std::strstr(szAllowed, szWeapon) != nullptr;
}

} // namespace restricted_weapons_vip
