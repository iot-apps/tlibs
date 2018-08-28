/**
 * @file logger_wifi.c
 *
 * BlueMode+S42 / GS2101M telit module \n
 * EVB3 \n
 * BlueEva+S \n
 * Arduino M0
 *
 * @brief Exemplo logger Bluetooth Wifi
 * 
 * Este e um exemplo de um sistema de logger baseado em uma Task (ASF) 
 * que repassa o trafego de uma conexao de portas seriais bluetooth para um socket aberto com um servidor remoto e 
 * bufferiza os bytes que sao recebidos enquanto a conexao nao estava ativa. 
 *
 * @version 1.00 
 */
/* 
 * INSTRUÇÕES DE USO
 * 
 * Esse exemplo foi criado para utilização da placa BlueEva+S integrado com a EVB3 da Telit e com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *
 *	Pino Arduino M0 | Pro	Pino BlueEva+S	/	Pino Arduino M0 | Pro Pino EVB3		/	Pino BlueEva+S | Pino EVB3
 *	--------------------------------------  /	---------------------------------	/	--------------------------
 *		3			|	X322/EXT-RES		/	3.3V			|  VIN_3V3			/		ext. PWR   |  VIN_3V3
 *		4			|	X324/UART-RXD		/	GND				|  GND				/		GND		   |  GND
 *		5			|	X323/UART-TXD		/	09				|  EXT_RTC_RESET_N	/	
 *											/	10				|  GPIO0/UART0_RX_M	/	
 *											/	11				|  GPIO1/UART0_TX_M	/
 * 
 * As configurações de servidor, porta, rede e senha deverão ser configurados manualmente através da alteração de constantes no código.
 * Para mais informações sobre o exemplo ou sobre a integração do Arduino e do kit da telit veja: https://www.????????????/
 *
 * INTERFACE BLUETOOTH
 *  
 *	A telit disponibiliza o aplicativo "Terminal IO Utility" para android e iOS o qual pode ser utilizado 
 * para estabelecer conexões bluetooth de baixa energia entre um smartphone e o BlueEva+S.
 * 	Neste exemplo se faz o uso deste aplicativo para a intefarce de conexão do dispositovo a placa da Telit, onde 
 * através do mesmo é realizado a conexão e troca de dados.
 * O aplicativo "Terminal IO Utility" da Telit pode ser encontrado na AppleStore e na Google Play Store para download.
 */

#include <asf.h>
#include <string.h>

#include "at.h"
#include "serial.h"
#include "bluetoothBms42.h"
#include "wifiGs2101m.h"
#include "logger_wifi.h"


/******************************************************************************
 *                               DEFINIÇÕES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conexão.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de memória livre no Heap.

#define LOGGER_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define LOGGER_TASK_DELAY       (100 / portTICK_PERIOD_MS)
#define WIFI_SSID               "Antares"
#define WIFI_PASSWORD           "17111993"
#define TCP_DEFAULT_ADDRESS     "10.0.0.4"
#define TCP_DEFAULT_PORT        35741

#define BT_MODE                 2
#define DATA_RECEIVED_MAX_LEN   64


/******************************************************************************
 *                               PROTÓTIPOS LOCAIS
 ******************************************************************************/
static void loggerTask(void *params);
static void LEDS_Init(void);


/******************************************************************************
 *                               VARIÁVEIS GLOBAIS
 ******************************************************************************/

static uint8_t       gu8_cid          = 0;
static QueueHandle_t gs_btInQueue     = NULL;
static QueueHandle_t gs_wifiInQueue   = NULL;
static QueueHandle_t gs_serialInQueue = NULL;


static void LEDS_Init(void)
{
   struct port_config pin_conf_output = { PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE, false };
   
    port_pin_set_config(LED_L_PIN , &pin_conf_output);
    port_pin_set_output_level(LED_L_PIN, 0);
    port_pin_set_config(LED_TX_PIN, &pin_conf_output);
    port_pin_set_output_level(LED_TX_PIN, 1);
    port_pin_set_config(LED_RX_PIN, &pin_conf_output);
    port_pin_set_output_level(LED_RX_PIN, 1);
}


