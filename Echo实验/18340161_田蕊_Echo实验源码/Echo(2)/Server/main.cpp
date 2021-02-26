#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "conio.h"

#define	WSVERS	MAKEWORD(2, 0)
#define _CRT_SECURE_NO_WARNINGS
#define BUFLEN 2000
#pragma comment(lib,"ws2_32.lib")  //使用winsock 2.2 library

void main(int argc, char *argv[]) {
	struct	sockaddr_in fsin;	    /* the from address of a client	  */
	SOCKET	msock, ssock;		    /* master & slave sockets	      */
	WSADATA wsadata;
	char	*service = "50500";
	struct  sockaddr_in sin;	    /* an Internet endpoint address		*/
	int	    alen;			        /* from-address length		        */
	char	*pts;			        /* pointer to time string	        */
	time_t	now;			        /* current time			            */

	int cc;
	char buf[BUFLEN + 1];
	char buf_PORT[BUFLEN + 1];
	unsigned short temp = 0;

	WSAStartup(WSVERS, &wsadata);						// 加载winsock library。WSVERS指明请求使用的版本。
														//wsadata返回系统实际支持的最高版本
	msock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// 创建套接字，参数：因特网协议簇(family)，流套接字，TCP协议
														// 返回：要监听套接字的描述符或INVALID_SOCKET

	memset(&sin, 0, sizeof(sin));						// 从&sin开始的长度为sizeof(sin)的内存清0
	sin.sin_family = AF_INET;							// 因特网地址簇(INET-Internet)
	sin.sin_addr.s_addr = INADDR_ANY;					// 监听所有(接口的)IP地址。
	sin.sin_port = htons((u_short)atoi(service));		// 监听的端口号。atoi--把ascii转化为int，
														//htons--主机序到网络序(host to network，s-short 16位)
	bind(msock, (struct sockaddr *)&sin, sizeof(sin));  // 绑定监听的IP地址和端口号

	listen(msock, 5);                                   // 建立长度为5的连接请求队列，并把到来的连接请求加入队列等待处理。
	printf("服务器已启动！\n");
	while (!_kbhit()) {
		alen = sizeof(struct sockaddr);                         // 取到地址结构的长度
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen); // 如果在连接请求队列中有连接请求，
																//则接受连接请求并建立连接，返回该连接的套接字，
																//否则，本语句被阻塞直到队列非空。fsin包含客户端IP地址和端口号
		unsigned char a = fsin.sin_addr.S_un.S_un_b.s_b1;
		unsigned char b = fsin.sin_addr.S_un.S_un_b.s_b2;
		unsigned char c = fsin.sin_addr.S_un.S_un_b.s_b3;
		unsigned char d = fsin.sin_addr.S_un.S_un_b.s_b4;

		temp = fsin.sin_port;
		sprintf(buf_PORT, "%d", temp);

		cc = recv(ssock, buf, BUFLEN, 0);                       // buf里面保存的是接收到的字符串     
		(void)time(&now);
		pts = ctime(&now);
		if (cc == SOCKET_ERROR) {
			printf("Error: %d.\n", GetLastError());
		}
		else if (cc == 0) {
			printf("Server closed!", buf);
		}
		else if (cc > 0) {
			buf[cc] = '\0';
			if (!strcmp(buf, "Exit")) {
				(void)closesocket(ssock);
				break;
			}
			printf("收到消息：%s\n", buf);
			printf("收到时间：%s", pts);
			printf("客户端IP地址：%d.%d.%d.%d\n", a, b, c, d);
			printf("客户端端口号：%d\n\n", fsin.sin_port);
			int dd = send(ssock, pts, strlen(pts), 0);
			dd = send(ssock, buf, cc, 0);
			dd = send(ssock, buf_PORT, strlen(buf_PORT), 0);

		}
		(void)closesocket(ssock);
	}
	(void)closesocket(msock);
	WSACleanup();
	printf("按回车键继续...");
	getchar();										// 等待任意按键
	getchar();
}
