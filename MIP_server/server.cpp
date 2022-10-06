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
#define MAXBUFFER 4096 //一次接收的buffer数量

typedef struct SocketMap{
    int sock_in_;
    int sock_out_;
}SOCK_MAP;

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
void* sock_trans_server(void* sockmap)
{
    SOCK_MAP sock_map = *(SOCK_MAP*) sockmap;
    char readBuf[MAXBUFFER]={0};
    bzero(readBuf,sizeof(readBuf));
    int ret_size;
    if((sock_map.sock_in_ < 0) || (sock_map.sock_out_ < 0))
    {
        return NULL;
    }
    printf("GTD: before server trans.\n");
    while(1)
    {
        bzero(readBuf,sizeof(readBuf));
        while((ret_size = read(sock_map.sock_out_,readBuf,MAXBUFFER)) > 0)
        {
            printf("GTD: RECEIVE NETWOEK SOCK DATA,buf = %s\n",readBuf);
            write(sock_map.sock_in_,readBuf,MAXBUFFER);
            bzero(readBuf,sizeof(readBuf));
        }
        bzero(readBuf,sizeof(readBuf));
        while((ret_size = read(sock_map.sock_in_,readBuf,MAXBUFFER) > 0))
        {
            printf("GTD: read ret size = %d\n",ret_size);
            printf("GTD: RECEIVE CLIENT SOCK DATA,buf = %s\n",readBuf);
            int retwr = write(sock_map.sock_out_,readBuf,MAXBUFFER);
            printf("GTD: write ret = %d\n",retwr);
            bzero(readBuf,sizeof(readBuf));  
        }
    }
    close(sock_map.sock_in_);
    close(sock_map.sock_out_);
    
}
int main(int argc, char** argv)
{
    int m_socket, m_clientsocket_in,m_clientsocket_out,ret;
    socklen_t len;
    struct sockaddr_in serveraddr,clientaddr,netaddr;
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
    len=sizeof(netaddr);
    bzero(&netaddr,sizeof(netaddr));

    //基本思路
    //1.首先和client建立连接，然后和互联网端口建立连接，初期版本先进行硬编码
    m_clientsocket_in = accept(m_socket, (struct sockaddr *) &clientaddr, &len);//接受内网客户端的连接,这个连接应该是常连接的。
    printf("GTD: in_clientsocket connect!\n");
    printf("%s 连接到服务器 \n",inet_ntop(AF_INET,&clientaddr.sin_addr,ip,sizeof(ip)));
    while (1)
    {
        m_clientsocket_out = accept(m_socket, (struct sockaddr *) &netaddr, &len);//接受互联网用户的连接
        printf("GTD: out_clientsocket connect!\n");
        printf("%s 连接到服务器 \n",inet_ntop(AF_INET,&netaddr.sin_addr,ip,sizeof(ip)));
        if (m_clientsocket_in < 0 || m_clientsocket_out < 0)
        {
            printf("accept error : %s, in = %d, out = %d\n", strerror(errno), m_clientsocket_in, m_clientsocket_out);
            exit(-1);
        }
        //设置内网客户端的socket为非阻塞模式
        int flags = fcntl(m_clientsocket_in, F_GETFL, 0);//获取文件的flag值
        fcntl(m_clientsocket_in, F_SETFL, flags | O_NONBLOCK);//设置文件为非阻塞状态

        //设置互联网用户的socket为非阻塞模式
        flags = fcntl(m_clientsocket_out, F_GETFL, 0);
        fcntl(m_clientsocket_out, F_SETFL, flags | O_NONBLOCK);

        SOCK_MAP sock_map = {m_clientsocket_in, m_clientsocket_out}; //创建映射对
        printf("GTD:before create thread!\n");
        pthread_t thread;
        //ret = pthread_create(&thread,NULL,echo_server,(void*)&m_clientsocket_in);
        ret = pthread_create(&thread,NULL,sock_trans_server,(void*)&sock_map);
        printf("GTD: pthread_creat ret = %d,strerror = %s\n", ret, strerror(errno));
        //echo_server(m_clientsocket);

    }
    close(m_socket);
    return 0;
}
