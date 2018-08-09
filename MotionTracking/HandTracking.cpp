#include <iostream>
#include <iomanip>
#include <string.h>
#include <mutex>
#include <thread>
#include <algorithm>
#include "Leap.h"

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
#include <math.h>
#include <fstream>

#define STATIC_IP "192.168.1.6"
#define PORT_NUMBER "9876"


//********************************************
//*************** Leap Scaling ***************
#define LEAP_X_CENTER    0.0
#define LEAP_Y_CENTER   30.0
#define LEAP_Z_CENTER    0.0

#define LEAP_X_SCALE    25.0
#define LEAP_Y_SCALE    20.0
#define LEAP_Z_SCALE    25.0

#define ROBOT_X_CENTER   0.0
#define ROBOT_Y_CENTER  25.0
#define ROBOT_Z_CENTER  80.0

#define ROBOT_X_SCALE   75.0
#define ROBOT_Y_SCALE   50.0
#define ROBOT_Z_SCALE   30.0

#define SCALE 1.0

#define X_SCALE ((ROBOT_X_SCALE/LEAP_X_SCALE*SCALE))
#define Y_SCALE ((ROBOT_Y_SCALE/LEAP_Y_SCALE*SCALE))
#define Z_SCALE ((ROBOT_Z_SCALE/LEAP_Z_SCALE*SCALE))
//********************************************


#include "./arduino-serial-lib.h"

#include "./ms_sleep.c"

using namespace Leap;

int sockfd = -1;
FILE * output_file = NULL;
unsigned int delay=50000; 

// Data to send, Leap State
struct {
    std::mutex mtx;
    Vector handPos;
    float handPinch;
} _leap;

// Control board state
struct {
    std::mutex mtx;
    bool emergency_stop = false;
    int potentiometer   = 0;
    int pause_resume    = 1;
    int power           = 1;
} _controller;


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void printState() {
    
    std::cout << "\n\033[5A";
    
    std::cout << "     X          Y          Z        Pinch    \n";
    std::cout << "---------------------------------------------\n";
    
    _leap.mtx.lock();
    std::cout << std::setprecision(5) << std::left << "|  "
    << std::setw(8) << _leap.handPos.x << "|  "
    << std::setw(8) << _leap.handPos.y << "|  "
    << std::setw(8) << _leap.handPos.z << "|  "
    << std::setprecision(3) << std::setw(6) << _leap.handPinch*100 << "% |"
    << "\n";
    _leap.mtx.unlock();
    
    std::cout << "---------------------------------------------\n";
    
    _controller.mtx.lock();
    std::cout
        << "Emergency stop: "           << std::setw(1) << _controller.emergency_stop
        << ",  Potentiometer: "         << std::setw(3) << _controller.potentiometer
        << ",  Pause\\Resume: "         << std::setw(2) << _controller.pause_resume
        << ",  Power: "                 << std::setw(2) << _controller.power;
    _controller.mtx.unlock();
    
    std::cout << std::flush;
}

void sendPlaybackData(std::string buf) {
    int n = write(sockfd,buf.c_str(),buf.size()); //write to the socket
    if (n < 0)
        error("ERROR writing to socket");
}

void sendData(Vector handCenter, float handPinch) {
    //std::cout << std::setprecision(5) << "\r"  << handCenter.x << " " << handCenter.y << " " << handCenter.z << " " << handPinch << std::flush;

    char buf[64];
    sprintf(buf, "%f,%f,%f,%f\n", handCenter.x, handCenter.y, handCenter.z, handPinch);
    if(output_file!=NULL){
        fprintf(output_file, "%s", buf);
    }
    int n = write(sockfd,buf,strlen(buf)); //write to the socket
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buf,64);
    n = read(sockfd,buf,63); //read reply from socket
    buf[63] = '\0';
    if (n < 0)
        error("ERROR reading from socket");
    //printf("%s\n",buf); //print response
}

class EventListener : public Listener {
public:
    virtual void onConnect(const Controller&);
    virtual void onFrame(const Controller&);
};

void EventListener::onConnect(const Controller& controller) {
    std::cout << "Connected\n\n\n\n\n" << std::endl;
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
    _controller.mtx.lock();
    bool should_update = (!_controller.emergency_stop && _controller.power && _controller.pause_resume);
    _controller.mtx.unlock();
    
    if (should_update) {
        _leap.mtx.lock();
        Vector handPos = hand.palmPosition();
        _leap.handPos.x = (-handPos.x*0.1 - LEAP_X_CENTER) * X_SCALE + ROBOT_X_CENTER;
        _leap.handPos.y = ( handPos.y*0.1 - LEAP_Y_CENTER) * Y_SCALE + ROBOT_Y_CENTER;
        _leap.handPos.z = (-handPos.z*0.1 - LEAP_Z_CENTER) * Z_SCALE + ROBOT_Z_CENTER;
        _leap.handPinch = hand.pinchStrength();
        _leap.mtx.unlock();
    }

}

