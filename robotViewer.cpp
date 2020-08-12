#include "defs.h"
#include "mpu6050.h"
#include "mpu9250.h"
#include "ak8963.h"

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

using namespace samsRobot;

// curses window handlers
int init_screen(void);
int message_box(const char* msg);
void create_board(void);
void destroy_board(void);
void draw_board(int keypress = 0);

void init_motors(void);

// other handlers
void* update_robot_status(void*);
void* draw_screen(void*);
void* drive_motors(void*);

// pointers to windows
WINDOW* w_lcontrol;
WINDOW* w_rcontrol;
WINDOW* w_status;
WINDOW* w_input;

// current user key
int current_key = 0;
int previous_key = 0;

int motor_status = 0;

int main(int argc, char **argv)
{
    int key = 0;

    // start the curses screen
	if(init_screen() != 0)
		exit(2);

    clear();
    create_board(); // this uses drawboard in the background

    // prepare the motors
    init_motors();

    mvwprintw(w_input, 1, 1, "INPUT: %c", key); wrefresh(w_input);

    // create a thread to handle draw screens and receive user input
    // create a thread to handle motors
    pthread_t userThreadID, motorThreadID, statusThreadID;
    pthread_attr_t userThreadAttr;
    pthread_attr_t motorThreadAttr;
    pthread_attr_t statusThreadAttr;
    pthread_attr_init(&userThreadAttr);
    pthread_attr_init(&motorThreadAttr);
    pthread_attr_init(&statusThreadAttr);
    pthread_create(&userThreadID, &userThreadAttr, draw_screen, &key);
    pthread_create(&motorThreadID, &motorThreadAttr, drive_motors, &key);
    pthread_create(&statusThreadID, &statusThreadAttr, update_robot_status, &key);

    // do other stuff here;

    pthread_join(userThreadID, NULL);
    pthread_join(motorThreadID, NULL);
    pthread_join(statusThreadID, NULL);

    /* when done, free up the board, and exit */
    destroy_board();

    endwin();
    exit(0);
}

// function to start the curses screen
int init_screen(void){
    initscr(); // start the curses screen
    raw(); // handle input immediately
    noecho(); // Don't show incoming text
    cbreak();
    halfdelay(2); // wait 2/10ths of a second for user input
    keypad(stdscr, TRUE); // allow full key capture
    curs_set(FALSE); //disable the cursor
    start_color(); // use colours if available (should use has_colors() to check really!)
    // define the colours i want to use as colour pair
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    clear();

    if ((LINES < MIN_TERM_HEIGHT) || (COLS < MIN_TERM_WIDTH)) {
        endwin();
        printf("Your terminal needs to be at least %dx%d", MIN_TERM_WIDTH, MIN_TERM_HEIGHT);
	return -1;
    }else{
	    return 0;
    }
}

void create_board(void)
{
	/* split screen into 20x20 grid */
	int wWidth = COLS, wHeight = LINES;

	w_status = newwin(11*wHeight/20, 18*wWidth/20, 1*wHeight/20, 1*wWidth/20); 
	w_lcontrol = newwin(4*wHeight/20, 8*wWidth/20, 12*wHeight/20, 1*wWidth/20);
	w_rcontrol = newwin(4*wHeight/20, 8*wWidth/20, 12*wHeight/20, 11*wWidth/20);
	w_input = newwin(3*wHeight/20, 18*wWidth/20, 16*wHeight/20, 1*wWidth/20);

	// only need to do this once!
	wborder(stdscr, 0,0,0,0,0,0,0,0); wrefresh(stdscr);

    // reuse the draw function
    draw_board(0);

    mvwprintw(w_input, 2, 1, "Please press h for help, q to quit."); wrefresh(w_input);

}

void destroy_board(void)
{
    /* erase every box and delete each window */
    wborder(w_lcontrol, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_lcontrol); delwin(w_lcontrol);
    wborder(w_rcontrol, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_rcontrol); delwin(w_rcontrol);
    wborder(w_status, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_status); delwin(w_status);
    wborder(w_input, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_input); delwin(w_input);
}

