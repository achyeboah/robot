#include "defs.h"
#include "mpu6050.h"
#include "mpu9250.h"
#include "ak8963.h"
#include "robotCurses.h"
#include "robotGL.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

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
int previous_key = 0;
float robotGL_fps = 0.0f;

// current motor_status, info returned from drive_motors() and update_robot_status();
int motor_status = 0;

		   static const float color_data[] = { 
			0.583f,  0.771f,  0.014f,
			0.609f,  0.115f,  0.436f,
			0.327f,  0.483f,  0.844f,
			0.822f,  0.569f,  0.201f,
			0.435f,  0.602f,  0.223f,
			0.310f,  0.747f,  0.185f,
			0.597f,  0.770f,  0.761f,
			0.559f,  0.436f,  0.730f,
			0.359f,  0.583f,  0.152f,
			0.483f,  0.596f,  0.789f,
			0.559f,  0.861f,  0.639f,
			0.195f,  0.548f,  0.859f,
			0.014f,  0.184f,  0.576f,
			0.771f,  0.328f,  0.970f,
			0.406f,  0.615f,  0.116f,
			0.676f,  0.977f,  0.133f,
			0.971f,  0.572f,  0.833f,
			0.140f,  0.616f,  0.489f,
			0.997f,  0.513f,  0.064f,
			0.945f,  0.719f,  0.592f,
			0.543f,  0.021f,  0.978f,
			0.279f,  0.317f,  0.505f,
			0.167f,  0.620f,  0.077f,
			0.347f,  0.857f,  0.137f,
			0.055f,  0.953f,  0.042f,
			0.714f,  0.505f,  0.345f,
			0.783f,  0.290f,  0.734f,
			0.722f,  0.645f,  0.174f,
			0.302f,  0.455f,  0.848f,
			0.225f,  0.587f,  0.040f,
			0.517f,  0.713f,  0.338f,
			0.053f,  0.959f,  0.120f,
			0.393f,  0.621f,  0.362f,
			0.673f,  0.211f,  0.457f,
			0.820f,  0.883f,  0.371f,
			0.982f,  0.099f,  0.879f
		};
	static const float vertex_data[] = { 
		   -1.0f,-1.0f,-1.0f,
		   -1.0f,-1.0f, 1.0f,
		   -1.0f, 1.0f, 1.0f,
		   1.0f, 1.0f,-1.0f,
		   -1.0f,-1.0f,-1.0f,
		   -1.0f, 1.0f,-1.0f,
		   1.0f,-1.0f, 1.0f,
		   -1.0f,-1.0f,-1.0f,
		   1.0f,-1.0f,-1.0f,
		   1.0f, 1.0f,-1.0f,
		   1.0f,-1.0f,-1.0f,
		   -1.0f,-1.0f,-1.0f,
		   -1.0f,-1.0f,-1.0f,
		   -1.0f, 1.0f, 1.0f,
		   -1.0f, 1.0f,-1.0f,
		   1.0f,-1.0f, 1.0f,
		   -1.0f,-1.0f, 1.0f,
		   -1.0f,-1.0f,-1.0f,
		   -1.0f, 1.0f, 1.0f,
		   -1.0f,-1.0f, 1.0f,
		   1.0f,-1.0f, 1.0f,
		   1.0f, 1.0f, 1.0f,
		   1.0f,-1.0f,-1.0f,
		   1.0f, 1.0f,-1.0f,
		   1.0f,-1.0f,-1.0f,
		   1.0f, 1.0f, 1.0f,
		   1.0f,-1.0f, 1.0f,
		   1.0f, 1.0f, 1.0f,
		   1.0f, 1.0f,-1.0f,
		   -1.0f, 1.0f,-1.0f,
		   1.0f, 1.0f, 1.0f,
		   -1.0f, 1.0f,-1.0f,
		   -1.0f, 1.0f, 1.0f,
		   1.0f, 1.0f, 1.0f,
		   -1.0f, 1.0f, 1.0f,
		   1.0f,-1.0f, 1.0f
		   };

int main(int argc, char **argv)
{
	/* options for this program are as follows
	 * c|C displays the ncurses screen
	 * g|G displays the opengl screen
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

	while((opt = getopt(argc, argv, "cgCG")) != -1){
		switch(opt){
			case 'c':
			case 'C':
				do_curses = TRUE;
				break;
			case 'g':
			case 'G':
				do_gl = TRUE;
				break;
			default:
				print_usage(argc, argv);
				exit(EXIT_FAILURE);
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
			keys = myscreen.update(motor_status); 

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
	float curr_yaw = 0, prev_yaw = 0;
	float curr_pitch = 0, prev_pitch = 0;
	float curr_roll = 0, prev_roll = 0;
	float temp = 0.0f;

	int x,y;
	
	// static mpu6050 boomIMU(1,0x68);

	do{
		// get the current accelerometer values
		//
		/*
		boomIMU.readSensorState();
		curr_yaw = boomIMU.getYaw();
		curr_pitch = boomIMU.getPitch();
		curr_roll = boomIMU.getRoll();
		temp = boomIMU.getTemp();

		if (((curr_yaw - prev_yaw) < YAW_THRESH) && 
			((curr_pitch - prev_pitch) < PITCH_THRESH) &&
			((curr_roll - prev_roll) < ROLL_THRESH))
			motor_status = 0;
		else
			motor_status = 1;
		
		prev_yaw = curr_yaw;
		prev_pitch = curr_pitch;
		prev_roll = curr_roll;
*/
		// go to sleep for a bit
		usleep(10000); // 10ms
	}while ((current_key != 'q') && (current_key!='Q') && (quit_gl !=TRUE));
	pthread_exit(0);
}

void init_motors(void){
}

void* draw_graphics(void*){
	robotGL glWin;
	// need to pass in some parameters
	// glWin->set_bg();
	glWin.set_mat(vertex_data, 3*4*12);
	glWin.set_col(color_data, 3*4*12);

	do{
		glWin.update();
		robotGL_fps = glWin.get_fps();
		usleep(50000); // achieving 80fps with 10ms sleep, 20fps with 50ms sleep
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
	printf("Please note that program %s is as follows. \n", argv[0]);
	printf("\t-c\tUse NCurses window\n");
	printf("\t-g\tUse openGL window\n");
	printf("\nOptions may be combined eg (-gc)\n\n");
}
