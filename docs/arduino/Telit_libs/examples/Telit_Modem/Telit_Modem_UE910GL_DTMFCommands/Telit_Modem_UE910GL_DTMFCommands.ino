/**
 * @file Telit_Modem_UE910GL_DTMFCommands.ino
 *
 * HE910/UE910/UL865/UE866 telit modem		\n
 * EVK2 \n
 * Arduino M0
 *
 * @brief Exemplo sistema de comandos com DTMF
 * 
 * Esse exemplo demonstra como criar um sistema com interface por chamadas telefonica(chamada de voz),
 * exibindo um menu e apresentando respostas por audio e permitindo o envio de comandos por DTMF (pelo teclado numerico); 
 *  
 * @version 1.00 
 */
/*
 * INSTRUÇÕES DE UTILIZAÇÃO DO EXEMPLO
 * 
 * Esse exemplo foi criado para utilização do kit EVK2 da Telit integrado com o arduino M0 PRO.
 * Para correto funcionamento a conexão das placas devem ser de acordo com a seguinte pinagem:
 *	Pino Arduino M0 Pro		  Pino EVK2
 *			GND					GND
 *			10					C104/RXD
 *			11					C103/TXD
 *			09					RESET
 * 
 *
 * INSTRUÇÕES PARA UTILIZAÇÃO DE ARQUIVOS DE AUDIO 
 * 
 * Para o uso de audio pre-gravados pelo modem da telit para respostas, é necessario que os arquivos de audio
 * esteja armazenado na memoria do modulo no formato wav ou pcm.
 * O carregamento do arquivo é realizado através do aplicativo para windows TELIT AT CONTROLLER, que pode ser encontrado
 * junto aos drivers do kit EVK2 em: https://www.telit.com/evkevb-drivers/
 * 
 * Visite site https://www.????????????/ para mais informações sobre como utilizar adequadamente e o app Telit AT Controller
 * e sobre duvidas da conexão entre o computador e o EVK2
 * 
 * CARREGAMENTO DE ARQUIVOS DE AUDIO TELIT AT CONTROLLER
 * 
 * Através do terminal do Telit AT Controller, siga os seguintes passos para o carregamento do arquivo.
 * 
 * Insira o comando abaixo. 
 * AT#ASEND="file.wav",320000
 ***** Nota:
 ***** O primeiro parametro é o nome do arquivo a ser amazenado na memoria do modulo.
 ***** Deve ser escrito junto ao nome a extensão do arquivo "wav".
 ***** O segundo parametro é o tamanho do arquivo a ser armazenado em bytes. Click com o botão direito no arquivo,
 ***** va em "Propiedades" na caixa de dialogo do Windows e observe o tamanho exato do arquivo em  bytes.
 * 
 * Após o comando aguarde a resposta do prompt "CONNECT" prompt, depois da resposta, você pode enviar o arquivo de
 * audio WAV usando a opção "Send File" do ferramente de terminal do Telit AT Controller (use RAW ASCII protocol).
 * Se a transferência do arquivo for completada com sucesso, o modulo retornara a messagem "OK". 
 * Se o arquivo WAV foi armazenado corretamente, o modulo retorna a messagem "#ASENDEV: 0". 
 * 
 * 
 * * Mais comandos e respostas do modulo que possam ser uteis
 * AT#ALIST <- Comando: lista os arquivos de audio armazenados no modulo.
 * #ALIST: <filename>,<filesize>,<crc>  <- Parameter: <filename> - file name, string type. <filesize> - file size in bytes
 *                                                    <crc> - CRC16 poly (x^16+x^12+x^5+1) of file in hex format
 * AT#APLAY=1,1,"file.wav" <- Comando: executa o audio no link do canal de voz.
 * OK   <- Notificação que o comando foi bem suscedido. 
 * #APLAYEV: 0 <- Notificação que o audio chegou ao fim de sua execução.
 * AT#ADELF="file.wav" <- deleta o arquivo de audio selecionado, armazenado na memoria interna do modulo.
 * OK
 * 
 * Note: Arquivos de audio editado podem possivelmente gerar ERRO no momento da execução 
 * ex: (AT#APLAY=1,1,"file.wav")  retorna "ERRO" e não executa o audio
 * 
 * REF:
 * https://www.telit.com/wp-content/uploads/2017/09/Telit_Audio_Settings_Application_Note__r6.pdf
 * https://www.telit.com/wp-content/uploads/2017/09/Telit_3G_Modules_AT_Commands_Reference_Guide_r11-1.pdf
 * 
 */
