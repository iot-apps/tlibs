/**
 * @file nativeMqtt.c
 *
 * Telit / GS2101M wifi / EVB3 / Arduino M0
 *
 * @brief Exemplo MQTT Native Wifi
 * 
 * Publica vari�veis em sistema MQTT, atrav�s do metodo m�todo nativo para modem:
 * Cria rotinas que leem uma fila de dados a serem enviados para o portal MQTT da Telit,
 * em pares "nome da da vari�vel" e "valor", e para cada item executa o post de vari�veis por MQTT embutido nos m�dulos.
 *
 * @version 1.00 
 */
/* 
 * INSTRU��ES DE UTILIZA��O
 * 
 * Esse exemplo foi criado para utiliza��o da placa EVB3 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conex�o das placas devem ser de acordo com a seguinte pinagem:
 *
 * Pino Arduino M0 Pro	Pino EVB3
 *		GND				  GND
 *		3.3V			VIN_3V3
 *		10			GPIO0/UART0_RX_M
 *		11			GPIO1/UART0_TX_M
 *		09			EXT_RTC_RESET_N
 * 
 * As configura��es de servidor, porta, rede e senha dever�o ser configurados manualmente atrav�s da altera��o de constantes no c�digo.
 * Para mais informa��es sobre o exemplo ou sobre a integra��o do Arduino e do kit da telit veja: https://www.????????????/
 */
 
#include <asf.h>
#include <string.h>

#include "serial.h"
#include "wifiGs2101m.h"
#include "nativeMqtt.h"


/******************************************************************************
 *                               DEFINI��ES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conex�o.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de mem�ria livre no Heap.

#define NATIVEMQTT_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define NATIVEMQTT_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define WIFI_SSID               "Antares"
#define WIFI_PASSWORD           "17111993"

#define MQTT_DEFAULT_ADDRESS     "10.0.0.5"
#define MQTT_DEFAULT_PORT        1883
#define MQTT_DEFAULT_CLIENT_ID   "nativeMqtt"

#define DATA_RECEIVED_MAX_LEN   64


/******************************************************************************
 *                               PROT�TIPOS LOCAIS
 ******************************************************************************/
static void nativeMqttTask(void *params);
static void LEDS_Init(void);


/******************************************************************************
 *                               VARI�VEIS GLOBAIS
 ******************************************************************************/
static uint8_t       gu8_cid          = 0;
static QueueHandle_t gs_tcpInQueue    = NULL;
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


void nativeMqttInit(void)
{
    xTaskCreate( nativeMqttTask           ,
                 (const char*) "Mqtt"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 NATIVEMQTT_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICA��O
 ******************************************************************************/
static void nativeMqttTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICA��O DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*          NATIVE MQTT       *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o Wifi ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando...");
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
     * O loop da aplica��o principal � respons�vel
     * pelo gerenciamento das conex�es WIFI e TCP.
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

        if ( !WIFI_IsConnected() )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( WIFI_Connect(WIFI_SSID, WIFI_PASSWORD) )
            {
                char* ip = (char*)pvPortMalloc(16);

                if ( WIFI_GetIp(ip) )
                {
#ifdef VERBOSE
                    SERIAL_Println("OK");

                    SERIAL_Print("  Meu IP: ");
                    SERIAL_Println(ip);
#endif
                }

                vPortFree(ip);
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
            if ( MQTT_Connect(MQTT_DEFAULT_ADDRESS, MQTT_DEFAULT_PORT, MQTT_DEFAULT_CLIENT_ID, "", "") )
                {
#ifdef VERBOSE
                    SERIAL_Println("MQTT Connect OK");
#endif
                }
                else
                {
#ifdef VERBOSE
                    SERIAL_Println("MQTT Connect ER");
#endif
                }
        }


        vTaskDelay(NATIVEMQTT_TASK_DELAY);
    }

    vQueueDelete(gs_serialInQueue);
    vQueueDelete(gs_tcpInQueue);
    vTaskDelete(NULL);
}


void SERIAL_ReadCallback(uint8_t data)
{
    xQueueSendFromISR(gs_serialInQueue, (uint8_t*)&data, NULL);
}


void WIFI_TcpReadCallback(uint8_t data, SerialPort_et port)
{
    xQueueSendFromISR(gs_tcpInQueue, (uint8_t*)&data, NULL);
}
