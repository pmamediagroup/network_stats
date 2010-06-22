#include "JSON_handler.h"

const char*
JSONHandler::to_json(char* type, char* data){
  JSONNode* root = libJSON::NewNode("", "{}");
  JSONNode* name = root->AddNewStringChild("name", "one");
  JSONNode* value = root->AddNewChild("value", "1");
  string jsoned = root->Write();
  
  return jsoned.c_str();
}


char*
JSONHandler::format_for_sending(char* unformatted){
  char* pch; 
  char formatted[4096];
  pch = strtok(unformatted, "\n");
  
  while(pch != NULL){
    // cout << "line " << pch << endl;
    strcat(formatted, pch);
    cout << formatted << endl;
    pch = strtok(NULL, "\n");
  }
  cout << formatted << endl;
  return strdup(formatted);
}
// 
// 
// int main(){
//   JSONHandler handle;
//   std::cout << handle.json_parse("this is some data\nYAAA") << std::endl;
// }