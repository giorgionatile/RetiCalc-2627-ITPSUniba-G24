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

#define PROTOPORT 27015 //numero di porta di default
#define QLEN 6 //dimensione della coda delle richieste
#define BUFFERSIZE 512 //dimensione massima del buffer per le stringhe

//Funzione per stampare i messaggi di errore
void ErrorHandler(char *errorMessage){
	printf("%s", errorMessage);
}

//Funzione per liberare le risorse della libreria winsock su windows
void ClearWinSock(){
	#if defined (_WIN32)
		WSACleanup();
	#endif
}

int main(int argc, char *argv[]) {
	char IPAddress[16] = "127.0.0.1"; //Indirizzo IP

	//Scelta del numero di porta (tra 5000 e 65535) se no porta di dafault (27015)
	int port;
	do {
		printf("Inserisci il numero di porta: ");
		scanf("%d", &port);
	}while(port != 0 && (port < 5000 || port > 65535));

	printf("Indirizzo IP: %s\n", IPAddress);
	if (port == 0) {
		port = PROTOPORT;
		printf("Numero di porta (di default): %d\n", port);
	}
	else
		printf("Numero di porta: %d\n", port);




	//inizializzazione della libreria winsock (solo windows)
	#if defined (_WIN32)
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

		if (iResult != 0) {
			printf("Errore in WSAStartup()\n");
			return -1;
		}
	#endif

	//CREAZIONE DELLA SOCKET
	//creazione di una socket tcp (sock_stream) su protocollo ipv4 (pf_inet)
	int SSocket;
	SSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SSocket < 0) {
		ErrorHandler("Creazione della socket fallita.\n");
		ClearWinSock();
		return -1;
	}

	//ASSEGNAZIONE DI UN INDIRIZZO ALLA SOCKET
	//azzeramento e configurazione della struttura sockaddr_in con indirizzo IP e porta
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(IPAddress);
	sad.sin_port = htons(port);
	if(bind(SSocket, (struct sockaddr*) &sad, sizeof(sad)) < 0){
		ErrorHandler("bind() ha fallito.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//IMPOSTAZIONE DELLA SOCKET ALL'ASCOLTO
	if(listen(SSocket, QLEN) < 0){
		ErrorHandler("listen() ha fallito.\n");
		closesocket(SSocket);
		ClearWinSock();
		return -1;
	}

	//ACCETTAZIONE DI UNA NUOVA CONNESSIONE
	//struttura per memorizzare l'indirizzo del client e descrittore della socket dedicata al singolo client
	struct sockaddr_in cad;
	int clientSocket;
	int clientLen;
	printf("Aspettando la connessione di un client...");

	//ciclo infinito del server per servire piu client in sequenza
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

		//RICEZIONE DEI 3 NUMERI E CALCOLO DELL'OPERAZIONE res = n1 + n2 - n3
		//ciclo interno per gestire piu richieste consecutive dello stesso client finche non esce
		while (1) {
			//array di 3 interi a 32 bit per ricevere i 12 byte inviati dal client
			int32_t dati[3];
			int totaleRicevuto = 0;
			char *ptr = (char*)dati;

			//ciclo di lettura che garantisce la ricezione completa di tutti i 12 byte dal flusso tcp
			while (totaleRicevuto < (int)sizeof(dati)) {
				int bytesRcvd = recv(clientSocket, ptr + totaleRicevuto, (int)sizeof(dati) - totaleRicevuto, 0);
				if (bytesRcvd <= 0) {
					break;
				}
				totaleRicevuto += bytesRcvd;
			}

			if (totaleRicevuto < (int)sizeof(dati)) {
				break;
			}

			//conversione dei tre interi dal formato di rete (network byte order) al formato dell'host
			int num1 = ntohl(dati[0]);
			int num2 = ntohl(dati[1]);
			int num3 = ntohl(dati[2]);

			int res = num1 + num2 - num3;
			printf("Operazione: %d + %d - %d = %d\n", num1, num2, num3, res);

			//CONTROLLO SUL RISULTATO
			if (res == 100) {
				//INVIO DELLA STRINGA "QUIT" AL CLIENT
				int stringLen1 = strlen("QUIT");
				if (send(clientSocket, "QUIT", stringLen1, 0) != stringLen1) {
					ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
				}

				break;
			}
			else {
				//conversione del risultato intero in stringa testuale prima dell'invio al client
				char str_res[BUFFERSIZE];
				snprintf(str_res, sizeof(str_res), "%d", res);

				int stringLen2 = (int)strlen(str_res);
				if (send(clientSocket, str_res, stringLen2, 0) != stringLen2) {
					ErrorHandler("send() ha inviato un numero differente di bytes rispetto al previsto");
				}
			}
		}

		//CHIUSURA DELLA CONNESSIONE CON IL CLIENT
		closesocket(clientSocket);
	}
}