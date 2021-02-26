/* P2PClient.cpp */
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <math.h>

#define	BUFLEN		2000                  // ��������С
#define NAMELEN     200                   // �ļ������������ļ�·����������С
#define CHATTYPE    1                     // ��ʾ�������͵ĳ���
#define FILETYPE    2                     // ��ʾ�ļ����͵ĳ���
#define INSLEN      4                     // ָ��ĳ��ȣ�rdir, chat, send, quit��
#define WSVERS		MAKEWORD(2, 0)        // ָ���汾2.0 
#pragma comment(lib,"ws2_32.lib")         // ʹ��winsock 2.0 Llibrary

CRITICAL_SECTION cs;
char destaddr[NAMELEN];
bool is_exit = false;

/*datatype��ʾ�������ͣ���Ϊ����(1)���ļ�(2)************************/
/*len��ʾ���ݵĳ��ȣ�������ļ������ļ���С����������ݾ������ݳ���*/
/*filename��¼�ļ�����������������ݾ�Ϊ��(ȫ��)********************/
typedef struct info {
	int datatype;
	int len;
	char filename[NAMELEN];
}info;

//newFileNameҲ�ǰ���·����
void getUniqueName(char *newFileName, char *filePathName) {
	int cnt = 1;
	int len = strlen(filePathName);
	FILE* test;
	char temp[NAMELEN] = { 0 };
	char ext[NAMELEN] = { 0 };
	char front[NAMELEN] = { 0 };

	for (int i = len; i >= 0; i--) {
		if (filePathName[i] == '.') {
			strcpy(ext, filePathName + i);
			strncpy(front, filePathName, i);
			break;
		}
	}

	test = fopen(filePathName, "r");
	//������ھͱ��
	if (test) {
		fclose(test);
		while (test) {
			fclose(test);
			memset(temp, 0, NAMELEN);
			sprintf(temp, "%s(%d)%s", front, cnt,ext);
			cnt++;
			test = fopen(temp, "r");
		}
		strcpy(newFileName, temp);
	}
	//���û�о͸���
	else {
		strcpy(newFileName, filePathName);
	}
}

unsigned __stdcall p2pRecv(void* p) {
	SOCKET* p1 = (SOCKET *)p;
	info datainfo;
	char buf[BUFLEN + 1] = { 0 };
	char PathandName[NAMELEN] = { 0 };
	char newFileName[NAMELEN] = { 0 };
	int  rc = -1;
	FILE* fp;

	while (1) {
		if (is_exit) {
			return 0;
		}
		memset(buf, 0, BUFLEN + 1);
		//����һ���ṹ
		rc = recv(*p1, (char*)&datainfo, sizeof(datainfo), 0);
		if (datainfo.datatype == CHATTYPE) {
			//�͹涨chat���ַ���������2000
			rc = recv(*p1, buf, datainfo.len, 0);
			if (!strcmp(buf, "quit")) {
				printf("�Է�������\n");
				break;
			}
			else {
				printf("%s\n", buf);
			}
		}
		else if (datainfo.datatype == FILETYPE) {
			//���ݽṹ�õ�Ŀ���ַ���ļ���
			sprintf(PathandName, "%s\\%s", destaddr, datainfo.filename);
			getUniqueName(newFileName, PathandName);
			errno_t err = fopen_s(&fp, newFileName, "wb");
			for (int i = 0; i < datainfo.len; i++) {
				rc = recv(*p1, buf, sizeof(char), 0);
				fwrite(buf, sizeof(char), rc, fp);
			}
			fclose(fp);
			printf("�յ��ļ� %s, ������%s��\n", datainfo.filename,newFileName);
		}
		
	}
	
	return 0;
}

int get_file_size(FILE* file) {
	//���浱ǰλ��
	unsigned int curr = ftell(file);
	int size = 0;
	//�ƶ���ĩβ
	fseek(file, 0, SEEK_END);
	//��ȡ�ļ���С
	size = ftell(file);
	//�ָ��ļ�ԭ����λ��
	fseek(file, curr, SEEK_SET);
	return size;
}

void get_name(char pathname[], char name[]) {
	int i = 0;
	for (i = strlen(pathname) - 1; i >= 0; i--) {
		if (pathname[i] == '\\') {
			break;
		}
	}
	strcpy(name, pathname + i + 1);
}

void sendChatPacket(SOCKET sock, char *chatData, info datainfo) {
	int cc = 0;
	cc = send(sock, (char*)&datainfo, sizeof(datainfo), 0);
	/*if (cc <= 0) {
		printf("error\n");
		return;
	}*/
	cc = send(sock, chatData, strlen(chatData), 0);
	/*if (cc <= 0) {
		printf("error\n");
		return;
	}*/
}

