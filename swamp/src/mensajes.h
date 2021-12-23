#ifndef MENSAJES_H_
#define MENSAJES_H_

#include "manejo.h"
#include "logs.h"


void recibir_mensaje_read(int socket_memoria);
t_buffer* serializar_rta_read(void* data_a_enviar);

void recibir_mensaje_write(int socket_memoria);
t_buffer* serializar_rta_write(uint32_t rta);

void recibir_eliminar(int socket_memoria);
char* recibir_tipo_asignacion(int socket_memoria);

#endif /* MENSAJES_H_ */
