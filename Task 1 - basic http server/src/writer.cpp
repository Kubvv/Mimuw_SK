#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "writer.h"
#include "defs.h"

/* This class is used as message builder and sender.
 * It creates correct parts of a message and sends it to
 * currently listening user. Exit failures occur here instead of
 * 500 error code as we've already sent response to user and he
 * is probably not expecting another one */

//-+-+-+-+-Private section-+-+-+-+-//

/* Adds mandatory headers to server's message */
void addMessageHeaders(std::string &message, const size_t bodySize) {
  message += TYPEHEADER;
  message += LENGTHHEADER + std::to_string(bodySize) + "\r\n\r\n";
}

void sendMessage(const int msgSock, const std::string &message) {
  ssize_t sndLen;

  //write message in parts of max 2^15 - 1 bytes for safety purposes
  std::size_t writtenLen = 0;
  while (writtenLen < message.size())
  {
    std::size_t len = std::min((std::size_t)(MAX_WRITE_LENGTH), message.size() - writtenLen);
    sndLen = write(msgSock, message.c_str() + writtenLen, len);
    if (errno == EPIPE) {
      errno = EIO;
      return;
    }
    if (sndLen != (ssize_t)len) {
      return;
    }
    writtenLen += len;
  }
}

//-+-+-+-+-Public section-+-+-+-+-//

bool writeError(const int msgSock, const std::string desc, const bool toClose, const int code) {
  std::string message = HTTPSTART + desc + SERVERHEADER;
  if (code == 400 || code == 500) {
    message += CONNECTIONHEADER;
  }
  message += "\r\n";
  sendMessage(msgSock, message);

  if (toClose) {
      if (close(msgSock) < 0) {
        exit(EXIT_FAILURE);
      }
      return true;
  }

  return false;
}

void writeResult(const int msgSock, const std::string desc, const std::string &bodyMessage, 
                  const bool toClose, const int method, const int code) {
  std::string message = HTTPSTART + desc + SERVERHEADER;
  if (code == 302) {
    message += LOCATIONHEADER + bodyMessage + "\r\n\r\n";
  }
  if (code == 200) {
    addMessageHeaders(message, bodyMessage.size());
    if (method == METHODGET) {
      message += bodyMessage;
    }
  }
  sendMessage(msgSock, message);
  if (toClose) {
    if (close(msgSock) < 0) {
      exit(EXIT_FAILURE);
    }
  }
}