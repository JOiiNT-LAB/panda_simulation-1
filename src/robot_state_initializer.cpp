#include <controller_manager_msgs/SwitchController.h>
#include <ros/ros.h>
#include <std_msgs/Float64MultiArray.h>
#include <gazebo_arm_controllers/PoseRPY.h>

int main(int argc, char **argv) {
  ros::init(argc, argv, "robot_state_initializer_node");
  ros::NodeHandle node_handle;

  // define variables
  std::string cartesian_position_command_topic{"/cartesian_pose_controller/command"},
      controller_manager_switch_topic{"/controller_manager/switch_controller"};
  ros::ServiceClient switch_controller_client =
      node_handle.serviceClient<controller_manager_msgs::SwitchController>(controller_manager_switch_topic);
  ros::Publisher cartesian_position_publisher =
      node_handle.advertise<gazebo_arm_controllers::PoseRPY>(cartesian_position_command_topic, 1);

  // sleep for 2 seconds in order to make sure that the system is up and running
  ros::Duration(2.0).sleep();

  // 1. switch from default joint trajectory controller to custom position controller
  std::string panda_arm_controller{"panda_arm_controller"}, panda_hand_controller{"panda_hand_controller"},
      cartesian_pose_controller{"cartesian_pose_controller"};
  std::vector<std::string> stop_controllers{panda_arm_controller, panda_hand_controller};
  std::vector<std::string> start_controllers{cartesian_pose_controller};

  controller_manager_msgs::SwitchController srv_switch_controller;
  srv_switch_controller.request.stop_controllers = stop_controllers;
  srv_switch_controller.request.start_controllers = start_controllers;
  srv_switch_controller.request.strictness = controller_manager_msgs::SwitchControllerRequest::BEST_EFFORT;

  // housekeeping for logging
  std::ostringstream stream_start_controllers, stream_stop_controllers;
  for (const auto &elem : start_controllers) {
    stream_start_controllers << elem << ", ";
  }
  for (const auto &elem : stop_controllers) {
    stream_stop_controllers << elem << ", ";
  }

  if (switch_controller_client.call(srv_switch_controller)) {
    ROS_INFO_STREAM("Success switching controllers from " << stream_stop_controllers.str() << " to "
                                                          << stream_start_controllers.str());
  } else {
    ROS_WARN_STREAM("Error switching controllers from " << stream_stop_controllers.str() << " to "
                                                        << stream_start_controllers.str());
    return -1;
  }

  // 2. publish the desired joint position to the custom controller
  gazebo_arm_controllers::PoseRPY command_msg;

  command_msg.id                 = 0;
  command_msg.orientation.roll   = 3.1416;
  command_msg.orientation.pitch  = 0.0;
  command_msg.orientation.yaw    = 0.0;
  command_msg.position.x         = 0.3;
  command_msg.position.y         = 0.0;
  command_msg.position.z         = 0.6;

  /*std_msgs::Float64MultiArray command_msg;
  command_msg.data = panda_ready_state;*/

  cartesian_position_publisher.publish(command_msg);

  // sleep for 1 seconds in order to make sure that the controller finishes moving the robot
  ros::Duration(1.0).sleep();

  // 3. Restore default controllers
  /*std::swap(stop_controllers, start_controllers);
  srv_switch_controller.request.stop_controllers = stop_controllers;
  srv_switch_controller.request.start_controllers = start_controllers;

  if (switch_controller_client.call(srv_switch_controller)) {
    ROS_INFO_STREAM("Success switching controllers from " << stream_stop_controllers.str() << " to "
                                                          << stream_start_controllers.str());
  } else {
    ROS_WARN_STREAM("Error switching controllers from " << stream_stop_controllers.str() << " to "
                                                        << stream_start_controllers.str());
    return -1;
  }*/

  ros::spin();
  return 0;
}
