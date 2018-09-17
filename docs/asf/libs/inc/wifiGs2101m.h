/**
*
*	@file wifiGs2101m.h
*
*	Esta biblioteca e utilizada por todos os exemplos do modulo wifi GS2101M.
* 
*	@brief A biblioteca GS2101M implementa funcionalidades para o modulo Telit GS2101M.
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Vinicius Roosvelt.
* 
*	@version 1.00
*/


#ifndef WIFIGS2101M_H_
#define WIFIGS2101M_H_


#include <stdbool.h>
#include <stdint.h>
#include "at.h"

#define WIFI_DEFAULT_SSID   "Lieno"
#define WIFI_DEFAULT_PASS   "belohorizonte"

#define WIFI_SSID_MAX_LEN   32
#define WIFI_PASS_MAX_LEN   32
#define WIFI_IP_MAX_LEN     16

#define WIFI_ADDR_TCP_MAX_LEN    32

#define WIFI_MAX_CIDS       16
#define WIFI_MODE_STATION   "0"
#define WIFI_MODE_AP        "1"

#define MQTT_TOPIC_MAX_LEN     128
#define MQTT_ADDRESS_MAX_LEN   128
#define MQTT_CLIENT_ID_MAX_LEN 64
#define MQTT_MESSAGE_MAX_LEN   256
#define MQTT_MAX_MESSAGES      5

typedef enum
{
    WIFI_NO_ERROR = 0x00,
    WIFI_ERROR_TIMEOUT,
    WIFI_ERROR_INVALID_ARG,
    WIFI_ERROR_UNEXPECTED_RESPONSE,
    WIFI_ERROR_UNEXPECTED_STATE,
    WIFI_ERROR_NOT_INITIALIZED,
    WIFI_ERROR_BUFFER_NULL,
    WIFI_ERROR_BUFFER_OVERFLOW,
    WIFI_ERROR_AUTH,
    WIFI_ERROR_ESP8266,
    WIFI_ERROR_CONNECTION,
    WIFI_ERROR_SENDING,

    WIFI_ERROR_UNKNOWN
} __attribute__((packed, aligned(1))) WifiError_et;


typedef enum {
    WIFI_ST_DEINITIALIZED = 0x00,
    WIFI_ST_INITIALIZING,
    WIFI_ST_ECHO_OFF,
    WIFI_ST_SET_MODE,
    WIFI_ST_INITIALIZED,
    WIFI_ST_DISCONNECTED,
    WIFI_ST_CONNECTING,
    WIFI_ST_DHCP,
    WIFI_ST_WPA,
    WIFI_ST_CONNECT,
    WIFI_ST_CONNECTED,
    WIFI_ST_SLEEPING,
    WIFI_ST_DISCONNECTING,
    WIFI_ST_TCP_CONNECTING,
    WIFI_ST_TCP_BDATA,
    WIFI_ST_TCP_CONNECTED,
    WIFI_ST_TCP_CHECK_CID,
    WIFI_ST_TCP_TRANSMITTING,
    WIFI_ST_TCP_RECEIVING,
    WIFI_ST_ERROR,
    MQTT_ST_CONNECTING,
    MQTT_ST_CONNECT,
    MQTT_ST_CONNECTED,
    MQTT_ST_SUBSCRIBING,
    MQTT_ST_PUBLISHING,
    MQTT_ST_DISCONNECTING,
    MQTT_ST_DISCONNECTED,
    MQTT_ST_ERROR,
    
    WIFI_ST_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) WifiState_et;

typedef struct
{
    uint8_t    cid;
    const char topic[MQTT_TOPIC_MAX_LEN];
    uint32_t   len;
    uint8_t    data[MQTT_MESSAGE_MAX_LEN];
    uint8_t    qos;
    bool       retain;

} MqttMessage_st;

typedef struct
{
    uint8_t cid; 
    const char topic[MQTT_TOPIC_MAX_LEN];
    uint8_t qos;
    uint32_t timeout;

} MqttTopic_st;

	/**
	*	@brief Funcao WIFI_Init: Inicializa a comunicacao com o modulo wifi a partir do comando basico "at" 
	*	em seguida envia o comando para desabilitar o "ate0" e por ultimo, envia 
	*	o comando para "at+wm = 0" para funcionar como station mode.
	* 
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_Init(void);
	/**
	*	@brief Funcao WIFI_Deinit: Desinacializa o modulo.
	* 
	*/
	void WIFI_Deinit(void);
