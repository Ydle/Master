#ifndef PROTOCOLRF_H
#define PROTOCOLRF_H

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <list>

using namespace std;

#define TYPE_ETAT 		1 // Node send data
#define TYPE_CMD  		2 // ON/OFF sortie etc...
#define TYPE_ACK  		3 // Acquit last command
#define TYPE_ETAT_ACK 	4 // Node send data and whant ACK


#define DATA_ETAT     1 // On - OFF (4bits)
#define DATA_DEGREEC  2 // Degrée Celsius ** -204.7 à 204.7 (12bits)
#define DATA_DEGREEF  3 // Degrée Fahrenheit ** -204.7 à 204.7 (12bits)
#define DATA_PERCENT  4 // Pourcentage ** -100% à 100% (12bits)
#define DATA_DISTANCE 5 // Distance en Cm ** 0 à 4095 (12 bits)
#define DATA_WATT     6 // Watt ** 0 à 1048575 (20bits)
#define DATA_HUMIDITY 7 // Pourcentage humidité ** 0 à 100% (12bits)
#define DATA_PRESSION 8 // Pression en hpa 0 à 4095 (12bits)


#define CMD_LINK  0 //Link a node to the master 
#define CMD_ON    1 //Send a ON command to node data = N° output
#define CMD_OFF   2 //Send a OFF command to node data = N° output
#define CMD_RESET 3 //Ask a node to reset is configuration 

extern pthread_mutex_t g_mutexSynchro;

class protocolRF
{

public:

	// Définie un type de structure Signal_t
	struct Frame_t
	{
		uint8_t sender;
		uint8_t receptor;
		uint8_t type;
		uint8_t taille;	// data len + crc in BYTES
		uint8_t data[30];
		uint8_t crc;
	};

	// Définie un type de structure Signal_t
	struct ACKCmd_t
	{
		Frame_t Frame;
		int Time;
		int iCount;
	};

	// Ecoute le récepteur pour l'arrivée d'un signal
	static void* listenSignal(void* pParam);
	void		listenSignal();

	// Check if CMD command was not received there ACK. retry if needed
	void checkACK();


	// Le constructeur qui lance une instance avec les numéros des pins de l'émetteur, du récepteur et du boutton
	// Par défaut, le récepteur est en 12, l'émetteur en 10 et le boutton en 2
	protocolRF(int rx, int tx);

	protocolRF();

	~protocolRF();

	// Retourne l'état de la Node
	bool initialized();

	// Pour activer le mode débug
	void debugMode();

	int getReceptor();

	int getSender();

	int getType();

	int getTaille();

	uint8_t* getData();

	int isSignal();

	bool isDone();

	void setDone(bool bvalue);

	bool bTransmitinprogress;

	// Boucle à verrouillage de phase qui se synchronise au signal
	void pll();

	// Crée une trame avec les infos données en paramètre
	void dataToFrame(unsigned long recepteur, unsigned long emetteur, unsigned long type);

	// extract any type of data from receivedsignal
	int extractData(int index,int &itype,int &ivalue,uint8_t* pBuffer=NULL,int len=0);

	// add TYPE_ETAT data
	void addData(int type,int data);

	// add TYPE_CMD data
	void addCmd(int type,int data);

	// Envoie des verrous et des bits formant une trame
	void transmit(bool bRetransmit = false);
	uint8_t computeCrc(Frame_t* frame);

protected:

	// Permet l'initialisation du node
	void initialisation();

	// Double un bit en code Manchester
	void sendPair(bool b);

	// Crée une pulsation haute de 310µs suivie d'une pulsation basse représentative du bit à envoyer
	void sendBit(bool b);


	// Compare le signal reçu au signal de référence
	bool checkSignal();

	// Ecriture en mémoire EEProm du signal de référence
	void checkEEProm();

	// get current time in usec
	int getTime();
	



	// Calcule 2^"power"
	unsigned long power2(int power);

	// Conversion décimal vers tableau de binaires
	void itob(unsigned long integer, int start, int length);


	// Affiche le contenue des trames reçues
	void printFrame(Frame_t & trame);


private:
	// Rx PIN
	int m_pinRx;

	// Tx PIN
	int m_pinTx;

	uint8_t  crc8(uint8_t* buf, int len);


	// L'information sur le type de transmission (requete, ACK,...)
	int transmissionType ;

	// Indique si le node est initialisé
	bool initializedState;


	// Le tableau contenant la trame 272 BITS MAX
	bool m_FrameBits[272]; 

	// Le tableau contenant l'octet de start
	bool start_bit2[8]; 

	// On déclare les structures 
	Frame_t m_receivedframe;  // received frame

	Frame_t m_sendframe;	 // send frame	

	// Pour activer le mode débug
	bool debugActivated ;

	// Disponibilité d'un sample
	uint8_t m_sample_value ;

	// Nombre de samples sur la période en cours
	uint8_t  sample_count ;

	// La valeur du dernier sample reçu
	uint8_t last_sample_value;

	// La rampe PLL, varie entre 0 et 79 sur les 8 samples de chaque période de bit
	// Quand la PLL est synchronisée, la transition de bit arrive quand la rampe vaut 0
	uint8_t pll_ramp;

	// La somme des valeurs des samples. si inférieur à 2 "1" samples dans le cycle de PLL
	// le bit est déclaré comme 0, sinon à 1
	uint8_t sample_sum;

	// Les 16 derniers bits reçus, pour repérer l'octet de start
	unsigned short rx_bits ;

	// Temps du premier sample de chaque bit
	unsigned long t_start ;

	// Flag pour indiquer la bonne réception du message de start
	uint8_t rx_active ;

	// Le débit de transfert en bits/secondes
	long speed ;

	// La période d'un bit en microseconds
	long t_per ;

	// La fréquence de prise de samples
	long f_bit ;

	// La valeur du dernier bit récupéré
	int bit_value ;

	// Le nombre de bits récupérés
	int bit_count;

	// Id sender reçue
	unsigned long sender;

	// Id receptor reçue
	unsigned long receptor;

	// Info type reçue
	unsigned long type;

	// Info parité reçue
	bool parite ;

	// Info taille reçue
	unsigned long taille;

	// Data reçues
	 uint8_t m_data[31]; // data + crc

	// Nombre d'octets reçus
	int rx_bytes_count ;

	// Disponibilité de la taille de trame
	int length_ok;

	// Si le message est complet
	int m_rx_done;
	
	std::list<protocolRF::ACKCmd_t> mListACK;
};

#endif // PROTOCOLRF_H
