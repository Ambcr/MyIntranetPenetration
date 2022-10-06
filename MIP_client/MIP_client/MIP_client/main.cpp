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
#define LOCALHOST_IP "127.0.0.1"

#define LOCALHOST_PORT 8000
#define SERVER_PORT 12347
#define MAX_BUFFER 256

int main() {
	LOG(LOG_INFO, "Main Thread Start!");
	int client_socket,local_socket, ret,ret_forlocal;
	struct sockaddr_in serveraddr,localaddr;
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

	local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建套接字
	if (local_socket == -1)
	{
		LOG(LOG_FATAL, "local_socket create failed");
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	memset(&localaddr, 0, sizeof(localaddr));

	serveraddr.sin_family = AF_INET;//服务器地址及port
	serveraddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP, &serveraddr.sin_addr);

	localaddr.sin_family = AF_INET;//本地地址及port
	localaddr.sin_port = htons(LOCALHOST_PORT);
	inet_pton(AF_INET, LOCALHOST_IP, &localaddr.sin_addr);

	ret = connect(client_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));//连接到服务器
	ret_forlocal = connect(local_socket, (struct sockaddr*)&localaddr, sizeof(localaddr));//连接本地端口
	if (ret != 0 || ret_forlocal !=0)
	{
		printf("GTD: connect ret %d,ret_forlocal %d\n", ret,ret_forlocal);
		closesocket(client_socket);
		closesocket(local_socket);
		LOG(LOG_FATAL, "GTD: cannot connect to server || local.");
		return -1;
	}
	//设置socket为非阻塞模式
	unsigned long ul = 1;
	ret = ioctlsocket(client_socket, FIONBIO, (unsigned long*)&ul);
	ret_forlocal = ioctlsocket(local_socket, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR || ret_forlocal == SOCKET_ERROR)//设置失败。
	{
		LOG(LOG_ERROR, "GTD:error set ioctlsocket,ret = %d,ret_forlocal = %d", ret, ret_forlocal);
	}
	LOG(LOG_INFO, "GTD:BEFORE TRANS.");
	while (1)
	{

		memset(buf, 0, sizeof(buf));
		while (recv(client_socket, buf, sizeof(buf), 0) > 0)
		{
			LOG(LOG_INFO, "GTD: RECEIVE CLIENT_SOCKET AND SEND TO LOCAL.");
			send(local_socket, buf, sizeof(buf), 0);
			memset(buf, 0, sizeof(buf));
		}
		memset(buf, 0, sizeof(buf));
		while (recv(local_socket, buf, sizeof(buf), 0) > 0)
		{
			LOG(LOG_INFO, "GTD: REVEIVE LOCAL_SOCKET AND SEND TO SERVER.");
			send(client_socket, buf, sizeof(buf), 0);
			memset(buf, 0, sizeof(buf));
		}
	}
	closesocket(client_socket);
	closesocket(local_socket);
	return 0;
}