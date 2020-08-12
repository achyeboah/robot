#include "defs.h"
#include "mpu6050.h"
#include "mpu9250.h"
#include "ak8963.h"

#include <iostream>

using namespace std;
using namespace samsRobot;

int main(void){
	int index = 0;

	mpu6050 m6050(1, 0x68);
	mpu9250 m9250(1, 0x71);
	ak8963 m8963(1,0x48);

	for(index = 0; index < 10; index++){
		cout<<index<<endl;
	}
}
