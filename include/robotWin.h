#ifndef ROBOTWIN_H
#define ROBOTWIN_H

#include <ncurses.h>

namespace samsRobot{

	// create our class to handle our curses window
	class robotWin{
		private:
			// pointers to windows
			WINDOW* w_lcontrol;
			WINDOW* w_rcontrol;
			WINDOW* w_status;
			WINDOW* w_input;

			bool validWins;
			bool motor_status = 0;
		public:
			robotWin();
			~robotWin();

			int message_box(const char* msg);
			void create_board(void);
			void destroy_board(void);
			void draw_board(int keypress = 0);
			int update(int motor_status = 0);
			
			int init_screen(void);
			bool getValidWins(void) const;
	};
}

#endif
