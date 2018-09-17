#Exemplo DTMF Commands

* [Arduino M0 Pro - XE910 - DTMF Commands](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_DTMFCommands\Telit_Modem_UE910GL_DTMFCommands.ino)

##Descrição

*Esse exemplo demonstra como criar um sistema com interface por chamadas telefonica(chamada de voz), 
exibindo um menu e apresentando respostas por audio e permitindo o envio de comandos por DTMF (pelo teclado numerico).*

 
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

*Para o uso de audio pre-gravados pelo modem da telit para respostas, é necessario que os arquivos de audio 
esteja armazenado na memoria do modulo no formato wav ou pcm. 
O carregamento do arquivo é realizado através do aplicativo para windows [TELIT AT CONTROLLER](https://www.telit.com/evkevb-drivers/).*

###Carregamento de Arquivos de Aúdio

*Através do terminal do Telit AT Controller, siga os seguintes passos para o carregamento do arquivo.*
 
 *Insira o comando abaixo:*
 
 * AT#ASEND="file.wav",320000

*Nota: O primeiro parametro é o nome do arquivo a ser amazenado na memoria do modulo e 
deve ser escrito junto ao nome a extensão do arquivo "wav". O segundo parametro é o 
tamanho do arquivo a ser armazenado em bytes. Click com o botão direito no arquivo, va em 
"Propiedades" na caixa de dialogo do Windows e observe o tamanho exato do arquivo em  bytes.*

*Após o comando aguarde a resposta do prompt "CONNECT" prompt, depois da resposta, pode-se enviar o 
arquivo de audio WAV usando a opção "Send File" do ferramente de terminal "Telit AT Controller" (use RAW ASCII protocol).* 
*Se a transferência do arquivo for completada com sucesso, o modulo retornara a messagem "OK".*
*Se o arquivo WAV foi armazenado corretamente, o modulo retorna a messagem "#ASENDEV: 0".* 

 *Mais comandos e respostas do modulo que possam ser uteis:*
 
 * AT#ALIST <- Comando: lista os arquivos de audio armazenados no modulo.
 * -->#ALIST: (filename),(filesize),(crc)  <- Parametros: (filename) - nome do arquivo. (filesize) - Tamanho em bytes. (crc) - Endereço de memoria armazenado
 * AT#APLAY=1,1,"file.wav" <- Comando: executa o audio no link do canal de voz.
 * -->OK   <- Notificação que o comando foi bem suscedido. 
 * -->#APLAYEV: 0 <- Notificação que o audio chegou ao fim de sua execução.
 * AT#ADELF="file.wav" <- deleta o arquivo de audio selecionado, armazenado na memoria interna do modulo.
 * -->OK
 
 *Nota: Arquivos de audio editado podem possivelmente gerar ERRO no momento da execução. Ex: (AT#APLAY=1,1,"file.wav")  retorna "ERRO" e não executa o audio.*

###Arquivos de audio

*Arquivos de audio utilizados no exemplo.*

* [1-dtmf.wav](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_DTMFCommands\1-dtmf.wav)
* [2-dtmf.wav](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_DTMFCommands\2-dtmf.wav)
* [menu-dtmf.wav](arduino\Telit_libs\examples\Telit_Modem\Telit_Modem_UE910GL_DTMFCommands\menu-dtmf.wav)
 