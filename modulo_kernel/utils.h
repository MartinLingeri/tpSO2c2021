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
#include<commons/collections/list.h>
#include<semaphore.h>
#include<math.h>
#include<commons/config.h>
#include<commons/temporal.h>
#include<conexiones.h>
#include<pthread.h>

t_log* logger;

pthread_mutex_t estados[7];
pthread_mutex_t existencia_semaforo;
pthread_mutex_t memoria;
pthread_mutex_t conexion_memoria;

sem_t proceso_ready;
sem_t proceso_susp;
sem_t proceso_bloqueado;
sem_t proceso_nuevo;
sem_t listo_para_trabajar;
sem_t listo_para_trabajar_ready;
sem_t listo_para_trabajar_susp;
sem_t multiproc;

typedef enum estado
{
	NEW,
	READY,
	EXEC,
	EXIT,
	BLOCKED,
	SUSPENDED_READY,
	SUSPENDED_BLOCKED,
}estado;

typedef enum casos_log{
	CAMBIO_ESTADO,
	INICIO_PROCESO,
	DEADLOCK_ELIMINA,
	DL_DETECTADO,
	IO_EN_USO,
	IO_LIBERADO,
	SEM_NOEXISTE,
	IO_NOEXISTE,
	SEM_POSTED,
	SEM_WAITED,
	SEM_CREATED,
	SEM_DELETED,
	INICIADO,
	FINALIZADO,
	LMEMALLOC,
	LOPMEM,
	MEM_ENVIO
}casos_log;
/*
typedef struct{
	int retorno;
	void* leido;
}respuesta_memread;
*/
typedef struct{
	op_code instruccion;
	uint32_t pid;
	char* nombre_sem_io;
	uint32_t valor_semaforo;
}t_instruccion;

typedef struct{
	uint32_t pid;
	int size;
	uint32_t dir;
	void* data;
}t_op_memoria;

typedef struct
{
	uint32_t pid;
	estado estado;
	t_instruccion* instruccion;
	double ultima_rafaga;
	double ultimo_estimado;
	double tiempo_llegada;
	int socket_carpincho;
	char* esperando;
	sem_t semaforo;
	t_list* tomando;
}t_carpincho;

typedef struct
{
	uint32_t id;
	char* nombre;
	uint32_t valor;
	t_list* tomado_por;
	t_list* bloqueando;
}t_semaforo;

typedef struct{
	uint32_t duracion;
	uint32_t bloqueado;
	char* nombre;
	pthread_mutex_t semaforo;
}t_io;

t_list* l_new;
t_list* l_ready;
t_list* l_exec;
t_list* l_exit;
t_list* l_blocked;
t_list* l_suspended_ready;
t_list* l_suspended_blocked;

t_list* lista_semaforos;
t_list* lista_dispositivos;

t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo);

t_instruccion* recibir_instruccion(int,int);
t_buffer* deserializar_parcial(int cod_op, int socket);

void responder(int,int,int);
void responder_memalloc(int cod_op, int32_t respuesta, int socket);

double timestamp();

t_buffer* serializar_respuesta(int rta);
t_buffer* serializar_respuesta_memalloc(int32_t rta);
t_buffer* serializar_suspender(uint32_t pid);
int32_t deserializar_rta_memalloc(int socket_carpincho);
int deserializar_rta(int socket_carpincho);
uint32_t  deserializar_iniciar(int socket_carpincho);
t_buffer* serializar_id(char* nombre);
t_op_memoria* recibir_memalloc(int socket_carpincho);
t_op_memoria* recibir_memfree(int socket_carpincho);
t_op_memoria* recibir_memwrite(int socket_carpincho);
t_op_memoria* recibir_memread(int socket_carpincho);

#endif /* UTILS_H_ */
