#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_IP "192.168.86.129"// ip
#define DEFAULT_PORT 56789 // 服务器监听端口
#define buffer_len 124     //缓存区大小

typedef struct {
    char gpgga_head[7];               //GPGGA头部段
    char utc_time[7];                 //时间，hhmmss（时分秒）格式
    char latitude[10];                //纬度，ddmm.mmmm（度分）格式（前面的0 也将被传输）
    char latitude_hemisphere;         //N或S，表示维度半球
    char longitude[11];               //经度，dddmm.mmmm（度分）格式（前面的0 也将被传输）
    char longitude_hemisphere;        //E或W，表示经度半球
    int gps_status;                   //GPS 状态，0=未定位，1=非差分定位，2=差分定位，6=正在估算 
    char satellites_used[3];          //正在使用解算位置的卫星数量（00-12）（前面的0 也将被传输）
    float hdop;                       //HDOP水平精度因子（0.5-99.9）
    float altitude;                   //海拔高度（-9999.9-99999.9）
    float geoid_height;               //地球椭球面相对大地水准面的高度
    float differential_time;          //差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
    char differential_station_id[5];  //差分站ID 号0000-1023（前面的0 也将被传输，如果不是差分定位将为空）
    int hash;                         //校验和
} GPGGAData;

void parseGPGGA(char *buffer,GPGGAData *gpgga){
    char *temp,*s;
    char copy[buffer_len];
    strcpy(copy, buffer);
    s = copy;

    // 使用逗号作为分隔符
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    strncpy(gpgga->gpgga_head, temp, 6);
    gpgga->gpgga_head[6] = '\0';

    temp = strsep(&s, ",");
    if (temp == NULL) return;
    strncpy(gpgga->utc_time, temp, 6);
    gpgga->utc_time[6] = '\0';

    temp = strsep(&s, ",");
    if (temp == NULL) return;
    strncpy(gpgga->latitude, temp, 9);
    gpgga->latitude[9] = '\0';

    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->latitude_hemisphere = temp[0];

    temp = strsep(&s, ",");
    if (temp == NULL) return;
    strncpy(gpgga->longitude, temp, 10);
    gpgga->longitude[10] = '\0';

    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->longitude_hemisphere = temp[0];

    //<6>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->gps_status = atoi(temp);

    //<7>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    strncpy(gpgga->satellites_used, temp, 10);
    gpgga->satellites_used[2] = '\0';

    //<8>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->hdop = atof(temp);

    //<9>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->altitude = atof(temp);

    //M
    temp = strsep(&s, ",");
    if (temp == NULL) return;

    //<10>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    gpgga->geoid_height = atof(temp);

    //M
    temp = strsep(&s, ",");
    if (temp == NULL) return;

    //<11>
    temp = strsep(&s, ",");
    if (temp == NULL) return;
    if (strlen(temp) == 0)
        gpgga->differential_time = -1;
    else
        gpgga->differential_time = atof(temp);

    //<12>
    temp = strsep(&s, "*");
    if (temp == NULL) return;
    strncpy(gpgga->differential_station_id, temp, 4);
    gpgga->differential_station_id[4] = '\0';

    //校验和
    gpgga->hash = strtol(copy,&temp,16);
}

void printGPGGA(GPGGAData *gpgga){
    printf("\n------------------------------\n");
    printf("时间：%s\n",gpgga->utc_time);
    printf("纬度：%s\n",gpgga->latitude);
    printf("纬度半球：%c\n",gpgga->latitude_hemisphere);
    printf("经度：%s\n",gpgga->longitude);
    printf("经度半球：%c\n",gpgga->longitude_hemisphere);
    printf("GPS 状态：%d\n",gpgga->gps_status);
    printf("使用的卫星数量：%s\n",gpgga->satellites_used);
    printf("HDOP 水平精度因子：%f\n",gpgga->hdop);
    printf("海拔高度：%.1lf\n",gpgga->altitude);
    printf("地球椭球面相对大地水准面的高度：%f\n",gpgga->geoid_height);
    printf("差分时间：%f\n",gpgga->differential_time);
    printf("差分站ID：%s\n",gpgga->differential_station_id);
    printf("------------------------------\n");
}

int main() {
    int valread;
    char buffer[buffer_len] = {0};
    char *data;
    GPGGAData gpgga;

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
        valread = read(client_socket, buffer, buffer_len);

        parseGPGGA(buffer,&gpgga);
        printGPGGA(&gpgga);

    }

    // 关闭套接字
    close(client_socket);
    close(server_socket);

    return 0;
}
