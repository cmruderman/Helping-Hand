#include <stdlib.h>
#include "arduino-serial-lib.h"
#include <stdio.h>
#include <sys/types.h> 
// This header file contains definitions of a 
// number of data types used in system calls. 
// These types are used in the next two include files.

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <math.h>
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define PORT_NUM 9876

int arduino_fd;
char buffer[256];
int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
float thetaE_deg;
float thetaS_deg;
float thetaB_deg;

void stop();
void startCameraStream();
void inverse_kinematics(float x_target, float y_target, float z_target, float pinch);
void send_to_arduino(float B, float E, float S, float pinch);
void arduino_debug();

FILE * fp_log;

inline void sleep_ms(int ms) {
	usleep(ms*1000);
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int main(int argc, char** argv){
    if(argc != 1 && argc != 2){
        std::cout << "Usage: " << argv[0] << " [/dev/ttyACM[X]]" << std::endl;
        return -1;
    }
    
    if (argc == 1)
		arduino_fd=serialport_init_auto(9600); // auto-detect Arduino
	else
		arduino_fd=serialport_init(argv[1], 9600); // Manually specify
		
	// If couldn't connect to Arduino
	if (arduino_fd < 0) {
		error("Cannot connect to Arduino! Exiting...");
	}
	
	fp_log = fopen("./base.csv", "w");
	
	cout << "Ready" << endl;
	
	float pinch=0.0;

	thetaE_deg=84.0;
	thetaS_deg=58.0;
	thetaB_deg=90.0;
	
	
//	for (;thetaB_deg<180.0;) {
//		send_to_arduino(thetaB_deg,thetaE_deg,thetaS_deg,pinch);
//		sleep_ms(100);
//		thetaB_deg++;
//	}
	
	
	while (true) {
		send_to_arduino(thetaB_deg,thetaE_deg,thetaS_deg,pinch);
		
		for (int i=0;i<10;i++)
			arduino_debug();
			
		sleep_ms(10);
		
	}
}


void stop() {
	serialport_flush(arduino_fd);
	serialport_close(arduino_fd);
	exit(0);
}

void arduino_debug() {
	
	char debug_buf[32];
	debug_buf[31] = '\0';
	serialport_read_until(arduino_fd, debug_buf, '\n', 31, 100);
	std::cout<< "Arduino says: " << debug_buf << std::endl;
	
	static int t = 0;
	
	fprintf(fp_log, "%d, %s",t++, debug_buf);
	fflush(fp_log);
	
}

void send_to_arduino(float B, float E, float S, float pinch) {
	
	float toSend[4];
    
    toSend[0] = B;
    toSend[1] = E;
    toSend[2] = S;
    toSend[3] = pinch;
    
    //send base, shoulder, elbow
    std::string angles = std::to_string(thetaB_deg)+","+std::to_string(thetaE_deg)+","+std::to_string(thetaS_deg)+".";
    const char *angle_ptr = angles.c_str();
    //printf("Elbow Degrees: %f Shoulder Degrees: %f\n", thetaE_deg, thetaS_deg);
    //printf("X:%fY:%f\n\n", x_target, y_target);
    
    //std::cout << "Sending to arduino: \n" << toSend[0] << " " << toSend[1] << " " << toSend[2] << " " << toSend[3] << std::endl;
	serialport_write(arduino_fd, (const uint8_t*)toSend, 16);
	
}

void inverse_kinematics(float x_target, float y_target, float z_target, float pinch) {
    printf("X: %f Y: %f Z: %f\n", x_target, y_target, z_target);
    float L1 = 59; //58.5 cm long, shoulder to elbow
    float L2 = 58; //58 cm long, elbow to wrist
    float y_target_squared = pow(y_target,2);
    float z_target_squared = pow(z_target,2);
    
    float thetaB = atan2f(x_target,z_target);
    
    x_target = sqrtf(x_target*x_target + z_target_squared);
    float x_target_squared = pow(x_target,2);
    
    float L1_squared = pow(L1,2);
    float L2_squared = pow(L2,2);
    float num = (L1_squared + x_target_squared + y_target_squared - L2_squared);
    float den = (2*L1*sqrt(x_target_squared+y_target_squared));
    float thetaS = acos(num/den) + atan2(y_target,x_target);
    float thetaE = acos((L1_squared+L2_squared-x_target_squared-y_target_squared)/(2*L1*L2));
    
    thetaE_deg = thetaE/(M_PI/180.f);
    thetaS_deg=thetaS/(M_PI/180.f);
    thetaB_deg = thetaB/(M_PI/180.f); 
    
    thetaB_deg = 180-(thetaB_deg-75)/(106-75)*(165-20)-25;
    
    if (isnan(thetaB_deg) || isnan(thetaE_deg) || isnan(thetaS_deg)) {
		cerr << "angle is NAN, skipping" << endl;
		return;
	}
    
    
	send_to_arduino(thetaB_deg,thetaE_deg,thetaS_deg,pinch);
    
}


