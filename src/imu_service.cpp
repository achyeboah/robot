#include "SocketServer.h"
#include "defs.h"
#include "config.h"
#include "imu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string> // already included from SocketServer
#include <cstring>

using namespace exploringRPi;
using namespace std;
using namespace samsRobot;

	void build_message(std::string& );

	int main (int argc, char** argv){

		unsigned int port = IMU_SERVICE_PORT;
		if (argc != 2){
			fprintf(stderr, "You need only one argument: the port to listen on\n");
			fprintf(stderr, "Rerun as follows, for example: %s %d\n", argv[0], 12345);
			fprintf(stderr, "Will use default port of %d\n", IMU_SERVICE_PORT);
		}else{
			port = atoi(argv[1]);
		}

		// create log file
		FILE* fpointer;
		char log_filename[] = "imu_service_log.txt";
		fpointer = fopen(log_filename, "w");
		if (fpointer == NULL){
			perror("imu_service: could not open log");
			return -1;
		}

		// print version (ideally should add start time too!)
		printf("imu_service v%d.%d.%d: starting on port %d\n", samsRobot_VERSION_MAJOR, samsRobot_VERSION_MINOR, samsRobot_VERSION_PATCH, port);
		fprintf(fpointer, "imu_service v%d.%d.%d: starting on port %d\n", samsRobot_VERSION_MAJOR, samsRobot_VERSION_MINOR, samsRobot_VERSION_PATCH, port);

		// start the server
		SocketServer server(port);
		server.listen();
		do{
			if(server.getConnected() == false){
				server.listen();
			}

			std::string client_req;
			client_req = server.receive(100);

			// gather the information we require
			std::string server_resp;
			build_message(server_resp);
			server.send(server_resp);

			// log the transaction
			char server_respc[server_resp.length()+1];
			strcpy(server_respc, server_resp.c_str());
			fprintf(fpointer, "Response: %s\n", server_respc );

		}while(1);// client_req.compare("quit" != 0));


		fclose(fpointer);

		return 0;
	}

	void build_message(std::string& server_resp){
		// gather all available IMUs, get data for each

		// need a better way to track data from all the IMUs and fuse them
		imu* boomIMU = new imu(1, 0x68, imu::MPU6050); 
		boomIMU->readSensorState();

		char temp[200];

		sprintf(temp, "%d %f %f %f %f", 
				boomIMU->getIMUtype() == 0 ? 6050 : 9250,
				boomIMU->getPitch(), boomIMU->getYaw(),	boomIMU->getRoll(),
				boomIMU->getTemp()
		       );

		server_resp.clear();
		server_resp += temp;

		delete boomIMU;
	}

