/*
 * test.h
 *
 *  Created on: 15 sep. 2021
 *      Author: utnso
 */

#ifndef SRC_CONEXIONES_H_
#define SRC_CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

#define KERNEL_INHABILITADO -1
#define PROCESO_NO_INICIADO -2
#define PROCESO_FINALIZADO -3
#define NO_EXISTE_SEMAFORO -4
#define NO_EXISTE_IO -5
#define FINALIZADO_POR_DEADLOCK -6
#define ERROR_DE_CONEXION -7
#define EXITO 1
#define ERROR_DE_MEMORIA -8
#define SIN_ESPACIO -9
#define MATE_FREE_FAULT -10
#define MATE_READ_FAULT -11
#define MATE_WRITE_FAULT -12
#define SWAP_FRAME_NULL -12
#define SWAP_PAGINA_NULL -13
#define SWAP_OVERFLOW -14
#define SWAP_LIMITE_FRAMES -15
#define SWAP_READ_FAULT -16

typedef enum op_code
{
	MENSAJE,
	PAQUETE,
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
	SWAP_READ,
	SWAP_WRITE,
	RESPUESTA,
	ENVIO_PID,
	SUSPENDIDO,
	SWAP_READ_RTA,
	SWAP_WRITE_RTA,
	RMEMALLOC,
	SWAP_READ_ERROR,
	ASIGNACION
}op_code;

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

void unaFuncion();
int crear_conexion(char *ip, char* puerto);
int iniciar_servidor(char* ip, char* puerto);
int esperar_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);
void enviar_mensaje(char* mensaje, int socket_cliente, op_code codigo_operacion);

void* serializar_paquete(t_paquete* paquete, int bytes);
void eliminar_paquete(t_paquete* paquete);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente);
t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo);
void enviar_paquete(t_paquete* paquete, int socket_cliente);

t_buffer* serializar_memalloc(uint32_t pid, int size);
t_buffer* serializar_memfree(uint32_t pid, int32_t direccion);
t_buffer* serializar_memread(uint32_t pid, int32_t origen, int size);
t_buffer* serializar_memwrite(uint32_t pid, int32_t destino, int size, void* origen);

#endif /* SRC_CONEXIONES_H_ */
