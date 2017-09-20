/************************************************************************************************************************

	spider www/web crawler - by gerabelo			SEGUNDA VERSAO

	www.geraldorabelo.com					nesta versão - pesquisa por palavra

	16/08/2010 03:20 AM


		arguments: host port page

		compiling : gcc -o spider spider.c `mysql_config --cflags --libs`

		02/10/2010 -	tratamento na passagem do caractere & nas paginas para o spider. 121
				setamento do campo status na base com valores de 1 à 6 de acordo com a classificação dada pelo parser (file 4, ignored 3, falha ao conectar 2, sucesso 1, status 0 significa que o link ainda nao passou pelo parser).

		30/12/2010 -	Exemplo de execução inicial: ./spider www.ecenter.com.br 80 /valer/html/index.php 0
				Exemplo de spider.conf:	192.168.0.2/root/Jgfrabelo82/bot/http

************************************************************************************************************************
	



//TRIGGER
/*
CREATE TRIGGER trg_md5 BEFORE INSERT ON spidering
FOR EACH ROW
BEGIN
	set @tmp = MD5(CONCAT(NEW.ip,NEW.page,NEW.data));
	IF EXISTS (SELECT * FROM spidering WHERE md5=@tmp) 
	THEN 
		SET NEW=NULL;
	ELSE 
		SET NEW.md5 = @tmp;
	END IF;
END;//
*************************************************************************************
*************************************************************************************/



#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "unistd.h"		//usleep

#include "netinet/in.h" 	//socket's hearders
#include "arpa/inet.h"  	
#include "sys/types.h"  	
#include "sys/socket.h" 	

#include "netdb.h"      	// gethostbyname()

#include "mysql/mysql.h"	

#define MAXSIZE 800
#define MAX 500
#define MIN 64

int isfile(char *url);

