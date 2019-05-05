#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include "httpUtils.h"

std::vector<std::string> split(std::string& str, char delimeter)
{
    std::vector<std::string> result;
    if(str.length()==0)return result;
    int slow=0,fast=0;
    while(slow<str.length()&&fast<str.length())
    {
        if(delimeter==str[fast])
        {
            result.push_back(str.substr(slow,fast-slow));
            slow=fast+1;
        }
        ++fast;
    }
    if(str.length()==fast&&slow<fast)result.push_back(str.substr(slow));
    return result;
}

//sent the txt file to the client socket
void send_TEXT(int sockfd, std::string absolutePath)
{
    std::string str;
    std::ostringstream oss;
    oss<<"HTTP/1.1 200 OK\r\n";
    oss<<"Content-Type: text/html\r\n";
    oss<<"Content-Length:";
    std::ifstream in(absolutePath);
    if(in.is_open())
    {
        while(!in.eof())
        {
            std::string tmp;
            getline(in,tmp);
            str+=tmp;
        }
        oss<<str.size()<<"\r\n\r\n"<<str;
        str=oss.str();
        send(sockfd,str.c_str(),str.size(),0);
    }
}

//The one can't show the image
void send_IMAGE(int sclient, std::string absolutePath) 
{
    int fd = open(absolutePath.c_str(), O_RDWR | O_CREAT, 0777);
    if(fd==-1)die("ERROR opening the image file\n");
    
    char* buffer=(char*)malloc(MAXIMAGESIZE);
    int n;
    if((n=read(fd,buffer,MAXIMAGESIZE))==-1)die("ERROR reading the image file\n");
    buffer[n]=0x00;
    close(fd);
    
    std::ostringstream oss;
    oss<<"HTTP/1.1 200 OK\r\n";
    oss<<"Content-Type: image/jpeg\r\n";
    oss<<"Content-Length:"<<n<<"\r\n\r\n";
    std::string response;
    response=oss.str();

    response.append(buffer,70000);
    write(sclient,response.c_str(),response.size());
    free(buffer);
}

//sent the image file to the client socket
//the crash one
void send_IMAGE1(int sclient,std::string absolutePath)
{
    FILE* fstream;
    fstream=fopen(absolutePath.c_str(),"rb");
    if(NULL==fstream)error(sclient);
    else
    {
        fseek(fstream,0L,SEEK_END);
        long size=ftell(fstream);
        fclose(fstream);
        std::ostringstream oss;
        oss<<"HTTP/1.1 200 OK\r\n";
        oss<<"Content-Type: image/jpeg\r\n";
        oss<<"Content-Length:"<<size<<"\r\n\r\n";
        std::string response;
        response.clear();
        response=oss.str();
        
        char* buffer=(char*)malloc(MAXIMAGESIZE);
        memset(buffer,0,MAXIMAGESIZE);
        int c=0;
        int j=0;

        FILE* fp;
        fp=fopen(absolutePath.c_str(),"rb");

        while((c=getc(fp))!=EOF)buffer[j++]=c;

        fclose(fp);
        response.append(buffer,j);
        send(sclient,response.c_str(),response.size(),0);
        free(buffer); 
    }
}

//the action after receive the method GET
void do_GET(int sockfd,std::string path)
{
    if(path.find(".html")!=-1)
    {
        std::string absolutePath=filePath+"/html/"+path.substr(path.find_last_of("/"));
        send_TEXT(sockfd,absolutePath);
    }
    if(path.find(".jpg")!=-1)
    {
        std::string absolutePath=filePath+"/img/"+path.substr(path.find_last_of('/'));

        //BUG HERE
        send_IMAGE(sockfd,absolutePath);
    }
    if(path.find(".txt")!=-1)
    {
        std::string absolutePath=filePath+"/txt/"+path.substr(path.find_last_of('/'));
        send_TEXT(sockfd,absolutePath);
    }
}

void do_POST(int sockfd,std::string login,std::string pwd)
{
    if(login.compare(LOGIN_ID)==0&&pwd.compare(PASSWORD)==0)ans_POST(sockfd,"200",true);
    else ans_POST(sockfd,"200",false);
}

void ans_POST(int sockfd,std::string code,bool ifLogin)
{
    std::string loginSuccess="<html><body>Success</body></html>";
    std::string loginFailure="<html><body>Fail</body></html>";
    std::ostringstream oss;
    oss<<"HTTP/1.1 200 OK\r\n";
    oss<<"Content-Type: text/html\r\n";
    oss<<"Content-Length:";
    if("200"==code)
    {
        if(ifLogin)oss<<loginSuccess.size()<<"\r\n\r\n"<<loginSuccess;
        else oss<<loginFailure.size()<<"\r\n\r\n"<<loginFailure;
        send(sockfd,oss.str().c_str(),oss.str().size(),0);
    }
}

void error(int sockfd)
{
    std::string s="404 NOT FOUND";
    std::ostringstream oss;
    oss<<"HTTP/1.1 404 Not Found\r\n";
    oss<<"Content-Type: text/html\r\n";
    oss<<"Content-Length:"<<s.length()<<"\r\n";
    oss<<s<<'\n';
    send(sockfd,oss.str().c_str(),oss.str().length(),0);
}

void die(char *mess)
{
    perror(mess);
    pthread_exit(NULL);
}