#include <AT.h>
#include <XE910.h>

#define SERIAL_BUFFER_SIZE 200

#include <Arduino.h>
#include "wiring_private.h"

#define RESET_PIN       9

#define COMMAND_LED_ON            "led on"
#define COMMAND_LED_OFF           "led off"
#define COMMAND_LED_ON_RESP       "led on ok!"
#define COMMAND_LED_OFF_RESP      "led off ok!"

Uart Serial2 (&sercom1, 11, 10, SERCOM_RX_PAD_0, UART_TX_PAD_2);
XE910 gsm(&Serial2);

void GetBuffer (char* resp);

void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

void setup()
{
     Serial.begin(9600);
     Serial2.begin(115200);   //modulo GSM

     pinPeripheral(10, PIO_SERCOM);
     pinPeripheral(11, PIO_SERCOM);

     delay(1000);
     
     pinMode(LED_BUILTIN, OUTPUT);
     digitalWrite(LED_BUILTIN, LOW);
     
     Serial.println("Testando modulo GSM Message...");
};

void loop()
{
  char resp[SERIAL_BUFFER_SIZE] = {0};
  bool call=false;
  Serial.println("Inicializando...");
  /*
   * CONFIG AUTO ANSWER CALL OPTION
   * //Sets the number of rings required before device automatically answers an incoming call
   * // n--> number of rings
   * gsm.RingAutoAnswer(n); 
   */
  if ( gsm.ConfigureDTMF() ) {  
                 
      Serial.println("Cofiguração DTMF: OK!");
       
      delay(10);

      Serial.println("Esperando ligação!");  

      while (1)
      {
        GetBuffer (resp);
        
        if (NULL != strstr(resp,"RING")) //A incoming call is indicated returning RING mensage
        {
          gsm.ReceiveCall();    //Answer a incoming call
          call = true;
          gsm.PlayAudio("menu-dtmf.wav");   //Play audio file stored in the modem, is necessary to write the file name and specify the file extension
        }
        
        while(call==true)
        { 
          char command = 0;
          GetBuffer (resp);
          /*
           Receiving of a DTMF tone is pointed out with anunsolicited message through
           AT interface in the following format: #DTMFEV: x with x as the DTMF digit
           */
          if (NULL != strstr(resp,"#DTMFEV:"))
          {
            for (int i=0; i< sizeof(resp) ; i++)
            {
              if (isDigit (resp[i]) )
              {
                command = resp[i];
                break;
              }
            }
          }
          /*
           * If there's no active connection it returns NO CARRIER
           */
          if (NULL != strstr(resp,"NO CARRIER"))   
          {
            call = false;
            Serial.println("Chamada terminada");
            Serial.println("Esperando ligação!");
          }
          switch (command)
          {
            case '1':
              digitalWrite(LED_BUILTIN, HIGH);
              Serial.println("Led ON!");
              gsm.PlayAudio("1-dtmf.wav");
            break;
            case '2':
              digitalWrite(LED_BUILTIN, LOW);
              Serial.println("Led OFF!");
              gsm.PlayAudio("2-dtmf.wav");
            break;
            case '3':
              gsm.PlayAudio("menu-dtmf.wav");
            break;
            default :
            break;
          }
        } 
      }
    }
  
};

//Read the incoming buffer from Telit module board
void GetBuffer (char* resp)
{
  char buff [SERIAL_BUFFER_SIZE] = {0};
  int i=0;
  while (Serial2.available()>0)
  {          
    buff[i] = Serial2.read();
    i++;
    resp[i] = '\0';
  }
  strcpy(resp , buff);
  delay (10);
}