int main(int argc, char **argv) {

  int sockfd,n,sql_err_count=0,port_=0,num_fields,c=0,f_ip;
  char *url,*url_,url_id[9],buff[MAXSIZE]="",sBuff[MAXSIZE],host[MAX]="",page[MAX]="",pagina[MAX]="-",port[6]="80",enter[MAX]="192.168.0.2/ce/index.php",sql_query[MAX]="",*tmp,tmp2[MIN]="",tmp3[MIN],data[MAX]="",link[MAX]="",getpage[MAX]="GET /",db_server[MAX],db_user[MAX],db_passwd[MAX],db_db[MAX],command[MAX]="",type[MAX]="";

  struct hostent *ip;					// struct do netdb para converter host para ip
  struct sockaddr_in end;				// struct da socket

  struct hostent *ip_;					// struct do netdb para converter host para ip
//  struct sockaddr_in end;				// struct da socket

  time_t seconds;
  time_t rawtime;
  time ( &rawtime );
  seconds = time (NULL);
//  memset(tmp2,'\0',sizeof(char)*MIN);// cleanning  	
  sprintf(tmp2,"%s",ctime (&rawtime));
  //tmp2[strlen(tmp2)-1]='\0';
	
  MYSQL *conn;
  MYSQL_RES *result;
  MYSQL_ROW row;

  memset(type,'\0',sizeof(char)*MAX);// cleanning  
  memset(host,'\0',sizeof(char)*MAX);// cleanning  
 
  //printf("\n[SPIDER STARTED");
  
  FILE *config_File;
  config_File = fopen("spider.conf", "r");
  if (config_File == NULL) printf("config file does not exist, please check!\n");
  fscanf (config_File, "%99[^/]/%99[^/]/%99[^/]/%99[^/]/%99[^\n]",db_server,db_user,db_passwd,db_db,type);
  fclose(config_File);  
  //printf("\n.%s.\n",type);
  sprintf(host,"%s",argv[1]);
  sprintf(port,"%s",argv[2]);
  //printf("\nExecuting spider...\n");
  if (argc == 5) {
	sprintf(page,"%s",argv[3]);
	sprintf(url_id,"%s",argv[4]);
  } else { sprintf(url_id,"%s",argv[3]); }

  sscanf(page,"\"%99[^\"]",page);
  //sprintf(pagina,url_id);
  //sscanf(page,pagina);
  sprintf(pagina,"%s",page);
  //printf("\n\npage= %s url_id=%s\n\n",page,url_id);
  
  ip = gethostbyname(host);  // função da lib netdb.h para converter ai nosso host para IP
  url=inet_ntoa(*(struct in_addr *)ip->h_addr);
  
  f_ip=0;
  if (!strcmp(url,"200.241.126.119")) { f_ip=1; } else { printf("\n%s\n",url); return 0; }
/*
  if (url[0]=='2') { 
	if (url[1]=='0') {
		if (url[2]=='0') { f_ip=1; }
		else if (url[2]=='1') { f_ip=1; }
	}	
  }
  else if (url[0]=='1') {
	if (url[1]=='8') {
		if (url[2]=='7') { f_ip=1; }
		else if (url[2]=='9') { f_ip=1; }
	}	
	else if (url[1]=='5' && url[2]=='0') { f_ip=1; }
	else if (url[1]=='6' && url[2]=='1') { f_ip=1; }
  } 
*/
  conn = mysql_init(NULL);
  mysql_real_connect(conn, db_server, db_user, db_passwd, db_db, 0, NULL, 0);

  //printf("\npage = \"%s\"\n", page);

  if (!strlen(page)) { 
	sprintf(page," HTTP/1.1\nHost: "); 
  } else { 
	if (isfile(page)) {
		//printf("\nWarning: a File has been found on url and it will be ignored.\n");
		memset(sql_query,'\0',sizeof(char)*MAX);// cleanning
		sprintf(sql_query,"INSERT INTO files (url) VALUES('");
		strcat(sql_query,host);
		strcat(sql_query,"/");
		strcat(sql_query,pagina);
		strcat(sql_query,"');");
		mysql_query(conn, sql_query);// ? printf("\nSQL Erro! FILES %s %s %s %s\n%s",url,host,port,page,sql_query) : NULL;
		sprintf(sql_query,"UPDATE spidering SET status=4 WHERE id=");
		strcat(sql_query,url_id);
		mysql_query(conn, sql_query);// ? sql_err_count++ : NULL;
		mysql_close(conn);
		return 0;		
	} else {
		strcat(page," HTTP/1.1\nHost: ");
	}
  }


  if (!f_ip) {
	memset(sql_query,'\0',sizeof(char)*MAX);// cleanning
	sprintf(sql_query,"INSERT INTO ignoreds (ip,url,port,page) VALUES('");
	strcat(sql_query,url);
	strcat(sql_query,"','");
	strcat(sql_query,host);
	strcat(sql_query,"','");
	strcat(sql_query,port);
	strcat(sql_query,"','");
	strcat(sql_query,pagina);
	strcat(sql_query,"')");
	//printf("\nWarning: the url is out of selected range and it will be ignored.\n\n");	
	mysql_query(conn, sql_query);// ? printf("\nSQL Error! IGNOREDS %s %s %s %s\n",url,host,port,page) : NULL;
	sprintf(sql_query,"UPDATE spidering SET status=3 WHERE id=");
	strcat(sql_query,url_id);
	mysql_query(conn, sql_query);// ? sql_err_count++ : NULL;
	mysql_close(conn);
	//sleep(1);
  //printf("0 objects founds in 0 seconds. ");
  //printf("0 Errors in SQL Queries!\n");
  //printf("****************************************************************************************\n\n");

	return 0;
  }

  strcat(page,host);
  strcat(page,"\nConnection: close\r\n");
//  strcat(page,"User-Agent: spider - www.geraldorabelo.com/spider.php \r\n");
  strcat(page,"User-Agent: Mozilla/5.0 (X11; U; Linux i686; pt-BR; rv:1.9.1.9) Gecko/20100401 Ubuntu/9.10 (karmic) Firefox/3.5.9 \r\n");
  strcat(page,"Referer: http://ufam.kicks-ass.org/\r\n");
  strcat(page,"Accept: application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n");
//  strcat(page,"Content-Length: 800\r\n");
//  strcat(page,"Accept-Encoding: chunked\r\n");//strcat(page,"Accept-Encoding: gzip,deflate,sdch\r\n");
  strcat(page,"Accept-Language: pt-BR,pt;q=0.8,en-US;q=0.6,en;q=0.4\r\n");
  strcat(page,"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n\n");

  strcat(getpage,page);
//  printf("\n%s",getpage);

  //memset(buff,'\0',sizeof(char)*MAXSIZE);
  memset(sBuff,'\0',sizeof(char)*MAXSIZE);
  strcpy(sBuff,getpage);

  if( (sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0 ){
    close(sockfd);
    return errno;
  } //else { printf("[SOCKET OPENNED]"); }

  port_=atoi(port);
  end.sin_family=AF_INET;
  end.sin_port=htons(port_);

  if( inet_pton(AF_INET,url,&end.sin_addr) < 0){
    close(sockfd);	
    return errno;
  }

  memset(end.sin_zero,'\0',8);

  if(connect(sockfd,(struct sockaddr*)&end,sizeof(struct sockaddr)) < 0){
    close(sockfd);	
    return errno;
  } //else { printf("[SOCKET CONNECTED]"); }

  if ((send(sockfd, sBuff, strlen(sBuff), 0)) < 0) {
    close(sockfd);
    sprintf(sql_query,"UPDATE spidering SET status=2 WHERE id=");
    strcat(sql_query,url_id);
    mysql_query(conn, sql_query);// ? sql_err_count++ : NULL;
    //sleep(1);
    return errno;
  } else {
	//modifica o status da url na base.
	//printf("[HTTP GET SENT]"); 
	sprintf(sql_query,"UPDATE spidering SET status=1 WHERE id=");
	strcat(sql_query,url_id);
	mysql_query(conn, sql_query) ? sql_err_count++ : NULL;
	//sleep(1);
  }

  int data_pos=0, i=1;

  //buff = (char *) malloc(MAXSIZE);
  //memset(buff,'\0',sizeof(char)*MAXSIZE);
  //printf("[SOCKET READING]");
  for (i=1;read(sockfd,buff,MAXSIZE);i++) {
	if (i == 1) {
		tmp = (char *) malloc(MAXSIZE*sizeof(char));
		//printf("[CALLOC DONE]");
		strcpy(tmp,buff);
	} else { 
		tmp = realloc(tmp,(strlen(tmp)+strlen(buff)+1)*sizeof(char));
		//printf("[REALOC DONE]");
		strcat(tmp,buff);
        }
	//memset(buff,'\0',sizeof(char)*MAXSIZE);
	//printf("[READ TIMES %d]",i);
  }
  //tmp[strlen(tmp)-1]='\0';
//	sprintf(tmp,"%s",buff);	
//	fprintf(stdout,"%s",tmp);
	//printf("%s",tmp);

	for (i=0;i<strlen(tmp);i++) {
		if (tmp[i]==type[0] && data_pos==0) {			
			//printf("%d\n",strlen(type));
			data_pos=i;//encontrou candidato			
			for (c=1;c<strlen(type);c++) {//testando todos os caracteres 
				if (tmp[i+c]!=type[c]) {
					data_pos=0;					
					break; 
				}
			}
		}
		//aqui está a caracteristica do fim	
		if (data_pos!=0 && ( tmp[i]=='<' || tmp[i]=='>' || tmp[i]=='\n') ) { //a procura do fim
					
			//printf("data_pos <> 0\n");

			memset(link,'\0',sizeof(char)*MAX);// cleanning

			for(c=0;c<(i-data_pos);c++) {//fim menos inicio 

				if (tmp[data_pos+c]=='\'') { link[c]='\\'; link[c+1]='\''; c=c+1; data_pos=data_pos-1;} 

				else {link[c]=tmp[data_pos+c];}
			}

			//link[i-data_pos]='>';

			//obj_count++;			  	

			data_pos=0;

			//printf("\n[OBJECT: %s]",link);
			//sleep(1);
			memset(sql_query,'\0',sizeof(char)*MAX);// cleanning
			sprintf(sql_query,"INSERT INTO spidering(date,ip,host,data,type,port,status,page) VALUES('");		

		  	strcat(sql_query,tmp2);
			sql_query[strlen(sql_query)-1] = '\0';
			strcat(sql_query,"','");
			strcat(sql_query,url);
			strcat(sql_query,"','");
			strcat(sql_query,host);
			strcat(sql_query,"','");
			strcat(sql_query,link);
			strcat(sql_query,"','");
	 	  	strcat(sql_query,type);
			strcat(sql_query,"','");
	 	  	strcat(sql_query,port);
			strcat(sql_query,"','0','");
			strcat(sql_query,pagina);
			strcat(sql_query,"')");

			mysql_query(conn, sql_query);// ? printf("\n[SQL ERROR! Object insertion: %s\n]",sql_query) : printf("\n[Object found: %s\n]",sql_query);//sql_err_count++ : NULL;
		}				
	}

	//printf("\nParsing Finished.");

  //fprintf(stdout,"%s",tmp);
  
  //free(buff);  
  close(sockfd);
  mysql_free_result(result);
  mysql_close(conn);      
  //seconds = time(NULL)-seconds;	

  //printf("\n%d objects founds in %ld seconds. ",obj_count,seconds);
  //printf("%d Errors in SQL Queries!\n",sql_err_count);
  //printf("****************************************************************************************\n\n");
  free(tmp);
  return 0;
}

int isfile(char *url) {
 int c;
 for (c=0;c<strlen(url);c++) {
  if (url[c] == '.' && (url[c+4] == '\0' || url[c+5] == '\0')) {
   if (url[c+1] == 'm' && url[c+2] == 'p') { 
    if (url[c+3] == '3' || url[c+3] == '4') { return (1); }
    if (url[c+3] == 'e' && url[c+4] == 'g') { return (1); }
    if (url[c+3] == 'g') { return (1); }
   }
   if (url[c+1] == 'g' && url[c+2] == 'i' && url[c+3] == 'f') { return (1); }
   if (url[c+1] == 'c' ) { 
	if ( url[c+2] == 'a' && url[c+3] == 'b') { return (1); }
	if ( url[c+2] == 's' && url[c+3] == 's') { return (1); }	
   }
   if (url[c+1] == 's' && url[c+2] == 'w' && url[c+3] == 'f') { return (1); }
   if (url[c+1] == 'e' && url[c+2] == 'x' && url[c+3] == 'e') { return (1); }
   if (url[c+1] == 'r' && url[c+2] == 'a' && url[c+3] == 'r') { return (1); }
   if (url[c+1] == 'z' && url[c+2] == 'i' && url[c+3] == 'p') { return (1); }
   if (url[c+1] == 'j' && url[c+2] == 'p') {
    if (url[c+3] == 'g') { return (1); }
    if (url[c+3] == 's' && url[c+4] == 'f') { return (1); }	
    if (url[c+3] == 'e' && url[c+4] == 'g') { return (1); }	
   }
   if (url[c+1] == 'p' && url[c+2] == 'd' && url[c+3] == 'f') { return (1); }
   if (url[c+1] == 'd' && url[c+2] == 'o' && url[c+3] == 'c') { return (1); }
   if (url[c+1] == 'a' && url[c+2] == 'v' && url[c+3] == 'i') { return (1); }
  }
 }
 return (0);
}
