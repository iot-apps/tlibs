/**
*
*	@file BMS42.h
*
*	Esta biblioteca deve ser integrada a IDE arduino para ser utilizada por todos os exemplos dos módulos BlueMode+S42.
* 
*	@brief A biblioteca BMS42 implementa um grande número de funcionalidades para os módulos Telit da família BlueMode+S.
* 
*	@author Lucas S. Feitosa.
* 
*	@version 1.00
*/

#ifndef BMS42_H
#define BMS42_H

#include <inttypes.h>
#include "AT.h"

class BMS42 {

private:
    AT at;
	int _pin; 

public:

	/**
    * Construtor que configura um objeto da classe Uart. 
    *        
    */
    BMS42(Uart* uartModule);
	
	/**
	*	@brief Função Reboot: Reinicializa o módulo.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool Reboot(void);
	/**
	*	@brief Função Reset: Realiza o reset do módulo por hadware.
	*
	*	@param[in]	pin Recebe o número do pino conectado ao pino de reset do módulo gsm.
	*/
	void Reset(int pin);
	/**
	*	@brief Função BtMode: Define o mode de execução do bluetooh.
	*
	*	@param[in]	mode 0-recebe conexões (bluetooth visível), 1-apenas realiza (bluetooth invisivel), 2-recebe e realiza conexões btl (bluetooth visivel)
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool BtMode(int mode);
	/**
	*	@brief Função BtConnect: Indentifica a realização de uma conexão bluetooth.
	*
	*	@return Retorna 1 se receber ocorrer uma conexão e 0 se não ocorrer.
	*/
	bool BtConnect(void);
	/**
	*	@brief Função BtWrite: Configura o módulo para envia de menssagens de texto.
	*
	*	@param[in]	buffer Recebe um array de char contendo uma menssagem.
	*		
	*/
	void BtWrite(char* buffer);
	/**
	*	@brief Função MessageRead: Lê menssagens de texto recebidas pelo módulo.
	*
	*	@param[out]	buffer: Preenche um array de char com a mensagem recebida pelo módulo.
	*	@param[in]  maxBufferLen: determina a quantidade máxima de bytes que deve ser lido.
	*	@param[in]	timeout: Recebe o tempo espera para o comando de resposta enviado pelo módulo.
	*		
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	int  BtRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout);
	
};

#endif