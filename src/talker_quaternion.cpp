#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <cmath>
#include <iostream>
#include <thread>

// Converts extrinsic ZYX Euler angles (degrees) to a quaternion
geometry_msgs::msg::Quaternion eulerToQuaternion(double roll_deg, double pitch_deg, double yaw_deg) {
    constexpr double DEG_TO_RAD = M_PI / 180.0;
    double roll  = roll_deg  * DEG_TO_RAD;
    double pitch = pitch_deg * DEG_TO_RAD;
    double yaw   = yaw_deg   * DEG_TO_RAD;

    double cr = std::cos(roll  * 0.5),  sr = std::sin(roll  * 0.5);
    double cp = std::cos(pitch * 0.5),  sp = std::sin(pitch * 0.5);
    double cy = std::cos(yaw   * 0.5),  sy = std::sin(yaw   * 0.5);

    geometry_msgs::msg::Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

class Talker_quaternion : public rclcpp::Node {
public:
    Talker_quaternion() : Node("talker_quaternion") {
        publisher_ = this->create_publisher<geometry_msgs::msg::Quaternion>("orientation", 10);

        input_thread_ = std::thread([this]() {
            int roll, pitch, yaw;
            while (rclcpp::ok()) {
                std::cout << "Enter roll pitch yaw (0-180): ";
                std::cin >> roll >> pitch >> yaw;

                roll  = std::clamp(roll,  0, 180);
                pitch = std::clamp(pitch, 0, 180);
                yaw   = std::clamp(yaw,   0, 180);

                auto msg = eulerToQuaternion(roll, pitch, yaw);
                publisher_->publish(msg);
                RCLCPP_INFO(this->get_logger(),
                    "Published rpy: [%d, %d, %d]  →  q: [w=%.4f, x=%.4f, y=%.4f, z=%.4f]",
                    roll, pitch, yaw, msg.w, msg.x, msg.y, msg.z);
            }
        });
    }

    ~Talker_quaternion() {
        if (input_thread_.joinable())
            input_thread_.join();
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Quaternion>::SharedPtr publisher_;
    std::thread input_thread_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Talker_quaternion>());
    rclcpp::shutdown();
    return 0;
}