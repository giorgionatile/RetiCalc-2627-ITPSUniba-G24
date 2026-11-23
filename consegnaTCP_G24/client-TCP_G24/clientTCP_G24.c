#if defined (_WIN32)
	#include <winsock2.h>
#else
	#define closesocket close
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 512
// #define PROTOPORT 5193 //Numero di porta di default

void ErrorHandler(char *errorMessage){
	printf("%s\n", errorMessage);
	system("pause");
}

void ClearWinSock(){
	#if defined (_WIN32)
		WSACleanup();
	#endif
}

int main (void){
	#if defined (_WIN32)
		// Initialize Winsock
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		if (iResult != 0) {
			printf("Error at WSAStartup()\n");
			return -1;
		}
	#endif

	//LETTURA DA TASTIERA DELL'IP DELL'HOST E DEL NUMERO DI PORTA
	char IPAddress[16];
	int port;
	printf("Inserisci l'indirizzo IP: ");
	scanf("%s", &IPAddress);
	do {
		printf("Inserisci il numero di porta: ");
		scanf("%d", &port);
	}while(port < 5000 || port > 65535); //porte disponibili per il client tra 5000 e 65535

	//CREAZIONE DELLA SOCKET
	int CSocket;
	CSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(CSocket < 0){
		ErrorHandler("Creazione della socket fallita.\n");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}

	//COSTRUZIONE DELL'INDIRIZZO DEL SERVER
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1"); //IP del server
	sad.sin_port = htons(27015); //Server port

	//CONNESSIONE AL SERVER
	if(connect(CSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0){
		ErrorHandler("Connessione fallita.\n");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}

	//RICEZIONE DELLA STRINGA "Connessione avvenuta" DAL SERVER
	int bytesRcvd;
	//int totalBytesRcvd = 0;
	char buf[BUFFERSIZE];
	printf("Received: ");

	//while(totalBytesRcvd < stringLen){
	if((bytesRcvd = recv(CSocket, buf, BUFFERSIZE - 1, 0)) <= 0){
		ErrorHandler("recv() failed or connection closed prematurely");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}
	//totalBytesRcvd += bytesRcvd; //Keep tally of total bytes
	buf[bytesRcvd] = '\0'; //Add \0 so printf knows where to stop
	printf("%s", buf); //Print the echo buffer
	//}

	//LETTURA DA TASTIERA DI 3 INTERI E INVIO AL SERVER

	//RICEZIONE DELLA STRINGA DAL SERVER E VISUALIZZAZIONE

	//CHIUSURA DELLA CONNESSIONE (se la stringa corrisponde a QUIT)
	printf("\nChiusura della connessione in corso...\n");
	closesocket(CSocket);
	ClearWinSock();
	printf("\n");
	system("pause");
	return(0);
}
