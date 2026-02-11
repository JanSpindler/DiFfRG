include("/opt/DiFfRG/cmake/CPM.cmake")
CPMAddPackage("NAME;rmm;VERSION;25.02.00a;GITHUB_REPOSITORY;rapidsai/rmm;SYSTEM;Off;OPTIONS;BUILD_TESTS OFF")
set(rmm_FOUND TRUE)