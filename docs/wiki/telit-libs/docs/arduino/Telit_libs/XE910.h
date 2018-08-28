/**
*
*	@file XE910.h
*
*	Esta biblioteca deve ser integrada a IDE arduino para ser utilizada por todos os exemplos dos módulos x910.
* 
*	@brief A biblioteca XE910 implementa um grande número de funcionalidades para os módulos Telit da família x910.
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Vinícius Roosvelt.
* 
*	@version 1.00
*/

#ifndef XE910_H
#define XE910_H

#include <inttypes.h>
#include "AT.h"

class XE910 {

private:
    AT at;
	int _pin; 

public:
       
	/**
    * Construtor que configura um objeto da classe Uart.
    * 
    * @param  
    *        
    */
    XE910(Uart* uartModule);

	/**
	*	@brief Função Init: Inicializa a comunicação com o módulo gsm a partir do comando
	*	basico "at" em seguida envia o comando para desabilitar o "ate0" e por último, envia
	*	o comando para AT+CGDCONT= 1,"IP","Access_Point_Name" para configurar a conexão com a
	*	operadora.
	*
	*	@param[in]	operadora Recebe o APN da operadora telefônica.
	*	@param[in]	ip Recebe o tipo de conexão.
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
    bool Init(const char* operadora, const char* ip);

	/**
	*	@brief Função Connect: Realiza a conexão do módulo com a operadora celular através do comando
	*	"AT#SGACT=1,1".
	*
	*	@param[out]	resp Preenche um array de char com o endereço ip fornecido pela operadora.
	*	@param[in]	respLen Determina o tamanho máximo que a resposta pode ter.
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
    bool Connect(char* resp, int respLen); 
    
	/**
	*	@brief Função Disconnect: Desconecta o módulo com a operadora celular através do comando
	*	"AT#SGACT=1,0".
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool Disconnect(void);
	
	/**
	*	@brief Função SocketConnect: Abre um socket entre um cliente(ex. módulo) e um servidor de destino
	*	(ex. pc) através do comando AT#SD=1,0,port,"server_address",0,0,1.
	*
	*	@param[in]	address Recebe o endereço ip para conexão de destino.
	*	@param[in]	port Recebe a porta para conexão de destino.
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
    bool SocketConnect(char* address, int port);
		
	/**
	*	@brief Função SocketWrite: Envia um array de char (dados) por socket através do comando
	*	"AT#SSEND=1".
	*	
	*	@param[in]	buffer recebe um array de dados para envio por socket.
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool SocketWrite(char* buffer);
	
	/**
	*	@brief Descrição: Função GetIP: Obtem o endereço IP, caso o já esteja conectado.
	*
	*	@param[out]	address Preenche um array de char com o endereço ip fornecido pela operadora.
	*	@return Retorna 1 se receber resposta "ok"(tem IP) do módulo e 0 se receber "error"(não tem IP).
	*/	
	bool GetIP(char* address);
	
