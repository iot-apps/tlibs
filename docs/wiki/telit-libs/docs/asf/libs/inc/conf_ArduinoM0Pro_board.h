/*
 * conf_arduinoM0Pro_board.h
 *
 * Created: 15/03/2018 14:12:07
 *  Author: Jonas
 */ 


#ifndef CONF_ARDUINOM0PRO_BOARD_H_
#define CONF_ARDUINOM0PRO_BOARD_H_


// Leds
#define LED_RX_PIN  PIN_PB03
#define LED_TX_PIN  PIN_PA27
#define LED_L_PIN   PIN_PA17

// Interface Serial Edbg
#define SERIAL_PAD0         PINMUX_UNUSED
#define SERIAL_PAD1         PINMUX_UNUSED
#define SERIAL_PAD2         PINMUX_PB22D_SERCOM5_PAD2
#define SERIAL_PAD3         PINMUX_PB23D_SERCOM5_PAD3
#define SERIAL_SERCOM       SERCOM5
#define SERIAL_MUX_SETTINGS USART_RX_3_TX_2_XCK_3

// Interface Serial 1
#define SERIAL1_PAD0            PINMUX_UNUSED
#define SERIAL1_PAD1            PINMUX_UNUSED
#define SERIAL1_PAD2            PINMUX_PA10D_SERCOM2_PAD2
#define SERIAL1_PAD3            PINMUX_PA11D_SERCOM2_PAD3
#define SERIAL1_SERCOM          SERCOM2
#define SERIAL1_MUX_SETTINGS    USART_RX_3_TX_2_XCK_3

// Interface Serial 2
#define SERIAL2_PAD0            PINMUX_PA16C_SERCOM1_PAD0
#define SERIAL2_PAD1            PINMUX_UNUSED
#define SERIAL2_PAD2            PINMUX_PA18C_SERCOM1_PAD2
#define SERIAL2_PAD3            PINMUX_UNUSED
#define SERIAL2_SERCOM          SERCOM1
#define SERIAL2_MUX_SETTINGS    USART_RX_0_TX_2_XCK_3

// Interface Serial 3
#define SERIAL3_PAD0			PINMUX_UNUSED
#define SERIAL3_PAD1            PINMUX_UNUSED
#define SERIAL3_PAD2            PINMUX_PA14C_SERCOM2_PAD2
#define SERIAL3_PAD3            PINMUX_PA15C_SERCOM2_PAD3
#define SERIAL3_SERCOM          SERCOM2
#define SERIAL3_MUX_SETTINGS    USART_RX_3_TX_2_XCK_3

typedef enum {
	SERIAL_PORT1 = 0x00,
	SERIAL_PORT2       ,
	SERIAL_PORT3       ,
	
} __attribute__((packed, aligned(1))) SerialPort_et;

#endif /* CONF_ARDUINOM0PRO_BOARD_H_ */