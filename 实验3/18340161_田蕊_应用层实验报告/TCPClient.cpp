/* TCPClient.cpp */
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>
#include <Ws2tcpip.h>//应该包含netbd.h，但是这个是unix下的，所以用Ws2tcpip.h
#define	BUFLEN		20<<10                  // 缓冲区大小
#define WSVERS		MAKEWORD(2, 0)        // 指明版本2.0 
#pragma comment(lib,"ws2_32.lib")         // 使用winsock 2.0 Llibrary

CRITICAL_SECTION cs;
unsigned __stdcall show(void* p) {
	SOCKET* p1 = (SOCKET *)p;
	char buf[BUFLEN + 1] = { 0 };
	while (1) {
		int rc = recv(*p1, buf, BUFLEN, 0);
		if (rc > 0) {
			buf[rc] = '\0';
			printf(">> %s", buf);
		}
	}
	
	return 0;
}
/*------------------------------------------------------------------------
 * main - TCP client for TIME service
 *------------------------------------------------------------------------
 */

void
main(int argc, char *argv[])
{
	//char*	  buf = (char *)malloc(BUFLEN);   	        /* buffer for one line of text	*/
	char      buf[BUFLEN];
	SOCKET	  sock;		  	            /* socket descriptor	    	*/
	int	      cc;			                    /* recv character count		    */
	addrinfo* host;              //要连接的信息
	HANDLE    thread;
	SOCKET*   p;
	unsigned  threadID;
	int       is_succ = 0;
	int       len = 0;

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //加载winsock library。WSVERS为请求的版本，wsadata返回系统实际支持的最高版本
  
	if (argc < 3) {
		return;
	}
	getaddrinfo(argv[1], argv[2], NULL, &host);
	//getaddrinfo("www.sysu.edu.cn", "80", NULL, &host);
	sock = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	is_succ = connect(sock, host->ai_addr, host->ai_addrlen);
	if (is_succ) {
		printf("Connect failed\n");
		return;
	}
	InitializeCriticalSection(&cs);
	p = &sock;
	thread = (HANDLE)_beginthreadex(NULL, 0, &show, (void *)p, 0, &threadID);

	while(1) {
		printf("Please input the message(end by EOF, Ctrl+Z in Windows) :\n");
		char* p1 = buf;
		len = 0;
		memset(buf, 0, BUFLEN);
		while (gets_s(p1, BUFLEN)) {
			//int a = strlen(buf);
			len += strlen(buf + len);
			buf[len++] = '\r';
			buf[len++] = '\n';
			p1 = buf + len ;
		}
		len += strlen(buf + len);
		buf[len] = '\0';
		cc = send(sock, buf, len, 0);
		if (!strcmp(buf, "Exit\r\n")) {
			CloseHandle(thread);
			printf("Leave!\n");
			break;
		}
	}
	shutdown(sock, SD_SEND);
    closesocket(sock);                             // 关闭监听套接字
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // 卸载winsock library
	free(buf);

    printf("按回车键继续...");
	getchar();										// 等待任意按键
}
