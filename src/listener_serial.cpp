#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <boost/asio.hpp>
#include <string>
#include <algorithm>

class Listener_serial : public rclcpp::Node
{
public:
    Listener_serial() : Node("listener_serial"), io_(), serial_(io_)
    {
        // Open serial port
        serial_.open("/dev/ttyACM0"); // Change to your Arduino's port
        serial_.set_option(boost::asio::serial_port_base::baud_rate(9600));

        subscription_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "euler_angles", 10,
            [this](const geometry_msgs::msg::Vector3::SharedPtr msg)
            {
                RCLCPP_INFO(this->get_logger(), "Heard roll: %.2f  pitch: %.2f  yaw: %.2f",
                            msg->x, msg->y, msg->z);
                // Format and send over serial (4 bytes: roll, pitch, yaw, 181)
                uint8_t data[4];
                data[0] = (uint8_t)std::clamp((int)msg->x, 0, 180); // roll
                data[1] = (uint8_t)std::clamp((int)msg->y, 0, 180); // pitch
                data[2] = (uint8_t)std::clamp((int)msg->z, 0, 180); // yaw
                data[3] = 181;                                      // end byte
                boost::asio::write(serial_, boost::asio::buffer(data, 4));
            });
    }

private:
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr subscription_;
    boost::asio::io_service io_;
    boost::asio::serial_port serial_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Listener_serial>());
    rclcpp::shutdown();
    return 0;
}