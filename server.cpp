#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/md5.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h> 
#define MAXLINE 1024
#define MAXCLIENTS 1024
#define MAX 100
using namespace std;


//char file_parts[2][MAX];
char *file_parts[2],file_type[MAX],file_main[MAX];
int sfd,cfd;
char ROOT[MAXLINE],username[MAX],password[MAX],dirname[MAX];
struct sockaddr_in cliaddr, servaddr;
DIR *dir;
struct dirent *ent;

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

void encrypt_data(char *message,int len,int KEY){
        int i;
        //printf("Encrypting the message\n");
        for(i = 0; i <len; i++)
                message[i] ^= KEY;

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

void list_process(){
	fstream fd;
	int file_size,len;
	char file_size_str[MAX],*buf;
	char filename[MAX],file_suffix[MAX],PATH[MAX],ROOT_PATH[MAX];
	bzero(ROOT_PATH,MAX);
	if(strlen(dirname)!=0){
		cout<<"dirname exists"<<dirname<<endl;
		len=strlen(ROOT)+1+strlen(dirname);
		strncpy(ROOT_PATH,ROOT,strlen(ROOT));
		strncat(ROOT_PATH,"/",1);
		strncat(ROOT_PATH,dirname,strlen(dirname));
		ROOT_PATH[len]='\0';
	}
	else{
		cout<<"directory is not given"<<endl;
		len=strlen(ROOT);
		strncpy(ROOT_PATH,ROOT,len);
		ROOT_PATH[len]='\0';
	}
	bzero(filename,MAX);
	bzero(PATH,MAX);
	getcwd(PATH, sizeof(PATH));
	strncpy(filename,PATH,strlen(PATH));
	strncat(filename,"/",1);
	strncat(filename,"list_temp_serv_",strlen("list_temp_serv_"));
	if(strstr(ROOT,"DFS1")){
		strncat(filename,"DFS1",4);
	}
	else if(strstr(ROOT,"DFS2")){
		strncat(filename,"DFS2",4);
	}
	else if(strstr(ROOT,"DFS3")) {
		strncat(filename,"DFS3",4);
	}
	else if(strstr(ROOT,"DFS4")){
		strncat(filename,"DFS4",4);
	}
	fd.open(filename,fstream::out);
	if(fd.fail())	perror("fd.open in list_process:");
	cout<<"Welcome to server listing"<<endl;
	cout<<"I am gone read all files present in the user:\t"<<username<<"and write it to file\t "<<filename<<"in the path"<<ROOT_PATH<<endl;
	dir=opendir(ROOT_PATH);
	if(dir){
		while((ent=readdir(dir))!=NULL){
			if((strncmp(ent->d_name,".",1)!=0)&&(strncmp(ent->d_name,"..",2)!=0)){
				cout<<ent->d_name<<endl;
				fd.write(ent->d_name,strlen(ent->d_name));
				fd.write("\n",1);
			}
		}
	}
	else
		perror("opendir");
	fd.close();
	cout<<"File with all files listing is ready to send"<<endl;	

	//send the file size

	file_size=compute_file_size(filename);
        sprintf(file_size_str,"%d",file_size);
        strcat(file_size_str,"\n");
	cout<<"sending the file_size_str:"<<file_size_str<<endl;
        sendto(cfd,file_size_str,strlen(file_size_str),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));

	//send the file contents

        fd.open(filename,fstream::in);
	buf=new char[file_size];
        fd.read(buf,file_size);
        sendto(cfd,buf,file_size,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
	delete buf;
        fd.close();

	cout<<"Done sending all the files of user"<<endl;
	remove(filename);
}


void send_parts(char filepath[MAX],int i){
	fstream fd;
	int count=2,file_size,path_len=0;
	char *buf,file_size_str[MAX],file_name_str[MAX],file_path[MAX];
	cout<<"Lets send the"<<file_parts[i]<<endl;
	//while(count--){
                //sending the file size of part1
		bzero(file_size_str,MAX);
		cout<<"File path of the file"<<filepath<<endl;
		path_len=strlen(filepath)+strlen(file_parts[i])+1;
		cout<<path_len<<endl;
		//file_path=new char[path_len];
		strcpy(file_path,filepath);
		strcat(file_path,"/");
		strcat(file_path,file_parts[i]);
		file_path[path_len]='\0';
		cout<<"file path is "<<file_path<<endl;
		file_size=compute_file_size(file_path);
                sprintf(file_size_str,"%d",file_size);
                strcat(file_size_str,"\n");
		cout<<"sending the file_size_str:"<<file_size_str<<endl;
                sendto(cfd,file_size_str,strlen(file_size_str),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));

                //sending the file name of part1
		bzero(file_name_str,MAX);
		cout<<"file name to be sent is"<<file_parts[i]<<endl;
                strncpy(file_name_str,file_parts[i],strlen(file_parts[i]));
                strncat(file_name_str,"\n",1);
		cout<<"sending the file name:"<<file_name_str<<endl;
                sendto(cfd,file_name_str,strlen(file_name_str),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));

                //sending the file content of part1
                fd.open(file_path,fstream::in|fstream::binary);
		buf=new char[file_size];
                fd.read(buf,file_size);
		int key=compute_md5sum(password,strlen(password));
		encrypt_data(buf,file_size,key);
                sendto(cfd,buf,file_size,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
		//delete buf;
		//delete (file_path);
                fd.close();
		//i++;
	//}	
}

