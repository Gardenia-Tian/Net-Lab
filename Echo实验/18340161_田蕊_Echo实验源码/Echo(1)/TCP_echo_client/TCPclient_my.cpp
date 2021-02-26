#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define	BUFLEN		2000                  // 缓冲区大小
#define WSVERS		MAKEWORD(2, 0)        // 指明版本2.0 
#pragma comment(lib,"ws2_32.lib")         // 使用winsock 2.0 Llibrary


void main(int argc, char *argv[]) {
	char	*host = "127.0.0.1";	    /* server IP to connect         */
	char	*service = "50500";  	    /* server port to connect       */
	struct sockaddr_in sin;	            /* an Internet endpoint address	*/
	char	buf[BUFLEN + 1];   		    /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	cc;			                    /* send character count		    */
	int dd;                             /* recv character count         */
	int ee;
	char pts[BUFLEN + 1];
	char buf_r[BUFLEN + 1];
	
	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //加载winsock library。
	                                                      //WSVERS为请求的版本，
	                                                      //wsadata返回系统实际支持的最高版本
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);     //创建套接字

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(host);
	sin.sin_port = htons((u_short)atoi(service));
	int ret = connect(sock, (struct sockaddr *)&sin, sizeof(sin));

	printf("如果想要关闭程序请输入Exit，否则请输入需要发送的消息：");
	scanf("%s", buf);
	cc = send(sock, buf, strlen(buf), 0);                 //发送字符串
	dd = recv(sock, pts, BUFLEN, 0);                      //收到字符串，首先受到的是时间
	ee = recv(sock, buf_r, BUFLEN, 0);
	
	if (dd == SOCKET_ERROR)                               // 出错。其后必须关闭套接字sock
		printf("Error: %d.\n", GetLastError());
	else if (dd == 0) {                                   // 对方正常关闭
		printf("Server closed!\n", pts);
	}
	else if (dd > 0) {
		pts[dd] = '\0';	                                  // ensure null-termination
		printf("\n收到的消息：\n"); 
		printf("%s", pts);                              // 显示所接收的字符串
	}

	if (ee == SOCKET_ERROR)                               // 出错。其后必须关闭套接字sock
		printf("Error: %d.\n", GetLastError());
	else if (dd > 0 && ee == 0) {                                 // 对方正常关闭
		printf("Server closed!", buf_r);
	}
	else if (ee > 0) {
		buf_r[ee] = '\0';
		printf("%s\n", buf_r);                            // 显示所接收的字符串
	}

	closesocket(sock);                                    // 关闭监听套接字
	WSACleanup();                                         // 卸载winsock library

	printf("按回车键继续...");
	getchar();										      // 等待任意按键
	getchar();
}


