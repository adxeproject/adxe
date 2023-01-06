option(AX_ENABLE_MSEDGE_WEBVIEW2 "Enable msedge webview2" TRUE)

if(WINDOWS)
    if(NOT ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32"))
        set(WIN64 TRUE)
        set(ARCH_ALIAS "x64")
    else()
        set(WIN32 TRUE)
        set(ARCH_ALIAS "x86")
    endif()
    set(OS "windows")
else()
    set(ARCH_ALIAS "x64")
    set(OS "linux")
endif()

if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
    set(BUILD_CONFIG_DIR "\$\(Configuration\)/")
else()
    set(BUILD_CONFIG_DIR "")
endif()

message(STATUS "AX_ENABLE_MSEDGE_WEBVIEW2=${AX_ENABLE_MSEDGE_WEBVIEW2}")

function(ax_link_cxx_prebuilt APP_NAME AX_ROOT_DIR AX_PREBUILT_DIR)
    if (NOT AX_USE_SHARED_PREBUILT)
        target_compile_definitions(${APP_NAME}
            PRIVATE AX_STATIC=1
        )
    endif()

    target_include_directories(${APP_NAME}
        PRIVATE ${AX_ROOT_DIR}/thirdparty/lua
        PRIVATE ${AX_ROOT_DIR}/extensions/scripting/lua-bindings/manual
        PRIVATE ${AX_ROOT_DIR}
        PRIVATE ${AX_ROOT_DIR}/thirdparty
        PRIVATE ${AX_ROOT_DIR}/extensions
        PRIVATE ${AX_ROOT_DIR}/core
        PRIVATE ${AX_ROOT_DIR}/core/platform
        PRIVATE ${AX_ROOT_DIR}/core/base
        PRIVATE ${AX_ROOT_DIR}/core/audio
        PRIVATE ${AX_ROOT_DIR}/core/platform/win32
        PRIVATE ${AX_ROOT_DIR}/thirdparty/fmt/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/robin-map/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/freetype/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/glfw/include/GLFW
        PRIVATE ${AX_ROOT_DIR}/thirdparty/box2d/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/chipmunk/include
        PRIVATE ${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/engine/thirdparty/freetype/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/webp/src/webp
        PRIVATE ${AX_ROOT_DIR}/thirdparty/pugixml
        PRIVATE ${AX_ROOT_DIR}/thirdparty/xxhash
        PRIVATE ${AX_ROOT_DIR}/thirdparty/ConvertUTF
        PRIVATE ${AX_ROOT_DIR}/thirdparty/openal/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/ogg/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/glad/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/glfw/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/png
        PRIVATE ${AX_ROOT_DIR}/thirdparty/unzip/.
        PRIVATE ${AX_ROOT_DIR}/thirdparty/llhttp/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/lua/plainlua
        PRIVATE ${AX_ROOT_DIR}/thirdparty/lua/tolua/.
        PRIVATE ${AX_ROOT_DIR}/thirdparty/lua/lua-cjson/.
        PRIVATE ${AX_ROOT_DIR}/extensions/cocostudio
        PRIVATE ${AX_ROOT_DIR}/extensions/spine/runtime/include
        PRIVATE ${AX_ROOT_DIR}/extensions/fairygui
        PRIVATE ${AX_ROOT_DIR}/extensions/GUI
        PRIVATE ${AX_ROOT_DIR}/thirdparty/zlib/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/jpeg-turbo/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/openssl/include
        PRIVATE ${AX_ROOT_DIR}/thirdparty/curl/include
    )

    SET (CONFIGURATION_SUBFOLDER "")
    target_link_directories(${APP_NAME}
        PRIVATE ${AX_ROOT_DIR}/thirdparty/openssl/prebuilt/${OS}/${ARCH_ALIAS}
        PRIVATE ${AX_ROOT_DIR}/thirdparty/zlib/prebuilt/${OS}/${ARCH_ALIAS}
        PRIVATE ${AX_ROOT_DIR}/thirdparty/jpeg-turbo/prebuilt/${OS}/${ARCH_ALIAS}
        PRIVATE ${AX_ROOT_DIR}/thirdparty/curl/prebuilt/${OS}/${ARCH_ALIAS}
        PRIVATE ${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/lib  # cmake will auto add suffix '/$(Configuration)', refer to https://github.com/Kitware/CMake/blob/master/Source/cmVisualStudio10TargetGenerator.cxx#L4145
    )

    # Linking OS libs
    if (WINDOWS)
        target_link_libraries(${APP_NAME} winmm Version)
    else()
        target_link_libraries(${APP_NAME} X11 fontconfig glib-2.0 gtk-3 gobject-2.0)
    endif()

    # Linking engine and thirdparty libs
    set(LIBS
        spine
        particle3d
        assets-manager
        cocostudio
        DragonBones
        axmol
        box2d
        chipmunk
        freetype
        recast
        bullet
        webp
        pugixml
        xxhash
        lz4
        clipper2
        ConvertUTF
        poly2tri
        astc
        ogg
        glad
        glfw
        png
        unzip
        llhttp
        physics-nodes
    )
    if (WINDOWS)
        target_link_libraries(${APP_NAME}
            ${LIBS}
            zlib
            jpeg-static
            libcrypto
            libssl
            libcurl_imp
            OpenAL32
        )
    else()
        target_link_libraries(${APP_NAME}
            ${LIBS}
            z
            jpeg
            curl
            ssl
            crypto
            openal
        )
    endif()

    # Copy dlls to app bin dir
        # copy thirdparty dlls to target bin dir
    # copy_thirdparty_dlls(${APP_NAME} $<TARGET_FILE_DIR:${APP_NAME}>)
    if(WINDOWS)
        set(ssl_dll_suffix "")
        if(WIN64)
            set(ssl_dll_suffix "-${ARCH_ALIAS}")
        endif()
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AX_ROOT_DIR}/thirdparty/openssl/prebuilt/windows/${ARCH_ALIAS}/libcrypto-3${ssl_dll_suffix}.dll"
            "${AX_ROOT_DIR}/thirdparty/openssl/prebuilt/windows/${ARCH_ALIAS}/libssl-3${ssl_dll_suffix}.dll"
            "${AX_ROOT_DIR}/thirdparty/curl/prebuilt/windows/${ARCH_ALIAS}/libcurl.dll"
            "${AX_ROOT_DIR}/thirdparty/zlib/prebuilt/windows/${ARCH_ALIAS}/zlib1.dll"
            "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}OpenAL32.dll"
            $<TARGET_FILE_DIR:${APP_NAME}>)

        if (BUILD_SHARED_LIBS)
            add_custom_command(TARGET ${ax_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}glad.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}glfw.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}pugixml.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}freetype.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}axmol.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}cocostudio.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}GUI.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}particle3d.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}physics-nodes.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}spine.dll"
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}assets-manager.dll"
                $<TARGET_FILE_DIR:${APP_NAME}>)
        endif()

        # Copy windows angle binaries
        if (AX_USE_COMPAT_GL)
            add_custom_command(TARGET ${APP_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${AX_ROOT_DIR}/thirdparty/angle/prebuilt/windows/${ARCH_ALIAS}/libGLESv2.dll
                ${AX_ROOT_DIR}/thirdparty/angle/prebuilt/windows/${ARCH_ALIAS}/libEGL.dll
                ${AX_ROOT_DIR}/thirdparty/angle/prebuilt/windows/${ARCH_ALIAS}/d3dcompiler_47.dll
                $<TARGET_FILE_DIR:${APP_NAME}>
            )
        endif()

        if (AX_ENABLE_MSEDGE_WEBVIEW2)
            if(CMAKE_GENERATOR STREQUAL "Ninja")
                target_link_libraries(${APP_NAME} ${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/packages/Microsoft.Web.WebView2/build/native/${ARCH_ALIAS}/WebView2Loader.dll.lib)
                target_include_directories(${APP_NAME} PRIVATE ${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/packages/Microsoft.Web.WebView2/build/native/include)
                add_custom_command(TARGET ${APP_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/packages/Microsoft.Web.WebView2/build/native/${ARCH_ALIAS}/WebView2Loader.dll"
                    $<TARGET_FILE_DIR:${APP_NAME}>
                )
            else()
                target_link_libraries(${APP_NAME} ${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/packages/Microsoft.Web.WebView2/build/native/Microsoft.Web.WebView2.targets)
            endif()
        endif()
    endif()
endfunction(ax_link_cxx_prebuilt)

function(ax_link_lua_prebuilt APP_NAME AX_ROOT_DIR AX_PREBUILT_DIR)
    if (NOT AX_USE_SHARED_PREBUILT)
        target_compile_definitions(${APP_NAME}
	        PRIVATE _USRLUASTATIC=1
        )
    endif()
    target_link_libraries(${APP_NAME} axlua lua-cjson tolua plainlua)

    ax_link_cxx_prebuilt(${APP_NAME} ${AX_ROOT_DIR} ${AX_PREBUILT_DIR})

    if (WINDOWS)
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${AX_ROOT_DIR}/${AX_PREBUILT_DIR}/bin/${BUILD_CONFIG_DIR}plainlua.dll"
             $<TARGET_FILE_DIR:${APP_NAME}>)
    endif()
endfunction(ax_link_lua_prebuilt)
