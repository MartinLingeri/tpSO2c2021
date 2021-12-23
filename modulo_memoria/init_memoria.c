#include "init_memoria.h"

t_config_memoria* initialize_cfg(){
	t_config_memoria* cfg = malloc(sizeof(t_config_memoria));
	cfg->ALGORITMO_REEMPLAZO_MMU = NULL;
	cfg->TIPO_ASIGNACION = NULL;
	cfg->ALGORITMO_REEMPLAZO_TLB = NULL;
	cfg->PATH_DUMP_TLB = NULL;
	return cfg;
}

bool init(){
	cfg = initialize_cfg();
	logger = log_create("memoria.log", MODULENAME, false, LOG_LEVEL_INFO);
	iniciar_mutex();
	return 1;
}

bool cargar_configuracion(char* path){
	t_config* cfg_file = config_create(path);

	if(cfg_file == NULL){
		log_error(logger, "No se encontro memoria.config");
	}

	cfg->TAMANIO = config_get_int_value(cfg_file, "TAMANIO");
	cfg->TAMANIO_PAGINA = config_get_int_value(cfg_file, "TAMANIO_PAGINA");
	cfg->ALGORITMO_REEMPLAZO_MMU = config_get_string_value(cfg_file, "ALGORITMO_REEMPLAZO_MMU");
	cfg->TIPO_ASIGNACION = config_get_string_value(cfg_file, "TIPO_ASIGNACION");
	cfg->MARCOS_POR_PROCESO = config_get_int_value(cfg_file, "MARCOS_POR_PROCESO");
	cfg->CANTIDAD_ENTRADAS_TLB = config_get_int_value(cfg_file, "CANTIDAD_ENTRADAS_TLB");
	cfg->ALGORITMO_REEMPLAZO_TLB = config_get_string_value(cfg_file, "ALGORIMTO_REEMPLAZO_TLB");
	cfg->RETARDO_ACIERTO_TLB = config_get_int_value(cfg_file, "RETARDO_ACIERTO_TLB");
	cfg->RETARDO_FALLO_TLB = config_get_int_value(cfg_file, "RETARDO_FALLO_TLB");

	cfg->PUERTO = config_get_int_value(cfg_file, "PUERTO");
	cfg->IP = config_get_int_value(cfg_file, "IP");

	printf("ALGORITMO: %s \n", cfg->ALGORITMO_REEMPLAZO_MMU);
	log_info(logger, "Archivo de configuracion cargado correctamente");

	config_destroy(cfg_file);



	return 1;
}

bool cargar_memoria(){
	inicio_memoria = malloc(cfg->TAMANIO);
	if(inicio_memoria ==  NULL){
		log_error(logger, "Fallo en el malloc en inicio_memoria");
		return 0;
	}
	memset(inicio_memoria, 0, cfg->TAMANIO);

	lista_de_tablas_de_paginas=list_create();
	if(lista_de_tablas_de_paginas == NULL){
		log_error(logger, "Fallo creando lista_de_tablas_de_paginas");
		return 0;
	}

	crear_lista_de_frames(inicio_memoria);
	if(lista_de_frames == NULL){
		log_error(logger, "Fallo creando la lista_de_frames");
		return 0;
	}
	tlb_hit_totales = 0;
	tlb_miss_totales = 0;
	tlb = list_create();
	if(tlb == NULL){
		log_error(logger, "Fallo creando la tlb");
		return 0;
	}
	return 1;
}

void cerrar_programa(){
	log_info(logger, "Finalizando programa...");

	log_destroy(logger);

	finalizar_lista_de_tablas_de_paginas();
	finalizar_lista_de_frames();
	finalizar_tlb();

	free(cfg->ALGORITMO_REEMPLAZO_MMU);
	free(cfg->ALGORITMO_REEMPLAZO_TLB);
	free(cfg->TIPO_ASIGNACION);
	free(cfg->PATH_DUMP_TLB);
	free(cfg);

	free(inicio_memoria);
	finalizar_mutex();
}

