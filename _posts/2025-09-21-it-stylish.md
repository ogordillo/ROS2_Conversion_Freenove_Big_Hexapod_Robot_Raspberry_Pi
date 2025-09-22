---
layout: default
title:  "Week 6b - 👁️ Depth Sensor Mounted"
date:   2025-09-21
thumbnail: /assets/images/depth_mount.png
---

#### TL;DR; - I mounted the RealSense Depth Sensor onto the Hexapod. I refactored the command application. Implemented a Kalman filter on the Gazebo side to simulate the current pub/sub architecture. The Hexapod in Gazebo got a makeover 

#### Mounting the Depth Sensor
This turned out to be pretty simple. I drilled holes into the corners of the acrylic ultra sonic sensor mount part and viola✨ 

![depth sensor mount]({{ '/assets/images/depth_mount.png' | relative_url }})

#### Kalman Filter
The current control system on the raspberry pi use a Kalman filter on the accelerometer data from the IMU before publishing the topic on the network. So the idea is to mimic the same behavior on the Gazebo side. 

#### Gazebo-ROS bridge parameters
Turns out the way to do this is to sub to the topic, and pub a modified topic. In doing this, I learned a bit about configuring the gazebo-ros bridge using arguments (as opposed to a yaml file). 

In the following snippet, in arguments, the pattern is as follows:

"GZ_TOPIC @ ROS_MESSAGE_TYPE <DIRECTION> GZ_MESSAGE_TYPE"

Where
- @ is a separator

and DIRECTION is either 
- [ gazebo to ros
- ] ros to gazebo
- { bidirectional

```
bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/imu/data@sensor_msgs/msg/Imu[gz.msgs.IMU",
            "/imu/data_sim@sensor_msgs/msg/Imu[gz.msgs.IMU"
        ],
        output='screen',
        parameters=[
                {'use_sim_time': True}
            ]
    )
```

#### Method to understanding the Kinematics
The last thing I want to do, and hope you don't either, is just copy and paste the inverse kinematics into your project without understanding how it works. It's tempting to think that you can just come back to it later after you get the robot walking but ask yourself, truthfully and honestly, when are you going to go back and wrestle the trig?

Anyways, my learning method: is a combination of an LLM, math webpages and pen/paper. So I provided the LLM with the source code of the inverse kinematics:

```
def coordinate_to_angle(self, x, y, z, l1=33, l2=90, l3=110):
        a = math.pi / 2 - math.atan2(z, y)
        x_3 = 0
        x_4 = l1 * math.sin(a)
        x_5 = l1 * math.cos(a)
        l23 = math.sqrt((z - x_5) ** 2 + (y - x_4) ** 2 + (x - x_3) ** 2)
        w = self.restrict_value((x - x_3) / l23, -1, 1)
        v = self.restrict_value((l2 * l2 + l23 * l23 - l3 * l3) / (2 * l2 * l23), -1, 1)
        u = self.restrict_value((l2 ** 2 + l3 ** 2 - l23 ** 2) / (2 * l3 * l2), -1, 1)
        b = math.asin(round(w, 2)) - math.acos(round(v, 2))
        c = math.pi - math.acos(round(u, 2))
        return round(math.degrees(a)), round(math.degrees(b)), round(math.degrees(c))
```

Then I asked the LLM to break down the process to deriving the IK step by step. The LLM does a pretty good job describing how each servo position is derived.

![coxa joint example]({{ '/assets/images/lim_trig_explain.png' | relative_url }})

Maybe that's clear as day to you but I still needed a bit more of a visualization for a deep deep deep understanding. 

![mathisfun link]({{ '/assets/images/mathisfun.png' | relative_url }})

(https://www.mathsisfun.com/algebra/trig-interactive-unit-circle.html
)[so I found this website which does a great job visualizing the trig and has so much more content in a beginner level format.]. 

Then I gave it a shot on pen and paper on my own to check my understanding.

![coxa joint example]({{ '/assets/images/coxa_trig.png' | relative_url }})

And then, once I feel confident I can teach it to someone else, I implement it in code.

This is the way. 

#### Now for the bad news
After these past 2 or 3 weeks integrating Gazebo Sim Harmonic with my ROS nodes, I've decided that I love it but hate it 🥲. I will defend Gazebo Sim to my last breath because of what they stand for (freedom of information, education, advancing society, collaboration, innovation, etc.) but one huge drawback that I can just not overlook is the fact the physics simulation does not leverage the GPU. The sim models, the visualizations, have as of late began to use compute, but the physics engines underneath still do not 😭. 

#### The break-up 
Now that I've decided to break up with Gazebo Sim 💔 it means I must digress into finding another sim package to integrate with my ROS project. I recently purchased a pretty little laptop G14 Zephyrus with an RTX 5070ti but after what I've see on youtube from other developers and what I've heard from word of mouth at TXRX, Isaac Sim documentation is awful. They are very young in their stage, a huge project, and at the speed GPUs are advancing, it's understandable the documentation would be the first to suffer. But another reason I am staying away is because of the minimum GPU requirements. 

![isaac sim sys req]({{ '/assets/images/isaac_sys_req.png' | relative_url }})

The ideal setup is basically unobtainium for the majority of developers. This is eye candy simulations for big companies (which I am not.)

#### Other options:
List of other options: 
- CoppeliaSim: 
    - Partial GPU accelerated physics through modified physics engines. 
    - Machine Learning through PyTorch
- Mujoco
    - MJWarp is a GPU optimized physics simulator
    - Machine Learning with MJX using Brax which uses Jax 👀
- WeBots
    - Does not support GPU accelerated Physics 
    - Machine Learning with PyTorch is an option

### Next Steps 
I guess it's back to finding a simulation home. I really want to pin down a home base devops workflow before committing more code or edits to the project so that I don't end up having to redo a bunch (even if AI does all the heavy lifting conversions for me lol)
