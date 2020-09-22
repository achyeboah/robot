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

	void build_message(std::string& server_resp, int GPIO, unsigned int i2caddress, imu::IMU_TYPE type);

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

		unsigned int i2caddress = 0;
		int pinAD0 = 0;
		unsigned int itype;
		imu::IMU_TYPE ttype = imu::MPU6050;
		std::string delim = " ";

		do{
			if(server.getConnected() == false){
				server.listen();
			}

			std::string client_req;
			client_req = server.receive(100);

			/* build the imu parameters. 
			 * delimiter is space
			 * request from client is of the form 
			 * [i2caddress type pinGPIO] */

			size_t pos; 
			if(client_req.length() > 1){
				// we've got some data
				pos = client_req.find(delim);	i2caddress = (unsigned int)(::atoi(client_req.substr(0, pos).c_str())); client_req.erase(0,pos+1);
				pos = client_req.find(delim);	pinAD0 = ::atoi(client_req.substr(0, pos).c_str()); client_req.erase(0,pos+1);
				pos = client_req.find(delim);	itype = ::atoi(client_req.substr(0, pos).c_str()); client_req.erase(0,pos+1);

				switch(itype){
					case 6050:
						ttype = imu::MPU6050; break;
					case 9250:
						ttype = imu::MPU9250; break;
					case 8963:
						ttype = imu::AK8963; break;
					default:
						ttype = imu::MPU6050; break;
				}
			}

			// gather the information we require
			std::string server_resp;
			build_message(server_resp, pinAD0, i2caddress, ttype);
			server.send(server_resp);

			// log the transaction
			char server_respc[server_resp.length()+1];
			strcpy(server_respc, server_resp.c_str());
			fprintf(fpointer, "Response: %s\n", server_respc );

		}while(1); // client_req.compare("quit" != 0));

			fclose(fpointer);
			return 0;
		}

	void build_message(std::string& server_resp, int GPIO, unsigned int i2caddress, imu::IMU_TYPE type){
		// gather all available IMUs, get data for each

		// need a better way to track data from all the IMUs and fuse them
		imu* IMU = new imu(1, GPIO, i2caddress, type); 
		IMU->readSensorState();
		unsigned int itype = 4;

		char temp[100];
		server_resp.clear();
		switch(IMU->getIMUtype()){
			case 0: itype = 6050;  break;
			case 1: itype = 9250; break;
			case 2: itype = 8963; break;
			default: itype = 6050; break;
		}

		sprintf(temp, "%d %f %f %f %f\n", 
				itype,
				IMU->getPitch(), IMU->getYaw(),	IMU->getRoll(),
				IMU->getTemp()
		       );
		server_resp += temp;

		delete IMU;
	}

