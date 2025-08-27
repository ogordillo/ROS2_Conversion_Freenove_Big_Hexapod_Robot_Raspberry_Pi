---
layout: default
title:  "Week 3b - 🌊 Setting up the Stream"
date:   2025-08-20
thumbnail: /assets/images/init-depth-img.png
---

## 'That came out of Middleware!' (aka DDS Hell)

#### TL;DR; - Setting up the depth stream on WiFi from Pi5 to Laptop was a bit more involved. Eventually it was resolved by choosing Zenoh middleware. 

<iframe
  src="https://www.youtube.com/embed/gR9VG59Kh1E"
  title="YouTube video player"
  frameborder="0"
  allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
  allowfullscreen>
</iframe>

#### Setting up the Stream:
So in my mind, I envisioned setting up the wifi stream from the pi to my laptop would be trivial. Maybe I'd set an ip address here, and maybe it would even involve designating a port. I was in for a ride. 

#### If things were simple
Actually, in a less than ideal setup, it actually is as easy as I was expecting it to be. What I mean is, if we were developing straight onto a Pi with no remote connections, there wouldn't even be a need to broadcast a network stream because all the nodes would be running locally. If we were on a linux machine connecting directly to the pi, that would be trivial too because there is a direct path between two 'physical' ports.

#### We're Bougie tho
But do I have an ideal setup? Hah! Why would I crawl around on the ground and make a mess on my user profile. I'm exaggerating a bit but what I mean and what I've already designed in my mind, nay, solidified in my soul; is that I want to containerize this whole project to create a reproducible development workspace. Because what good is sharing a project on Github if the user has to reconfigure it for every situation? 

#### My Setup: 
Windows -> VSCode (WSL2) -> Docker -> ROS Jazzy

Windows -> VSCode (SSH) -> Raspberry Pi -> Docker -> ROS Jazzy

Granted my setup is a wee-bit unnecessary, and really tailored to me using VSCode to SSH into the pi, and VScode for WSL on my laptop. But now-a-days, VSCode rules, albeit I recently had an affair with Vim +Plugins... *clears throat* Anyways, here is the layout - and you can see why I thought this would be simple pimple, easy breezy but there is a significant amount of time spent setting up and trial and error for each component in the chain:

#### The struggle with Middlewares
Below I note how to set things up the right way. I want to make it clear that I spent two whole days between middlewares. I had to resort to all kinds of fun (nostalgic?) network tools like netcat, wireshark, ipconfig, ifconfig, ip addr, etc. To investigate the communication issues and determine where I was in solving the problem. The initial config was using CycloneDDS, but there was zero between my nodes, then I tried FastDDS, and I managed to configure that such that UDP was streaming between both machine/docker containers but the discovery part was missing. This was arguably worse (like chasing shadows). You know it's bad when the problem is un-google-able. The signature is so unique to my config that it felt like I was the only one on earth with this exact problem. In an almost last-ditch effort, I tried one more Middleware, Zenoh. This one proved to be the right (although high maintenance) solution for my network. Okay, enough trauma bonding, enjoy the solution 😛

#### Windows -> VSCode (WSL2)
##### We have to setup up Firewall Rules to allow inbound UDP from the Pi. On a Powershell:
```
# Figure out your adapter name
Get-NetAdapter

# Create a new virtual switch
New-VMSwitch -SwitchName "WSLBridge" -NetAdapterName "Wi-Fi" -AllowManagementOS $true

# Set it in a variable
$AdapterName = "vEthernet (WSLBridge)"

# Create a Firewall Rule
New-NetFirewallRule -DisplayName "Allow All WSL2 Inbound" -Direction Inbound -Action Allow -RemoteAddress Any -LocalAddress $wslIP
```

##### Update your .wslconfig file to the following. It's in your %Userprofile%:
```
[wsl2]
networkingMode=bridged
vmSwitch=WSLBridge
```

#### VSCode (WSL2) -> Docker
There is nothing to be done here. It is handeld by the WSLBridge we created.

#### Docker -> ROS Jazzy
##### In docker-compose.yml: We want to set the neccessary ROS and Zenoh parameters so you can be discovered by the discovery server.
```
environment:
- ROS_DOMAIN_ID=0
- RMW_IMPLEMENTATION=rmw_zenoh_cpp
- ZENOH_SESSION_CONFIG_URI=/root/config/client.yaml
volume:
- ./config/client_config.yaml:/root/config/client.yaml
```

##### The client_config.yaml file looks like this:
```
connect:
  endpoints:
    - udp/10.0.0.224:7447
    - tcp/10.0.0.224:7447
```

##### In the dockerfile.wsl we want to append to the apt-get install the following package:
```
ros-jazzy-rmw-zenoh-cpp \
```

#### Windows -> VSCode (SSH) -> Raspberry Pi -> Docker
There is nothing to be done here either. Use RemoteSSH VSCode extension to connect to your Pi's IP

##### Docker -> ROS Jazzy: For this we need to setup a Discovery Server and a Client for our onboard Node. The main point here is that we're using a router_config.yaml file to set up discovery information. We're also getting Zenoh from them directly
```
services:
  zenoh_router:
    image: eclipse/zenoh:1.5.0-21-gb222944cb
    container_name: zenoh_router
    network_mode: host
    volumes:
      - ./config/router_config.yaml:/etc/zenoh/router.yaml:ro
    command: --config /etc/zenoh/router.yaml

  pi_control:
    depends_on:
      - zenoh_router
    network_mode: host
    environment:
      - ROS_DOMAIN_ID=0
      - RMW_IMPLEMENTATION=rmw_zenoh_cpp
    volumes:
      - ./config/realsense_params.yaml:/root/ros2_ws/install/pi_control/share/pi_control/config/realsense_params.yaml
    command: ["bash", "-c", "source /opt/ros/jazzy/setup.bash && colcon build && source install/setup.bash && ros2 launch pi_control robot_launch.py"]
```

##### The router_config.yaml file: The compression helps keep network saturation down. A good indicator the network is saturated is the VSCode SSH client becomes unresponsive (behaves as interrupted). Lowering the resolution of the streams helps too obviously.
```
listen:
  endpoints:
    - udp/[::]:7447
    - tcp/[::]:7447

transport:
  unicast: 
    compression: 
        enabled: true
```


### Next Steps:
Now that I know my project design (streaming the data to a remote machine for computation) is feasible, I can move back into Digital Twinning. Which is designing the parts for the Realsense sensor and battery pack and Adding joints to the model in fusion for simulation. After that, it's about setting up RViz2 and working on kinematics and sensors.
