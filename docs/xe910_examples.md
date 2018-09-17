# Exemplos GSM HE910

##Túnel TCP

*Sistema de logger baseado em modem: Tasks contínuas (ASF/FreeRTOS) ou Loop infinito (Arduino) que repassa o tráfego de uma das portas seriais para um socket aberto com um servidor remoto, através dos comandos AT para sockets. Bufferiza os bytes que são recebidos enquanto a conexão não estava ativa. As configurações de servidor, baudrate e afins são manuais, através de constantes no código. O sistema verifica que a conexão caiu (contexto com a APN ou o socket) e tenta continuamente uma reconexão. Permite também o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.*

* [Arduino - HE910 - Túnel TCP](example_arduino_he910_tunnel-tcp)
* [ASF/FreeRTOS - HE910 - Túnel TCP](example_asf_he910_tunnel-tcp)

##Geofence GPS

*Determina o pertencimento de uma latitude/longitude recebida através dos códigos NMEA emitidos automaticamente pelo módulo GPS, pela porta serial, com o interior polígono definido através de um array de coordenadas. Quando o fix, representado pela mensagem $GPGGA, está fora do polígono, envia onda quadrada para um buzzer/falante e acende um LED.*

* [Arduino - HE910 - Geofence GPS](example_arduino_he910_geofence-gps)
* [ASF/FreeRTOS - HE910 - Geofence GPS](example_asf_he910_geofence-gps)

##Geofence LBS

*Utiliza os comandos AT#MONI e AT#AGPSSND para obter a posição por triangulação as ERBs (antenas da rede celular), com análise de polígono considerando a grande imprecisão da posição obtida. Os comandos são simples, utilizam: AT#MONI=7 (Aguarda), AT#MONI, AT#AGPSSND (recebe lat/long). Devido a imprecisão, o status de dentro e fora do polígono pode ser gradual, ao invés de binário / tudo ou nada.*

* [Arduino - HE910 - Geofence LBS](example_arduino_he910_geofence-lbs)
* [ASF/FreeRTOS - HE910 - Geofence LBS](example_asf_he910_geofence-lbs)

##MQTT Nativo

*Publica variáveis em sistema MQTT, comandos AT de MQTT (método nativo) para modem: Cria task (FreeRTOS) ou rotina (Arduino) que lê uma fila de dados a serem enviados para o portal MQTT da Telit, em pares "nome da da variável" e "valor", e para cada item executa os comandos AT de post de variáveis por MQTT embutido nos módulos.*

* [Arduino - HE910 - MQTT Nativo](example_arduino_he910_mqtt-native)
* [ASF/FreeRTOS - HE910 - MQTT Nativo](example_asf_he910_mqtt-native)

##MQTT Software

*Publica variável em sistema MQTT, comandos AT de Socket (método por software) para modem: Criar task (FreeRTOS) ou rotina (Arduino) que lê uma fila de dados a serem enviados para o portal MQTT da Telit, em pares "nome da da variável" e "valor", e para cada item efetua chamadas a uma biblioteca para o protocolo MQTT, se conectando ao servidor através dos comandos AT de socket do módulo. Esta implementação utiliza a biblioteca [Paho - Embedded MQTT C/C++ Client Libraries](http://www.eclipse.org/paho/clients/c/embedded/).*

* [Arduino - HE910 - MQTT Software](example_arduino_he910_mqtt-software)
* [ASF/FreeRTOS - HE910 - MQTT Software](example_asf_he910_mqtt-software)

##Geofence MQTT

*Associação do sistema de geofence com publicação de posições lidas a partir do GPS/LBS em um servidor MQTT.*

* [Arduino - HE910 - Geofence MQTT](example_arduino_he910_geofence-mqtt)
* [ASF/FreeRTOS - HE910 - Geofence MQTT](example_asf_he910_geofence-mqtt)

##Comandos SMS

*Comunicação por SMS: task ou rotina que lê continuamente as mensagens SMS recebidas pelo módulo, lendo seu conteúdo e executando comandos conforme o texto enviado pela mensagem, com resposta por SMS.*

* [Arduino - HE910 - Comandos SMS](example_arduino_he910_sms-command)
* [ASF/FreeRTOS - HE910 - Comandos SMS](example_asf_he910_sms-command)

##Geofence SMS

*Associação entre o sistema de geofence com envio de posições lidas a partir do GPS/LBS através do sistema de comunicação por SMS.*

* [Arduino - HE910 - Geofence SMS](example_arduino_he910_geofence-sms)
* [ASF/FreeRTOS - HE910 - Geofence SMS](example_asf_he910_geofence-sms)

##Menu de Voz DTMF

*Sistema de atendimento de ligações com recebimento de comandos por DTMF e resposta/menu através de sons pré-gravados. Os comandos de detecção de DTMF possuem utilização simples e para tocar os sons basta chamar AT#APLAY=1,"diretorio","nome_do_arquivo". Ou AT#SAMR. Implementa menus de exemplo e em alguns deles o modem envia mensagens de SMS conforme o código digitado e envia variáveis para o servidor MQTT conforme o código digitado.*

* [Arduino - HE910 - Menu de Voz DTMF](example_arduino_he910_voice-menu-dtmf)
* [ASF/FreeRTOS - HE910 - Menu de Voz DTMF](example_asf_he910_voice-menu-dtmf)

##Porteiro Eletrônico

*Sistema porteiro eletrônico: se um botão for pressionado, liga para um telefone predefinido.*

* [Arduino - HE910 - Porteiro Eletrônico](example_arduino_he910_intercom-electronic)
* [ASF/FreeRTOS - HE910 - Porteiro Eletrônico](example_asf_he910_intercom-electronic)
