#=============================================================================
# Copyright (c) 2026, NVIDIA CORPORATION.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================


# CPM Search for spdlog
#
# Make sure we search for a build-dir config module for the CPM project
set(possible_package_dir "/home/nikolas/masterthesis/DiFfRG/Nikolas/QuarkMesonDiquarkComplexPotential/build/_deps/spdlog-build")
if(possible_package_dir AND NOT DEFINED spdlog_DIR)
  set(spdlog_DIR "${possible_package_dir}")
endif()

CPMFindPackage(
  "NAME;spdlog;VERSION;1.14.1;GIT_REPOSITORY;https://github.com/gabime/spdlog.git;GIT_TAG;v1.14.1;GIT_SHALLOW;ON;EXCLUDE_FROM_ALL;OFF;OPTIONS;SPDLOG_INSTALL ON;SPDLOG_FMT_EXTERNAL_HO ON"
  )

if(possible_package_dir)
  unset(possible_package_dir)
endif()
#=============================================================================
