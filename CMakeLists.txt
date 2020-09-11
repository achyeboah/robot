cmake_minimum_required(VERSION 3.10)
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED true)

# project name and version
project(samsRobot
       	VERSION 2020.09.11
	DESCRIPTION "Simple program to manipulate a 4DoF robot"
	LANGUAGES CXX
	)

configure_file(config.h.in config.h)

find_package(GLU REQUIRED)
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
	)

target_include_directories(robot PUBLIC 
	"${PROJECT_SOURCE_DIR}/include" 
	"/home/sam/src/glm/"
	"${PROJECT_BINARY_DIR}"
      	)

target_link_libraries(robot PUBLIC ncurses dl glfw3 GL pthread GLEW)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
	message(STATUS "dir found = '${dir}'")
endforeach()
