//librerie di rete per garantire la portabilità
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
#include <stdint.h> //per usare interi a dimensione fissa (int32_t)

#define BUFFERSIZE 512 //dimensione massima del buffer per le stringhe

//Funzione per stampare i messaggi di errore
void ErrorHandler(char *errorMessage){
	printf("%s\n", errorMessage);
	system("pause");
}

//Funzione per liberare le risorse della libreria winsock su windows
void ClearWinSock(){
	#if defined (_WIN32)
		WSACleanup();
	#endif
}

int main (void){
	//inizializzazione della libreria winsock (solo windows)
	#if defined (_WIN32)
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		if (iResult != 0) {
			printf("Errore in WSAStartup()\n");
			return -1;
		}
	#endif

	//LETTURA DA TASTIERA DELL'IP DELL'HOST E DEL NUMERO DI PORTA
	//acquisizione dell'indirizzo ip del server come stringa
	char IPAddress[16];
	int port;
	printf("Inserisci l'indirizzo IP: ");
	scanf("%s", &IPAddress);

	//acquisizione e validazione della porta nell'intervallo consentito
	do {
		printf("Inserisci il numero di porta: ");
		scanf("%d", &port);
	}while(port < 5000 || port > 65535); //porte disponibili per il client tra 5000 e 65535

	//CREAZIONE DELLA SOCKET
	//creazione di una socket tcp (sock_stream) su protocollo ipv4 (pf_inet)
	int CSocket;
	CSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(CSocket < 0){
		ErrorHandler("Creazione della socket fallita.\n");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}

	//COSTRUZIONE DELL'INDIRIZZO DEL SERVER
	//azzeramento e configurazione della struttura sockaddr_in del server
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(IPAddress); //IP del server
	sad.sin_port = htons(port); //Porta del server

	//CONNESSIONE AL SERVER
	//invio della richiesta di connessione tcp verso il server
	if(connect(CSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0){
		ErrorHandler("Connessione fallita.\n");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}

	//RICEZIONE DELLA STRINGA "Connessione avvenuta" DAL SERVER
	int bytesRcvd;
	char buf[BUFFERSIZE];

	if((bytesRcvd = recv(CSocket, buf, BUFFERSIZE - 1, 0)) <= 0){
		ErrorHandler("recv() ha fallito o connessione chiusa permanentemente");
		closesocket(CSocket);
		ClearWinSock();
		return -1;
	}

	buf[bytesRcvd] = '\0';
	printf("%s\n", buf);

	int repeat = 0;
	do {
		//LETTURA DA TASTIERA DI 3 INTERI E INVIO AL SERVER
		int num1, num2, num3;
		printf("Inserisci il 1 numero: ");
		scanf("%d", &num1);
		printf("Inserisci il 2 numero: ");
		scanf("%d", &num2);
		printf("Inserisci il 3 numero: ");
		scanf("%d", &num3);

		//conversione dei tre interi a 32 bit in formato di rete (network byte order)
		int32_t dati[3];
		dati[0] = htonl(num1);
		dati[1] = htonl(num2);
		dati[2] = htonl(num3);

		//invio unico dei 12 byte (3 interi) al server tramite la socket
		if (send(CSocket, (char*)dati, sizeof(dati), 0) != (int)sizeof(dati)) {
			ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
			closesocket(CSocket);
			ClearWinSock();
			return -1;
		}

		//RICEZIONE DELLA STRINGA DAL SERVER E VISUALIZZAZIONE
		if((bytesRcvd = recv(CSocket, buf, BUFFERSIZE - 1, 0)) <= 0){
			ErrorHandler("recv() ha fallito o connessione chiusa permanentemente");
			closesocket(CSocket);
			ClearWinSock();
			return -1;
		}
		buf[bytesRcvd] = '\0';
		printf("%s\n", buf);

		//CONTROLLO DELLA STRINGA
		//verifica se il server ha inviato il comando di terminazione quit
		if (strcmp(buf, "QUIT")==0) {
			repeat = 0;
		}
		else {
			repeat = 1;
		}
	}while (repeat==1);

	//CHIUSURA DELLA CONNESSIONE
	//chiusura della socket del client e rilascio delle risorse di rete
	printf("\nChiusura della connessione in corso...\n");
	closesocket(CSocket);
	ClearWinSock();
	printf("\n");
	system("pause");
	return(0);
}
