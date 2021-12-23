#ifndef SRC_INIT_MEMORIA_H_
#define SRC_INIT_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>

#include"paginacion.h"

#define MODULENAME "MEMORIA"

typedef struct{
	uint32_t TAMANIO;
	uint32_t TAMANIO_PAGINA;
	char* ALGORITMO_REEMPLAZO_MMU;
	char* TIPO_ASIGNACION;
	uint32_t MARCOS_POR_PROCESO;
	uint32_t CANTIDAD_ENTRADAS_TLB;
	char* ALGORITMO_REEMPLAZO_TLB;
	uint32_t RETARDO_ACIERTO_TLB;
	uint32_t RETARDO_FALLO_TLB;
	char* PATH_DUMP_TLB;

	uint32_t PUERTO;
	uint32_t IP;
}t_config_memoria;

t_log* logger;
t_config_memoria* cfg;

t_config_memoria* initialize_cfg();
bool init();
bool cargar_configuracion(char* path);
void cerrar_programa();
bool cargar_memoria();

#endif
