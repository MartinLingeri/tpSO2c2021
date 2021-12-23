#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<stdint.h>
#include<commons/config.h>
#include<commons/string.h>
#include<conexiones.h>

t_log* logger;
t_config* config_carpincho;

typedef enum{
    NUEVO_PROCESO,
    FIN_PROCESO,
    SEM_CREATE,
    SEM_POST,
    SEM_WAIT,
    SEM_DELETE,
    IO,
    MEM_ALLOC,
    MEM_READ,
    MEM_WRITE,
    MEM_FREE,
    CONEXION_KERNEL,
    CONEXION_MEMORIA,
    FUNCION_INHABILITADA,
	ERROR_OP
}situacion;

void logear(int situacion, int dato, char* otroDato);

//===== COSAS DE CONEXIONES =======
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo);

int recibir_respuesta(int socket_cliente);
#endif /* UTILS_H_ */
