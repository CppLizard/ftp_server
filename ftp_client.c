/*
 *      基于TCP的FTP客户端程序
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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int sfd;
//自定义信号处理函数
//按CTRL+C的时候被调用并给服务器发送退出消息
void doit(int n)
{
    write(sfd, "quit", 4);
    exit(1);
}

//根据服务器响应获取文件
void get_file(char msg[], char filename[])
{
    int fd;
    int rd;
    int rrd;
    int wr;
    int ac;
    int mk;
    char mkname[128] = {0};
    char buf[1024] = {0};
    char *tmp;
    if (strncmp(msg, "404", 3) == 0)
    {
        printf("没有找到文件:%s\n", filename);
        return;
    }
    if (strncmp(msg, "200", 3) == 0)
    {
        printf("找到文件:%s\n", filename);
        printf("正在下载文件...\n");
        //以读写的方式创建文件
        ac = access("download", F_OK);
        if (ac == -1)
        {
            mk = mkdir("download", 0755);
            if (mk == -1)
            {
                perror("mkdir");
                exit(1);
            }
        }
        strcpy(mkname, "./download/");
        filename = strcat(mkname, filename);
        fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0664);
        if (fd == -1)
        {
            perror("open");
            exit(1);
        }
        //读取服务器上的文件
        while ((rd = read(sfd, buf, 1024)) > 0)
        {
            tmp = buf;
            while (1)
            {
                //如果一部分数据写入失败让它继续循环完全写入
                //保证数据的完整性
                wr = write(fd, tmp, rd);
                rrd = rd - wr;
                tmp = tmp + wr;
                if (rrd == 0)
                    break;
            }
            if (buf[rd] == '\0')
                break;
            memset(buf, 0, sizeof(buf));
        }
        printf("下载完毕!\n");
    }
    return;
}

int main(int argc, char *argv[])
{
    int cfd;
    int rd;
    char filename[128] = {0};
    char msg[3] = {0};
    struct sockaddr_in server;
    //创建socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
    {
        perror("socket");
        return 1;
    }
    //对服务器的地址和端口号初始化
    server.sin_family = AF_INET;
    server.sin_port = htons(7777);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    //连接sfd到服务器地址
    cfd = connect(sfd, (struct sockaddr *)&server, sizeof(server));
    if (cfd == -1)
    {
        perror("connect");
        return 2;
    }
    while (1)
    {
        memset(filename, 0, sizeof(filename));
        //向服务器发送用户自定义信号
        signal(2, doit);
        //向服务器请求文件
        printf("客户端:\n");
        gets(filename);
        if (strncmp(filename, "quit", 4) == 0)
        {
            doit(1);
        }
        write(sfd, filename, strlen(filename) + 1);
        printf("服务器:\n");
        //接受从服务器返回的状态码
        rd = read(sfd, msg, 3);
        //根据服务器响应获取文件
        get_file(msg, filename);
    }
    close(sfd);
    return 0;
}
