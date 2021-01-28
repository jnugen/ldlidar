#include <iostream>
#include "cmd_interface_linux.hpp"
#include <stdio.h>
#include "lipkg.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "tofbf.hpp"
#include <string>

rclcpp::Node::SharedPtr node;

int main(int argc , char **argv)
{
	rclcpp::init(argc, argv);
	node = rclcpp::Node::make_shared("product");
 	
	LiPkg * lidar = new LiPkg(); // pass raw pointer
  
    CmdInterfaceLinux cmd_port;
    std::vector<std::pair<std::string, std::string> > device_list;
    std::string port_name;
    cmd_port.GetCmdDevices(device_list);
    for (auto n : device_list)
    {
        std::cout << n.first << "    " << n.second << std::endl;
        if(strstr(n.second.c_str(),"CP2102"))
        {
            port_name = n.first;
        }
    }

	if(port_name.empty())
	{
		RCLCPP_ERROR(node->get_logger(), "Can't find LiDAR_LD06");
		return 1;
	}

	RCLCPP_INFO(node->get_logger(), "Found LiDAR_LD06");
	cmd_port.SetReadCallback([&lidar](const char *byte, size_t len) {
		if(lidar->Parse((uint8_t*)byte, len))
		{
			lidar->AssemblePacket();  
		}
	});

	if(cmd_port.Open(port_name))
		RCLCPP_INFO(node->get_logger(), "LiDAR_LD06 started successfully");
	
	auto lidar_pub = node->create_publisher<sensor_msgs::msg::LaserScan>("LiDAR/LD06", 1);
	
	while (rclcpp::ok())
	{
		if (lidar->IsFrameReady())
		{
			lidar_pub->publish(lidar->GetLaserScan());  // Fixed Frame:  lidar_frame
			lidar->ResetFrameReady();
		}
	}

	rclcpp::shutdown();
    return 0;
}
