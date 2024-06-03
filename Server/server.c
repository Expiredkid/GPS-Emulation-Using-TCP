#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_IP "192.168.86.129"// 服务器ip
#define DEFAULT_PORT 56789 // 服务器监听端口

int main() {
    int valread;
    char buffer[4096] = {0};

    // 创建套接字
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //设置服务端信息
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;   //接收任意地址

    // 绑定套接字到指定的端口
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 开始监听连接
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 56789...\n");

    // 接受客户端连接请求,创建客户端套接字
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // 接收并处理GPS数据
    while (1) {
        valread = read(client_socket, buffer, 4096);
        for(int i=0;i<4096;i++){
            printf("%c",buffer[i]);
        }
    }

    // 关闭套接字
    close(client_socket);
    close(server_socket);

    return 0;
}