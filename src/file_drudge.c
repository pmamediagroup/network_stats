#include "file_drudge.h"

using namespace std;

char*
FileDrudge::read_file(char* filename){
  char ch;
  char* file_data;
  file_data = (char*)malloc(FILE_SIZE_T); // 50k
  
  if(!filename){
    throw(runtime_error("Please provide a file to be read"));
  }
  
  ifstream in(filename, ios::in | ios::binary);
  if(!in){
    throw(runtime_error("Cannot open file for reading"));
  }
  int i = 0;
  while(in.get(ch)){
    if(in){
      file_data[i] = ch;
      i++;
    }
  }
  in.close();
  free(in);
  return file_data;
}

// int main(){
//   FileDrudge fd;
//   char* file_name;
//   char* file_data;
//   file_data = (char*)malloc(51200);
//   file_name = (char*)malloc(1024);
//   strcpy(file_name, "src/file_drudge.h");
//   file_data = fd.read_file(file_name);
//   cout << file_data << endl;
//   free(file_data);
//   free(file_name);
//   return 0;
// }