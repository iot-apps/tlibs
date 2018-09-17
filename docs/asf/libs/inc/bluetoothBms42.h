/**
*
*	@file bluetoothBms42.h
*
*	Esta biblioteca e utilizada por todos os exemplos do modulo BlueMode+S42.
* 
*	@brief A biblioteca XE910 implementa funcionalidades para os modulos Telit da familia BlueMode+S.
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Thales.
* 
*	@version 1.00
*/


#ifndef BLUETOOTHBMS42_H_
#define BLUETOOTHBMS42_H_


#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    BT_NO_ERROR = 0x00,
    BT_ERROR_TIMEOUT,
    BT_ERROR_INVALID_ARG,
    BT_ERROR_UNEXPECTED_RESPONSE,
    BT_ERROR_UNEXPECTED_STATE,
    BT_ERROR_NOT_INITIALIZED,
    BT_ERROR_BUFFER_NULL,
    BT_ERROR_BUFFER_OVERFLOW,
    BT_ERROR_CONNECTION,
    BT_ERROR_SENDING,

    BT_ERROR_UNKNOWN
} __attribute__((packed, aligned(1))) BtError_et;


typedef enum {
    BT_ST_DEINITIALIZED = 0x00,
    BT_ST_INITIALIZING,
    BT_ST_ECHO_OFF,
    BT_ST_SET_MODE,
    BT_ST_INITIALIZED,
    BT_ST_DISCONNECTED,
    BT_ST_CONNECTING,
    BT_ST_CONNECT,
    BT_ST_CONNECTED,
    BT_ST_SLEEPING,
    BT_ST_DISCONNECTING,
    BT_ST_TCP_CONNECTING,
    BT_ST_TCP_BDATA,
    BT_ST_TCP_CONNECTED,
    BT_ST_TCP_CHECK_CID,
    BT_ST_TCP_TRANSMITTING,
    BT_ST_TCP_RECEIVING,
    BT_ST_ERROR,

    BT_ST_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) BtState_et;

	/**
	*	@brief Funcao BT_Init: Inicializa o modulo Wifi e a task de gerenciamento.
	*
	*	@param[in]	mode: define o modo de funcionamento 0-recebe conexoes (bluetooth visivel), 1-apenas realiza (bluetooth invisivel), 2-recebe e realiza conexoes btl (bluetooth visivel)
	*
	*	@return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
	*/
	bool       BT_Init(uint8_t mode);
	/**
	*	@brief Funcao BT_Deinit: Desinicializa modulo.
	*
	*/
	void       BT_Deinit(void);
		/**
	*	@brief Funcao BT_ResetModule: Reinicia o Modulo.
	*
	*	@return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
	*/
	bool       BT_ResetModule(void);
	
	bool       BT_SetSleep(bool sleep);
	/**
	*	@brief Funcao BT_ResetModule: Reinicia o Modulo.
	*
	*	@return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
	*/
	BtState_et BT_GetState(void);
	/**
	*	@brief Funcao BT_Connect: Indentifica a realizacao de uma conexao bluetooth.
	*
	*	@return Retorna 1 se receber ocorrer uma conexao e 0 se nao ocorrer.
	*/
	bool       BT_Connect(void);
	bool       BT_IsConnected(void);
	bool       BT_GetIp(char *ip);
	/**
	*	@brief Funcao BT_Disconnect: Desconecta o bluetooth.
	*
	*/
	void       BT_Disconnect(void);
		/**
	*	@brief Funcao BtWrite: Configura o modulo para envia de menssagens de texto.
	*
	*	@param[in]	data: Recebe um array de char contendo uma menssagem.
	*	@param[in]	dataLen: Recebe o tamanho do array de dados
	*	
	*	@return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
	*/
	bool       BT_Write(uint8_t* data, uint16_t dataLen);

	void       BT_ReadCallback(uint8_t data, SerialPort_et port); // Deve ser implementado na aplicação.

#endif /* BLUETOOTHBMS42_H_ */