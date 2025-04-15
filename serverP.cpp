  /* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <map>
#include <algorithm> 

using namespace std;
   
map<string,int> list_client;
//--------------------------------------
  string i_to_s(string size_element,int size){
      string ss;
      size_t digits = size_element.length();
      if(digits < size){
        ss = string(size-digits,'0') + size_element;
      }
      return ss;
  }
//-------------------------------------------
  void login(int Client,int size_nick){
    char buffer[256];
    bzero(buffer,255);
    //nick
    int n = read(Client,buffer,size_nick);
    buffer[n]='\0';
    string client_nick(buffer);
    
    if(client_nick.empty())
      perror("No Nickname");
  
    //add to list_client  Active Clients
    list_client[client_nick]=Client;
    cout<<client_nick<<" conected"<<endl;

    // Response 
    string to_send = "00000000000000000001o";
    int m = write(Client,to_send.c_str(),to_send.size());

  }

void list(int Client,int size){
  string names;
  for(auto client:list_client){
    string name = client.first +',';
    names += name;
  }
  if(names.back() == ',')
    names.pop_back();

  string ss_size_names = i_to_s(to_string(names.size()+1),20);

  string to_send = ss_size_names+"l"+names;
  cout<<"tosend_ "<<to_send<<endl;
  int m = write(Client,to_send.c_str(),to_send.size());
}

void send_broadcast(int Client,int size){
    char buffer[256];
    bzero(buffer,255);

    //size msg
    int n = read(Client,buffer,5);
    buffer[n]='\0';
    int size_msg = atoi(buffer);
    
    //msg
    n = read(Client,buffer,size_msg);
    buffer[n]='\0';
    string message(buffer);

    //convert size msg
    string ss_size_msg = i_to_s(to_string(size_msg),5);
    
    //find nick origen
    string origen;
    for(auto it:list_client){
      if(it.second == Client){
        origen = it.first;
        break; 
      }
    }
    //size nick origen
    string ss_size_origen = i_to_s(to_string(origen.size()),5);
    
    string ss_size_to_send = i_to_s(to_string(message.size() + origen.size()+11),20);
    
    //enviar a todos

    string to_send = ss_size_to_send + "b" + ss_size_msg + message + ss_size_origen + origen;
    cout<<"tosend_ "<<to_send<<endl;
    for(auto it:list_client){
      //if(it.second == Client)
        //continue;
      int m = write(list_client[it.first],to_send.c_str(),to_send.size()); 
    }
  
  }

  void send_message(int Client, int size){
    char buffer[256];
    bzero(buffer,255);
    
    //size msg
    int n = read(Client,buffer,5);
    buffer[n]='\0';
    int size_msg = atoi(buffer);
    
    //msg
    n = read(Client,buffer,size_msg);
    buffer[n]='\0';
    string message(buffer);
    
    //size nick
    n = read(Client,buffer,5);
    buffer[n]='\0';
    int size_nick = atoi(buffer);
    
    //nick destino
    n = read(Client,buffer,size_nick);
    buffer[n]='\0';
    string recipient(buffer);
    int destino = list_client[recipient];
    

    string ss_size_msg = i_to_s(to_string(size_msg),5);

    string origen;
    for(auto it:list_client){
      if(it.second == Client){
        origen = it.first;
        break; 
      }
    }

    string ss_size_origen = i_to_s(to_string(origen.size()),5);
    string ss_size_to_send = i_to_s(to_string(message.size() + origen.size()+11),20);

    string to_send = ss_size_to_send+ "m" + ss_size_msg + message + ss_size_origen + origen;

    cout<<"tosend_ "<<to_send<<endl;
    int m = write(list_client[recipient],to_send.c_str(),to_send.size());
  
  }

  void quit(int Client, int size){

    string client_nick;
    for(auto it:list_client){
      if(it.second == Client){
        client_nick = it.first;
        break; 
      }
    }
    
    list_client.erase(client_nick);
    cout<<client_nick<<" disconnected"<<endl;
    // Response
    string to_send = "00000000000000000001o";
    int m = write(Client,to_send.c_str(),to_send.size());
  }
   
  void send_file(int Client, int size){
    char buffer[3000];
    bzero(buffer,2099);

    //size origen
    int n = read(Client,buffer,5);
    buffer[n]='\0';
    int size_nick_destino = atoi(buffer);
    
    //nick origen
    n = read(Client,buffer,size_nick_destino);
    buffer[n]='\0';
    string nick_destino(buffer);
    
    //size file_name
    n = read(Client,buffer,5);
    buffer[n]='\0';
    int size_file_name = atol(buffer);
    
    //file name
    n = read(Client,buffer,size_file_name);
    buffer[n]='\0';
    string path_file(buffer);
    
    //size file
    n = read(Client,buffer,18);
    buffer[n]='\0';
    long size_file = atol(buffer);
    
    cout<<nick_destino<<endl;
    cout<<path_file <<endl;
    cout<<size_file<<endl;    
    //file
    string contenido;

    while((long)contenido.size() < size_file){
      long bytes_to_read = min(1024L, size_file - (long)contenido.size());
      n = read(Client,buffer,bytes_to_read);
      if (bytes_to_read <= 0) {
        break; // Error or end of file
      }
      contenido.append(buffer, n);
    }

    //origen
    string origen;
    for(auto it:list_client){
      if(it.second == Client){
        origen = it.first;
        break; 
      }
    }

    //size origen
    string ss_size_origen = i_to_s(to_string(origen.size()),5);

    //size file name
    string ss_size_file_name = i_to_s(to_string(path_file.size()),5);

    //size file
    string ss_size_file = i_to_s(to_string(contenido.size()),18);

    //size to send
    string ss_size_to_send = i_to_s(to_string( origen.size() + path_file.size() + contenido.size() + 29),20);
    
    string to_send = ss_size_to_send + "f" + ss_size_origen + origen + ss_size_file_name + path_file + ss_size_file + contenido;
    int m = write(list_client[nick_destino],to_send.c_str(),to_send.size());
  }

  void allways_read(int Client){
    char buffer[256];

    for(;;){
      
      bzero(buffer,255);
      int n = read(Client,buffer,20);
      buffer[n]='\0';
      int size = atoi(buffer);
      n = read(Client,buffer,1);
      buffer[n]='\0';

      if(buffer[0]=='N'){
          login(Client,size-1);
      } else if(buffer[0]=='L'){
          list(Client,size);
      } else if(buffer[0]=='B'){
          send_broadcast(Client,size);
      } else if(buffer[0]=='M'){
          send_message(Client,size);
      } else if(buffer[0]=='F'){
          send_file(Client,size);
      } else if(buffer[0]=='Q'){
          quit(Client,size);
      }

    }
    shutdown(Client, SHUT_RDWR);
    close(Client);

  }
   
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
 
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1500);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    for(;;)
    {
      int ClientST = accept(SocketFD, NULL, NULL);
 
      if(0 > ClientST)
      {
        perror("error accept failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
      }

      thread(allways_read,ClientST).detach();

    
 
     /* perform read write operations ... */
 
      
    }
 
    close(SocketFD);
    return 0;
  }
