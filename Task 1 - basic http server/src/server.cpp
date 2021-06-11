#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <csignal>

#include "server.h"
#include "packet.h"
#include "writer.h"
#include "defs.h"

namespace {
  jmp_buf acceptJmp;
}

/* jmpFunc jumps to for loop after accept in case of broken pipe exception. 
 * man7.org confirms that using jumps is a fine way of dealing with signals. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void jmpFunc(int signum) {
  longjmp(acceptJmp, 1);
}
#pragma GCC diagnostic pop

serverInfo createServerInfo(const std::string &basePath, const std::map<std::string, std::pair<std::string, int>> &corelatedFiles, const int portNum) {
  serverInfo sI;
  sI.basePath = basePath;
  sI.corelatedFiles = corelatedFiles;
  sI.portNum = portNum;
  return sI;
}

/* echo-server is responsible for basic setup and creating the tcp server that
 * listens for clients. It is also responsible for reading and saving messages to buffer. */
void runServer(const serverInfo &sI) { 
  
  /* Server related vars */
  int sock, msgSock;
  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLen;

  int state = START_LINE; //Expecting start_line at the beggining
  char buffer[BUFFER_SIZE];
  ssize_t len;
  std::string prevLine(""); //prevLine is responsible for holding unfinished lines.
  bool closed = true;

  packetInfo pI = createPacket();

  sock = socket(PF_INET, SOCK_STREAM, 0); // creating IPv4 TCP socket
  if (sock < 0)
    exit(EXIT_FAILURE);
  // after socket() call; we should close(sock) on any execution path;
  // since all execution paths exit immediately, sock would be closed when program terminates

  serverAddress.sin_family = AF_INET; // IPv4
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
  serverAddress.sin_port = htons(sI.portNum); // listening on port portNum

  // setting up timeout info
  struct timeval timeout;
  timeout.tv_sec = 120;
  timeout.tv_usec = 0;

  // bind the socket to a concrete address
  if (bind(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    exit(EXIT_FAILURE);

  // switch to listening (passive open)
  if (listen(sock, QUEUE_LENGTH) < 0)
    exit(EXIT_FAILURE);

  signal(SIGPIPE, jmpFunc);

  for (;;) {
    clientAddressLen = sizeof(clientAddress);
    setjmp(acceptJmp);
    // get client connection from the socket
    msgSock = accept(sock, (struct sockaddr *) &clientAddress, &clientAddressLen);
    if (msgSock < 0) {
      exit(EXIT_FAILURE);
    }
    // set timeout on msgSock so that server ends connection if client is inactive
    if (setsockopt(msgSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
      writeError(msgSock, FIZZ, true, 500);
    }
    closed = false;
    pI.client = msgSock;

    /* reading loop */
    do {
      len = read(msgSock, buffer, sizeof(buffer));
      if (len > 0) {
        std::string inputMessage(buffer, len);
        closed = analyzeMessage(inputMessage, prevLine, state, pI, sI.basePath, sI.corelatedFiles); 
      }
      else if (len == 0 || //client disconnected without sending connection clode header
                (len == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))) {
        if (close(msgSock) < 0) {
            exit(EXIT_FAILURE);
          }
        break;
      }
      else if (len < 0) {
        writeError(msgSock, FIZZ, true, 500); //internal server problem, write 500 error
      }
    } while (!closed);
    cleanPacket(pI);
    prevLine = ""; //reset the incomplete line from previous client
    state = START_LINE;
  }
}
