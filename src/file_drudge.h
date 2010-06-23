#ifndef FILE_DRUDGE_H
#define FILE_DRUDGE_H

#include "dependents.h"

#define FILE_SIZE_T 51200

class FileDrudge {
public:
  char* read_file(char* filename);
};

#endif