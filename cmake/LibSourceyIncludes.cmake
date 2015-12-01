#
### Macro: sourcey_find_library
#
# Finds libraries with finer control over search paths
# for compilers with multiple configuration types.
#
macro(sourcey_find_library prefix)

  include(CMakeParseArguments REQUIRED)
  # cmake_parse_arguments(prefix options singleValueArgs multiValueArgs ${ARGN})
  cmake_parse_arguments(${prefix}
    ""
    ""
    "NAMES;DEBUG_NAMES;RELEASE_NAMES;DEBUG_PATHS;RELEASE_PATHS;PATHS"
    ${ARGN}
    )

  if(WIN32 AND MSVC)

    if(NOT ${prefix}_DEBUG_PATHS)
      list(APPEND ${prefix}_DEBUG_PATHS ${${prefix}_PATHS})
    endif()

    # Reloading to ensure build always passes and picks up changes
    # This is more expensive but proves useful for fragmented libraries like WebRTC
    set(${prefix}_DEBUG_LIBRARY ${prefix}_DEBUG_LIBRARY-NOTFOUND)
    find_library(${prefix}_DEBUG_LIBRARY
      NAMES
        ${${prefix}_DEBUG_NAMES}
        ${${prefix}_NAMES}
      PATHS
        ${${prefix}_DEBUG_PATHS}
        # ${${prefix}_PATHS}
      )

    if(NOT ${prefix}_RELEASE_PATHS)
      list(APPEND ${prefix}_RELEASE_PATHS ${${prefix}_PATHS})
    endif()

    set(${prefix}_RELEASE_LIBRARY ${prefix}_RELEASE_LIBRARY-NOTFOUND)
    find_library(${prefix}_RELEASE_LIBRARY
      NAMES
        ${${prefix}_RELEASE_NAMES}
        ${${prefix}_NAMES}
      PATHS
        ${${prefix}_RELEASE_PATHS}
        # ${${prefix}_PATHS}
      )

    if(${prefix}_DEBUG_LIBRARY OR ${prefix}_RELEASE_LIBRARY)
      if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        #if (${prefix}_RELEASE_LIBRARY)
          list(APPEND ${prefix}_LIBRARY "optimized" ${${prefix}_RELEASE_LIBRARY})
        #endif()
        #if (${prefix}_DEBUG_LIBRARY)
          list(APPEND ${prefix}_LIBRARY "debug" ${${prefix}_DEBUG_LIBRARY})
        #endif()
      else()
        if (${prefix}_RELEASE_LIBRARY)
          list(APPEND ${prefix}_LIBRARY ${${prefix}_RELEASE_LIBRARY})
        elseif (${prefix}_DEBUG_LIBRARY)
          list(APPEND ${prefix}_LIBRARY ${${prefix}_DEBUG_LIBRARY})
        endif()
      endif()
      #mark_as_advanced(${prefix}_DEBUG_LIBRARY ${prefix}_RELEASE_LIBRARY)
    endif()

  else()

    find_library(${prefix}_LIBRARY
      NAMES
        # ${${prefix}_RELEASE_NAMES}
        # ${${prefix}_DEBUG_NAMES}
        ${${prefix}_NAMES}
      PATHS
        # ${${prefix}_RELEASE_PATHS}
        # ${${prefix}_DEBUG_PATHS}
        ${${prefix}_PATHS}
      )

  endif()

  #message("*** Sourcey find library for ${prefix}")
  #message("Debug Library: ${${prefix}_DEBUG_LIBRARY}")
  #message("Release Library: ${${prefix}_RELEASE_LIBRARY}")
  #message("Library: ${${prefix}_LIBRARY}")
  #message("Debug Paths: ${${prefix}_RELEASE_PATHS}")
  #message("Release Paths: ${${prefix}_DEBUG_PATHS}")
  #message("Paths: ${${prefix}_PATHS}")
  #message("Debug Names: ${${prefix}_RELEASE_NAMES}")
  #message("Release Names: ${${prefix}_DEBUG_NAMES}")
  #message("Names: ${${prefix}_NAMES}")

endmacro(sourcey_find_library)


