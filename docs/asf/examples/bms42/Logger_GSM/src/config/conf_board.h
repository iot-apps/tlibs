

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#include <asf.h>

#include <conf_ArduinoM0Pro_board.h>


// Interface Bluetooth
#define BMS42_PIN_RESET     PIN_PA07
#define BMS42_BAUDRATE      9600
#define BMS42_DATABIT       USART_CHARACTER_SIZE_8BIT
#define BMS42_STOPBIT       USART_STOPBITS_1
#define BMS42_PARITY        USART_PARITY_NONE

#define COM3_PIN_RESET      PIN_PA07
#define COM3_BAUDRATE       115200
#define COM3_DATABIT        USART_CHARACTER_SIZE_8BIT
#define COM3_STOPBIT        USART_STOPBITS_1
#define COM3_PARITY         USART_PARITY_NONE

// Interface Serial
#define SERIAL_BAUDRATE     115200
#define SERIAL_DATABIT      USART_CHARACTER_SIZE_8BIT
#define SERIAL_STOP_BITS    USART_STOPBITS_1
#define SERIAL_PARITY       USART_PARITY_NONE

// Interface Serial2
#define SERIAL2_BAUDRATE    BMS42_BAUDRATE
#define SERIAL2_DATABIT     BMS42_DATABIT
#define SERIAL2_STOP_BITS   BMS42_STOPBIT
#define SERIAL2_PARITY      BMS42_PARITY

// Interface Serial3
#define SERIAL3_BAUDRATE    COM3_BAUDRATE
#define SERIAL3_DATABIT     COM3_DATABIT
#define SERIAL3_STOP_BITS   COM3_STOPBIT
#define SERIAL3_PARITY      COM3_PARITY

#endif // CONF_BOARD_H
