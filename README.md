基于TCP的FTP服务器程序
    客户端给服务器发送一个文件的名字，服务器收到文件的名字
    在指定的路径下查找文件是否存在
    如果不存在服务器给客户端发送404，客户端收到404，则显示找不到这个文件
    如果存在，向客户端发送200，客户端收到200，创建这个文件
    然后将这个文件内容下载到客户端的文件中
    1.access(2)     检测文件的有效性