void get_process(){
	char file_search[MAX],ROOT_PATH[MAX],part[MAX],buf[MAX];
	int search_len=0,parts_count=0,len,n,dec;
	/*fstream fd;
	socklen_t client_len=sizeof(cliaddr);
	//receive decision about wherther to send or not
	bzero(buf,MAX);
	fd.open("temp1",fstream::out);
        while((n=recvfrom(cfd,buf,1,0,(struct sockaddr *)&cliaddr,&client_len))>0){
		cout<<"received from client is :"<<buf<<endl;
                if(strncmp(buf,"\n",1)==0) {
                        fd.close();
                        break;
                }
                else fd.write(buf,1);
        }
        fd.open("temp1",fstream::in);
        bzero(buf,MAX);
        fd.getline(buf,MAX);
        fd.close();
	dec=atoi(buf);
        cout<<"\nReceived from client about decision is"<<dec<<endl;	
	//receive which part to be sent?
	bzero(buf,MAX);
        fd.open("temp",fstream::out);
        while((n=recvfrom(cfd,buf,1,0,(struct sockaddr *)&cliaddr,&client_len))>0){
		cout<<"received from client is :"<<buf<<endl;
                if(strncmp(buf,"\n",1)==0) {
                        fd.close();
                        break;
                }
                else fd.write(buf,1);
        }
        fd.open("temp",fstream::in);
        bzero(buf,MAX);
        fd.getline(buf,MAX);
        fd.close();
	bzero(part,1);
        strncpy(part,buf,1);
	part[1]='\0';
        cout<<"\nReceived from client about part is"<<part<<endl;*/
	//commmand process
	cout<<"This is GET command processing"<<endl;
	cout<<"Need to search for : "<<file_main<<" of type : "<<file_type<<"  for : "<<username<<"in the direcorty: "<<ROOT<<endl;	
	if(strlen(dirname)!=0){
		cout<<"dirname is\t:"<<dirname<<endl;
		len=strlen(ROOT)+1+strlen(dirname);
		bzero(ROOT_PATH,MAX);
		strncpy(ROOT_PATH,ROOT,strlen(ROOT));
		strncat(ROOT_PATH,"/",1);
		strncat(ROOT_PATH,dirname,strlen(dirname));
		ROOT_PATH[len]='\0';
		cout<<"The files will be fected from \t:"<<ROOT_PATH<<endl;
	}
	else{
		cout<<"No dirname"<<endl;
		bzero(ROOT_PATH,MAX);
		len=strlen(ROOT);
		strncpy(ROOT_PATH,ROOT,strlen(ROOT));
		ROOT_PATH[len]='\0';		
		cout<<"The files will be fected from \t:"<<ROOT_PATH<<endl;
	}
	dir=opendir(ROOT_PATH);
	if(dir){
		while((ent=readdir(dir))!=NULL){
			len=strlen(ent->d_name);
			//file_search=new char[len];
			strncpy(file_search,ent->d_name,len);
			file_search[len]='\0';
			if(strstr(file_search,file_main)){
				search_len=strlen(file_search);
				cout<<"file is :"<<file_search<<"of length"<<search_len<<endl;
				file_parts[parts_count]=new char[search_len];
				strncpy(file_parts[parts_count],file_search,search_len);
				file_parts[parts_count][search_len]='\0';
				cout<<"Found : "<<file_parts[parts_count]<<"of length: "<<strlen(file_parts[parts_count])<<endl;
				parts_count++;
			}
			//delete file_search;
		}
		if(parts_count==2){
			cout<<"Required parts of the file obtained"<<endl;
			cout<<"Files are :"<<endl;
			for(int i=0;i<parts_count;i++){
				cout<<"File "<<i<<"is :"<<file_parts[i]<<endl;
				send_parts(ROOT_PATH,i);
			}
		}
		else{
			cout<<"sorry file parts not found, are you sure you have distributed it?"<<endl;
		}
	}	
	else{
		perror("opendir:");
	}
}

