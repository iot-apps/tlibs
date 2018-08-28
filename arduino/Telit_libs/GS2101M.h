/**
*
*	@file GS2101M.h
*
*	Esta biblioteca deve ser integrada a IDE arduino para ser utilizada por todos os exemplos do módulo wifi GS2101M.
* 
*	@brief A biblioteca GS2101M implementa funcionalidades para o módulo Telit GS2101M.
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Vinícius Roosvelt.
* 
*	@version 1.00
*/
#ifndef GS2101M_H
#define GS2101M_H

#include <inttypes.h>
#include "AT.h"

class GS2101M {

private:
    AT at;
	int _pin; 

public:

    GS2101M(Uart* uartModule);

	/**
	*	@brief Função Init: Inicializa a comunicação com o módulo wifi a partir do comando basico "at" 
	*	em seguida envia o comando para desabilitar o "ate0" e por último, envia 
	*	o comando para "at+wm = 0" para funcionar como station mode.
	* 
	*	@return  retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" 
	*/
    bool Init(void);

	/**
	*	@brief Função Connect: Realiza a conexão do módulo com um AP(access point). Antes da conexão são 
	*	enviados alguns comandos de configuração: "at+ndhcp=1,ssid,1" para habilitar o dhcp, "at+ wwpa=password" 
	*	informa ao módula qual é o password da rede de conexão e por fim se conecta com o módulo através do comando
	*	"at+wa=ssid". 
	*
	*	@param[out] resp: Preenche um array de char com os dados com os dados da conexão: ip, subnet e gateway.
	*	@param[in] respLen: Determina o tamanho máximo que a resposta pode ter.
	*	@param[in] ssid ID: da rede que se deseja conectar.
	*	@param[in] password: Senha da rede que se deseja conectar.
	*
	*	@return retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" 
	*/
    bool Connect(char* resp, int respLen, const char* ssid, const char* password); 
    
	/**
	*	@brief Função Disconnect: Fecha a conecão do módulo wifi com o AP(acess point) a partir do comando "at+wd".
	*
	*	@return retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" 
	*/
	bool Disconnect(void);
	
	/**
	*	@brief Função TcpConnect: Abre um socket tcp entre um cliente(ex. módulo) e um servidor de destino 
	*	(ex. pc) através do comando "at+nctcp=address, port". 
	*
	*	@param[in] addres: recebe o endereço ip para conexão de destino.
	*	@param[in] port: recebe a porta para conexão de destino.
	*
	*	@return retorna o id da conexão que foi aberta.
	*/
    byte TcpConnect(char* address, int port);
    
	/**
	*	@brief Função BData: Habilita o envio/recepção de dados no formato bulk mode através do comando "at+bdata=in". 
	*
	*	@param[in] in: recebe 0 para desabilitar e 1 para habilitar. 
	*
	*	@return retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" 
	*/
	bool BData(bool in);
	
	/**
	*	@brief Função TcpWrite: Envia um array de char (dados) no formato bulk mode.
	*
	*	@param[in] cid: recebe o id da conexão que foi aberta . 
	*	@param[in] buffer: recebo dados para envio. 
	*
	*/
	void TcpWrite(byte cid, char* buffer);
	
	/**
	*	@brief Função TcpRead: Realiza a leitura de dados recebedos através da conexão tcp. 
	*
	*	@param[in] cid: recebe o id da conexão que foi aberta. 
	*	@param[out] buffer: preeche buffer com os dados lidos via tcp. 
	*	@param[in] maxBufferLen: determina a quantidade máxima de bytes que deve ser lido. 
	*
	*	@return retorna a quantidade de dados recebidos. 
	*/
	int  TcpRead(byte cid, char* buffer, uint16_t maxBufferLen, uint16_t timeout);
    
	bool TcpDisconnect(void);
	
	/**
	*	@brief Função Reset: Realiza o reset do módulo por hadware.
	*
	*	@param[in] pin: recebe o número do pino conectado ao pino de reset do módulo wifi. 
	* 
	*/
	void Reset(int pin);

};

#endif