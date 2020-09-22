cmake_minimum_required(VERSION 3.00)
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED true)

# project name and version
project(robot
       	VERSION 2020.09.11
	#DESCRIPTION "Simple program to manipulate a 4DoF robot"
	LANGUAGES CXX
	)

configure_file(config.h.in config.h)

if(${CMAKE_VERSION} VERSION_LESS "3.8.0") 
	message(WARNING "Old version of CMAKE detected - not using CMP0072 for openGL")
else()
	message (NOTICE " CMAKE version " ${CMAKE_VERSION} " detected. Using CMP0072 for openGL")
	project(samsRobot DESCRIPTION "Simple program to manipulate a 4DoF robot")

	# use new policy (GLVND) for openGL (required from CMAKE 3.11 upwards)
	cmake_policy(SET CMP0072 NEW)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(robot 
	robot.cpp 
	robotSeg.cpp
	i2cdev.cpp 
	imu.cpp 
	utils.cpp
	robotGL.cpp
	robotCurses.cpp
	SocketClient.cpp
	)
target_include_directories(robot PUBLIC 
	"${PROJECT_SOURCE_DIR}/include" 
	"/home/sam/src/glm/"
	"${PROJECT_BINARY_DIR}"
      	)
target_link_libraries(robot PUBLIC ncurses dl glfw3 GL pthread GLEW)

add_executable(imu_service
	imu_service.cpp
	SocketServer.cpp
	i2cdev.cpp
	imu.cpp
	GPIO.cpp
	)
target_include_directories(imu_service PUBLIC 
	"${PROJECT_SOURCE_DIR}/include" 
	"${PROJECT_BINARY_DIR}"
      	)
target_link_libraries(imu_service pthread)
