#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "include/h_func.h"
#include <fstream>

#define USERAGENT "HTMLGET 1.0"
using namespace std;
bool isEntity_start=false;
//std::ofstream ofs ;
FILE* file ;
void error(const char *errorMessage)
{
    perror(errorMessage);
    exit(0);
}

/*
check response return the state of the response
from the server machine
parameters: string fl which is supposed to be the first line of the whole response message
returns:bool , 0 represent "404 not found"
              , 1 represent "200 ok"
*/

//it is possible later to modify the method so it can check other lines by providing it with integer
bool check_response(string fl){
if(fl=="HTTP/1.0 404 Not Found\r")
    return 0;

else if(fl=="HTTP/1.0 200 OK\r")
    return 1;
return 0;


}
void processMessage(char * buffer,char* page_name,int r)
{
    char temp[256];
    string f ;
    bcopy(buffer,temp,r);
    //temp[strlen(buffer)]='\0';
    char * pch;
    //bool isEntity_start=false;
//FILE* file = fopen(page_name, "w");
cout<<buffer<<endl;
    if (!isEntity_start)
    {
        pch = strtok (temp,"\n");
        f=pch;

         if (!check_response(f)) return;
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

            else cout<<f<<endl;
        }
    }
    else
    {

       // cout<<"here"<<r<<endl;


        fwrite(buffer, sizeof(unsigned char), r, file);

    }

}

int main(int argc, char *argv[])
{
    cout << "connecting to server..." << endl;
    cout << "please wait..." << endl;
    int socketFileDescriptor, portNumber, numberOfCharacters;
    struct sockaddr_in serverAddress;
    struct hostent *server;

    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "ERROR, usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portNumber = atoi(argv[2]);
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr,
                "ERROR, no such hostname, please consult system administrator\n");
        exit(0);
    }
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portNumber);
    if (connect(socketFileDescriptor, (struct sockaddr *) &serverAddress,
                sizeof(serverAddress)) < 0)
        error("ERROR connecting");





    string input;
    getline(cin,input);
    vector<string > tokens=  h_func::split(input);
    string page_name =tokens.at(1);

    //parse user command
    int w;
    char buf [256];
    page_name.copy(buf,page_name.length(),0);
    buf[page_name.length()]='\0';
    char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";

    string request_method =tokens.at(0);
    h_func::upper(&request_method);
    if (request_method==  "GET" )
    {
        cout <<"here"<<endl;
        char* query = (char *)malloc(strlen(argv[1])+page_name.length()+strlen(USERAGENT)+strlen(tpl)-5);
        sprintf(query, tpl, buf, argv[1], USERAGENT);
        cout<<query<<" "<<socketFileDescriptor<<endl;
        int w= write(socketFileDescriptor,query,strlen(query));
        cout <<w<<"hh"<<endl;

        file = fopen(buf, "w");

        int r=read(socketFileDescriptor,buffer,255);
        cout<<r<<"LLL"<<endl;
cout<<buffer<<endl;
        while(r>0)
        {

//cout<<buffer<<r<<" kikkkkkkkkkik"<<endl;
//cout<<strlen(buffer)<<r<<endl;
            processMessage(buffer,buf,r);

            bzero(buffer,256);

            r=read(socketFileDescriptor,buffer,255);


        }

    }

    else if(request_method =="POST")
    {
        char *tpl = "POST /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
        char* query = (char *)malloc(strlen(argv[1])+page_name.length()+strlen(USERAGENT)+strlen(tpl)-5);
        sprintf(query, tpl, buf, argv[1], USERAGENT);
        cout<<query<<" "<<socketFileDescriptor<<endl;

        int w= write(socketFileDescriptor,query,strlen(query));

        read(socketFileDescriptor,buffer,255);
        cout<<buffer<<endl;
        file = fopen(page_name.c_str(), "r");
         cout<<"file not found"<<endl;
        if (file==NULL) return 0;

        bzero(buffer,256);
        fseek(file, 0, SEEK_END);
        long fileLen = ftell(file);
        cout << ftell(file) << endl;
        fseek(file, 0, SEEK_SET);
        size_t bytes_read = 0;

        int i;

        long rem =fileLen;
        while(rem>0)
        {
            bzero(buffer, 256);
            bytes_read = fread(buffer, sizeof(unsigned char),255, file);
            cout<<strlen(buffer)<<endl;
            int bytes_written = write(socketFileDescriptor, buffer, bytes_read);
            rem=rem-bytes_read;


            if (bytes_written< 0)
            {
                perror("ERROR reading from socket");
                exit(1);
            }

        }



    }

    close(socketFileDescriptor);
    return 0;
}
