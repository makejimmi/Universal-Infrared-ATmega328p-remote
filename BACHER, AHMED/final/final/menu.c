/*
 * ir.c
 * 
 * This module is responsible for the infrared record / replay
 * 
 * TBD: extend this file header with infos & name
 */

#include "common.h"

#define BUTTONPD2 ((PIND & (1<<2)) == 0)
#define BUTTONPD3 ((PIND & (1<<3)) == 0)
#define BUTTONPD4 ((PIND & (1<<4)) == 0)
#define BUTTONPD5 ((PIND & (1<<5)) == 0)

/**@brief Init UI/LCD
 *
 *This function initializes the SPI interface
 *and displays the welcome message.
 */
volatile int8_t selectedOption = -1;

void button()
{
	if (BUTTONPD2)
	{
		selectedOption++;
		while (BUTTONPD2) {} //Solang der Button gepresst ist bleibt man bei der ausgewählten Option (Iteration wird verhindert)
	}

	if (selectedOption >= 3) //sobald Selected option größer als 3 ist, wird die Variable auf Null gesetzt um wieder bei record starten zu können 
	{
		selectedOption = 0;
	}

	_delay_ms(50);	 //Button Debouncing 
}

void record()

{
	lcdClear();
	lcdWriteString(0, 0, "  >>>RECORD<<<");
}

void replay()

{
	lcdWriteString(0, 0, "  >>>REPLAY<<<");
}

void delete()

{
	lcdWriteString(0, 0, "  >>>DELETE<<<");
}

uint8_t recording()

{
	lcdClear();
	lcdWriteString(1, 0, "RECORDING...");
	return 0;
}

uint8_t replaying()

{
	lcdClear();
	lcdWriteString(1, 0, "REPLAYING...");
	return 1;
}

uint8_t deleting()

{
	lcdClear();
	lcdWriteString(1, 0, "DELETING...");
	return 2;
}

void ui_init()
{
	DDRD |= (0x3c);	//0011 1100
	PORTD |= (0x3c); 

	lcdSpiInit();
	lcdInit();
	lcdWriteString(0, 0, "WELCOME");
	lcdWriteString(1, 0, "(press S1)");
	uart_sendstring("\n\rWELCOME");
}

//TBD: call the init function of dogm_lcd

//TBD: display welcome message.

/**@brief Main UI/menu/LCD function
 *
 *This function is called each time when the program requests user input
 *This function blocks as long as no command is selected.
 *
 *When the function returns, the return value is a number for the
 *selected command. This number is used in main.c to do the corresponding
 *action (record/replay/delete).
 *
 *@param index This pointer is set here to have an index value for main.c
 *@param ir_name This string is set here to a command name (for store)
 *@return Selected command
 *
 *TBD: clearify the command return by using macros
 */

uint8_t ui_get_selection(uint8_t *index, char *ir_name)
{
	//this delay is necessary, otherwise this function is called
	// as fast as possible (flooding of the terminal)
	_delay_ms(1000);
	uint8_t var = 5; //in der Main Funktion

	while (1)
	{
		button();

		if (selectedOption == 0)  
		{
			record();
			if (BUTTONPD3) //mit S2 wird bestätigt welche der drei Optionen ausgeführt werden soll
			{
				_delay_ms(1000);
				uart_sendstring("\n\rrecording...");
				var = recording();
			}
		}
		else if (selectedOption == 1)
		{
			replay();
			if (BUTTONPD3)
			{
				uart_sendstring("\n\rreplaying...");
				var = replaying();
			}
		}
		else if (selectedOption == 2)
		{
			delete();
			if (BUTTONPD3)
			{
				uart_sendstring("\n\rdeleting...");
				var = deleting();
			}
		}

		if (var != 5) break; //breaked raus aus der while Loop sobald eines der Optionen gewählt wird
	}

	selectedOption = -1;
	return var;
}

