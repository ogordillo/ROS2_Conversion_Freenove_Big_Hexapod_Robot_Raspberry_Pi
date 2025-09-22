import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
from .sim_imu_filter import SimIMUFilter # Import the class from step 1

class GazeboIMUFilterNode(Node):
    def __init__(self):
        super().__init__('gazebo_imu_filter_node')
        
        self.subscription = self.create_subscription(
            Imu,
            '/imu/data',
            self.imu_callback,
            10)
            
        self.publisher = self.create_publisher(Imu, '/imu/data_sim', 10)
        
        self.imu_filter = SimIMUFilter()
        self.get_logger().info('Gazebo IMU Filter Node has started.')

    def imu_callback(self, msg: Imu):

        accel_data = {
            'x': msg.linear_acceleration.x,
            'y': msg.linear_acceleration.y,
            'z': msg.linear_acceleration.z
        }
        gyro_data = {
            'x': msg.angular_velocity.x,
            'y': msg.angular_velocity.y,
            'z': msg.angular_velocity.z
        }
        
        filtered_data = self.imu_filter.update_imu_state(accel_data, gyro_data)
        
        filtered_msg = Imu()
        filtered_msg.header.stamp = self.get_clock().now().to_msg()
        filtered_msg.header.frame_id = 'imu_link'
        
        q = filtered_data['orientation']
        filtered_msg.orientation.w = q['w']
        filtered_msg.orientation.x = q['x']
        filtered_msg.orientation.y = q['y']
        filtered_msg.orientation.z = q['z']

        la = filtered_data['linear_acceleration']
        filtered_msg.linear_acceleration.x = la['x']
        filtered_msg.linear_acceleration.y = la['y']
        filtered_msg.linear_acceleration.z = la['z']
        
        av = filtered_data['angular_velocity']
        filtered_msg.angular_velocity.x = av['x']
        filtered_msg.angular_velocity.y = av['y']
        filtered_msg.angular_velocity.z = av['z']
        
        self.publisher.publish(filtered_msg)

def main(args=None):
    rclpy.init(args=args)
    node = GazeboIMUFilterNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()