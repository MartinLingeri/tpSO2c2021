#include "client.h"

//extern t_config_memoria* cfg;

void sighandler(int x){

	switch(x){
		case SIGUSR1:
			dump_tlb_sigusr1();
			break;
		case SIGUSR2:
			dump_tlb_sigusr2();
			break;
		case SIGINT:
			dump_tlb_sigint();
			liberar_conexion(memoria_server);
			cerrar_programa();
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
	}
	return;
}

int main(void){
	logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
	config = config_create("memoria.config");

	puts("A");
	// terminar_programa(conexion_swap, logger, config);

	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);
	signal(SIGINT, sighandler);

	if(!init() || !cargar_configuracion("memoria.config") || !cargar_memoria()){
		cerrar_programa();
		return EXIT_FAILURE;
	}

	t_config* cfg_file = config_create("memoria.config");
	conexion_swap = crear_conexion(config_get_string_value(cfg_file, "IP_SWAP"), config_get_string_value(cfg_file, "PUERTO_SWAP"));

	if(conexion_swap != -1){ //TODO distinto -1
		puts("0");
		t_buffer* buffer = serializar_asignacion();
		t_paquete* paquete = crear_mensaje(buffer, 22); //TODO codigo
		puts("0");
		enviar_paquete(paquete, conexion_swap);
		log_info(logger, "Conexion establecida con SWAP");
	}else{
		log_error(logger, "ERROR - SWAP no disponible");
	}
	puts("1");
	memoria_server = iniciar_servidor(config_get_string_value(config,"IP"),config_get_string_value(config,"PUERTO"));
	printf("MEM SERVER: %d -", memoria_server);
	puts("2");
	while(server_escuchar(SERVERNAME, memoria_server));

	config_destroy(cfg_file);
	close(conexion_swap);
	cerrar_programa();
	liberar_conexion(memoria_server);
	config_destroy(config);
	return EXIT_SUCCESS;
}

/*t_log* iniciar_logger()
{
	t_log* nuevo_logger = log_create("memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);

	return nuevo_logger;
}*/
/*
t_config* iniciar_config()
{
	t_config* nuevo_config = config_create("memoria.config");
	return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(config);
}*/
