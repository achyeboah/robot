#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

namespace samsRobot{

	unsigned int read_imu_data(const char* filename, imu_data& data){

		FILE* fhandle;
		if (filename == NULL)
			return 0;
		fhandle = fopen(filename, "r");
		if (fhandle == NULL)
			return 0;

		// parse the data into the object
		fscanf(fhandle, "%d: %f %f %f; %f %f %f; %f %f %f; %f", 
				&(data.type), 
				&(data.pitch), &(data.yaw), &(data.roll), 
				&(data.wVelX), &(data.wVelY), &(data.wVelZ), 
				&(data.headX), &(data.headY), &(data.headZ),
			       	&(data.temp)
				);

		return 0;

	}

	unsigned int write_imu_data(const char* filename, const imu_data data){
		// open a handle to a local file and write to it
		FILE *fhandle;
		if(filename == NULL)
			fhandle = fopen("imu_data.txt","w+");
		else
			fhandle = fopen(filename, "w+");

		if(fhandle==NULL){
			perror("file could not be opened\n");
			return -1;
		}

		fprintf(fhandle, "%d: %f %f %f; %f %f %f; %f %f %f; %f\n",
				data.type == 0 ? 6050 : 9250,
				data.pitch, data.yaw, data.roll,
				data.wVelX, data.wVelY, data.wVelZ,
				data.headX, data.headY, data.headZ,
				data.temp);
			   
		fclose(fhandle);
		return 0;
	}

}

