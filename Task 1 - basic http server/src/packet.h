#ifndef _packet_
#define _packet_

#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <map>

/* packetInfo is used in holding crucial client message info. */
struct packetInfo {
    int client; //Number of currently linked client
    int method; //Describes method from start-line
    std::string path; //Describes path to file that client asked for
    bool correctPath; //Describes if user's path is correct
    std::map<int, std::string> headers; //Describes all given valid headers
};


/* creates a simple packetInfo instance */
packetInfo createPacket();

/* cleans crucial packetInfo variables */
void cleanPacket(packetInfo &pI);

/* Analyzes given message inputMessage, so that when CRLF is reached, read line
 * is sent to parser. It also saves prevLine, if we reached end of input, but we have'nt
 * seen CRLF chars. if prevLine was not empty on function entry, it is added when line is
 * ready to be parsed, ai it reached CRLF. */
bool analyzeMessage(std::string &inputMessage, std::string &prevLine, 
                    int &state, packetInfo &pI, const std::string &basePath, 
                    const std::map<std::string, std::pair<std::string, int>> &corelatedFiles);

#endif