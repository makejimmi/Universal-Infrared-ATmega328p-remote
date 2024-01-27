/*
 * common.h
 * 
 * Copyright 2020 Benjamin Aigner <aignerb@technikum-wien.at>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#ifndef _COMMON_H_
#define _COMMON_H

//include all modules
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "ir.h"
#include "dogm_lcd.h"
#include "eeprom.h"
#include "menu.h"



/** @brief Length of IR timings array
 * @warning Size in bytes is double (we allocate this number of uint16_t)
 */
#define MAX_IR_EDGES 250

/** @brief Max length of any IR command name
 * 
 * Please take care of the length, do not accept input names longer than this array!
 */
#define MAX_NAME_LEN 10


/** @brief Array of timestamps between edges
 * 
 * This array is used as global "scratchpad" to store IR edges on recording
 * (because the EEPROM is too slow) or as memory for replaying a command.
 * (EEPROM is too slow on loading -> timings must be in the array before
 * replay starts).
 * 
 * @see MAXSIZE_IR_TIMINGS
 */
extern uint16_t  ir_timings[MAX_IR_EDGES];

/** @brief IR command name string
 * 
 * This array is used to store a name temporarily (either for replay or record).
 * 
 * @see MAXSIZE_IR_NAME
 */
extern char ir_name[MAX_NAME_LEN];



////////////////////////////////////////////////////////////////////////
/////////// UART functions (from lecture)
////////////////////////////////////////////////////////////////////////


/** @brief Init UART
 * 
 * @param baudrate Used baudrate
 */
void uart_init(uint32_t baudrate);

/** @brief Transmit one character (blocking)
 * @param c Character to be transmitted
 * @note Blocking function!
 */
void uart_transmit(uint8_t c);

/** @brief Transmit a string (blocking)
 * @param str String to be sent
 * @note Blocking function!
 */
void uart_sendstring(char * str );

void int_to_str(uint16_t val, char * target);

#endif /* _COMMON_H_ */
