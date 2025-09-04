
<img src="https://github.com/user-attachments/assets/2af0d8b0-47ad-464d-9fad-072bb90bf590" width="800">

<h2>Sojourner Model</h2>

This repository contains the software and documentation for a project to create a small, partly atonomous and fully operational model of the Sojourner Rover from the Mars Pathfinder project. The robot itself was build as a gift for author Andy Weir, author of The Martian. In good spirit of the project, the rover model is designed to replicate the appearance and structure of the real rover, and even includes fully functional solar panels, some sensor apparatus for data collection, and a fully autonomous mode. 

<p align="center">
<img src="https://github.com/user-attachments/assets/b0d0ef0e-d3b2-4624-ac28-bd54b6df87bb" width="400">
</p>

Although mostly intended as a toy, and a playful thanks from a fan to an author, the design was taken seriously. The robot includes two primary gear-driven wheels controlled with dual H-bridge chips, as well as four suspension wheels for balance and support, implementing skid steering. AN ultrasonic sensor is used for obstacle detection, and two IR sensors on over-wheel armatures for behavior based wall following and navigation. It is also equipped with temperature, humidity, and light level sensors in the 'science package' for doing whatever research you'd like with it while roaming around.

<p align="center">
<img src="https://github.com/user-attachments/assets/42deddfd-5fa6-4c86-9a66-0450a2eb8264" width="400">
</p>

The additional support systems include a LiPo battery pack and charge control system, which allow the rover to be charged either by its solar panels, or by a USB connection, and a wireless bluetooth module for communication, which connects to the controller

<p align="center">
<img src="https://github.com/user-attachments/assets/8ce7428d-272b-43bf-84aa-6d6815e1921a" width="400">
</p>

The controller enables manual drive control of the rover, using the built in joysticks, as well as selection between manual and auto navigation modes, and system status indicators including battery level, mode selection, and signal loss. It also acts as a wireless bridge to connect the data stream from the rover's sensors to a PC, should you want to collect data directly. An attached battery pack alows for mobile use of the controller, but it is also free to be powered by external USB

<p align="center">
<img src="https://github.com/user-attachments/assets/ecb72c47-8c8d-41e1-bcd7-afadda83822d" width="400">
</p>

Both the rover and controller include a built in USB/serial connection, allowing the firmware sets to be modified and reprogrammed at will, and collect serial data streams od scientific and diagnostic data from them.

<p align="center">
<img src="https://github.com/user-attachments/assets/89eb5d91-1fd4-48ff-b593-5f99b17cc945" width="400">
</p>

The rover implements a state machine for both manual and autonomous control modes, the manual mode states managing receipt of control signals and motor actuation, and the autonomous mode implementing a behavior-based obstacal avoidance and wall following exploration routine.

Further and more detailed information is contained within the attached manual, including further hardware assemply details and a full description of the autonomous mode software






