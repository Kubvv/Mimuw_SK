#include <iostream>
#include <map>
#include <dirent.h>
#include <stdlib.h>

#include "server.h"
#include "defs.h"
#include "reader.h"

int main(int argc, char *argv[]) {
  
  if (argc < 3) { //We are expecting at least 3 args.
    exit(EXIT_FAILURE);
  }

  char* buff = canonicalize_file_name(argv[1]);
  if (buff == NULL) { //no dir found;
    exit(EXIT_FAILURE);
  }
  std::string baseDir(buff);
  std::string corelatedPath(argv[2]);
  std::map<std::string, std::pair<std::string, int>> corelatedFiles;
  int portNum;

  /* check if given base directory exist */
  DIR* dir = opendir(baseDir.c_str());
  if (dir) {
    closedir(dir);
  }
  else if (ENOENT == errno) {
    exit(EXIT_FAILURE);
  }
  /* read the corelated file and save it to map */
  if (!readCorelated(corelatedPath, corelatedFiles)) {
    exit(EXIT_FAILURE);
  }

  /* change port if it was given */
  if (argc == 3) {
    portNum = 8080;
  }
  else {
    portNum = atoi(argv[3]);
  }

  serverInfo sI = createServerInfo(baseDir, corelatedFiles, portNum);

  runServer(sI);
  
  return 0;
}