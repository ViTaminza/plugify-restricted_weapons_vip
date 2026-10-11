{
    global:
        Plugify_*;

        /* s2sdk binding slots (defined by s2sdk_bindings, filled by the plugify module) */
        __s2sdk_*;

        /* RestrictedWeapons binding slots (defined by the generated consumer bindings, filled by
           the plugify module from the RestrictedWeapons plugin's exported methods) */
        __RestrictedWeapons_*;

        /* vip binding slots (defined by the generated vip.cpp, filled by the plugify module from
           the vip plugin's exported methods) */
        __vip_*;

        /* hudmenu binding slots (defined by hudmenu_bindings, filled by the plugify module from
           the hudmenu plugin's exported GetClientMenuType/SetClientMenuType) */
        __hudmenu_*;

        /* No exported methods: restricted_weapons_vip exposes nothing to other plugins. */

    local: *;
};
