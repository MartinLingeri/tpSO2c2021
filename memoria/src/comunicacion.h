#ifndef SRC_COMUNICACION_H_
#define SRC_COMUNICACION_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <commons/config.h>
#include <conexiones.h>

#include "utils.h"
#include "init_memoria.h"

typedef struct{
	uint32_t pid;
	int size;
}t_memalloc;

typedef struct{
	uint32_t pid;
	uint32_t dir_log;
}t_memfree;

typedef struct{
	uint32_t pid;
	int32_t dir_log;
	int size;
	void* data;
}t_memwrite;

typedef struct{
	uint32_t pid;
	uint32_t dir_log;
	int size;
}t_memread;

int server_escuchar(char* server_name, int server_socket);
void mensaje_read_swap();
void mensaje_write_swap();
t_buffer* serializar_read_swap();
t_buffer* serializar_write_swap();

t_memalloc* recibir_memalloc(int socket_carpincho);
t_memfree* recibir_memfree(int socket_carpincho);
t_memwrite* recibir_memwrite(int socket_carpincho);
t_memread* recibir_memread(int socket_carpincho);
uint32_t recibir_suspender_carpincho(int socket_carpincho);
void responder_dirLogica(int cod_op, int32_t dirLog, int socket);
t_buffer* serializar_dirLogica(int32_t dirLog);
void responder_data_leida(int cod_op, void* data, uint32_t tamanioData, int socket);
t_buffer* serializar_data_leida(void* data, uint32_t tamanioData);
t_buffer* serializar_id(char* nombre);
t_buffer* serializar_respuesta(int rta);
void responder(int cod_op, int respuesta, int socket);
uint32_t deserializar_iniciar(int socket_carpincho);

#endif
