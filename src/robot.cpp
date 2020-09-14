#include "defs.h"
#include "config.h"
#include "imu.h"
#include "utils.h"
#include "robotCurses.h"
#include "robotGL.h"
#include "robotSeg.h"
// add the socketclient to read from rPi
#include "SocketClient.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h> // for rand()
#include <string>
#include <cstring>

using namespace samsRobot;

void init_motors(void);
void print_usage(int argc, char **argv);

// other handlers
void* update_robot_status(void*);
void* draw_curses(void*);
void* drive_motors(void*);
void* draw_graphics(void*);

// current user key, info returned from draw_curses();
int current_key = 0;
bool quit_gl = FALSE;
bool do_fullscreen = FALSE;
int previous_key = 0;
float robotGL_fps = 0.0f;

// current motor_status, info returned from drive_motors() and update_robot_status();
int motor_status = 0;

// global because accessed by multiple threads
robotSeg ground, up_axis;

// create a structure to house our robot!
struct myRobot{
	// robot segments go here
	robotSeg base;
	robotSeg boom;
	robotSeg dipper;
	robotSeg bucket;
	// robot IMUs go here
	imu_data boom_data;
} theRobot;

int main(int argc, char **argv)
{
	/* options for this program are as follows
	 * c|C displays the ncurses screen
	 * g|G displays the opengl screen
	 * f|F displays the opengl screen in fullscreen
	 * absence of either of these, or any other displays usage screen
	 */

	int key = 0;
	bool do_curses = FALSE;
	bool do_gl = FALSE;
	int opt; // track options
	if (argc < 2){
		print_usage(argc, argv);
		exit(EXIT_FAILURE);
	}

	while((opt = getopt(argc, argv, "cfgCFG")) != -1){
		switch(opt){
			case 'c':
			case 'C':
				do_curses = TRUE;
				break;
			case 'f':
			case 'F':
				do_fullscreen = TRUE;
				do_gl = TRUE;
				break;
			case 'g':
			case 'G':
				do_gl = TRUE;
				break;
			default:
				print_usage(argc, argv);
				do_gl = TRUE;
				// exit(EXIT_FAILURE);
		}
	}

	// if both curses and opengl are not started, nothing to show, so just exit
	if ((do_gl == FALSE) && (do_curses == FALSE))
		exit(0);

	// prepare the motors
	init_motors();

	// create a thread to handle draw screens and receive user input
	// create a thread to handle motors
	// create a thread to get the status of the motors/controller
	// create a thread to draw the opengl window
	pthread_t userThreadID, motorThreadID, statusThreadID, openGLThreadID;
	pthread_attr_t userThreadAttr;
	pthread_attr_t motorThreadAttr;
	pthread_attr_t statusThreadAttr;
	pthread_attr_t openGLThreadAttr;
	pthread_attr_init(&userThreadAttr);
	pthread_attr_init(&motorThreadAttr);
	pthread_attr_init(&statusThreadAttr);
	pthread_attr_init(&openGLThreadAttr);

	if (do_curses == TRUE)
		pthread_create(&userThreadID, &userThreadAttr, draw_curses, &key);
	if (do_gl == TRUE)
		pthread_create(&openGLThreadID, &openGLThreadAttr, draw_graphics, &key);

	pthread_create(&motorThreadID, &motorThreadAttr, drive_motors, &key);
	pthread_create(&statusThreadID, &statusThreadAttr, update_robot_status, &key);

	// do other stuff here;

	if (do_gl == TRUE)
		pthread_join(openGLThreadID, NULL);
	if (do_curses == TRUE)
		pthread_join(userThreadID, NULL);

	pthread_join(motorThreadID, NULL);
	pthread_join(statusThreadID, NULL);

	exit(0);
}

void* draw_curses(void*){
	int keys = 0;
	robotCurses myscreen;

	if(myscreen.getValidWins() != TRUE)
		pthread_exit(0);

	// dont need to check for valid windows - we've got this far already!
	do {
		// check if some other thread said to quit
		if (quit_gl != TRUE){

			myscreen.set_ogl_fps(robotGL_fps);
			keys = myscreen.update(); 

			// make the current key available to other threads
			previous_key = current_key;
			current_key = keys;
			usleep(20000); // rest a bit
		}else{
			keys = 'q';
			current_key = keys;
		}

	} while ((keys != 'q') && (keys != 'Q'));

	// received a q
	pthread_exit(0);
}


void* drive_motors(void*){
	// we've received this key. check to see if it requires movement
	do{
		usleep(20000); // run the motor

	}while ((current_key != 'q') && (current_key != 'Q') && (quit_gl != TRUE));

	pthread_exit(0);
}


