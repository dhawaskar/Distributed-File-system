#include <sstream>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include <string.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <dirent.h> 

using namespace std;

#define MAXLINE 1024
#define MAX 100

void parser(char filename[50]);
int compute_file_size(char filename[MAX]);
int compute_md5sum(char *buffer,int file_size);
void split_file(char  filename[100]);
void write_sub_file(char filename[100],int file_size,int chunk_size,int left_size);
void send_partsto_servers(int mod,int part_num,int cfd_num);
void put_process(char filename[MAX]);
int receive_parts(int cfd_num);
void get_complete_file_parts(char file_main[MAX]);
void get_combined_file(char filename[MAX],int file_size,int chunk_size,int left_size);
void get_process(char filename[MAX]);
void create_client_dir();
void user_process();
void list_process();
void send_decision(char fiename[MAX]);
void encrypt_data(char *message,int len, int key);

DIR *dir;
struct dirent *ent;
struct server_info{
	char ip_address[32];
	int port;
};
server_info servers[4];
struct file_name{
        char filename[100];
	int file_size;
};
file_name files_name[4];
struct sockaddr_in server_addr;
char username[MAX],password[MAX],ROOT[MAXLINE],CLI_PATH[MAXLINE],*file_parts[4],err_mesg[MAX];
int cfd[4];


void send_decision(char filename[MAX]){
	int len;
	len=strlen(filename);
	filename[len]='\0';
	
}

void encrypt_data(char *message,int len,int KEY){
        int i;
        //printf("Encrypting the message\n");
        for(i = 0; i <len; i++)
                message[i] ^= KEY;

}


void parser(char filename[50]){
	fstream fd;
	char buf[MAXLINE],port_str[MAXLINE];
	fd.open(filename,fstream::in|fstream::binary);
	if(fd.fail())	perror("open");
	while(fd.getline(buf,MAXLINE)){
		if(strstr(buf,"server DFS1")){
			strtok(buf," ");
			strtok(NULL," ");
			strcpy(servers[0].ip_address,strtok(NULL,":"));
			strcpy(port_str,strtok(NULL,"NULL"));
			servers[0].port=atoi(port_str);
			cout<<"server 1 IP address:"<<servers[0].ip_address<<"server 1 port number:"<<servers[0].port<<endl;
		}
		else if(strstr(buf,"server DFS2")){
			strtok(buf," ");
			strtok(NULL," ");
			strcpy(servers[1].ip_address,strtok(NULL,":"));
			strcpy(port_str,strtok(NULL,"NULL"));
			servers[1].port=atoi(port_str);
			cout<<"server 2 IP address:"<<servers[1].ip_address<<"server 2 port number:"<<servers[1].port<<endl;
		}
		else if(strstr(buf,"server DFS3")){
			strtok(buf," ");
			strtok(NULL," ");
			strcpy(servers[2].ip_address,strtok(NULL,":"));
			strcpy(port_str,strtok(NULL,"NULL"));
			servers[2].port=atoi(port_str);
			cout<<"server 3 IP address:"<<servers[2].ip_address<<"server 3 port number:"<<servers[2].port<<endl;
		}
		else if(strstr(buf,"server DFS4")){
			strtok(buf," ");
			strtok(NULL," ");
			strcpy(servers[3].ip_address,strtok(NULL,":"));
			strcpy(port_str,strtok(NULL,"NULL"));
			servers[3].port=atoi(port_str);
			cout<<"server 4 IP address:"<<servers[3].ip_address<<"server 4 port number:"<<servers[3].port<<endl;
		}
		else if(strstr(buf,"User")){
			strtok(buf," ");
			strcpy(username,strtok(NULL,"NULL"));
			cout<<"UserName:"<<username<<endl;
		}
		else if(strstr(buf,"Password")){
			strtok(buf," ");
			strcpy(password,strtok(NULL,"NULL"));
			cout<<"Password:"<<password<<endl;
		}
	}
}

int compute_file_size(char filename[MAX]){
	cout <<"file size calculation for "<<filename<<endl;
	int filesize;
	fstream fd;
	fd.open(filename,fstream::in);
	if(fd.fail())	perror("open fail in file size compute function");
	fd.seekg(0,fstream::end);
	filesize=fd.tellg();
	fd.close();
	cout<<"calculated File size is:"<<filesize<<endl;
	return filesize;
}

