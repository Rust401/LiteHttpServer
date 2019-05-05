#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>

#define MAXPENDING 50
#define BUFFSIZE 8192
#define PORT 4069
#define MAXIMAGESIZE 409600
#define LOGIN_ID "21714069"
#define PASSWORD "4069"

const std::string serverName = "Rust401 server\n";
const std::string filePath= "/Users/apple/RustHttpServer/docs";

std::vector<std::string> split(std::string &str,char delimeter);

void error(int sockfd);

void die(char *mess);

void send_TEXT(int sockfd, std::string absolutePath);

void send_IMAGE(int sockfd,std::string absolutePath);

void do_GET(int sockfd,std::string path);

void ans_POST(int sockfd,std::string code,bool ifLogin);

void do_POST(int sockfd,std::string login,std::string pwd);

#endif // !HTTPUTILS_H