#
### Macro:  include_dependency
#
# Includes a 3rd party dependency into the LibSourcey solution.
#
macro(include_dependency name)
  #message(STATUS "Including dependency: ${name}")

  find_package(${name} ${ARGN})

  # Determine the variable scope
  set(var_root ${name})
  set(lib_found 0)
  string(TOUPPER ${var_root} var_root_upper)
  if(${var_root}_FOUND)
    set(lib_found 1)
  else()
    # Try to use old style uppercase variable accessor
    if(${var_root_upper}_FOUND)
      set(var_root ${var_root_upper})
      set(lib_found 1)
    endif()
  endif()

  # Exit message on failure
  if (NOT ${var_root}_FOUND)
    message("Failed to include dependency: ${name}. Please build LibSourcey dependencies first by enabling BUILD_DEPENDENCIES and disabling BUILD_MODULES and BUILD_APPLICATIONS")
    return()
  endif()

  # Set a HAVE_XXX variable at parent scope for our Config.h
  set(HAVE_${var_root_upper} 1)
  #set(HAVE_${var_root_upper} 1 PARENT_SCOPE)

  # Expose to LibSourcey
  if(${var_root}_INCLUDE_DIR)
    #message(STATUS "- Found ${name} Inc Dir: ${${var_root}_INCLUDE_DIR}")
    #include_directories(${${var_root}_INCLUDE_DIR})
    list(APPEND LibSourcey_INCLUDE_DIRS ${${var_root}_INCLUDE_DIR})
  endif()
  if(${var_root}_INCLUDE_DIRS)
    #message(STATUS "- Found ${name} Inc Dirs: ${${var_root}_INCLUDE_DIRS}")
    #include_directories(${${var_root}_INCLUDE_DIRS})
    list(APPEND LibSourcey_INCLUDE_DIRS ${${var_root}_INCLUDE_DIRS})
  endif()
  if(${var_root}_LIBRARY_DIR)
    #message(STATUS "- Found ${name} Lib Dir: ${${var_root}_LIBRARY_DIR}")
    #link_directories(${${var_root}_LIBRARY_DIR})
    list(APPEND LibSourcey_LIBRARY_DIRS ${${var_root}_LIBRARY_DIR})
  endif()
  if(${var_root}_LIBRARY_DIRS)
    message(STATUS "- Found ${name} Lib Dirs: ${${var_root}_LIBRARY_DIRS}")
    #link_directories(${${var_root}_LIBRARY_DIRS})
    list(APPEND LibSourcey_LIBRARY_DIRS ${${var_root}_LIBRARY_DIRS})
  endif()
  if(${var_root}_LIBRARY)
    message(STATUS "- Found dependency lib ${name}: ${${var_root}_LIBRARY}")
    list(APPEND LibSourcey_INCLUDE_LIBRARIES ${${var_root}_LIBRARY})
    #list(APPEND LibSourcey_BUILD_DEPENDENCIES ${${var_root}_LIBRARY})
  endif()
  if(${var_root}_LIBRARIES)
    message(STATUS "- Found dependency libs ${name}: ${${var_root}_LIBRARIES}")
    list(APPEND LibSourcey_INCLUDE_LIBRARIES ${${var_root}_LIBRARIES})
    #list(APPEND LibSourcey_BUILD_DEPENDENCIES ${${var_root}_LIBRARIES})
  endif()
  if(${var_root}_DEPENDENCIES)
    message(STATUS "- Found external dependency ${name}: ${${var_root}_DEPENDENCIES}")
    list(APPEND LibSourcey_INCLUDE_LIBRARIES ${${var_root}_DEPENDENCIES})
    #list(APPEND LibSourcey_BUILD_DEPENDENCIES ${${var_root}_DEPENDENCIES})
  endif()

  list(REMOVE_DUPLICATES LibSourcey_INCLUDE_DIRS)
  list(REMOVE_DUPLICATES LibSourcey_LIBRARY_DIRS)
  list(REMOVE_DUPLICATES LibSourcey_INCLUDE_LIBRARIES)
  #list(REMOVE_DUPLICATES LibSourcey_BUILD_DEPENDENCIES)
endmacro()

