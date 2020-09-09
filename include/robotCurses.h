#ifndef ROBOTWIN_H
#define ROBOTWIN_H

#include <ncurses.h>

namespace samsRobot{

	// create our class to handle our curses window
	class robotCurses{
		private:
			// pointers to windows
			WINDOW* w_lcontrol;
			WINDOW* w_rcontrol;
			WINDOW* w_status;
			WINDOW* w_input;

			bool validWins;
			float ogl_fps = 3.1f;
		public:
			robotCurses();
			~robotCurses();

			int message_box(const char* msg);
			void create_board(void);
			void destroy_board(void);
			void draw_board(int keypress = 0);
			int update(void);
			
			int init_screen(void);
			bool getValidWins(void) const;
			void set_ogl_fps(const float fps);
	};
}

#endif
