#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define PORT 60000

typedef unsigned char  u8; 
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

#define MAGIC 0xb000b


void lgErr(const char* errmsg , ...){
	va_list args;
	fprintf(stderr, "[ERORR] : ");
	va_start(args, errmsg);
	vfprintf(stderr, errmsg , args);
	va_end(args);
	fprintf(stderr, "\n");
}

typedef struct
{
	enum {
		PING=0,
		PONG=1,
		GET_TIME=2,
		TIME=3,
	} Sig;
	
	u64 Magic;
	char data[1024-16];
}Packat;



int getServerSocket(const char* addrr, u16 port) //pinging and if it is valide return the socket discriptor
{
	int status, valread, client_fd;
	struct sockaddr_in serv_addr;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, addrr, &serv_addr.sin_addr)<= 0) {
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((status= connect(client_fd, (struct sockaddr*)&serv_addr,    sizeof(serv_addr)))< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	Packat pac = {.Sig=PING,.Magic=MAGIC};
	
	send(client_fd, &pac, sizeof(pac), 0);
	valread = read(client_fd, &pac,sizeof(pac)); 
	if(pac.Sig==PONG){
		return client_fd;
	}
	return -1;
}






void server(u16 port){
	time_t currentTime;
  	time(&currentTime);
	
	int status,lisntsocket;
	struct sockaddr_in addrsv;
	addrsv.sin_family=AF_INET;
	addrsv.sin_port=htons(port);
	addrsv.sin_addr.s_addr = INADDR_ANY;
	
	if ((lisntsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
	}
	if( bind(lisntsocket,(struct sockaddr*)&addrsv ,sizeof(addrsv)))
	{
		lgErr("Binding error");
		exit(EXIT_FAILURE);
	}
	Packat pack;
	while(1){
		listen(lisntsocket, 1);
		int cleintsock = accept(lisntsocket, NULL, NULL);
		
		struct sockaddr cleintaddrr;
		socklen_t cleintaddrrlen=sizeof(cleintaddrr);
		getsockname(cleintsock, &cleintaddrr, &cleintaddrrlen);
		

		printf("New conection: %d.%d.%d.%d\n",(u8)cleintaddrr.sa_data[2],(u8)cleintaddrr.sa_data[3],(u8)cleintaddrr.sa_data[4],(u8)cleintaddrr.sa_data[5]);
		for(int i = 0 ; i < 2 ; i++){	
			read(cleintsock, &pack, sizeof(pack));
			if(pack.Magic==MAGIC && pack.Sig==PING){
				pack.Sig=PONG;
				send(cleintsock, &pack, sizeof(pack),0);
			}


			if(pack.Magic==MAGIC && pack.Sig==GET_TIME){
				pack.Sig=TIME;
				gettimeofday((struct timeval*)&pack.data[0], 0);
				send(cleintsock, &pack, sizeof(pack),0);
			}
		}
		close(cleintsock);
	}
	close(lisntsocket);
}



void cleint(const char* addrr, u16 port){
	int serverfd = getServerSocket(addrr, port);
	if(serverfd==-1){
		lgErr("clould not conecet to the server %s @ port %d, server offline or app not runing.\n",addrr,port);	
		exit(EXIT_FAILURE);
	}
	Packat pack={.Sig=GET_TIME,.Magic=MAGIC};
	send(serverfd, &pack, sizeof(pack), 0);
	
	read(serverfd,&pack,sizeof(pack));
	if(pack.Sig!=TIME){
		lgErr("Comnication with the server is currpted or not working");
	}
	
	struct timeval *newtime=(struct timeval*)&pack.data[0];
	
	int settimeret=settimeofday(newtime, 0);
	if(!settimeret){
		printf("Time seted secsusfuly to %s",ctime(&newtime->tv_sec));
	}else {
		lgErr("Faild at seting the system time to %s. perhapes you are not root?",ctime(&newtime->tv_sec));
	}
	close(serverfd);
	return;
}



#define OptionsAvliable "\nOptin1: server ; so you been the server\nOptin2: cleint <serveraddr>, so you get time frome a server."




int main(int argc, char const* argv[])
{
	if(argc<2){
		lgErr("wrong argimunt struct."OptionsAvliable);
		return -1;
	}
	int opetinlen = strlen(argv[1]);
	//server -> 6
	//cleint -> 6
	if(opetinlen!= 6){
		lgErr("No option \"%s\" , option avliabel:"OptionsAvliable,argv[1]);	
		return -1;
	}
	if(!memcmp(argv[1], "server", 6)){
		server(PORT);
	}
	else if(!memcmp(argv[1], "cleint", 6)){
		const char* server__ = NULL; 
		if(argc<3){
			lgErr("You Have to select the server in the nexr argimunt");
			server__ = (char*)malloc(128);
			memset(server__, 0, 128);
			printf("\ninput the server address: ");
			fgets(server__, 15/*AAA.AAA.AAA.AAA*/, stdin);
		}else {
			server__=argv[2];
		}
		cleint(server__, PORT);

	}else {
		lgErr("\"%s\" is not a valide option"OptionsAvliable,argv[1]);
		return -1;

	}

	return 0;
}
