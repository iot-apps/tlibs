/*
*
*	@file BMS42.cpp
*
*	Esta biblioteca deve ser integrada a IDE arduino para ser utilizada por todos os exemplos dos módulos BlueMode+S42.
* 
*	@brief A biblioteca BMS42 implementa um grande número de funcionalidades para os módulos Telit da família BlueMode+S.
* 
*	@author Lucas S. Feitosa.
* 
*	@version 1.00
*/


#include "BMS42.h"

#include <string.h>


#define NUM_TRIES   3

#define CMD_BT_REBOOT				"AT+RESET"

#define CMD_BT_CONFIG_MODE			"AT+LEROLE"

BMS42::BMS42(Uart* uartModule):at(uartModule) {


}

/**
*	@brief Função Reboot: Reinicializa o módulo.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool BMS42::Reboot(void){
	
	bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_BT_REBOOT);
    
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
    
	return result;
}

bool BMS42::BtMode(int mode){
	bool result = false;
	char command[64] = {0};
	char num[2] = {0};
	num[1] = '\0';
	num[0] = mode + 48;
	strcpy(command, CMD_BT_CONFIG_MODE);
    strcat(command, "=");
	strcat(command, num);
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
	return result;
}

bool BMS42::BtConnect(void){
    bool result = false;
	char resp[200] = {0};

    if ( (at.Read(resp, 200, 10000, 100)) > 0 ) {
		if ( NULL != strstr(resp, STR_CONNECT) ) {
			result = true;
		}
	}
	return result;
}	

void BMS42::BtWrite(char* buffer) {
    
	char toSend[200] = {0};
	
	//sprintf(toSend,"\x1bZ%d%04d%s", strlen(buffer), buffer);
	//at.WriteLn(toSend);
	at.WriteLn(buffer);
}

int  BMS42::BtRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout) {
    char     resp[200] = {0};
    uint16_t respLen   =  0;

    if ( (respLen = at.Read(buffer, maxBufferLen, timeout, 100)) >= 7 ) {
		}
	
    return respLen;
}

void BMS42::Reset(int pin){
	pinMode(pin, OUTPUT);
	
	digitalWrite(pin, LOW);
    delay(1000); 
    digitalWrite(pin, HIGH); 
}