void connect_to_server() {
    int portno;
    
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
    if (sockfd < 0) {
        perror("Warning! No connection to Raspberry Pi!\n");
        return;
    }

    std::thread t([](){
        std::cout << "Begin TX Loop" << std::endl;
        for(;;){
            _leap.mtx.lock();
            sendData(_leap.handPos, _leap.handPinch);
            _leap.mtx.unlock();
            printState();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    t.detach();
}

int beginUserControlBoardCommLoop() {
    
    int fd = ::serialport_init_auto(9600);
    if (fd < 0) {
        perror("No User Control Board Detected");
        return -1; // failure
    }
    
    std::thread t([fd](){
        std::cout << "Begin User Control Board Comm Loop" << std::endl;

        char buf[32]; buf[31] = '\0';
        int timeout = 1;
        int read_num_chars = 4;
        char until = '\n';
        
        for (;;) {
            if ( ::serialport_read_until(fd, buf, until, read_num_chars, timeout) == 0 ) {
                _controller.mtx.lock();
                
                // emergency stop
                if (buf[0] == '!') {
                    //std::cout << "Emergency stop" << std::endl;
                    _controller.emergency_stop = true;
                    _controller.potentiometer  = -1;
                    _controller.pause_resume   = -1;
                    _controller.power          = -1;
                    
                    // Open the gripper on EMG stop
                    _leap.mtx.lock();
                    _leap.handPos = Vector(NAN, NAN, NAN);
                    _leap.handPinch = 0;
                    _leap.mtx.unlock();
                }
                // get state
                else if (buf[0] == '@') {
                    _controller.emergency_stop = false;
                    _controller.potentiometer  = (unsigned char)buf[1];
                    _controller.pause_resume   = (unsigned char)buf[2];
                    _controller.power          = (unsigned char)buf[3];
                    
                    //std::cout << "\rPotentiometer: " << potentiometer << ",  Pause\\Resume: " << pause_resume << ",  Power: " << power << std::flush;
                    
                    // Set the arm back to it's default position on power-off
                    if (_controller.power == false) {
                        _leap.mtx.lock();
                        _leap.handPos = Vector(0,-23,26);
                        _leap.handPinch = 0;
                        _leap.mtx.unlock();
                    }
                    
                    // Pause all movement
                    if (_controller.pause_resume == false) {
                        _leap.mtx.lock();
                        _leap.handPos.x = NAN; // NAN signals a stop
                        _leap.mtx.unlock();
                    }
                }
                
                _controller.mtx.unlock();
            } else {
                //std::cerr << "Couldn't read from User Control Board: " << buf << std::endl;
            }
            
            //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
    });
    t.detach();
    return 0; // success
}

void startWebcamViewing(){
    std::string url =  "./webcam_stream.html";
    std::string op = std::string("open ").append(url);
    system(op.c_str());
}

char* getCmdOption(char** begin, char** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int main(int argc, char** argv) {
    
    std::cout
        << "\n"
        << "*****************************************\n"
        << "****** Helping Hand Control System ******\n"
        << "*****************************************\n"
    << std::endl;
    
    beginUserControlBoardCommLoop();
    
    if (!cmdOptionExists(argv, argv+argc, "--no-server")) {
        connect_to_server();
        std::cout << "Connected to Raspberry Pi" << std::endl;
    } else {
        // Print State loop when not using a server
        std::thread t([](){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            for (;;) {
                printState();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
        t.detach();
    }
    
    if (cmdOptionExists(argv, argv+argc, "--webcam")) {
        startWebcamViewing();
    }

    if (cmdOptionExists(argv, argv+argc, "--record")) {
        output_file = fopen("./savedata.csv", "w");
    }
    else if (cmdOptionExists(argv, argv+argc, "--playback")) {

        std::thread t([](){

            std::ifstream infile("savedata.csv");
            std::vector<std::string> lines_in_file;
            std::string line;

            if(!infile.is_open()) {
                error("No Playback file found :(");
            }

            while(std::getline(infile,line))
            {
                lines_in_file.push_back(line); //make file into vector
            }
            infile.close();
            for(;;){     
                //loop through vector 
                for(std::vector<std::string>::const_iterator i = lines_in_file.begin(); i != lines_in_file.end(); ++i) {
                    sendPlaybackData(*i);
                    usleep(delay);
                }
            } 

        });
        t.detach();

        // Keep this process running until Enter is pressed
        std::cout << "Press Enter to quit...\n" << std::endl;
        std::cin.get();
        return 0;
    }

    EventListener listener;
    Controller controller;
    
    // Allow program to run in the background
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    
    controller.addListener(listener);
    
    beginTxLoop();
    
    // Keep this process running until Enter is pressed
    std::cout << "Press Enter to quit...\n" << std::endl;
    std::cin.get();
    
    controller.removeListener(listener);
    
    close(sockfd);
    if(output_file!=NULL){
        fclose(output_file);
    }
    
    return 0;
}
