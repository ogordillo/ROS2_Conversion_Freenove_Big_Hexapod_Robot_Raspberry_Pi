---
layout: default
title:  "Week 5a - 👾 Attack of the Clones!"
date:   2025-09-04
thumbnail: /assets/images/rviz_init.png
---

## If I move, you move, just like that!

#### TL;DR; - I finished modeling joints in Fusion 360, found an up-to-date URDF Exporter, and Launched RViz, Gazebo (kinda!) 

#### Setting up Joints:
I initially was following an older Fusion->URDF exporter script [older Fusion->URDF exporter script](https://github.com/syuntoku14/fusion2urdf) and I overlooked that this script outputs for older ROS versions and uses catkin.

This doesn't work for us because we're on ROS Jazzy and are using colcon, so after succeeding with the older script (and failing at the end), I found [this more recent script](https://github.com/runtimerobotics/fusion360-urdf-ros2). Thankfully, it seems to be a fork of the original script I was using because all the same principles applies. Those being:
- You need to define a 'base-link' component.
- Component2 when defining a joint, must be the parent component.

#### Sharing is Caring:
I put the .stl files from the export on [my google drive](https://drive.google.com/drive/folders/1VZxh9XcH7hnItRFQC2-phcYuDbOMEweY?usp=sharing) for people to download. Simply copy the meshes folder into /src/Hexapod_Robot_description/. 

#### Initial Launch of RViz/Gazebo:
So I haven't bridged over ROS topics to Gazebo or RViz yet. But you can see the model and it's joints by running:
```
docker-compose up windows_command
```

On a diff terminal:
```
docker exec -it windows_command bash
source /opt/ros/jazzy/setup.py
source install/setup.py # There is currently something unconfigured that we need to source this again
```

##### RViz launch cmd
```
ros2 launch Hexapod_Robot_description display.launch.py
```
![rviz-init-img]({{ '/assets/images/rviz_init.png' | relative_url }})

###### Something of note here:
I see the Joints using RViz's joint-state-publisher-gui are init at 0. 

But I modeled the robot at 0,0,90 - hip, hip, knee respectively (side note: I've been avoiding using spider anatomy terminology but maybe it'll help... meh, we'll see.)

My concern is the init position might be a pain in the but if I don't nail it down early and get ROS, visualization tools, and my models to all agree. 

##### Gazebo launch cmd
```
ros2 launch Hexapod_Robot_description gazebo.launch.py
```
![gazebo-init-img]({{ '/assets/images/gazebo_init.png' | relative_url }})

As you can see, there is an error msg with the Leg*_Femur links having invalid inertia.
Clearly, there was some setup skipped and we'll just have to go back and iron out these missing details! 😛

### Next Steps
I still need to model the RealSense Depth sensor mount and print it or laser cut it.
We also need to fix the Gazebo bug about inertia (and probably a lot more setup)
and then bridge ROS topics into Gazebo. 


