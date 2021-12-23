#ifndef SRC_PAGINACION_H_
#define SRC_PAGINACION_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <commons/string.h>
#include <math.h>
#include <time.h>
#include "init_memoria.h"

/*uint32_t tlb_hit_totales;
uint32_t tlb_miss_totales;*/

typedef struct{
	uint32_t prevAlloc;
	uint32_t nextAlloc;
	uint8_t isFree;
}HeapMetadata;

typedef struct{
	uint32_t pid;
	t_list* paginas;
	uint32_t tlbHit;
	uint32_t tlbMiss;
}t_tabla_de_paginas;

typedef struct{
	uint32_t nroFrame;
	uint8_t estaLibre;
}t_frame;

typedef struct{
	uint32_t nro_pagina;
	uint32_t nro_frame;
	bool bit_presencia;
	time_t uso;
	uint32_t bitMod;
	uint32_t bitUso;
}t_pagina;

typedef struct{
	uint32_t pid;
	uint32_t nroPagina;
	uint32_t nroFrame;
	time_t uso;
}t_pagina_de_tlb;

t_list* lista_de_tablas_de_paginas;
t_list* lista_de_frames;
t_list* tlb;

uint32_t static_pid;
uint32_t static_nro_frame;

pthread_mutex_t MUTEX_MP;
pthread_mutex_t MUTEX_FRAME;
pthread_mutex_t MUTEX_TABLAS_DE_PAGINAS;
pthread_mutex_t MUTEX_TLB;

void *inicio_memoria;
void iniciar_mutex();
void finalizar_mutex();
uint32_t memalloc(uint32_t pid, uint32_t size);
uint32_t alloc_disponible(uint32_t pid, t_tabla_de_paginas* tabla, uint32_t size);
void memfree(uint32_t pid, uint32_t dir_log);
void* memread(uint32_t pid, uint32_t direccionLogica, uint32_t *tamanioData);
//void memread(uint32_t pid, uint32_t direccionLogica, uint32_t tamanioData, void* destino);
void memwrite(uint32_t pid, uint32_t dir_log, uint32_t size, void* data);
void suspender_carpincho(uint32_t pid);
void traer_a_mp(uint32_t pid, t_pagina*pagina);
void mandar_a_swap(uint32_t pid, t_pagina* pagina);

//void LRU();
//void CLOCK_MODIFICADO();

void* get_pagina_data(uint32_t nro_frame);
void memcpy_pagina_en_frame(uint32_t nro_frame, uint32_t desplazamiento, void* data, uint32_t size);
void clear_frame(uint32_t nro_frame);
uint32_t cant_frames_libres();
bool entra_en_mp(uint32_t tamanio);
t_frame *encontrar_frame_vacio();
void pagina_en_tlb(uint32_t nroPagina, uint32_t nroFrame, uint32_t pid);

void dump_tlb_sigusr1();
void dump_tlb_sigint();
void dump_fecha();
void dump_tlb_sigusr2();
//MANEJO DE TABLAS
void crear_lista_de_frames(void *inicio_memoria);
void crear_tabla_de_paginas(uint32_t pid);
void crear_pagina(uint32_t pid, t_tabla_de_paginas *tablaDePaginas, uint32_t nroFrame);
void list_add_tabla_de_paginas(t_tabla_de_paginas* elem);
bool tabla_de_paginas_tiene_pid(void* x);
t_tabla_de_paginas* list_find_by_pid(uint32_t pid);
t_tabla_de_paginas* list_remove_by_pid(uint32_t pid);
void free_tabla_de_paginas(void* x);
void finalizar_lista_de_tablas_de_paginas();
void finalizar_lista_de_frames();
void finalizar_tlb();

#endif
