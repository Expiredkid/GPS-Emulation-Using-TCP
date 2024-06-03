#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")//引入ws2_32.lib库,使用Winsock的程序必须连接此文件

#define DEFAULT_IP "192.168.86.129"// 服务器ip
#define DEFAULT_PORT 56789 // 服务器监听端口

int main(){
    printf("begin\n");

    //初始化
    WSADATA wsaData;
    int iResult;        //存储初始化的输出结果
    iResult = WSAStartup(MAKEWORD(2,2),&wsaData);
    if(iResult !=0){
        printf("WSAStartuo 失败：%d\n",iResult);
        return 1;
    }

    //创建套接字
    SOCKET client_socket = socket(AF_INET,SOCK_STREAM,0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket 创建失败\n");
        return 1;
    }

    // 设置服务器地址和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    server_addr.sin_addr.s_addr = inet_addr(DEFAULT_IP);

    // 连接到服务器
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("连接失败\n");
        return 1;
    }

    // 生成并发送模拟GPS数据
    while (1) {
        char *gps_data = "$GPGGA,115542.000,3155.3446,N,11852.4283,E,1,03,4.4,32.6,M,5.4,M,,0000*5A";  // 生成NMEA-0183格式的GPS数据
        send(client_socket, gps_data, strlen(gps_data), 0);
        Sleep(1000);  // 休眠1秒
    }

    closesocket(client_socket);
    WSACleanup();   //反初始化
    
    printf("end\n");
    return 0;
}