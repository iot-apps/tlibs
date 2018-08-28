
#include <asf.h>

#include "serial.h" // DEBUG
#include "logger.h"


int _read (int file, void *buffer, size_t size);
int _write(int file, void *buffer, size_t size);


int _read(int file, void *buffer, size_t size)
{
    return 0;
}

int _write(int file, void *buffer, size_t size)
{
    SERIAL_Write(buffer, size);
    
    return 0;
}

/******************************************************************************
 *                                PONTO DE ENTRADA
 ******************************************************************************/
int main (void)
{
 	system_init();          // Inicia sistema (ASF).

    loggerInit();       // Inicia tarefa da aplica��o (FreeRTOS).
	
    vTaskStartScheduler();  // Inicia agendador de tarefas (FreeRTOS).

	while (true) { }
}
