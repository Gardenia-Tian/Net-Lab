#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define	BUFLEN		2000                  // ��������С
#define WSVERS		MAKEWORD(2, 0)        // ָ���汾2.0 
#pragma comment(lib,"ws2_32.lib")         // ʹ��winsock 2.0 Llibrary


void main(int argc, char *argv[]) {
	char	*host = "127.0.0.1";	    /* server IP to connect         */
	char	*service = "50500";  	    /* server port to connect       */
	struct sockaddr_in sin;	            /* an Internet endpoint address	*/
	char	buf[BUFLEN + 1];   		    /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	cc;			                    /* send character count		    */
	int dd;                             /* recv character count         */
	int ee, gg;
	char pts[BUFLEN + 1];
	char buf_r[BUFLEN + 1];
	char buf_port[BUFLEN + 1];


	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //����winsock library��
														  //WSVERSΪ����İ汾��
														  //wsadata����ϵͳʵ��֧�ֵ���߰汾
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);     //�����׽���

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(host);
	sin.sin_port = htons((u_short)atoi(service));
	int ret = connect(sock, (struct sockaddr *)&sin, sizeof(sin));


	printf("�����Ҫ�رճ���������Exit��������������Ҫ���͵���Ϣ��");
	scanf("%s", buf);
	cc = send(sock, buf, strlen(buf), 0);                 //�����ַ���
	dd = recv(sock, pts, BUFLEN, 0);                      //�յ��ַ����������ܵ�����ʱ��
	ee = recv(sock, buf_r, BUFLEN, 0);
	gg = recv(sock, buf_port, BUFLEN, 0);
	if (dd == SOCKET_ERROR)                               // ����������ر��׽���sock
		printf("Error: %d.\n", GetLastError());
	else if (dd == 0) {                                   // �Է������ر�
		printf("Server closed!\n", pts);
	}
	else if (dd > 0) {
		pts[dd] = '\0';	                                  // ensure null-termination
		printf("\n�յ�����Ϣ��\n");
		printf("ʱ�䣺%s", pts);                              // ��ʾ�����յ��ַ���
	}

	if (ee == SOCKET_ERROR)                               // ����������ر��׽���sock
		printf("Error: %d.\n", GetLastError());
	else if (dd > 0 && ee == 0) {                                 // �Է������ر�
		printf("Server closed!", buf_r);
	}
	else if (ee > 0) {
		buf_r[ee] = '\0';
		printf("���ݣ�%s\n", buf_r);                            // ��ʾ�����յ��ַ���
	}
	if (dd > 0 && ee > 0) {
		printf("�ͻ���IP��ַ��%s\n", host);
	}
	

	if (gg == SOCKET_ERROR)                               // ����������ر��׽���sock
		printf("Error: %d.\n", GetLastError());
	else if (dd > 0 && ee > 0 && gg == 0) {                                 // �Է������ر�
		printf("Server closed!", buf_port);
	}
	else if (gg > 0) {
		buf_port[gg] = '\0';
		int port = atoi(buf_port);
		printf("�ͻ��˶˿ںţ�%d\n", port);                            // ��ʾ�����յ��ַ���
	}

	closesocket(sock);                                    // �رռ����׽���
	WSACleanup();                                         // ж��winsock library

	printf("���س�������...");
	getchar();										      // �ȴ����ⰴ��
	getchar();
}


