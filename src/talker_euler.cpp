#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <iostream>
#include <thread>

class Talker_euler : public rclcpp::Node {
public:
    Talker_euler() : Node("talker_euler") {
        publisher_ = this->create_publisher<geometry_msgs::msg::Vector3>("euler_angles", 10);

        input_thread_ = std::thread([this]() {
            int roll, pitch, yaw;
            while (rclcpp::ok()) {
                std::cout << "Enter roll pitch yaw (0-180): ";
                std::cin >> roll >> pitch >> yaw;

                roll  = std::clamp(roll,  0, 180);
                pitch = std::clamp(pitch, 0, 180);
                yaw   = std::clamp(yaw,   0, 180);

                auto msg = geometry_msgs::msg::Vector3();
                msg.x = roll;
                msg.y = pitch;
                msg.z = yaw;
                publisher_->publish(msg);
                RCLCPP_INFO(this->get_logger(), "Published roll: %d  pitch: %d  yaw: %d", roll, pitch, yaw);
            }
        });
    }

    ~Talker_euler() {
        if (input_thread_.joinable())
            input_thread_.join();
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr publisher_;
    std::thread input_thread_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Talker_euler>());
    rclcpp::shutdown();
    return 0;
}