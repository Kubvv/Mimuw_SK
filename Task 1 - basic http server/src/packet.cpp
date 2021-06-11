#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "packet.h"
#include "defs.h"
#include "writer.h"
#include "reader.h"

/* packet class is used to parse messages into packetInfo struct
 * and is responsible for oprations on those parsed informations. */

//-+-+-+-+-Private section-+-+-+-+-//

//--------Converters--------//

/* converts header name to corresponding integer value,
 * and returns this value. */
int findHeaderCode(std::string &header) {
  for (size_t i = 0; i < header.size(); i++) {
    header[i] = tolower(header[i]); //header field is not case-sensitive
  }

  if (header.compare("connection") == 0) {
    return HEADCONNECT;
  }
  else if (header.compare("content-type") == 0) {
    return HEADTYPE;
  }
  else if (header.compare("content-length") == 0) {
    return HEADLENGTH;
  }
  else if (header.compare("server") == 0) {
    return HEADSERVER;
  }
  else {
    return UNKNOWN;
  }
}

/* converts method name to corresponding integer value,
 * and returns this value. */
int findMethodCode(const std::string &method) {
  if (method.compare("GET") == 0) {
    return METHODGET;
  }
  else if (method.compare("HEAD") == 0) {
    return METHODHEAD;
  }
  else {
    return UNKNOWN;
  }
}

//--------Request handlers--------//


/* Checks if given path reaches beneath base directory or if the file does'nt exist. */
bool checkPath(const std::string &path, const std::string &basePath) {
  
  char* buff = canonicalize_file_name((basePath + path).c_str());
  if (buff == NULL) { //No file was found
    return false;
  }
  std::string cleanedPath(buff);

  return basePath.compare(cleanedPath.substr(0, basePath.size())) == 0; //if paths corelate, return true
}

/* Find the corelated server based on path. If it exists, 
 * write it's info to content string and return true, otherwise return false. */
bool findCorelated(const std::string &path, std::string &content, 
                    const std::map<std::string, std::pair<std::string, int>> &corelatedFiles) {
  auto it = corelatedFiles.find(path);
  if (it != corelatedFiles.end()) { //corelated server was found.
    content = "http://" + it->second.first + ":" + std::to_string(it->second.second) + it->first;
    return true;
  }

  return false;
}

/* Handle given request in pI and then redirect to writing an appropriate message. Returns true if 
 * message was correct and it had connection close header, otherwise returns false. */
bool handleRequest(packetInfo &pI, std::string const basePath, 
                    const std::map<std::string, std::pair<std::string, int>> &corelatedFiles) {
  bool toClose = false; //detect if user wants to end connection
  auto it = pI.headers.find(HEADCONNECT);
  if (it != pI.headers.end() && it->second.compare("close") == 0) {
    toClose = true;
  }
  
  if (!pI.correctPath) {
    writeError(pI.client, FOZF, toClose, 404);
    cleanPacket(pI); //write error message, reset pI and leave.
    return toClose; 
  }
  if (pI.method == UNKNOWN) {
    writeError(pI.client, FIZO, toClose, 501);
    cleanPacket(pI); //write error message, reset pI and leave.
    return toClose;
  }

  std::string content;
  std::string lookPath = basePath + pI.path;
  if (checkPath(pI.path, basePath) && readFile(lookPath, content)) {

    writeResult(pI.client, TWZZ, content, toClose, pI.method, 200);
    cleanPacket(pI);
    return toClose;
  }

  if (!findCorelated(pI.path, content, corelatedFiles)) {
    writeError(pI.client, FOZF, toClose, 404);
  }
  else {
    writeResult(pI.client, THZT, content, toClose, pI.method, 302);
  }


  cleanPacket(pI);
  return toClose;
}

//--------Parsers & Validators--------//

/* Checks if given char is an allowed path char. */
inline bool validateChar(const char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
          || c == '-' || c == '/' || c == '.';
}

/* Parses and validates given line, using rules from start-line. 
 * parseStartLine assigns appropiate values to pI method and pI path.
 * It returns true if parsed line was correct and false otherwise. */
