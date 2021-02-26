/* TCPClient.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>

#define	BUFLEN		2000                  // 缓冲区大小
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
	char	*host = "127.0.0.1";	    /* server IP to connect         */
	//char    *host = "103.26.79.35";
	char	*service = "50520";  	    /* server port to connect       */
	//char    *service = "50500";
	struct sockaddr_in sin;	            /* an Internet endpoint address	*/
	char	buf[BUFLEN+1];   	        /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	cc;			                    /* recv character count		    */

	int index = 0;
	HANDLE thread;
	SOCKET* p;
	unsigned threadID;
	//char buf_s[BUFLEN + 1] = { 0 };

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //加载winsock library。WSVERS为请求的版本，wsadata返回系统实际支持的最高版本
  
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	  //创建套接字，参数：因特网协议簇(family)，流套接字，TCP协议
														  //返回：要监听套接字的描述符或INVALID_SOCKET
    memset(&sin, 0, sizeof(sin));						  // 从&sin开始的长度为sizeof(sin)的内存清0
    sin.sin_family = AF_INET;							  // 因特网地址簇
    sin.sin_addr.s_addr = inet_addr(host);                // 服务器IP地址(32位)
    sin.sin_port = htons((u_short)atoi(service));         // 服务器端口号  
    connect(sock, (struct sockaddr *)&sin, sizeof(sin));  // 连接到服务器

	InitializeCriticalSection(&cs);
	p = &sock;
	thread = (HANDLE)_beginthreadex(NULL, 0, &show, (void *)p, 0, &threadID);

	while(1) {
		gets_s(buf);
		cc = send(sock, buf, strlen(buf), 0);
		if (!strcmp(buf, "Exit")) {
			CloseHandle(thread);
			printf("Leave!\n");
			break;
		}
	}
	//WaitForSingleObject(thread, INFINITE);
    closesocket(sock);                             // 关闭监听套接字
	//CloseHandle(thread);
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // 卸载winsock library

    printf("按回车键继续...");
	getchar();										// 等待任意按键
}








//cc = recv(sock, buf, BUFLEN, 0);                // cc为接收到的字符的个数(>0)或对方已关闭(=0)或连接出错(<0)
	/*if(cc == SOCKET_ERROR || cc==0)
		 printf("Error: %d.\n",GetLastError());     //出错。其后必须关闭套接字sock。
	else if(cc > 0) {
		 buf[cc] = '\0';	                       // ensure null-termination
		 printf("%s",buf);                         // 显示所接收的字符串
	}*/