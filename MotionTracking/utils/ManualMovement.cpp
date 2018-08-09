#include <iostream>
#include <iomanip>
#include <string.h>
#include <mutex>
#include <thread>
#include <algorithm>

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
#include <termios.h>


#define STATIC_IP "192.168.1.6"
#define PORT_NUMBER "9876"

int sockfd = -1;

std::mutex mtx;
float x = 0.0, y = 300.0, z = 700.0, pinch = 1.0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
    perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror ("tcsetattr ~ICANON");
    return (buf);
}

void printState() {
    
    std::cout << "\n\033[5A";
    
    std::cout << "     X          Y          Z        Pinch    \n";
    std::cout << "---------------------------------------------\n";
    
    //mtx.lock();
    std::cout << std::setprecision(5) << std::left << "|  "
    << std::setw(8) << x/10 << "|  "
    << std::setw(8) << y/10 << "|  "
    << std::setw(8) << z/10 << "|  "
    << std::setprecision(3) << std::setw(6) << pinch*100 << "% |"
    << "\n";
    //mtx.unlock();
    
    std::cout << "---------------------------------------------\n";
    
    std::cout << std::flush;
}

void sendData() {
    char buf[64];
    
    //mtx.lock();
    sprintf(buf, "%f,%f,%f,%f\n", x*0.1, y*0.1, -z*0.1, pinch);
    //mtx.unlock();
    
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
            sendData();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    t.detach();
}



int main() {
    
    connect_to_server();
    beginTxLoop();
    
    for (;;) {
        printState();
        char key = getch();
        //mtx.lock();
        switch (key) {
            // X +/-
            case 'q':
                x+=10;
            break;
            case 'a':
                x-=10;
            break;
            
            // Y +/-
            case 'w':
                y+=10;
            break;
            case 's':
                y-=10;
            break;
            
            // Z +/-
            case 'e':
                z+=10;
            break;
            case 'd':
                z-=10;
            break;
            
            // gripper +/-
            case 'r':
            if (pinch<=0.91)
                pinch+=0.1;
            break;
            case 'f':
            if (pinch>=0.09)
                pinch-=0.1;
            break;
            
            // Press ` to quit
            case '`':
                close(sockfd);
                return 0;
            break;
            
            default:
            break;
        }
        //mtx.unlock();
    }
    
    return 0;
}
