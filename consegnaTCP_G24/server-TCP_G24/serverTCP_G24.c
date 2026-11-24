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

// da rivedere
#define PROTOPORT 27015 //default protocol port number
#define QLEN 6 //size of request queue
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
		port = atoi(argv[1]); //if argument specified convert argument to binary
	}
	else{
		port = PROTOPORT; //use default port number
	}

	if(port < 0){
		printf("bad port number %s \n", argv[1]);
		return 0;
	}

	#if defined (_WIN32)
		// Initialize Winsock
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		if (iResult != 0) {
			printf("Error at WSAStartup()\n");
			return -1;
		}
	#endif

	// CREAZIONE DELLA SOCKET
	int SSocket;
	SSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SSocket < 0) {
		ErrorHandler("socket creation failed.\n");
		ClearWinSock();
		return -1;
	}

	//ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); //ensueres that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port); /*converts values beetween the host and network byte order.
	Specifically, htons() converts 16-bit quantities from host byte order to network byte order*/
	if(bind(SSocket, (struct sockaddr*) &sad, sizeof(sad)) < 0){
		ErrorHandler("bind() failed.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//SETTAGGIO DELLA SOCKET ALL'ASCOLTO
	if(listen(SSocket, QLEN) < 0){
		ErrorHandler("listen() failed.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//ACCETTARE UNA NUOVA CONNESSIONE
	struct sockaddr_in cad;
	int clientSocket;
	int clientLen;
	printf("Waiting for a client to connect...");
	while(1){
		clientLen = sizeof(cad);
		if((clientSocket = accept(SSocket, (struct sockaddr *) &cad, &clientLen)) < 0){
			ErrorHandler("accept() failed.\n");

			// CHIUSURA DELLA CONNESSIONE
			closesocket(SSocket);
			ClearWinSock();
			return 0;
		}
		printf("Handling client %s\n", inet_ntoa(cad.sin_addr));

		//INVIO DEL MESSAGGIO DI "Connessione avvenuta"
		char* inputString = "connessione avvenuta";
		int stringLen = strlen(inputString);
		if (send(clientSocket, inputString, stringLen, 0) != stringLen) {
			ErrorHandler("send() sent a different number of bytes than expected");
		}

		//RICEVE I 3 NUMERI E FA LA SOMMA
		char str1[BUFFERSIZE];
		int bytesRcvd = recv(clientSocket, str1, BUFFERSIZE - 1, 0);
		if (bytesRcvd > 0) {
		    str1[bytesRcvd] = '\0';
		    printf("Prima stringa: %s\n", str1);
		}


		//INVIO DELLA STRINGA "QUIT" AL CLIENT
		int stringLen1 = strlen("QUIT");
		if (send(clientSocket, "QUIT", stringLen1, 0) != stringLen1) {
			ErrorHandler("send() sent a different number of bytes than expected");
		}

		//CHIUSURA DELLA CONNESSIONE CON IL CLIENT
		closesocket(clientSocket);
	}


}