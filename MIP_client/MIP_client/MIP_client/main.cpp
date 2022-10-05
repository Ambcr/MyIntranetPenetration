#include <stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<ws2tcpip.h>
#include"serverIP.h" //ONLY DEFINE SERVER_IP
//网络通信需要包含的头文件、需要加载的库文件
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
#include "LOG_UTILS.h"


#define SERVER_PORT 12347
#define MAX_BUFFER 256

int main() {
	LOG(LOG_INFO, "Main Thread Start!");
	int client_socket, ret;
	struct sockaddr_in serveraddr;
	char buf[MAX_BUFFER];
	//网络通信的初始化
	WSADATA data;
	ret = WSAStartup(MAKEWORD(1, 1), &data); //1.1版本的协议
	if (ret)
	{
		return -1;
	}
	LOG(LOG_INFO, "GTD: WSAStartup successfull.");
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建套接字
	if (client_socket == -1)
	{
		LOG(LOG_FATAL, "client_socket create failed");
	}
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serveraddr.sin_addr);
	ret = connect(client_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));//连接到服务器
	if (ret != 0)
	{
		printf("GTD: connect ret %d\n", ret);
		closesocket(client_socket);
		LOG(LOG_FATAL, "GTD: cannot connect to server.");
		return -1;
	}
	while (1)
	{
		memset(buf, 0, sizeof(buf));
		scanf("%s", buf);
		printf("GTD: get scanf string : %s\n", buf);
		send(client_socket, buf, sizeof(buf),0);
		memset(buf, 0, sizeof(buf));
		recv(client_socket, buf, sizeof(buf),0);
		printf("ECHO:%s\n", buf);
	}
	closesocket(client_socket);
	return 0;
}