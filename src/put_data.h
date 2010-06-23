#ifndef PUT_DATA_H
#define PUT_DATA_H
#include "dependents.h"

class SendData{
public:
  SendData();
  ~SendData();
  int connect_to_server(struct in_addr* host_addr, int port);
  void put_data(char* data);
  void select_tube(char* tube_name);
  in_addr* get_ip_addr(char* hostname);
private:
  int main_socket;
  
  char* get_response();
  void send_to_server(char* command);
  char* format_for_sending(char* data);
};

#endif