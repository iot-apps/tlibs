/*
*
*	@file HE910G.cpp
*
*	Esta biblioteca deve ser integrada a IDE arduino para ser utilizada por todos os exemplos dos módulos HE910/UE910/UL865/UE866 .
* 
*	@brief A biblioteca HE910G implementa um grande número de funcionalidades para os módulos Telit das famílias HE910/UE910/UL865/UE866 .
* 
*	@author Jonas de Luna Ieno, Lucas S. Feitosa, Pedro Granville e Vinícius Roosvelt.
* 
*	@version 1.00
*/
#include "HE910G.h"

#include <string.h>


#define NUM_TRIES   3


#define CMD_ECHO            		"ATE"

#define CMD_GSM_OPERADORA			"AT+CGDCONT"
#define CMD_GSM_CONNECT   			"AT#SGACT"
#define CMD_GSM_SK_CONNECT  		"AT#SD"
#define CMD_GSM_SK_DISCONN			"AT#SH"
#define CMD_GSM_SEND        		"AT#SSEND"
#define CMD_GSM_RECV	    		"AT#SRECV"
#define CMD_GSM_GETIP				"AT+CGPADDR="

#define CMD_GSM_SMS_CSMS			"AT+CSMS"
#define CMD_GSM_SMS_TEXT	   		"AT+CMGF"
#define CMD_GSM_SMS_SEND 	 		"AT+CMGS"
#define CMD_GSM_SMS_ACKNOW			"AT+CNMA"
#define CMD_GSM_SMS_CNMI			"AT+CNMI"

#define CMD_GSM_DTMF_MODE			"AT#DTMF"

#define CMD_GSM_CONFIG_CLOUD		"AT#DWCFG"
#define CMD_GSM_CONNECT_CLOUD		"AT#DWCONN"
#define CMD_GSM_SEND2CLOUD			"AT#DWSEND"
#define CMD_GSM_SEND_REPLY			"AT#DWRCV"	

#define CMD_GPS_RST					"AT$GPSRST"
#define CMD_GPS_ClR					"AT$GPSNVRAM"
#define CMD_GPS_ACP					"AT$GPSACP"
#define CMD_GPS_GET					"AT$GPSNMUN"
#define CMD_GPS_START				"AT$GPSP"

#define CMD_GSM_AUTO_ANSWER_CALL	"ATS0"
#define CMD_GSM_RECEIVE_A_CALL		"ATA "
#define CMD_GSM_DO_A_CALL			"ATD "
#define CMD_GSM_CLOSE_CALL			"ATH"

#define CMD_GSM_AUDIO_PLAY			"AT#APLAY"

#define CMD_GSM_REBOOT				"AT#REBOOT"

HE910G::HE910G(Uart* uartModule):at(uartModule) {


}

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
bool HE910G::Init(const char* operadora, const char* ip) {
    bool result = false;
		
    result = AT_RESP_OK == at.SendATCmdWaitResp(STR_AT, 500, 100, STR_OK, NUM_TRIES);

    if (result) {
        char command[64] = {0};

        strcpy(command, CMD_ECHO);
        strcat(command, "0");

        result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

        if (result) {
            strcpy(command, CMD_GSM_OPERADORA);
            strcat(command, "=1,");
            strcat(command, ip);
			strcat(command, ",");
			strcat(command, operadora);			

            result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
        }
    }

    return result;
}

