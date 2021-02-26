#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define	BUFLEN		5000                  // 缓冲区大小
#define WSVERS		MAKEWORD(2, 2)        // 指明版本2.2 
#pragma comment(lib,"ws2_32.lib")         // 加载winsock 2.2 Llibrary

void main(int argc, char *argv[]) {
	char	*host = "127.0.0.1";	    /* server IP to connect         */
	char	*service = "50500";  	    /* server port to connect       */
	struct sockaddr_in toAddr;	        /* an Internet endpoint address	*/
	int     toAddr_size = sizeof(toAddr);
	char	buf[BUFLEN + 1];   		    /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	cc,dd;			                    /* recv character count		    */
	char	pts[BUFLEN+1];			            /* pointer to time string	    */
	char	recv[BUFLEN + 1];

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);       /* 启动某版本Socket的DLL        */

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons((u_short)atoi(service));    //atoi：把ascii转化为int.
														// htons：主机序(host)转化为网络序(network), s--short
	toAddr.sin_addr.s_addr = inet_addr(host);           //如果host为域名，
														//需要先用函数gethostbyname把域名转化为IP地址

	printf("如果想要关闭程序请输入Exit，否则请输入想要发送的消息：\n");
	scanf("%s", buf);
	cc = sendto(sock, buf, strlen(buf), 0, (SOCKADDR *)&toAddr, sizeof(toAddr));   //送出信息
	if (cc == SOCKET_ERROR) {
		printf("发送失败，错误号：%d\n", WSAGetLastError());
	}
	
	if (strcmp(buf, "Exit")) {
		dd = recvfrom(sock, recv, BUFLEN, 0, (SOCKADDR *)&toAddr, &toAddr_size);
		if (dd == SOCKET_ERROR) {
			printf("recvfrom() failed; %d\n", WSAGetLastError());
		}
		else {
			recv[dd] = '\0';
			printf("%s\n", recv);
		}
	}
	closesocket(sock);
	WSACleanup();       	          /* 卸载某版本的DLL */

	printf("按任意键退出...");
	getchar();
	getchar();
}