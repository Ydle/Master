#include "protocolRF.h"
#include "master.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <wiringPi.h>

#include "logging.h"

using namespace std;

#define TIME_OUT_ACK  5000000 //microsecondl


extern void scheduler_realtime();
extern void scheduler_standard ();

static uint8_t _atm_crc8_table[256] = {
		0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
		0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
		0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
		0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
		0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
		0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
		0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
		0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
		0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
		0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
		0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
		0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
		0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
		0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
		0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
		0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
		0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
		0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
		0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
		0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
		0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
		0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
		0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
		0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
		0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
		0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
		0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
		0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
		0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
		0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
		0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
		0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

uint8_t  protocolRF::crc8(uint8_t* buf, int len)
{
	// The inital and final constants as used in the ATM HEC.
	const uint8_t initial = 0x00;
	const uint8_t final = 0x55;
	uint8_t crc = initial;
	while (len) {
		crc = _atm_crc8_table[(*buf ^ crc)];
		buf++;
		len--;
	}
	return crc ^ final;
}

uint8_t protocolRF::computeCrc(Frame_t* frame){
	uint8_t *buf, crc;
	int a,j;

	buf = (uint8_t*)malloc(frame->taille+3);
	memset(buf, 0x0, frame->taille+3);

	buf[0] = frame->sender;
	buf[1] = frame->receptor;
	buf[2] = frame->type << 5;
	buf[2] |= frame->taille;

	for(a=3, j=0 ;j < frame->taille - 1;a++, j++){
		buf[a] = frame->data[j];
	}
	// message size = Sender (1byte) + receptor(1 byte) + type/size (1byte) + data/crc (size bytes)
	// without crc : total size : 3 + taille - 1 = taille+2
	crc = crc8(buf,frame->taille+2);
	free(buf);
	return crc;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  RX and TX PIN

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::protocolRF(int rx, int tx)
{

	m_pinRx=rx;
	m_pinTx=tx;

	pinMode(m_pinTx, OUTPUT);
	pinMode(m_pinRx, INPUT);
	initialisation();
	YDLE_DEBUG << "Init protocolRF TX: " << m_pinTx;
	YDLE_DEBUG << "Init protocolRF RX: " << m_pinRx;
}


//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::protocolRF()
{
	m_pinRx=0;
	m_pinTx=6;

	pinMode(m_pinTx, OUTPUT);
	pinMode(m_pinRx, INPUT);
	initialisation();

	YDLE_DEBUG << "Init protocolRF TX: " << m_pinTx;
	YDLE_DEBUG << "Init protocolRF RX: " << m_pinRx;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: Destructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::~protocolRF()
{
}

//
// ----------------------------------------------------------------------------
/**
	   Function: initialisation
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
void protocolRF::initialisation()
{

	transmissionType = 0;
	initializedState = false;



	memset(start_bit2,0,sizeof(start_bit2));
	start_bit2[1]=true;
	start_bit2[6]=true;


	debugActivated = false;

	m_sample_value = 0;

	sample_count = 1;

	last_sample_value = 0;

	pll_ramp = 0;

	sample_sum = 0;

	rx_bits = 0;

	t_start = 0;

	rx_active = 0;

	speed = 1000;

	t_per = 1000000/speed;

	f_bit = t_per/8;

	bit_value = 0;

	bit_count = 0;

	sender = 0;

	receptor = 0;

	type = 0;

	parite = false;

	taille = 0;

	memset(m_data,0,sizeof(m_data));

	rx_bytes_count = 0;

	length_ok = 0;

	m_rx_done = 0;
}


// ----------------------------------------------------------------------------
/**
	   Function: sendPair
	   Inputs:  bit value 

	   Outputs: Send  bit with Manchester codage

 */
// ----------------------------------------------------------------------------
void protocolRF::sendPair(bool b) 
{
	sendBit(b);
	sendBit(!b);
}


// ----------------------------------------------------------------------------
/**
	   Function: sendBit
	   Inputs:  bit value 

	   Outputs: 

// 			Send a pulse on PIN
//			1 = t_per µs HIGH
//			0 = t_per µs LOW
 */
// ----------------------------------------------------------------------------
void protocolRF::sendBit(bool b)
{

	if (b) {                       // si "1"
		digitalWrite(m_pinTx, HIGH);   // Pulsation à l'état haut
		delayMicroseconds(t_per);      // t_per
	}
	else {                         // si "0"
		digitalWrite(m_pinTx, LOW);    // Pulsation à l'état bas
		delayMicroseconds(t_per);      // t_per
	}
}


// ----------------------------------------------------------------------------
/**
	   Function: transmit
	   Inputs:  

	   Outputs: 

// 			Transmit Message
 */
// ----------------------------------------------------------------------------
void protocolRF::transmit(bool bRetransmit)
{
	int j = 0;
	int a = 0;
	int i = 0;
	uint8_t crc;
	// calcul crc

	m_sendframe.taille++; // add crc BYTE
	crc = computeCrc(&m_sendframe);
	YDLE_DEBUG << "Send ACK";
	m_sendframe.crc = crc;

	itob(m_sendframe.receptor,0,8);
	itob(m_sendframe.sender,8,8);
	itob(m_sendframe.type,16,3);
	itob(m_sendframe.taille,19,5);
	for(a=0;a<m_sendframe.taille-1;a++)
	{
		itob(m_sendframe.data[a],24+(8*a),8);
	}

	itob(m_sendframe.crc,24+(8*a),8);

	// If CMD then wait	 for ACK ONLY IF it's not already a re-retransmit
	if(m_sendframe.type == TYPE_CMD && !bRetransmit) {
		ACKCmd_t newack;

		memcpy(&newack.Frame,&m_sendframe,sizeof(Frame_t));
		newack.iCount=0;
		newack.Time = getTime();
		mListACK.push_back(newack);
	}

	//Lock reception when we end something
	pthread_mutex_lock(&g_mutexSynchro);

//	scheduler_realtime();


	// Sequence AGC
	for (int x=0; x < 32; x++)
	{
		sendBit(true);
		sendBit(false);
	}

	for (j=0; j<8; j++)
	{
		sendPair(start_bit2[j]);
	}

	// Send Data
	for(i=0; i<(m_sendframe.taille+3)*8;i++) // data+entete+crc
	{
		sendPair(m_FrameBits[i]);
	}

	digitalWrite(m_pinTx, LOW);

//	scheduler_standard ();

//SF		if(debugActivated) YDLE_DEBUG << ("end sending");

	//unLock reception when we finished
	pthread_mutex_unlock(&g_mutexSynchro);
}

// ----------------------------------------------------------------------------
/**
	   Function: extractData
	   Inputs:  int index: index de la value recherche (0..29)
				int itype: en retour type de la value
				int ivalue: en retour, value
				pBuffer : bufer to search in , if NULL then use m_receivedframe
	   Outputs: 1 value trouve,0 non trouve,-1 no data

 */
// ----------------------------------------------------------------------------
int protocolRF::extractData(int index,int &itype,int &ivalue,uint8_t* pBuffer /*=NULL*/,int ilen /*=0*/)
{
	uint8_t* ptr;
	bool bifValueisNegativ=false;
	int iCurrentValueIndex=0;
	bool bEndOfData=false;
	int  iLenOfBuffer = 0;
	int  iModifType=0;
	int  iNbByteRest=0;

	if(pBuffer==NULL)
	{
		ptr=m_receivedframe.data;
		iLenOfBuffer=m_receivedframe.taille;
	}	
	else
	{
		iLenOfBuffer=ilen;
		ptr=pBuffer;
	}	

	if(iLenOfBuffer <2) // Min 1 byte of data with the 1 bytes CRC always present, else there is no data
		return -1;

	while (!bEndOfData)
	{
		itype=(uint8_t)*ptr>>4;
		bifValueisNegativ=false;

		// This is a very ugly code :-( Must do something better
		if(m_receivedframe.type==TYPE_CMD)
		{
			// Cmd type if always 12 bits signed value
			iModifType=DATA_DEGREEC;
		}
		else if(m_receivedframe.type==TYPE_ETAT)
		{
			iModifType=itype;
		}
		else
		{
			iModifType=itype;
		}

		switch(iModifType)
		{
		// 4 bits no signed
		case DATA_ETAT :
			ivalue=*ptr&0x0F;
			ptr++;
			iNbByteRest--;
			break;	

			// 12 bits signed
		case DATA_DEGREEC:
		case DATA_DEGREEF :
		case DATA_PERCENT :
		case DATA_HUMIDITY:
			if(*ptr&0x8)
				bifValueisNegativ=true;
			ivalue=(*ptr&0x07)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			if(bifValueisNegativ)
				ivalue=ivalue *(-1);
			iNbByteRest-=2;
			break;	

			// 12 bits no signed
		case DATA_DISTANCE:
		case DATA_PRESSION:
			ivalue=(*ptr&0x0F)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=2;
			break;	

			// 20 bits no signed
		case DATA_WATT  :
			ivalue=(*ptr&0x0F)<<16;
			ptr++;
			ivalue+=(*ptr)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=3;
			break;	
		}

		if (index==iCurrentValueIndex)
			return 1;

		iCurrentValueIndex++;

		if(iNbByteRest<1)
			bEndOfData =true;;
	}

	return 0;	
}




// ----------------------------------------------------------------------------
/**
	   Function: addCmd
	   Inputs:  int type type of data
				int data

	   Outputs: 

 */
// ----------------------------------------------------------------------------
void protocolRF::addCmd(int type,int data)
{
	m_sendframe.data[m_sendframe.taille]=type<<4;
	m_sendframe.data[m_sendframe.taille+1]=data;
	m_sendframe.taille+=2;
}

// ----------------------------------------------------------------------------
/**
	   Function: addData
	   Inputs:  int type type of data
				int data

	   Outputs: 

 */
// ----------------------------------------------------------------------------
void protocolRF::addData(int type,int data)
{
	int oldindex = m_sendframe.taille;


	switch (type)
	{
	// 4 bits no signed
	case DATA_ETAT :
		if (m_sendframe.taille<29)
		{
			m_sendframe.taille++;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=data&0x0f;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	

		// 12 bits signed
	case DATA_DEGREEC:
	case DATA_DEGREEF :
	case DATA_PERCENT :
	case DATA_HUMIDITY:
		if (m_sendframe.taille<28)
		{
			m_sendframe.taille+=2;
			m_sendframe.data[oldindex]=type<<4;
			if (data <0)
			{
				data=data *-1;
				m_sendframe.data[oldindex]^=0x8;
			}
			m_sendframe.data[oldindex]+=(data>>8)&0x0f;
			m_sendframe.data[oldindex+1]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";

		break;	

		// 12 bits no signed
	case DATA_DISTANCE:
	case DATA_PRESSION:
		if (m_sendframe.taille<28)
		{
			m_sendframe.taille+=2;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=(data>>8)&0x0f;
			m_sendframe.data[oldindex+1]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	

		// 20 bits no signed
	case DATA_WATT  :
		if (m_sendframe.taille<27)
		{
			m_sendframe.taille+=3;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=(data>>16)&0x0f;
			m_sendframe.data[oldindex+1]=(data>>8)&0xff;
			m_sendframe.data[oldindex+2]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	
	}

}


// ----------------------------------------------------------------------------
/**
	   Function: pll
	   Inputs:  

	   Outputs: 

// Called 4 times for each bit period
// This function try to syncronize signal
 */
// ----------------------------------------------------------------------------
void protocolRF::pll()
{
	sample_count ++;

	// On additionne chaque sample et on incrémente le nombre du prochain sample
	if (m_sample_value)
	{
		sample_sum++;
	}

	// On vérifie s'il y a eu une transition de bit
	if (m_sample_value != last_sample_value)
	{

		// Transition, en avance si la rampe > 40, en retard si < 40
		if(pll_ramp < 80)
		{
			pll_ramp += 11; 
		} else
		{
			pll_ramp += 29;
		}
		last_sample_value = m_sample_value;
	}
	else
	{
		// Si pas de transition, on avance la rampe de 20 (= 80/4 samples)
		pll_ramp += 20;
	}

	// On vérifie si la rampe à atteint son maximum de 80
	if (pll_ramp >= 160)
	{
		//	log ("pll ok Bits:",rx_bits);
		t_start = micros();
		// On ajoute aux 16 derniers bits reçus rx_bits, MSB first
		// On stock les 16 derniers bits
		rx_bits <<= 1;

		// On vérifie la somme des samples sur la période pour savoir combien était à l'état haut
		// S'ils étaient < 2, on déclare un 0, sinon un 1;
		if (sample_sum >= 5)
		{
			rx_bits |= 0x1;
			bit_value = 1;
			//	   		digitalWrite(pinCop, HIGH);
		}
		else
		{
			rx_bits |= 0x0;
			bit_value = 0;
			//	   		digitalWrite(pinCop,LOW);
		}
		pll_ramp -= 160; // On soustrait la taille maximale de la rampe à sa valeur actuelle
		sample_sum = 0; // On remet la somme des samples à 0 pour le prochain cycle
		sample_count = 1; // On ré-initialise le nombre de sample



		// Si l'on est dans le message, c'est ici qu'on traite les données
		if (rx_active)
		{
			//			if(debugActivated)
			//				log("message : ",rx_bytes_count);

			bit_count ++;

			// On récupère les bits et on les places dans des variables
			// 1 bit sur 2 avec Manchester
			if (bit_count % 2 == 1)
			{
				if (bit_count < 16)
				{
					// Les 8 premiers bits de données
					receptor <<= 1;
					receptor |= bit_value;
				}
				else if (bit_count < 32)
				{
					// Les 8 bits suivants
					sender <<= 1;
					sender |= bit_value;
				}
				else if (bit_count < 38)
				{
					// Les 3 bits de type
					type <<= 1;
					type |= bit_value;
				}
				else if (bit_count < 48)
				{
					// Les 5 bits de longueur de trame
					rx_bytes_count <<= 1;
					rx_bytes_count |= bit_value;
				}
				else if ((bit_count-48) < (rx_bytes_count * 16))
				{
					length_ok = 1;
					m_data[(bit_count-48)/16] <<= 1;
					m_data[(bit_count-48)/16]|= bit_value;
				}
			}

			// Quand on a reçu les 24 premiers bits, on connait la longueur de la trame
			// On vérifie alors que la longueur semble logique	
			if (bit_count >= 48)
			{
				// Les bits 19 à 24 informent de la taille de la trame
				// On les vérifie car leur valeur ne peuvent être < à 1 et > à 31
				if (rx_bytes_count < 1 || rx_bytes_count > 31)
				{
					if(debugActivated)
						YDLE_DEBUG << "error!" << rx_bytes_count;

					// Mauvaise taille de message, on ré-initialise la lecture
					rx_active = false;
					sample_count = 1;
					bit_count = 0;
					length_ok = 0;
					sender = 0;
					receptor = 0;
					type = 0;
					taille = 0;
					memset(m_data,0,sizeof(m_data));
					t_start = micros();
					return;
				}
			}

			// On vérifie si l'on a reçu tout le message
			if ((bit_count-48) >= (rx_bytes_count*16) && (length_ok == 1))
			{
				if(debugActivated)
					YDLE_DEBUG <<  ("complete");

				rx_active = false;
				m_receivedframe.sender = sender;
				m_receivedframe.receptor = receptor;
				m_receivedframe.type = type;
				m_receivedframe.taille = rx_bytes_count;
				memcpy(m_receivedframe.data,m_data,rx_bytes_count-1); // copy data len - crc

				// crc calcul
				m_receivedframe.crc = computeCrc(&m_receivedframe);

				if(m_data[rx_bytes_count-1] != m_receivedframe.crc) {	
					if(debugActivated)
						YDLE_WARN << "crc error !!!";
				}
				else
				{
					m_rx_done = true;
				}
				length_ok = 0;
				sender = 0;
				receptor = 0;
				type = 0;
				taille = 0;
				memset(m_data,0,sizeof(m_data));
			}

		}

		// Pas dans le message, on recherche l'octet de start
		else
		{
			if (rx_bits == 0x06559)
			{
				if(debugActivated)
					YDLE_DEBUG << ("start received");
				// Octet de start, on commence à collecter les données
				rx_active = true;
				bit_count = 0;
				rx_bytes_count = 0;
			}
		}
	}
}

// ----------------------------------------------------------------------------
/**
	   Function: listenSignal
	   Inputs:  

	   Outputs: 

//thread reading RX PIN
 */
// ----------------------------------------------------------------------------
void protocolRF::listenSignal()
{
	int err = 0;
	scheduler_realtime();
	timespec time;

	while(1)
	{
		// Si le temps est atteint, on effectue une mesure (sample) puis on appelle la PLL
		//  *-------  1 CAS, on calcul le temps a attendre entre 2 lecture ------
		int tempo=(t_start + (sample_count * f_bit)) -micros() -2;

		if(tempo<5) {
			tempo=5; // la fonction delayMicroseconds n'aime pas la valeur negative
		}
		time.tv_sec = 0;
		time.tv_nsec = tempo * 1000;
		nanosleep(&time, NULL);

		// try to received ONLY if we are not currently sending something
		err=pthread_mutex_lock(&g_mutexSynchro);
		if( err== 0)
		{		  
			if(isDone())
			{
				setDone(false);
			}

			m_sample_value = digitalRead(m_pinRx);

			pll();


			// if a full signal is received
			if(isDone())
			{
				// If it's a ACK then handle it
				if(m_receivedframe.type == TYPE_ACK)
				{
					std::list<protocolRF::ACKCmd_t>::iterator i;
					for(i=mListACK.begin(); i != mListACK.end(); ++i)
					{
						if(m_receivedframe.sender == i->Frame.receptor
								&& m_receivedframe.receptor == i->Frame.sender)
						{
							YDLE_DEBUG << "Remove ACK from pending list";
							i=mListACK.erase(i);
							break; // remove only one ACK at a time.
						}
					}
				}
				else if(m_receivedframe.type == TYPE_ETAT_ACK)
				{
					// Send ACK	
					dataToFrame(m_receivedframe.sender,m_receivedframe.receptor,TYPE_ACK);				
					delay (250);
					// Sequence AGC supplémentaire nécessaire
					for (int x=0; x < 32; x++)
					{
						sendBit(true);
						sendBit(false);
					}
					transmit(0);
					AddToListCmd(m_receivedframe);
				}
				else //else send it to IHM
				{
					YDLE_DEBUG << "New frame ready to be sent :";
					printFrame(m_receivedframe);
					AddToListCmd(m_receivedframe);
				}
			}

			// Let's Send thread
			pthread_mutex_unlock(&g_mutexSynchro);
		}
		else
		{
			YDLE_WARN << "error acquire mutex" << err;
		}
		// check if we need re-transmit	
		checkACK();
	}	
	// Code jamais atteint....
	scheduler_standard();
}

void* protocolRF::listenSignal(void* pParam)
{
	YDLE_DEBUG << "Enter in thread listen";
	protocolRF* parent=(protocolRF*)pParam;
	
//	scheduler_realtime();

	if (pParam)
	{
		parent->listenSignal() ;
	}
	YDLE_INFO << "Exit of thread listen";

//	scheduler_standard ();

	return NULL;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: power2()
	   Inputs:  power

	   Outputs:

    Calcul le nombre 2^chiffre indiqué, fonction utilisé par itob pour la conversion decimal/binaire
 */
// ----------------------------------------------------------------------------
unsigned long protocolRF::power2(int power)
{
	unsigned long integer=1;
	for (int i=0; i<power; i++)
	{
		integer*=2;
	}
	return integer;
}


// ----------------------------------------------------------------------------
/**
	   Routine: itob()
	   Inputs:  integer
	   start,
	   length

	   Outputs:


 */
// ----------------------------------------------------------------------------
void protocolRF::itob(unsigned long integer, int start, int length)
{
	for (int i=0; i<length; i++)
	{
		int pow2 = 1 << (length-1-i) ;
		m_FrameBits[start + i] = ((integer & pow2) != 0);
	}
}

// ----------------------------------------------------------------------------
/**
	   Routine: dataToFrame
	   Inputs:  Receiver
	   Transmitter,
	   type
		data
	   Outputs:


 */
// ----------------------------------------------------------------------------
void protocolRF::dataToFrame(unsigned long Receiver, unsigned long Transmitter, unsigned long type)
{
	memset(m_sendframe.data,0,sizeof(m_sendframe.data));
	m_sendframe.sender=Transmitter;
	m_sendframe.receptor=Receiver;
	m_sendframe.type=type;
	m_sendframe.taille=0;
	m_sendframe.crc=0;
} 


// ----------------------------------------------------------------------------
/**
	   Routine: printFrame
	   Inputs:  


	   // log Frame
 */
// ----------------------------------------------------------------------------
void protocolRF::printFrame(Frame_t & trame)
{
	// if debug
	if(debugActivated)
	{
		char sztmp[255];
		YDLE_DEBUG << "Emetteur : " << (int)trame.sender;
		YDLE_DEBUG << "Recepteur :" << (int)trame.receptor;
		YDLE_DEBUG << "Type :" << (int)trame.type;
		YDLE_DEBUG << "Taille :" << (int)trame.taille;
		YDLE_DEBUG << "CRC :" << (int)trame.crc;

		sprintf(sztmp,"Data Hex: ");
		for (int a=0;a<trame.taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,trame.data[a]);
		YDLE_DEBUG << sztmp;

		sprintf(sztmp,"Data Dec: ");
		for (int a=0;a<trame.taille-1;a++)
			sprintf(sztmp,"%s %d",sztmp,trame.data[a]);
		YDLE_DEBUG << sztmp;
	}
}




// ----------------------------------------------------------------------------
/**
	   Routine: debugMode
	   Inputs:  


	   // Activate debug mode
 */
// ----------------------------------------------------------------------------
void protocolRF::debugMode()
{
	debugActivated = true;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
int protocolRF::getType()
{
	return m_receivedframe.type;
}



// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
int protocolRF::getTaille()
{
	return m_receivedframe.taille;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
uint8_t* protocolRF::getData()
{
	return m_receivedframe.data;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
int protocolRF::isSignal()
{
	return rx_active;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
void protocolRF::setDone(bool bvalue)
{
	m_rx_done=bvalue;
}

// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
bool protocolRF::isDone()
{
	return m_rx_done;
}


//
// ----------------------------------------------------------------------------
/**
	   Routine: checkACK()
	   Inputs:  

	   Outputs:

    Check if CMD command was not received there ACK. retry if needed
 */
// ----------------------------------------------------------------------------
void protocolRF::checkACK()
{
	if(!mListACK.empty()) {
		std::list<protocolRF::ACKCmd_t>::iterator i;

		int iTime = getTime() ;

		for(i=mListACK.begin(); i != mListACK.end(); ++i)
		{
			if ( (iTime - i->Time) > TIME_OUT_ACK ||  i->Time > iTime )
			{
				YDLE_WARN << "Ack not receive from receptor: " << (int)i->Frame.receptor;
				// if more than 2 retry, then remove it
				if( i->iCount >=2)
				{
					YDLE_WARN << "ACK never received.";
					i=mListACK.erase(i);		// TODO : Send IHM this error
				}
				else
				{
					memcpy(&m_sendframe, &i->Frame,sizeof(Frame_t));
					m_sendframe.taille--; // remove CRC.It will be add in the transmit function
					i->Time=iTime;
					i->iCount++;
					transmit(true);	// re-Send frame;
				}		 
			}
		}
	}
}

int protocolRF::getTime()
{
	struct timeval localTime;
	gettimeofday(&localTime, NULL); 
	int iTime=localTime.tv_sec * 1000000;
	iTime+=localTime.tv_usec;
	return iTime;
}
