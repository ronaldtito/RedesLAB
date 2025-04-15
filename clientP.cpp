 /* Client code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <cstring>
  #include <string>
  #include <iostream>
  #include <sstream>
  #include <unistd.h>
  #include <vector>
  #include <thread>
  #include <fstream>
  //#include <openssl/sha.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <algorithm>  
  using namespace std;


  //PROTOCOL TO READ
  
void message_indv(int Socket, int size){
  
  char buffer[256];
  bzero(buffer,255);
  //size msg
  int n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_msg = atoi(buffer);
  //msg
  n = read(Socket,buffer,size_msg);
  buffer[n]='\0';
  string message(buffer);

  //size nick remitente
  n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_nick = atoi(buffer);
  //nick remitente
  n = read(Socket,buffer,size_nick);
  buffer[n]='\0';
  string nick(buffer);
  
  cout<<"[Message received from: "<<nick<<" ] : "<<message<<endl;
}

void message_broadcast(int Socket, int size){
  
  char buffer[256];
  bzero(buffer,255);
  //size msg
  int n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_msg = atoi(buffer);
  //msg
  n = read(Socket,buffer,size_msg);
  buffer[n]='\0';
  string message(buffer);

  //size nick origen
  n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_nick = atoi(buffer);
  //nick origen
  n = read(Socket,buffer,size_nick);
  buffer[n]='\0';
  string nick(buffer);

  
  cout<<"[Message received from: "<<nick<<" for everyone ] :"<<message<<endl;
}

void list(int Socket, int size){

  char buffer[256];
  bzero(buffer,255);
  //# clients
  //list_client
  int n = read(Socket,buffer,size-1);
  buffer[n]='\0';
  string list_client(buffer);
  

  vector<string> names;
  istringstream iss(list_client);
  
  string name;
    while(getline(iss,name,',')){
    names.push_back(name);
  }
  for(auto it:names){
    cout<<it<<endl;
  }
}

void okey(int Socket, int size){
  cout<<"Successful"<<endl;
}

void recieve_file(int Socket, int size){
  char buffer[1024];

  bzero(buffer,1023);
  
  //size origen
  int n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_origen = atoi(buffer);
  
  //origen
  n = read(Socket,buffer,size_origen);
  buffer[n]='\0';
  string origen(buffer);

  //size file_name
  n = read(Socket,buffer,5);
  buffer[n]='\0';
  int size_file_name = atoi(buffer);

  //file_name
  n = read(Socket,buffer,size_file_name);
  buffer[n]='\0';
  string file_name(buffer);
  
  //size file
  n = read(Socket,buffer,18);
  buffer[n]='\0';
  long size_file = atol(buffer);
  
  
  string contenido;

  while((long)contenido.size() < size_file){
    long bytes_to_read = min(1024L, size_file - (long)contenido.size());
    n = read(Socket,buffer,bytes_to_read);
    if (bytes_to_read <= 0) {
      break; // Error or end of file
    }
    contenido.append(buffer, n);
  }

  mkdir("recieved_files",0777);
  string path = "recieved_files/" + file_name;

  ofstream file_received(path, ios::binary);
  if(!file_received){
    perror("Error open");
    return;
  }
  file_received.write(contenido.c_str(), contenido.size());
  file_received.close();
  //cout<<"File "<<file_name<<" received from "<<origen<<endl;
  printf("File %s received from %s \n",file_name.c_str(),origen.c_str());
}


//Principal READ

  void thread_read_client(int Socket){
  	char buffer[256];
  	for(;;){
  	 	bzero(buffer,255);
  	  int n = read(Socket,buffer,20);
      buffer[n]='\0';
      int size = atoi(buffer);
      n = read(Socket,buffer,1);
      buffer[n]='\0';

      if(buffer[0]=='m'){
          message_indv(Socket,size);
      }else if (buffer[0]=='o'){
          okey(Socket,size);
      }else if (buffer[0]=='l'){
          list(Socket,size);
      }else if (buffer[0]=='b'){
          message_broadcast(Socket,size);
      }else if (buffer[0]=='f'){
        recieve_file(Socket,size);
      }

  	  if (n < 0) perror("ERROR reading from socket");   
  	}
  }

//--------------------------------------
  string i_to_s(string size_element,int size){
      string ss;
      size_t digits = size_element.length();
      if(digits < size){
        ss = string(size-digits,'0') + size_element;
      }
      return ss;
  }


  //PROTOCOL TO SEND
//-------------------------------------------
  //NICKNAME LOGIN 
  void Login(int SocketFD){
    string nickname;
    
    printf("nickname: ");
    getline(cin,nickname);

    string ss_size_nick = i_to_s(to_string(nickname.size()+1),20);

    string to_send = ss_size_nick+"N"+nickname;

    write(SocketFD,to_send.c_str(),to_send.size());
  }
  //List
  void request_list(int SocketFD){
    string to_send ="00000000000000000001L";
    int m = write(SocketFD,to_send.c_str(),to_send.size());
  }
  //Send a message to everyone
  void send_broadcast(int SocketFD){
    string message;
  	printf("Write a message for everyone:  \n");
    getline(cin,message);		

    string ss_size_message = i_to_s(to_string(message.size()),5);

    string ss_size_to_send = i_to_s(to_string(message.size() + 6),20);

    string to_send = ss_size_to_send + "B" + ss_size_message + message;
    int m = write(SocketFD,to_send.c_str(),size(to_send));
  }

  void send_message(int SocketFD){
    printf("Nickname of the recipient: ");
    string destino;
    getline(cin,destino);

    string message;
  	printf("Write a message for %s:  \n",destino.c_str());
    getline(cin,message);		

    string ss_size_destino = i_to_s(to_string(destino.size()),5);
        
    string ss_size_message = i_to_s(to_string(message.size()),5);

    string ss_size_to_send = i_to_s(to_string(destino.size() + message.size() + 11),20);

    string to_send = ss_size_to_send+"M"+ss_size_message + message + ss_size_destino + destino;
    
    int m = write(SocketFD,to_send.c_str(),size(to_send));
      
  }

  void send_file(int SocketFD){
    
    printf("Insert path of file\n");
    string path;
    getline(cin,path);
    
    ifstream file_to_send(path, ios::binary);
    if(!file_to_send){
      perror("Error open");
      return;
    }
    
    if(file_to_send.peek() == std::ifstream::traits_type::eof()){
      cout<<"File is empty"<<endl;
      return;
    }

    //guardamos el archivo en un buffer
    stringstream buffer;
    buffer << file_to_send.rdbuf();
    string file_content = buffer.str();
    file_to_send.clear();
    //file_to_send.seekg(0, ios::beg);
    

    //destino
    printf("Insert Destino\n");
    string destino;
    getline(cin,destino);

    //size file
    string ss_size_file = i_to_s(to_string(file_content.size()),18);
    //size file_name
    string ss_size_file_name = i_to_s(to_string(path.size()),5);
    //size destino
    string ss_size_destino = i_to_s(to_string(destino.size()),5);

    string ss_size_to_send = i_to_s(to_string(destino.size() + path.size() + file_content.size() + 29),20);

    //falta agreagar el hash
    cout<<ss_size_file<<endl;
    
    string to_send = ss_size_to_send + "F" + ss_size_destino + destino + ss_size_file_name + path + ss_size_file + file_content;
 

    int m = write(SocketFD,to_send.c_str(),size(to_send));
    
    /*char buffer[1000];
    
    size_t b_read;
    while(file_to_send.read(buffer,sizeof(buffer))){
      m = write(SocketFD,buffer,file_to_send.gcount()); 
    }
    file_to_send.close();*/
  }

  //Logout  
  
  void quit(int SocketFD){
    string to_send = "00000000000000000001Q";
    int m = write(SocketFD,to_send.c_str(),to_send.size());
  }

  void actions(){
    cout<<"1 - Login"<<endl;
    cout<<"2 - List"<<endl;
    cout<<"3 - Broadcast"<<endl;
    cout<<"4 - Send a Message"<<endl;
    cout<<"5 - Send a File"<<endl;
    cout<<"6 - Logout"<<endl;
    cout<<"7 - Menu"<<endl;
  }
   
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1500);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    
    thread(thread_read_client,SocketFD).detach();
    
    actions();
  
    char action;
    for(;;){
      cin>>action;
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if(action =='1')
          Login(SocketFD);
      else if(action =='2')
          request_list(SocketFD);
      else if(action =='3')
          send_broadcast(SocketFD);
      else if(action =='4')
          send_message(SocketFD);
      else if(action =='5')
          send_file(SocketFD);
      else if(action =='6'){
          quit(SocketFD);
          shutdown(SocketFD, SHUT_RDWR);
          close(SocketFD);
          return 0;
      }
      else if(action =='7')
          actions();
      else
          cout<<"not valid"<<endl;
    }
    
  }
