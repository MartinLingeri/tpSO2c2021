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

t_log* logger;
t_config* config;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum
{
	PAQUETE,
	MENSAJE,
	INIT,
	DELETE,
	S_CREATE,
	S_WAIT,
	S_POST,
	S_DELETE,
	CALL_IO,
	MEMALLOC,
	MEMFREE,
	MEMREAD,
	MEMWRITE,
}op_code;

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
}situacion;

void logear(int situacion, int dato, char* otroDato);

//===== COSAS DE CONEXIONES =======
//TODO ver funciones de conexion crear mensaje enviar paquete params segun modulo template
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

#endif /* UTILS_H_ */
