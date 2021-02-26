/* FtpClient.cpp */
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>
#include <Ws2tcpip.h>//应该包含netbd.h，但是这个是unix下的，所以用Ws2tcpip.h
#define	BUFLEN		2<<10                  // 缓冲区大小
#define NSTRING     100
#define WSVERS		MAKEWORD(2, 0)        // 指明版本2.0 
#pragma comment(lib,"ws2_32.lib")         // 使用winsock 2.0 Llibrary

CRITICAL_SECTION cs;
char bufRecv[BUFLEN + 1] = { 0 };


int recvFromNet(SOCKET* p1) {
	memset(bufRecv, 0, BUFLEN + 1);
	int rc = recv(*p1, bufRecv, BUFLEN, 0);
	if (rc > 0) {
		bufRecv[rc] = '\0';
		printf("%s", bufRecv);
	}
	
	return 0;
}

SOCKET dataConn(char* addr,char* port) {
	SOCKET    data_sock;
	int       is_succ;
	addrinfo* host;
	getaddrinfo(addr, port, NULL, &host);
	data_sock = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	is_succ = connect(data_sock, host->ai_addr, host->ai_addrlen);
	if (is_succ) {
		printf("Connect failed\n");
		
	}
	
	return data_sock;
}

int saveFile(SOCKET sock, char* fileName) {
	FILE* fp;
	char buf[BUFLEN] = { 0 };
	char* p = buf;
	int rc = -1;
	int size = 0;
	errno_t err = fopen_s(&fp, fileName, "wb");
	while (rc) {
		rc = recv(sock, buf, 1, 0);
		size = fwrite(buf, sizeof(char), rc, fp);
	}
		
	fclose(fp);
	shutdown(sock, SD_SEND);
	closesocket(sock);                             // 关闭监听套接字
	return rc;
}

int getPort() {
	int comma = 0;
	char* p = bufRecv;
	int index = 0;
	char s1[NSTRING] = { 0 };
	char s2[NSTRING] = { 0 };
	int t1 = 0, t2 = 0;
	int ret = 0;
	while (*p != '\0') {
		if (*p == ',') comma++;
		p++;
		if (comma == 4) break;
	}
	while (*p != ',') {
		s1[index++] = *p;
		p++;
	}
	s1[index] = '\0';
	index = 0;
	p++;
	while (*p != ')') {
		s2[index++] = *p;
		p++;
	}
	s2[index] = '\0';
	t1 = atoi(s1);
	t2 = atoi(s2);
	ret = t1 * 256 + t2;
	return ret;
}

/*------------------------------------------------------------------------
 * main - Ftp client for TIME service
 *------------------------------------------------------------------------
 */

void
main(int argc, char *argv[])
{   	
	/******************************变量定义********************************************/
	char      buf[BUFLEN];				/* buffer for one line of text	*/
	SOCKET	  control_sock;		  	            /* socket descriptor	    	*/
	SOCKET    data_sock;
	int	      cc;			            /* recv character count		    */
	addrinfo* host;                     /* 要连接的信息                  */
	HANDLE    thread;
	SOCKET*   p;
	unsigned  threadID;
	int       is_succ = 0;
	int       len = 0;
	char      addr[NSTRING];
	char      dataport[NSTRING];

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //加载winsock library。WSVERS为请求的版本，wsadata返回系统实际支持的最高版本
  
	if (argc < 3) {
		return;
	}

	/******************************指令设置*******************************************/
	char ins0[] = "user net\r\n";
	char ins1[] = "pass 123456\r\n";
	char ins2[] = "pasv\r\n";
	char ins3[NSTRING] = { 0 };
	char filename[] = "\\ebook\\ftp.pdf";
	sprintf(ins3, "retr %s\r\n", filename);
	char ins4[] = "quit\r\n";
	char ins5[] = "type I\r\n";
	int len0 = strlen(ins0);
	int len1 = strlen(ins1);
	int len2 = strlen(ins2);
	int len3 = strlen(ins3);
	int len4 = strlen(ins4);
	int len5 = strlen(ins5);

	/******************************套接字连接******************************************/
	strcpy(addr, argv[1]);
	getaddrinfo(addr, "21", NULL, &host);
	control_sock = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	is_succ = connect(control_sock, host->ai_addr, host->ai_addrlen);
	if (is_succ) {
		printf("Connect failed\n");
		return;
	}



	/******************************不同的指令交互**************************************************/
	//user net 220 331
	cc = send(control_sock, ins0, len0, 0);
	printf("%s", ins0);
	recvFromNet(&control_sock);
	recvFromNet(&control_sock);
	//pass 123456 230
	cc = send(control_sock, ins1, len1, 0);
	printf("%s", ins1);
	recvFromNet(&control_sock);

	//pasv 227
	cc = send(control_sock, ins2, len2, 0);
	printf("%s", ins2);
	recvFromNet(&control_sock);
	itoa(getPort(),dataport,10);
	data_sock = dataConn(addr, dataport);
	//retr 125 226
	cc = send(control_sock, ins3, len3, 0);
	printf("%s", ins3);
	recvFromNet(&control_sock);
	saveFile(data_sock, argv[3]);
	recvFromNet(&control_sock);
	cc = send(control_sock, ins4, len4, 0);
	printf("%s", ins4);
	recvFromNet(&control_sock);
	
	
	shutdown(control_sock, SD_SEND);
    closesocket(control_sock);                             // 关闭监听套接字
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // 卸载winsock library

    printf("按回车键继续...");
	getchar();										// 等待任意按键
}
//FtpClient 103.26.79.35 \ebook\ftp.pdf E:\some\ftp2.pdf