void loggerInit(void)
{
    gs_btInQueue     = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    gs_wifiInQueue   = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
	gs_serialInQueue = xQueueCreate( DATA_RECEIVED_MAX_LEN, sizeof(uint8_t) );
    
    vQueueAddToRegistry(gs_btInQueue    , "btIn");  // Registro necessário para exibição no FreeRTOS Viewer..
	vQueueAddToRegistry(gs_wifiInQueue  , "wifiIn");
    vQueueAddToRegistry(gs_serialInQueue, "serialIn");

    xTaskCreate( loggerTask              ,
                 (const char*) "Logger"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 LOGGER_TASK_PRIORITY    ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICAÇÃO
 ******************************************************************************/
static void loggerTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICAÇÃO DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*         LOGGER WIFI        *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o Bluetooth ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando o módulo Bluetooth...");

//     if ( BT_Init(BT_MODE) )
//     {
// #ifdef VERBOSE
//         SERIAL_Println("OK");
// #endif
//     }
//     else
//     {
// #ifdef VERBOSE
//         SERIAL_Println("ER");
// #endif
//     }

    SERIAL_Print("Iniciando o módulo Wifi...");

    if ( WIFI_Init() )
    {
#ifdef VERBOSE
        SERIAL_Println("OK");
#endif
    }
    else
    {
#ifdef VERBOSE
        SERIAL_Println("ER");
#endif
    }

    /**********************************************
     * O loop da aplicação principal é responsável
     * pelo gerenciamento das conexões.
     **********************************************/
    for(;;)
    {
#ifdef SHOW_HEAP_FREE
        {
            char* toPrint = (char*)pvPortMalloc(32);

            sprintf(toPrint, "Heap Free: %d", xPortGetFreeHeapSize());
            SERIAL_Println(toPrint);
            
            vPortFree(toPrint);
        }
#endif

        if ( false/*!BT_IsConnected()*/ )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( BT_Connect() )
            {
#ifdef VERBOSE
                SERIAL_Println("OK");
#endif                
            }
            else
            {
#ifdef VERBOSE
                SERIAL_Println("ER");
#endif
            }                
        }
        else
        {
            if ( !WIFI_IsConnected() )
            {
#ifdef VERBOSE
                SERIAL_Print("Conectando Wifi...");
#endif                
                if (WIFI_Connect(WIFI_SSID, WIFI_PASSWORD))
                {
#ifdef VERBOSE
                    SERIAL_Println("OK");
#endif                                    
                }
                else
                {
#ifdef VERBOSE
                    SERIAL_Println("ER");
#endif
                }

            }
            else
            {
                if ( gu8_cid == 0)
                {
#ifdef VERBOSE
                    SERIAL_Print("Conectando TCP...");
#endif
                    
                    if ( (gu8_cid = WIFI_TcpConnect(TCP_DEFAULT_ADDRESS, TCP_DEFAULT_PORT)) <= 0x0f )
                    {
#ifdef VERBOSE
                        SERIAL_Println("OK");
#endif
                    }
                    else
                    {
                        gu8_cid = 0;
#ifdef VERBOSE 
                        SERIAL_Println("ER");
#endif
                    }
                }
                else
                {
                    char     ch     = 0;
                    uint8_t  bufferIndex = 0;
                    uint8_t* buffer = (uint8_t*)pvPortMalloc(DATA_RECEIVED_MAX_LEN+1);

                    // Se houver dados na fila da serial, envia para o tcp
                    for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_serialInQueue, &ch, 0); bufferIndex++)
                    {
                        buffer[bufferIndex] = ch;
                    }

                    if (bufferIndex > 0)
                    {
                        BT_Write(buffer, bufferIndex);
    					SERIAL_Write(buffer, bufferIndex);
                    }
        			
        			memset(buffer, 0, DATA_RECEIVED_MAX_LEN+1);

                    // Se houver dados na fila do tcp, envia para a serial
                    for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_btInQueue, &ch, 0); bufferIndex++)
                    {
                        buffer[bufferIndex] = ch;
                    }

                    if (bufferIndex > 0)
                    {
    					WIFI_TcpWrite(gu8_cid, buffer, bufferIndex);
                        SERIAL_Write(buffer, bufferIndex);
                    }
					
					memset(buffer, 0, DATA_RECEIVED_MAX_LEN+1);

                    // Se houver dados na fila do tcp, envia para a serial
                    for (bufferIndex = 0; (bufferIndex < DATA_RECEIVED_MAX_LEN) && xQueueReceive(gs_wifiInQueue, &ch, 0); bufferIndex++)
                    {
                        buffer[bufferIndex] = ch;
                    }
					
					if (bufferIndex > 0)
                    {
    					BT_Write(buffer, bufferIndex);
                        SERIAL_Write(buffer, bufferIndex);
                    }

                    vPortFree(buffer);
                }
            }
        }

        vTaskDelay(LOGGER_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_btInQueue);
	vQueueDelete(gs_wifiInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}

BT_ReadCallback(uint8_t data, SerialPort_et port) {}

void AT_ReadCallback(uint8_t data, SerialPort_et port)
{
	    switch(port)
	    {
		    case SERIAL_PORT2:
		    {
                if (true/*WIFI_HandleReceivedData(data)*/)
                {
			        xQueueSendFromISR(gs_wifiInQueue, (uint8_t*)&data, NULL);
                }
		    }
		    break;
		    case SERIAL_PORT3:
		    {
			    xQueueSendFromISR(gs_btInQueue, (uint8_t*)&data, NULL);
		    }
		    break;
		    default: break;
	    }
}
