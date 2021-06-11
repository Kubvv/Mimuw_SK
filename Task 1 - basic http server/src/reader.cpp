#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "reader.h"

/* Checks if given path represents a file */
bool checkIfFile(const std::string &path) {
    struct stat s;
    return stat(path.c_str(), &s) == 0 && (s.st_mode & S_IFREG);
}

bool readCorelated(const std::string &path, 
                    std::map<std::string, std::pair<std::string, int>> &corelatedFiles) {
    std::ifstream fileIn;
    std::string line;
    std::string file;
    std::string address;
    int port;
    fileIn.open(path);
    if(fileIn.fail() || !checkIfFile(path)) {
        return false;
    }

    while (getline(fileIn, line))
	{
		std::istringstream is(line); //splitting the line

        is >> file >> address >> port;
        corelatedFiles.insert({file, std::make_pair(address, port)});
	}

	fileIn.close();
    return true;
}

bool readFile(const std::string &path, std::string &content) {

    std::ifstream fileIn;
    fileIn.open(path);
    if (fileIn.fail() || !checkIfFile(path)) {
        return false;
    }

    std::stringstream strStream;
    strStream << fileIn.rdbuf(); //read the file
    content = strStream.str(); //str holds the content of the file

    fileIn.close();
    return true;
}