include("/opt/DiFfRG/cmake/CPM.cmake")
CPMAddPackage("NAME;CCCL;GITHUB_REPOSITORY;nvidia/cccl;GIT_TAG;v2.7.0")
set(CCCL_FOUND TRUE)