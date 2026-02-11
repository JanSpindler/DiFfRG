cmake_minimum_required(VERSION 3.26.4)

project(QuarkMesonDiquarkLPA)

find_package(DiFfRG REQUIRED HINTS /opt/DiFfRG/)

add_executable(QuarkMesonDiquarkLPA QMD.cc)
setup_application(QuarkMesonDiquarkLPA)

execute_process(
  COMMAND
    ${CMAKE_COMMAND} "-E" "create_symlink"
    "${CMAKE_CURRENT_SOURCE_DIR}/parameter.json"
    "${CMAKE_CURRENT_BINARY_DIR}/parameter.json")