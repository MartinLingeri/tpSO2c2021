#include "client.h"

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/



	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	logger = log_create("/home/utnso/log.txt", "Client", true, LOG_LEVEL_TRACE);
	// Usando el logger creado previamente
	log_info(logger, "Hola! Soy un log");
	// Escribi: "Hola! Soy un log"

	config = iniciar_config();
	config = config_create("/home/utnso/tp0.config");

	// Usando el config creado previamente
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	valor = config_get_string_value(config, "VALOR");
	// Lee las variables de IP, Puerto y Valor

	//Loggear valor de config
	log_info(logger, "IP: %s", ip);
	log_info(logger, "PUERTO: %s", puerto);
	log_info(logger, "VALOR: %s", valor);

	leer_consola(logger);


	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	// Antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	conexion = crear_conexion(ip, puerto);
	if(conexion < 0){
		log_info(logger, "El servidor no esta disponible");
		exit(-1);
	}else{
		paquete(conexion);
	}

	// Creamos una conexión hacia el servidor


	//enviar CLAVE al servirdor



	terminar_programa(conexion, logger, config);

	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
}

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger;

	return nuevo_logger;
}

t_config* iniciar_config(void)
{
	t_config* nuevo_config;

	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;
	leido = readline(">");
	comandos_consola= list_create();
	//El primero te lo dejo de yapa
	while(strcmp(leido, "") != 0)
		{
			log_info(logger, "Consola: %s", leido);
			list_add(comandos_consola, leido);
			leido = readline(">");
		}

	free(leido);
	// Acá la idea es que imprimas por el log lo que recibis de la consola.


}

void paquete(int conexion)
{
	//Ahora toca lo divertido!

	t_paquete* paquete=crear_paquete();
	for(int i=0; i<list_size(comandos_consola); i++){
		agregar_a_paquete(paquete, list_get(comandos_consola, i), strlen(list_get(comandos_consola, i))+1);
	}
	agregar_a_paquete(paquete, valor, strlen(valor)+1);

	enviar_paquete(paquete, conexion);

	eliminar_paquete(paquete);

}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	liberar_conexion(conexion);
	config_destroy(config);
	exit(0);
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
}
