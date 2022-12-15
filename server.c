#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>

#define BUFSIZE 1024

//接受内容处理
// void client_data_processing(int);
void client_data_processing(void*);
//读取每行内容
int getLine(int,char*,int);
//请求行
void requestLineInfo(char*,int);
//请求头
void requestHeadInfo(char*,int);
//请求数据
void requestData(char*,int);


int main(int argc,char* argv[])
{
	if(argc != 2)
	{
		printf("input error!");
		return -1;
	}
	//创建服务端的socket
	int socket_fd = socket(AF_INET,SOCK_STREAM,0);//固定
	if(-1 == socket_fd)
	{
		perror("socket");
		return -1;
	}
	//地址、端口绑定
	//struct sockaddr一般不使用，一般使用对等的sockaddr_in（在netinet/in.h）
	struct sockaddr_in serverDressInfo;
	memset(&serverDressInfo,0,sizeof(serverDressInfo));
	//协议族，在socket编程中使用AF_INET
	serverDressInfo.sin_family = AF_INET;
	//设置任意ip
	serverDressInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	//设置端口
	serverDressInfo.sin_port = htons(atol(argv[1]));
	//进行绑定，分别为sock文件描述符，绑定信息结构体的地址，以及结构体大小
	int bind_rt = bind(socket_fd,(struct sockaddr_in*)&serverDressInfo,sizeof(serverDressInfo));	
	if(-1 == bind_rt)
	{
		perror("bind");
		return -1;
	}
	//把socket设置为监听模式
	int listen_stat = listen(socket_fd,10);
	if(-1 == listen_stat)
	{
		perror("listen");
		return -1;
	}
	//接受客户端链接
	//
	int done=1;
	while(done)
	{
		struct sockaddr_in client;
		int clientSock_fd;
		int length;
		char clientIp[64];
		socklen_t clientAddrLen;
		clientAddrLen = sizeof(client);
		clientSock_fd = accept(socket_fd,(struct sockaddr_in*)&client,&clientAddrLen);
		if(-1 == clientSock_fd)
		{
			perror("accept");
			return -1;
		}
		printf("client connect cuccess\n");
		//输出客户端信息,打印客服端IP地址和端口号
		printf("client ip: %s\t port : %d\n",inet_ntop(AF_INET, &client.sin_addr.s_addr, clientIp, sizeof(clientIp)),ntohs(client.sin_port));
		//接受、发送报文,处理
		char buf[BUFSIZE];
		int* thread_in_clientSock_fd = (int*)malloc(sizeof(int));
		*thread_in_clientSock_fd = clientSock_fd;
		pthread_t id;
		//读取客户端发送内容
		printf("start process receive info\n");
		//要是使用recv接收信息，会导致文件描述符在文件末尾，使用自己的函数getline读不到信息
		
		// if ( recv(clientSock_fd,buf,BUFSIZE,0) <= 0) 
		// {
		// 	printf("receive error");
		// 	// break;
		// }
		// else
		// {

			printf("start receive client info\n");
			printf("receive info:\n%s\n",buf);
			// client_data_processing(clientSock_fd);
			printf("process receive info finish\n");
			int ret = pthread_create(&id, NULL, client_data_processing, (void *)thread_in_clientSock_fd);
			void* value = NULL;
			pthread_join(id, &value);
			// if (ret != 0)
			// {
			// 	printf("error number: %d\n", ret);
			// 	// 根据错误号打印错误信息
			// 	printf("error information: %s\n", strerror(ret));
			// }

			// //向客户端发送响应结果
			// printf("send info to client\n");
			// strcpy(buf,"ok");
			// if ( send(clientSock_fd,buf,BUFSIZE,0) <= 0) 
			// {
			// 	perror("send");
			// 	break;
			// }
			// else
			// {
			// 	printf("send to client:%s\n",buf);
			// 	printf("finlsh\n");
			// }
		// }
	}
	// 关闭文件
	// close(clientSock_fd);
	// 关闭socket，释放资源
	close(socket_fd);
}

int getLine(int fd,char* buf,int size)
{
	memset(buf,0,size);
	int len =0;
	char flag;
	//每次一个字符，并判断
	//linux下
	while(len < size-1)//如果在此处判断‘\n’，会导致一直无法进行到下一行
	{
		int rd_n = read(fd,&flag,1);
		if(1 == rd_n)
		{
			if(flag == '\n')
			{
				break;
			}
			else
			{
				buf[len] = flag;
				++len;
				continue;
			}
		}
		if(-1 == rd_n)
		{
			perror("read");
			exit(0);
		}
	}
	buf[len] = '\0';
	return len;
}

void requestLineInfo(char* buf,int len)
{
	int flagLen = 0;
	int flag = 0;
	char method[5];
	char url[20];
	char http[10];
	memset(method, 0, sizeof(method));
	memset(url, 0, sizeof(url));
	memset(http, 0, sizeof(http));
	//请求方法
	while (!isspace(buf[flagLen]) && buf[flagLen] != '\0')
	{
		method[flag++] = buf[flagLen++];
	}
	flag = 0;
	++flagLen;
	while (!isspace(buf[flagLen]) && buf[flagLen] != '\0')
	{
		url[flag++] = buf[flagLen++];
	}
	flag = 0;
	++flagLen;
	while (!isspace(buf[flagLen]) && buf[flagLen] != '\0')
	{
		http[flag++] = buf[flagLen++];
	}
	printf("method:%s\n", method);
	printf("url:%s\n", url);
	printf("http:%s\n", http);

}

void requestHeadInfo(char* buf,int len)
{
	int flag = 0;
	while(':' != buf[flag])
	{
		++flag;
	}
	char infoName[20];
	memset(infoName,0,sizeof(infoName));
	memcpy(infoName,buf,flag);
	char info[100];
	memset(info,0,sizeof(info));
	int i = 0;
	while(flag < len &&buf[flag] != '\0')
	{
		info[i++] = buf[++flag];
	}
	printf("name:%s    ",infoName);
	printf("info:%s\n",info);
}

void requestData(char* buf,int len)
{
	//可以进行其他处理
	printf("main_info:%s\n",buf);
}

void client_data_processing(void* thread_in_clientSock_fd)
// void client_data_processing(int clientSock_fd)
{
	char buf[BUFSIZE];
	int len;
	int clientSock_fd = *(int*)thread_in_clientSock_fd;
	//请求行的信息处理
	printf("process line info\n");
	len = getLine(clientSock_fd,buf,sizeof(buf));
	printf("get %d byte\n",len);
	requestLineInfo(buf,len);
	int times = 1;
	//请求头部信息处理
	printf("begin");
	printf("process head info\n");
	while((len = getLine(clientSock_fd,buf,sizeof(buf))) > 1)
	{
		printf("time:%d\n",times);
		requestHeadInfo(buf,len);
		++times;
	}
	// len = getLine(clientSock_fd,buf,sizeof(buf));
	printf("process main info:\n");
	recv(clientSock_fd,buf,BUFSIZE,0);
	requestData(buf,len);
	//向客户端发送响应结果
	strcpy(buf,"ok");
	if ( send(clientSock_fd,buf,BUFSIZE,0) <= 0) 
	{
		perror("send");
	}
	else
	{
		printf("send to client:%s\n",buf);
		printf("finlsh\n");
	}
	close(clientSock_fd);
	if(thread_in_clientSock_fd)
	{
		free(thread_in_clientSock_fd);
	}
}
