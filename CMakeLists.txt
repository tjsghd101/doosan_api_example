cmake_minimum_required (VERSION 2.8.11)
project (doosan_api_example)

add_definitions(-D__XENO__=true)

set(ENV{XENOMAI_ROOT_DIR} /usr/xenomai)

# Needed Modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FindPkgConfig)
include(${PROJECT_SOURCE_DIR}/cmake-modules/CMakeFunctions.cmake)

# Set policy for LINK_INTERFACE
cmake_policy(SET CMP0022 NEW)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)

#common commands for building c++ executables and libraries
find_package(Xenomai)

include_directories(${PROJECT_SOURCE_DIR}/include)

#https://stackoverflow.com/questions/8774593/cmake-link-to-external-library
# Your-external "mylib", add GLOBAL if the imported library is located in directories above the current.
add_library( drfl SHARED IMPORTED )
# You can define two import-locations: one for debug and one for release.
set_target_properties( drfl PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/library/libDRFL.a )

add_library( poconet SHARED IMPORTED )
set_target_properties( poconet PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/library/libPocoNet.so.62 )

add_library( pocofoundation SHARED IMPORTED )
set_target_properties( pocofoundation PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/library/libPocoFoundation.so.62 )

add_executable (doosan_api_example src/main.cpp)

target_link_libraries(doosan_api_example drfl poconet pocofoundation ${Xenomai_NATIVE_DL_FLAGS})
target_include_directories(doosan_api_example PUBLIC ${Xenomai_NATIVE_INCLUDE_DIRS})
set_target_properties(doosan_api_example PROPERTIES COMPILE_FLAGS ${Xenomai_NATIVE_DEFINITIONS})


add_executable (doosan_api_rt_example src/main_rt.cpp)

target_link_libraries(doosan_api_rt_example drfl poconet pocofoundation ${Xenomai_NATIVE_DL_FLAGS})
target_include_directories(doosan_api_rt_example PUBLIC ${Xenomai_NATIVE_INCLUDE_DIRS})
set_target_properties(doosan_api_rt_example PROPERTIES COMPILE_FLAGS ${Xenomai_NATIVE_DEFINITIONS})
