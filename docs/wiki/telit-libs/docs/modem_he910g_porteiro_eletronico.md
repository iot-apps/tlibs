#Exemplo Porteiro Eletrônico

* [Arduino M0 Pro - XE910 - Porteiro Eletrônico](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_HE910G_Porteiro_Eletronico\Telit_Modem_HE910G_Porteiro_Eletronico.ino)
* [ASF/FreeRTOS - XE910 - Porteiro Eletrônico](asf\examples\he910\PorteiroEletronico.rar)

##Descrição

*Esse exemplo mostra como criar um sistema que permite ao acionar um botão a realização de uma ligação para um numero predefinido.*


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

*A configuração do número a ser realizado a ligação devera ser configurado manualmente através da alteração de constantes no código:* 

```C++
#define PHONE_NUMBER  "+5583-número do telefone-"
```

