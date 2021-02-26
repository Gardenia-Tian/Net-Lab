/* TCPClient.cpp */
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>
#include <Ws2tcpip.h>//Ӧ�ð���netbd.h�����������unix�µģ�������Ws2tcpip.h
#define	BUFLEN		20<<10                  // ��������С
#define WSVERS		MAKEWORD(2, 0)        // ָ���汾2.0 
#pragma comment(lib,"ws2_32.lib")         // ʹ��winsock 2.0 Llibrary

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
	addrinfo* host;              //Ҫ���ӵ���Ϣ
	HANDLE    thread;
	SOCKET*   p;
	unsigned  threadID;
	int       is_succ = 0;
	int       len = 0;

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //����winsock library��WSVERSΪ����İ汾��wsadata����ϵͳʵ��֧�ֵ���߰汾
  
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
    closesocket(sock);                             // �رռ����׽���
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // ж��winsock library
	free(buf);

    printf("���س�������...");
	getchar();										// �ȴ����ⰴ��
}
