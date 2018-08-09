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
#include <chrono>
#include <cmath>

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

FILE * fp_log = NULL;

std::chrono::high_resolution_clock::time_point time_0;


void loop();
void stop();
void startCameraStream();
int read_ethernet_data(int sock);
void inverse_kinematics(float x_target, float y_target, float z_target, float pinch);
void inverse_kinematics2(float px, float py, float pz, float pinch);
void send_to_arduino(float thetaB_deg, float thetaE_deg, float thetaS_deg, float pinch);


void error(const char *msg)
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
	
	
	// Uncomment to log data to file
	//fp_log = fopen("./log.csv", "w");
	//time_0 = std::chrono::high_resolution_clock::now();
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int enable = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
		error("setsockopt(SO_REUSEADDR) failed");
	}
    bzero((char *) &serv_addr, sizeof(serv_addr)); //sets all values in buffer to zero
    portno = PORT_NUM; //port number on which server will listen
    //for connections needs to be passed in!
    serv_addr.sin_family = AF_INET;//first field of serv_adder- should always be set to the symbolic constant AF_INET
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    	error("ERROR on binding");
    listen(sockfd,5);//listen on the socket for connections
    clilen = sizeof(cli_addr);
    //this is where we will start the camera and open up the ui
    startCameraStream();
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            while(read_ethernet_data(newsockfd) == 0);
        }
        else close(newsockfd);
    } /* end of while */
}

void startCameraStream(){
  system("sudo service motion restart");
  system("sudo service motion start");
}

void loop() {
	//read data from computer via ethernet
	//perform inv kin
	//transmit to arduino
	//read_ethernet_data();
	// send_to_arduino();
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
	
	if (fp_log != NULL) {
		//static int t = 0;
		auto current_time = std::chrono::high_resolution_clock::now();
		int t = std::chrono::duration_cast<std::chrono::milliseconds>(
			current_time - time_0
		).count();
	
		fprintf(fp_log, "%d, %s",t, debug_buf);
		fflush(fp_log);
	}
	
}

int read_ethernet_data(int sock)
{
   int n;
   //server reads characters from the socket connection into this buffer.
      
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) {
	   perror("ERROR reading from socket\n");
	   return n;
   }
   printf("%s\n", buffer);
    
    
   //arduino_debug();
    
    
    
   /*std::string socket_str(buffer);
   std::string x_coordinate = socket_str.substr(socket_str.find("X")+2, socket_str.find("Y")-socket_str.find("X")-2); 
   cout << x_coordinate <<endl;
   std::string y_coordinate = socket_str.substr(socket_str.find("Y")+2, socket_str.find("Z")-socket_str.find("Y")-2); 
   cout << y_coordinate<<endl;
   std::string z_coordinate = socket_str.substr(socket_str.find("Z")+2, socket_str.length()-socket_str.find("Z")); 
   cout <<"'"<< z_coordinate<< "'"<<endl;
   inverse_kinematics(::atof(x_coordinate.c_str()), ::atof(y_coordinate.c_str()), ::atof(z_coordinate.c_str()));*/
   
   float x = ::atof(strtok (buffer,","));
   float y = ::atof(strtok (NULL,","));
   float z = ::atof(strtok (NULL,","));
   float pinch = ::atof(strtok (NULL,",")); // % of hand closed, from 0.0 (fully open) to 1.0 (fully closed)
    
    // NAN signals Emergency stop
    if (::isnan(x) || ::isnan(y) || ::isnan(z)) {
        send_to_arduino(NAN,NAN,NAN,pinch);
        n = write(sock,"EMG Stop Received",17);
        if (n < 0) {
            perror("ERROR writing to socket\n");
            return n;
        }
        return 0;
    }
    
   //z = z + 65;
   //y = y - 25;
   //x*=1.5;
   //y*=1.5;
   //z*=1.5;
   
   //cout << "x: " << x << " y: " << y << " z: " << z <<endl;
   

   
   inverse_kinematics(z,y,x,pinch);     // Jacob's inverse kin
   //inverse_kinematics2(x,y,z,pinch);  // Joshua's inverse kin
   
   n = write(sock,"I got your message",18);
   if (n < 0) {
	   perror("ERROR writing to socket\n");
	   return n;
   }
   
   return 0;
}

