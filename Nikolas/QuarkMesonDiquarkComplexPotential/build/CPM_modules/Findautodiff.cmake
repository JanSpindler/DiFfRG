include("/opt/DiFfRG/cmake/CPM.cmake")
CPMAddPackage("NAME;autodiff;GITHUB_REPOSITORY;autodiff/autodiff;GIT_TAG;v1.1.0;PATCHES;autodiff.patch;OPTIONS;AUTODIFF_BUILD_TESTS OFF;AUTODIFF_BUILD_EXAMPLES OFF;AUTODIFF_BUILD_DOCS OFF;AUTODIFF_BUILD_PYTHON OFF;Eigen3_DIR /home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/eigen3-build")
set(autodiff_FOUND TRUE)