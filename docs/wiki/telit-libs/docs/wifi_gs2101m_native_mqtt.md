#Exemplo Sistema MQTT Native

* [ASF/FreeRTOS - GS2101M - MQTT Native](asf\examples\gs2101m\NativeMqtt.rar)

##Descrição

*Publica variáveis em sistema MQTT, por comandos AT de MQTT para modem: Cria rotinas que 
leem uma fila de dados a serem enviados para o portal MQTT da Telit,em pares "nome da da variável" 
e "valor", e para cada item executa o post de variáveis por MQTT embutido nos módulos.*


##Configurações de Hardware

*Esse exemplo foi criado para utilização da placa BlueEva+S integrado com a EVB3 da Telit e com o arduino M0 PRO.
Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:*
 
Pino Arduino M0 Pro|Pino EVB3
:-----------------:|:--------:
GND|GND
3.3V|VIN_3V3
10|GPIO0/UART0_RX_M
11|GPIO1/UART0_TX_M
09|EXT_RTC_RESET_N

![](images/ard_gs12.png)

##Configuração de Software

*As configurações de rede(SSID), senha, servidor(endereço ip) e protocolo deverão ser configurados manualmente através da alteração de constantes no código:* 

```C++
#define WIFI_SSID               "yourNetwork" //Nome de sua rede (SSID)
#define WIFI_PASSWORD           "yourPassword" //Senha de sua rede (WPA ou WEP key)

#define MQTT_DEFAULT_ADDRESS     "10.0.0.5" //Ip do server a conectar
#define MQTT_DEFAULT_PORT        1883 //Porta de comunicação do server
#define MQTT_DEFAULT_CLIENT_ID   "nativeMqtt"
```
