#include "put_data.h"
// #include "JSON_handler.h"

SendData::SendData(){}
SendData::~SendData(){
  close(main_socket);
}

using namespace std;



int 
SendData::connect_to_server(struct in_addr* host_addr, int port){
  struct sockaddr_in serv_addr;
  int sock;
  main_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(main_socket < 0)
    throw(runtime_error("socket creation error"));
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *host_addr;
  serv_addr.sin_port = htons(port);
  
  sock = connect(main_socket, (sockaddr*) &serv_addr, sizeof(serv_addr));
  if(sock < 0)
    throw(runtime_error("could not connect to server"));
  
  return 0;
}

in_addr*
SendData::get_ip_addr(char* hostname){
  hostent* record = gethostbyname(hostname);
  if(record == NULL){
    throw(runtime_error("Could not find ip address using hostname"));
  }
  
  in_addr* addressptr = (in_addr*) record->h_addr;
  return addressptr;
}

void
SendData::select_tube(char* tube_name){
  char get[100] = "use ";
  strcat(get, tube_name);
  send_to_server(get);
  
  char* response = get_response();
  cout << response << endl;
}

void
SendData::send_to_server(char* command){
  int tmpres, data_sent;
  data_sent = 0;
  strcat(command, "\r\n");
  cout << command << endl;
  while(data_sent < strlen(command)){
    tmpres = send(main_socket, command+data_sent, strlen(command)-data_sent, 0);
    if(tmpres == -1){
      throw(runtime_error("Can't select tube"));
    }
    data_sent += tmpres;
  }
}

void 
SendData::put_data(char* data){
  char put_command[1024] = "put 0 0 60";
  char* json_command;
  char* length;
  length = new char[1024];
  json_command = strdup(data);
  sprintf(length, " %lu\r\n", strlen(json_command));
  strcat(put_command, length);
  strcat(put_command, json_command);
  send_to_server(put_command);
  char* response = get_response();
  cout << response << endl;
}

char*
SendData::get_response(){
  sleep(0.2);   // give the server time to respond
  int tmpres;
  char buf[BUFSIZ*2];
  memset(buf, 0, sizeof(buf));
  int start = 0;
  char* content;
  while((tmpres = recv(main_socket, buf, BUFSIZ*2, 0)) > 0){
    if(start == 0){
      content = buf;
      if(content != NULL){
        start = 1;
      }
    }else{
      strcat(content, buf);
    }
    if(start != 0)
      break;
  }
  
  return content;
}