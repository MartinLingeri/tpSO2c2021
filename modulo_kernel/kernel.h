#ifndef CLIENT_H_
#define CLIENT_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<locale.h>
#include<time.h>
#include<conexiones.h>

#include "utils.h"
#include "logs.h"

uint32_t ultimo_id = 1;
uint32_t semaforo_id = 0;
uint32_t cantidad_io= 0;
t_config* config;

char** array_duraciones;
char** array_dispositivos;

int NUM_THREADS;
t_config* configuracion;

int esperarClientes(int);

void cambiar_estado(int estado_nuevo, t_carpincho* carpincho);
void planificador_largo_plazo(int socket_cliente);
void planificador_corto_plazo();
void planificador_mediano_plazo_suspender();
void planificador_mediano_plazo_ready();
void detectar_deadlock();
int semaforo_tomado(char* sem);
void buscar_deadlock(t_list* lista);

void desbloquear_en_espera_io(t_carpincho* carpincho);
int desbloquear_en_espera_sem(t_carpincho* carpincho);
void desbloquear_todos(t_carpincho* salido_de_io);
int crear_semaforo(t_carpincho* carpincho);
int post_semaforo(t_carpincho* carpincho);
void usar_io(t_io* objetivo, t_carpincho* carpincho);
void finalizar_instruccion_sem(t_carpincho* carpincho, int inicio_exec);

int informar_finalizado(t_carpincho* carpincho);
void finalizar_por_dl(t_carpincho* carpincho);

t_carpincho* buscar_carpincho(char* algoritmo);
double sjf(t_carpincho* c);
double hrrn(t_carpincho* c);

void hacer_tarea(void* args);
void* atender_op_memoria(int retorno, uint32_t rta_memalloc, int cod_op, t_carpincho* carpincho);

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);

#endif /* CLIENT_H_ */
