#include "AT.h"


AT::AT(Uart *uartModule) {

     serialModule = uartModule;
};


void AT::Init(void) {
}


void AT::Write(const char* command) {
     serialModule->print(command);
}

void AT::WriteLn(const char* command) {
     serialModule->println(command);
}


int AT::Read(char* result, uint16_t resultMaxlen, uint16_t timeoutStart, uint16_t timeoutMiddle) {
     uint16_t resultIndex = 0;
     uint16_t timeout     = timeoutStart;

     while( (serialModule->available() < 1) && (timeout > 0) ) {
          delay(1);
		  timeout--;
     }

     if (timeout > 0) {

          while ( resultIndex < (resultMaxlen - 1) ) {

               timeout = timeoutMiddle;
               while( (serialModule->available() < 1) && (timeout > 0) ) {
                    delay(1);
					timeout--;
               }

               if (timeout > 0) {
                    result[resultIndex] = serialModule->read();

                    if (result[resultIndex] > 0) {
                         resultIndex++;
                    }
               } else {
                    break;
               }
          }
     }
     
     result[resultIndex] = '\0';

     return resultIndex;
}

byte AT::IsRxFinished(void) {
     byte num_of_bytes;
     byte ret_val = RX_NOT_FINISHED;  // default not finished

     if (rx_state == RX_NOT_STARTED) {

          if (!serialModule->available()) {
               if ((unsigned long)(millis() - prev_time) >= start_reception_tmout) {
                    comm_buf[comm_buf_len] = 0x00;
                    ret_val                = RX_TMOUT_ERR;
               }
          } else {
               prev_time = millis(); // init tmout for inter-character space
               rx_state  = RX_ALREADY_STARTED;
          }
     }

     if (rx_state == RX_ALREADY_STARTED) {
          num_of_bytes = serialModule->available();

          if (num_of_bytes) prev_time = millis();

          while (num_of_bytes) {
               num_of_bytes--;
               if (comm_buf_len < COMM_BUF_LEN) {
                    *p_comm_buf = serialModule->read();

                    p_comm_buf++;
                    comm_buf_len++;
                    comm_buf[comm_buf_len] = 0x00;  // and finish currently received characters
               } else {
                    serialModule->read();
               }
          }

          if ((unsigned long)(millis() - prev_time) >= interchar_tmout) {
               comm_buf[comm_buf_len] = 0x00;  // for sure finish string again

               ret_val = RX_FINISHED;
          }
     }


     return (ret_val);
}

byte AT::IsStringReceived(char const *compare_string)
{
     char *ch;
     byte ret_val = 0;

     if(comm_buf_len) {
          ch = strstr((char *)comm_buf, compare_string);
          if (ch != NULL) {
               ret_val = 1;
          } else {

          }
     } else {

     }

     return (ret_val);
}


void AT::RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
     rx_state              = RX_NOT_STARTED;
     start_reception_tmout = start_comm_tmout;
     interchar_tmout       = max_interchar_tmout;
     prev_time             = millis();
     comm_buf[0]           = 0x00; // end of string
     p_comm_buf            = &comm_buf[0];
     comm_buf_len          = 0;
     serialModule->flush(); // erase rx circular buffer
}


byte AT::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                   char const *expected_resp_string)
{
     byte status;
     byte ret_val;

     RxInit(start_comm_tmout, max_interchar_tmout);

     do {
          status = IsRxFinished();
     } while (status == RX_NOT_FINISHED);

     if (status == RX_FINISHED) {
          if(IsStringReceived(expected_resp_string)) {
               ret_val = RX_FINISHED_STR_RECV;
          } else {
               ret_val = RX_FINISHED_STR_NOT_RECV;
          }
     } else {
          ret_val = RX_TMOUT_ERR;
     }

     return (ret_val);
}


char AT::SendATCmdReturnResp(char const *AT_cmd_string,
                            uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                            char *resp,
							uint16_t respLen,
                            byte no_of_attempts) {
    char ret_val = AT_RESP_ERR_NO_RESP;

	for (int tries = 0; tries < no_of_attempts; tries++) {

		WriteLn(AT_cmd_string);

		if ( Read(resp, respLen, start_comm_tmout, max_interchar_tmout) > 0 ) {
			ret_val = AT_RESP_OK;
			break;
		}
	}

	return ret_val;
}


char AT::SendATCmdWaitResp(char const *AT_cmd_string,
                            uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                            char const *response_string,
                            byte no_of_attempts)
{
     byte status;
     char ret_val = AT_RESP_ERR_NO_RESP;
     byte i;

     for (i = 0; i < no_of_attempts; i++) {
          if (i > 0) delay(500);

          serialModule->println(AT_cmd_string);
          status = WaitResp(start_comm_tmout, max_interchar_tmout);

          if (status == RX_FINISHED) {
               if(IsStringReceived(response_string)) {
                    ret_val = AT_RESP_OK;
                    break;  // response is OK => finish
               } else ret_val = AT_RESP_ERR_DIF_RESP;
          } else {
               ret_val = AT_RESP_ERR_NO_RESP;
          }
     }

     WaitResp(1000, 5000);
     return (ret_val);
}


char AT::SendATCmdWaitResp(const __FlashStringHelper *AT_cmd_string,
                            uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                            char const *response_string,
                            byte no_of_attempts)
{
     byte status;
     char ret_val = AT_RESP_ERR_NO_RESP;
     byte i;

     for (i = 0; i < no_of_attempts; i++) {
          if (i > 0) delay(500);

          serialModule->println(AT_cmd_string);
          status = WaitResp(start_comm_tmout, max_interchar_tmout);
          if (status == RX_FINISHED) {
               if(IsStringReceived(response_string)) {
                    ret_val = AT_RESP_OK;
                    break;  // response is OK => finish
               } else ret_val = AT_RESP_ERR_DIF_RESP;
          } else {
               ret_val = AT_RESP_ERR_NO_RESP;
          }

     }

     return (ret_val);
}

byte AT::WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
     byte status;
	uint16_t myTimeout = start_comm_tmout;
	 
	 
     RxInit(start_comm_tmout, max_interchar_tmout);

     do {
          status = IsRxFinished();
		  delay(1);
		  myTimeout--;
     } while ( (status == RX_NOT_FINISHED) && (myTimeout > 0) );
	 
     return (status);
}
