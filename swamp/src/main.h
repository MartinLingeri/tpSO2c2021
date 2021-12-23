#ifndef MAIN_H_
#define MAIN_H_

#include "mensajes.h"

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void terminar_programa(int, t_log*, t_config*);
void agregar_frames_a_lista(void* archivo, uint32_t t_swap, uint32_t t_pagina, char* path);

#endif /* MAIN_H_ */
