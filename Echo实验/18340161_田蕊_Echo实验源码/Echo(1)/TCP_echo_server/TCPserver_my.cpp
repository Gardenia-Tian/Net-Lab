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
#pragma comment(lib,"ws2_32.lib")  //ʹ��winsock 2.2 library

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
	char buf_IP[BUFLEN + 1];
	char buf_PORT[BUFLEN + 1];
	unsigned short temp = 0;
	int wei = 0,wei_ = 0;

	WSAStartup(WSVERS, &wsadata);						// ����winsock library��WSVERSָ������ʹ�õİ汾��
	                                                    //wsadata����ϵͳʵ��֧�ֵ���߰汾
	msock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// �����׽��֣�������������Э���(family)�����׽��֣�TCPЭ��
														// ���أ�Ҫ�����׽��ֵ���������INVALID_SOCKET

	memset(&sin, 0, sizeof(sin));						// ��&sin��ʼ�ĳ���Ϊsizeof(sin)���ڴ���0
	sin.sin_family = AF_INET;							// ��������ַ��(INET-Internet)
	sin.sin_addr.s_addr = INADDR_ANY;					// ��������(�ӿڵ�)IP��ַ��
	sin.sin_port = htons((u_short)atoi(service));		// �����Ķ˿ںš�atoi--��asciiת��Ϊint��
	                                                    //htons--������������(host to network��s-short 16λ)
	bind(msock, (struct sockaddr *)&sin, sizeof(sin));  // �󶨼�����IP��ַ�Ͷ˿ں�

	listen(msock, 5);                                   // ��������Ϊ5������������У����ѵ������������������еȴ�����
	printf("��������������\n");
	while (!_kbhit()) {
		alen = sizeof(struct sockaddr);                         // ȡ����ַ�ṹ�ĳ���
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen); // ����������������������������
		                                                        //������������󲢽������ӣ����ظ����ӵ��׽��֣�
		                                                        //���򣬱���䱻����ֱ�����зǿա�fsin�����ͻ���IP��ַ�Ͷ˿ں�
		cc = recv(ssock, buf, BUFLEN, 0);                       // buf���汣����ǽ��յ����ַ���     
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
			printf("�յ���Ϣ��%s\n", buf);
			printf("�յ�ʱ�䣺%s\n", pts);
			int dd = send(ssock, pts, strlen(pts), 0);
			dd = send(ssock, buf, cc, 0);
		}	
		(void)closesocket(ssock);
	}
	(void)closesocket(msock);
	WSACleanup();
	printf("���س�������...");
	getchar();										// �ȴ����ⰴ��
	getchar();
}