#ifndef _reader_
#define _reader_

#include <iostream>
#include <map>

/* reads the corelated http file under path var, parses it and saves
 * values under prepared map */
bool readCorelated(const std::string &path, 
                    std::map<std::string, std::pair<std::string, int>> &corelatedFiles);

/* reads the file under path var and copies it's result to content */
bool readFile(const std::string &path, std::string &content);

#endif