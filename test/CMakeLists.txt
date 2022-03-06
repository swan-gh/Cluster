cmake_minimum_required (VERSION 3.21)

project (ClusterTests)

add_subdirectory(gtest)

add_executable(cluster_array_test ClusterArray.cpp)

target_include_directories(cluster_array_test PUBLIC "${gtest_SOURCE_DIR}/include")
target_link_libraries(cluster_array_test gtest)
target_link_libraries(cluster_array_test gtest_main)

add_executable(cluster_map_test ClusterMap.cpp)

message(STATUS "${gtest_BINARY_DIR}/libgtest.a")
target_include_directories(cluster_map_test PUBLIC "${gtest_SOURCE_DIR}/include")
target_link_libraries(cluster_map_test gtest)
target_link_libraries(cluster_map_test gtest_main)