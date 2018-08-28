/**
*
*	@file gsmXE910.h
*
*	Esta biblioteca e utilizada por todos os exemplos dos modulos UE910G/HE910.
* 
*	@brief A biblioteca XE910 implementa funcionalidades para os modulos Telit da familia x910.
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Thales.
* 
*	@version 1.00
*/


#ifndef GSMXE910_H_
#define GSMXE910_H_


#include <stdbool.h>
#include <stdint.h>

#define GSM_IP_MAX_LEN        16
#define GSM_APN_MAX_LEN       32
#define GSM_PROTO_MAX_LEN     16
#define GSM_TCP_ADDR_MAX_LEN  32
#define GSM_API_KEY_MAX_LEN   32
#define GSM_API_VALUE_MAX_LEN 32
#define GSM_PHONE_MAX_LEN     16
#define STR_SEND_OK          "> "
#define STR_GPS_RESP         "$GPSNMUN: $GPGGA"
#define STR_GPS_NOSIGNAL     "$GPSNMUN: $GPGGA,,,,,,0,,,,M,,M"
#define STR_GPS_END_DATA     ",M,,M,"
#define STR_MSG_RCV          "+CMT:"



typedef enum
{
    GSM_NO_ERROR = 0x00,
    GSM_ERROR_TIMEOUT,
    GSM_ERROR_INVALID_ARG,
    GSM_ERROR_UNEXPECTED_RESPONSE,
    GSM_ERROR_UNEXPECTED_STATE,
    GSM_ERROR_NOT_INITIALIZED,
    GSM_ERROR_BUFFER_NULL,
    GSM_ERROR_BUFFER_OVERFLOW,
    GSM_ERROR_AUTH,
    GSM_ERROR_ESP8266,
    GSM_ERROR_CONNECTION,
    GSM_ERROR_SENDING,

    GSM_ERROR_UNKNOWN
} __attribute__((packed, aligned(1))) GsmError_et;


typedef enum {
    GSM_ST_DEINITIALIZED = 0x00,
    GSM_ST_INITIALIZING,
    GSM_ST_ECHO_OFF,
    GSM_ST_REGISTER,
    GSM_ST_INITIALIZED,
    GSM_ST_DISCONNECTED,
    GSM_ST_CONNECTING,
    GSM_ST_DHCP,
    GSM_ST_WPA,
    GSM_ST_CONNECT,
    GSM_ST_CONNECTED,
    GSM_ST_SLEEPING,
    GSM_ST_DISCONNECTING,
    GSM_ST_TCP_CONNECTING,
    GSM_ST_API_CONNECTING,
    GSM_ST_TCP_BDATA,
    GSM_ST_TCP_CONNECTED,
    GSM_ST_API_CONNECTED,
    GSM_ST_API_SENDING,
    GSM_ST_TCP_CHECK_CID,
    GSM_ST_TCP_TRANSMITTING,
    GSM_ST_TCP_DISCONNECTING,
    GSM_ST_API_DISCONNECTING,
    GSM_ST_TCP_RECEIVING,
    GSM_ST_SMS_CFG_CSMS,
    GSM_ST_SMS_CFG_TEXT,
    GSM_ST_SMS_CFG_CNMI,
    GSM_ST_SMS_CONFIGURED,
    GSM_ST_SMS_MSG_WRITE,
    GSM_ST_GPS_CFG_RST,
    GSM_ST_GPS_CFG_ClR,
    GSM_ST_GPS_CFG_ACP,
    GSM_ST_GPS_CFG_START,
    GSM_ST_GPS_CONFIGURED,
    GSM_ST_GPS_STARTING,
    GSM_ST_GPS_STARTED,
    GSM_ST_GPS_STOPPING,
    GSM_ST_CALL,
    GSM_ST_ON_CALL,
    GSM_ST_CLOSE_CALL,
    GSM_ST_ERROR,
    
    GSM_ST_UNKNOWN = 0xff
} __attribute__((packed, aligned(1))) GsmState_et;