void* update_robot_status(void*){
	char serv[] = "192.168.10.46";
	exploringRPi::SocketClient sc(serv, 12321);
	sc.connectToServer();

	std::string delim = " ";
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
	float temp = 0.0f;

	do{
		pitch = 0.0f;
		yaw = 0.0f;
		roll = 0.0f;
		temp = 0.0f;

		if(sc.isClientConnected() != true){
			fprintf(stderr, "Please make sure the imu service is running on the remote server\n");
			usleep(1000000);
			sc.connectToServer();
		}

		// send a message to read from imu on address 0x68
		std::string cmsg("0x68");
		sc.send(cmsg);

		std::string smsg = sc.receive(100);
		std::string delim = " ";
		size_t pos; 
		// received stream is space delimited as type, then in groups of three, then temp
		// now [type pitch yaw roll temp]
		if(smsg.length() > 1){
			// we've got some data
			pos = smsg.find(delim);	theRobot.boom_data.type = ::atoi(smsg.substr(0, pos).c_str()); smsg.erase(0,pos+1);
			pos = smsg.find(delim);	theRobot.boom_data.pitch = ::atof(smsg.substr(0, pos).c_str()); smsg.erase(0,pos+1);
			pos = smsg.find(delim);	theRobot.boom_data.roll = ::atof(smsg.substr(0, pos).c_str()); smsg.erase(0,pos+1);
			pos = smsg.find(delim);	theRobot.boom_data.yaw = ::atof(smsg.substr(0, pos).c_str()); smsg.erase(0,pos+1);
			pos = smsg.find(delim);	theRobot.boom_data.temp = ::atof(smsg.substr(0, pos).c_str()); smsg.erase(0,pos+1);
		}

		// go to sleep for a bit
		usleep(100000); // 10ms
	}while ((current_key != 'q') && (current_key!='Q') && (quit_gl !=TRUE));

	pthread_exit(0);
}

void init_motors(void){
}

void* draw_graphics(void*){
	robotGL glWin(do_fullscreen);
	// our window was created successfully (should check this!)

	// create axes here
	up_axis.setID(2);
	up_axis.set_axis(1);
	up_axis.set_dimensions(0.05,1000,0.05);
	up_axis.set_colors(0.5,0.5,0.5);
	ground.setID(3);
	ground.set_axis(1);
	ground.set_dimensions(1000.0,0.01,1000);
	ground.set_colors(0.4,0.4,0.4);

	// create robot here
	theRobot.base.setID(5);
	// theRobot.base.set_axis(0.1);
	theRobot.base.set_dimensions(1);
	theRobot.base.set_colors(0.3,0.3,0.3);
	theRobot.boom.setID(6);
	theRobot.boom.set_dimensions(6);
	theRobot.boom.set_colors(0.3,0.5,0.3);
	theRobot.boom.setParent(&theRobot.base);
	theRobot.dipper.setID(7);
	theRobot.dipper.set_dimensions(4);
	theRobot.dipper.set_colors(0.5,0.3,0.5);
	theRobot.dipper.setParent(&theRobot.boom);
	theRobot.bucket.setID(8);
	theRobot.bucket.set_dimensions(1,1,1);
	theRobot.bucket.set_axis(1);
	theRobot.bucket.set_colors(0.3,0.5,0.5);
	theRobot.bucket.setParent(&theRobot.dipper);

	// need to pass in some parameters (make opaque for now)
	glWin.set_bg(0.0f, 0.0f, 0.1f, 0.8f);

	glWin.create_cuboid(theRobot.base);	
	glWin.create_cuboid(theRobot.boom);	
	glWin.create_cuboid(theRobot.dipper);	
	glWin.create_cuboid(theRobot.bucket);	

	glWin.create_cuboid(up_axis);
	glWin.create_cuboid(ground);

	// remember seg1 does not move
	do{
		glWin.set_segAngles(theRobot.base.getID(), theRobot.boom_data.roll, 0, 0); // needs some work
		glWin.set_segAngles(theRobot.boom.getID(), theRobot.boom_data.pitch, theRobot.boom_data.yaw, theRobot.boom_data.roll); // needs some work

	 	glWin.updateScreen();
		robotGL_fps = glWin.get_fps();
		usleep(20000); // achieving 80fps with 10ms sleep, 20fps with 50ms sleep
	}while(glWin.get_progFinished() == FALSE);

	// send a message to other threads that its time to quit!
	quit_gl = TRUE;
	pthread_exit(0);

}

void print_usage(int argc, char** argv){
	printf("\n\nWelcome! ");
	printf("This is %s v%02d.%02d.%02d\n",
			argv[0],
			samsRobot_VERSION_MAJOR,
			samsRobot_VERSION_MINOR,
			samsRobot_VERSION_PATCH );
	printf("Please note that program %s usage is as follows. \n", argv[0]);
	printf("\t-c\tUse NCurses window\n");
	printf("\t-g\tUse openGL window\n");
	printf("\t-f\tUse full screen (calls -g)\n");
	printf("\nOptions may be combined eg (-gc)\n\n");
	printf("Keys inside the application:\n");
	printf("\tLArrow\tMove camera left\n");
	printf("\tRArrow\tMove camera right\n");
	printf("\tUArrow\tMove camera up\n");
	printf("\tDArrow\tMove camera down\n");
	printf("\tPgUp\tMove camera towards view\n");
	printf("\tPgDn\tMove camera away from view\n");
	printf("\n");
}
