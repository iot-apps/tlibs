#ATMEL STUDIO + ASF + FreeRTOS

## Sobre o Atmel Studio

Atmel Studio é a IDE de desenvolvimento baseada no Visual Studio, para construção de firmwares para os microcontroladores das famílias AVR e ARM Cortex M da Microchip.



##Sobre o ASF - Advanced Software Framework

O ASF (Advanced Software Framework) é o framework da Microchip que integra drivers, serviços e sistema operacional, integrando-os com o firmware em desenvolvimento de maneira simples.

##Sobre o FreeRTOS

Sistema operacional de tempo real para microcontroladores.

##Documentações Úteis

* [Application Note - Getting Started with FreeRTOS on SAM
D20/D21/R21/L21/L22](http://ww1.microchip.com/downloads/en/AppNotes/Atmel-42138-Getting-Started-with-FreeRTOS-on-SAM-D20-D21-R21-L21-L22_ApplicationNote_AT03664.pdf)
* [ASF - FreeRTOS](https://www.freertos.org/FreeRTOS-Plus/BSP_Solutions/Atmel/asf.html)
* [ASF - FreeRTOS Workflow](https://www.freertos.org/FreeRTOS-Plus/BSP_Solutions/Atmel/workflow.html)

##Criando um Projeto Atmel Studio com ASF e FreeRTOS

Para criar um novo projeto utilizando o FreeRTOS é recomendado um estudo preliminar sobre o SO e seguir o tutorial

[Application Note - Getting Started with FreeRTOS on SAM
D20/D21/R21/L21/L22](http://ww1.microchip.com/downloads/en/AppNotes/Atmel-42138-Getting-Started-with-FreeRTOS-on-SAM-D20-D21-R21-L21-L22_ApplicationNote_AT03664.pdf)

**Obs.:** Para possibilitar a programação do Arduino M0 Pro através do Atmel Studio, é necessário efeturar pelo menos uma vez o procedimento de gravação do bootloader do Arduino através da IDE do Arduino, seguindo os passos:

- Abrir a IDE do Arduino
- ...
- Gravar o Bootloader
