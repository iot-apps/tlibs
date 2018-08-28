![BLUEMOD+S42](images/Bluemod_S42.jpg)

*Aqui são disponibilizadas todas as informações necessárias para montagem, configuração, testes e utilização do módulo Telit BlueMod+S42.*

##Sobre o Módulo BlueMod+S42

Módulo plug-and-play Bluetooth 4.2 Low Energy

O BlueMod+S42 é um módulo de modo único qualificado de alta performance com Bluetooth 4.2. Ele apresenta baixo consumo de energia e flexibilidade para projetistas como o perfil I/O de terminal para casos de uso de substituição de cabo serial, bem como a maioria dos perfis Bluetooth baseados em GATT por meio de uma implementação genérica do GATT. Parte da família BlueMod + Sx, o BlueMod + S42 é intercambiável com as gerações anteriores sem precisar de novos spins de hardware ou software. Embora seja um módulo de baixo consumo de energia de modo único, ele oferece uma alta taxa de transferência de até 470kbit/s usando o SPP exclusivo da Telit, como o serviço Terminal I/O.

###Características

- Plataforma IOT autônoma ou aplicativos hospedados por Bluetooth
- Módulo qualificado Bluetooth v4.2
- RED, FCC, IC, KC, MIC, RCM e Certificação Anatel
- Poderoso Cortex M4F MCU
- Projetado para aplicações de baixa potência
- Melhor do seu desempenho de intervalo de classe
- Melhor desempenho de rendimento da sua classe
- Cliente genérico GATT ou implementação de servidor
- Comunicações ponto a ponto ou ponto a multiponto
- Perfil de E/S de terminal (semelhante a SPP) para transferência de dados transparente rápida e fácil
- Perfil de E/S de automação para transferência de informações de status digital e analógico
- I/O de terminal e código de amostra de E/S de automação para iOS e Android
- Suporte NFC para emparelhamento simples seguro
- LE Secure Connection com criptografia AES de 128 bits

##Referências

* [Página do Módulo](https://www.telit.com/m2m-iot-products/wifi-bluetooth-modules/bluemods42/)
* [Datasheet](https://www.telit.com/wp-content/uploads/2017/09/Telit_BlueMod_s42_Datasheet_07-3.pdf)
* [BlueEva Hardware Guide](http://www.telit.com/wp-content/uploads/2017/09/BlueModS42_Hardware_User_Guide_r7.pdf)
* [Comandos AT](http://www.telit.com/wp-content/uploads/2017/09/BlueMod_S42_Central_AT_Command_Reference_r4.pdf)

##Sobre o Kit de Desenvolvimento BlueEva+S

![](images/bms42_hardware.png?400)

###Referências

* [BlueEva+S/Central User Guide](https://www.telit.com/wp-content/uploads/2018/01/BlueEvaS42_Central_User_Guide_r1.pdf)
* [BlueEva Software Guide](http://www.telit.com/wp-content/uploads/2018/02/Telit_BlueModS42_Software_User_Guide_r2.pdf)
* [Drivers](https://www.telit.com/evkevb-drivers/)

##Configuração do Ambiente de Hardware

*O ambiente de hardware utilizado para os exemplos consiste na integração da placa Arduino M0 Pro com o Kit de Desenvolvimento BlueEva+S.*

*Os procedimentos de setup inicial serão listados a seguir:*

**1 - Baixar e instalar os drives USB's do link: [Drivers](https://www.telit.com/evkevb-drivers/). Devem ser instalados os drivers destacados em azul, conforme indicado na imagem abaixo.**

![Drivers](images/drivers.png?400)

**2 - Conectar a placa BlueEva+S através de seu conector miniUSB na porta USB do computador, conectar a fonte de alimentação na placa base (EVK2) e pressionar o botão de on por 5s, como indicado na imagem abaixo.**

**3 - Para testar a comunicação com o módulo através de comandos AT, deve-se instalar o software serial Telit AT Controller, disponível no mesmo link usado anteriormente pra baixer os drivers: [Drivers](https://www.telit.com/evkevb-drivers/).**

![](images/controller1.png?400)

*Tela inicial do Telit AT Controller:*

![](images/controller2.png?400)

*Para iniciar os testes com o módulo é preciso definir as configurações de conexão:*

![](images/controller3.png?400) 

*Configurada a conexão, basta clicar então no botão de "Connect", que é o botão da esquerda no meio da tela. Dando tudo certo e com o módulo conectado, são exibidas uma série de informações tais como endereço Bluetooth, Fabricante, Modelo e versão de Firmware no centro da tela do programa, indicando que o mesmo está comunicando e recebendo dados do módulo. Veja o resultado a conexão com sucesso na figura abaixo.*

 ![](images/telit_at_controller_bt.png?400)
 
**4 - Para atualizar o firmware acesse a [Download Zone](https://www.telit.com/support-training/download-zone/) da Telit e va em Software > Short Range > Software Releases Short Range > Bluetooh > BlueMod+S42 e realize o download da versão mais recente do BM+S42_xxx_FWupdate.exe.**
 
 ![](images/firmware_download_BM.png?400)

*Siga a instruções as instruções abaixo para a atualização do firmware:*

* Configure o jumper J2 para a posição 2-3 para ativar o bootloader ao iniciar.
* Conecte o modulo BlueEva+S42 através da porta USB ao PC (Tenha certeza que os driver estão instalados). Se o modulo já estiver conectado reinicie o modulo pressionando o botão de reset da placa.
* Execute o BM+S42_xxx_FWupdate.exe 
 
 ![](images/iot_updater.png?400)

* Selecione a porta e click em update, aguarde a atualização ser concluida, e click em finish.
* Após o termino da atualização coloque o jumper J2 novamente na posição 1-2 e reinicie o modulo.
* Pelo o terminal do "Telit AT Controller" envie os comando AT&F para restaurar as configurações de fabrica, e comece a usar.
###Integração com Arduino M0 Pro

*A Conexão do BlueEva+S com o arduino deve ser de acordo com a seguinte pinagem:*

Pino Arduino M0 Pro | Pino BlueEva+S	
:------------------:|:----------:
        3.3V        | ext. PWR  
		GND         | GND    
		 9	        |X322/RESET  
		 10         |X324/UART-RXD|
         11         |X323/UART-TXD| 

		 
![](images/ard_bm.jpg?400)

*As descrições dos pinos utilizados na placa BlueEva+S podem ser observadas documento: [BlueEva+S/Central User Guide](https://www.telit.com/wp-content/uploads/2018/01/BlueEvaS42_Central_User_Guide_r1.pdf)*
