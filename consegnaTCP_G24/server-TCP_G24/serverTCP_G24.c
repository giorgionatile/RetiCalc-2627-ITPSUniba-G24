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
#include <ctype.h>

#define PROTOPORT 27015 //numero di porta di default
#define QLEN 6 //dimensione della coda delle richieste
#define BUFFERSIZE 512

void ErrorHandler(char *errorMessage){
	printf("%s", errorMessage);
}

void ClearWinSock(){
	#if defined (_WIN32)
		WSACleanup();
	#endif
}

int main(int argc, char *argv[]) {
	int port;
	if(argc > 1){
		port = atoi(argv[1]);
	}
	else{
		port = PROTOPORT;
	}

	if(port < 0){
		printf("numero di porta non valido %s \n", argv[1]);
		return 0;
	}

	#if defined (_WIN32)
		// Initialize Winsock
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		if (iResult != 0) {
			printf("Errore in WSAStartup()\n");
			return -1;
		}
	#endif

	// CREAZIONE DELLA SOCKET
	int SSocket;
	SSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SSocket < 0) {
		ErrorHandler("Creazione della socket fallita.\n");
		ClearWinSock();
		return -1;
	}

	//ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port);
	if(bind(SSocket, (struct sockaddr*) &sad, sizeof(sad)) < 0){
		ErrorHandler("bind() ha fallito.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//SETTAGGIO DELLA SOCKET ALL'ASCOLTO
	if(listen(SSocket, QLEN) < 0){
		ErrorHandler("listen() ha fallito.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//ACCETTARE UNA NUOVA CONNESSIONE
	struct sockaddr_in cad;
	int clientSocket;
	int clientLen;
	printf("Aspettando la connessione di un client...");
	while(1){
		clientLen = sizeof(cad);
		if((clientSocket = accept(SSocket, (struct sockaddr *) &cad, &clientLen)) < 0){
			ErrorHandler("accept() ha fallito.\n");

			// CHIUSURA DELLA CONNESSIONE
			closesocket(SSocket);
			ClearWinSock();
			return 0;
		}

		//VISUALIZZAZIONE DEL MESSAGGIO DI CONNESSIONE CON IL CLIENT
		printf("\nConnesso con il client il cui nome IP:  %s\n", inet_ntoa(cad.sin_addr));

		//INVIO DEL MESSAGGIO DI "Connessione avvenuta"
		char* inputString = "Connessione avvenuta";
		int stringLen = strlen(inputString);
		if (send(clientSocket, inputString, stringLen, 0) != stringLen) {
			ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
		}

		//RICEVE I 3 NUMERI E FA LA SOMMA
		int num1;
		char str_num1[BUFFERSIZE];
		int bytesRcvd = recv(clientSocket, str_num1, BUFFERSIZE - 1, 0);
		if (bytesRcvd > 0) {
			str_num1[bytesRcvd] = '\0';
			num1 = atoi(str_num1);
		}

		int num2;
		char str_num2[BUFFERSIZE];
		bytesRcvd = recv(clientSocket, str_num2, BUFFERSIZE - 1, 0);
		if (bytesRcvd > 0) {
			str_num2[bytesRcvd] = '\0';
			num2 = atoi(str_num2);
		}

		int num3;
		char str_num3[BUFFERSIZE];
		bytesRcvd = recv(clientSocket, str_num3, BUFFERSIZE - 1, 0);
		if (bytesRcvd > 0) {
			str_num3[bytesRcvd] = '\0';
			num3 = atoi(str_num3);
		}

		int res = num1 + num2 + num3;
		printf("La somma di %d + %d + %d = %d\n", num1, num2, num3, res);

		//CONTROLLO SUL RISULTATO
		if (res == 100) {
			//INVIO DELLA STRINGA "QUIT" AL CLIENT
			int stringLen1 = strlen("QUIT");
			if (send(clientSocket, "QUIT", stringLen1, 0) != stringLen1) {
				ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
			}
		}
		else {
			int stringLen2 = strlen((char*)&res);
			if (send(clientSocket, (char*)&res, stringLen2, 0) != stringLen2) {
				ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
			}
		}

		//CHIUSURA DELLA CONNESSIONE CON IL CLIENT
		closesocket(clientSocket);
	}
}