int parser(vector <string> cds){
	int size=cds.size(),len=0,dfs_size;
	string temp,line;
	char buffer[MAX];
	fstream fd;
	cout<<"The size of the command is \t:"<<size<<endl;
	if((strncmp(cds[0].c_str(),"PUT",3)==0)|(strncmp(cds[0].c_str(),"GET",3)==0)){
		cout<<"parsing username and password from"<< cds[0].c_str()<<"command"<<endl;
		bzero(username,MAX);
		bzero(password,MAX);
		len=strlen(cds[2].c_str());
		strncpy(username,cds[2].c_str(),len);
		username[len]='\0';
		len=strlen(cds[3].c_str());
                strncpy(password,cds[3].c_str(),len);
                password[len]='\0';
	}
	else if(strncmp(cds[0].c_str(),"LIST",3)==0){
		cout<<"parsing username and password from LIST command"<<endl;
                bzero(username,MAX);
                bzero(password,MAX);
                len=strlen(cds[1].c_str());
                strncpy(username,cds[1].c_str(),len);
                username[len]='\0';
                len=strlen(cds[2].c_str());
                strncpy(password,cds[2].c_str(),len);
                password[len]='\0';
	}
        cout<<"Username received from client: "<<username<<"Password from client is: "<<password<<endl;
        bzero(buffer,MAX);
        fd.open("dfs.conf",fstream::in);
        if(fd.fail()){
                cout<<"I cannot run without the configuration file"<<endl;
                exit(0);
        }
	bzero(buffer,MAX);
        while(fd.getline(buffer,MAX)){
		cout<<"Red from buffer is"<<buffer<<endl;
		vector <string> dfs;
		line=buffer;
		stringstream s(line);
		while(s>>temp){
			dfs.push_back(temp);
		}
		dfs_size=dfs.size();
		cout<<"vector size is"<<dfs_size<<endl;
		for(int i=0;i<dfs_size;i++){
			cout<<"Obtained from dfs.conf\t"<<dfs[i].c_str()<<endl;
		}
		cout<<"Username:"<<username<<"obtained username is:"<<dfs[0].c_str()<<endl;
		cout<<"Password:"<<password<<"obtained password is:"<<dfs[1].c_str()<<endl;
		if((strstr(username,dfs[0].c_str()))&&(strstr(password,dfs[1].c_str()))){
			cout<<"user found and password is matching"<<endl;
                        strncat(ROOT,"/",1);
                        strncat(ROOT,username,strlen(username));
                        mkdir(ROOT, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        cout<<"The file path will be:"<<ROOT<<endl;
			sendto(cfd,"valid Username/Password. noPlease try again\n",strlen("valid Username/Password. noPlease try again")+1,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
			return 1;
		}
		bzero(buffer,MAX);
        }
        if(fd.eof()){
                cout<<"Sorry username not found"<<endl;
		sendto(cfd,"Invalid Username/Password. Please try again\n",strlen("Invalid Username/Password. Please try again")+1,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
		return 0;
	}
}

void client_handle(){
	int n,file_size,count=2,size,parse_flag=0;
	char buf[MAXLINE],filepath[MAX],file_name[MAX],dirpath[MAX];
	vector <string> cds;
	string command,temp;
	fstream fd,fd1,fd2,fd3;
	socklen_t client_len=sizeof(cliaddr);
	//receive command from client
	fd1.open("temp",fstream::out);
	while((n=recvfrom(cfd,buf,1,0,(struct sockaddr *)&cliaddr,&client_len))>0){	
		if(strncmp(buf,"\n",1)==0) {
			fd1.close();
			break;
		}
		else fd1.write(buf,1);
	}
	fd1.open("temp",fstream::in);
	bzero(buf,MAXLINE);
	fd1.getline(buf,MAXLINE);
	fd1.close();
	cout<<"\nReceived from client is"<<buf<<endl;
	command=buf;
	stringstream s(command);
	while(s>>temp)
		cds.push_back(temp);
	size=cds.size();
	//locate the user and create corresponding directory
	parse_flag=parser(cds);
	if(parse_flag==0)
		return;
	//parse the input to process them accordingly
	bzero(buf,MAXLINE);
	fd1.open("temp",fstream::in);
        fd1.getline(buf,MAXLINE);
        fd1.close();
	cout<<"\nbuf after parsing:"<<buf<<endl;
	
	cout<<"Lets get the filename and file type"<<endl;
	strtok(buf," ");
	strcpy(file_main,strtok(NULL," "));
	cout<<"file main is:"<<file_main<<endl;
	char *str_tmp;
	str_tmp=strstr(file_main,".");
	if(str_tmp){
		strcpy(file_type,str_tmp);
		cout<<"file type is "<<file_type<<endl;
	}
	else{
		cout<<"No file type is specified!!!!"<<endl;
		strcpy(file_type,".dat");
		cout<<"file type is "<<file_type<<endl;
	}	
	bzero(buf,MAXLINE);
	fd1.open("temp",fstream::in);
        fd1.getline(buf,MAXLINE);
        fd1.close();
        cout<<"\nbuf after parsing:"<<buf<<endl;
	if(strstr(buf,"PUT")){
		bzero(dirname,MAX);
		if(size==5)
			strncpy(dirname,cds[4].c_str(),strlen(cds[4].c_str()));
		if(strlen(dirname)!=0)	cout<<"dirname is \t"<<dirname<<endl;
		while(count--){
		//receive file size
		bzero(buf,MAXLINE);
		fd1.open("temp1",fstream::out);
		while((n=recvfrom(cfd,buf,1,0,(struct sockaddr *)&cliaddr,&client_len))>0){
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
                while((n=recvfrom(cfd,buf,1,0,(struct sockaddr *)&cliaddr,&client_len))>0){
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
		cout<<"File type is"<<file_type<<endl;
		int len=strlen(buf)+strlen(file_type);
		//file_name=new char[len];
		bzero(file_name,MAX);
                strncpy(file_name,buf,strlen(buf)); 
		strncat(file_name,file_type,strlen(file_type));
		file_name[len]='\0';
                cout<<"Received file name is :  "<<file_name<<endl;

		if(strlen(dirname)!=0){
			cout<<"directory is mentioned in the command\t:"<<dirname<<endl;
			bzero(dirpath,MAX);
			cout<<"The path of ROOT\t:"<<ROOT<<endl;
			strncpy(dirpath,ROOT,strlen(ROOT));
			strncat(dirpath,"/",1);
			strncat(dirpath,dirname,strlen(dirname));
			cout<<"All files will be saved in the path\t:"<<dirpath<<endl;
			mkdir(dirpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);	
			len=strlen(dirpath)+strlen(file_name)+1;
			//filepath=new char[len];
			bzero(filepath,MAX);
			strncpy(filepath,dirpath,strlen(dirpath));
			strncat(filepath,"/",1);
                        strncat(filepath,file_name,strlen(file_name));
                        filepath[len]='\0';
		}
		else{
			//create file path
			len=strlen(ROOT)+strlen(file_name)+1;	
			//filepath=new char[len];
			bzero(filepath,len);
			strncpy(filepath,ROOT,strlen(ROOT));
                        strncat(filepath,"/",1);
			strncat(filepath,file_name,strlen(file_name));
			filepath[len]='\0';
		}
		cout<<"File path"<<filepath<<endl;

		//receive file contents
		bzero(buf,MAXLINE);
		fd.open(filepath,fstream::out|fstream::binary);
		if(fd.fail())	perror("fd.open in client_handle:");
		while(file_size--){
			recv(cfd,buf,1,0);
			int key=compute_md5sum(password,strlen(password));
			encrypt_data(buf,1,key);
			fd.write(buf,1);
		}
		fd.close();
		cout<<"writing is done"<<endl;
		}
		//delete(file_name);
		//delete(filepath);		
	}else if(strstr(buf,"GET")){
		bzero(dirname,MAX);
                if(size==5)
                        strncpy(dirname,cds[4].c_str(),strlen(cds[4].c_str()));
                if(strlen(dirname)!=0)  cout<<"dirname is \t"<<dirname<<endl;
		int bla=8;
		while(bla--){
		get_process();}
	}
	else if(strstr(buf,"LIST")){
		cout<<"This is list command processing"<<endl;
		bzero(dirname,MAX);
                if(size==4)
                        strncpy(dirname,cds[3].c_str(),strlen(cds[3].c_str()));
                if(strlen(dirname)!=0)  cout<<"dirname is \t"<<dirname<<endl;
		list_process();
	}	
	else {
		cout<<"Sorry I don't understand command you sent!!!"<<endl;
	}
}

int main(int argc,char **argv){
	int childpid,port;
	
	if(argc!=3){
		cout<<"Usage:"<<argv[0]<<" /DFS_PATH "<<" port number "<<endl;
		exit (0);
	}
	
	port=atoi(argv[2]);
	if (getcwd(ROOT, sizeof(ROOT))){
		strcat(ROOT,argv[1]);
       		cout<<"The current directory is"<<ROOT<<endl;
	}
   	else
       		perror("getcwd() error");
	mkdir(ROOT, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr =inet_addr("127.0.0.1");
        servaddr.sin_port = htons(port);
	socklen_t client_len=sizeof(cliaddr);
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (struct sockaddr *) &servaddr, sizeof(servaddr));

	if((sfd=socket(AF_INET,SOCK_STREAM, 0))==-1){
                perror("Scoket creation failed");
                exit(1);
        }
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (struct sockaddr *) &servaddr, sizeof(servaddr));
        if(bind (sfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
                perror("could not bind\n");
                exit(1);
        }

        if((listen(sfd,MAXCLIENTS))==-1){
                perror("Listening of server failed");
                exit(1);
        }
        cout<<"Listen is done and waiting for clients to accept"<<endl;
        while(1){
                cfd = accept(sfd, (struct sockaddr *) &cliaddr,&client_len);
                if(cfd==-1){
                        perror("accept of client is failed");
                }
                if ( (childpid = fork ()) == 0 )
                {
                        close(sfd);
                        cout<<"I am child and my sfd is \n"<<sfd<<endl;
                        client_handle();
                        cout<<"\nDone handling the client\n"<<endl;
			exit(0);
                }
                close(cfd);
        }
	close(sfd);
        return 0;
}
