# robot
Simple robot automatic control and visualisation program for a simple off-shelf 4DoF arm manipulator. 
Offers support for reading IMU status on I2C, network broadcast of IMU state and values.
Uses MPU6050 and MPU9250 (with AK8963) gyroscope/accelerometer/magnetometer as IMUs


## Table of contents
* [General info](#general-info)
* [Technologies](#technologies)
* [Setup](#setup)
* [Todo](#todo)

## General info
This project is a simple robot automatic control and visualisation program for a simple off-shelf 4DoF arm manipulator. 
Offers support for reading IMU status on I2C, network broadcast of IMU state and values.
Uses MPU6050 and MPU9250 (with AK8963) gyroscope/accelerometer/magnetometer as IMUs
	
## Technologies
Project is created with:
* Some code from Derek Molloy's exploring Raspberry Pi
* GLFW3
* GLEW

Project is validated on
* Debian 10 in a virtual machine (virtualBox ) running on a Macbook
* Raspian OS 4 on a Raspberry Pi 3
	
## Setup
To run this project, download the source code and build it:

```
$ mkdir build
$ cd ../build
$ cmake ../
$ make
```

## Todo
Titles and internal titles
Introduction - the project's aim
Technologies
Launch
Illustrations
Scope of functionalities 
Examples of use
Project status 
Sources
Other information
