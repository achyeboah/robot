#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "GPIO.h"
using namespace exploringRPi;

namespace samsRobot{

	class dcmotor{
		public:
			enum MOTOR_DIR{STOPPED, CLOCKW, ACLOCKW};

		private:
			GPIO* pinEN, pinA, pinB;
			MOTOR_DIR direction;

		public:
			dcmotor(GPIO*, GPIO*, GPIO*);
			dcmotor(int , int , int);
			virtual ~dcmotor();

			virtual MOTOR_DIR getDirection const(void);
			virtual void setDirection(MOTOR_DIR);


			virtual void go(void); // go until stopped
			virtual void go(MOTOR_DIR ); // go until stopped
			virtual void go_ms(MOTOR_DIR, int); // go for a specific amount of time
			virtual void stop(void); // stop rotation
			virtual void reverse_dir(void); // reverse direction of travel

				
		};

}
#endif
