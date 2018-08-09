// g++ -o myClient client.c -w
//To run:    ./myClient <X coordinate> <Y coordinate> <Z coordinate>


////for reference: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

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

#define STATIC_IP "192.168.2.6"
#define PORT_NUMBER "9876"

void error(char *msg)
{
    perror(msg);
    exit(0);
}

char buf[256];
char * newPointsData(float x, float y, float z){
    sprintf(buf, "%f,%f,%f\n", x, y, z); // puts string into buffer
    return buf;
}

char buffer[256];

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    //variable serv_addr will contain the address 
    //of the server to which we want to connect.
    // It is of type struct sockaddr_in.
    struct hostent *server;
    //The variable server is a pointer to a structure of type hostent. 
    //This structure is defined in the header file netdb.h as follows:


// struct  hostent {
//         char    *h_name;        /* official name of host */
//         char    **h_aliases;    /* alias list */
//         int     h_addrtype;     /* host address type */
//         int     h_length;       /* length of address */
//         char    **h_addr_list;  /* list of addresses from name server */
// #define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
// };

// It defines a host computer on the Internet. The members of this structure are:
// h_name       Official name of the host.

// h_aliases    A zero  terminated  array  of  alternate
//              names for the host.

// h_addrtype   The  type  of  address  being  returned;
//              currently always AF_INET.

// h_length     The length, in bytes, of the address.

// h_addr_list  A pointer to a list of network addresses
//              for the named host.  Host addresses are
//              returned in network byte order.

    // if (argc < 3) {
    //    fprintf(stderr,"usage %s hostname port\n", argv[0]);
    //    exit(0);
    // }

    portno = atoi(PORT_NUMBER); 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(STATIC_IP); 

//////////
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
// This code sets the fields in serv_addr. 
//Much of it is the same as in the server. However, because the 
//field server->h_addr is a character string, we use the function:
// void bcopy(char *s1, char *s2, int length)
// which copies length bytes from s1 to s2.
///////////

////
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
// The connect function is called by the client to establish a connection to the server. 
// It takes three arguments, the socket file descriptor, the address of the host to 
// which it wants to connect (including the port number), and the size of this address. 
// This function returns 0 on success and -1 if it fails
////

    //open browser to see 
    string url =  "./webcam_stream.html";
    string op = string("open ").append(url);
    system(op.c_str());
    strcpy(buffer, newPointsData(atof(argv[1]), atof(argv[2]), atof(argv[3]))); 
    // Sends three arguments <X coordinate> <Y coordinate> <Z coordinate> to socket
    n = write(sockfd,buffer,strlen(buffer)); //write to the socket
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255); //read reply from socket
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer); //print response
    return 0;
}
