#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "conio.h"

#define _CRT_SECURE_NO_WARNINGS
#define	BUFLEN		2000                  // ��������С
#define WSVERS		MAKEWORD(2, 2)        // ָ���汾2.2 
#pragma comment(lib,"ws2_32.lib")         // ����winsock 2.2 Llibrary

void main(int argc, char *argv[]) {
	char   *host = "127.0.0.1";	    /* server IP Address to connect */
	char   *service = "50500";      /* server port to connect       */
	struct sockaddr_in sin;	        /* an Internet endpoint address	*/
	struct sockaddr_in from;        /* sender address               */
	struct sockaddr_in toAddr;
	int    fromsize = sizeof(from);
	char*   pts;
	char   buf[BUFLEN + 1];   	    /* buffer for one line of text	*/
	char   back[BUFLEN + 1];
	SOCKET	sock;		  	        /* socket descriptor	    	*/
	int	cc;			                /* recv character count		    */
	int dd = 0,ee = 0;
	time_t now;

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);   /* ����winsock library��WSVERSΪ����汾,wsadata����ϵͳʵ��֧�ֵ���߰汾��    */
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); // ����UDP�׽���, ������������Э���(family)�����ݱ��׽��֣�UDPЭ��ţ�
	                                                 //���أ�Ҫ�����׽��ֵ���������INVALID_SOCKET
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;                     // ��(����)���еĽӿڡ�
	sin.sin_port = htons((u_short)atoi(service));         // ��ָ���ӿڡ�
														  //atoi--��asciiת��Ϊint��
														  //htons -- ������(host)ת��Ϊ������(network), Ϊshort���͡� 
	bind(sock, (struct sockaddr *)&sin, sizeof(sin));     // �󶨱��ض˿ںţ��ͱ���IP��ַ)
	printf("��������������\n");
	while (!_kbhit()) {
		cc = recvfrom(sock, buf, BUFLEN, 0, (SOCKADDR *)&from, &fromsize);
		if (cc == SOCKET_ERROR) {
			printf("recvfrom() failed; %d\n", WSAGetLastError());
			break;
		}
		else if (cc == 0)
			break;
		else {
			buf[cc] = '\0';
			if (!strcmp(buf, "Exit")) {
				break;
			}
			printf("�ͻ��˵���Ϣ��%s\n", buf);

		}
		printf("�ͻ��˵ĵ�ַ��%s\n", inet_ntoa(from.sin_addr));
		printf("�ͻ��˵Ķ˿ںţ�%d\n", from.sin_port);
		(void)time(&now);
		pts = ctime(&now);
		printf("ʱ�䣺%s\n\n", pts);
		
		 /*�����ǰ���Ϣ���ͻ�ȥ*/
		memset(&toAddr, 0, sizeof(toAddr));
		toAddr.sin_family = AF_INET;
		toAddr.sin_port = from.sin_port;
		toAddr.sin_addr = from.sin_addr;
		
		sprintf(back, "�ͻ��˵���Ϣ��%s\n�ͻ��˵ĵ�ַ��%s\n�ͻ��˵Ķ˿ںţ�%d\nʱ�䣺%s",
			buf, inet_ntoa(from.sin_addr), from.sin_port, pts);
		
		ee = sendto(sock, back, sizeof(back), 0, (SOCKADDR *)&toAddr, sizeof(toAddr));
	
	}
	closesocket(sock);
	WSACleanup();       	          /* ж��ĳ�汾��DLL */

	printf("��������˳�...");
	getchar();
}