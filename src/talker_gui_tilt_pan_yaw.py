#!/usr/bin/env python3
import tkinter as tk
import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import QuaternionStamped
import threading
from scipy.spatial.transform import Rotation
import numpy as np

class SliderPublisher(Node):
    def __init__(self):
        super().__init__('talker_quaternion')
        self.pub = self.create_publisher(QuaternionStamped, 'orientation', 10)

    def publish_rpy(self, tilt_deg, pan_deg, yaw_deg):
        t = math.radians(tilt_deg)
        p = math.radians(pan_deg)
        y = math.radians(yaw_deg)

        q = self.tilt_pan_yaw_to_quaternion(t, p, y)

        msg = QuaternionStamped()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'base_link'
        msg.quaternion.x = q[0]
        msg.quaternion.y = q[1]
        msg.quaternion.z = q[2]
        msg.quaternion.w = q[3]

        self.pub.publish(msg)
        self.get_logger().info(
            f'RPY: [{tilt_deg:.1f}, {pan_deg:.1f}, {yaw_deg:.1f}] → '
            f'q: [w={msg.quaternion.w:.4f}, x={msg.quaternion.x:.4f}, '
            f'y={msg.quaternion.y:.4f}, z={msg.quaternion.z:.4f}]'
        )
    
    def tilt_pan_yaw_to_quaternion(self, tilt: float, pan: float, yaw: float) -> np.ndarray:
        # 1. Pan: rotate around Z to aim in the right XY direction
        r_pan = Rotation.from_euler('z', pan)

        # 2. Tilt: rotate around the (now-panned) X axis to lean away from Z
        r_tilt = Rotation.from_euler('x', tilt)

        # 3. Yaw: rotate around the resulting normal vector (back-applied Z)
        r_yaw = Rotation.from_euler('z', yaw - pan)

        # Compose: first pan, then tilt, then yaw (intrinsic, right-to-left)
        r = r_pan * r_tilt * r_yaw

        return r.as_quat()  # returns [x, y, z, w]

def main():
    rclpy.init()
    node = SliderPublisher()

    ros_thread = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
    ros_thread.start()

    root = tk.Tk()
    root.title("Quaternion Publisher")
    root.resizable(False, False)

    sliders = {}
    for i, axis in enumerate(['tilt', 'pan', 'Yaw']):
        tk.Label(root, text=axis, width=6, anchor='w').grid(row=i, column=0, padx=10, pady=8)
        var = tk.DoubleVar(value=0.0)
        tk.Scale(root, from_=-180, to=180, resolution=1,
                 orient=tk.HORIZONTAL, length=300, variable=var).grid(row=i, column=1, padx=10)
        val_label = tk.Label(root, text="0°", width=5)
        val_label.grid(row=i, column=2, padx=5)
        sliders[axis] = (var, val_label)

    def on_slider_change():
        r = sliders['tilt'][0].get()
        p = sliders['pan'][0].get()
        y = sliders['Yaw'][0].get()
        for axis, val in zip(['tilt', 'pan', 'Yaw'], [r, p, y]):
            sliders[axis][1].config(text=f"{val:.0f}°")
        node.publish_rpy(r, p, y)

    for axis, (var, _) in sliders.items():
        var.trace_add('write', lambda *_: on_slider_change())

    tk.Button(root, text="Publish", command=on_slider_change,
              bg='#4CAF50', fg='white', padx=20).grid(row=3, column=1, pady=12)

    root.protocol("WM_DELETE_WINDOW", lambda: (rclpy.shutdown(), root.destroy()))
    root.mainloop()

if __name__ == '__main__':
    main()