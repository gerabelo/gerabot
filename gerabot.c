#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"
#include "mysql/mysql.h"

#define MAXSIZE 800
#define MAX 512
#define N_THR_MAX 3
#define TEMPO 1

char db_server[MAX],db_user[MAX],db_passwd[MAX],db_db[MAX];

char sfood[MAX];

char url_id[9]; 

void thr_spider(void *v) {
	int thr_id;
	thr_id = *(int *) v;
	system(sfood);
	pthread_exit((void*) v);
	//return v;	
}

char* getdata();

int main(int argc, char **argv) {

  void *res = NULL;

  char *data,food[MAX]="",command[MAXSIZE]="",page[MAX]="",host[MAX]="",port[MAX]="80",tm[30]="";
  int i,c_thr;
  pthread_t spider[N_THR_MAX];

  //carregar arquivos de configurações para conexao com a base de dados
  FILE *config_File,*log;
  config_File = fopen("gerabot.conf", "r");
  if (config_File == NULL) printf("config file does not exist, please check this out!\n");
  fscanf (config_File, "%99[^/]/%99[^/]/%99[^/]/%99[^\n]",db_server,db_user,db_passwd,db_db);
  fclose(config_File);

  //outro detalhe estético...

  data = getdata();
  log = fopen("gerabot.log", "a+b");
  if (log == NULL) printf("Erro ao criar arquivo de log!\n");
  
  c_thr = 0;
  
  while (strlen(data)) {
	/*limpar memoria

		memset(food,'\0',sizeof(char)*MAX);

	*/
	//usleep(500000);
	//thr_count == 5 ? sleep(3) : sleep(1);
	
	sprintf(food,"%s",data);
	sscanf(food, "http://%99[^\n]",food);

	system("clear");

	printf("\n[FOOD: %s]",food);

	for (i=0;i<strlen(food);i++) {

		if (food[i]==':') { 
			sscanf(food, "%99[^:]:%99[^/]/%99[^\n]", host, port, page);
			break;
		}
	  	if (food[i]=='/') { 
			sscanf(food, "%99[^/]/%99[^\n]", host, page);
			sprintf(port,"80");
			break;
		}
		if (i==(strlen(food)-1)) { 
			sprintf(host,"%s",food);
			sprintf(port,"80");
			sprintf(page,"");
		}
	}

	//validar página
	
	//strlen(page) ? NULL : sprintf(page,"0");
	strlen(port) ? NULL : sprintf(port,"%s","80");
	
	strlen(host) ? NULL : sprintf(page,food);
	
	//printf("\n[HOST: %s]\n[PORT: %s]\n[PAGE: %s] ",host,port,page);

	sprintf(command,"./spider ");
	strcat(command,host);
	strcat(command," ");
	strcat(command,port);
	strcat(command," \"");
	strcat(command,page);
	strcat(command,"\" ");
	strcat(command,url_id);
	
	printf("\n[COMMAND: %s]\n",command);
	fprintf(log,"%s\n",command);
	//system(command); //alimentar a aranha	
	
	sprintf(sfood,"%s",command);//mascara só por garantia 
	//thr_count++;
	
	if (c_thr < (N_THR_MAX-1)) {
	
		sleep(TEMPO);
	
		if (pthread_create(&spider[c_thr],NULL,thr_spider, (void*) &c_thr)) {

			if (pthread_cancel(spider[c_thr])) {

				pthread_join(spider[c_thr], &res);
				//c_thr--;
				//printf("\n[THREAD FAIL]\n");
				//sleep(10);
				//if (res == 0) {}
			}
		
		} else {

		   c_thr++;

		}
		
	} else {
   	sleep(TEMPO);
		for (c_thr=0;c_thr < (N_THR_MAX-1); c_thr++) {
			//printf("\n[N_THR: %d] pthread_join",c_thr);
			pthread_join(spider[c_thr], &res);			

			c_thr--;

		}		
		
	}
	
	data = getdata(); //seleciona mais dados e fica no while
	
  }
  
  fclose(log);
  
  return 0; 
}

char* getdata(){
	//sleep(1);
	char* link[MAX];
        char sql_query[MAX]="SELECT data, id FROM spidering WHERE status=0 ORDER BY id LIMIT 1";
	char caracteres_validos[73]="%&?=;~abcdefghijklmnopqrstuvwxyz-0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ./:";//& is a bad kid
	char data[MAX],tmp[MAX];//id[9]=""; (86)
	int c1,c2,f1,sql_err_count=0; //contadores e flags

	// declaração de variáveis para a base de dados
	MYSQL *mysql;
	MYSQL_RES *results;
	MYSQL_ROW records;


	//inicialização e conexão (base)	
	mysql = mysql_init(NULL);
	mysql_real_connect(mysql, db_server, db_user, db_passwd, db_db, 0, NULL, 0);


	//precaução contra as surpresinhas das multiplas chamadas
	memset(data,'\0',sizeof(char)*MAX);
	memset(tmp,'\0',sizeof(char)*MAX);

	//primeira consulta. obtem um endereço junto à base
	mysql_query(mysql, sql_query);
        results = mysql_use_result(mysql);

	//o campo data contem o endereço.
	while ((records = mysql_fetch_row(results)) != NULL) {
		sprintf(data,"%s", records[0]);
		sprintf(url_id,"%s", records[1]);
		//url_id=atoi(records[1]);
		//printf("\n\nrecord[1]= %s  url_id= %d\n\n",records[1],url_id);
	}

	//modifica o status da url na base. 
	sprintf(sql_query,"UPDATE spidering SET status=6 WHERE id=");
	strcat(sql_query,url_id);
	mysql_query(mysql, sql_query) ? sql_err_count++ : NULL;
	mysql_free_result(results);
	mysql_close(mysql);

	//validar url. durante a coleta notei que alguns links possuíam caracteres inválidos junto ao corpo das paginas.
	for (c1=0;c1<=strlen(data);c1++) {
		f1=0;
		for (c2=0;c2<73;c2++) {
			if (data[c1]==caracteres_validos[c2]) { f1=1; }
		}
		if (f1<1) { 
			for (;c1<=strlen(data);c1++) {
				data[c1]='\0';
			}
			break; 
		}
	}

	if (!sql_err_count && strlen(url_id)) {sprintf(link,"%s",data);} else {sprintf(link,"");}
//	if (!sql_err_count && strlen(id)) {sprintf(link,"%s",tmp);} else {sprintf(link,"");}
	
	//printf("",tmp);

	return (link);
}
