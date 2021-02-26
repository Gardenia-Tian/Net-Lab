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

CRITICAL_SECTION cs;		           // 临界区。每个时刻只有一个线程可以进入临界区

#pragma comment(lib,"ws2_32.lib")  //使用winsock 2.2 library

SOCKET allssock[MAX];
HANDLE thread[MAX];   //保存创建的线程句柄

struct struparam {             /* 用于带入参数 */
	int threadno;            /* 线程编号 */
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
		(void)time(&now);                                      // 取得系统时间
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
	struparam  p[MAX];        //所有的线程创建都只传这个
	struparam* ptr[MAX];      //指向p
	int        thcnt = 0;             //线程总数
	int        index = 0;             //保存找到的第一个为null的thread单元号
	unsigned   threadID[MAX] = { 0 }; //保存和thread对应的单元的线程号
	char buf[BUFLEN + 1] = { 0 };

	InitializeCriticalSection(&cs);   // 临界区初始化

	WSAStartup(WSVERS, &wsadata);						// 加载winsock library。WSVERS指明请求使用的版本。wsadata返回系统实际支持的最高版本
	msock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// 创建套接字，参数：因特网协议簇(family)，流套接字，TCP协议
														// 返回：要监听套接字的描述符或INVALID_SOCKET

	memset(&sin, 0, sizeof(sin));						// 从&sin开始的长度为sizeof(sin)的内存清0
	sin.sin_family = AF_INET;							// 因特网地址簇(INET-Internet)
	sin.sin_addr.s_addr = INADDR_ANY;					// 监听所有(接口的)IP地址。
	sin.sin_port = htons((u_short)atoi(service));		// 监听的端口号。atoi--把ascii转化为int，htons--主机序到网络序(16位)
	bind(msock, (struct sockaddr *)&sin, sizeof(sin));  // 绑定监听的IP地址和端口号

	listen(msock, 5);                                   // 等待建立连接的队列长度为5

	for (int i = 0; i < MAX; i++) {
		thread[i] = 0;
		allssock[i] = 0;
		p[i].threadno = -1;
	}

    while(!_kbhit()){ 		                             // 检测是否有按键
       alen = sizeof(struct sockaddr);                   // 取到地址结构的长度
	   ssock = accept(msock, (struct sockaddr *)&fsin, &alen); // 如果有新的连接请求，返回连接套接字，否则，被阻塞。fsin包含客户端IP地址和端口号

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

	   (void)time(&now);                                      // 取得系统时间
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
    (void) closesocket(msock);                                 // 关闭监听套接字
     WSACleanup();                                             // 卸载winsock library

	 DeleteCriticalSection(&cs);  // 删除临界区
	 
	 HANDLE temp[MAX];
	 for (int i = 0; i < MAX; i++) {
		 if (thread[i]) {

			 WaitForSingleObject(thread[i], INFINITE);
			 CloseHandle(thread[i]);
		 }
	 } 
}



/* (void) time(&now);                                      // 取得系统时间
	  pts = ctime(&now);                                      // 把时间转换为字符串
	  (void) send(ssock, pts, strlen(pts), 0);                // 把缓冲区(指针,长度)的数据发送出去
	  printf("%s", pts);                                      // 显示发送字符串
	  (void) closesocket(ssock);                              // 关闭连接套接字*/