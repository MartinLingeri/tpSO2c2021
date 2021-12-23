/*
 * serializaciones.h
 *
 *  Created on: 8 oct. 2021
 *      Author: utnso
 */

#ifndef SRC_SERIALIZACIONES_H_
#define SRC_SERIALIZACIONES_H_

#include "utils.h"

t_buffer* serializar_iniciar(uint32_t pid);
t_buffer* serializar_delete(uint32_t pid);
t_buffer* serializar_crear_sem(uint32_t pid, unsigned int valor, char* nombre);
t_buffer* serializar_wait_sem(uint32_t pid, char* nombre);
t_buffer* serializar_post_sem(uint32_t pid, char* nombre);
t_buffer* serializar_delete_sem(uint32_t pid, char* nombre);
t_buffer* serializar_usar_io(uint32_t pid, char* nombre);/*
t_buffer* serializar_memalloc(uint32_t pid, int size);
t_buffer* serializar_memfree(uint32_t pid, int32_t direccion);
t_buffer* serializar_memread(uint32_t pid, int32_t origen, int size);
t_buffer* serializar_memwrite(uint32_t pid, int32_t destino, int size, void* origen);*/

char* deserializar_id(int socket);
int deserializar_rta(int socket);
int32_t deserializar_rta_memalloc(int socket);

#endif /* SRC_SERIALIZACIONES_H_ */
