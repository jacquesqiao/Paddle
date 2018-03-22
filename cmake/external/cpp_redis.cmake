# Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#IF(MOBILE_INFERENCE OR NOT WITH_DISTRIBUTE)
IF(MOBILE_INFERENCE)
  return()
ENDIF()

include (ExternalProject)

SET(CPP_REDIS_SOURCES_DIR ${THIRD_PARTY_PATH}/cpp_redis)
SET(CPP_REDIS_INSTALL_DIR ${THIRD_PARTY_PATH}/install/cpp_redis)
SET(CPP_REDIS_INCLUDE_DIR "${CPP_REDIS_INSTALL_DIR}/include/" CACHE PATH "cpp_redis include directory." FORCE)

include_directories(${CPP_REDIS_SOURCES_DIR}/src/extern_cpp_redis/include)

ExternalProject_Add(
        extern_cpp_redis
        ${EXTERNAL_PROJECT_LOG_ARGS}
        GIT_REPOSITORY    "https://github.com/Cylix/cpp_redis.git"
        GIT_TAG           "4.3.1"
        PREFIX            "${CPP_REDIS_SOURCES_DIR}"
        GIT_SUBMODULES    "tacopie"
        UPDATE_COMMAND    ""
        CMAKE_ARGS      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                        -DCMAKE_INSTALL_PREFIX=${CPP_REDIS_INSTALL_DIR}
                        -DCMAKE_INSTALL_LIBDIR=${CPP_REDIS_INSTALL_DIR}/lib
                        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                        -DBUILD_TESTING=OFF
                        -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE}
                        ${EXTERNAL_OPTIONAL_ARGS}
        CMAKE_CACHE_ARGS  -DCMAKE_INSTALL_PREFIX:PATH=${CPP_REDIS_INSTALL_DIR}
                          -DCMAKE_INSTALL_LIBDIR:PATH=${CPP_REDIS_INSTALL_DIR}/lib
                          -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                          -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
        BUILD_COMMAND     make -j4
        INSTALL_COMMAND make install
)

add_library(cpp_redis STATIC IMPORTED GLOBAL)
set_property(TARGET cpp_redis PROPERTY IMPORTED_LOCATION
        "${CPP_REDIS_INSTALL_DIR}/lib/libcpp_redis.a")

add_library(tacopie STATIC IMPORTED GLOBAL)
set_property(TARGET tacopie PROPERTY IMPORTED_LOCATION
        "${CPP_REDIS_INSTALL_DIR}/lib/libtacopie.a")

include_directories(${CPP_REDIS_INCLUDE_DIR})
add_dependencies(cpp_redis extern_cpp_redis)