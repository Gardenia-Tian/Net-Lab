#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define	BUFLEN		5000                  // ��������С
#define WSVERS		MAKEWORD(2, 2)        // ָ���汾2.2 
#pragma comment(lib,"ws2_32.lib")         // ����winsock 2.2 Llibrary

void main(int argc, char *argv[]) {
	char	*host = "127.0.0.1";	    /* server IP to connect         */
	char	*service = "50500";  	    /* server port to connect       */
	struct sockaddr_in toAddr;	        /* an Internet endpoint address	*/
	int     toAddr_size = sizeof(toAddr);
	char	buf[BUFLEN + 1];   		    /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	cc,dd;			                    /* recv character count		    */
	char	pts[BUFLEN+1];			            /* pointer to time string	    */
	char	recv[BUFLEN + 1];

	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);       /* ����ĳ�汾Socket��DLL        */

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons((u_short)atoi(service));    //atoi����asciiת��Ϊint.
														// htons��������(host)ת��Ϊ������(network), s--short
	toAddr.sin_addr.s_addr = inet_addr(host);           //���hostΪ������
														//��Ҫ���ú���gethostbyname������ת��ΪIP��ַ

	printf("�����Ҫ�رճ���������Exit��������������Ҫ���͵���Ϣ��\n");
	scanf("%s", buf);
	cc = sendto(sock, buf, strlen(buf), 0, (SOCKADDR *)&toAddr, sizeof(toAddr));   //�ͳ���Ϣ
	if (cc == SOCKET_ERROR) {
		printf("����ʧ�ܣ�����ţ�%d\n", WSAGetLastError());
	}
	
	if (strcmp(buf, "Exit")) {
		dd = recvfrom(sock, recv, BUFLEN, 0, (SOCKADDR *)&toAddr, &toAddr_size);
		if (dd == SOCKET_ERROR) {
			printf("recvfrom() failed; %d\n", WSAGetLastError());
		}
		else {
			recv[dd] = '\0';
			printf("%s\n", recv);
		}
	}
	closesocket(sock);
	WSACleanup();       	          /* ж��ĳ�汾��DLL */

	printf("��������˳�...");
	getchar();
	getchar();
}