	/**
	*	@brief Função SocketRead: Realiza a leitura de dados recebedos através da conexão por socket a
	*	partir do comando "AT#SRECV=1,<tamanho_do_dado>".
	*
	*	@param[out]	buffer Preeche um buffer com os dados recebidos por socket.
	*	@param[in]	maxBufferLen Determina a quantidade máxima de bytes que deve ser lido.
	*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
	*	@return Retorna a quantidade de dados recebidos.
	*/
	int  SocketRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout );
		
	/**
	*	@brief Função SocketDisconnect: Disconecta uma conexão aberta por socket através do comando
	*	"AT#SH=1".
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool SocketDisconnect(void);
	
	/**
	*	@brief Função Reset: Realiza o reset do módulo por hadware.
	*
	*	@param[in]	pin Recebe o número do pino conectado ao pino de reset do módulo gsm.
	*/
	void Reset(int pin);
	
	/**
	*	@brief Função Reboot: Reinicializa o módulo.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/	
	bool Reboot(void);
	
	/**
	*	@brief Função CloudConnect: Realiza uma conexão M2M através do comando de configuração
	*	AT#DWCFG=<api server>,0,<token da aplicação> e do comando de conexão AT#DWCONN=1.
	*
	*	@param[in]	apiServer Recebe o endereço da aplicação.
	*	@param[in]	token Recebe o token da aplicação.
	*	
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool CloudConnect(char* apiServer, char* appToken);
	
	/**
	*	@brief Descrição: Função Send2Cloud: Realiza a publicação de uma propriedade de uma variável e seu valor.
	*
	*	@param[in]	variable Recebe o nome da variável.
	*	@param[in]	value Recebe o valor da variável.
	*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
	*	
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool Send2Cloud(char* variable, char* value, uint16_t timeout);
	
	/**
	*	@brief Função CloudDisconnect: Disconecta do servidor de aplicação.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool CloudDisconnect(void);
	
	/**
	*	@brief Função ConfigureSMS:Configura o módulo para envia de menssagens de texto.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool ConfigureSMS(void);
	
	/**
	*	@brief Função MessageWrite: Configura o módulo para envia de menssagens de texto.
	*
	*	@param[in]	number Recebe o número do celular.
	*	@param[in]	buffer Recebe um array de char contendo uma menssagem.
	*		
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool MessageWrite(const char* number, char* buffer);

	/**
	*	@brief Função MessageRead: Lê menssagens de texto recebidas pelo módulo.
	*
	*	@param[out]	buffer Preenche um array de char com a mensagem recebida pelo módulo.
	*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
	*		
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool  MessageRead(char* buffer, uint16_t timeout );
    
	/**
	*	@brief Função ConfigureGPS: Inicializa o módulo GPS com a configuração padrão.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool ConfigureGPS(void);

	/**
	*	@brief Função StartGPS: Ativar mensagens não solicitadas de dados de GPS no formato NMEA.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool StartGPS(void);
	
	/**
	*	@brief Função GetFixGPS: Captura os dados GPS no formato NMEA. 
	*
	*	@param[out]	time Preenche um array de char com a hora recebida pelo módulo GPS. 
	*	@param[out]	latitude Atualiza a variável latitude com a latitude.
	*	@param[out]	longitude Atualiza a variável longitude com a longitude.
	*	@param[out]	satelites Preenche um array de char com a quantidade de satelites que o módulo GPS encontrou.
	*	@param[out]	altitude Preenche um array de char com a altitude recebida pelo módulo GPS.
	*	
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool GetFixGPS(char* time, double* latitude, double* longitude, char* satelites, char* altitude);
	
	/**
	*	@brief Função StopGPS: Desativar mensagens não solicitadas de dados de GPS no formato NMEA.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool StopGPS(void);

	/**
	*	@brief Função ConfigureGSM: Inicializa comunicação com módulo GSM. 
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool ConfigureGSM(void);

	/**
	*	@brief Função DoACall: Realiza uma chamada telefônica para o número passado como parâmetro.
	*
	*	@param[in]	number Recebe um numera para chamada telefônica no formato +xx()xxxx-xxxxx.
	*	
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool DoACall(char* number);

	/**
	*	@brief Função CloseCall: Finaliza uma chamada telefônica. 
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool CloseCall(void);

	/**
	*	@brief Função GetFixLBS: Obtem a localização por LBS (triangulação por rede de celular).
	*
	*	@param[out]	latitude Atualiza a variável latitude com a latitude.
	*	@param[out]	longitude Atualiza a variável longitude com a longitude.
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool GetFixLBS(double* latitude, double* longitude);
	
	/**
	*	@brief Função Fix2Cloud: Realiza a publicação, por MQTT, de uma coordenada e seu valor.
	*
	*	@param[in]	coordenada Recebe o nome da coordenada.
	*	@param[in]	valor Recebe o valor da coordenada.
	*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
	*	
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool Fix2Cloud(char* coordenada, char* valor, uint16_t timeout);
	/**
	*	@brief Função ReceiveCall: Atende uma chamada telefônica. 
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool ReceiveCall(void);
	/**
	*	@brief Função PlayAudio: Executa o arquivo de audio armazenado previamente na memoria do modem. 
	*
	*	@param[in] file: Recebe o nome do arquivo de audio a ser executado no formato "audio.wav"
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/	
	bool PlayAudio(char* file);
		/**
	*	@brief Função ConfigureDTMF: Habilita o DTMF decoder (habilita a leitura dos digitos enviado pelo teclado numérico)
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/
	bool ConfigureDTMF(void);
	/**
	*	@brief Função RingAutoAnswer: Habilita o auto atendimento de uma chamada telefonica;
	*
	*	@param[in] nRING: Recebe um numero inteiro para a definição de quantidade de "RINGs" antes de responder a chamada,
	*	caso o valor for 0, o auto atendimento é desativado
	*
	*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
	*/	
	bool RingAutoAnswer(int nRING);
	
	
};

#endif