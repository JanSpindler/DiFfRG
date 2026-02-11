include("/opt/DiFfRG/cmake/CPM.cmake")
CPMAddPackage("NAME;thread-pool;GITHUB_REPOSITORY;bshoshany/thread-pool;VERSION;5.0.0;DOWNLOAD_ONLY;YES")
set(thread-pool_FOUND TRUE)