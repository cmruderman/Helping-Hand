#include <iostream>
#include <iomanip>
#include <string.h>
#include <mutex>
#include <thread>
#include "../LeapSDK/include/Leap.h"

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
#include <fstream>

/*
This program saves to the CSV file "savedata.csv", and sends that data to the pi. Notice on line 51, we save
the coordinates of the hand to the file and multiply by 0.1 so we do not have to do so when we read this file
*/

#define STATIC_IP "192.168.1.6"
#define PORT_NUMBER "9876"

#include "../arduino-serial-lib.h"

#include "../ms_sleep.c"

using namespace Leap;

int sockfd;

// Data to send
std::mutex _mtx;
Vector _handPos;
float _handPinch;
std::ofstream output_file;


void error(char *msg)
{
    perror(msg);
    exit(0);
}

void sendData(Vector handCenter, float handPinch) {
    char buf[64];
    sprintf(buf, "%f,%f,%f,%f\n", handCenter.x*0.1, handCenter.y*0.1, -handCenter.z*0.1, handPinch);
    output_file<<buf;
    int n = write(sockfd,buf,strlen(buf)); //write to the socket
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buf,64);
    n = read(sockfd,buf,63); //read reply from socket
    buf[63] = '\0';
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n",buf); //print response
}

class EventListener : public Listener {
public:
    virtual void onConnect(const Controller&);
    virtual void onFrame(const Controller&);
};

void EventListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
}

void EventListener::onFrame(const Controller& controller) {
    const Frame frame = controller.frame();
//    std::cout << "Frame id: " << frame.id()
//        << ", timestamp: " << frame.timestamp()
//        << ", hands: " << frame.hands().count()
//        << ", fingers: " << frame.fingers().count();
    HandList hands = frame.hands();
    Hand hand = hands[0];
    
    if (!hand.isValid())
        return;
    
    
    // update hand pos
    _mtx.lock();
    _handPos = hand.palmPosition();
    _handPinch = hand.pinchStrength();
    _mtx.unlock();

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

void beginTxLoop() {
    std::thread t([](){
        std::cout << "Begin TX Loop" << std::endl;
        for(;;){
            _mtx.lock();
            sendData(_handPos, _handPinch);
            _mtx.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    t.detach();
}

void startWebcamViewing(){
    std::string url =  "./webcam_stream.html";
    std::string op = std::string("open ").append(url);
    system(op.c_str());
}

int main(int argc, char** argv) {
    std::cout << "Begin" << std::endl;
    output_file.open("savedata.csv");
    if (!output_file.is_open()) {
        std::cout << "Error";
        exit(-1);
    }
    //beginUserControlBoardCommLoop();
    
    connect_to_server();
    std::cout << "Connected to Raspberry Pi" << std::endl;
    //startWebcamViewing();

    EventListener listener;
    Controller controller;
    
    // Allow program to run in the background
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    
    controller.addListener(listener);
    
    beginTxLoop();
    
    // Keep this process running until Enter is pressed
    std::cout << "Press Enter to quit..." << std::endl;
    std::cin.get();
    
    controller.removeListener(listener);
    
    close(sockfd);
    output_file << std::flush;
    output_file.close();
    
    return 0;
}
