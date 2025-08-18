---
layout: default
title:  "Week 1 - If you build it, they will come."
date:   2025-08-08
---

## Robot Building Montage!

#### TL;DR; - The people at TXRX point me in the right direction! Here is a fun video of me building the hexapod!

Since I was a teenager, I've always wanted to learn ROS. I've installed it before, messed around with gazebo, but never really made or controlled a real robot. Fast-forward 18 years to a C.S. degree, hack-a-thon gold medals, a vast network of like-minded friends and colleagues, commanding space robotics on the International Space Station, and now I've finally begun to dedicate myself to actually programming robots. 

On August 8th, I poked my head into the Houston Robotics Group meeting over at TXRX and asked around for advice on the best way to get started. I had brought with me a box with only a raspberry pi 5, a jetson tx2, and a jackery 100 explorer battery. I courageously and yet naively asked what the best way to design a robot from scratch is. The team I spoke with asked me what my main goal was. I said I wanted to dive deep into learning ROS. Then they pointed me to a Freenove robot kit. In my head I thought: 'oh no, not a kit, aren't those kits for kids...' but then something inside me fought back and said "they know the path, I should listen. so I asked - "Why a kit?" Their answer almost floored me, they said "If you can't make this kit run on ROS, you can't make anything run on ROS". And that was definitely not what I wanted to hear but it sure was what I needed to hear. They were obviously right. 

So I took the plunge and purchased a Freenove robot kit. They have several models to choose from on [amazon](https://amzn.to/47vyFz6). Building the robot was time consuming to say the least - wanting to design every piece bottom up would have been detrimental to my goal of learning ROS. The kit is open-source python that operates with a client/server architecture (pi is the server, windows launches a client). This is different from ROS DDS, but we'll get into that later.

For now, check out this 2-minute time lapse of the 6 hours it took me to build the robot. 

<iframe
  src="https://www.youtube.com/embed/UhZJ0yrpg-4"
  title="YouTube video player"
  frameborder="0"
  allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
  allowfullscreen>
</iframe>

### Next Steps
Now that I've gotten the robot set up and working with the open-source provided software, I need to think about how I'm going to design and implement a ROS network of components. My spidey senses are telling me I'll need ROS Foxy because of the jetson tx2 being EOL and I'll probably want to use docker on everything to keep things repeatable and swappable. Stay tuned for the next one!