#include "utils.h"


//PARA DESACTIVAR LOGS
/*void logear(int situacion, int dato, char* otroDato){
	return;
}*/

void logear(int situacion, int dato, char* otroDato){
	switch(situacion){
		case NUEVO_PROCESO:
			t_log* logger = log_create("matelib.log", "MATELIB", true, config_get_int_value(config,"LOG_LEVEL"));
			log_info(logger,"NUEVO PROCESO - Iniciado carpincho %d", dato);
			break;

		case FIN_PROCESO:
			log_info(logger,"FIN DE PROCESO - Finalizado carpincho %d", dato);
		    log_destroy(logger);
			break;

		case SEM_CREATE:
			log_info(logger,"PROCESO %d - CREATE de semaforo: %s", dato, otroDato);
			break;

		case SEM_WAIT:
			log_info(logger,"PROCESO %d - WAIT de semaforo: %s", dato, otroDato);
			break;

		case SEM_POST:
			log_info(logger,"PROCESO %d - POST de semaforo: %s", dato, otroDato);
			break;

		case SEM_DELETE:
			log_info(logger,"PROCESO %d - DELETE de semaforo: %s", dato, otroDato);
			break;

		case IO:
			log_info(logger,"PROCESO %d - Usa I/O: %s", dato, otroDato);
			break;

		case MEM_ALLOC:
			log_info(logger,"PROCESO %d - Solicita memoria", dato);
			break;

		case MEM_READ:
			log_info(logger,"PROCESO %d - Lee de memoria", dato);
			break;

		case MEM_WRITE:
			log_info(logger,"PROCESO %d - Escribe en memoria", dato);
			break;

		case MEM_FREE:
			log_info(logger,"PROCESO %d - Libera memoria", dato);
			break;

		case CONEXION_KERNEL:
			log_info(logger,"PROCESO %d - Conectado a Kernel", dato);
			break;

		case CONEXION_MEMORIA:
			log_info(logger,"PROCESO %d - Conectado a Memoria", dato);
			break;

		case FUNCION_INHABILITADA:
			log_error(logger,"PROCESO %d - Función inhabilitada, Kernel no conectado", dato);
			break;

		default:
			return;
	}
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = 0;
	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
