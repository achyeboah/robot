#include "defs.h"
#include "dcmotor.h"
#include "GPIO.h"
#include <unistd.h>

using namespace exploringRPi;

namespace samsRobot{

	dcmotor::dcmotor(GPIO* pinEN, GPIO* pinA, GPIO* pinB):
		pinEN(pinEN), pinA(pinA), pinB(pinB)
	{
		direction = dcmotor::STOPPED;
	}
	
	dcmotor::dcmotor(int en, int a, int b){
		this->pinEN = new GPIO(en);
		this->pinA = new GPIO(a);
		this->pinB = new GPIO(b);
		
		this->pinEN->setDirection(OUTPUT);
		this->pinA->setDirection(OUTPUT);
		this->pinB-setDirection(OUTPUT);
		
		this->direction = dcmotor::STOPPED;
	}

	dcmotor::~dcmotor();


	MOTOR_DIR dcmotor::getDirection(void) const {
		return this->direction;
	}
	
	void dcmotor::reverse_dir(void){
		if(this->getDirection() == dcmotor::CLOCKW)
			this->setDirection(ACLOCKW);
		else
			this->setDirection(CLOCKW);
	}

	void dcmotor::setDirection(MOTOR_DIR dir){
		this->direction = dir;
		switch(dir){
			case(STOPPED):
				this->pinEN->setValue(LOW);
				this->pinA->setValue(LOW);
				this->pinB->setValue(LOW);
				break;
			case(ACLOCKW):
				this->pinA->setValue(HIGH);
				this->pinB->setValue(LOW);
				break;
			case(CLOCKW):
				this->pinA->setValue(LOW);
				this->pinB->setValue(HIGH);
				break;
			default:
				this->pinEN->setValue(LOW);
				this->pinA->setValue(LOW);
				this->pinB->setValue(LOW);
				break;
		}
	}
	void dcmotor::go(void){
		// presume clockwise
		this->setDirection(dcmotor::CLOCKW);
		this->pinEN->setValue(HIGH);
	}

	 void dcmotor::go(MOTOR_DIR direction){
		this->setDirection(direction);
		this->pinEN->setValue(HIGH);
	 }

	 void dcmotor::go_ms(MOTOR_DIR dir, int ms){
		 this->go(dir);
		 usleep(ms*1000);
		 this->stop();
	 }

	 void dcmotor::stop(void){
		 this->setDirection(dcmotor::STOPPED);
	 }

		 

}