uint8_t Alphabet(char *name)
{
	lcdClear();
	uint8_t col = 0;
	uint8_t row = 0;
	uint8_t i = 0;
	uint8_t page = 0;
	uint8_t shown = 1;

	lcdCursorOnOff(1, 1);
	char arr[10];

	while (1)
	{
		out: lcdSetCursor(row, col);

		if ((page == 0) && (shown))
		{
			lcdClear();
			lcdWriteString(0, 0, "ABCDEFGHIJKLMNOP");
			uart_sendstring("\n\rABCDEFGHIJKLMNOP");
			lcdWriteString(1, 0, "QRSTUVWXYZ      ");
			uart_sendstring("\n\rQRSTUVWXYZ");
			shown = 0;
		}
		else if ((page == 1) && (shown))
		{
			lcdClear();
			lcdWriteString(0, 0, "0123456789      ");
			uart_sendstring("\n\r123456789");
			shown = 0;
		}

		if (BUTTONPD4)	//Button nach rechts (S3)
		{
			_delay_ms(50);
			while (BUTTONPD4) {}

			col++;

			if ((row == 0) && (col >= 16)) //Wird Reihe 0, Spalte 16 erreicht, gelangt man zu Reihe 1, Spalte 0
			{
				row = 1;
				col = 0;
			}

			if ((row == 1) && (col >= 16)) //Wird Reihe 1, Spalte 16 erreicht, gelangt man zu Reihe 0, Spalte 0, Seite 1
			{
				col = 0;
				row = 0;
				shown = 1;
				if (page == 1)
				{
					page = 0;
					goto out;
				}

				page++;
				if (page > 1) page = 0; 
				if (page == 0) page = 1;
			}
		}

		if (BUTTONPD5)	//Button nach links (S4)
		{
			_delay_ms(50);
			while (BUTTONPD5) {}

			col--;

			if ((row == 0) && (col <= 0))
			{
				row = 1;
				col = 16;
				shown = 1;
				if (page == 0)
				{
					page = 1;
					goto out;
				}

				page--;
				if (page < 0) page = 1;
				if (page == 1) page = 0;
			}

			if ((row == 1) && (col <= 0))
			{
				row = 0;
				col = 16;
			}
		}

		if (BUTTONPD3)	// Um Namen auswählen zu können (S2)
		{
			_delay_ms(50);
			while (BUTTONPD3);
			if ((row == 0) && (col == 0) && (page == 0))
			{
				arr[i] = 'a';
				i++;
			}

			if ((row == 0) && (col == 1) && (page == 0))
			{
				arr[i] = 'b';
				i++;
			}

			if ((row == 0) && (col == 2) && (page == 0))
			{
				arr[i] = 'c';
				i++;
			}

			if ((row == 0) && (col == 3) && (page == 0))
			{
				arr[i] = 'd';
				i++;
			}

			if ((row == 0) && (col == 4) && (page == 0))
			{
				arr[i] = 'e';
				i++;
			}

			if ((row == 0) && (col == 5) && (page == 0))
			{
				arr[i] = 'f';
				i++;
			}

			if ((row == 0) && (col == 6) && (page == 0))
			{
				arr[i] = 'g';
				i++;
			}

			if ((row == 0) && (col == 7) && (page == 0))
			{
				arr[i] = 'h';
				i++;
			}

			if ((row == 0) && (col == 8) && (page == 0))
			{
				arr[i] = 'i';
				i++;
			}

			if ((row == 0) && (col == 9) && (page == 0))
			{
				arr[i] = 'j';
				i++;
			}

			if ((row == 0) && (col == 10) && (page == 0))
			{
				arr[i] = 'k';
				i++;
			}

			if ((row == 0) && (col == 11) && (page == 0))
			{
				arr[i] = 'l';
				i++;
			}

			if ((row == 0) && (col == 12) && (page == 0))
			{
				arr[i] = 'm';
				i++;
			}

			if ((row == 0) && (col == 13) && (page == 0))
			{
				arr[i] = 'n';
				i++;
			}

			if ((row == 0) && (col == 14) && (page == 0))
			{
				arr[i] = 'o';
				i++;
			}

			if ((row == 0) && (col == 15) && (page == 0))
			{
				arr[i] = 'p';
				i++;
			}

			if ((row == 1) && (col == 0) && (page == 0))
			{
				arr[i] = 'q';
				i++;
			}

			if ((row == 1) && (col == 1) && (page == 0))
			{
				arr[i] = 'r';
				i++;
			}

			if ((row == 1) && (col == 2) && (page == 0))
			{
				arr[i] = 's';
				i++;
			}

			if ((row == 1) && (col == 3) && (page == 0))
			{
				arr[i] = 't';
				i++;
			}

			if ((row == 1) && (col == 4) && (page == 0))
			{
				arr[i] = 'u';

				i++;
			}

			if ((row == 1) && (col == 5) && (page == 0))
			{
				arr[i] = 'v';
				i++;
			}

			if ((row == 1) && (col == 6) && (page == 0))
			{
				arr[i] = 'w';
				i++;
			}

			if ((row == 1) && (col == 7) && (page == 0))
			{
				arr[i] = 'x';
				i++;
			}

			if ((row == 1) && (col == 8) && (page == 0))
			{
				arr[i] = 'y';
				i++;
			}

			if ((row == 1) && (col == 9) && (page == 0))
			{
				arr[i] = 'z';
				i++;
			}

			if ((row == 0) && (col == 0) && (page == 1))
			{
				arr[i] = '0';
				i++;
			}

			if ((row == 0) && (col == 1) && (page == 1))
			{
				arr[i] = '1';
				i++;
			}

			if ((row == 0) && (col == 2) && (page == 1))
			{
				arr[i] = '2';
				i++;
			}

			if ((row == 0) && (col == 3) && (page == 1))
			{
				arr[i] = '3';
				i++;
			}

			if ((row == 0) && (col == 4) && (page == 1))
			{
				arr[i] = '4';
				i++;
			}

			if ((row == 0) && (col == 5) && (page == 1))
			{
				arr[i] = '5';
				i++;
			}

			if ((row == 0) && (col == 6) && (page == 1))
			{
				arr[i] = '6';
				i++;
			}

			if ((row == 0) && (col == 7) && (page == 1))
			{
				arr[i] = '7';
				i++;
			}

			if ((row == 0) && (col == 8) && (page == 1))
			{
				arr[i] = '8';
				i++;
			}

			if ((row == 0) && (col == 9) && (page == 1))
			{
				arr[i] = '9';
				i++;
			}
		}

		if (i >= 8)
		{
			goto end;
		}


		if (BUTTONPD2) //Mit S1 wird der Name bestätigt und ausgeprinted
		{
			end: arr[i++] = 0;
			i = 0;
			while (BUTTONPD2);
			lcdClear();
			uint8_t len = 0;
			while (arr[len] != 0)
			{
				len++;
			}

			lcdSetCursor(0, 0);
			for (i = 0; i < len; i++)
			{
				lcdSetCursor(0, i);
				lcdWriteChar(arr[i]);
				name[i] = arr[i];
			}

			uart_sendstring(name);
			return 0;
		}
	}
}
