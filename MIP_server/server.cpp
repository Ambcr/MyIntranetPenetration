#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>//FLIE CONTROL OPERATION 文件相关操作
#include "serverIP.h"//only define serverip

#define SERVERPORT 12347 //服务器对内网端口
#define SERVERPORT_OUT 12346 //服务器对外网端口
#define MAXBUFFER 256 //一次接收的buffer数量

void* echo_server(void* clientsocket)
{
    int m_clientsocket = *(int*)clientsocket;
    int ret;
    char readBuf[MAXBUFFER]={0};
    printf("GTD: In echo_server.");
    while((ret=read(m_clientsocket,readBuf,MAXBUFFER)))//读客户端发送的数据
    {
        if(ret > 0)
        {
            printf("GTD:receive message.\n");
            write(m_clientsocket,readBuf,MAXBUFFER);//写回客户端
            bzero(readBuf,MAXBUFFER);
        } 
        //sleep(1);
    }
    if(ret==0)
    {
        printf("客户端关闭连接\n");         
    }else
    {
        printf("read error:%s\n",strerror(errno));
    }
    close(m_clientsocket);
}
int main(int argc, char** argv)
{
    int m_socket, m_clientsocket_in,m_clientsocket_out,ret;
    socklen_t len;
    struct sockaddr_in serveraddr,clientaddr;
    char ip[40]={0};
    m_socket=socket(AF_INET,SOCK_STREAM,0);//创建socket
    if(m_socket < 0)
    {
        printf("socket error:%s\n",strerror(errno));
        exit(-1);
    }
    //设置端口可复用
    int opt = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)& opt, sizeof(opt));

    bzero(&serveraddr,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(SERVERPORT);
    inet_pton(AF_INET,SERVERIP,&serveraddr.sin_addr);//将c语言字节序转换为网络字节序
    ret=bind(m_socket,(struct sockaddr *)&serveraddr,sizeof(serveraddr));//绑定IP和端口
    if(ret!=0)
    {
        close(m_socket);
        printf("bind error:%s\n",strerror(errno));
        exit(-1);
    }
    ret=listen(m_socket,5);//监听
    if(ret!=0)
    {
       close(m_socket);
       printf("listen error:%s\n",strerror(errno));
       exit(-1);
    }
    len=sizeof(clientaddr);
    bzero(&clientaddr,sizeof(clientaddr));
    while (1)
    {
        //基本思路
        //1.首先和client建立连接，然后和互联网端口建立连接，初期版本先进行硬编码
        m_clientsocket_in = accept(m_socket, (struct sockaddr *) &clientaddr, &len);//接受内网客户端的连接
        printf("GTD: in_clientsocket connect!\n");
        // bzero(&clientaddr,sizeof(clientaddr));
        // m_clientsocket_out = accept(m_socket, (struct sockaddr *) &clientaddr, &len);//接受互联网用户的连接
        // printf("GTD: out_clientsocket connect!");
        printf("%s 连接到服务器 \n",inet_ntop(AF_INET,&clientaddr.sin_addr,ip,sizeof(ip)));
        if (m_socket < 0)
        {
            printf("accept error : %s\n", strerror(errno));
            continue;
        }
        int flags = fcntl(m_clientsocket_in, F_GETFL, 0);//获取文件的flag值
        fcntl(m_clientsocket_in, F_SETFL, flags | O_NONBLOCK);//设置文件为非阻塞状态
        printf("GTD:before create thread!\n");
        pthread_t thread;
        ret = pthread_create(&thread,NULL,echo_server,(void*)&m_clientsocket_in);
        printf("GTD: pthread_creat ret = %d,strerror = %s\n", ret, strerror(errno));
        //echo_server(m_clientsocket);

    }
    close(m_socket);
    return 0;
}
