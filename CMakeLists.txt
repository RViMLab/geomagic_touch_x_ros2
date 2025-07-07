cmake_minimum_required(VERSION 3.8)
project(geomagic_touch_x)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
add_compile_options(-Wno-attributes) # surpress Linux warning for HDCALLBACK

# default to Release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# find cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3)
find_package(geometry_msgs REQUIRED)
find_package(OpenHaptics REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)

# standalone driver
add_library(
  geomagic_touch_x_driver SHARED
  src/device.cpp
)
target_include_directories(geomagic_touch_x_driver
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(geomagic_touch_x_driver
  OpenHaptics::HD
  OpenHaptics::HL
)

# ROS 2 node
add_library(geomagic_touch_x_component SHARED
  src/geomagic_touch_x_component.cpp
)
ament_target_dependencies(geomagic_touch_x_component
  Eigen3
  geometry_msgs
  rclcpp
  rclcpp_components
  sensor_msgs
  tf2
  tf2_ros
)
target_link_libraries(geomagic_touch_x_component
  geomagic_touch_x_driver
)
rclcpp_components_register_node(
    geomagic_touch_x_component
    PLUGIN "geomagic_touch_x::DeviceDriver"
    EXECUTABLE device_driver
)

install(DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_targets(export_geomagic_touch_x_component)
install(TARGETS geomagic_touch_x_driver geomagic_touch_x_component
        EXPORT export_geomagic_touch_x_component
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
