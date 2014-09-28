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


int main(void)
{
	init_uart();																			// UART initalisieren und einschalten. Alle n�tigen Schnittstellen-Parameter und -Funktionen werden in rs232.h definiert
	i2c_init();																				// init I2C interface
	sei();
	uart_puts_p(FlashString);																// Demonstriert "rs232.c/uarts_put_p" f�r die Ausgabe eines Strings
	uart_puts ("input 'w','a','s','d' to move forward, left,backward, right or 'x' to stop, followed by enter"CR);
	uart_puts ("input 'T' or 't' followed by enter to Enable or Disable MD49 Timeout function."CR);
	while(1)																				// Main- Endlosschleife
    {
		if (UART_MSG_FLAG==1)																// UART_MSG_FLAG auswerten: gesetzt in Empfangs- Interruptroutine wenn "CR" empfangen oder UART- Puffer voll
		{
			if (UART_RXBuffer[0]==119){// "w"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
					i2c_write(0x00);  														// Buffer Startadresse setzen
					i2c_write(255);															// Bytes schreiben...
					i2c_write(255);															// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if
			if (UART_RXBuffer[0]==115){// "s"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
					i2c_write(0x00);  														// Buffer Startadresse setzen
					i2c_write(0x00);														// Bytes schreiben...
					i2c_write(0x00);														// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if
			if (UART_RXBuffer[0]==120){// "x"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
					i2c_write(0x00);  														// Buffer Startadresse setzen
					i2c_write(128);															// Bytes schreiben...
					i2c_write(128);															// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if
			if (UART_RXBuffer[0]==97){// "a"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
					i2c_write(0x00);  														// Buffer Startadresse setzen
					i2c_write(0x00);														// Bytes schreiben...
					i2c_write(255);															// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if
			if (UART_RXBuffer[0]==100){// "d"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									// Slave bereit zum schreiben?
					i2c_write(0x00);  														// Buffer Startadresse setzen
					i2c_write(255);															// Bytes schreiben...
					i2c_write(0x00);														// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if

			//enable Timeout
			if (UART_RXBuffer[0]==84){// "T"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									//Slave bereit zum schreiben?
					i2c_write(0x06);  														// Buffer Startadresse setzen
					i2c_write(0x01);														// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if
			//disable Timeout
			if (UART_RXBuffer[0]==116){// "t"
				if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))){									//Slave bereit zum schreiben?
					i2c_write(0x06);  														// Buffer Startadresse setzen
					i2c_write(0x00);														// Bytes schreiben...
					i2c_stop();       														// Zugriff beenden
				}
			}//end.if


			// (3) Daten von I2C- Slave lesen im MR-Mode
			// if(!(i2c_start(SLAVE_ADRESSE+I2C_WRITE))) //Slave bereit zum lesen?
			//	{
			//		uart_puts("Daten von Slave lesen: ");
			//		i2c_write(0x00); //Buffer Startadresse zum Auslesen
			//		i2c_rep_start(SLAVE_ADRESSE+I2C_READ); //Lesen beginnen
//
			//		for (i=0;i<UART_RxCount;i++)											// Empfangene Daten in Array schreiben
			//			{
			//				if ((i) < UART_RxCount-1)
			//				{
			//					TWI_RXBuffer[i]=i2c_readAck();									// Bytes schreiben...
			//					uart_putc(TWI_RXBuffer[i]);
			//				}
			//				else
			//				{
			//					TWI_RXBuffer[i]=i2c_readNak();									// letztes Bytes schreiben...
			//					uart_putc(TWI_RXBuffer[i]);
			//				}
//
			//			}
//
			//		i2c_stop();																// Zugriff beenden
			//		uart_puts(CR);
			//	}
			// else
			//	{
			//		/* Hier k�nnte eine Fehlermeldung ausgegeben werden... */
			//		uart_puts("Fehler beim lesen der Daten an Slave"CR);
			//	}
			UART_MSG_FLAG=0;
			UART_RxCount=0;
		}//end.if uart_msg_flag set
	}//end.while(1) endless loop
}//end.main
