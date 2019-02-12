#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "h_func.h"
using namespace std;
bool isEntity_start=false;
void error(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

/*
check the request GET or POST
-parameters:
request_line:the first line to get the request method
-returns :
int type determine the request method (get or post)
-string page_name :the file name to  send or receive

*/
string check_request(string request_line,int * type)
{
    vector<string > tokens=  h_func::split(request_line);
    string request_method =tokens.at(0);
    string page_name =tokens.at(1);
    string temp="";
    for(int i=1; i<page_name.length(); i++)
    {
        temp+=page_name.at(i);
    }
    page_name=temp;
    h_func::upper(&request_method);
    if (request_method=="GET")
        *type=1;
    else if (request_method=="POST")
        *type=2;
    else *type=0; //not recognized message

    return page_name;
}

/*
send the required file to the client on the socket 'sock' if the file exists
parameters:
-file :pointer to the required file
-sock:socket file desc.
*/

void send_file(string page_name,FILE * file ,int sock )
{
    char buffer[256];
    bzero(buffer,256);
    fseek(file, 0, SEEK_END);
    long fileLen = ftell(file);
    cout << ftell(file) << endl;
    fseek(file, 0, SEEK_SET);
    size_t bytes_read = 0;

    int i;
    //for (i = 0; i < fileLen; i++) {
    long rem =fileLen;
    while(rem>0)
    {
        bzero(buffer, 256);
        bytes_read = fread(buffer, sizeof(unsigned char),255, file);
        cout<<strlen(buffer)<<endl;
        int bytes_written = write(sock, buffer, bytes_read);
        rem=rem-bytes_read;
        //cout<<"rem"<<rem<<" "<<bytes_read<<endl;

        if (bytes_written< 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

    }

}

/*
receive the data from the client
this method used by POST requests
*parameters
 sockfd: socket file descriptor
 filename :the received file name
 buffer: the data (header or data)
 r :the number of read bytes
*/
void receive_file(int sockfd ,string filename,char* buffer,int r)
{

    FILE *file  =fopen(filename.c_str(),"w");

    do
    {
        if (!isEntity_start)
        {
            string f ;
            char temp[256];
            bcopy(buffer,temp,strlen(buffer));
            temp[strlen(buffer)]='\0';
            char * pch;


            pch = strtok (temp,"\n");
            f=pch;
            while (pch != NULL)
            {
                f=pch;
                pch = strtok (NULL, "\n");

                if(f=="\r"&&!isEntity_start) //the start of the content
                {
                    cout<<"kimoo"<<endl;

                    isEntity_start=true;

                }
                else if(isEntity_start)
                {

                    char buf [256];
                    f.copy(buf,f.length(),0);

                    fwrite(buf, sizeof(unsigned char), f.length(), file);
                    fwrite("\n",sizeof(unsigned char),1, file);

                }
            }

        }
        else
        {

            cout<<"here"<<r<<endl;


            fwrite(buffer, sizeof(unsigned char), r, file);

        }
        bzero(buffer,256);

        r=read(sockfd,buffer,255);

    }
    while (r>0);




}

/*
process client message and determine whether it GET or POST
parameters: buffer,the message of client
            r ,the number of read bytes from the socket 'sockfd'  message

*/
void process_client_message(char* buffer,int r,int sockfd)
{

    char temp[256];
    string f ;
    bcopy(buffer,temp,strlen(buffer));
    temp[strlen(buffer)]='\0';
    char * pch;


    pch = strtok (temp,"\n");
    f=pch;
    int type;
    string page_name= check_request(f,&type);

    if(type ==1)
    {
        cout<<"checking file :"<<page_name<<endl;

        FILE *file  =fopen(page_name.c_str(),"r");


        if (file!=NULL)
        {

            write(sockfd,"HTTP/1.0 200 OK\r\nContent-Length: 2\r\nContent-Type: text/html\r\n\r\n",
                  sizeof("HTTP/1.0 200 OK\r\nContent-Length: 2\r\nContent-Type: text/html\r\n\r\n"));
            cout<<"sending file"<<endl;
            send_file(page_name,file,sockfd);
        }

        else
        {

            write(sockfd,"HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n",
                  sizeof("HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n"));
           cout<<"file not found"<<endl;
            //return;
        }

        ///
        while (pch != NULL)
        {
            f=pch;


            if(f=="\r"&&!isEntity_start) //the start of the content
            {
                pch = strtok (NULL, "");
                break;

            }
            pch = strtok (NULL, "\n");
        }
    }
    else if (type ==2)
    {
        write(sockfd,"HTTP/1.0 200 OK\r\n\r\n",sizeof("HTTP/1.0 200 OK\r\n\r\n"));
        cout<<"receiving file...."<<endl;
        receive_file(sockfd,page_name,buffer,r);


    }
    else
    {
        write(sockfd,"HTTP/1.0 400 Bad Request\r\n\r\n",sizeof("HTTP/1.0 200 OK\r\n\r\n"));
        cout<<"bad request"<<endl;
        return;
    }



}


/*
talking with the client on "sock"

*/

void doprocessing (int sock,sockaddr_in clientAdd)
{
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(sock,buffer,255);
    cout<<"message received"<<endl;
    process_client_message(buffer,n,sock,clientAdd);

    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
    }




}


int main(int argc, char *argv[])
{
    cout << "server starting..." << endl;
    cout << "please wait..." << endl;
    int socketFileDescriptor, connectionFileDescriptor, portNumber;
    int backlog = 5;
    socklen_t clientAddressLength;
    char buffer[256];
    struct sockaddr_in serverAddress, clientAddress;
    int numberOfCharacters;
    int n, pid;

    if (argc < 2)
    {
        fprintf(stderr,
                "ERROR, missing port numbers, please enter a port number\n");
        exit(1);
    }
    //get a socket ,TCP socket working on IPV4
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
        error("ERROR opening socket");

    bzero((char *) &serverAddress, sizeof(serverAddress));
    portNumber = atoi(argv[1]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //Little or Big Endian byte order
    serverAddress.sin_port = htons(portNumber); //htons :host to network short(2 bytes)

    // bind a port to listen connections on it
    if (bind(socketFileDescriptor, (struct sockaddr *) &serverAddress,
             sizeof(serverAddress)) < 0)
        error(
            "ERROR on assigning an address for the requested socket[Binding]");


    cout << "socket connection established" << endl;

    //listen for requests to connect from cliens
    listen(socketFileDescriptor, backlog);
    clientAddressLength = sizeof(clientAddress);





    while (1)
    {
        //accept a client request and get a new socket file descriptor and let the old to listen for other requests;
        //so we have one for processing the request and one for listening
        connectionFileDescriptor = accept(socketFileDescriptor,
                                          (struct sockaddr *) &clientAddress, &clientAddressLength);
        if (connectionFileDescriptor < 0)
            error("ERROR on accept");

        /* Create child process */
        pid = fork();

        if (pid < 0)
        {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0)
        {
            /* This is the client process */
            close(socketFileDescriptor);
            doprocessing(connectionFileDescriptor,clientAddress);
            exit(0);
        }
        else
        {
            close(connectionFileDescriptor);
        }

    }



    return 0;
}
