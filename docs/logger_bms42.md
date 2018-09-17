#Exemplo Logger Bluetooth BlueMode+S42

* [Arduino M0 Pro- BlueMode+S42 - Logger](arduino\Telit_libs\examples\Telit_Bluetooth\Telit_Bluetooth_BMS42_Logger\Telit_Bluetooth_BMS42_Logger.ino)
* [ASF/FreeRTOS - BlueMode+S42 - Logger](asf\examples\bms42\Logger.rar)

##Descrição

*Este é um exemplo de um sistema de logger baseado em bluetooth de Tasks contínuas (ASF/FreeRTOS) ou Loop infinito (Arduino) 
que repassa o tráfego de uma das portas seriais através de uma conexão bluetooth I/O,
 através dos comandos AT para terminal I/O do modulo.  Permite também o fluxo de dados entre os dispositivo.
 Um tunel de porta serial por bluetooth.*
 
##Configurações de Hardware

*Esse exemplo foi criado para utilização da placa BlueEva+S integrado com o arduino M0 PRO.
 Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:*
 
 Pino Arduino M0 Pro | Pino BlueEva+S	
:------------------: |:----------:
        3.3V         | ext. PWR  
		GND          | GND    
		 9	         |X322/RESET  
		 10          |X324/UART-RXD|
         11          |X323/UART-TXD| 
		 
![](images/ard_bm.jpg?400)

*As descrições dos pinos utilizados na placa BlueEva+S podem ser observadas documento: [BlueEva+S/Central User Guide](https://www.telit.com/wp-content/uploads/2018/01/BlueEvaS42_Central_User_Guide_r1.pdf)*

##Configuração de Software

*Para a comunicação bluetooth neste exemplo é necessário a utilização de um aplicativo de interface para comunicação e transmissão de dados com o BlueMod+S42.*

###Telit "Terminal IO Utility" App

*A telit disponibiliza o aplicativo "Terminal IO Utility" para android e iOS o qual pode ser utilizado 
para estabelecer conexões bluetooth de baixa energia entre um smartphone e o BlueEva+S
Os seguintes QR-Codes proveem o link para download do "Terminal IO Utility".*

![](images/telit_ioT_APP.png)

*O “Terminal IO Utility” App permite o usuario se conectar ao terminal I/O dos dispositivos perifericos
e trocar dados provendo uma simples emulação do terminal.*