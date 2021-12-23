#include "main.h"

int main(void)
{
	logger = iniciar_logger();
	t_config* config = iniciar_config();

	char* ip = config_get_string_value(config, "IP");
	char* puerto = config_get_string_value(config, "PUERTO");

	t_swap = config_get_int_value(config, "TAMANIO_SWAP");
	t_pagina_size = config_get_int_value(config, "TAMANIO_PAGINA");
	marcos_por_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	int retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
	char** archivos = config_get_array_value(config, "ARCHIVOS_SWAP");

	int i = 0;
	lista_archivos_swap = list_create();
	lista_paginas = list_create();
	lista_frames = list_create();

	puts("1");
	while(archivos[i] != NULL) {
		puts("2");
		int archivo_swap = open(archivos[i], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
		if(archivo_swap == -1){
			puts("error en archivo");
		}
		puts("3");
		ftruncate(archivo_swap, t_swap);
		void* inicio_archivo = mmap(NULL, t_swap, PROT_READ|PROT_WRITE, MAP_SHARED, archivo_swap, 0);
		puts("4");
		if(inicio_archivo == MAP_FAILED){
			puts("error en mmap");
		}
		puts("5");
		memset(inicio_archivo, '\0', t_swap);
		puts("6");
		t_archivo_swap* archivo = malloc(sizeof(t_archivo_swap));
		archivo->path = archivos[i];
		archivo->acceso = inicio_archivo;
		list_add(lista_archivos_swap, archivo);
		puts("7");
		agregar_frames_a_lista(inicio_archivo, t_swap, t_pagina_size, archivos[i]);
		i++;
		puts("8");
	}

	puts("9");
	int socket_server = iniciar_servidor(ip, puerto);
	puts("10");
	int socket_memoria = esperar_cliente(socket_server);

	log_info(logger, "Esperando conexiones");
	//int socket_memoria = 0;
	while (socket_memoria != -1) {
		//puts("ESPERA OP");
		int codigo_operacion = recibir_operacion(socket_memoria);

		switch(codigo_operacion) {
		puts("RECIBE OPERACION");
			case SWAP_READ:
				recibir_mensaje_read(socket_memoria);
				break;
			case SWAP_WRITE:
				recibir_mensaje_write(socket_memoria);
				break;
			case DELETE:
				recibir_eliminar(socket_memoria);
				break;
			case 22:
				//TODO agregar codigo asignacion
				puts("ASIGN");
				tipo_asignacion = recibir_tipo_asignacion(socket_memoria);
				break;
			}
	}

	if(false){
		sleep(retardo_swap); //Todo
	}
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger = log_create("swap.log", "SWAP", true, LOG_LEVEL_INFO);
	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config = config_create("swap.config");
	return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	close(conexion);
	log_destroy(logger);
	config_destroy(config);
}

void agregar_frames_a_lista(void* archivo, uint32_t t_swap, uint32_t t_pagina_size, char* path) {
	void* inicio = archivo;
	t_frame* nuevo_frame;
	for(uint32_t i = 0; i < (t_swap/t_pagina_size); i++) {
		nuevo_frame = malloc(sizeof(t_frame));
		nuevo_frame->numero_frame = i;
		nuevo_frame->espacio_libre = t_pagina_size;
		nuevo_frame->inicio = inicio;
		nuevo_frame->path_archivo = path;
		nuevo_frame->pid = -1;
		list_add(lista_frames, nuevo_frame);
		inicio += t_pagina_size;
	}
}