#
### Macro: set_default_project_directories
#
# Set the default header and library directories for a LibSourcey project.
#
macro(set_default_project_directories)
  foreach(module ${ARGN})
    list(APPEND LibSourcey_LIBRARY_DIRS ${LibSourcey_BUILD_DIR}/src/${module})
    list(APPEND LibSourcey_INCLUDE_DIRS ${LibSourcey_SOURCE_DIR}/${module}/include)
  endforeach()

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
  include_directories(${LibSourcey_INCLUDE_DIRS})
  link_directories(${LibSourcey_LIBRARY_DIRS})
endmacro()


#
### Macro: set_default_project_dependencies
#
# Set the default dependencies for a LibSourcey project.
#
macro(set_default_project_dependencies name)
  foreach(dep ${LibSourcey_BUILD_DEPENDENCIES})
    add_dependencies(${name} ${dep})
  endforeach()

  # Include dependent modules
  foreach(module ${ARGN})
    #if(NOT ${module} MATCHES "util")
      add_dependencies(${name} ${module})
    #endif()
  endforeach()

  # Include all linker libraries
  set(${name}_LIBRARIES ${ARGN})
  list(APPEND ${name}_LIBRARIES ${LibSourcey_BUILD_DEPENDENCIES})
  list(APPEND ${name}_LIBRARIES ${LibSourcey_INCLUDE_LIBRARIES})
  list(REMOVE_DUPLICATES ${name}_LIBRARIES)
  target_link_libraries(${name} ${${name}_LIBRARIES})
endmacro()

