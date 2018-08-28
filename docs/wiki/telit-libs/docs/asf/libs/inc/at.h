/*
 * at.h
 *
 * Created: 15/03/2018 16:19:38
 *  Author: Jonas
 */ 


#ifndef AT_H_
#define AT_H_


#include <asf.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "serial2.h"
#include "serial3.h"


#define AT_MAX_COMMAND_STRING_SIZE  128

#define AT_TIMEOUT_DEFAULT          (  1000 )
#define AT_TIMEOUT_CONNECTION       ( 15000 )
#define AT_TIMEOUT_SOCKET_SEND      ( 30000 )


typedef enum {
    AT_ST_SND = 0x00  ,
    AT_ST_SND_OK      ,
    AT_ST_RCV_WAITING ,
    AT_ST_RCV_OK      ,
    AT_ST_RCV_OK_PARAM,
    AT_ST_ERROR       ,
    AT_ST_TIMEOUT
    
} __attribute__((packed, aligned(1))) AtState_et;


void AT_Init(const char** atCommands);
void AT_Deinit(void);
AtState_et AT_RequestSerial(SerialPort_et port, uint8_t commandIndex, char* parameters, char* receivedParams, uint16_t receivedParamsMaxLen, uint16_t timeout);
AtState_et AT_Request(uint8_t commandIndex, char* parameters, char* receivedParams, uint16_t receivedParamsMaxLen, uint16_t timeout);
AtState_et AT_SendCommand(SerialPort_et port, uint8_t commandIndex, char* parameters);
AtState_et AT_CommandReceived(SerialPort_et port, void* receivedParams, uint16_t receivedParamsMaxLen, uint16_t *receivedParamsLen, uint16_t timeout);
AtState_et AT_SendData (uint8_t* data, uint16_t dataLen);
void AT_Write(SerialPort_et port, void* data, uint16_t dataLen);
void AT_Println(SerialPort_et port, char* data);
void AT_ReadCallback(uint8_t data, SerialPort_et port);


#endif /* AT_H_ */