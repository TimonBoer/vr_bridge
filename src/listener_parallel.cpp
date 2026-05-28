#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <boost/asio.hpp>
#include <string>
#include <algorithm>
#pragma once
#include <cmath>

// ── Neck geometry (millimetres) ──────────────────────────────────────────────
constexpr double VERTEBRA_RADIUS     = 52.0;  // radius of the curve of a vertebra
constexpr double VERTEBRA_HEIGHT     = 7.0;   // height of a vertebra
constexpr double CENTER_RADIUS       = 5.0;   // radius of the central column
constexpr double ROPE_OFFSET         = 20.0;  // rope attachment distance from disc centre
constexpr int    N_VERTEBRAE         = 6;     // number of vertebrae in the neck
constexpr double SPINDLE_RADIUS      = 20.0;  // motor spindle radius (converts length → angle)

constexpr double NEUTRAL_ROPE_LENGTH = VERTEBRA_HEIGHT * 2.0 * N_VERTEBRAE;

class Rope {
public:
    // ── Public interface ─────────────────────────────────────────────────────

    /**
     * @param angular_pos         Angular position of this rope around the neck (rad).
     * @param motor_neutral_angle Motor angle (deg) that corresponds to neutral posture.
     */
    Rope(double angular_pos, int motor_neutral_angle)
        : angular_pos_(angular_pos)
        , motor_neutral_angle_(motor_neutral_angle)
        , angle_from_curve_centre_(0.0)
        , dist_from_curve_centre_(0.0)
        , rope_length_(0.0)
        , angle_offset_(0.0)
        , motor_angle_(motor_neutral_angle)
    {
        update(0.0, 0.0);
    }

    /** Recompute rope length and motor angle for the given neck pose.
     * @param tilt_angle  Neck tilt away from vertical (rad).
     * @param pan_angle   Neck pan / rotation around vertical axis (rad).
     */
    void update(double tilt_angle, double pan_angle) {
        updateRopeGeometry(pan_angle);
        updateRopeLength(tilt_angle);
        updateMotorAngle();
    }

    /** Return the motor angle (deg) required for the current pose. */
    double getMotorAngle() const { return motor_angle_; }

    // Accessors for derived geometry (mirrors Python's public attributes)
    double getAngleFromCurveCentre() const { return angle_from_curve_centre_; }
    double getDistFromCurveCentre()  const { return dist_from_curve_centre_; }
    double getRopeLength()           const { return rope_length_; }
    double getAngleOffset()          const { return angle_offset_; }

private:
    // ── Private helpers ──────────────────────────────────────────────────────

    /** Compute the rope attachment point relative to the disc centre. */
    void updateRopeGeometry(double pan_angle) {
        double x = ROPE_OFFSET * std::cos(pan_angle + angular_pos_) + CENTER_RADIUS;
        double y = VERTEBRA_RADIUS - VERTEBRA_HEIGHT;

        dist_from_curve_centre_  = std::sqrt(x * x + y * y);
        angle_from_curve_centre_ = -std::atan2(x, y);
    }

    /** Compute total rope length from disc geometry and neck tilt. */
    void updateRopeLength(double tilt_angle) {
        double tilt_per_vertebra   = tilt_angle / N_VERTEBRAE;
        double projected_offset    = dist_from_curve_centre_ * std::cos(angle_from_curve_centre_ - tilt_per_vertebra);
        double length_per_vertebra = VERTEBRA_RADIUS - projected_offset;
        rope_length_               = length_per_vertebra * 2.0 * N_VERTEBRAE;
    }

    /** Convert the change in rope length to a motor angle offset (deg). */
    void updateMotorAngle() {
        constexpr double RAD_TO_DEG = 180.0 / M_PI;
        double length_delta  = NEUTRAL_ROPE_LENGTH - rope_length_;
        double offset_rad    = length_delta / SPINDLE_RADIUS;
        angle_offset_        = offset_rad * RAD_TO_DEG;
        motor_angle_         = static_cast<int>(motor_neutral_angle_ + angle_offset_);
    }

    // ── Members ──────────────────────────────────────────────────────────────
    double angular_pos_;
    int    motor_neutral_angle_;

    double angle_from_curve_centre_;
    double dist_from_curve_centre_;
    double rope_length_;
    double angle_offset_;
    double motor_angle_;
};

class Listener : public rclcpp::Node
{
public:
    Listener() : Node("listener"), io_(), serial_(io_)
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
    rclcpp::spin(std::make_shared<Listener>());
    rclcpp::shutdown();
    return 0;
}