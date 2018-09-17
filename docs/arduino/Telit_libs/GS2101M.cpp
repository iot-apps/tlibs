#include "GS2101M.h"

#include <string.h>


#define NUM_TRIES   3

#define WIFI_MODE_STATION   "0"
#define WIFI_MODE_AP        "1"

#define CMD_ECHO            "ATE"
#define CMD_WIFI_DISCONNECT "AT+WD"
#define CMD_WIFI_SET_MODE   "AT+WM"
#define CMD_WIFI_DHCP       "AT+NDHCP"
#define CMD_WIFI_WPA        "AT+WWPA"
#define CMD_WIFI_CONNECT    "AT+WA"
#define CMD_CID				"AT+CID"
#define CMD_TCP_CONNECT		"AT+NCTCP"
#define CMD_BDATA			"AT+BDATA"

GS2101M::GS2101M(Uart* uartModule):at(uartModule) {


}

/************************************************************************************************************/
/***Descrição: Função Init: Inicializa a comunicação com o módulo wifi a partir do comando basi-*************/
/************* co "at" em seguida envia o comando para desabilitar o "ate0" e por último, envia *************/
/************* o comando para "at+wm = 0" para funcionar como station mode.**********************************/
/************************************************************************************************************/
/***Prototipo: bool GS2101M::Init(void) *********************************************************************/
/***Parâmetros: vazio. **************************************************************************************/
/***Retorno: retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" ****************************/
/************************************************************************************************************/
bool GS2101M::Init(void) {
    bool result = false;

    result = AT_RESP_OK == at.SendATCmdWaitResp(STR_AT, 500, 100, STR_OK, NUM_TRIES);

    if (result) {
        char command[64] = {0};

        strcpy(command, CMD_ECHO);
        strcat(command, "0");

        result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);

        if (result) {
            strcpy(command, CMD_WIFI_SET_MODE);
            strcat(command, "=");
            strcat(command, WIFI_MODE_STATION);

            result = AT_RESP_OK == at.SendATCmdWaitResp(command, 500, 100, STR_OK, NUM_TRIES);
        }
    }

    return result;
}

/************************************************************************************************************/
/***Descrição: Função Connect: Realiza a conexão do módulo com um AP(access point). Antes da conexão são ****/
/************* enviados alguns comandos de configuração: "at+ndhcp=1,ssid,1" para habilitar o dhcp, "at+ ****/
/************* wwpa=password" informa ao módula qual é o password da rede de conexão e por fim se conec- ****/
/************* ta com o módulo através do comando "at+wa=ssid". *********************************************/
/************************************************************************************************************/
/***Prototipo: bool GS2101M::Connect(char* resp, int respLen, const char* ssid, const char* password)********/
/***Parâmetros: *********************************************************************************************/
/*********** -> resp: preenche um array de char com os dados com os dados da conexão: ip, subnet e gateway.**/
/*********** -> respLen: determina o tamanho máximo que a resposta pode ter.*********************************/
/*********** -> ssid: id da rede que se deseja conectar.*****************************************************/
/*********** -> password: senha da rede que se deseja conectar.**********************************************/
/***Retorno: retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" ****************************/
/************************************************************************************************************/
bool GS2101M::Connect(char* resp, int respLen, const char* ssid, const char* password) {
    bool result      = false;
    char command[64] = {0};

	resp[0] = '\0';

    strcpy(command, CMD_WIFI_DHCP);
    strcat(command, "=1,");
    strcat(command, ssid);
    strcat(command, ",1");

    result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);

    if (result) {
        strcpy(command, CMD_WIFI_WPA);
        strcat(command, "=");
        strcat(command, password);

        result = AT_RESP_OK == at.SendATCmdWaitResp(command, 5000, 100, STR_OK, NUM_TRIES);

        if (result) {
			strcpy(command, CMD_WIFI_CONNECT);
			strcat(command, "=");
			strcat(command, ssid);

			if ( AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, respLen, NUM_TRIES) ) {
				// Trata resposta
				if ( (strlen(resp) > 0) && (NULL == strstr(resp, STR_ERROR)) ) {
					result = true;
				}
			}
		}

		if (!result) {
			Disconnect();
		}
    }

    return result;
}


/************************************************************************************************************/
/***Descrição: Função Disconnect: Fecha a conecão do módulo wifi com o AP(acess point) a partir do comando***/
/************* "at+wd". *************************************************************************************/
/************************************************************************************************************/
/***Prototipo: bool GS2101M::Disconnect(void) ***************************************************************/
/***Parâmetros: vazio. **************************************************************************************/
/***Retorno: retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" ****************************/
/************************************************************************************************************/
bool GS2101M::Disconnect(void) {
    return AT_RESP_OK == at.SendATCmdWaitResp(CMD_WIFI_DISCONNECT, 1000, 100, STR_OK, 3);
}

