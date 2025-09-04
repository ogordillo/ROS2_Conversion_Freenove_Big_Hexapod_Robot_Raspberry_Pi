---
layout: default
---
# How to convert the Freenove Hexapod Robot into a ROS2 Robot 🤖 

---

## Project Blog

In my posts I'll include updates, videos, struggles, and technical deep dives as I work through the project. 

<ul>
  {% for post in site.posts %}
    <li style="list-style-type: none; padding-bottom: 20px; clear: both; overflow: hidden;">
      
      {% if post.thumbnail %}
        <a href="{{ post.url | relative_url }}">
          <img src="{{ post.thumbnail | relative_url }}" alt="{{ post.title }}" 
               style="width: 80px; height: 80px; object-fit: cover; border-radius: 8px; margin-right: 15px; float: left;">
        </a>
      {% endif %}

      <a href="{{ post.url | relative_url }}">{{ post.title }}</a> - <span class="post-date">{{ post.date | date: "%B %d, %Y" }}</span>

    </li>
  {% endfor %}
</ul>

---

## Main Code Repository:
[ogordillo/ROS2_Hexapod](https://github.com/ogordillo/ROS2_Conversion_Freenove_Big_Hexapod_Robot_Raspberry_Pi/tree/foxy-develop)


## Parts List:
- Windows 11 Pro (pro is needed for HyperV) /w WSL2
- [Freenove Big Hexapod FNK0052 - Amazon Link](https://amzn.to/47vyFz6)
- [Raspberry Pi 5 - Amazon Link](https://amzn.to/45EQbOT)
- [Intel Realsense Depth Camera D435 - Amazon Link](https://amzn.to/46X4d0F)

## About me:
My name is Orlando Gordillo. I was born in a small town called Laredo, Texas. I have a B.S. in Computer Science from the University of Texas at El Paso.
I also have 10 years industry experience with Robotics Operations and Software Development for the Space industry. My main strengths are DevOps and Rapid Prototyping. I believe in diversity, grit, determination, the power of ingenuity, team-work and vision. I value sustainability, recycling, minimizing environmental impacts and helping each other. I hit the gym 3-4 times a week because a healthy mind needs a healthy body. We are all one. 

- "It takes a village to raise a child." - African Proverb
- "Make food thy medicine and medicine be thy food." - Hippocrates
- "We are made of star stuff" - Carl Sagan
- "Si, se puede!"

### For Recruiters:
- [My LinkedIn](https://www.linkedin.com/in/orlando-gordillo-933376a4/)
- [My Resume](https://drive.google.com/file/d/158RF4BewtNTz6CmujgvbqKpIV2s55kuR/view?usp=sharing)
- [My DevPost](https://devpost.com/OrlyGordo)
- [My Github](https://github.com/ogordillo/ROS2_Conversion_Freenove_Big_Hexapod_Robot_Raspberry_Pi/tree/jazzy-develop/)