int compute_md5sum(char *buffer,int file_size){
	unsigned char digest[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)buffer, file_size, (unsigned char*)&digest);
        char mdString[32];
        for(int i = 0; i < 16; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
	//cout<<"MD5sum is: "<<mdString<<endl;
        int v1, v2, v3, v4;
        sscanf( &mdString[0], "%x", &v1 );
        sscanf( &mdString[8], "%x", &v2 );
        sscanf( &mdString[16], "%x", &v3 );
        sscanf( &mdString[24], "%x", &v4 );
        int hash = v1 ^ v2 ^ v3 ^v4;
        //cout<<"hash value:"<<abs(hash)<<endl;
	return abs(hash);
}

void split_file(char  filename[100]){
        char temp[1],sub_name[100];
        for(int i=1;i<5;i++){
                sprintf(temp,"%d",i);
                strcpy(sub_name,filename);
                strcat(sub_name,".");
                strncat(sub_name,temp,1);
                strcpy(files_name[i].filename,sub_name);
        }
        for(int i=1;i<5;i++){
                cout<<"File name::"<<files_name[i].filename<<endl;
        }
}

void write_sub_file(char filename[100],int file_size,int chunk_size,int left_size){
	cout<<"Received file name is"<<filename<<"in write to sub files"<<endl;
	fstream f1,f2,f3,f4,fd;
	cout<<"Total file size is"<<file_size<<endl;
	char buf[file_size];
	cout<<"Received file name is"<<filename<<endl;
	f1.open(files_name[1].filename,fstream::out|fstream::binary);
	if(f1.fail())	perror("f1 open");
	else
		cout<<"f1 opened"<<endl;
	f2.open(files_name[2].filename,fstream::out|fstream::binary);
	if(f2.fail())	perror("f2 open");
	else
		cout<<"f2 opened"<<endl;
	f3.open(files_name[3].filename,fstream::out|fstream::binary);
	if(f3.fail())	perror("f3 open");
	else
		cout<<"f3 opened"<<endl;
	f4.open(files_name[4].filename,fstream::out|fstream::binary);
	if(f4.fail())	perror("f4 open");
	else
		cout<<"f4 opened"<<endl;
	fd.open(filename,fstream::in|fstream::binary);
	if(fd.fail())	perror("fd open");
	else
		cout<<"fd opened"<<endl;
	cout<<"all files opened"<<endl;
	fd.read(buf,chunk_size);
//	cout<<"sub contents red:"<<buf<<endl;
	f1.write(buf,chunk_size);
	files_name[1].file_size=chunk_size;
	fd.read(buf,chunk_size);
//	cout<<"sub contents red:"<<buf<<endl;
        f2.write(buf,chunk_size);
	files_name[2].file_size=chunk_size;
	fd.read(buf,chunk_size);
//	cout<<"sub contents red:"<<buf<<endl;
        f3.write(buf,chunk_size);
	files_name[3].file_size=chunk_size;
	fd.read(buf,left_size);
//	cout<<"sub contents red:"<<buf<<endl;
        f4.write(buf,left_size);
	files_name[4].file_size=left_size;
	f1.close();
	f2.close();
	f3.close();
	f4.close();
	fd.close();
}