#
### Macro: include_sourcey_modules
#
# Includes dependent LibSourcey module(s) into a project.
#
#macro(include_sourcey_modules)
#  foreach(name ${ARGN})
#    #message(STATUS "Including module: ${name}")
#
#    set_scy_libname(${name} lib_name)
#
#    # Include the build library directory.
#    list(APPEND LibSourcey_LIBRARY_DIRS ${LibSourcey_BUILD_DIR}/src/${name})
#
#    # Include the module headers.
#    # These may be located in the "src/" root directory or in a sub directory.
#    set(HAVE_SOURCEY_${name} 0)
#    if(IS_DIRECTORY "${LibSourcey_SOURCE_DIR}/${name}/include")
#      #include_directories("${LibSourcey_SOURCE_DIR}/${name}/include")
#      list(APPEND LibSourcey_INCLUDE_DIRS ${LibSourcey_SOURCE_DIR}/${name}/include)
#      set(HAVE_SOURCEY_${name} 1)
#    else()
#      subdirlist(subdirs "${LibSourcey_SOURCE_DIR}")
#      foreach(dir ${subdirs})
#        set(dir "${LibSourcey_SOURCE_DIR}/${dir}/${name}/include")
#        if(IS_DIRECTORY ${dir})
#          #include_directories(${dir})
#          list(APPEND LibSourcey_INCLUDE_DIRS ${dir})
#          set(HAVE_SOURCEY_${name} 1)
#        endif()
#      endforeach()
#    endif()
#
#    if (NOT HAVE_SOURCEY_${name})
#      message(ERROR "Unable to include dependent LibSourcey module ${name}. The build may fail.")
#    endif()
#
#    # Create a Debug and a Release list for MSVC
#    if (MSVC)
#
#      # Find the module giving priority to the build output folder, and install folder second.
#      # This way we don't have to install updated modules before building applications
#      # simplifying the build process.
#
#      # Always reset the module folders.
#      set(LibSourcey_${name}_RELEASE LibSourcey_${name}_RELEASE-NOTFOUND)
#      #set(LibSourcey_${name}_RELEASE LibSourcey_${name}_RELEASE-NOTFOUND PARENT_SCOPE)
#      set(LibSourcey_${name}_DEBUG LibSourcey_${name}_DEBUG-NOTFOUND)
#      #set(LibSourcey_${name}_DEBUG LibSourcey_${name}_DEBUG-NOTFOUND PARENT_SCOPE)
#
#      # Since CMake doesn't give priority to the PATHS parameter, we need to search twice:
#      # once using NO_DEFAULT_PATH, and once using default values.
#      # TODO: Better handle nested module build dirs, or have all modules build to
#      # intermediate directory for easy searching.
#      find_library(LibSourcey_${name}_RELEASE "${lib_name}"
#        PATHS
#          ${LibSourcey_BUILD_DIR}/src/${name}
#          ${LibSourcey_BUILD_DIR}/src/anionu-sdk/${name}
#          ${LibSourcey_BUILD_DIR}/src/anionu-private/${name}
#        PATH_SUFFIXES Release
#        NO_DEFAULT_PATH)
#      find_library(LibSourcey_${name}_DEBUG "${lib_name}d"
#        PATHS
#          ${LibSourcey_BUILD_DIR}/src/${name}
#          ${LibSourcey_BUILD_DIR}/src/anionu-sdk/${name}
#          ${LibSourcey_BUILD_DIR}/src/anionu-private/${name}
#        PATH_SUFFIXES Debug
#        NO_DEFAULT_PATH)
#
#      # Search the module install folder if none was located.
#      find_library(LibSourcey_${name}_RELEASE "${lib_name}")
#      find_library(LibSourcey_${name}_DEBUG "${lib_name}d")
#
#      if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
#        if (LibSourcey_${name}_RELEASE)
#          list(APPEND LibSourcey_INCLUDE_LIBRARIES "optimized" ${LibSourcey_${name}_RELEASE})
#          mark_as_advanced(LibSourcey_${name}_RELEASE)
#        endif()
#        if (LibSourcey_${name}_DEBUG)
#          list(APPEND LibSourcey_INCLUDE_LIBRARIES "debug" ${LibSourcey_${name}_DEBUG})
#          mark_as_advanced(LibSourcey_${name}_DEBUG)
#        endif()
#      else()
#        if (LibSourcey_${name}_RELEASE)
#          list(APPEND LibSourcey_INCLUDE_LIBRARIES ${LibSourcey_${name}_RELEASE})
#          mark_as_advanced(LibSourcey_${name}_RELEASE)
#        endif()
#        if (LibSourcey_${name}_DEBUG)
#          mark_as_advanced(LibSourcey_${name}_DEBUG)
#        endif()
#      endif()
#    else()
#      # Find the module giving preference to the build output folder.
#      #find_library(LibSourcey_${name} "${lib_name}"
#      #  PATHS ${LibSourcey_BUILD_DIR}/lib/${name}
#      #  NO_DEFAULT_PATH)
#      find_library(LibSourcey_${name} "${lib_name}")
#      if (LibSourcey_${name})
#        # Prepend module libraries otherwise linking may fail
#        # on compilers that require ordering of link libraries.
#        set(LibSourcey_INCLUDE_LIBRARIES ${LibSourcey_${name}} ${LibSourcey_INCLUDE_LIBRARIES})
#        mark_as_advanced(LibSourcey_${name})
#      else()
#      endif()
#    endif()
#  endforeach()
#endmacro()


### Macro: set_component_alias
#
# Sets the current module component alias variables.
#
macro(set_component_alias module component)
  set(ALIAS                    ${module}_${component})
  set(ALIAS_FOUND              ${ALIAS}_FOUND)
  set(ALIAS_LIBRARIES          ${ALIAS}_LIBRARIES)
  set(ALIAS_RELEASE_LIBRARIES  ${ALIAS}_RELEASE_LIBRARIES)
  set(ALIAS_DEBUG_LIBRARIES    ${ALIAS}_DEBUG_LIBRARIES)
  set(ALIAS_INCLUDE_DIRS       ${ALIAS}_INCLUDE_DIRS)
  set(ALIAS_LIBRARY_DIRS       ${ALIAS}_LIBRARY_DIRS)
  set(ALIAS_DEFINITIONS        ${ALIAS}_CFLAGS_OTHER)
  set(ALIAS_VERSION            ${ALIAS}_VERSION)
endmacro()


