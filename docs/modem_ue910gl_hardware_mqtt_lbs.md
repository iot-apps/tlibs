#Exemplo Sistema MQTT com LBS

* [Arduino M0 Pro - XE910 - MQTT Hardware LBS](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_HARDWARE_MQTT_LBS\Telit_Modem_UE910GL_HARDWARE_MQTT_LBS.ino)

##Descrição

*Este exemplo mostra como enviar por MQTT (nativo) a localição obtida com tecnlogia LBS, para o portal da Telit.*

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
