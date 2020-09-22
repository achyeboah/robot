#include "robotCurses.h"
#include "defs.h"
#include "config.h"

#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>

namespace samsRobot{
	robotCurses::robotCurses(){
		// doesn't do much
		validWins = FALSE;
		if(init_screen() == 0){
			this->validWins = TRUE;
	    		clear();
			create_board(); // this uses drawboard in the background
		}
	}

	robotCurses::~robotCurses(){
    		/* when done, free up the board, and exit */
		destroy_board();
		endwin();
	}

	// function to start the curses screen
	int robotCurses::init_screen(void){
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

	void robotCurses::create_board(void){
		/* split screen into 20x20 grid */
		int wWidth = COLS, wHeight = LINES;

		w_status = newwin(7*wHeight/20, 18*wWidth/20, 1*wHeight/20, 1*wWidth/20); 
		w_lcontrol = newwin(6*wHeight/20, 8*wWidth/20, 9*wHeight/20, 1*wWidth/20);
		w_rcontrol = newwin(6*wHeight/20, 8*wWidth/20, 9*wHeight/20, 11*wWidth/20);
		w_input = newwin(3*wHeight/20, 18*wWidth/20, 16*wHeight/20, 1*wWidth/20);

		// only need to do this once!
		wborder(stdscr, 0,0,0,0,0,0,0,0); wrefresh(stdscr);

		// reuse the draw function
		draw_board(0);

		mvwprintw(w_input, 2, 1, "Please press h for help, q to quit."); wrefresh(w_input);
	}

	void robotCurses::draw_board(int keypress){
		if(this->getValidWins() == TRUE){
			// redraw the keys + status - taking a keypress as argument.
			/* put border on each window and refresh */

			wborder(w_lcontrol,0,0,0,0,0,0,0,0); wrefresh(w_lcontrol);
			wborder(w_rcontrol,0,0,0,0,0,0,0,0); wrefresh(w_rcontrol);
			wborder(w_status,0,0,0,0,0,0,0,0);  wrefresh(w_status);
			wborder(w_input,0,0,0,0,0,0,0,0); wrefresh(w_input);

			// identify each window and show user keys
			mvprintw(LINES - 1, (COLS - 13) / 2, "robotViewer %02d.%02d.%02d",
					samsRobot_VERSION_MAJOR,
					samsRobot_VERSION_MINOR,
					samsRobot_VERSION_PATCH );

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
			mvwprintw(w_status, y-y, (x - 12)/2, "Status");
			mvwprintw(w_status, 1,1, "openGL FPS = %02.2f", ogl_fps);

			for (int i = 0; i < (this->ilen); i++){
				mvwprintw(w_status, 2, 1, "T%02.1fC, P%02.1f, Y%02.1f, R%02.1f",
					idata[i].temp, idata[i].pitch, idata[i].yaw, idata[i].roll);
			}

			// draw all windows
			wrefresh(w_lcontrol);
			wrefresh(w_rcontrol);
			wrefresh(w_input);
			wrefresh(w_status);
		}
	}

	void robotCurses::set_imu(const imu_data* data, const unsigned int len){
		unsigned int index = 0;
		/* no error checking, presuming len is valid */
		ilen = len;
		for(index = 0; index < len; index++){
			idata[index] = data[index];
		}
	}

	bool robotCurses::getValidWins(void) const {
		return validWins;
	}

	void robotCurses::destroy_board(void){
		/* erase every box and delete each window 
		only if we have a window anyway */
		if(this->getValidWins() == TRUE){
			wborder(w_lcontrol, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_lcontrol); delwin(w_lcontrol);
			wborder(w_rcontrol, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_rcontrol); delwin(w_rcontrol);
			wborder(w_status, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_status); delwin(w_status);
			wborder(w_input, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(w_input); delwin(w_input);
		}
	}

	int robotCurses::message_box(const char* msg){
		int key;
		WINDOW* temp = newwin(1*LINES/2, 1*COLS/2, 1*LINES/4, 1*COLS/4);
		wborder(temp, 0,0,0,0,0,0,0,0);
		mvwprintw(temp, 1, 1, msg);
		wrefresh(temp);
		key = getch();  // will draw the screen
		wborder(temp, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); wrefresh(temp);delwin(temp);
		return key;

	}

	int robotCurses::update(void){
		// let us know whats happening in the outside world

		int keys = getch();

		// update the screens
		this->draw_board(keys);

		// provide help if required
		if((keys == 'h') || (keys == 'H')){
			keys = this->message_box("Hello there, this is RobotViewer v1.");
			usleep(2000000);
		}

		usleep(100000); // rest for 50ms 
		this->draw_board(0); // remove the highlight

		return keys;
	}
		
	void robotCurses::set_ogl_fps(const float fps){
		this->ogl_fps = fps;
	}
}
