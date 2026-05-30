#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <cmath>
#include <iostream>
#include <thread>

class Talker_quaternion : public rclcpp::Node
{
public:
    Talker_quaternion() : Node("talker_quaternion")
    {
        publisher_ = this->create_publisher<geometry_msgs::msg::Quaternion>("orientation", 10);

        input_thread_ = std::thread([this]()
                                    {
            double roll, pitch, yaw;
            while (rclcpp::ok()) {
                std::cout << "Enter roll pitch yaw (0-180): ";
                std::cin >> roll >> pitch >> yaw;
                
                constexpr double DEG_TO_RAD = M_PI/180;
                roll  = roll * DEG_TO_RAD;
                pitch = pitch * DEG_TO_RAD;
                yaw   = yaw * DEG_TO_RAD;

                tf2::Quaternion q;
                q.setRPY(roll, pitch, yaw);

                geometry_msgs::msg::Quaternion msg = tf2::toMsg(q);

                publisher_->publish(msg);
                RCLCPP_INFO(this->get_logger(),
                    "Published rpy: [%f, %f, %f]  →  q: [w=%.4f, x=%.4f, y=%.4f, z=%.4f]",
                    roll, pitch, yaw, msg.w, msg.x, msg.y, msg.z);
            } });
    }

    ~Talker_quaternion()
    {
        if (input_thread_.joinable())
            input_thread_.join();
    }

private:
    rclcpp::Publisher<geometry_msgs::msg::Quaternion>::SharedPtr publisher_;
    std::thread input_thread_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Talker_quaternion>());
    rclcpp::shutdown();
    return 0;
}