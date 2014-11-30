// I2C-Master-Routinen von Peter Fleury verwenden
// siehe http://homepage.hispeed.ch/peterfleury/avr-software.html#libs
// Hier sind auch die Dateien: i2cmaster.h und twimaster.c zu finden, die ben�tigt werden
// Letztes Update des Codes 5. April 2010 durch HannoHupmann

#include <avr/io.h>
#include <avr/interrupt.h>			// Standard-Headerdatei(avr-libc): Funktionen in diesem Modul erm�glichen das Handhaben von Interrupts
#include <avr/pgmspace.h>
#include "global.h"					// globale Deklarationen
#include "rs232.h"					// Headerdatei fuer rs232.c
#include "TWI_Master.h"				// Headerdatei für TWI_Master.c
#define SLAVE_ADRESSE 0x50			// Slaveadresse von AVR-Slave-Drivecontrol

//const char FlashString[] PROGMEM = ("MASTER gestartet..."CR);
volatile unsigned char MD49data[18];
volatile uint8_t MD49commands[15];

void initMD49commands(void){
	MD49commands[0]=128; 	// default speed1=128 (stop)
	MD49commands[1]=128;	// default speed2=128 (stop)
	MD49commands[2]=5;		// default acceleration=5
	MD49commands[3]=0;		// default mode=0
	MD49commands[4]=0;		// 0=do not reset encoders (default, 1=reset encoders
	MD49commands[5]=1;		// regulator enabled (default)
	MD49commands[6]=0;		// timeout disabled (default)
	MD49commands[7]=0;		// Encoder1 Byte 1
	MD49commands[8]=0;		// Encoder1 Byte 2
	MD49commands[9]=0;		// Encoder1 Byte 3
	MD49commands[10]=0;		// Encoder1 Byte 4
	MD49commands[11]=0;		// Encoder2 Byte 1
	MD49commands[12]=0;		// Encoder2 Byte 2
	MD49commands[13]=0;		// Encoder2 Byte 3
	MD49commands[14]=0;		// Encoder2 Byte 4
}

// Sende in Array MD49commands gesetzte Befehle für MD49 an AVR_Slave_DriveControl
// im Master-Transmitter-Mode
void sendMD49commands(void){
	uint8_t i;
	if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){		// Slave bereit zum schreiben?
		i2c_write(0x00);  							// Buffer Startadresse setzen
		for (i=0;i<15;i++){
			i2c_write(MD49commands[i]);
		}
		i2c_stop();       							// Zugriff beenden
	}
	else
	{
	//		/* Fehlerbehandlung... */
	}
}

// Lese MD49 Daten (Encodervalues, Mode, Acceleration, etc..)
// von Slave_Drivecontrol und speichere sie in Array MD49data
void readMD49data(void){
	uint8_t i;
	// Daten von I2C- Slave lesen im Master-Receiver-Mode
	if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))) 			// Slave bereit zum lesen?
	{
		i2c_write(15); 									// Buffer Startadresse zum Auslesen
		i2c_rep_start(SLAVE_ADRESSE+I2C_READ); 			// Lesen beginnen
			for (i=0;i<18;i++)							// Empfangene Daten in Array schreiben
				{
					if ((i) < 17)
					{
						MD49data[i]=i2c_readAck();		// Bytes lesen...
					}
					else
					{
						MD49data[i]=i2c_readNak();		// letztes Byte lesen...
					}
				}
		i2c_stop();										// Zugriff beenden
	}
	else
	{
	//		/* Fehlerbehandlung... */
	}
}

int main(void)
{
	init_uart();										// UART initalisieren und einschalten. Alle n�tigen Schnittstellen-Parameter und -Funktionen werden in rs232.h definiert
	i2c_init();											// init I2C interface
	//sei();
	initMD49commands();

	uint8_t UART_RXBuffer;

	while(1)											// Main- Endlosschleife
    {

		sendMD49commands();
		readMD49data();

		//parse commands
		UART_RXBuffer=uart_getc();

		//if (UART_MSG_FLAG==1){						// UART_MSG_FLAG auswerten: gesetzt in Empfangs- Interruptroutine wenn "CR" empfangen oder UART- Puffer voll
			if (UART_RXBuffer==84){// "T"				// T als erstes Zeichen empfangen bedeutet, dass alle MD49Kommandos im Paket gesendet wurden.
				uint8_t i;

				for (i=0;i<15;i++){
					UART_RXBuffer=uart_getc();
					MD49commands[i]=UART_RXBuffer;		// MD49Kommandos mit neu empfangenen Kommandos überschreiben
				}
			}

			else if (UART_RXBuffer==88){// "X"			// X als erstes Zeichen empfangen bedeutet, dass ein bestimmtes MD49Kommando empfangen wurde
				UART_RXBuffer=uart_getc();
				if (UART_RXBuffer==115){//"s"		// s: setSpeed 1 + 2
					UART_RXBuffer=uart_getc();
					MD49commands[0]=UART_RXBuffer;	// set speed1 with received value
					UART_RXBuffer=uart_getc();
					MD49commands[1]=UART_RXBuffer;	// set speed2 with received value
				}
				if (UART_RXBuffer==97){//"a"			// a: Acceleration
					UART_RXBuffer=uart_getc();
					MD49commands[2]=UART_RXBuffer;	// set Acceleration with received value
				}
				if (UART_RXBuffer==109){//"m"		// m: setMode
					UART_RXBuffer=uart_getc();
					MD49commands[3]=UART_RXBuffer;	// set Mode with received value
				}
			}
			else if (UART_RXBuffer==82){// "R"			// R als erstes Zeichen empfangen bedeutet,	dass alle MD49-Daten angefordert wurden
			// Data angefordert, MD49data senden:
				uint8_t i;
				for (i=0;i<18;i++){
					uart_putc(MD49data[i]);				// Alle MD49-Daten senden...
				}
			}

			//UART_MSG_FLAG=0;
			//UART_RxCount=0;
		//}//end.if uart_msg_flag set
	}//end.while(1) endless loop
}//end.main