/**
*	@brief Função Connect: Realiza a conexão do módulo com a operadora celular através do comando
*	"AT#SGACT=1,1".
*
*	@param[out]	resp Preenche um array de char com o endereço ip fornecido pela operadora.
*	@param[in]	respLen Determina o tamanho máximo que a resposta pode ter.
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::Connect(char* resp, int respLen) {
    bool result      = false;
    char command[64] = {0};

	resp[0] = '\0';

    strcpy(command, CMD_GSM_CONNECT);
    strcat(command, "=1,1");
 	
    result = AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, respLen, NUM_TRIES);

	if (result) {

		if ( (strlen(resp) > 0) && (NULL == strstr(resp, STR_ERROR)) ) {
			result = true;
		}
		else{
			result = false;
		}
	}

    return result;
}


/**
*	@brief Função Disconnect: Desconecta o módulo com a operadora celular através do comando
*	"AT#SGACT=1,0".
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::Disconnect(void) {
    
    bool result      = false;
    char command[64] = {0};

    strcpy(command, CMD_GSM_CONNECT);
    strcat(command, "=1,0");
 	
    result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);

	return result;
}

/**
*	@brief Função SocketConnect: Abre um socket entre um cliente(ex. módulo) e um servidor de destino
*	(ex. pc) através do comando AT#SD=1,0,port,"server_address",0,0,1.
*
*	@param[in]	address Recebe o endereço ip para conexão de destino.
*	@param[in]	port Recebe a porta para conexão de destino.
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::SocketConnect(char* address, int port) {
    
	bool result      = false;
	char command[64] = {0};
	char portStr[10];
	
	strcpy(command, CMD_GSM_SK_CONNECT);
    strcat(command, "=1,0,");
    sprintf(portStr,"%d", port);
	strcat(command, portStr);
	strcat(command, ",");
	strcat(command, address);
	strcat(command, ",0,0,1");

	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
		
    return result;
}


/**
*	@brief Descrição: Função GetIP: Obtem o endereço IP, caso o já esteja conectado.
*
*	@param[out]	address Preenche um array de char com o endereço ip fornecido pela operadora.
*	@return Retorna 1 se receber resposta "ok"(tem IP) do módulo e 0 se receber "error"(não tem IP).
*/
bool HE910G::GetIP(char* address) {
    bool result      = false;
	char command[64] = {0};
	char     resp[100] = {0};
	char n1Str[10] = {0};
	char n2Str[10] = {0};
	char n3Str[10] = {0};
	char n4Str[10] = {0};
	int n1 = 0;
	int n2 = 0;
	int n3 = 0;
	int n4 = 0;
	
	address[0] = '\0';
	strcpy(command, CMD_GSM_GETIP);

	result = AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, 100, NUM_TRIES);
	sscanf(resp,"\r\n+CGPADDR: 1,\"%d.%d.%d.%d\"\r\n\r\nOK\r\n", &n1,&n2,&n3,&n4);
	
	if (result && (strlen(resp) > 25))
	{
		sprintf(n1Str,"%d", n1);
		sprintf(n2Str,"%d", n2);
		sprintf(n3Str,"%d", n3);
		sprintf(n4Str,"%d", n4);
		strcpy(address, n1Str);
		strcat(address, ".");
		strcat(address, n2Str);
		strcat(address, ".");
		strcat(address, n3Str);
		strcat(address, ".");
		strcat(address, n4Str);
	}
	else result = false;		
    return result;
}


/**
*	@brief Função SocketWrite: Envia um array de char (dados) por socket através do comando
*	"AT#SSEND=1".
*	
*	@param[in]	buffer recebe um array de dados para envio por socket.
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::SocketWrite(char* buffer) {
    char command[64] = {0};
	bool result      = false;
	char toSend[200] = {0};
	char     resp[100] = {0};
	
	strcpy(command, CMD_GSM_SEND);
    strcat(command, "=1");
	
	result = AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, 100, NUM_TRIES);
	
	if(result){
		//Se receber (0x3E 0x20) "> " escreve o comando abaixo.
		if ( (strlen(resp) > 0) && (NULL != strstr(resp, STR_SEND_OK)) ) {
			sprintf(toSend,"%s\x1A", buffer);
			//at.WriteLn(toSend);
			result = AT_RESP_OK == at.SendATCmdWaitResp(toSend, 10000, 100, STR_OK, NUM_TRIES);
		}
	}
	return result;
}


/**
*	@brief Função SocketRead: Realiza a leitura de dados recebedos através da conexão por socket a
*	partir do comando "AT#SRECV=1,<tamanho_do_dado>".
*
*	@param[out]	buffer Preeche um buffer com os dados recebidos por socket.
*	@param[in]	maxBufferLen Determina a quantidade máxima de bytes que deve ser lido.
*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
*	@return Retorna a quantidade de dados recebidos.
*/
int  HE910G::SocketRead(char* buffer, uint16_t maxBufferLen, uint16_t timeout) {
    bool result = false;
	int      dataReceivedLen = -1;
    char     resp[200] = {0};
    char	 receive[100] = {0};
	char     dataReceivedLenStr[50+1] = {0};
	char command[64] = {0};
	char lenStr[10] = {0};
	
    
	if ( (at.Read(receive, 200, timeout, 100)) >= 7 ) {
		
		if ((strlen(receive) > 0) && (NULL != strstr(receive, STR_SRING)))
		{		
			strcpy(command, CMD_GSM_RECV);
			strcat(command, "=1,");
			sprintf(lenStr,"%d", maxBufferLen);
			strcat(command, lenStr);
			
			result = AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, 100, NUM_TRIES);
			
			if(result){	
				
				char i = 0;
				for (i = 2; i < strlen(resp); i++) {
					
					dataReceivedLenStr[i] = resp[i]; 
					
					if ( (resp[i] == 0x0D)  ) {
						dataReceivedLenStr[i] = '\0';
						strncpy(dataReceivedLenStr, resp+2, i);
						sscanf(dataReceivedLenStr, "#SRECV: 1,%d", &dataReceivedLen);
						break;
					}
				}	
				
				i += 2;
				
				if (0 < dataReceivedLen) {
					
					memcpy(buffer, resp+i, (dataReceivedLen-1));
					buffer[dataReceivedLen-1] = '\0';
				}	
				
			}	
		}
	}
    return dataReceivedLen;
}


