/* TCPServer.cpp - main */

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <time.h>
#include <windows.h>
#include <process.h>
#include <math.h>
#include "conio.h"

#define QLEN	   5
#define	WSVERS	MAKEWORD(2, 0)
#define MAX        10
#define BUFLEN    2000

CRITICAL_SECTION cs;		           // �ٽ�����ÿ��ʱ��ֻ��һ���߳̿��Խ����ٽ���

#pragma comment(lib,"ws2_32.lib")  //ʹ��winsock 2.2 library

SOCKET allssock[MAX];
HANDLE thread[MAX];   //���洴�����߳̾��

struct struparam {             /* ���ڴ������ */
	int threadno;            /* �̱߳�� */
	char buf[BUFLEN + 1];
	sockaddr_in fsin;
};

unsigned __stdcall echo(void* p) {
	EnterCriticalSection(&cs);
	struparam* p1 = (struparam *)p;
	int index = p1->threadno;
	LeaveCriticalSection(&cs);

	char buf_back[BUFLEN + 1];
	char *pts;			        /* pointer to time string	        */
	time_t now;
	int dd = 0;

	while (1) {
		int cc = recv(allssock[index], p1->buf, BUFLEN, 0);
		(void)time(&now);                                      // ȡ��ϵͳʱ��
		pts = ctime(&now);
		
		if (cc == SOCKET_ERROR) {
			printf("Error: %d.\n", GetLastError());
			(void)closesocket(allssock[index]);
			allssock[index] = 0;
			thread[index] = 0;
			break;
		}
		else if (cc == 0) {
			(void)closesocket(allssock[index]);
			allssock[index] = 0;
			thread[index] = 0;
			break;
		}
		else if (cc > 0) {
			p1->buf[cc] = '\0';
			sprintf(buf_back, "ip:%s  port:%d\ntime:%smessage:%s\n\n", 
				inet_ntoa(p1->fsin.sin_addr), p1->fsin.sin_port, pts, p1->buf);
			printf("%s", buf_back);

			if (!strcmp(p1->buf, "Exit")) {
				(void)closesocket(allssock[index]);
				allssock[index] = 0;
				thread[index] = 0;

				for (int i = 0; i < MAX; i++) {
					if (allssock[i]) {
						dd = send(allssock[i], buf_back, strlen(buf_back), 0);
					}
				}

				sprintf(buf_back, "ip:%s  port:%d\ntime:%smessage:Leave!\n\n",
					inet_ntoa(p1->fsin.sin_addr), p1->fsin.sin_port, pts);
				printf("%s", buf_back);

				for (int i = 0; i < MAX; i++) {
					if (allssock[i]) {
						dd = send(allssock[i], buf_back, strlen(buf_back), 0);
					}
				}
				break;
			}
			else {
				for (int i = 0; i < MAX; i++) {
					if (allssock[i]) {
						dd = send(allssock[i], buf_back, strlen(buf_back), 0);
					}
				}
			}	
		}
	}
	
	return 0;
}


/*------------------------------------------------------------------------
 * main - Iterative TCP server for TIME service
 *------------------------------------------------------------------------
 */
void
main(int argc, char *argv[]) 
{
	struct	sockaddr_in fsin;	    /* the from address of a client	  */
	SOCKET	msock, ssock;		    /* master & slave sockets	      */
	WSADATA wsadata; 
	char	*service = "50520";
	struct  sockaddr_in sin;	    /* an Internet endpoint address		*/
    int	    alen;			        /* from-address length		        */
	char	*pts;			        /* pointer to time string	        */
	time_t	now;			        /* current time			            */
	
	int        cc = 0;
	struparam  p[MAX];        //���е��̴߳�����ֻ�����
	struparam* ptr[MAX];      //ָ��p
	int        thcnt = 0;             //�߳�����
	int        index = 0;             //�����ҵ��ĵ�һ��Ϊnull��thread��Ԫ��
	unsigned   threadID[MAX] = { 0 }; //�����thread��Ӧ�ĵ�Ԫ���̺߳�
	char buf[BUFLEN + 1] = { 0 };

	InitializeCriticalSection(&cs);   // �ٽ�����ʼ��

	WSAStartup(WSVERS, &wsadata);						// ����winsock library��WSVERSָ������ʹ�õİ汾��wsadata����ϵͳʵ��֧�ֵ���߰汾
	msock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// �����׽��֣�������������Э���(family)�����׽��֣�TCPЭ��
														// ���أ�Ҫ�����׽��ֵ���������INVALID_SOCKET

	memset(&sin, 0, sizeof(sin));						// ��&sin��ʼ�ĳ���Ϊsizeof(sin)���ڴ���0
	sin.sin_family = AF_INET;							// ��������ַ��(INET-Internet)
	sin.sin_addr.s_addr = INADDR_ANY;					// ��������(�ӿڵ�)IP��ַ��
	sin.sin_port = htons((u_short)atoi(service));		// �����Ķ˿ںš�atoi--��asciiת��Ϊint��htons--������������(16λ)
	bind(msock, (struct sockaddr *)&sin, sizeof(sin));  // �󶨼�����IP��ַ�Ͷ˿ں�

	listen(msock, 5);                                   // �ȴ��������ӵĶ��г���Ϊ5

	for (int i = 0; i < MAX; i++) {
		thread[i] = 0;
		allssock[i] = 0;
		p[i].threadno = -1;
	}

    while(!_kbhit()){ 		                             // ����Ƿ��а���
       alen = sizeof(struct sockaddr);                   // ȡ����ַ�ṹ�ĳ���
	   ssock = accept(msock, (struct sockaddr *)&fsin, &alen); // ������µ��������󣬷��������׽��֣����򣬱�������fsin�����ͻ���IP��ַ�Ͷ˿ں�

	   for (int i = 0; i < MAX; i++) {
		   if (thread[i] == 0) {
			   index = i;
			   break;
		   }
	   }
	   allssock[index] = ssock;
	   p[index].threadno = index;
	   p[index].fsin = fsin;
	   ptr[index] = &p[index];

	   (void)time(&now);                                      // ȡ��ϵͳʱ��
	   pts = ctime(&now);
	   sprintf(buf, "ip:%s  port:%d\ntime:%smessage:Enter\n\n",
		   inet_ntoa(fsin.sin_addr), fsin.sin_port, pts);
	   printf("%s", buf);
	   for (int i = 0; i < MAX; i++) {
		   if (allssock[i]) {
			   cc = send(allssock[i], buf, strlen(buf), 0);
		   }
	   }
	   thread[index] = (HANDLE)_beginthreadex(NULL, 0, &echo, (void*)ptr[index], 0, &threadID[index]);
    }
    (void) closesocket(msock);                                 // �رռ����׽���
     WSACleanup();                                             // ж��winsock library

	 DeleteCriticalSection(&cs);  // ɾ���ٽ���
	 
	 HANDLE temp[MAX];
	 for (int i = 0; i < MAX; i++) {
		 if (thread[i]) {

			 WaitForSingleObject(thread[i], INFINITE);
			 CloseHandle(thread[i]);
		 }
	 } 
}



/* (void) time(&now);                                      // ȡ��ϵͳʱ��
	  pts = ctime(&now);                                      // ��ʱ��ת��Ϊ�ַ���
	  (void) send(ssock, pts, strlen(pts), 0);                // �ѻ�����(ָ��,����)�����ݷ��ͳ�ȥ
	  printf("%s", pts);                                      // ��ʾ�����ַ���
	  (void) closesocket(ssock);                              // �ر������׽���*/