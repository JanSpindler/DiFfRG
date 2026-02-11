# Install script for directory: /home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmt.so.11.0.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmt.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/libfmt.so.11.0.2"
    "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/libfmt.so.11"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmt.so.11.0.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libfmt.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/libfmt.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fmt" TYPE FILE FILES
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/args.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/base.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/chrono.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/color.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/compile.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/core.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/format.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/format-inl.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/os.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/ostream.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/printf.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/ranges.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/std.h"
    "/home/nikolas/.cache/CPM/fmt/cc9505ea880bebe7edd7464fd9d50f6db1beee73/include/fmt/xchar.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES
    "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/fmt-config.cmake"
    "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/fmt-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake"
         "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/CMakeFiles/Export/b834597d9b1628ff12ae4314c3a2e4b8/fmt-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/fmt.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/fmt-build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