/**
*	@brief Função SocketDisconnect: Disconecta uma conexão aberta por socket através do comando
*	"AT#SH=1".
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::SocketDisconnect(void) {
    bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GSM_SK_DISCONN);
    strcat(command, "=1");

	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
    
	return result;
}


/**
*	@brief Função Reset: Realiza o reset do módulo por hadware.
*
*	@param[in]	pin Recebe o número do pino conectado ao pino de reset do módulo gsm.
*/
void HE910G::Reset(int pin){
	
	pinMode(pin, OUTPUT);
	
	digitalWrite(pin, LOW);
    delay(1000); 
    digitalWrite(pin, HIGH); 

}


/**
*	@brief Função Reboot: Reinicializa o módulo.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::Reboot(void){
	
	bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GSM_REBOOT);
    
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
    
	return result;
}


/**
*	@brief Função CloudConnect: Realiza uma conexão M2M através do comando de configuração
*	AT#DWCFG=<api server>,0,<token da aplicação> e do comando de conexão AT#DWCONN=1.
*
*	@param[in]	apiServer Recebe o endereço da aplicação.
*	@param[in]	token Recebe o token da aplicação.
*	
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::CloudConnect(char* apiServer, char* appToken) {
    
	bool result      = false;
	char command[64] = {0};
	char portStr[10];
	
	strcpy(command, CMD_GSM_CONFIG_CLOUD);
    strcat(command, "=");
   	strcat(command, apiServer);
	strcat(command, ",0,");
	strcat(command, appToken);
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 5000, 100, STR_OK, NUM_TRIES);
	
	
	if (result)
	{
		command[0] = '\0';
		
		strcpy(command, CMD_GSM_CONNECT_CLOUD);
		strcat(command, "=1");
		
		result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
		
	}
    return result;
}


/**
*	@brief Descrição: Função Send2Cloud: Realiza a publicação de uma propriedade de uma variável e seu valor.
*
*	@param[in]	variable Recebe o nome da variável.
*	@param[in]	value Recebe o valor da variável.
*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
*	
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::Send2Cloud(char* variable, char* value, uint16_t timeout) {
    char  command[64] = {0};
	bool  result      = false;
	char	 receive[100] = {0};
	char    resp[100] = {0};
	int match1 = 0;
	char RING[10] = {0};
		
	strcpy(command, CMD_GSM_SEND2CLOUD);
    strcat(command, "=0,property.publish,key,");
	strcat(command, variable);
	strcat(command, ",value,");
	strcat(command, value);
		
	result = AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, 100, NUM_TRIES);
	
	sscanf(resp,"\r\n#DWSEND: %d\r\n\r\nOK\r\n", &match1);
	
	if (result)
	{
		
		while(timeout-- > 0){
			delay(1);
						
			if (at.Read(receive, 200, timeout, 100))
				break;
				
		}
		
		char i = 0;
		bool comma = false;
		
		for (i = 0; i < strlen(receive); i++){
			
			if (receive[i] == 0x2C){
				comma = true;
				i++;
				break;
			}
		}
		
		char j = 0;
		
		for (j = 0; j < strlen(receive); j++){
				
			if (receive[i] != 0x2C){
				RING[j] = receive[i++];
			}else{
				RING[j+1] = '\0';
				break;
			}
		}
				
		int match2 = atoi(RING);
		char* strMatch[10] = {0};
		
		if (match1 == match2){
			strcpy(command, CMD_GSM_SEND_REPLY);
            strcat(command, "=");
            strcat(command, RING);

            result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
		}
			
	}
		
	return result;
}


/**
*	@brief Função CloudDisconnect: Disconecta do servidor de aplicação.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::CloudDisconnect(void) {
    bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GSM_CONNECT_CLOUD);
    strcat(command, "=0");

	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);
    
	return result;
}


/**
*	@brief Função ConfigureSMS:Configura o módulo para envia de menssagens de texto.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::ConfigureSMS(void) {
    bool result = false;
		
    result = AT_RESP_OK == at.SendATCmdWaitResp(STR_AT, 500, 100, STR_OK, NUM_TRIES);

    if (result) {
        char command[64] = {0};

        strcpy(command, CMD_ECHO);
        strcat(command, "0");

        result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

        if (result) {
            //command [0] = '\0';
			
			strcpy(command, CMD_GSM_SMS_CSMS);
            strcat(command, "=1");
            
            result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
			
			if (result) {
				//command [0] = '\0';
				
				strcpy(command, CMD_GSM_SMS_TEXT);
				strcat(command, "=1");
				
				result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

				if (result) {
					//command [0] = '\0';
					
					strcpy(command, CMD_GSM_SMS_CNMI);
					strcat(command, "=2,2,0,0,0");
					
					result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
				}

			}
 
		}
    
	}

    return result;
}

/**
*	@brief Função MessageWrite: Configura o módulo para envia de menssagens de texto.
*
*	@param[in]	number Recebe o número do celular.
*	@param[in]	buffer Recebe um array de char contendo uma menssagem.
*		
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::MessageWrite(const char* number, char* buffer) {
    char command[64] = {0};
	bool result      = false;
	char toSend[200] = {0};
	char     respSend[100] = {0};
	
	strcpy(command, CMD_GSM_SMS_SEND);
    strcat(command, "=");
	strcat(command, number);
	
	result = AT_RESP_OK == at.SendATCmdReturnResp(command, 5000, 100, respSend, 100, NUM_TRIES);
	if (result){	
		
		if ( (strlen(respSend) > 0) && (NULL != strstr(respSend, STR_SEND_OK)) ) {
			sprintf(toSend,"%s\x1A", buffer);
			result = AT_RESP_OK == at.SendATCmdWaitResp(toSend, 5000, 100, STR_OK, NUM_TRIES);
		}
	}
	return result;
}

/**
*	@brief Função MessageRead: Lê menssagens de texto recebidas pelo módulo.
*
*	@param[out]	buffer Preenche um array de char com a mensagem recebida pelo módulo.
*	@param[in]	timeout Recebe o tempo espera para o comando de resposta enviado pelo módulo.
*		
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool  HE910G::MessageRead(char* buffer, uint16_t timeout) {
    bool result = false;

    char	 receive[100] = {0};
	char     dataReceivedLenStr[50+1] = {0};
	char command[64] = {0};
    
	if ( (at.Read(receive, 200, timeout, 100)) >= 7 ) {
		
		if ((strlen(receive) > 0) && (NULL != strstr(receive, STR_MSG_RCV)))
		{		
			char i = 0;
			char j = 0;
			bool head = false;
			
			for (i = 2; i < strlen(receive); i++) {
				
				if ( (receive[i] == 0x0A)  ) {
					i++;
					head = true;
					break;
				}
			}
			for (j = 0; strlen(receive); j++)
			{
				if (receive[i] != 0x0D)
					dataReceivedLenStr[j] = receive[i++];
				else
				{	
					dataReceivedLenStr[j+1] = '\0';
					break;
				}
			}				
			
			strcpy(buffer, dataReceivedLenStr);
			
			strcpy(command, CMD_GSM_SMS_ACKNOW);
			result = AT_RESP_OK == at.SendATCmdWaitResp(command, 5000, 100, STR_OK, NUM_TRIES);
			
		}
	}
    return result;
}

/**
*	@brief Função ConfigureGPS: Inicializa o módulo GPS com a configuração padrão.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::ConfigureGPS(void) {
    bool result = false;
	char command[64] = {0};
		
    result = AT_RESP_OK == at.SendATCmdWaitResp(STR_AT, 500, 100, STR_OK, NUM_TRIES);

    if (result) {
        
        strcpy(command, CMD_ECHO);
        strcat(command, "0");

        result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

        if (result) {
            
			strcpy(command, CMD_GPS_RST);
            result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
			
			if (result) {
			
				strcpy(command, CMD_GPS_ClR);
				strcat(command, "=15,0");
				
				result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

				if (result) {
					
					strcpy(command, CMD_GPS_ACP);
					result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
				
				}
			}
		}
	}

    return result;
}

/**
*	@brief Função StartGPS: Ativar mensagens não solicitadas de dados de GPS no formato NMEA.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::StartGPS(void) {
	bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GPS_GET);
	strcat(command, "=1,1,0,0,0,0,0");
		
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

	if (result)
	{	
		strcpy(command, CMD_GPS_START);
		strcat(command, "=1");
	
		result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
	}
	else{
		result = false;
	}
	return result;
}

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
bool HE910G::GetFixGPS(char* time, double* latitude, double* longitude, char* satelites, char* altitude) {

	bool result = false;
	char      receive[300] = {0};
	char     position[100] = {0};
	char         resp[100] = {0};
	
	char  sexaLatitude[20] = {0};
	char sexaLongitude[20] = {0};
	
	char*  tempLate;
	char*  tempLonge;

	double  lGraus = 0.0;
	double	lMinutos = 0.0;
	double	lSegundos = 0.0;
	
	double convert_lat = 0.0;
	double convert_lon = 0.0;
	
	char      latGraus[10] = {0};
	char    latMinutos[10] = {0};
	char   latSegundos[10] = {0};	
	char      lonGraus[10] = {0};
	char    lonMinutos[10] = {0};
	char   lonSegundos[10] = {0};
	
	int      inHeader = 0; 
		
		
	if((at.Read(receive, 300, 1000, 500)) >= 10){
		
		if ((strlen(receive) > 0) && (NULL == strstr(receive, STR_GPS_NOSIGNAL))){
			
			char i = 0; 
			
			for(i = 10; i <= strlen(receive); i++){
				if (receive[i] == 0x0A)
				{
					inHeader = i++;
					break;
				}
			}
			
			memcpy(resp, &receive[inHeader],(sizeof(receive) - inHeader)); 
			
			delay(1000);
			
			if (NULL != strstr(resp, STR_GPS_END_DATA)){
			
				char j = 0;
			
				for(j = 20; j <= strlen(resp); j++)
				{
					if(resp[j] == 0x4D)
					{ 
						resp[--j] = '\0';
						break;
					}
				}
				
				int k = 0;
				uint8_t paramIndex = 0;
				uint8_t respLen    = strlen(resp);
				char*   field      = NULL;
				
				for(k = 0; k < respLen; k++)
				{
					if (resp[k] == ',')
					{
						resp[k] = '\0';
						switch(paramIndex)
						{
							case 0:
								field = resp+k+1;	// Time
							break;
							case 1:			
								strcpy(time, field);	
								field = resp+k+1;	// Latitude
							break;
							case 2:
								//S
								strcpy(sexaLatitude, field);
								field = resp+k+1;	// S
							break;
							case 3:
								field = resp+k+1;	// Longitude
							break;
							case 4:
								strcpy(sexaLongitude, field);
								field = resp+k+1;	// W
								//W
							break;
							case 5:
								field = resp+k+1;	// 1
								//
							break;
							case 6:
								field = resp+k+1;	// Satelites
							break;
							case 7:
								strcpy(satelites, field);
								field = resp+k+1;	// 1.00
							break;
							case 8:
								field = resp+k+1;	// Altitude
							break;

						}
						
						paramIndex++;
					}
					strcpy(altitude, field);
					
				}
				
				// Transformar de coordenada sexagesimal para coordenada decimal.
				/*
				strcpy(latitude, sexaLatitude);
				strcat(latitude," = ");
								
				strcpy(longitude, sexaLongitude);
				strcat(longitude," = ");
				*/
				
				tempLate = strtok(sexaLatitude,".");
				strcat(latGraus,tempLate);
				tempLate = strtok(NULL,".");
				strcpy(latSegundos,tempLate);
				strncpy(latMinutos,latGraus+2, strlen(latGraus)-2);	
				latGraus[2] = '\0';
				
				lGraus = atof(latGraus);
				lMinutos =  (atof(latMinutos)/60.0);
				lSegundos = ((atoi(latSegundos)/100.0)/3600.0);
				
				*latitude = (-1.0) * (lGraus + lMinutos + lSegundos);
				
				//sprintf(latitude, "%s", String(convert_lat,6).c_str());
					
				tempLonge = strtok(sexaLongitude,".");
				strcat(lonGraus,tempLonge);
				tempLonge = strtok(NULL,".");
				strcpy(lonSegundos,tempLonge);
				strncpy(lonMinutos,lonGraus+3, strlen(lonGraus)-3);	
				lonGraus[3] = '\0';

				lGraus = atof(lonGraus);
				lMinutos = atof(lonMinutos)/60.0;
				lSegundos = ((atoi(lonSegundos)/100.0)/3600.0);
				
				*longitude = (-1.0) * (lGraus + lMinutos + lSegundos);
				
				//sprintf(longitude, "%s", String(convert_lon,6).c_str());
	
				result = true;
			}
			
		}
		else{
			result = false;
		}
	
	}
	
	return result;
}