/**
*   @brief Funcao Init: Inicializa a comunicacao com o modulo gsm a partir do comando
*   basico "at" em seguida envia o comando para desabilitar o "ate0" e por ultimo, envia
*   o comando para AT+CGDCONT= 1,"IP","Access_Point_Name" para configurar a conexao com a
*   operadora.
*
*   @param[in]  operadora Recebe o APN da operadora telefonica.
*   @param[in]  ip Recebe o tipo de conexao.
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_Init(const char* operadora, const char* ip);

/**
*   @brief Funcao Connect: Realiza a conexao do modulo com a operadora celular atraves do comando
*   "AT#SGACT=1,1".
*
*   @param[out] resp Preenche um array de char com o endereco ip fornecido pela operadora.
*   @param[in]  respLen Determina o tamanho maximo que a resposta pode ter.
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_Connect(void); 

/**
*   @brief Funcao IsConnected: Indica se o modulo esta conectado
*
*   @return Retorna 1 se estiver conectado e 0 caso contrario".
*/
bool GSM_IsConnected(void);

/**
*   @brief Funcao Disconnect: Desconecta o modulo com a operadora celular atraves do comando
*   "AT#SGACT=1,0".
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_Disconnect(void);

/**
*   @brief Funcao SocketConnect: Abre um socket entre um cliente(ex. modulo) e um servidor de destino
*   (ex. pc) atraves do comando AT#SD=1,0,port,"server_address",0,0,1.
*
*   @param[in]  address Recebe o endereco ip para conexao de destino.
*   @param[in]  port Recebe a porta para conexao de destino.
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_SocketConnect(char* address, int port);
    
/**
*   @brief Funcao SocketWrite: Envia um array de char (dados) por socket atraves do comando
*   "AT#SSEND=1".
*   
*   @param[in]  buffer recebe um array de dados para envio por socket.
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_SocketWrite(char* buffer,  uint16_t bufferLen);

/**
*   @brief Funcao GetIP: Obtem o endereco IP, caso o ja esteja conectado.
*
*   @param[out] address Preenche um array de char com o endereco ip fornecido pela operadora.
*   @return Retorna 1 se receber resposta "ok"(tem IP) do modulo e 0 se receber "error"(nao tem IP).
*/  
bool GSM_GetIP(char* address);

/**
*   @brief Funcao SocketRead: Realiza a leitura de dados recebedos atraves da conexao por socket a
*   partir do comando "AT#SRECV=1,<tamanho_do_dado>".
*
*   @param[out] buffer Preeche um buffer com os dados recebidos por socket.
*   @param[in]  maxBufferLen Determina a quantidade maxima de bytes que deve ser lido.
*   @param[in]  timeout Recebe o tempo espera para o comando de resposta enviado pelo modulo.
*   @return Retorna a quantidade de dados recebidos.
*/
int  GSM_SocketRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout );
    
/**
*   @brief Funcao SocketDisconnect: Disconecta uma conexao aberta por socket atraves do comando
*   "AT#SH=1".
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_SocketDisconnect(void);

/**
*   @brief Funcao Reset: Realiza o reset do modulo por hadware.
*
*   @param[in]  pin: Recebe o numero do pino conectado ao pino de reset do modulo gsm.
*/
void GSM_Reset(int pin);

