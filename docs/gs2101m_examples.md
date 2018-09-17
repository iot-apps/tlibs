# Exemplos Wifi GS2101M

##Túnel TCP

*Sistema de logger baseado em modem: Tasks contínuas (ASF/FreeRTOS) ou Loop infinito (Arduino) que repassa o tráfego de uma das portas seriais para um socket aberto com um servidor remoto, através dos comandos AT para sockets. Bufferiza os bytes que são recebidos enquanto a conexão não estava ativa. As configurações de servidor, baudrate e afins são manuais, através de constantes no código. O sistema verifica que a conexão caiu (contexto com a APN ou o socket) e tenta continuamente uma reconexão. Permite também o fluxo de dados servidor->dispositivo, refletido na porta serial. Um tunel de porta serial por TCP.*

* [Arduino - GS2101M - Túnel TCP](example_arduino_gs2101m_tunnel-tcp)
* [ASF/FreeRTOS - GS2101M - Túnel TCP](example_asf_gs2101m_tunnel-tcp)

##MQTT Nativo

*Publica variáveis em sistema MQTT, comandos AT de MQTT (método nativo) para modem: Cria task (FreeRTOS) ou rotina (Arduino) que lê uma fila de dados a serem enviados para o portal MQTT da Telit, em pares "nome da da variável" e "valor", e para cada item executa os comandos AT de post de variáveis por MQTT embutido nos módulos.*

* [Arduino - GS2101M - MQTT Nativo](example_arduino_gs2101m_mqtt-native)
* [ASF/FreeRTOS - GS2101M - MQTT Nativo](example_asf_gs2101m_mqtt-native)

##MQTT Software

*Publica variável em sistema MQTT, comandos AT de Socket (método por software) para modem: Criar task (FreeRTOS) ou rotina (Arduino) que lê uma fila de dados a serem enviados para o portal MQTT da Telit, em pares "nome da da variável" e "valor", e para cada item efetua chamadas a uma biblioteca para o protocolo MQTT, se conectando ao servidor através dos comandos AT de socket do módulo. Esta implementação utiliza a biblioteca [Paho - Embedded MQTT C/C++ Client Libraries](http://www.eclipse.org/paho/clients/c/embedded/).*

* [Arduino - GS2101M - MQTT Software](example_arduino_gs2101m_mqtt-software)
* [ASF/FreeRTOS - GS2101M - MQTT Software](example_asf_gs2101m_mqtt-software)

