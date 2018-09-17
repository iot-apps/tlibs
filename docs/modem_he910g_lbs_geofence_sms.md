#Exemplo LBS Geofence SMS

* [Arduino M0 Pro - XE910 - LBS Geofence SMS](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_HE910G_LBS_Geofence_SMS\Telit_Modem_HE910G_LBS_Geofence_SMS.ino)


##Descrição

*Esse exemplo mostra como criar um sistema de geofence com LBS associado a comunicação SMS: 
Através da solicitação por sms "get fix" o modulo envia uma resposta por sms com os dados 
resultantes do geofence.*

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

*As configurações do array de coordenadas do poligono deverão ser configurados manualmente através da alteração de constantes no código.*

```C++
#define CELL_NUMBER               "+83-número do telefone-"

const uint8_t polySides = 4; // Quantidade de lados do poligono.
//                              A,        B,         C,          D
float polyX[polySides] = { -7.107206, -7.108143, -7.108930, -7.107962 }; // X
float polyY[polySides] = { -34.835296, -34.834491, -34.835607, -34.836444 }; // Y
```
