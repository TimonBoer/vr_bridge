#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import QuaternionStamped

import numpy as np
from scipy.spatial.transform import Rotation


class ImuOrientationSubscriber(Node):

    def __init__(self):
        super().__init__('imu_orientation_subscriber')
        self.subscription = self.create_subscription(
            QuaternionStamped,
            'imu/orientation',
            self.listener_callback,
            10
        )

    def listener_callback(self, msg):
        q = msg.quaternion
        # scipy expects [x, y, z, w]
        q_list = [q.x, q.y, q.z, q.w]
        tilt, pan, yaw = self.quaternion_to_tilt_pan_yaw(q_list)
        self.get_logger().info(
            f'tilt={np.degrees(tilt):.2f}°  '
            f'pan={np.degrees(pan):.2f}°  '
            f'yaw={np.degrees(yaw):.2f}°'
        )

    def quaternion_to_tilt_pan_yaw(self, q):
        """
        Convert a quaternion [x, y, z, w] into (tilt, pan, yaw) angles in radians.
        """
        rot = Rotation.from_quat(q)

        # get normal vector by applying rotation to vector pointing up (0, 0, 1)
        v_norm = rot.apply([0.0, 0.0, 1.0])
        tilt = np.clip(np.arccos(np.clip(v_norm[2], -1.0, 1.0)), 0.0, np.pi / 2)
        pan = -np.arctan2(v_norm[0], v_norm[1]) + np.pi

        # get forward vector by applying rotation to the vector pointing forward (1, 0, 0)
        v_forward = rot.apply([1.0, 0.0, 0.0])

        # get rotation of only the tilt and pan, having 0 yaw rotation:
        # the minimal rotation that takes "up" onto v_norm
        tilt_pan_rot, _ = Rotation.align_vectors([v_norm], [[0.0, 0.0, 1.0]])

        # rotating vector (1, 0, 0) results in the vector with 0 yaw
        expected_forward = tilt_pan_rot.apply([1.0, 0.0, 0.0])

        # calculate angle between expected forward and actual forward vector
        yaw = np.arccos(np.clip(np.dot(v_forward, expected_forward), -1.0, 1.0))

        # determine the sign of the yaw angle by checking the direction of the cross product
        cross = np.cross(expected_forward, v_forward)
        if np.dot(cross, v_norm) < 0:
            yaw = -yaw

        return [float(tilt), float(pan), float(yaw)]


def main(args=None):
    rclpy.init(args=args)
    node = ImuOrientationSubscriber()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()