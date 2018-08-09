#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include "../arduino-serial-lib.h"
#include "../ms_sleep.c"
#include <iterator>
#include <fstream>

/*
This program reads from the CSV file "savedata.csv", and sends that data to the pi. Notice on line 49, 
that we do not multiply the coordinate's by .1 because we have already done so when saving to our .csv file.
*/

#define STATIC_IP "192.168.1.6"
#define PORT_NUMBER "9876"

int sockfd;

unsigned int delay=50000; // .01 second delay

using namespace std;
 
vector<vector<float> > rectangular_prism;
ifstream infile("savedata.csv");
float x_coordinate, y_coordinate, z_coordinate, hand_pinched;
std::vector<std::string> lines_in_file;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void sendPlaybackData(std::string buf) {
    int n = write(sockfd,buf.c_str(),buf.size()); //write to the socket
    if (n < 0)
        error("ERROR writing to socket");
}

void connect_to_server() {
    int portno, n;
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    portno = atoi(PORT_NUMBER);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    
    server = gethostbyname(STATIC_IP);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
}

int main(int argc, char** argv) {

    std::cout << "Begin" << std::endl;
    connect_to_server();
    std::cout << "Connected to Raspberry Pi" << std::endl;
    std::string line;
    while(std::getline(infile,line) && infile.is_open())
    {
        lines_in_file.push_back(line); //make file into vector
    }
    infile.close();
    for(;;){     
        //loop through vector 
        for(vector<string>::const_iterator i = lines_in_file.begin(); i != lines_in_file.end(); ++i) {
            sendPlaybackData(*i);
            usleep(delay);
        }
    }   
    close(sockfd);
    return 0;
}