bool parseStartLine(const std::string &line, packetInfo &pI) {
  std::string buff;
  size_t i = 0;

  while (i < line.size() && line[i] != ' ') {
    i++;
  }
  if (i == line.size() || i == 0) { //leave if there were no method or space char
    return false;
  }
  buff = line.substr(0, i); //save read method to buff.

  pI.method = findMethodCode(buff);

  int start = ++i; //beginning of path var.
  if (line[i] != '/') {
    return false; //leave if file doesn't begin with '/'.
  }

  while (i < line.size() && line[i] != ' ') {
    if (!validateChar(line[i])) {
      pI.correctPath = false;
    }
    i++;
  }
  if (i == line.size()) {
    return false; //we are expecting a space at some point.
  }

  pI.path = line.substr(start, i - start);
  start = ++i; //beginning of http version var.

  buff = HTTPTITLE;
  if (line.substr(start).compare(buff) != 0) { 
    return false; //check if remaining chars are HTTP/1.1\r\n.
  }

  return true;
}

/* Parses and validates given line, using rules from headers. 
 * parseHeader assigns appropiate values to pI usedHeaders and pI headers.
 * It returns true if parsed line was correct and false otherwise. */
bool parseHeader(const std::string &line, packetInfo &pI) {
  std::string header;
  std::string val;
  size_t i = 0;

  while (i < line.size() && line[i] != ':') i++;
  if (i == line.size() || i == 0) {
    return false; //check for argument value
  }
  header = line.substr(0, i++);

  while (i < line.size() && line[i] == ' ') i++; //skip to next nonspace char
  if (i == line.size() - 2) { //no chars were found
    return false;
  }
  int start = i; //beginning of field-value
  while (line[i] != ' ' && (line[i] != '\r' || line[i+1] != '\n')) i++;

  val = line.substr(start, i - start);

  while (line[i] == ' ') i++;
  if (line[i] != '\r' || line[i+1] != '\n') {
    return false; //check it there weren't any excess chars
  }

  int code = findHeaderCode(header);
  if (code == HEADLENGTH && stoi(val) > 0) { //We don't except any body message
    return false;
  }
  else if (code != UNKNOWN) {
    if (pI.headers.find(code) != pI.headers.end()) {
      return false; //this header is already present
    }
    pI.headers.insert({code, val});
  }
  return true;
}

/* Checks the state variable and decides which parser to use.
 * changes state variable according to previously parsed line.
 * returns true if parsing was succesful and false otherwise. */
bool analyzeLine(const std::string &line, int &state, packetInfo &pI, bool &isEndOfRequest) {
  bool result = true;
  
  switch(state) {
    case START_LINE:
      result = parseStartLine(line, pI);
      if (result) { //if parsing was succesfull we are waiting for Headers.
        state = HEADERS;
      }
      break;

    case HEADERS:
      if (line.size() == 2 && line[0] == '\r' && line[1] == '\n') {
        isEndOfRequest = true;
        state = START_LINE; //this configuration means end of header
      }
      else {
        result = parseHeader(line, pI);
      }
      break;

    default:
      break;
  }

  return result;
}

//-+-+-+-+-Public section-+-+-+-+-//

//--------PackageInfo--------//

packetInfo createPacket() {
    packetInfo pI;
    pI.correctPath = true; //true will be the default value
    return pI;
}

void cleanPacket(packetInfo &pI) {
  pI.correctPath = true;
  pI.headers.clear();
}

//--------Message analyzation--------//

bool analyzeMessage(std::string &inputMessage, std::string &prevLine, 
                    int &state, packetInfo &pI, const std::string &basePath, 
                    const std::map<std::string, std::pair<std::string, int>> &corelatedFiles) {
  std::string line;
  size_t start = 0; //keeps a look on start of currently considered line.
  size_t i = 0;
  if (prevLine != "") {
    inputMessage = prevLine + inputMessage;
    i = prevLine.size();
    prevLine = "";
  }
  bool isEndOfRequest = false;
  
  for (; i < inputMessage.size(); i++) {
    if (inputMessage[i] == '\n' && i != 0 && inputMessage[i-1] == '\r') { //end of line
      line = inputMessage.substr(start, i - start + 1);
      if (!analyzeLine(line, state, pI, isEndOfRequest)) {
        state = START_LINE; //if parsing wasn't successful, write error and reset.
        cleanPacket(pI);
        return writeError(pI.client, FOZZ, true, 400); //400 code always terminates connection
      }
      if (isEndOfRequest) {
        isEndOfRequest = false;
        if (handleRequest(pI, basePath, corelatedFiles)) {
          return true;
        }
      }
      start = i+1;
    }
  }

  if (start < inputMessage.size()) { //Save prevLine.
    prevLine += inputMessage.substr(start, i - start + 1);
    if (prevLine.size() > MAX_LINE_LENGTH) { //max line length was reached, exit with 400 error code
      writeError(pI.client, FOZZ, true, 400);
      return true;
    }
  }
  return false; //connection is not yet closed.
}