/************************************************************************************************************/
/***Descrição: Função TcpConnect: Abre um socket tcp entre um cliente(ex. módulo) e um servidor de destino **/
/************* (ex. pc) através do comando "at+nctcp=address, port". ****************************************/
/************************************************************************************************************/
/***Prototipo: byte GS2101M::TcpConnect(char* address, int port)*********************************************/
/***Parâmetros: *********************************************************************************************/
/*********** -> addres: recebe o endereço ip para conexão de destino.****************************************/
/*********** -> port: recebe a porta para conexão de destino.************************************************/
/***Retorno: retorna o id da conexão que foi aberta.*********************************************************/
/************************************************************************************************************/
byte GS2101M::TcpConnect(char* address, int port) {
    char cid = 0xFFFF;
	char resp[100+1] = {0};
	char* cidStr     = NULL;
			
	char command[64] = {0};
	char portStr[10];
	
    strcpy(command, CMD_TCP_CONNECT);
    strcat(command, "=");
    strcat(command, address);
    strcat(command, ",");
    sprintf(portStr,"%d", port);
	strcat(command, portStr);

	if (AT_RESP_OK == at.SendATCmdReturnResp(command, 10000, 100, resp, 200, NUM_TRIES) ) {
		if ( NULL != strstr(resp, STR_CONNECT) ) {
				
			
			sscanf(resp, "\r\nCONNECT %c\r\n\r\nOK\r\n", &cid);
			
			if (cid >= '0' && cid <= '9')
				cid -= '0';
			else if (cid >= 'a' && cid <= 'f')
				cid = cid - 'a'+10;
			
		}
	}
	
    return (byte)cid;
}

/************************************************************************************************************/
/***Descrição: Função BData: Habilita o envio/recepção de dados no formato bulk mode através do comando "at */
/************* +bdata=in". **********************************************************************************/
/************************************************************************************************************/
/***Prototipo: bool GS2101M::BData(bool in)******************************************************************/
/***Parâmetros: *********************************************************************************************/
/*********** -> in: recebe 0 para desabilitar e 1 para habilitar. *******************************************/
/***Retorno: retorna 1 se receber resposta "ok" do módulo e 0 se receber "error" ****************************/
/************************************************************************************************************/
bool GS2101M::BData(bool in) {
    bool result      = false;
    char command[64] = {0};

    strcpy(command, CMD_BDATA);
	if (in)
		strcat(command, "=1");
	else
		strcat(command, "=0");
	 
    result = AT_RESP_OK == at.SendATCmdWaitResp(command, 10000, 100, STR_OK, NUM_TRIES);

    return result;
}

/************************************************************************************************************/
/***Descrição: Função TcpWrite: Envia um array de char (dados) no formato bulk mode.*************************/
/************************************************************************************************************/
/***Prototipo: void GS2101M::TcpWrite(byte cid, char* buffer)************************************************/
/***Parâmetros: *********************************************************************************************/
/*********** -> cid: recebe o id da conexão que foi aberta . ************************************************/
/*********** -> buffer: recebo dados para envio. ************************************************************/
/***Retorno: vazio. *****************************************************************************************/
/************************************************************************************************************/
void GS2101M::TcpWrite(byte cid, char* buffer) {
    
	char toSend[200] = {0};
	
	sprintf(toSend,"\x1bZ%d%04d%s", (int)cid, strlen(buffer), buffer);
	at.WriteLn(toSend);
}


/************************************************************************************************************/
/***Descrição: Função TcpRead: Realiza a leitura de dados recebedos através da conexão tcp. *****************/
/************************************************************************************************************/
/***Prototipo: int  GS2101M::TcpRead(byte cid, char* buffer, uint16_t maxBufferLen, uint16_t timeout)********/
/***Parâmetros: *********************************************************************************************/
/*********** -> cid: recebe o id da conexão que foi aberta. *************************************************/
/*********** -> buffer: preeche buffer com os dados lidos via tcp. ******************************************/
/*********** -> maxBufferLen: determina a quantidade máxima de bytes que deve ser lido. *********************/
/*********** -> timeout: ************************************************************************************/
/***Retorno: retorna a quantidade de dados recebidos. *******************************************************/
/************************************************************************************************************/
int  GS2101M::TcpRead(byte cid, char* buffer, uint16_t maxBufferLen, uint16_t timeout) {
    int      dataReceivedLen = -1;
    char     resp[200] = {0};
    uint16_t respLen   =  0;
    char     dataReceivedLenStr[4+1] = {0};

    if ( (respLen = at.Read(resp, 200, timeout, 100)) >= 7 ) {

        uint8_t i;

        // Locate a valid start header of tcp receive:
        for (i = 0; i < (respLen-3); i++) {

            if ( respLen >= (i + 7) ) {

                if ( (resp[i  ] == 0x1b        ) /*&&
                     (resp[i+1] == 'Z'         ) &&
                     (resp[i+2] == (cid + '0') )*/     ) {
                    break;
                }
            }
        }

		i += 3;
		
		if (i < (respLen-3) ) {
			strncpy(dataReceivedLenStr, resp+i, 4);

			i += 4;

			sscanf(dataReceivedLenStr, "%d", &dataReceivedLen);
			memcpy(buffer, resp+i, dataReceivedLen);
			buffer[dataReceivedLen] = '\0';
		}

    }

    return dataReceivedLen;
}


bool GS2101M::TcpDisconnect(void) {
    bool result = false;

    return result;
}


/************************************************************************************************************/
/***Descrição: Função Reset: Realiza o reset do módulo por hadware.******************************************/
/************************************************************************************************************/
/***Prototipo: void GS2101M::Reset(int pin)******************************************************************/
/***Parâmetros: *********************************************************************************************/
/*********** -> pin: recebe o número do pino conectado ao pino de reset do módulo wifi. *********************/
/***Retorno: vazio. *****************************************************************************************/
/************************************************************************************************************/
void GS2101M::Reset(int pin){
	pinMode(pin, OUTPUT);
	
	digitalWrite(pin, LOW);
    delay(1000); 
    digitalWrite(pin, HIGH); 
}