#ifndef AT_H
#define AT_H

#include <inttypes.h>
#include <Arduino.h>   // required before wiring_private.h


#define CR              13 //Ascii character for carriage return. 
#define LF              10 //Ascii character for line feed. 
#define AT_LIB_VERSION 001 // library version X.YY (e.g. 1.00)

#define SERIAL_MODULE_DEFAULT_BAUDRATE  9600

#define COMM_BUF_LEN        200

#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1

// Common string used
#define STR_OK      		"OK"       //string to reduce stack usage
#define STR_AT      		"AT"       //string to reduce stack usage
#define STR_ERROR			"ERROR"
#define STR_CONNECT 		"CONNECT"
#define STR_SEND_OK			"> "
#define STR_SRING			"SRING: 1"	
#define STR_MSG_RCV			"+CMT:"	
#define STR_GPS_RESP		"$GPGGA"
#define STR_GPS_NOSIGNAL	"$GPSNMUN: $GPGGA,,,,,,0,,,,M,,M"
#define STR_GPS_END_DATA	",M,,M,"

enum rx_state_enum {
     RX_NOT_FINISHED = 0,      // not finished yet
     RX_FINISHED,              // finished, some character was received
     RX_FINISHED_STR_RECV,     // finished and expected string received
     RX_FINISHED_STR_NOT_RECV, // finished, but expected string not received
     RX_TMOUT_ERR,             // finished, no character received
     // initial communication tmout occurred
     RX_LAST_ITEM
};

enum at_resp_enum {
     AT_RESP_ERR_NO_RESP = -1,   // nothing received
     AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
     AT_RESP_OK = 1,             // response_string was included in the response

     AT_RESP_LAST_ITEM
};

class AT {

private:
    Uart *serialModule;

    byte         *p_comm_buf;            // pointer to the communication buffer
    byte          comm_buf_len;          // num. of characters in the buffer
    byte          rx_state;              // internal state of rx state machine
    uint16_t      start_reception_tmout; // max tmout for starting reception
    uint16_t      interchar_tmout;       // previous time in msec.
    unsigned long prev_time;             // previous time in msec.

public:
     byte comm_buf[COMM_BUF_LEN+1];      // communication buffer +1 for 0x00 termination

    AT(Uart *uartModule);

    void Init(void);
    void Deinit(void);

    void Write(const char* command);
    void WriteLn(const char* command);
    int  Read(char* result, uint16_t resultMaxlen, uint16_t timeoutStart, uint16_t timeoutMiddle);

    void RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
    byte IsRxFinished(void);
    byte IsStringReceived(char const *compare_string);

    byte WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
    byte WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
               char const *expected_resp_string);
	char SendATCmdReturnResp(char const *AT_cmd_string,
                            uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                            char *resp,
							uint16_t respLen,
                            byte no_of_attempts);
    char SendATCmdWaitResp(char const *AT_cmd_string,
                        uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                        char const *response_string,
                        byte no_of_attempts);
    char SendATCmdWaitResp(const __FlashStringHelper *AT_cmd_string,
                        uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                        char const *response_string,
                        byte no_of_attempts);

};


#endif