int message_box(const char* msg){
	int key;
	WINDOW* temp = newwin(1*LINES/2, 1*COLS/2, 1*LINES/4, 1*COLS/4);
	wborder(temp, 0,0,0,0,0,0,0,0);
	mvwprintw(temp, 1, 1, msg);
	wrefresh(temp);
	key = getch();  // will draw the screen
	wborder(temp, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(temp);delwin(temp);
	return key;

}

void draw_board(int keypress){
	// redraw the keys + status - taking a keypress as argument.
	/* put border on each window and refresh */

	wborder(w_lcontrol,0,0,0,0,0,0,0,0); wrefresh(w_lcontrol);
	wborder(w_rcontrol,0,0,0,0,0,0,0,0); wrefresh(w_rcontrol);
	wborder(w_status,0,0,0,0,0,0,0,0);  wrefresh(w_status);
	wborder(w_input,0,0,0,0,0,0,0,0); wrefresh(w_input);

	// identify each window and show user keys
	mvprintw(LINES - 1, (COLS - 13) / 2, "robotViewer v1");

	int x,y;

	getmaxyx(w_lcontrol,y,x);
	mvwprintw(w_lcontrol, 0, (x - 14)/2, "LH(Boom/Swing)");
	mvwprintw(w_lcontrol, 1, (2*x)/4, "^");
	if(keypress==(int)'w' || (keypress==(int)'W')){
		wattron(w_lcontrol, A_BOLD); wattron(w_lcontrol, COLOR_PAIR(1));
		mvwprintw(w_lcontrol, 2, (2*x)/4, "W");
		wattroff(w_lcontrol, A_BOLD); wattroff(w_lcontrol, COLOR_PAIR(1));
	}
	else{
		mvwprintw(w_lcontrol, 2, (2*x)/4, "W");
	}
	if(keypress== 'a' || (keypress== 'A')){
		wattron(w_lcontrol, A_BOLD); wattron(w_lcontrol, COLOR_PAIR(1));
		mvwprintw(w_lcontrol, 3, (1*(x))/4-2, "<-A");
		wattroff(w_lcontrol, A_BOLD); wattroff(w_lcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_lcontrol, 3, (1*(x))/4-2, "<-A");
	}
	if(keypress== 's' || (keypress== 'S')){
		wattron(w_lcontrol, A_BOLD); wattron(w_lcontrol, COLOR_PAIR(1));
		mvwprintw(w_lcontrol, 4, (2*x)/4, "S");
		wattroff(w_lcontrol, A_BOLD); wattroff(w_lcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_lcontrol, 4, (2*x)/4, "S");
	}
	mvwprintw(w_lcontrol, 5, (2*x)/4, "v");
	if(keypress== 'd' || (keypress== 'D')){
		wattron(w_lcontrol, A_BOLD); wattron(w_lcontrol, COLOR_PAIR(1));
		mvwprintw(w_lcontrol, 3, (3*x)/4, "D->");
		wattroff(w_lcontrol, A_BOLD); wattroff(w_lcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_lcontrol, 3, (3*x)/4, "D->");
	}

	// do w_rcontrol;
	getmaxyx(w_rcontrol,y,x);
	mvwprintw(w_rcontrol, 0, (x - 12)/2, "RH(Dip/Curl)");
	mvwprintw(w_rcontrol, 1, (2*x)/4, "^");
	if(keypress==(int)'i' || (keypress==(int)'I')){
		wattron(w_rcontrol, A_BOLD); wattron(w_rcontrol, COLOR_PAIR(1));
		mvwprintw(w_rcontrol, 2, (2*x)/4, "I");
		wattroff(w_rcontrol, A_BOLD); wattroff(w_rcontrol, COLOR_PAIR(1));
	}
	else{
		mvwprintw(w_rcontrol, 2, (2*x)/4, "I");
	}
	if(keypress== 'j' || (keypress== 'J')){
		wattron(w_rcontrol, A_BOLD); wattron(w_rcontrol, COLOR_PAIR(1));
		mvwprintw(w_rcontrol, 3, (1*(x))/4-2, "<-J");
		wattroff(w_rcontrol, A_BOLD); wattroff(w_rcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_rcontrol, 3, (1*(x))/4-2, "<-J");
	}
	if(keypress== 'k' || (keypress== 'K')){
		wattron(w_rcontrol, A_BOLD); wattron(w_rcontrol, COLOR_PAIR(1));
		mvwprintw(w_rcontrol, 4, (2*x)/4, "K");
		wattroff(w_rcontrol, A_BOLD); wattroff(w_rcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_rcontrol, 4, (2*x)/4, "K");
	}
	mvwprintw(w_rcontrol, 5, (2*x)/4, "v");
	if(keypress== 'l' || (keypress== 'L')){
		wattron(w_rcontrol, A_BOLD); wattron(w_rcontrol, COLOR_PAIR(1));
		mvwprintw(w_rcontrol, 3, (3*x)/4, "L->");
		wattroff(w_rcontrol, A_BOLD); wattroff(w_rcontrol, COLOR_PAIR(1));
	}else{
		mvwprintw(w_rcontrol, 3, (3*x)/4, "L->");
	}
	
    	// process input
    	mvwprintw(w_input, 1, 1, "INPUT: ");
    	wattron(w_input, A_BOLD); wattron(w_input, COLOR_PAIR(1));
    	mvwprintw(w_input, 1, 8, "%c", keypress);
    	wattroff(w_input, A_BOLD); wattroff(w_input, COLOR_PAIR(1));

	// update the status window
	getmaxyx(w_status,y,x);
	mvwprintw(w_status, 0, (x - 12)/2, "Status");
	// mvwprintw(w_status, 1, 1, "T%02.1fC, P%02.1f, Y%02.1f, R%02.1f", 
			// temp, curr_pitch, curr_yaw, curr_roll);
	if (motor_status == 0)
		mvwprintw(w_status, 2, 1, "Status: STOPPED ");
	else
		mvwprintw(w_status, 2, 1, "Status: MOVEMENT ");

	// draw all windows
	wrefresh(w_lcontrol);
	wrefresh(w_rcontrol);
    	wrefresh(w_input);
	wrefresh(w_status);

}

void* draw_screen(void*){
	int keys = 0;

    do {
    	keys = getch();

    	// make the current key available to other threads
	previous_key = current_key;
    	current_key = keys;

	// update the screens
    	draw_board(keys);

	// provide help if required
    	if((keys == 'h') || (keys == 'H')){
    		keys = message_box("Hello there, this is RobotViewer v1.");
		usleep(2000000);
    	}

    	usleep(50000); // rest for 50ms 
    	draw_board(); // remove the highlight

	// this line below, handle elsewhere
    	current_key = 0;// disable the motor running indefinately

    } while ((keys != 'q') && (keys != 'Q'));
	pthread_exit(0);
}


void* drive_motors(void*){
	// we've received this key. check to see if it requires movement
	do{
		usleep(20000); // run the motor

	}while ((current_key != 'q') && (current_key != 'Q'));

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
	}while ((current_key != 'q') && (current_key!='Q'));
	pthread_exit(0);
}

void init_motors(void){
}
