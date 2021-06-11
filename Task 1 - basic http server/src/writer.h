#ifndef _writer_
#define _writer_

#include <iostream>

/* Writes an error message with given statusCode and description desc to
 * listener with number msg_sock. If error message was terminating (400 or connection: close), 
 * also closes the connection. writeError returns true if message was terminating
 * and false otherwise */
bool writeError(const int msg_sock, const std::string desc, const bool toClose, const int code);

/* Writes the result message with given statusCode and description, as
 * well as the bodyMessage. writeResult creates neccessary headers before writing the message */
void writeResult(const int msgSock, const std::string desc, const std::string &bodyMessage, 
                const bool toClose, const int method, const int code);

#endif
