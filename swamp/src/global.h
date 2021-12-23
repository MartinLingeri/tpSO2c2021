#ifndef GLOBAL_H_
#define GLOBAL_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include "stdbool.h"
#include<fcntl.h>
#include<pthread.h>
#include<sys/socket.h>
#include<sys/mman.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<conexiones.h>

typedef enum casos_log{
	PAGINA_GUARDADA,
	PAGINA_SOLICITADA,
	CARPINCHO_FINALIZADO,
	PAGINA_NULL,
	FRAME_NULL,
	OVERFLOW,
	LIMITE_FRAMES,
	SIN_FRAMES
}casos_log;

t_log* logger;
char* tipo_asignacion;

typedef struct{
	uint32_t numero_frame;
	uint32_t espacio_libre;
	void* inicio;
	char* path_archivo;
	uint32_t pid;
}t_frame;

typedef struct{
	char* path;
	int acceso;
}t_archivo_swap;

typedef struct{
	uint32_t nro_pagina;
	uint32_t nro_pagina_en_mem;
	t_frame* frame;
}t_pagina;

t_list* lista_archivos_swap;
t_list* lista_paginas;
t_list* lista_frames;
int t_pagina_size;
int t_swap;
int marcos_por_carpincho;
uint32_t static_nro_pagina;
uint32_t static_nro_frame;
t_frame* static_frame;
uint32_t static_pid;
char* static_path;

#endif /* GLOBAL_H_ */
