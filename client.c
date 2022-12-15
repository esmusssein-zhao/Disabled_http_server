#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 1024


int main(int argc,char* argv[])
{
	if(argc != 3)
	{
		printf("input ip and port!");
		return -1;
	}
	//创建服务端的socket
	int socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == socket_fd)
	{
		perror("socket");
		return -1;
	}
	//向服务端发起连接清求
	struct hostent* h;
	// 指定服务端的ip
	if ((h = gethostbyname(argv[1])) == 0 )
	{
		printf("gethostbyname failed.\n"); 
		close(socket_fd); 
		return -1; 
	}
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	// 指定服务端的通信端口
	servaddr.sin_port = htons(atoi(argv[2]));
	memcpy(&servaddr.sin_addr,h->h_addr,h->h_length);
	// 向服务端发起连接清求
	if (connect(socket_fd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
	{ 
		perror("connect"); 
		close(socket_fd); 
		return -1; 
	}
	char buf[BUFSIZE];
	memset(buf,0,sizeof(BUFSIZE));
	//与服务端通信
	int request_fd = open("request.txt",O_RDONLY);
	read(request_fd,buf,BUFSIZE);
	// 向服务端发送请求报文
	if (send(socket_fd,buf,BUFSIZE,0) <= 0) 
	{
		perror("send"); 
	}
	else
	{
		printf("------send info-------%s\n",buf);
		memset(buf,0,sizeof(BUFSIZE));
		strcpy(buf,"send finish");
		printf("------waiting receive-------%s\n",buf);
		//接收返回信息
		if (recv(socket_fd,buf,BUFSIZE,0) <= 0) 
		{ 
			perror("recv");
		}
		else
		{
			printf("-------------end-----------\n%s\n",buf);
		}
	}
	close(socket_fd);
}	
