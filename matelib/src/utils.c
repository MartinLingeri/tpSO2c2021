#include "utils.h"


//PARA DESACTIVAR LOGS
/*void logear(int situacion, int dato, char* otroDato){
	return;
}*/

void logear(int situacion, int dato, char* otroDato){
	if(logger == NULL && situacion != NUEVO_PROCESO && situacion != ERROR_OP){
		return;
	}
	int level = config_get_int_value(config_carpincho,"LOG_LEVEL");
	switch(situacion){
		case NUEVO_PROCESO:
			logger = log_create("matelib.log", "MATELIB", true, level);
			log_info(logger,"NUEVO PROCESO - Iniciado carpincho %d", dato);
			break;

		case FIN_PROCESO:
			log_info(logger,"FIN DE PROCESO - Finalizado carpincho %d", dato);
		    //log_destroy(logger);
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

		case ERROR_OP:
			if(logger == NULL){
				logger = log_create("matelib.log", "MATELIB", true, level);
			}
			log_error(logger,"PROCESO %d - Error al responder de servidor", dato);
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

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = buffer;
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

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