/**
*   @brief Funcao Reboot: Reinicializa o modulo.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/  
bool GSM_Reboot(void);

/**
*   @brief Funcao CloudConnect: Realiza uma conexao M2M atraves do comando de configuracao
*   AT#DWCFG=<api server>,0,<token da aplicacao> e do comando de conexao AT#DWCONN=1.
*
*   @param[in]  apiServer: Recebe o endereco da aplicacao.
*   @param[in]  token: Recebe o token da aplicacao.
*   
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_CloudConnect(char* apiServer, char* appToken);

/**
*   @brief Funcao Send2Cloud: Realiza a publicacao de uma propriedade de uma variavel e seu valor.
*
*   @param[in]  variable: Recebe o nome da variavel.
*   @param[in]  value: Recebe o valor da variavel.
*   @param[in]  timeout: Recebe o tempo espera para o comando de resposta enviado pelo modulo.
*   
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_Send2Cloud(char* variable, char* value, uint16_t timeout);

/**
*   @brief Funcao CloudDisconnect: Disconecta do servidor de aplicacao.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_CloudDisconnect(void);

/**
*   @brief Funcao ConfigureSMS:Configura o modulo para envia de menssagens de texto.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_ConfigureSMS(void);

/**
*   @brief Funcao MessageWrite: Configura o modulo para envia de menssagens de texto.
*
*   @param[in]  number: Recebe o numero do celular.
*   @param[in]  buffer: Recebe um array de char contendo uma menssagem.
*       
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_MessageWrite(const char* number, char* buffer);

/**
*   @brief Funcao MessageRead: Le menssagens de texto recebidas pelo modulo.
*
*   @param[out] receive: Preenche um array de char com a mensagem recebida pelo modulo.
*   @param[in]  buffer: de entrada com os dados recebidos pela serial.
*   @param[in]  timeout: Recebe o tempo espera para o comando de resposta enviado pelo modulo.
*       
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool  GSM_MessageRead(char *receive, char* buffer, uint16_t timeout );

/**
*   @brief Funcao ConfigureGPS: Inicializa o modulo GPS com a configuracao padrao.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_ConfigureGPS(void);

/**
*   @brief Funcao StartGPS: Ativar mensagens nao solicitadas de dados de GPS no formato NMEA.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_StartGPS(void);

/**
*   @brief Funcao GetFixGPS: Captura os dados GPS no formato NMEA. 
*
*   @param[in]  buffer: de entrada com os dados recebidos pela serial.
*   @param[out] time: Preenche um array de char com a hora recebida pelo modulo GPS. 
*   @param[out] latitude: Atualiza a variavel latitude com a latitude.
*   @param[out] longitude: Atualiza a variavel longitude com a longitude.
*   @param[out] satelites: Preenche um array de char com a quantidade de satelites que o modulo GPS encontrou.
*   @param[out] altitude: Preenche um array de char com a altitude recebida pelo modulo GPS.
*   
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_GetFixGPS(const char* receive, char* time, double* latitude, double* longitude, char* satelites, char* altitude);

/**
*   @brief Funcao StopGPS: Desativar mensagens nao solicitadas de dados de GPS no formato NMEA.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_StopGPS(void);

/**
*   @brief Funcao ConfigureGSM: Inicializa comunicacao com modulo GSM. 
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_ConfigureGSM(void);

/**
*   @brief Funcao DoACall: Realiza uma chamada telefonica para o numero passado como parametro.
*
*   @param[in]  number: Recebe um numera para chamada telefonica no formato +xx()xxxx-xxxxx.
*   
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_DoACall(char* number);

/**
*   @brief Funcao CloseCall: Finaliza uma chamada telefonica. 
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_CloseCall(void);

/**
*   @brief Funcao GetFixLBS: Obtem a localizacao por LBS (triangulacao por rede de celular).
*
*   @param[out] latitude: Atualiza a variavel latitude com a latitude.
*   @param[out] longitude: Atualiza a variavel longitude com a longitude.
*
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_GetFixLBS(double* latitude, double* longitude);

/**
*   @brief Funcao Fix2Cloud: Realiza a publicacao, por MQTT, de uma coordenada e seu valor.
*
*   @param[in]  coordenada: Recebe o nome da coordenada.
*   @param[in]  valor: Recebe o valor da coordenada.
*   @param[in]  timeout: Recebe o tempo espera para o comando de resposta enviado pelo modulo.
*   
*   @return Retorna 1 se receber resposta "ok" do modulo e 0 se receber "error".
*/
bool GSM_Fix2Cloud(char* coordenada, char* valor, uint16_t timeout);

GsmState_et GSM_getState(void);
GsmState_et GSM_getDevState(void);


void GSM_TcpReadCallback(uint8_t data);

#endif /* GSMXE910_H_ */