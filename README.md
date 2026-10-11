# restricted_weapons_vip (Plugify)

VIP module of the RestrictedWeapons core (port of the Pisex module '[Restricted Weapons] VIP'): registers the vip feature 'restricted_weapons' (VIP_BOOL, TOGGLABLE) whose per-group value is a semicolon-separated weapon list (weapon_awp;weapon_ssg08;...) and hooks the core's OnWeaponRestricted, unblocking a restricted weapon when its classname occurs in the player's list. Requires the RestrictedWeapons and vip cores.

Plugify port (fork) of **[Restricted Weapons] VIP (VIP_Module)** by Pisex (Metamod (C++)).
Original: https://github.com/Pisex/cs2-restricted_weapons
The code was rewritten for Plugify / s2sdk; behaviour follows the original.

## Requirements
- [Plugify](https://github.com/untrustedmodders/plugify) with [s2sdk](https://github.com/untrustedmodders/plugify-plugin-s2sdk)
- Plugify dependencies: s2sdk, vip, RestrictedWeapons, hudmenu (optional)

## Installation
Download `restricted_weapons_vip-1.0.0.zip` from Releases and extract it into `game/csgo/`.

## Building
Requires Docker. `./build.sh` builds in the Steam Runtime 3 (sniper) SDK image; the package
is written to `build/package/restricted_weapons_vip`.

`plugins/RestrictedWeapons`, `plugins/vip` carry only the API headers of the plugins this one uses.

## License
GPL-3.0, see LICENSE.

