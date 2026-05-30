cmake_minimum_required(VERSION 3.20)
project(vr_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(talker_euler src/talker_euler.cpp)
ament_target_dependencies(talker_euler rclcpp geometry_msgs)

add_executable(talker_quaternion src/talker_quaternion.cpp)
ament_target_dependencies(talker_quaternion rclcpp geometry_msgs tf2 tf2_ros tf2_geometry_msgs)

add_executable(listener_serial src/listener_serial.cpp)
ament_target_dependencies(listener_serial rclcpp geometry_msgs)
target_link_libraries(listener_serial ${Boost_LIBRARIES})

add_executable(listener_parallel src/listener_parallel.cpp)
ament_target_dependencies(listener_parallel rclcpp geometry_msgs tf2 tf2_ros tf2_geometry_msgs)
target_link_libraries(listener_parallel ${Boost_LIBRARIES} tf2_geometry_msgs::tf2_geometry_msgs)

install(TARGETS talker_euler talker_quaternion listener_serial listener_parallel
    DESTINATION lib/${PROJECT_NAME})

# Python scripts installed separately
install(PROGRAMS
    src/talker_quaternion_gui.py
    DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()