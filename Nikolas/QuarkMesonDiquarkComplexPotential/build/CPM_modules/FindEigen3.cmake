include("/opt/DiFfRG/cmake/CPM.cmake")
CPMAddPackage("NAME;Eigen3;VERSION;3.4.0;URL;https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz;DOWNLOAD_ONLY;YES")
set(Eigen3_FOUND TRUE)