/**
*	@brief Função StopGPS: Desativar mensagens não solicitadas de dados de GPS no formato NMEA.
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::StopGPS(void) {
	bool result = false;
	char command[64] = {0};
	 
	strcpy(command, CMD_GPS_GET);
	strcat(command, "=0,0,0,0,0,0,0");
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

	if (result)
	{	
		strcpy(command, CMD_GPS_START);
		strcat(command, "=0");
	
		result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
	}
	else{
		result = false;
	}
	return result;
}
/**
*	@brief Função DoACall: Realiza uma chamada telefônica para o número passado como parâmetro.
*
*	@param[in]	number Recebe um numera para chamada telefônica no formato +xx()xxxx-xxxxx.
*	
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::DoACall(char* number) {
	bool result = false;
	char command[64] = {0};
		 
	strcpy(command, CMD_GSM_DO_A_CALL);
	strcat(command, number);
	strcat(command, ";");
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);

	return result;
}

/**
*	@brief Função CloseCall: Finaliza uma chamada telefônica. 
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::CloseCall(void) {
	bool result = false;
	char command[64] = {0};
	 
	strcpy(command, CMD_GSM_CLOSE_CALL);
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

	return result;
}

/**
*	@brief Função ReceiveCall: atende uma chamada telefônica. 
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::ReceiveCall(void) {
	bool result = false;
	char command[64] = {0};
	 
	strcpy(command, CMD_GSM_RECEIVE_A_CALL);
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
	
	return result;
}

/**
*	@brief Função PlayAudio: Executa o arquivo de audio armazenado previamente na memoria do modem. 
*
*	@param[in] file: Recebe o nome do arquivo de audio a ser executado no formato "audio.wav"
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/	
bool HE910G::PlayAudio(char* file) 	//name = nome do arquivo de audio com o formato, armazendado na memoria interna do modulo
{
	bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GSM_AUDIO_PLAY);
	strcat(command, "= 1,1, ");
	strcat(command, file);
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
	return result;	
}

/**
*	@brief Função ConfigureDTMF: Habilita o DTMF decoder (habilita a leitura dos digitos enviado pelo teclado numérico)
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/
bool HE910G::ConfigureDTMF(void) {
	bool result = false;
	char command[64] = {0};
	
	strcpy(command, CMD_GSM_DTMF_MODE);
	strcat(command, "=1");
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

	return result;
}

/**
*	@brief Função RingAutoAnswer: Habilita o auto atendimento de uma chamada telefonica;
*
*	@param[in] nRING: Recebe um numero inteiro para a definição de quantidade de "RINGs" antes de responder a chamada,
*	caso o valor for 0, o auto atendimento é desativado
*
*	@return Retorna 1 se receber resposta "ok" do módulo e 0 se receber "error".
*/	
bool HE910G::RingAutoAnswer(int nRING) 	
{
	bool result = false;
	char command[64] = {0};
	char num[2] = {0};
	num[1] = '\0';
	num[0] = nRING + 48;
	strcpy(command, CMD_GSM_AUTO_ANSWER_CALL);
	strcat(command, "= ");
	strcat(command, num);
	
	result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
	return result;	
}