void sendFilePacket(SOCKET sock, char * PathandName, info datainfo) {
	int cc = 0;
	FILE* fp;
	char ch;


	get_name(PathandName, datainfo.filename);
	errno_t err = fopen_s(&fp, PathandName, "rb");
	if (err) {
		printf("error\n");
		return;
	}
	datainfo.len = get_file_size(fp);
	cc = send(sock, (char*)&datainfo, sizeof(datainfo), 0);
	/*if (cc <= 0) {
		printf("error\n");
		return;
	}*/
	//ÿ�η�һ���ֽ�
	for (int i = 0; i < datainfo.len; i++) {
		fread(&ch, sizeof(char), 1, fp);
		cc = send(sock, &ch, sizeof(ch), 0);
	}
	fclose(fp);
	
}


/*------------------------------------------------------------------------
 * main - P2P client for TIME service
 *------------------------------------------------------------------------
 */
void
main(int argc, char *argv[])
{
	char	*host = "127.0.0.1";	    /* server IP to connect         */	
	char	*service = "50520";  	    /* server port to connect       */
	struct sockaddr_in sin;	            /* an Internet endpoint address	*/
	char	buf[BUFLEN+1];   	        /* buffer for one line of text	*/
	SOCKET	sock;		  	            /* socket descriptor	    	*/
	int	    cc;			                /* recv character count		    */

	HANDLE   thread;
	SOCKET*  p;
	unsigned threadID;
	char     ins[INSLEN + 1];
	info     datainfo;


	WSADATA wsadata;
	WSAStartup(WSVERS, &wsadata);						  //����winsock library��WSVERSΪ����İ汾��wsadata����ϵͳʵ��֧�ֵ���߰汾
  
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	  //�����׽��֣�������������Э���(family)�����׽��֣�TCPЭ��
														  //���أ�Ҫ�����׽��ֵ���������INVALID_SOCKET
    memset(&sin, 0, sizeof(sin));						  // ��&sin��ʼ�ĳ���Ϊsizeof(sin)���ڴ���0
    sin.sin_family = AF_INET;							  // ��������ַ��
    sin.sin_addr.s_addr = inet_addr(host);                // ������IP��ַ(32λ)
    sin.sin_port = htons((u_short)atoi(service));         // �������˿ں�  
    
	printf("�����С�\n");
	connect(sock, (struct sockaddr *)&sin, sizeof(sin));  // ���ӵ�������
	printf("���ӳɹ���\n");
	
	InitializeCriticalSection(&cs);
	p = &sock;
	thread = (HANDLE)_beginthreadex(NULL, 0, &p2pRecv, (void *)p, 0, &threadID);
	memset(destaddr, 0, sizeof(destaddr));
	memset(buf, 0, sizeof(buf));
	while(1) {
		//��ȡһ��ָ��
		gets_s(buf);
		//��ȡָ������
		strncpy(ins, buf, INSLEN);
		ins[INSLEN] = 0;
		//��ʼ��Ϊ0
		memset(&datainfo, 0, sizeof(datainfo));
		char* p = buf + INSLEN + 1;
		if (!strcmp(ins, "rdir")) {
			memset(destaddr, 0, sizeof(destaddr));
			strcpy(destaddr, p);
			printf("ת��·���ɹ�\n");
		}
		else if (!strcmp(ins, "chat")) {
			datainfo.datatype = CHATTYPE;
			datainfo.len = strlen(p);
			sendChatPacket(sock, p, datainfo);
		}
		else if (!strcmp(ins, "file")) {
			datainfo.datatype = FILETYPE;
			//strcpy(datainfo.filename, p);
			sendFilePacket(sock, p, datainfo);
		}
		else if (!strcmp(ins, "quit")) {
			char temp[] = "quit";
			datainfo.datatype = CHATTYPE;
			datainfo.len = strlen(temp);
			sendChatPacket(sock, temp, datainfo);
			(void)closesocket(sock);
			is_exit = true;
			//CloseHandle(thread);
			break;
		}
		else {
			datainfo.datatype = CHATTYPE;
			datainfo.len = strlen(buf);
			sendChatPacket(sock, buf, datainfo);
		}
		
	}
	//WaitForSingleObject(thread, INFINITE);
    closesocket(sock);                             // �رռ����׽���
	CloseHandle(thread);
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // ж��winsock library

    printf("���س�������...");
	getchar();										// �ȴ����ⰴ��

	
}