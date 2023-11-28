cmake_minimum_required(VERSION 3.22)
project(clitester
  DESCRIPTION "A standalone binary to exercize ipfs_client in a non-Chromium way"
  LANGUAGES CXX
)

file(COPY_FILE
    "${CMAKE_CURRENT_LIST_DIR}/conanfile.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/conanfile.txt"
)
execute_process(
    COMMAND conan install --output-folder=${CMAKE_CURRENT_BINARY_DIR} --build=missing --settings build_type=${CMAKE_BUILD_TYPE} "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
execute_process(
    COMMAND ls -lrth
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
message(WARNING "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR} CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR} CMAKE_MODULE_PATH='${CMAKE_MODULE_PATH}'")
find_package(ipfs_client
    REQUIRED
    CONFIG
)
#PATHS "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_BINARY_DIR}"

file(GLOB srcs  CONFIGURE_DEPENDS src/*.cc)
add_executable(clitester
  ${srcs}
)
target_compile_features(clitester
    PUBLIC
        cxx_std_20
)
target_link_libraries(clitester
    PUBLIC
        ipfs_client::ipfs_client
)
