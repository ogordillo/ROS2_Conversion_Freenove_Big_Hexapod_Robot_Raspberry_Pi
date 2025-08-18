---
layout: default
---
# My Hexapod ROS 2 Dev Journal 

Welcome to my page for the ROS 2 Hexapod project! 

## About me:
My name is Orlando Gordillo. I was born in the small town Laredo, Texas. I have a B.S. in Computer Science from the University of Texas at El Paso.
I have 10 years industry experience doing Robotics Operations and Software Development. 

I've always been curious to learn about ROS and I've now officially started my journey.
My personal goal is to contribute to the fast growing world of humanoid robotics and their intergration into our everyday lives. I know this robot is not exactly humanioid or humanoid-ish, but this Hexapod is a start and I hope the lessons here translate to the more advanced robotics projects I hope to undertake. 
 
---

## Follow My Progress through Journal Posts

In my posts I'll include updates, videos, and technical deep dives as I work on the robot. 

<ul>
  {% for post in site.posts %}
    <li>
        <a href="{{ post.url | relative_url }}">{{ post.title }}</a> - <span class="post-date">{{ post.date | date: "%B %d, %Y" }}</span>
    </li>
  {% endfor %}
</ul>

---

## Main Code Repository:
[ogordillo/ROS2_Hexapod](https://github.com/ogordillo/ROS2_Conversion_Freenove_Big_Hexapod_Robot_Raspberry_Pi/tree/foxy-develop)


## Parts List:
Aside from a laptop with Windows 11, these are the parts I'm using. Some of these parts are old, like the jetson tx2, and this is because I wanted to use what I had instead of going out to buy something new.
- [Freenove Big Hexapod FNK0052 - Amazon Link](https://amzn.to/47vyFz6)
- [Raspberry Pi 5 - Amazon Link](https://amzn.to/45EQbOT)
- [Nvidia Jetson TX2 - Amazon Link](https://amzn.to/3JlThQk)
- [Jetson TX2 Orbitty Carrier Board - Vender Link](https://www.google.com/url?sa=j&url=https%3A%2F%2Fconnecttech.com%2Fproduct%2Forbitty-carrier-for-nvidia-jetson-tx2-tx1%2F&uct=1750005445&usg=oWNIqRq9dAVoZOkQFAXqlN4Yqbw.&opi=73833047&source=chat)
- [Intel Realsense Depth Camera D435 - Amazon Link](https://amzn.to/46X4d0F)