void send_to_arduino(float thetaB_deg, float thetaE_deg, float thetaS_deg, float pinch) {
	
	float toSend[4];
    
    toSend[0] = thetaB_deg;
    toSend[1] = thetaE_deg;
    toSend[2] = thetaS_deg;
    toSend[3] = pinch;
    
    //send base, shoulder, elbow
    std::string angles = std::to_string(thetaB_deg)+","+std::to_string(thetaE_deg)+","+std::to_string(thetaS_deg)+".";
    const char *angle_ptr = angles.c_str();
    //printf("Elbow Degrees: %f Shoulder Degrees: %f\n", thetaE_deg, thetaS_deg);
    //printf("X:%fY:%f\n\n", x_target, y_target);
    
    std::cout << "Sending to arduino: \n" << toSend[0] << " " << toSend[1] << " " << toSend[2] << " " << toSend[3] << std::endl;
	serialport_write(arduino_fd, (const uint8_t*)toSend, 16);
	
}

void inverse_kinematics(float x_target, float y_target, float z_target, float pinch) {
    printf("X: %f Y: %f Z: %f\n", x_target, y_target, z_target);
    float L1 = 48.5; //48.5 cm long, shoulder to elbow
    float L2 = 73;//73 cm long, elbow to wrist
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
    
    //thetaB_deg = 180-(thetaB_deg-75)/(106-75)*(165-20)-25;
    thetaB_deg = (thetaB_deg);
    if (thetaB_deg < 0) { 
		thetaB_deg+=180;
	}
    
    if (isnan(thetaB_deg) || isnan(thetaE_deg) || isnan(thetaS_deg)) {
		cerr << "angle is NAN, skipping" << endl;
		return;
	}
    
    send_to_arduino(thetaB_deg, thetaE_deg, thetaS_deg, pinch);
}





//-------------TEST------------------


void inverse_kinematics2(float px, float py, float pz, float pinch) {

    // Distance from base to first joint
    float d1 = 0.0; //7.6;

    // Length from shoulder to elbow
    float L2 = 59.f;

    // Length from elbow to gripper
    float L3 = 58.f;

//-------------------------------------------------------   
// Calculate Elbow angle

    float c3 = (px*px + py*py + (pz-d1)*(pz-d1) - L2*L2 - L3*L3) / (2*L2*L3);
    
    // if p is out of workspace
    if (c3 < -1 || c3 > 1) {
        std::cerr << "p is out of workspace" << std::endl;
        return;// float3(INVALID);
    }
    
    float s3 = sqrt(1 - c3);  // +/-

    // Elbow joint angle
    float q3 = atan2(s3, c3);

//------------------------------------------------------- 
// Calculate Base angle

    // infinite solutions
    if (px*px + py*py == 0) {
        return;// float3(INVALID);
    }

    float c1 = px / sqrt(px*px + py*py);
    float s1 = py / sqrt(px*px + py*py);

    // Base angle (from right)
    float q1 = atan2(py,px);

//------------------------------------------------------- 
// Calculate Shoulder angle

    float c2 = (L2*c1*px - L3*d1*s3 + L2*py*s1 + L3*pz*s3 + L3*c1*c3*px + L3*c3*py*s1)/(L2*L2 + 2*L2*L3*c3 + L3*L3*c3*c3 + L3*L3*s3*s3);
    float s2 = (L2*d1 - L2*pz + L3*c3*d1 - L3*c3*pz + L3*py*s1*s3 + L3*c1*px*s3)/(L2*L2 + 2*L2*L3*c3 + L3*L3*c3*c3 + L3*L3*s3*s3);


    // Shoulder angle
    float q2 = atan2(s2, c2);

//------------------------------------------------------- 
// Convert to degrees

    float base_angle     = 180 - q1 *(180/M_PI);
    float shoulder_angle = 90  + q2 *(180/M_PI);
    float elbow_angle    = 180 - q3 *(180/M_PI);

//------------------------------------------------------- 
// Correct the range

	if (base_angle > 180.0)
		base_angle -= 360.0;
		
	if (base_angle < -180.0)
		base_angle += 360.0;

	if (shoulder_angle > 180.0)
		shoulder_angle -= 360.0;
		
	if (shoulder_angle < -180.0)
		shoulder_angle += 360.0;

	if (elbow_angle > 180.0)
		elbow_angle -= 360.0;
		
	if (elbow_angle < -180.0)
		elbow_angle += 360.0;

//-------------------------------------------------------

	send_to_arduino(base_angle, elbow_angle, shoulder_angle, pinch);

}
