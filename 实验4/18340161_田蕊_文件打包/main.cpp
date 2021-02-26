#pragma warning(disable:4996)
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#define USER_NAME_LEN 100
#define EMAIL_LEN     100
#define NAMELEN       300
#define BUFLEN        2<<10
#define BUFLEN1       100//一次就读一个字节感觉没必要开太大
#define TIME_BUF_LEN  30
#define NAMECNT       50
#define NUMLEN        5 //四位数差不多就够了

struct Person {
	char userName[USER_NAME_LEN];
	int level;
	char email[EMAIL_LEN];
	time_t inputTime;
	time_t sendTime;
	void clear() {
		memset(userName, 0, USER_NAME_LEN);
		memset(email, 0, USER_NAME_LEN);
		level = -1;
		inputTime = 0;
		sendTime = 0;
	}
};

struct fileentry {
	//time_t changeTime;
	//FILETIME lpCreationTime;
	//FILETIME lpLastAccessTime;
	//FILETIME lpLastWriteTime;
	int size;//byte
	char filename[NAMELEN];
	
};

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

void write() {
	FILE* fp;
	errno_t err = fopen_s(&fp, "Person.txt", "wb");

	char strBUF[BUFLEN1] = { 0 };
	int numBUF = 0;
	Person towrite;
	int cnt = 1;

	printf("username %d: ", cnt);
	scanf_s("%s", strBUF, BUFLEN1);
	while (strcmp(strBUF, "exit")) {
		strcpy_s(towrite.userName, strBUF);
		memset(strBUF, 0, BUFLEN1);

		printf("level:      ");
		scanf_s("%d", &numBUF, sizeof(int));
		towrite.level = numBUF;

		printf("email:      ");
		scanf_s("%s", strBUF, BUFLEN1);
		strcpy_s(towrite.email, strBUF);
		memset(strBUF, 0, BUFLEN1);

		time(&towrite.inputTime);
		time(&towrite.sendTime);


		fwrite(&towrite, sizeof(Person), 1, fp);

		cnt++;
		printf("\nusername %d : ", cnt);
		scanf_s("%s", strBUF, BUFLEN1);
	};

	fclose(fp);
}

void read() {
	FILE* fp;
	errno_t err = fopen_s(&fp, "Person.txt", "rb");

	char strBUF[BUFLEN1] = { 0 };
	int numBUF = 0;
	Person toread;
	int cnt = 1;

	char pts[TIME_BUF_LEN];

	while (fread(&toread, sizeof(Person), 1, fp) == 1) {
		printf("username:          %s \n", toread.userName);
		printf("level:             %d \n", toread.level);
		printf("email:             %s \n", toread.email);
		ctime_s(pts, TIME_BUF_LEN, &toread.inputTime);
		printf("Registration time: %s", pts);
		memset(pts, 0, TIME_BUF_LEN);
		ctime_s(pts, TIME_BUF_LEN, &toread.sendTime);
		printf("Send time:         %s \n", pts);
		memset(pts, 0, TIME_BUF_LEN);
	}
}

void IntToStr(int a, char b[]) {
	int cnt = 0;
	int cnt1 = 0;
	int tempa = a;
	char tempb[NUMLEN] = { 0 };
	while (tempa) {
		tempb[cnt] = tempa % 10;
		tempa = tempa / 10;
		cnt++;
	}
	cnt1 = cnt;
	for (int i = 0; i < cnt; i++) {
		b[i] = tempb[--cnt1] + '0';
	}
	b[cnt] = '\0';
}

//filename -> NAMELEN
void numName(int a, char name[]) {
	char num[NUMLEN] = { 0 };
	char tempname[NAMELEN] = { 0 };
	IntToStr(a, num);

	for (int i = strlen(name); i >=0 ; i--) {
		if (name[i] == '.') {
			strncpy(tempname, name, i);
			IntToStr(a, num);
			strcat(tempname, "(");
			strcat(tempname, num);
			strcat(tempname, ")");
			strcat(tempname, name + i);
			break;
		}
	}
	strcpy(name, tempname);
}

//newname里面原来是路径，后来是新名字
void get_new_path(char oldname[], char newname[]) {
	char name[NAMELEN] = { 0 };
	int i = 0;
	/*for (i = strlen(oldname) - 1; i >= 0; i--) {
		if (oldname[i] == '\\') {
			break;
		}
	}*/
	//strcpy(name, oldname + i);
	strcat(newname, "\\");
	strcat(newname, oldname);
}

//把名字在路径中提取出来
void get_name(char pathname[], char name[]) {
	int i = 0;
	for (i = strlen(pathname) - 1; i >= 0; i--) {
		if (pathname[i] == '\\') {
			break;
		}
	}
	strcpy(name, pathname + i);
}

