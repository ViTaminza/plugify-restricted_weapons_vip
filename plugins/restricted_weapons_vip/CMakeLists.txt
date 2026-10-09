# restricted_weapons_vip — Plugify C++ plugin: port of the Pisex RestrictedWeapons module
# "[Restricted Weapons] VIP".
#
#   plugify-restricted_weapons_vip  the plugin shared library (entry "bin/restricted_weapons_vip")
#
# Consumer of both the RestrictedWeapons API (rw_api.hpp, HookOnWeaponRestricted) and the vip
# API (vip_api.hpp, VIP_RegisterFeature / VIP_GetClientFeatureString). Compiles the generated
# binding globals of plugins/RestrictedWeapons (__RestrictedWeapons_*) and plugins/vip
# (__vip_*); the s2sdk/hudmenu ones come in through pisexcompat.

set(RESTRICTED_WEAPONS_VIP_VERSION "1.0.0")

# --- generated consumer bindings --------------------------------------------------
# rw_bindings (plugins/RestrictedWeapons) and vip_bindings (plugins/vip) exist when their
# directories configure first; GLOB order is not guaranteed, so a missing target is replaced by
# a private OBJECT library over the same checked-in generated .cpp (the lr_vip_multi pattern).
if(TARGET rw_bindings)
    set(RW_VIP_RW_BINDING_OBJECTS "$<TARGET_OBJECTS:rw_bindings>")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/plugins/RestrictedWeapons/include/external/plugify/include/RestrictedWeapons/RestrictedWeapons.cpp")
    add_library(rw_bindings_restricted_weapons_vip OBJECT
            "${CMAKE_SOURCE_DIR}/plugins/RestrictedWeapons/include/external/plugify/include/RestrictedWeapons/RestrictedWeapons.cpp")
    target_include_directories(rw_bindings_restricted_weapons_vip SYSTEM PUBLIC
            "${CMAKE_SOURCE_DIR}/plugins/RestrictedWeapons/include/external/plugify/include"
            "${CMAKE_SOURCE_DIR}/sdk/include")
    target_compile_definitions(rw_bindings_restricted_weapons_vip PUBLIC _GLIBCXX_USE_CXX11_ABI=1)
    set(RW_VIP_RW_BINDING_OBJECTS "$<TARGET_OBJECTS:rw_bindings_restricted_weapons_vip>")
else()
    message(FATAL_ERROR "restricted_weapons_vip needs the generated RestrictedWeapons consumer bindings (plugins/RestrictedWeapons/include/external/plugify)")
endif()

if(TARGET vip_bindings)
    set(RW_VIP_VIP_BINDING_OBJECTS "$<TARGET_OBJECTS:vip_bindings>")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/plugins/vip/include/external/plugify/include/vip/vip.cpp")
    add_library(vip_bindings_restricted_weapons_vip OBJECT
            "${CMAKE_SOURCE_DIR}/plugins/vip/include/external/plugify/include/vip/vip.cpp")
    target_include_directories(vip_bindings_restricted_weapons_vip SYSTEM PUBLIC
            "${CMAKE_SOURCE_DIR}/plugins/vip/include/external/plugify/include"
            "${CMAKE_SOURCE_DIR}/sdk/include")
    target_compile_definitions(vip_bindings_restricted_weapons_vip PUBLIC _GLIBCXX_USE_CXX11_ABI=1)
    set(RW_VIP_VIP_BINDING_OBJECTS "$<TARGET_OBJECTS:vip_bindings_restricted_weapons_vip>")
else()
    message(FATAL_ERROR "restricted_weapons_vip needs the generated vip consumer bindings (plugins/vip/include/external/plugify)")
endif()

# --- plugin -------------------------------------------------------------------
add_library(plugify-restricted_weapons_vip SHARED
        src/plugin.cpp
        $<TARGET_OBJECTS:s2sdk_bindings>
        ${RW_VIP_RW_BINDING_OBJECTS}
        ${RW_VIP_VIP_BINDING_OBJECTS}
)
target_include_directories(plugify-restricted_weapons_vip PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
# SYSTEM: warnings from the generated RestrictedWeapons/vip/s2sdk/plg headers and the wrappers
# must not fail -Werror on our sources (src/plugin.cpp stays warning-clean).
target_include_directories(plugify-restricted_weapons_vip SYSTEM PRIVATE
        "${CMAKE_SOURCE_DIR}/plugins/RestrictedWeapons/include"
        "${CMAKE_SOURCE_DIR}/plugins/RestrictedWeapons/include/external/plugify/include"
        "${CMAKE_SOURCE_DIR}/plugins/vip/include"
        "${CMAKE_SOURCE_DIR}/plugins/vip/include/external/plugify/include"
        "${CMAKE_SOURCE_DIR}/sdk/include"
        "${CMAKE_SOURCE_DIR}/libs/pisexcompat/include")
target_link_libraries(plugify-restricted_weapons_vip PRIVATE
        pisexcompat
        -static-libstdc++ -static-libgcc
)
target_compile_options(plugify-restricted_weapons_vip PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(plugify-restricted_weapons_vip PRIVATE
        _GLIBCXX_USE_CXX11_ABI=1
        PLUGIFY_MODULE="restricted_weapons_vip"
        PLUGIFY_IS_DEBUG=0
        PLUGIFY_IS_RELEASE=1
)
set_target_properties(plugify-restricted_weapons_vip PROPERTIES OUTPUT_NAME restricted_weapons_vip)

if(UNIX AND NOT APPLE)
    target_link_options(plugify-restricted_weapons_vip PRIVATE
            "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/restricted_weapons_vip.pplugin.in"
        "${CMAKE_CURRENT_BINARY_DIR}/restricted_weapons_vip.pplugin"
)

# --- tests --------------------------------------------------------------------
# Engine-free: the strstr weapon test and the slot guard of OnWeaponRestricted (header-only,
# no pisexcompat needed).
add_executable(restricted_weapons_vip_tests tests/restricted_weapons_vip_tests.cpp)
target_include_directories(restricted_weapons_vip_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_options(restricted_weapons_vip_tests PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(restricted_weapons_vip_tests PRIVATE _GLIBCXX_USE_CXX11_ABI=1)
target_link_libraries(restricted_weapons_vip_tests PRIVATE -static-libstdc++ -static-libgcc)
set_target_properties(restricted_weapons_vip_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