#
### Macro: set_module_found
#
# Marks the given module as found if all required components are present.
#
macro(set_module_found module)

  set(${module}_FOUND FALSE)
  #set(${module}_FOUND FALSE PARENT_SCOPE)

  # Compile the list of required vars
  set(_${module}_REQUIRED_VARS ${module}_LIBRARIES) # ${module}_INCLUDE_DIRS
  foreach (component ${${module}_FIND_COMPONENTS})
    # NOTE: Not including XXX_INCLUDE_DIRS as required var since it may be empty
    list(APPEND _${module}_REQUIRED_VARS ${module}_${component}_LIBRARIES) # ${module}_${component}_INCLUDE_DIRS
    if (NOT ${module}_${component}_FOUND)
      message(ERROR "Required ${module} component ${component} missing. Please recompile ${module} with ${component} enabled.")
    #else()
    #  message(STATUS "  - Required ${module} component ${component} found.")
    endif()
  endforeach()

  # Cache the vars.
  set(${module}_INCLUDE_DIRS ${${module}_INCLUDE_DIRS} CACHE STRING   "The ${module} include directories." FORCE)
  set(${module}_LIBRARY_DIRS ${${module}_LIBRARY_DIRS} CACHE STRING   "The ${module} library directories." FORCE)
  set(${module}_LIBRARIES    ${${module}_LIBRARIES}    CACHE STRING   "The ${module} libraries." FORCE)
  set(${module}_FOUND        ${${module}_FOUND}        CACHE BOOLEAN  "The ${module} found status." FORCE)
  #set(${module}_INCLUDE_DIRS ${${module}_INCLUDE_DIRS} PARENT_SCOPE)
  #set(${module}_LIBRARY_DIRS ${${module}_LIBRARY_DIRS} PARENT_SCOPE)
  #set(${module}_LIBRARIES    ${${module}_LIBRARIES}    PARENT_SCOPE)
  #set(${module}_FOUND        ${${module}_FOUND}        PARENT_SCOPE)

  # Ensure required variables have been set, or fail in error.
  if (${module}_FIND_REQUIRED)
    # Give a nice error message if some of the required vars are missing.
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(${module} DEFAULT_MSG ${_${module}_REQUIRED_VARS})

    # Set the module as found.
    set(${module}_FOUND TRUE)
    #set(${module}_FOUND TRUE PARENT_SCOPE)
  else()
    message("Failed to locate ${module}. Please specify paths manually.")
  endif()

  mark_as_advanced(${module}_INCLUDE_DIRS
                   ${module}_LIBRARY_DIRS
                   ${module}_LIBRARIES
                   ${module}_DEFINITIONS
                   ${module}_FOUND)

  #message("Module Found=${module}")

endmacro()


#
### Macro: set_component_found
#
# Marks the given component as found if both *_LIBRARIES AND *_INCLUDE_DIRS is present.
#
macro(set_component_found module component)

  set_component_alias(${module} ${component})

  #message("${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
  #message("${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")
  #message("${ALIAS_LIBRARY_DIRS}=${${ALIAS_LIBRARY_DIRS}}")

  #if (${module}_${component}_LIBRARIES AND ${module}_${component}_INCLUDE_DIRS)
  if (${ALIAS_LIBRARIES}) # AND ${ALIAS_INCLUDE_DIRS} (XXX_INCLUDE_DIRS may be empty)
    #message(STATUS "  - ${module} ${component} found.")
    set(${ALIAS_FOUND} TRUE)
    #set(${ALIAS_FOUND} TRUE PARENT_SCOPE)

    # Add component vars to the perant module lists
    append_unique_list(${module}_INCLUDE_DIRS ${ALIAS_INCLUDE_DIRS})
    append_unique_list(${module}_LIBRARY_DIRS ${ALIAS_LIBRARY_DIRS})
    append_unique_list(${module}_LIBRARIES    ${ALIAS_LIBRARIES})
    append_unique_list(${module}_DEFINITIONS  ${ALIAS_DEFINITIONS})

    #set(${module}_INCLUDE_DIRS ${${module}_INCLUDE_DIRS} PARENT_SCOPE)
    #set(${module}_LIBRARY_DIRS ${${module}_LIBRARY_DIRS} PARENT_SCOPE)
    #set(${module}_LIBRARIES    ${${module}_LIBRARIES}    PARENT_SCOPE)
    #set(${module}_DEFINITIONS  ${${module}_DEFINITIONS}  PARENT_SCOPE)

    #message("Find Component Paths=${module}:${component}:${library}:${header}")
    #message("${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")
    #message("${ALIAS_RELEASE_LIBRARIES}=${${ALIAS_RELEASE_LIBRARIES}}")
    #message("${ALIAS_DEBUG_LIBRARIES}=${${ALIAS_DEBUG_LIBRARIES}}")
    #message("${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
    #message("${module}_INCLUDE_DIRS=${${module}_INCLUDE_DIRS}")
    #message("${module}_LIBRARIES=${${module}_LIBRARIES}")

    # Only mark as advanced when found
    mark_as_advanced(
      ${ALIAS_INCLUDE_DIRS}
      ${ALIAS_LIBRARY_DIRS})

  else()
     # NOTE: an error message will be displayed in set_module_found if the module is REQUIRED
     #message(STATUS "  - ${module} ${component} not found.")
  endif()

  mark_as_advanced(
    ${ALIAS_FOUND}
    ${ALIAS_DEBUG_LIBRARIES}
    ${ALIAS_RELEASE_LIBRARIES}
    ${ALIAS_LIBRARIES}
    ${ALIAS_DEFINITIONS}
    ${ALIAS_VERSION})

