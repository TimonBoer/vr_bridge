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
target_link_libraries(talker_euler PUBLIC
    rclcpp::rclcpp
    ${geometry_msgs_TARGETS})

add_executable(talker_quaternion src/talker_quaternion.cpp)
target_link_libraries(talker_quaternion PUBLIC
    rclcpp::rclcpp
    ${geometry_msgs_TARGETS}
    tf2::tf2
    tf2_ros::tf2_ros
    tf2_geometry_msgs::tf2_geometry_msgs)

add_executable(listener_serial src/listener_serial.cpp)
target_link_libraries(listener_serial PUBLIC
    rclcpp::rclcpp
    ${geometry_msgs_TARGETS}
    ${Boost_LIBRARIES})

add_executable(listener_parallel src/listener_parallel.cpp)
target_link_libraries(listener_parallel PUBLIC
    rclcpp::rclcpp
    ${geometry_msgs_TARGETS}
    tf2::tf2
    tf2_ros::tf2_ros
    tf2_geometry_msgs::tf2_geometry_msgs
    ${Boost_LIBRARIES})

install(TARGETS talker_euler talker_quaternion listener_serial listener_parallel
    DESTINATION lib/${PROJECT_NAME})

# Python scripts installed separately
install(PROGRAMS
    src/talker_gui_rpy.py
    src/talker_gui_tilt_pan_yaw.py
    src/talker_movella_dot.py
    DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()