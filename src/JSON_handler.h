#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H
#include "json/JSON_Defs.h"   //for the libJSON namespace
#include "json/jsonmain.h"    //for JSONNode
#include <iostream> //for cout
#include <string>   //for string

class JSONHandler{
public:
  const char* to_json(char* data);
private:
  char* format_for_sending(char* subject);
};
#endif