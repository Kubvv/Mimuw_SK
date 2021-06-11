#ifndef _echo_server_
#define _echo_server_

#include <iostream>
#include <map>

struct serverInfo {
    int portNum; //Describes the port number of the server
    std::string basePath; //Describes the base path of our server
    std::map<std::string, std::pair<std::string, int>> corelatedFiles; //Describes all corelated http servers
};

/* Creates serverInfo instance */
serverInfo createServerInfo(const std::string &basePath, const std::map<std::string, 
                            std::pair<std::string, int>> &corelatedFiles, const int portNum);

/* Runs server using information from sI */
void runServer(const serverInfo &sI);

#endif
