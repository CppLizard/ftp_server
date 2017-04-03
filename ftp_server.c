/*
 *      基于TCP的FTP服务器程序
 *
 *      客户端给服务器发送一个文件的名字，服务器收到文件的名字
 *      在指定的路径下查找文件是否存在
 *      如果不存在服务器给客户端发送404，客户端收到404，则显示找不到这个文件
 *      如果存在，向客户端发送200，客户端收到200，创建这个文件
 *      然后将这个文件内容下载到客户端的文件中
 *      1.access(2)     检测文件的有效性
 * */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

int cfd;

//处理客户端请求并响应客户端
set_file(int ac, char filename[])
{
    int fd;
    int rd;
    int wr;
    char buf[1024] = {0};
    if (ac == -1)
    {
        //如果没有找到文件给客户端发送404
        write(cfd, "404", 3);
    }
    if (ac == 0)
    {
        
        //发送给客户端
        write(cfd, "200", 3);
        fd = open(filename, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            exit(1);
        }
        while ((rd = read(fd, buf, 1024)) > 0)
        {
            wr = write(cfd, buf, rd);
        }
        printf("文件%s下载成功!\n", filename);
    }
}
int main()
{
    int sfd;
    int bd;
    int ac;
    char filename[128] = {0};
    char IP[128] = {0};
    int len;
    struct sockaddr_in server;
    struct sockaddr_in client;
    //创建conket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket");
        return 1;
    }
    //对服务器的地址和端口号初始化
    server.sin_family = AF_INET;
    server.sin_port = htons(7777);
    server.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY本机上的所有IP地址
    //绑定服务器的地址和端口号到通讯描述符
    bd = bind(sfd, (struct sockaddr *)&server, sizeof(server));
    if (bd == -1)
    {
        perror("bind");
        return 2;
    }
    //在sfd描述符上监听连接
    listen(sfd, 5);
    while (1)
    {
        //阻塞等待客户端连接请求
        len = sizeof(client);
        cfd = accept(sfd, (struct sockaddr *)&client, &len);
        if (cfd == -1)
        {
            perror("accept");
            return 3;
        }
        printf("IP:%s\n", inet_ntop(AF_INET, &client.sin_addr, IP, 128));
        while (1)
        {
            //获取客户端数据
            memset(filename, 0, sizeof(filename));
            read(cfd, filename, 128);
            if (strncmp(filename,"quit", 4) == 0)
            {
                break;
            }
            //查找指定目录下有没有客户端发送过来的文件
            ac = access(filename, R_OK|F_OK);
            //向客户端发送文件
            set_file(ac, filename);
        }
        //关闭连接
        close(cfd);
    }
    close(sfd);
    return 0;
}