void send_partsto_servers(int mod,int part_num,int cfd_num){
	int count=2,i,j,len;
	fstream fd;
	char *buf,file_size_str[MAX],file_name_str[MAX],file_name_size_str[MAX];
	cout<<"This is mod"<<mod<<" function"<<endl;
        cout<<"Sending the "<<part_num<<" and "<<part_num+1<<" parts of file to server DFS"<<cfd_num+1<<endl;
	i=part_num;
	j=cfd_num;
	cout<<"i:"<<i<<"j:"<<j<<endl;
        while(count--){
                //sending the file size of part1
		bzero(file_size_str,MAX);
                sprintf(file_size_str,"%d",files_name[i].file_size);
                strcat(file_size_str,"\n");
		cout<<"sending the file_size_str:"<<file_size_str<<endl;
		if(cfd[j]!=-1)
                	sendto(cfd[j],file_size_str,strlen(file_size_str),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
		else	cout<<err_mesg<<endl;


                //sending the file name of part1
		bzero(file_name_str,MAX);
		cout<<"file name to be sent is"<<files_name[i].filename<<endl;
                strncpy(file_name_str,files_name[i].filename,strlen(files_name[i].filename));
                strncat(file_name_str,"\n",1);
		cout<<"sending the file name:"<<file_name_str<<endl;
		if(cfd[j]!=-1)
                	sendto(cfd[j],file_name_str,strlen(file_name_str),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
		else    cout<<err_mesg<<endl;

                //sending the file content of part1
                fd.open(files_name[i].filename,fstream::in|fstream::binary);
		int len=files_name[i].file_size;
		buf=new char[len];
		bzero(buf,len);
                fd.read(buf,files_name[i].file_size);
		int key;
		key=compute_md5sum(password,strlen(password));
		encrypt_data(buf,files_name[i].file_size,key);
		if(cfd[j]!=-1)
                	sendto(cfd[j],buf,files_name[i].file_size,0,(struct sockaddr *)&server_addr,sizeof(server_addr));
		else    cout<<err_mesg<<endl;
		delete buf;
                fd.close();
		if((i+1)==5) i=1;
		else
			i++;
	}	
}

void put_process(char filename[MAX]){
	int file_size=0,chunk_size=0,left_size=0;
	streamsize n;
	fstream fd;
	char *buf;
	int hash,hash_mod;
	cout<<"Received file name:  "<<filename<<endl;
	file_size=compute_file_size(filename);
	cout<<"obtained File size is:"<<file_size<<endl;
	cout<<"file size is: "<<file_size<<endl;
	chunk_size=file_size/4;
        left_size=file_size-(chunk_size*3);
	buf=new char[file_size];
	bzero(buf,file_size);
	fd.open(filename,fstream::in|fstream::binary);
	fd.read(buf,file_size);
	fd.close();
	hash=compute_md5sum(buf,file_size);
	cout<<"obtained hash value is: "<<hash<<endl;
	split_file(filename);
	cout<<" Sub Files are creted "<<endl;
	write_sub_file(filename,file_size,chunk_size,left_size);
	cout<<"The sub files are ready to upload onto servers"<<endl;	
	hash_mod=hash%4;
	for(int i=0;i<4;i++){
		cout<<"cfd for client "<<cfd[i]<<endl;
	}
	cout<<"The following are the file name and file size"<<endl;
	for(int i=1;i<5;i++){
		cout<<"file name:"<<files_name[i].filename<<"file size:"<<files_name[i].file_size<<endl;
	}
	switch(hash_mod){
		case 0:
			cout<<"hash is"<<hash_mod<<endl;
			send_partsto_servers(hash_mod,1,0);
			send_partsto_servers(hash_mod,2,1);
			send_partsto_servers(hash_mod,3,2);
			send_partsto_servers(hash_mod,4,3);
			break;
		case 1:
			cout<<"hash is"<<hash_mod<<endl;
			send_partsto_servers(hash_mod,4,0);
			send_partsto_servers(hash_mod,1,1);
			send_partsto_servers(hash_mod,2,2);
			send_partsto_servers(hash_mod,3,3);
			break;
		case 2:
			cout<<"hash is"<<hash_mod<<endl;
			send_partsto_servers(hash_mod,3,0);
			send_partsto_servers(hash_mod,4,1);
			send_partsto_servers(hash_mod,1,2);
			send_partsto_servers(hash_mod,2,3);
			break;
		case 3:
			cout<<"hash is"<<hash_mod<<endl;
			send_partsto_servers(hash_mod,2,0);
			send_partsto_servers(hash_mod,3,1);
			send_partsto_servers(hash_mod,4,2);
			send_partsto_servers(hash_mod,1,3);
			break;
	}
}


int receive_parts(int cfd_num){
	int file_size,count=2,n,j=cfd_num,path_len;
	fstream fd1,fd;
	char buf[MAXLINE],*file_name,filepath[MAX];
	cout<<"Lets receive the file parts from server for "<<endl;
	if(cfd[j]!=-1){
		while(count--){
			//receive file size
			bzero(buf,MAXLINE);
			fd1.open("temp1",fstream::out);
			size_t server_len=sizeof(server_addr);
			while((n=recvfrom(cfd[j],buf,1,0,(struct sockaddr *)&server_addr,&server_len))>0){
			       	if(strncmp(buf,"\n",1)==0) {
				       	fd1.close();
				       	break;
				}
				else  fd1.write(buf,1);
			}
			fd1.open("temp1",fstream::in);
			bzero(buf,MAXLINE);
			fd1.getline(buf,MAXLINE);
			fd1.close();
			cout<<"Received buffer"<<buf<<endl;
			file_size=atoi(buf);	
			cout<<"Received file size is :  "<<file_size<<endl;

			//receive file name
			bzero(buf,MAXLINE);
			fd1.open("temp2",fstream::out);
			while((n=recvfrom(cfd[j],buf,1,0,(struct sockaddr *)&server_addr,&server_len))>0){
				if(strncmp(buf,"\n",1)==0) {
					  fd1.close();
				          break;
				}
				else fd1.write(buf,1);
			}
			fd1.open("temp2",fstream::in);
			fd1.getline(buf,MAX);
			cout<<"buffer name received:"<<buf<<endl;
			fd1.close();
			int len=strlen(buf);
			file_name=new char[len];
			bzero(file_name,len);
			strncpy(file_name,buf,len); 
			file_name[len]='\0';
			cout<<"Received file name is :  "<<file_name<<endl;
	
			bzero(ROOT,MAX);
			getcwd(ROOT,sizeof(ROOT));	
			cout<<"******ROOT****"<<ROOT<<endl;
			//create file path


			bzero(filepath,MAX);
			getcwd(filepath,MAX);
        		strncat(filepath,"/DFC/",5);
        		strncat(filepath,username,strlen(username));
        		strncat(filepath,"/",1);
			strncat(filepath,file_name,strlen(file_name));
			path_len=strlen(filepath);


			/*strncpy(filepath,ROOT,strlen(ROOT));
			strncat(filepath,"/DFC/",strlen("/DFC/"));
			cout<<"File path"<<filepath<<endl;
			strncat(filepath,file_name,strlen(file_name));*/
			cout<<"File path"<<filepath<<endl;
			//receive file contents*/
			fd.open(filepath,fstream::out|fstream::binary);
			if(fd.fail())	perror("fd.open in receive parts"); 
			int key=compute_md5sum(password,strlen(password));
			while(file_size--){
				recv(cfd[j],buf,1,0);
				encrypt_data(buf,1,key);
				fd.write(buf,1);
			}
			fd.close();
			cout<<"writing is done"<<endl;
		}
		delete(file_name);
		return 1;
	}
	else{
		cout<<err_mesg<<endl;
		return 0;
	}

}
void get_combined_file(char filename[MAX],int file_size,int chunk_size,int left_size){
	char *f1_name,*f2_name,*f3_name,*f4_name,file_main[MAX],*buf,CLI_PATH[MAX];
	fstream fd,f1,f2,f3,f4;
	int len;
	bzero(CLI_PATH,MAX);
	getcwd(CLI_PATH,sizeof(CLI_PATH));
        strncat(CLI_PATH,"/DFC/",5);
        mkdir(CLI_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strncat(CLI_PATH,username,strlen(username));
        strncat(CLI_PATH,"/",1);
        mkdir(CLI_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout<<"creating the directory for all clients files\tin"<<CLI_PATH<<endl;
	for(int i=0;i<4;i++){
		if(strstr(file_parts[i],".1.")){
			cout <<"Found the first part"<<endl;
			len=strlen(file_parts[i])+strlen(CLI_PATH)+1;
        		f1_name=new char[len];
			bzero(f1_name,len);
			strcpy(f1_name,CLI_PATH);
        		strcat(f1_name,"/");
        		strncat(f1_name,file_parts[i],strlen(file_parts[i]));
        		f1_name[len]='\0';
		}
		else if(strstr(file_parts[i],".2.")){
			cout <<"Found the second part"<<endl;
			len=strlen(file_parts[i])+strlen(CLI_PATH)+1;
        		f2_name=new char[len];
			bzero(f2_name,len);
			strcpy(f2_name,CLI_PATH);
        		strcat(f2_name,"/");
        		strncat(f2_name,file_parts[i],strlen(file_parts[i]));
        		f2_name[len]='\0';
                }
		else if(strstr(file_parts[i],".3.")){
			cout <<"Found the third part"<<endl;
			len=strlen(file_parts[i])+strlen(CLI_PATH)+1;
        		f3_name=new char[len];
			bzero(f3_name,len);
			strcpy(f3_name,CLI_PATH);
        		strcat(f3_name,"/");
        		strncat(f3_name,file_parts[i],strlen(file_parts[i]));
        		f3_name[len]='\0';
                }
		else if(strstr(file_parts[i],".4.")){
			cout <<"Found the fourth part"<<endl;
			len=strlen(file_parts[i])+strlen(CLI_PATH)+1;
        		f4_name=new char[len];
			bzero(f4_name,len);
			strcpy(f4_name,CLI_PATH);
        		strcat(f4_name,"/");
        		strncat(f4_name,file_parts[i],strlen(file_parts[i]));
        		f4_name[len]='\0';
                }
	}
	cout<<"File in sorted order:"<<f1_name<<"\n"<<f2_name<<"\n"<<f3_name<<"\n"<<f4_name<<endl;
	cout<<"Lets combine the entire file into single one"<<endl;
	bzero(file_main,len);
	strcpy(file_main,CLI_PATH);
	strcat(file_main,"/");
	strncat(file_main,filename,strlen(filename));
	cout<<"File is: "<<file_main<<endl;
	fd.open(file_main,fstream::out|fstream::binary);
	if(fd.fail())	{
		perror("combined get file open failed:");
		return;
	}
	f1.open(f1_name,fstream::in|fstream::binary);
	f2.open(f2_name,fstream::in|fstream::binary);
	f3.open(f3_name,fstream::in|fstream::binary);
	f4.open(f4_name,fstream::in|fstream::binary);
	buf=new char[chunk_size];
	bzero(buf,chunk_size);
	f1.read(buf,chunk_size);
	//cout<<"Red buffer"<<buf<<endl;
        fd.write(buf,chunk_size);
	//delete buf;
	buf=new char[chunk_size];
	bzero(buf,chunk_size);
        f2.read(buf,chunk_size);
	//cout<<"Red buffer"<<buf<<endl;
        fd.write(buf,chunk_size);
	//delete buf;
	buf=new char[chunk_size];
	bzero(buf,chunk_size);
        f3.read(buf,chunk_size);
	//cout<<"Red buffer"<<buf<<endl;
        fd.write(buf,chunk_size);
	//delete buf;
	buf=new char[left_size];
	bzero(buf,left_size);
        f4.read(buf,left_size);
	//cout<<"Red buffer"<<buf<<endl;
        fd.write(buf,left_size);
	//delete buf;
	fd.close();
	f1.close();
	f2.close();
	f3.close();
	f4.close();
	delete (f1_name);
	delete (f2_name);
	delete (f3_name);
	delete (f4_name);
	cout<<"Got the file and is in:	"<<CLI_PATH<<endl;
}


void get_complete_file_parts(char file_main[MAX],int file_size,int chunk_size,int left_size){
	char file_search[MAX],path[MAX],CLI_PATH[MAX];
	int search_len=0,parts_count=0,len,path_len;
	bzero(CLI_PATH,MAX);
	getcwd(CLI_PATH,sizeof(CLI_PATH));
        strncat(CLI_PATH,"/DFC/",5);
        mkdir(CLI_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strncat(CLI_PATH,username,strlen(username));
        strncat(CLI_PATH,"/",1);
        mkdir(CLI_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        cout<<"creating the directory for all clients files\tin"<<CLI_PATH<<endl;
	cout<<"Need to combine all parts of "<<file_main<<"from distributed servers"<<endl;
	cout<<"Seek for file in "<<CLI_PATH<<endl;
	path_len=strlen(file_main)+1+strlen(CLI_PATH);
	bzero(path,path_len);
	strcpy(path,CLI_PATH);
	strncat(path,"/",1);
	strncat(path,file_main,strlen(file_main));
	remove(path);
	dir=opendir(CLI_PATH);
	if(dir){
		while((ent=readdir(dir))!=NULL){
			len=strlen(ent->d_name);
			file_search[len]='\0';
			bzero(file_search,len);
			strncpy(file_search,ent->d_name,len);
			if(strstr(file_search,file_main)){
				search_len=strlen(file_search);
				cout<<"file is :"<<file_search<<"of lenth"<<search_len<<endl;
				file_parts[parts_count]=new char[search_len];
				strncpy(file_parts[parts_count],file_search,search_len);
				file_parts[parts_count][search_len]='\0';
				cout<<"Found : "<<file_parts[parts_count]<<"of length: "<<strlen(file_parts[parts_count])<<endl;
				parts_count++;
			}
			//delete file_search;
		}
		if(parts_count==4){
			cout<<"Required parts of the file obtained"<<endl;
			cout<<"Files are :"<<endl;
			for(int i=0;i<parts_count;i++)
				cout<<"File "<<i<<"is :"<<file_parts[i]<<endl;
			get_combined_file(file_main,file_size,chunk_size,left_size);
		}
		else{
			cout<<"sorry file parts not found, are you sure you have got all parts of it?"<<endl;
		}
	}	
	else{
		perror("opendir:");
	}
	
}


void get_process(char filename[MAX]){
	fstream fd;
	char file_type[MAX],*str_tmp,*buf;
	int hash,file_size=0,hash_mod,chunk_size,left_size,dec=0;
	cout<<"Need to get "<<filename<<"from distributed servers"<<endl;
        str_tmp=strstr(filename,".");
        if(str_tmp){
                strcpy(file_type,str_tmp);
                cout<<"file type is "<<file_type<<endl;
        }
        else{
                cout<<"No file type is specified!!!!"<<endl;
                strcpy(file_type,".dat");
                cout<<"file type is "<<file_type<<endl;
        }
        file_size=compute_file_size(filename);
	chunk_size=file_size/4;
        left_size=file_size-(chunk_size*3);
        cout<<"obtained File size is:"<<file_size<<endl;
	if(file_size>0){
	        buf=new char[file_size];
		bzero(buf,file_size);
	}
	else { cout<<"Received file size is 0 and exiting from get_process*******"<<endl;}
        fd.open(filename,fstream::in|fstream::binary);
        fd.read(buf,file_size);
        fd.close();
        hash=compute_md5sum(buf,file_size);
        cout<<"obtained hash value is: "<<hash<<endl;		
	hash_mod=hash%4;
	switch(hash_mod){
                case 0:
                        dec=receive_parts(0);
			if(dec==0)dec=receive_parts(1);
                        dec=receive_parts(2);
                        if(dec==0)receive_parts(3);
                        break;
                case 1:
                        dec=receive_parts(0);
			if(dec==0)dec=receive_parts(1);
                        dec=receive_parts(2);
                        if(dec==0)receive_parts(3);
                        break;
                case 2:
                        dec=receive_parts(0);
			if(dec==0)dec=receive_parts(1);
                        dec=receive_parts(2);
                        if(dec==0)receive_parts(3);
                        break;
                case 3:
                        dec=receive_parts(0);
			if(dec==0)dec=receive_parts(1);
                        dec=receive_parts(2);
                        if(dec==0)receive_parts(3);
                        break;
        }
	get_complete_file_parts(filename,file_size,chunk_size,left_size);
}

void create_client_dir(){
	char PATH[MAX];
	bzero(PATH,MAX);
	getcwd(PATH,sizeof(PATH));
	strncat(PATH,"/DFC/",5);
	mkdir(PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	strncat(PATH,username,strlen(username));
	strncat(PATH,"/",1);
	mkdir(PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	cout<<"creating the directory for all clients files\tin"<<PATH<<endl;
}

void  list_name_print(char file[MAX]){
	struct list{
		char file_name[100];
		int count;
		bool dir;
	};
	struct list list_files[1024];
	int k=0;
	int line_count=0,count=1,i=0,j=0,len;
	char buf[1024],filename[MAX];
	fstream fd;
	len=strlen(file);
	//filename=new char[len];
	bzero(filename,len);
	strncpy(filename,file,len);
	filename[len]='\0';
	cout<<"File name where are servers have written the listing is "<<filename<<endl;
	fd.open(filename,fstream::in);
	if(fd.fail()){
		//cout<<"In list_name_print"<<endl;
		perror("fail:open");
		return;
	}
	while(fd.getline(buf,1024)){
		line_count++;
	}
	fd.close();
	cout<<"total number of lines :"<<line_count<<endl;
	fd.open(filename,fstream::in);
	char lines_array[line_count][MAX];
	while(fd.getline(buf,1024)){
		int len;
		len=strlen(buf);
		//lines_array[i]=new char[len];
		strncpy(lines_array[i],buf,len);
		lines_array[i][len]='\0';
		i++;
	}
	fd.close();
	for (i = 1; i <line_count ; i++)
	{
		bool matching = false;
		for (j = 0; (j < i) && (matching == false); j++){
			if (strncmp(lines_array[i],lines_array[j],strlen(lines_array[j]))==0)
				matching = true;
		}
		if (!matching)
			count++;
	}
	cout<<"Total files:"<<count<<endl;
	k=0;
	if(count){
		//count++;
		char lines[count][MAX];
		i=1;
		j=0;
		for(i=1;i<line_count;i++){
			bool matching=false;
		        for(j=0;(j<i)&&(matching==false);j++){
		                if(strncmp(lines_array[i],lines_array[j],strlen(lines_array[j]))==0){
					matching=true;
		                }
		        }
			if (!matching){
				len=strlen(lines_array[i]);
				//lines[k]=new char [len];
				strncpy(lines[k],lines_array[i],len);
				lines[k][len]='\0';
				k++;
		        }
		}
		len=strlen(lines_array[0]);
		//cout<<"creating the memory of\t"<<len<<"bytes"<<endl;
		//lines[k]=new char [len];
		//cout<<"created the memory of\t"<<len<<"bytes"<<endl;
		strncpy(lines[k],lines_array[0],len);
		lines[k][len]='\0';
		cout<<"non duplicate files"<<endl;
		for(i=0;i<count;i++){
			cout<<lines[i]<<endl;;
		}
		char src[MAX],src_type[MAX];
		int flag=1,k=0,digit_flag=0,l=0;
		for(i=0;i<count;i++){
			flag=1;
			digit_flag=0;
			bzero(src,MAX);
			strncpy(src,lines[i],strlen(lines[i]));
			cout<<"adding the entry is\t:"<<src<<endl;
			if(strstr(src,".")){//files enumerated
				strcpy(src,strtok(src,"."));
				bzero(src_type,MAX);
				strcpy(src_type,strtok(NULL,"."));
				//cout<<"file type is "<<src_type<<endl;
				if(isdigit(src_type[0])){
					strcpy(src_type,"");
					digit_flag=1;
				}
				for(l=0;l<k;l++){
					if(strncmp(src,list_files[l].file_name,strlen(src))==0){
						flag=0;
						break;
					}
				}
				if(flag){
				strcpy(list_files[k].file_name,src);
				//cout<<"********************digit_flag*****************"<<digit_flag;
				if(!digit_flag){
					strcat(list_files[k].file_name,".");
					strcat(list_files[k].file_name,src_type);
				}
				list_files[k].count=1;
				cout<<"added to entry is:	"<<list_files[k].file_name<<endl;
				for(j=i+1;j<count;j++){
					if(strncmp(lines[j],src,strlen(src))==0){
						list_files[k].count++;		
					}
				}
				k++;
				}
			}
			else{//directories enumeration
				cout<<"directory listing:"<<endl;
                        	for(l=0;l<k;l++){
                                	if(strncmp(src,list_files[l].file_name,strlen(src))==0){
                                        	flag=0;
                                        	break;
                                	}
                        	}
                        	if(flag){
                                	strcpy(list_files[k].file_name,src);
                                	list_files[k].count=1;
					list_files[k].dir=1;
                                	cout<<"added to entry is:     "<<list_files[k].file_name<<endl;
                                	for(j=i+1;j<count;j++){
                                        	if(strncmp(lines[j],src,strlen(src))==0)
                                                	list_files[k].count++;
                                	}
                                	k++;
                        	}
			}	
		}
		cout<<"###############################################################################"<<endl;
		for(int i=0;i<k;i++){
			cout<<list_files[i].file_name<<"\t:"<<list_files[i].count<<endl;
		}
		for(int i=0;i<k;i++){
			if(list_files[i].count>=4){
				cout<<list_files[i].file_name<<endl;
			}
			else {
				if(list_files[i].dir)
					cout<<list_files[i].file_name<<endl;
				else
				cout<<list_files[i].file_name<<"[Incomplete]"<<endl;
			}
		}
		cout<<"###############################################################################"<<endl;
	}
	else{
		cout<<"No files in any of servres: please upload files onto servres before you list it"<<endl;
	}

}

void list_process(){
	int file_size,count=2,n,len;
        fstream fd1,fd;
        char buf[MAXLINE],filename[MAX];
	cout<<"Receive the four files from four servers that has long listing of files"<<endl;	
	len=strlen("list_temp_cli")+strlen(ROOT)+1;
	bzero(filename,MAX);
	bzero(buf,MAXLINE);
	strncpy(filename,ROOT,strlen(ROOT));
	strncat(filename,"/",1);
	strncat(filename,"list_temp_cli",strlen("list_temp_cli"));
	cout<<"All listing will be written in \t:"<<filename<<endl;
	fd.open(filename,fstream::out);
	if(fd.fail())   perror("fd.open in list_process");
	for(int j=0;j<4;j++){
		if(cfd[j]!=-1){
			//receive file size
			bzero(buf,MAXLINE);
			fd1.open("temp1",fstream::out);
			size_t server_len=sizeof(server_addr);
			while((n=recvfrom(cfd[j],buf,1,0,(struct sockaddr *)&server_addr,&server_len))>0){
				if(strncmp(buf,"\n",1)==0) {
				        fd1.close();
				        break;
				}
				else  fd1.write(buf,1);
			}
			fd1.open("temp1",fstream::in);
			bzero(buf,MAXLINE);
			fd1.getline(buf,MAXLINE);
			fd1.close();
			file_size=atoi(buf);
			cout<<"Received file size is :  "<<file_size<<endl;
		
			//receive file contents*/
			while(file_size--){
				recv(cfd[j],buf,1,0);
				fd.write(buf,1);
			}
			//cout<<"writing is done"<<endl;
		}
		else cout<<err_mesg<<endl;
	}
	fd.close();
	list_name_print(filename);
	cout<<"listing of files is done and now I am removing the file"<<endl;
	//remove(filename);
	//Implement code here:
}


void user_process(){
	char command[MAX],filename[MAX],dirname[MAX],buf[MAX];
	int n,size,len;
	fstream fd,fd1;
	size_t server_len=sizeof(server_addr);
	cout<<"you can have three options from servres\nPUT filename <dirname/>\nGET filename <dirname/>\nLIST <dirname/>\n enter command"<<endl;
	bzero(command,MAX);
	bzero(dirname,MAX);
	cin.getline(command,MAX);
	string command1,temp;
	vector <string> cds;
	command1=command;
	cout<<"\ncommand in string is\t:\n"<<command1<<endl;
	stringstream s(command1);
	while(s>>temp)
		cds.push_back(temp);
	size=cds.size();
	bzero(command,MAX);
	strncpy(command,cds[0].c_str(),strlen(cds[0].c_str()));
	if((strstr(cds[0].c_str(),"PUT")!=NULL)|(strstr(cds[0].c_str(),"GET")!=NULL)){
		bzero(filename,MAX);
		strcpy(filename,cds[1].c_str());
		cout<<"the filename is "<<filename<<endl;
		strncat(command," ",1);
		strncat(command,filename,strlen(filename));
		strncat(command," ",1);
		strncat(command,username,strlen(username));
		strncat(command," ",1);
                strncat(command,password,strlen(password));
		if(size==3){
			strncpy(dirname,cds[2].c_str(),strlen(cds[2].c_str()));
			strncat(command," ",1);
                	strncat(command,dirname,strlen(dirname));
			strncat(command,"\n",1);	
		}
		else{
			strncat(command,"\n",1);
		}
		if(strlen(dirname)!=0)	cout<<"directory name is\t:"<<dirname<<endl;
		
	}
	else if(strstr(cds[0].c_str(),"LIST")){
		strncat(command," ",1);
	        strncat(command,username,strlen(username));
        	strncat(command," ",1);
                strncat(command,password,strlen(password));
		if(size==2){
                        strncpy(dirname,cds[1].c_str(),strlen(cds[1].c_str()));
			strncat(command," ",1);
                        strncat(command,dirname,strlen(dirname));
			strncat(command,"\n",1);	
                }
		else{
			strncat(command,"\n",1);
		}
                if(strlen(dirname)!=0)     cout<<"directory name is\t:"<<dirname<<endl;
	}
	else{
		cout<<"Invalid command!!"<<endl;
		return ;
	}
	cout<<"command to be sent to servers :\t"<<command<<endl;
	for(int i=0;i<4;i++){
                int port = servers[i].port; 
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);
                server_addr.sin_addr.s_addr = inet_addr(servers[i].ip_address);
                if((cfd[i] = socket(AF_INET, SOCK_STREAM, 0))<0){
                        cout<<"Sorry couln't create the socket"<<endl;
                        perror("socket:");
                }
                if (connect(cfd[i],(struct sockaddr *)&server_addr, sizeof(server_addr))< 0){
                        perror("connect");
                        cout<<"cfd couln't connect to server for socket:\t"<<cfd[i]<<endl;
                        cfd[i]=-1;
                }
        }
        for(int i=0;i<4;i++){
                if(cfd[i]!=-1){
                        n=sendto(cfd[i],command,strlen(command),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
                        if(n<0){
                                perror("sendto in user_process:");
                        }
			bzero(buf,MAX);
			fd.open("temp_ack",fstream::out);
			if(fd.fail())	perror("In user process");
			while((n=recvfrom(cfd[i],buf,1,0,(struct sockaddr *)&server_addr,&server_len))>0){
				//cout<<"received from server is"<<buf<<endl;
         		       	if(strncmp(buf,"\n",1)==0) {
                        		fd.close();
                        		break;
                		}
                		else  fd.write(buf,1);
        		}
			bzero(buf,MAX);
			fd1.open("temp_ack",fstream::in);
			if(fd1.fail())   perror("In user process");
			fd1.read(buf,44);
			fd1.close();
			cout<<"Received from server is \t:"<<buf<<endl;
			if(strstr(buf,"valid Username/Password. noPlease try again")){
				cout<<"user is genuine"<<endl;
			}
			else{
				len=strlen(buf);
				strncpy(err_mesg,buf,len);
				err_mesg[len]='\0';
				cfd[i]=-1;
				cout<<err_mesg<<endl;
			}
                }
        }
	cout<<"Now command is sent lets process the commands at client"<<endl;
	if(strstr(cds[0].c_str(),"PUT")){
		cout<<"User wants put file onto server"<<endl;
		put_process(filename);
	}
	else if(strstr(cds[0].c_str(),"GET")){
		cout<<"User wants to get files from servers"<<endl;
		create_client_dir();
		get_process(filename);
	}
	else if(strstr(cds[0].c_str(),"LIST")){
		cout<<"List of files in servers"<<endl;
		list_process();
	}
}



int main(int argc,char **argv){
	if(argc!=2){
		cout<<"Usage: "<<argv[0]<<" <dfc.conf> "<<endl;
		exit(0);
	}
	if (getcwd(ROOT,sizeof(ROOT))){
                cout<<"The current directory is"<<ROOT<<endl;
        }
        else{
		perror("getcwd:");
	}
	parser(argv[1]);
	while(1){
		user_process();
		cout<<"Done with processing all files"<<endl;
		//for(int i=0;i<4;i++)
		//close(cfd[i]);
	}
	return 0;

			}
