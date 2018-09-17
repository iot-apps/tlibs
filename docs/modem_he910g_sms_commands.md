#Exemplo SMS Commands

* [Arduino M0 Pro - XE910 - SmsCommands](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_HE910G_SMSCommands\Telit_Modem_HE910G_SMSCommands.ino)
* [ASF/FreeRTOS - XE910 - SmsCommands](asf\examples\he910\SmsCommands.rar)

##Descrição

*Este exemplo mostra como executar comandos através de Comunicação por SMS: 
Tasks ou rotinas que leem continuamente as mensagens SMS recebidas pelo módulo, 
lendo seu conteúdo e executando comandos conforme o texto enviado pela mensagem e com resposta(feedback) por SMS.*


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

*A configuração do número a ser enviado o feedback, devera ser configurado manualmente através da alteração de constantes no código:* 

```C++
#define CELL_NUMBER               "+83-número do telefone-"
```