//bool WIFI_Config(St_WifiConfig* config);
	/**
	*	@brief Funcao WIFI_ResetModule: Reinicia o modulo.
	* 
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_ResetModule(void);
	
	bool WIFI_SetSleep(bool sleep);
	/**
	*	@brief  Retorna o estado atual do gerenciamento do modulo.
	* 
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	WifiState_et WIFI_GetState(void);
	/**
	*	@brief Funcao Connect: Realiza a conexao do modulo com um AP(access point). Antes da conexao sao 
	*	enviados alguns comandos de configuracao: "at+ndhcp=1,ssid,1" para habilitar o dhcp, "at+ wwpa=password" 
	*	informa ao modula qual e o password da rede de conexao e por fim se conecta com o modulo atraves do comando
	*	"at+wa=ssid". 
	*
	*	@param[in] ssid: da rede que se deseja conectar.
	*	@param[in] password: Senha da rede que se deseja conectar.
	*
	*	@return retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_Connect(const char* ssid, const char* pass);
	/**
	*	@brief  Funcao WIFI_IsConnected: Verifica se o modulo esta conectado a rede wifi.
	* 
	*	@return  retorna 1 se se estiver conectado a rede e 0 se nao.
	*/
	bool WIFI_IsConnected(void);
	/**
	*	@brief  Funcao WIFI_GetIp: Obtem o endereco de IP Local.
	* 
	*	@param[out] ip: Retorna o endereco de IP
	*
	*	@return  Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error". 
	*/
	bool WIFI_GetIp(char *ip);
	/**
	*	@brief Funcao WIFI_Disconnect: Fecha a conexao do modulo wifi com o AP(acess point) a partir do comando "at+wd".
	*
	*/
	void WIFI_Disconnect(void);
	/**
	*	@brief Funcao WIFI_TcpConnect: Abre um socket tcp entre um cliente(ex. modulo) e um servidor de destino 
	*	(ex. pc) atraves do comando "at+nctcp=address, port". 
	*
	*	@param[in] addres: recebe o endereco ip para conexao de destino.
	*	@param[in] port: recebe a porta para conexao de destino.
	*
	*	@return Retorna o id da conexao que foi aberta (CID).
	*/
	uint8_t WIFI_TcpConnect(const char* address, uint16_t port);

 	/**
	*	@brief  Funcao WIFI_TcpGetCids: Indentifica a mascara dos CIDs (connection ID) das conexoes ativas
	*
	*	@return  Retorna a mascara dos CIDs (connection ID) das conexoes ativas 
	*/
	uint16_t  WIFI_TcpGetCids(void);
		/**
	*	@brief Funcao TcpWrite: Envia dados via TCP para o host remoto
	*
	*	@param[in] cid: envia o id da conexao que foi aberta . 
	*	@param[in] data: recebo dados para envio. 
	*	@param[in] dataLen: envia o tamanho da sting de dados
	*
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_TcpWrite(uint8_t cid, uint8_t* data, uint16_t dataLen);
	/**
	*	@brief Funcao WIFI_TcpDisconnect: Desconecta de uma conexao TCP. 
	*
	*	@param[in] cid: recebe o CID (connection ID) da conexao a ser finalizada
	*
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_TcpDisconnect (uint8_t cid);
	/**
	*	@brief Funcao WIFI_HandleReceivedData: Trata os dados TCP recebidos. 
	*
	*	@param[in] data: Recebe os dados proveniente da conexao TCP.
	*
	*	@return  retorna 1 se receber resposta "ok" do modulo e 0 se receber "error" 
	*/
	bool WIFI_HandleReceivedData(uint8_t data);
	/**
	*	@brief Funcao WIFI_TcpReadCallback: Trata a recepcao dos dados TCP.
	*
	*	@param[in] data: dado. 
	*	
	*	@param[in] SerialPort_et port: porta de comunicacao. 
	* 
	*/
	void WIFI_TcpReadCallback(uint8_t data, SerialPort_et port); // Deve ser implementado na aplicação.

/**
 * MQTT Functions
 */
 
bool         MQTT_Init(void);
void         MQTT_Deinit(void);
WifiState_et MQTT_GetState(void);
bool         MQTT_IsConnected(void);
uint8_t      MQTT_Connect(const char* address, uint16_t port, const char* clientId, const char* usr, const char* pwd);
bool         MQTT_Publish(uint8_t cid, const char* topic, uint32_t len, uint8_t *data, uint8_t qos, bool retain);
bool         MQTT_Write(uint8_t cid, uint8_t* data, uint16_t dataLen);
bool         MQTT_Subscribe(uint8_t cid, const char* topic, uint8_t qos, uint16_t timeout);
bool         MQTT_Ping(uint8_t cid, uint16_t timeout);
void         MQTT_Disconnect(uint8_t cid);

void MQTT_ReadCallback(uint8_t data); // Deve ser implementado na aplicação.


#endif /* WIFIGS2101M_H_ */