/**
 * @file softwareMqtt.c
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo sistema MQTT
 * 
 * Publica variaveis em sistema MQTT, atraves do metodo por software para modem:
 * Cria rotinas que leem uma fila de dados a serem enviados para o portal MQTT da Telit,
 * em pares "nome da variavel" e "valor", e para cada item executa o post de variaveis por MQTT embutido nos modulos.
 *
 * @version 1.00 
 */
/*
 * INSTRU��ES DE UTILIZA��O
 * 
 * Esse exemplo foi criado para utiliza��o do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conex�o das placas devem ser de acordo com a seguinte pinangem:
 *	Pino Arduino M0 Pro		  Pino EVK2
 *			GND					GND
 *			10					C104/RXD
 *			11					C103/TXD
 *			09					RESET
 * 
 * As configura��es de servidor, baudrate e afins dever�o ser realizadas manualmente, atrav�s das constantes no c�digo.
 * Para mais informa��es sobre o exemplo ou sobre a integra��o do Arduino e do kit da telit veja: https://www.????????????/
 */
#include <asf.h>
#include <string.h>

#include "serial.h"
#include "wifiGs2101m.h"
#include "gsmXE910.h"
#include "softwareMqtt.h"


/******************************************************************************
 *                               DEFINI��ES
 ******************************************************************************/

#define VERBOSE         // Exibe os status da conex�o.
//#define SHOW_HEAP_FREE  // Exibe a quantidade de mem�ria livre no Heap.

#define SOFTWAREMQTT_TASK_PRIORITY      (tskIDLE_PRIORITY + 2)
#define SOFTWAREMQTT_TASK_DELAY         (100 / portTICK_PERIOD_MS)

#define DEFAULT_GSM_APN         "gprs.oi.com.br"
#define DEFAULT_GSM_PROTOCOL    "IP"

#define MQTT_DEFAULT_ADDRESS     "52.200.8.131" //TODO: N�o consegui testar DNS
#define MQTT_DEFAULT_PORT        1883
#define MQTT_DEFAULT_CLIENT_ID   "gs2k_178db1"
#define MQTT_DEFAULT_USR         "Default"
#define MQTT_DEFAULT_PWD         "pLh5olDkMVIIG8Pd"

#define NUM_MAX_KEY_VALUE        5
#define VARIABLE                 0
#define VALUE                    1


/******************************************************************************
 *                               PROT�TIPOS LOCAIS
 ******************************************************************************/
static void softwareMqttTask(void *params);
static void LEDS_Init(void);
static void sendMessages(void);


/******************************************************************************
 *                               VARI�VEIS GLOBAIS
 ******************************************************************************/
static uint8_t       gu8_cid         = 0;
static char          g_msgs [NUM_MAX_KEY_VALUE][2][40] = {
    {"thing/sensor1/property/latitude" ,"35"  },
    {"thing/sensor1/property/longitude" ,"34"  },
    {"thing/sensor1/property/latitude" ,"23"  },
    {"thing/sensor1/property/longitude" ,"56"  },
    {"thing/sensor1/property/longitude" ,"89"  }};

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


void softwareMqttInit(void)
{
    MQTT_Init();
    
    xTaskCreate( softwareMqttTask           ,
                 (const char*) "Mqtt"  ,
                 configMINIMAL_STACK_SIZE,
                 NULL                    ,
                 SOFTWAREMQTT_TASK_PRIORITY ,
                 NULL                     );
}

/******************************************************************************
 *                                APLICA��O
 ******************************************************************************/
static void softwareMqttTask(void *params)
{
    LEDS_Init();
    SERIAL_Init();

#ifdef VERBOSE
    SERIAL_Print("\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("* APLICA��O DE EXEMPLO TELIT *\r\n");
    SERIAL_Print("*          SOFTWARE MQTT       *\r\n");
    SERIAL_Print("******************************\r\n");
    SERIAL_Print("\r\n");
#endif

    /**********************************************
     * Aguarda o Wifi ser iniciado com sucesso.
     **********************************************/
    SERIAL_Print("Iniciando...");
    if ( GSM_Init(DEFAULT_GSM_APN, DEFAULT_GSM_PROTOCOL) )
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

        if ( !GSM_IsConnected() )
        {
#ifdef VERBOSE
            SERIAL_Print("Conectando...");
#endif

            if ( GSM_Connect() )
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
            if (! MQTT_IsConnected())
            {
                gu8_cid = MQTT_Connect(MQTT_DEFAULT_ADDRESS, MQTT_DEFAULT_PORT, MQTT_DEFAULT_CLIENT_ID, MQTT_DEFAULT_USR, MQTT_DEFAULT_PWD);
            }
            else if ( gu8_cid != 0xFF )
            {
#ifdef VERBOSE
                SERIAL_Println("MQTT Connect OK");

#endif          
                sendMessages();
            }
            else
            {
#ifdef VERBOSE
                SERIAL_Println("MQTT Connect ER");
#endif
            }
        }


        vTaskDelay(SOFTWAREMQTT_TASK_DELAY);
    }

    MQTT_Deinit();
    vTaskDelete(NULL);
}

static void sendMessages(void)
{
    uint8_t i;

    for(i = 0; i < NUM_MAX_KEY_VALUE; i++)
    {
        if (  MQTT_Publish(gu8_cid, g_msgs[i][VARIABLE], strlen(g_msgs[i][VALUE]), g_msgs[i][VALUE], 1, false) )
        {
#ifdef VERBOSE
            SERIAL_Println("MQTT Publish OK");
#endif                    
        }
        else
        {
#ifdef VERBOSE
            SERIAL_Println("MQTT Publish ER");
#endif
        }
    }
}


void SERIAL_ReadCallback(uint8_t data)
{

}