endmacro()


#
### Macro: set_module_notfound
#
# Marks the given component as not found, and resets the cache for find_path and find_library results.
#
macro(set_module_notfound module)

  #message(STATUS "  - Setting ${module} not found.")
  set(${module}_FOUND FALSE)
  #set(${module}_FOUND FALSE PARENT_SCOPE)

  if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    set(${module}_RELEASE_LIBRARIES "")
    set(${module}_DEBUG_LIBRARIES "")
    set(${module}_LIBRARIES "")
    #set(${module}_RELEASE_LIBRARIES "" PARENT_SCOPE)
    #set(${module}_DEBUG_LIBRARIES "" PARENT_SCOPE)
    #set(${module}_LIBRARIES "" PARENT_SCOPE)
  else()
    set(${module}_LIBRARIES ${ALIAS_LIBRARIES}-NOTFOUND)
    #set(${module}_LIBRARIES ${ALIAS_LIBRARIES}-NOTFOUND PARENT_SCOPE)
  endif()

endmacro()


#
### Macro: set_component_notfound
#
# Marks the given component as not found, and resets the cache for find_path and find_library results.
#
macro(set_component_notfound module component)

  set_component_alias(${module} ${component})

  #message(STATUS "  - Setting ${module} ${component} not found.")
  set(${ALIAS_FOUND} FALSE)
  #set(${ALIAS_FOUND} FALSE PARENT_SCOPE)

  if (${module}_MULTI_CONFIGURATION AND (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
    set(${ALIAS_RELEASE_LIBRARIES} ${ALIAS_RELEASE_LIBRARIES}-NOTFOUND)
    set(${ALIAS_DEBUG_LIBRARIES} ${ALIAS_DEBUG_LIBRARIES}-NOTFOUND)
    set(${ALIAS_LIBRARIES} "") #${module}_${component}_LIBRARIES-NOTFOUND)
    #set(${ALIAS_RELEASE_LIBRARIES} ${ALIAS_RELEASE_LIBRARIES}-NOTFOUND PARENT_SCOPE)
    #set(${ALIAS_DEBUG_LIBRARIES} ${ALIAS_DEBUG_LIBRARIES}-NOTFOUND PARENT_SCOPE)
    #set(${ALIAS_LIBRARIES} "") #${module}_${component}_LIBRARIES-NOTFOUND PARENT_SCOPE)
  else()
    set(${ALIAS_LIBRARIES} ${ALIAS_LIBRARIES}-NOTFOUND)
    #set(${ALIAS_LIBRARIES} ${ALIAS_LIBRARIES}-NOTFOUND PARENT_SCOPE)
  endif()

endmacro()


#
### Macro: find_paths
#
# Finds the given component library and include paths.
#
macro(find_component_paths module component library header)
  #message(STATUS "Find Component Paths=${module}:${component}:${library}:${header}")

  # Reset alias namespace (force recheck)
  set_component_alias(${module} ${component})

  # Reset search paths (force recheck)
  set_component_notfound(${module} ${component})

  find_path(${ALIAS_INCLUDE_DIRS} ${header}
    #HINTS
    #  ${${ALIAS_INCLUDE_DIRS}}
    PATH_SUFFIXES
      ${${module}_PATH_SUFFIXES}
  )

  # Create a Debug and a Release list for multi configuration builds.
  # NOTE: <module>_CONFIGURATION_TYPES must be set to use this.
  if (${module}_MULTI_CONFIGURATION AND (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE))
    find_library(${ALIAS_RELEASE_LIBRARIES}
      NAMES
        ${library}
        #lib${library}
        #lib${library}.so
        #${library}.lib
        #${library}
      #HINTS
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
    )
    find_library(${ALIAS_DEBUG_LIBRARIES}
      NAMES
        ${library}d
        #lib${library}d.a
        #lib${library}d.so
        #${library}d.lib
      #HINTS
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
    )
    if (${ALIAS_RELEASE_LIBRARIES})
      list(APPEND ${ALIAS_LIBRARIES} "optimized" ${${ALIAS_RELEASE_LIBRARIES}})
    endif()
    if (${ALIAS_DEBUG_LIBRARIES})
      list(APPEND ${ALIAS_LIBRARIES} "debug" ${${ALIAS_DEBUG_LIBRARIES}})
    endif()
  else()
    find_library(${ALIAS_LIBRARIES}
      NAMES # setting in order might help overcome find_library bugs :/
        #lib${library}.so
        #lib${library}.a
        #${library}.lib
        ${library}
      #HINTS
      PATHS
        ${${ALIAS_LIBRARY_DIRS}}
    )
  endif()

  set_component_found(${module} ${component})
endmacro()


#
### Macro: find_component
#
# Checks for the given component by invoking pkgconfig and then looking up the libraries and
# include directories.
#
macro(find_component module component pkgconfig library header)
  # message("Find Component=${module}:${component}:${pkgconfig}:${library}:${header}")

  # Reset component alias values (force recheck)
  set_component_alias(${module} ${component})

  # Use pkg-config to obtain directories for
  # the FIND_PATH() and find_library() calls.
  find_package(PkgConfig QUIET)
  if (PKG_CONFIG_FOUND)
    #set(PKG_ALIAS                   PKG_${component})
    #pkg_check_modules(${PKG_ALIAS}  ${pkgconfig})
    #set(${ALIAS}_FOUND              ${${PKG_ALIAS}_FOUND})
    #set(${ALIAS}_LIBRARIES          ${${PKG_ALIAS}_LIBRARIES})
    #set(${ALIAS}_INCLUDE_DIRS       ${${PKG_ALIAS}_INCLUDE_DIRS})
    #set(${ALIAS}_LIBRARY_DIRS       ${${PKG_ALIAS}_LIBRARY_DIRS})
    #set(${ALIAS}_DEFINITIONS        ${${PKG_ALIAS}_CFLAGS_OTHER})
    #set(${ALIAS}_VERSION            ${${PKG_ALIAS}_VERSION})

    pkg_search_module(${ALIAS} ${pkgconfig})
    #message(STATUS "Find Component PkgConfig=${ALIAS}:${${ALIAS}_FOUND}:${${ALIAS}_LIBRARIES}:${${ALIAS}_INCLUDE_DIRS}:${${ALIAS}_LIBRARY_DIRS}:${${ALIAS}_LIBDIR}:${${ALIAS}_INCLUDEDIR}")
  endif()

  #message(STATUS "${ALIAS_FOUND}=${${ALIAS_FOUND}}")
  #message(STATUS "${ALIAS_LIBRARIES}=${${ALIAS_LIBRARIES}}")
  #message(STATUS "${ALIAS_INCLUDE_DIRS}=${${ALIAS_INCLUDE_DIRS}}")

  if(NOT ${ALIAS_FOUND})
    #message(STATUS "  - ${module} ${component} pkg-config not found, searching...")
    find_component_paths(${module} ${component} ${library} ${header})
  else()
    #message(STATUS "  - ${module} ${component} pkg-config found.")
    set_component_found(${module} ${component})
  endif()
endmacro()


#
### Macro: find_multi_component
#
# Checks for the given multi configuration component by invoking pkgconfig and then looking up the
# libraries and include directories.
# Extra helper variables may be set to assist finding libraries:
#   ${module}_PATH_SUFFIXES
#
macro(find_multi_component module component pkgconfig library header)

  # FIXME: currently broken, need to update API

  if (NOT WIN32)
     # use pkg-config to get the directories and then use these values
     # in the FIND_PATH() and find_library() calls
     find_package(PkgConfig)
     if (PKG_CONFIG_FOUND)
       pkg_check_modules(PC_${component} ${pkgconfig})
     endif()
  endif()

  find_path(${component}_INCLUDE_DIRS ${header}
    HINTS
      ${PC_LIB${component}_INCLUDEDIR}
      ${PC_LIB${component}_INCLUDE_DIRS}
    PATH_SUFFIXES
      ${${module}_PATH_SUFFIXES}
  )

  # Create a Debug and a Release list for multi configuration builds
  if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    #set(${ALIAS_RELEASE_LIBRARIES} ${ALIAS_RELEASE_LIBRARIES}-NOTFOUND)
    #set(${ALIAS_DEBUG_LIBRARIES} ${ALIAS_DEBUG_LIBRARIES}-NOTFOUND)
    #set(${ALIAS_LIBRARIES})
    find_library(${ALIAS_RELEASE_LIBRARIES}
      NAMES
        lib${library}.a
        ${library}.lib
        ${library}
      HINTS
        ${PC_LIB${component}_LIBDIR}
        ${PC_LIB${component}_LIBRARY_DIRS}
      PATH_SUFFIXES
        lib
        bin
    )
    find_library(${ALIAS_DEBUG_LIBRARIES}
      NAMES
        lib${library}d.a
        ${library}d.lib
        ${library}d
      HINTS
        ${PC_LIB${component}_LIBDIR}
        ${PC_LIB${component}_LIBRARY_DIRS}
      PATH_SUFFIXES
        lib
        bin
    )
    if (${ALIAS_RELEASE_LIBRARIES})
      list(APPEND ${ALIAS_LIBRARIES} "optimized" ${${ALIAS_RELEASE_LIBRARIES}})
    endif()
    if (${ALIAS_DEBUG_LIBRARIES})
      list(APPEND ${ALIAS_LIBRARIES} "debug" ${${ALIAS_DEBUG_LIBRARIES}})
    endif()
  else()
    #set(${ALIAS_LIBRARIES})
    find_library(${ALIAS_LIBRARIES}
      NAMES # setting in order might help overcome find_library bugs :/
        #lib${library}.a
        #${library}.lib
        ${library}
      HINTS
        ${PC_LIB${component}_LIBDIR}
        ${PC_LIB${component}_LIBRARY_DIRS}
      PATH_SUFFIXES
        lib
        bin
    )
  endif()

  #message("find_component =Searching for: ${library}")
  #message("find_component module=${${module}_PATH_SUFFIXES}")
  #message("find_component _PATH_SUFFIXES=${${module}_PATH_SUFFIXES}")
  #message("find_component _INCLUDE_DIRS=${${component}_INCLUDE_DIRS}")
  #message("find_component =lib${library}.a")
  #message("find_component =${${component}_INCLUDE_DIRS}")
  #message("find_component _INCLUDE_DIRS=${${component}_INCLUDE_DIRS}")
  #message("find_component _LIBRARIES=${${ALIAS_LIBRARIES}}")
  #message("${${component}_INCLUDE_DIRS}/lib")
  #message("${PC_LIB${component}_LIBDIR}")
  #message("${PC_LIB${component}_LIBRARY_DIRS}")
  #message("${header}")
  #message("${library}")

  set(${component}_DEFINITIONS  ${PC_${component}_CFLAGS_OTHER} CACHE STRING "The ${component} CFLAGS.")
  set(${component}_VERSION      ${PC_${component}_VERSION}      CACHE STRING "The ${component} version number.")

  set_component_found(${component})

  mark_as_advanced(
    ${component}_INCLUDE_DIRS
    ${ALIAS_LIBRARIES}
    ${ALIAS_DEBUG_LIBRARIES}
    ${ALIAS_RELEASE_LIBRARIES}
    ${component}_DEFINITIONS
    ${component}_VERSION)

endmacro()
