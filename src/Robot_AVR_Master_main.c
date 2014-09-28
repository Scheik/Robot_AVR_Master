// I2C-Master-Routinen von Peter Fleury verwenden
// siehe http://homepage.hispeed.ch/peterfleury/avr-software.html#libs
// Hier sind auch die Dateien: i2cmaster.h und twimaster.c zu finden, die ben�tigt werden
// Letztes Update des Codes 5. April 2010 durch HannoHupmann

#include <avr/io.h>
#include <avr/interrupt.h>																	// Standard-Headerdatei(avr-libc): Funktionen in diesem Modul erm�glichen das Handhaben von Interrupts
#include <avr/pgmspace.h>
#include "global.h"
#include "rs232.h"																			// Headerdatei fuer rs232.c
#include "TWI_Master.h"
#define SLAVE_ADRESSE 0x50																	// slaveadress drivecontrol

const char FlashString[] PROGMEM = ("MASTER gestartet..."CR);
volatile uint8_t MD49data[18];
volatile uint8_t MD49commands[15];

void initMD49commands(void){
	MD49commands[0]=128; 		// default speed1=128 (stop)
	MD49commands[1]=128;		// default speed2=128 (stop)
	MD49commands[2]=5;			// default acceleration=5
	MD49commands[3]=0;			// default mode=0
	MD49commands[4]=0;			// 0=do not reset encoders (default, 1=reset encoders
	MD49commands[5]=1;			// regulator enabled (default)
	MD49commands[6]=0;			// timeout disabled (default)
	MD49commands[7]=0;			// Encoder1 Byte 1
	MD49commands[8]=0;			// Encoder1 Byte 2
	MD49commands[9]=0;			// Encoder1 Byte 3
	MD49commands[10]=0;			// Encoder1 Byte 4
	MD49commands[11]=0;			// Encoder2 Byte 1
	MD49commands[12]=0;			// Encoder2 Byte 2
	MD49commands[13]=0;			// Encoder2 Byte 3
	MD49commands[14]=0;			// Encoder2 Byte 4
}


//Funktion: Sende in Array MD49commands gesetzte Befehle für MD49 an AVR_Slave_DriveControl
void sendMD49commands(void){
	uint8_t i;
	if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
		i2c_write(0x00);  														// Buffer Startadresse setzen
		for (i=0;i<15;i++){
			i2c_write(MD49commands[i]);
		}
		i2c_stop();       														// Zugriff beenden
	}
}

// Lese MD49 Daten von Slave_Drivecontrol und speichere sie in Array MD49data
void readMD49data(void){
	uint8_t i;
	// Daten von I2C- Slave lesen im MR-Mode
	if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))) 												// Slave bereit zum lesen?
	{
		i2c_write(0x15); 																	// Buffer Startadresse zum Auslesen
		i2c_rep_start(SLAVE_ADRESSE+I2C_READ); 												// Lesen beginnen
			for (i=0;i<18;i++)																// Empfangene Daten in Array schreiben
				{
					if ((i) < 17)
					{
						MD49data[i]=i2c_readAck();										// Bytes lesen...
					}
					else
					{
						MD49data[i]=i2c_readNak();										// letztes Byte lesen...
					}
				}
		i2c_stop();																			// Zugriff beenden
	}
	else
	{
	//		/* Fehlerbehandlung... */
	}
}

int main(void)
{
	init_uart();																			// UART initalisieren und einschalten. Alle n�tigen Schnittstellen-Parameter und -Funktionen werden in rs232.h definiert
	i2c_init();																				// init I2C interface
	sei();
	initMD49commands();
	uart_puts_p(FlashString);																// Demonstriert "rs232.c/uarts_put_p" f�r die Ausgabe eines Strings
	uart_puts ("input 'w','a','s','d' to move Full-orward, -left,-backward, -right and 'x' to stop, followed by enter"CR);
	uart_puts ("input 'T' or 't' followed by enter to Enable or Disable (default) MD49 Timeout."CR);
	uart_puts ("input 'R' or 'r' followed by enter to Enable (default) or Disable MD49 Regulator."CR);
	uart_puts ("input 'g' followed by enter to read MD49 Data."CR);
	while(1)																				// Main- Endlosschleife
    {
		readMD49data();
		sendMD49commands();

		//parse single commands for test purposes, may be deleted laters
		if (UART_MSG_FLAG==1)																// UART_MSG_FLAG auswerten: gesetzt in Empfangs- Interruptroutine wenn "CR" empfangen oder UART- Puffer voll
		{
			if (UART_RXBuffer[0]==119){// "w"
				MD49commands[0]=255;
				MD49commands[1]=255;
			}//end.if
			if (UART_RXBuffer[0]==115){// "s"
				MD49commands[0]=0x00;
				MD49commands[1]=0x00;
			}//end.if
			if (UART_RXBuffer[0]==120){// "x"
				MD49commands[0]=128;
				MD49commands[1]=128;
			}//end.if
			if (UART_RXBuffer[0]==97){// "a"
				MD49commands[0]=0x00;
				MD49commands[1]=255;
			}//end.if
			if (UART_RXBuffer[0]==100){// "d"
				MD49commands[0]=255;
				MD49commands[1]=0x00;
			}//end.if
			//enable Timeout
			if (UART_RXBuffer[0]==84){// "T"
				MD49commands[6]=0x01;
			}//end.if
			//disable Timeout
			if (UART_RXBuffer[0]==116){// "t"
				MD49commands[6]=0x00;
			}//end.if

			//NEW
			//enable Regulator
			if (UART_RXBuffer[0]==82){// "R"
				MD49commands[5]=0x01;
			}//end.if
			//disable Regulator
			if (UART_RXBuffer[0]==114){// "r"
				MD49commands[5]=0x00;
			}//end.if
			// send array MD49data to serial
			if (UART_RXBuffer[0]==103){// "g"
				uart_puts("Speed1=");uart_puti(MD49data[8]);uart_puts(" Speed2=");uart_puti(MD49data[9]);uart_puts(" Volts=");uart_puti(MD49data[10]);uart_puts(" Current1=");uart_puti(MD49data[11]);uart_puts(" Current2=");uart_puti(MD49data[12]);uart_puts(" Acceleration=");uart_puti(MD49data[14]);uart_puts(CR);
				uart_puts("Error=");uart_puti(MD49data[13]);uart_puts(" Mode=");uart_puti(MD49data[15]);uart_puts(" Regulator=");uart_puti(MD49data[16]);uart_puts("  Timeout=");uart_puti(MD49data[17]);uart_puts(CR);
				uart_puts("Encoder1 Byte1=");uart_puti(MD49data[0]);uart_puts(" Byte2=");uart_puti(MD49data[1]);uart_puts(" Byte3=");uart_puti(MD49data[2]);uart_puts("  Byte4=");uart_puti(MD49data[3]);uart_puts(CR);
				uart_puts("Encoder2 Byte1=");uart_puti(MD49data[4]);uart_puts(" Byte2=");uart_puti(MD49data[5]);uart_puts(" Byte3=");uart_puti(MD49data[6]);uart_puts("  Byte4=");uart_puti(MD49data[7]);uart_puts(CR);
			}//end.if
			//end.NEW

			UART_MSG_FLAG=0;
			UART_RxCount=0;
		}//end.if uart_msg_flag set
	}//end.while(1) endless loop
}//end.main
