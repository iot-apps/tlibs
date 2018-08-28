
#include <asf.h>

#include "serial.h" // DEBUG
#include "gpsGeofenceSMS.h"


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

    gpsGeofenceSMSInit();       // Inicia tarefa da aplicação (FreeRTOS).
	
    vTaskStartScheduler();  // Inicia agendador de tarefas (FreeRTOS).

	while (true) { }
}
