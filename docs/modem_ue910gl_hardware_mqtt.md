#Exemplo Sistema MQTT

* [Arduino M0 Pro - XE910 - MQTT Hardware](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_HARDWARE_MQTT\Telit_Modem_UE910GL_HARDWARE_MQTT.ino)
* [ASF/FreeRTOS - XE910 - MQTT Software](asf\examples\he910\SoftwareMqtt.rar)

##Descrição

*Publica variáveis em sistema MQTT, por comandos AT de MQTT ou AT de Socket para modem: Cria rotinas que 
leem uma fila de dados a serem enviados para o portal MQTT da Telit,em pares "nome da da variável" 
e "valor", e para cada item executa o post de variáveis por MQTT embutido nos módulos.*


##Configurações de Hardware

*Esse exemplo foi criado para utilização do kit EVK2 da Telit integrado com o arduino M0 PRO.
Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:*
 
Pino Arduino M0 Pro | Pino EVK2
:------------------:|:----------:
        GND         | GND    
         10         |C104/RXD
         11         |C103/TXD
         09         |RESET   

![](images/arduino.png?400)

*As descrições dos pinos utilizados na placa EVK2 podem ser observadas documento: [EVK2 User Guide](https://www.telit.com/wp-content/uploads/2017/09/1vv0300704_EVK2_User_Guide_Rev21.pdf)*

##Configuração de Software

*As configurações de operadora, protocolo, servidor e afins deverão ser configurados manualmente através da alteração de constantes no código:* 

```C++
#define GPRS_OI           "gprs.oi.com.br" 	//Operadora do chip utilizado
#define GPRS_PROTOCOL     "IP"				//Protocolo de internet
#define ENDPOINT          "api-dev.devicewise.com:1883"
#define APP_TOKEN         "pLh5olDkMVIIG8Pd"
```
