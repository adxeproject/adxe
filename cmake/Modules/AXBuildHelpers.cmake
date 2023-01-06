include(CMakeParseArguments)

# copy resource `FILES` and `FOLDERS` to TARGET_FILE_DIR/Resources
function(ax_copy_target_res ax_target)
    ax_def_copy_resource_target(${ax_target})
    set(oneValueArgs LINK_TO)
    set(multiValueArgs FOLDERS)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT TARGET SYNC_RESOURCE-${ax_target})
        message(WARNING "SyncResource targe for ${ax_target} is not defined")
        return()
    endif()

    # linking folders
    foreach(cc_folder ${opt_FOLDERS})
        #get_filename_component(link_folder ${opt_LINK_TO} DIRECTORY)
        get_filename_component(link_folder_abs ${opt_LINK_TO} ABSOLUTE)
        add_custom_command(TARGET SYNC_RESOURCE-${ax_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "    copying to ${link_folder_abs}"
            COMMAND ${PYTHON_COMMAND} ARGS ${_AX_ROOT_PATH}/cmake/scripts/sync_folder.py
                -s ${cc_folder} -d ${link_folder_abs}
        )
    endforeach()
endfunction()

## create a virtual target SYNC_RESOURCE-${ax_target}
## Update resource files in Resources/ folder everytime when `Run/Debug` target.
function(ax_def_copy_resource_target ax_target)
    add_custom_target(SYNC_RESOURCE-${ax_target} ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Copying resources for ${ax_target} ..."
    )
    add_dependencies(${ax_target} SYNC_RESOURCE-${ax_target})
    set_target_properties(SYNC_RESOURCE-${ax_target} PROPERTIES
        FOLDER Utils
    )
endfunction()


function(ax_copy_lua_scripts ax_target src_dir dst_dir)
    set(luacompile_target COPY_LUA-${ax_target})
    if(NOT TARGET ${luacompile_target})
        add_custom_target(${luacompile_target} ALL
            COMMAND ${CMAKE_COMMAND} -E echo "Copying lua scripts ..."
        )
        add_dependencies(${ax_target} ${luacompile_target})
        set_target_properties(${luacompile_target} PROPERTIES
            FOLDER Utils
        )
    endif()
    if(MSVC)
        add_custom_command(TARGET ${luacompile_target} POST_BUILD
            COMMAND ${PYTHON_COMMAND} ARGS ${_AX_ROOT_PATH}/cmake/scripts/sync_folder.py
                -s ${src_dir} -d ${dst_dir} -m $<CONFIG>
        )
    else()
        if("${CMAKE_BUILD_TYPE}" STREQUAL "")
            add_custom_command(TARGET ${luacompile_target} POST_BUILD
                COMMAND ${PYTHON_COMMAND} ARGS ${_AX_ROOT_PATH}/cmake/scripts/sync_folder.py
                -s ${src_dir} -d ${dst_dir}
            )
        else()
            add_custom_command(TARGET ${luacompile_target} POST_BUILD
                COMMAND ${PYTHON_COMMAND} ARGS ${_AX_ROOT_PATH}/cmake/scripts/sync_folder.py
                    -s ${src_dir} -d ${dst_dir} -m ${CMAKE_BUILD_TYPE}
            )
        endif()
    endif()

endfunction()


function(ax_get_resource_path output ax_target)
    get_target_property(rt_output ${ax_target} RUNTIME_OUTPUT_DIRECTORY)
    set(${output} "${rt_output}/${CMAKE_CFG_INTDIR}/Resources" PARENT_SCOPE)
endfunction()


# mark `FILES` and files in `FOLDERS` as resource files, the destination is `RES_TO` folder
# save all marked files in `res_out`
function(ax_mark_multi_resources res_out)
    set(oneValueArgs RES_TO)
    set(multiValueArgs FILES FOLDERS)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(tmp_file_list)
    foreach(cc_file ${opt_FILES})
        get_filename_component(cc_file_abs ${cc_file} ABSOLUTE)
        get_filename_component(file_dir ${cc_file_abs} DIRECTORY)
        ax_mark_resources(FILES ${cc_file_abs} BASEDIR ${file_dir} RESOURCEBASE ${opt_RES_TO})
    endforeach()
    list(APPEND tmp_file_list ${opt_FILES})

    foreach(cc_folder ${opt_FOLDERS})
        file(GLOB_RECURSE folder_files "${cc_folder}/*")
        list(APPEND tmp_file_list ${folder_files})
        ax_mark_resources(FILES ${folder_files} BASEDIR ${cc_folder} RESOURCEBASE ${opt_RES_TO})
    endforeach()
    set(${res_out} ${tmp_file_list} PARENT_SCOPE)
endfunction()

# get all linked libraries including transitive ones, recursive
function(search_depend_libs_recursive ax_target all_depends_out)
    set(all_depends_inner)
    set(targets_prepare_search ${ax_target})
    while(true)
        foreach(tmp_target ${targets_prepare_search})
            get_target_property(target_type ${tmp_target} TYPE)
            if(${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "STATIC_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY" OR ${target_type} STREQUAL "EXECUTABLE")
                get_target_property(tmp_depend_libs ${tmp_target} LINK_LIBRARIES)
                list(REMOVE_ITEM targets_prepare_search ${tmp_target})
                list(APPEND tmp_depend_libs ${tmp_target})
                foreach(depend_lib ${tmp_depend_libs})
                    if(TARGET ${depend_lib})
                        list(APPEND all_depends_inner ${depend_lib})
                        if(NOT (depend_lib STREQUAL tmp_target))
                            list(APPEND targets_prepare_search ${depend_lib})
                        endif()
                    endif()
                endforeach()
            else()
                list(REMOVE_ITEM targets_prepare_search ${tmp_target})
            endif()
        endforeach()
        list(LENGTH targets_prepare_search targets_prepare_search_size)
        if(targets_prepare_search_size LESS 1)
            break()
        endif()
    endwhile(true)
    list(REMOVE_DUPLICATES all_depends_inner)
    set(${all_depends_out} ${all_depends_inner} PARENT_SCOPE)
endfunction()

# get `ax_target` depend all dlls, save the result in `all_depend_dlls_out`
function(get_target_depends_ext_dlls ax_target all_depend_dlls_out)

    set(depend_libs)
    set(all_depend_ext_dlls)
    search_depend_libs_recursive(${ax_target} depend_libs)
    foreach(depend_lib ${depend_libs})
        if(TARGET ${depend_lib})
            get_target_property(target_type ${depend_lib} TYPE)
            if(${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "STATIC_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY" OR ${target_type} STREQUAL "EXECUTABLE")
                get_target_property(found_shared_lib ${depend_lib} IMPORTED_IMPLIB)
                if(found_shared_lib)
                    get_target_property(tmp_dlls ${depend_lib} IMPORTED_LOCATION)
                    list(APPEND all_depend_ext_dlls ${tmp_dlls})
                endif()
            endif()
        endif()
    endforeach()
    list(REMOVE_DUPLICATES all_depend_ext_dlls)
    set(${all_depend_dlls_out} ${all_depend_ext_dlls} PARENT_SCOPE)
endfunction()

function(copy_thirdparty_dlls ax_target destDir)
    # init dependency list with direct dependencies
    get_property(DEPENDENCIES TARGET ${_AX_THIRDPARTY_NAME} PROPERTY LINK_LIBRARIES)
    # We're not intersted in interface link libraries of the top-most target
#     if (INCLUDE_INTERFACE_LINK_LIBRARIES)
#        get_property(INTERFACE_LINK_LIBRARIES TARGET ${_AX_THIRDPARTY_NAME} PROPERTY
#   INTERFACE_LINK_LIBRARIES)
#        list(APPEND DEPENDENCIES ${INTERFACE_LINK_LIBRARIES})
#     endif()

    if(AX_ENABLE_EXT_LUA)
        list(APPEND DEPENDENCIES ${LUA_ENGINE})
        list(APPEND DEPENDENCIES tolua)
    endif()

    if (DEPENDENCIES)
      list(REMOVE_DUPLICATES DEPENDENCIES)
      # message (STATUS "${LIB} dependens on ${DEPENDENCIES}")
    endif()

    SET(EXT_LIB_FILES "")
 
    foreach(DEPENDENCY ${DEPENDENCIES})
      # message(STATUS ${DEPENDENCY} " depends by ${_AX_THIRDPARTY_NAME}")
      get_property(IMPORTLIB TARGET ${DEPENDENCY} PROPERTY IMPORTED_IMPLIB)
      get_property(IMPORTDLL TARGET ${DEPENDENCY} PROPERTY IMPORTED_LOCATION)
      if(IMPORTLIB)
          # message(STATUS "${DEPENDENCY} have import lib ${IMPORTLIB}")
          list(APPEND EXT_LIB_FILES ${IMPORTLIB})
      endif()
      if(IMPORTDLL)
          list(APPEND EXT_LIB_FILES ${IMPORTDLL})
      endif()
    endforeach()

    message(STATUS "EXT_LIB_FILES=${EXT_LIB_FILES}")
    add_custom_command(TARGET ${ax_target}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXT_LIB_FILES}
        ${destDir} # ${CMAKE_BINARY_DIR}/lib/$<CONFIG>
    )
endfunction()

# copy the `ax_target` needed dlls into TARGET_FILE_DIR
function(ax_copy_target_dll ax_target)
    get_target_depends_ext_dlls(${ax_target} all_depend_dlls)
    # remove repeat items
    if(all_depend_dlls)
        list(REMOVE_DUPLICATES all_depend_dlls)
    endif()
    foreach(cc_dll_file ${all_depend_dlls})
        get_filename_component(cc_dll_name ${cc_dll_file} NAME)
        add_custom_command(TARGET ${ax_target} POST_BUILD
            #COMMAND ${CMAKE_COMMAND} -E echo "copy dll into target file dir: ${cc_dll_name} ..."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${cc_dll_file} "$<TARGET_FILE_DIR:${ax_target}>/${cc_dll_name}"
        )
    endforeach()

    # copy thirdparty dlls to target bin dir
    # copy_thirdparty_dlls(${ax_target} $<TARGET_FILE_DIR:${ax_target}>)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        set(BUILD_CONFIG_DIR "\$\(Configuration\)/")
    endif()

    add_custom_command(TARGET ${ax_target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${ax_target}> $<TARGET_FILE_DIR:${ax_target}>
        COMMAND_EXPAND_LISTS
    )

    # Copy windows angle binaries
    if (AX_USE_COMPAT_GL)
        add_custom_command(TARGET ${ax_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${_AX_ROOT_PATH}/${_AX_THIRDPARTY_NAME}/angle/prebuilt/windows/${ARCH_ALIAS}/libGLESv2.dll
            ${_AX_ROOT_PATH}/${_AX_THIRDPARTY_NAME}/angle/prebuilt/windows/${ARCH_ALIAS}/libEGL.dll
            ${_AX_ROOT_PATH}/${_AX_THIRDPARTY_NAME}/angle/prebuilt/windows/${ARCH_ALIAS}/d3dcompiler_47.dll
            $<TARGET_FILE_DIR:${ax_target}>
        )
    endif()

    # Copy webview2 for ninja
    if(AX_ENABLE_MSEDGE_WEBVIEW2)
        if(CMAKE_GENERATOR STREQUAL "Ninja")
            add_custom_command(TARGET ${ax_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/${ARCH_ALIAS}/WebView2Loader.dll"
            $<TARGET_FILE_DIR:${ax_target}>)
        endif()
    endif()
endfunction()

function(ax_copy_lua_dlls ax_target)
    if(NOT AX_USE_LUAJIT)
        if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
            set(BUILD_CONFIG_DIR "\$\(Configuration\)/")
        endif()
        add_custom_command(TARGET ${ax_target} POST_BUILD
           COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/bin/${BUILD_CONFIG_DIR}plainlua.dll"
             $<TARGET_FILE_DIR:${ax_target}>)
    endif()
endfunction()

# mark `FILES` as resources, files will be put into sub-dir tree depend on its absolute path
function(ax_mark_resources)
    set(oneValueArgs BASEDIR RESOURCEBASE)
    set(multiValueArgs FILES)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT opt_RESOURCEBASE)
        set(opt_RESOURCEBASE Resources)
    endif()

    get_filename_component(BASEDIR_ABS ${opt_BASEDIR} ABSOLUTE)
    foreach(RES_FILE ${opt_FILES} ${opt_UNPARSED_ARGUMENTS})
        get_filename_component(RES_FILE_ABS ${RES_FILE} ABSOLUTE)
        file(RELATIVE_PATH RES ${BASEDIR_ABS} ${RES_FILE_ABS})
        get_filename_component(RES_LOC ${RES} PATH)
        set_source_files_properties(${RES_FILE} PROPERTIES
                                    MACOSX_PACKAGE_LOCATION "${opt_RESOURCEBASE}/${RES_LOC}"
                                    HEADER_FILE_ONLY 1
                                    )

        if(XCODE OR VS)
            string(REPLACE "/" "\\" ide_source_group "${opt_RESOURCEBASE}/${RES_LOC}")
            source_group("${ide_source_group}" FILES ${RES_FILE})
        endif()
    endforeach()
endfunction()

# mark the code sources of `ax_target` into sub-dir tree
function(ax_mark_code_files ax_target)
    set(oneValueArgs GROUPBASE)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT opt_GROUPBASE)
        set(root_dir ${CMAKE_CURRENT_SOURCE_DIR})
    else()
        set(root_dir ${opt_GROUPBASE})
        message(STATUS "target ${ax_target} code group base is: ${root_dir}")
    endif()

    # message(STATUS "ax_mark_code_files: ${ax_target}")

    get_property(file_list TARGET ${ax_target} PROPERTY SOURCES)

    foreach(single_file ${file_list})
        source_group_single_file(${single_file} GROUP_TO "Source Files" BASE_PATH "${root_dir}")
    endforeach()

endfunction()

# source group one file
# cut the `single_file` absolute path from `BASE_PATH`, then mark file to `GROUP_TO`
function(source_group_single_file single_file)
    set(oneValueArgs GROUP_TO BASE_PATH)
    cmake_parse_arguments(opt "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    # get relative_path
    get_filename_component(abs_path ${single_file} ABSOLUTE)
    file(RELATIVE_PATH relative_path_with_name ${opt_BASE_PATH} ${abs_path})
    get_filename_component(relative_path ${relative_path_with_name} PATH)
    # set source_group, consider sub source group
    string(REPLACE "/" "\\" ide_file_group "${opt_GROUP_TO}/${relative_path}")
    source_group("${ide_file_group}" FILES ${single_file})
endfunction()

# setup a ax application
function(setup_ax_app_config app_name)
    # put all output app into bin/${app_name}
    set_target_properties(${app_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${app_name}")
    if(APPLE)
        # output macOS/iOS .app
        set_target_properties(${app_name} PROPERTIES MACOSX_BUNDLE 1)
        if(IOS AND (NOT ("${CMAKE_OSX_SYSROOT}" MATCHES ".*simulator.*")))
            set_xcode_property(${APP_NAME} CODE_SIGNING_REQUIRED "YES")
            set_xcode_property(${APP_NAME} CODE_SIGNING_ALLOWED "YES")
        else()
            # By default, explicit disable codesign for macOS PC
            set_xcode_property(${APP_NAME} CODE_SIGN_IDENTITY "")
            set_xcode_property(${APP_NAME} CODE_SIGNING_ALLOWED "NO")
            set_xcode_property(${APP_NAME} CODE_SIGN_IDENTITY "NO")
        endif()
    elseif(MSVC)
        # visual studio default is Console app, but we need Windows app
        set_property(TARGET ${app_name} APPEND PROPERTY LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    endif()
    # auto mark code files for IDE when mark app
    if(XCODE OR VS)
        ax_mark_code_files(${app_name})
    endif()

    if (XCODE)
        ax_config_app_xcode_property(${app_name})
    endif()

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${app_name} PRIVATE SPINEPLUGIN_API=DLLIMPORT) # spine dll
    endif()
    target_link_libraries(${app_name} ${_AX_EXTENSION_LIBS})

    if(XCODE AND AX_USE_ALSOFT AND ALSOFT_OSX_FRAMEWORK)
        # Embedded soft_oal embedded framework
        # XCODE_LINK_BUILD_PHASE_MODE BUILT_ONLY
        # ???CMake BUG: XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY works for first app
        message(STATUS "Embedding framework soft_oal to ${app_name}...")
        set_target_properties(${app_name} PROPERTIES
            XCODE_LINK_BUILD_PHASE_MODE KNOWN_LOCATION
            XCODE_EMBED_FRAMEWORKS OpenAL
            XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
            XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY ON
        )
    endif()
endfunction()

# if cc_variable not set, then set it cc_value
macro(ax_set_default_value cc_variable cc_value)
    if(NOT DEFINED ${cc_variable})
        set(${cc_variable} ${cc_value})
    endif()
endmacro()

# set Xcode property for application, include all depend target
macro(ax_config_app_xcode_property ax_app)
    set(depend_libs)
    search_depend_libs_recursive(${ax_app} depend_libs)
    foreach(depend_lib ${depend_libs})
        if(TARGET ${depend_lib})
            ax_config_target_xcode_property(${depend_lib})
        endif()
    endforeach()
endmacro()

# custom Xcode property for iOS target
macro(ax_config_target_xcode_property ax_target)
    if(IOS)
        set(real_target)
        get_property(real_target TARGET ${ax_target} PROPERTY ALIASED_TARGET)
        if (NOT real_target)
            set(real_target ${ax_target})
        endif()
        set_xcode_property(${real_target} ENABLE_BITCODE "NO")
        set_xcode_property(${real_target} ONLY_ACTIVE_ARCH "YES")
    endif()
endmacro()

# This little macro lets you set any XCode specific property, from ios.toolchain.cmake
function(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property(TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endfunction(set_xcode_property)

# works same as find_package, but do additional care to properly find
macro(ax_find_package pkg_name pkg_prefix)
    if(NOT ${pkg_prefix}_FOUND)
        find_package(${pkg_name} ${ARGN})
    endif()
    if(NOT ${pkg_prefix}_INCLUDE_DIRS AND ${pkg_prefix}_INCLUDE_DIR)
        set(${pkg_prefix}_INCLUDE_DIRS ${${pkg_prefix}_INCLUDE_DIR})
    endif()
    if(NOT ${pkg_prefix}_LIBRARIES AND ${pkg_prefix}_LIBRARY)
        set(${pkg_prefix}_LIBRARIES ${${pkg_prefix}_LIBRARY})
    endif()

    message(STATUS "${pkg_name} include dirs: ${${pkg_prefix}_INCLUDE_DIRS}")
endmacro()

# ax_use_pkg(pkg) function.
# This function applies standard package variables (after find_package(pkg) call) to current scope
# Recognized variables: <pkg>_INCLUDE_DIRS, <pkg>_LIBRARIES, <pkg>_LIBRARY_DIRS
# Also if BUILD_SHARED_LIBS variable off, it is try to use <pkg>_STATIC_* vars before
function(ax_use_pkg target pkg)
    set(prefix ${pkg})

    set(_include_dirs)
    if(NOT _include_dirs)
        set(_include_dirs ${${prefix}_INCLUDE_DIRS})
    endif()
    if(NOT _include_dirs)
        # backward compat with old package-find scripts
        set(_include_dirs ${${prefix}_INCLUDE_DIR})
    endif()
    if(_include_dirs)
        include_directories(${_include_dirs})
        message(STATUS "${pkg} add to include_dirs: ${_include_dirs}")
    endif()

    set(_library_dirs)
    if(NOT _library_dirs)
        set(_library_dirs ${${prefix}_LIBRARY_DIRS})
    endif()
    if(_library_dirs)
        link_directories(${_library_dirs})
        # message(STATUS "${pkg} add to link_dirs: ${_library_dirs}")
    endif()

    set(_libs)
    if(NOT _libs)
        set(_libs ${${prefix}_LIBRARIES})
    endif()
    if(NOT _libs)
        set(_libs ${${prefix}_LIBRARY})
    endif()
    if(_libs)
        target_link_libraries(${target} ${_libs})
        message(STATUS "${pkg} libs added to '${target}': ${_libs}")
    endif()

    set(_defs)
    if(NOT _defs)
        set(_defs ${${prefix}_DEFINITIONS})
    endif()
    if(_defs)
        add_definitions(${_defs})
        message(STATUS "${pkg} add definitions: ${_defs}")
    endif()

    set(_dlls)
    if(NOT _dlls)
        set(_dlls ${${prefix}_DLLS})
    endif()
    if(_dlls)
        if(MSVC)
            # message(STATUS "${target} add dll: ${_dlls}")
            get_property(pre_dlls
                         TARGET ${target}
                         PROPERTY AX_DEPEND_DLLS)
            if(pre_dlls)
                set(_dlls ${pre_dlls} ${_dlls})
            endif()
            set_property(TARGET ${target}
                         PROPERTY
                         AX_DEPEND_DLLS ${_dlls}
                         )
        endif()
    endif()

endfunction()

# The axmol preprocessors config helper macro
macro(ax_config_pred target_name pred)
    if(${pred})
        target_compile_definitions(${target_name} PUBLIC ${pred}=1)
    endif()
endmacro()
