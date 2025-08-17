---
layout: default
title:  "If you build it, they will come."
date:   2025-08-08
---

## Robot Building Montage!

Since I was a teenager, I've always really wanted to learn ROS. I've installed it before, messed around with gazebo but never really made or controlled a real robot. Fast-forward 18 years to a C.S. degree, hack-a-thon gold medals, a vast network of like-minded friends and colleagues, commanding space robotics on the International Space Station, and now I've finally begun my journey into programming robots. On August 8th, I poked my head into the Houston Robotics Group meeting over at TXRX and asked around for advice on how to get started. I brought with me a box with a raspberry pi 5, a jetson tx2, and a jackery 100 explorer battery and asked what's the best way to design a robot from scratch. They pointed at a Freenove robot kit and said "If you can't make this thing run on ROS, you can't make anything run on ROS". And while that wasn't what I wanted to hear, it was definitely what I needed to hear. So I took the plunge, purchased a Freenove robot kit (tons to choose from online) and built it. The kit is open-source python that operates with a client/server architecture (pi is the server, windows launches a client). This is different from ROS DDS, but we'll get into that later. 

For now, check out this 2-minute time lapse of the 6 hours it took me to build the robot. 

<div class="responsive-youtube-short">
  <iframe
    src="https://www.youtube.com/embed/UhZJ0yrpg-4"
    title="YouTube video player"
    frameborder="0"
    allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
    allowfullscreen>
  </iframe>
</div>

### Next Steps
Think about how I'm going to design and implement a ROS network of components. My spidey senses are telling me I'll need ROS Foxy because of the jetson tx2 being EOL and I'll probably want to use docker on everything to keep things repeatable and swappable. Stay tuned for the next one!