void packet(){
	char filename[NAMELEN] = { 0 };
	FILE* dest;
	int cnt = 1;
	char destpath[NAMELEN] = { 0 };

	printf("please input the package file path and file name ");
	scanf_s("%s", destpath, NAMELEN);
	//FileSet.pak
	errno_t err = fopen_s(&dest, destpath, "wb");
	printf("please input the filename %d: ", cnt);
	scanf_s("%s", filename, NAMELEN);

	while (strcmp(filename, "exit")) {
		fileentry towrite;
		FILE* src;
		//以只读的方式打开一个文件
		errno_t err1 = fopen_s(&src, filename, "rb");
		//filename是带路径的，存进去的应该是不带路径的
		char name1[NAMELEN] = { 0 };
		name1[0] = '\0';
		get_name(filename, name1);

		strcpy(towrite.filename, name1);
		//如果有这个文件，成功打开了
		if (!err1) {
			towrite.size = get_file_size(src);
			//把基本信息写进去
			fwrite(&towrite, sizeof(fileentry), 1, dest);
			//copy内容
			int fileBytes = 0;
			char buf[BUFLEN] = { 0 };
			
			char ch;
			while (fread(&ch, sizeof(char), 1, src) != 0)
				fwrite(&ch, sizeof(char), 1, dest);
			fclose(src);
		}
		else {
			printf("file not fount\n");
		}
		cnt++;
		memset(filename, 0, NAMELEN);
		printf("please input the filename %d: ", cnt);
		scanf_s("%s", filename, NAMELEN);
		cnt = cnt + 1 - 1;
	}
	fclose(dest);
}



void unpacket() {
	FILE* read;
	char src[NAMELEN] = { 0 };
	fileentry thefile;
	char destfile[NAMELEN] = { 0 };

	char allname[NAMECNT][NAMELEN] = { 0 };
	int  namecnt[NAMECNT] = { 0 };
	int index_allname = 0;


	printf("please input filename and path to extract: ");
	scanf_s("%s", src, NAMELEN);
	errno_t err = fopen_s(&read, src, "rb");

	printf("please input destination folder path: ");
	scanf_s("%s", destfile, NAMELEN);
	while (fread(&thefile, sizeof(fileentry), 1, read) == 1) {

		int size = thefile.size;
		FILE* write;

		bool isfind = false;
		for (int i = 0; i < index_allname; i++) {
			//如果已经有了
			if (!strcmp(thefile.filename, allname[i])) {
				namecnt[i]++;
				numName(namecnt[i], thefile.filename);
				isfind = true;
				break;
			}
		}

		//如果没找到的话，还要在名字数组里面加一个新的
		if (!isfind) {
			strcpy(allname[index_allname], thefile.filename);
			index_allname++;
		}

		char tempname[NAMELEN] = { 0 };
		strcpy(tempname, destfile);
		get_new_path(thefile.filename, tempname);
		errno_t err1 = fopen_s(&write, tempname, "wb");
		memset(tempname, 0, NAMELEN);
		if (!err1) {
			int readBytes = 0;
			char buf[BUFLEN1] = { 0 };
			while (readBytes != size) {
				readBytes += fread(buf, sizeof(char), 1, read);
				fwrite(buf, sizeof(char), strlen(buf), write);
				//printf("%c", *buf);
				memset(buf, 0, BUFLEN1);
			}
			fclose(write);
		}
		

		memset(&thefile, 0, sizeof(fileentry));
	}
}

int main(int argc, char *argv[]) {
	char instr[BUFLEN1] = { 0 };
	printf("Instruction StructSave: Save the input structure data to a file\n");
	printf("Instruction StructRead: Read the structure data from the file and display it\n");
	printf("Instruction FilePack  : Package the imported file\n");
	printf("Instruction FileUnpack: Unpackage the imported file\n");
	printf("Instruction exit      : Exit program\n");

	while (1) {
		printf("please input instruction: ");
		scanf_s("%s", instr, BUFLEN1);
		if (!strcmp(instr, "StructSave")) {
			write();
			printf("success!\n\n");
		}
		else if (!strcmp(instr, "StructRead")) {
			read();
			printf("success!\n\n");
		}
		else if (!strcmp(instr, "FilePack")) {
			packet();
			printf("success!\n\n");
		}
		else if (!strcmp(instr, "FileUnpack")) {
			unpacket();
			printf("success!\n\n");
		}
		else if (!strcmp(instr, "exit")) {
			break;
		}
	}

	
	
	printf("input any key to continue\n");
	getchar();
	return 0;
}