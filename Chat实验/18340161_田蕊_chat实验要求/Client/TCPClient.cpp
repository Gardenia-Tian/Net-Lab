/* TCPClient.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>

#define	BUFLEN		2000                  // ��������С
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
	WSAStartup(WSVERS, &wsadata);						  //����winsock library��WSVERSΪ����İ汾��wsadata����ϵͳʵ��֧�ֵ���߰汾
  
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	  //�����׽��֣�������������Э���(family)�����׽��֣�TCPЭ��
														  //���أ�Ҫ�����׽��ֵ���������INVALID_SOCKET
    memset(&sin, 0, sizeof(sin));						  // ��&sin��ʼ�ĳ���Ϊsizeof(sin)���ڴ���0
    sin.sin_family = AF_INET;							  // ��������ַ��
    sin.sin_addr.s_addr = inet_addr(host);                // ������IP��ַ(32λ)
    sin.sin_port = htons((u_short)atoi(service));         // �������˿ں�  
    connect(sock, (struct sockaddr *)&sin, sizeof(sin));  // ���ӵ�������

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
    closesocket(sock);                             // �رռ����׽���
	//CloseHandle(thread);
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // ж��winsock library

    printf("���س�������...");
	getchar();										// �ȴ����ⰴ��
}








//cc = recv(sock, buf, BUFLEN, 0);                // ccΪ���յ����ַ��ĸ���(>0)��Է��ѹر�(=0)�����ӳ���(<0)
	/*if(cc == SOCKET_ERROR || cc==0)
		 printf("Error: %d.\n",GetLastError());     //����������ر��׽���sock��
	else if(cc > 0) {
		 buf[cc] = '\0';	                       // ensure null-termination
		 printf("%s",buf);                         // ��ʾ�����յ��ַ���
	}*/