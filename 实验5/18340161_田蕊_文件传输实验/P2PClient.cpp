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

#define	BUFLEN		2000                  // 缓冲区大小
#define NAMELEN     200                   // 文件名缓冲区，文件路径缓冲区大小
#define CHATTYPE    1                     // 表示聊天类型的常量
#define FILETYPE    2                     // 表示文件类型的常量
#define INSLEN      4                     // 指令的长度（rdir, chat, send, quit）
#define WSVERS		MAKEWORD(2, 0)        // 指明版本2.0 
#pragma comment(lib,"ws2_32.lib")         // 使用winsock 2.0 Llibrary

CRITICAL_SECTION cs;
char destaddr[NAMELEN];
bool is_exit = false;

/*datatype表示数据类型，分为聊天(1)和文件(2)************************/
/*len表示数据的长度，如果是文件就是文件大小，如果是数据就是数据长度*/
/*filename记录文件名，如果是聊天数据就为空(全零)********************/
typedef struct info {
	int datatype;
	int len;
	char filename[NAMELEN];
}info;

//newFileName也是包含路径的
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
	//如果存在就编号
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
	//如果没有就复制
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
		//先收一个结构
		rc = recv(*p1, (char*)&datainfo, sizeof(datainfo), 0);
		if (datainfo.datatype == CHATTYPE) {
			//就规定chat的字符数不超过2000
			rc = recv(*p1, buf, datainfo.len, 0);
			if (!strcmp(buf, "quit")) {
				printf("对方已下线\n");
				break;
			}
			else {
				printf("%s\n", buf);
			}
		}
		else if (datainfo.datatype == FILETYPE) {
			//根据结构得到目标地址和文件名
			sprintf(PathandName, "%s\\%s", destaddr, datainfo.filename);
			getUniqueName(newFileName, PathandName);
			errno_t err = fopen_s(&fp, newFileName, "wb");
			for (int i = 0; i < datainfo.len; i++) {
				rc = recv(*p1, buf, sizeof(char), 0);
				fwrite(buf, sizeof(char), rc, fp);
			}
			fclose(fp);
			printf("收到文件 %s, 保存在%s处\n", datainfo.filename,newFileName);
		}
		
	}
	
	return 0;
}

int get_file_size(FILE* file) {
	//保存当前位置
	unsigned int curr = ftell(file);
	int size = 0;
	//移动到末尾
	fseek(file, 0, SEEK_END);
	//获取文件大小
	size = ftell(file);
	//恢复文件原来的位置
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
	//每次发一个字节
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
	WSAStartup(WSVERS, &wsadata);						  //加载winsock library。WSVERS为请求的版本，wsadata返回系统实际支持的最高版本
  
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	  //创建套接字，参数：因特网协议簇(family)，流套接字，TCP协议
														  //返回：要监听套接字的描述符或INVALID_SOCKET
    memset(&sin, 0, sizeof(sin));						  // 从&sin开始的长度为sizeof(sin)的内存清0
    sin.sin_family = AF_INET;							  // 因特网地址簇
    sin.sin_addr.s_addr = inet_addr(host);                // 服务器IP地址(32位)
    sin.sin_port = htons((u_short)atoi(service));         // 服务器端口号  
    
	printf("连接中…\n");
	connect(sock, (struct sockaddr *)&sin, sizeof(sin));  // 连接到服务器
	printf("连接成功…\n");
	
	InitializeCriticalSection(&cs);
	p = &sock;
	thread = (HANDLE)_beginthreadex(NULL, 0, &p2pRecv, (void *)p, 0, &threadID);
	memset(destaddr, 0, sizeof(destaddr));
	memset(buf, 0, sizeof(buf));
	while(1) {
		//获取一行指令
		gets_s(buf);
		//截取指令内容
		strncpy(ins, buf, INSLEN);
		ins[INSLEN] = 0;
		//初始化为0
		memset(&datainfo, 0, sizeof(datainfo));
		char* p = buf + INSLEN + 1;
		if (!strcmp(ins, "rdir")) {
			memset(destaddr, 0, sizeof(destaddr));
			strcpy(destaddr, p);
			printf("转换路径成功\n");
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
    closesocket(sock);                             // 关闭监听套接字
	CloseHandle(thread);
	DeleteCriticalSection(&cs);
	WSACleanup();                                  // 卸载winsock library

    printf("按回车键继续...");
	getchar();										// 等待任意按键

	
}