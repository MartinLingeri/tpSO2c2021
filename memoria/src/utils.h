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
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<semaphore.h>
#include<pthread.h>
#include<conexiones.h>

//int ultimo_id = 0;

	t_log* logger;
	t_config* config;

int crear_conexion(char* ip, char* puerto);
//void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
//void enviar_paquete(t_paquete* paquete, int socket_cliente);
//void liberar_conexion(int socket_cliente);
//void eliminar_paquete(t_paquete* paquete);

#endif /* UTILS_H_ */
