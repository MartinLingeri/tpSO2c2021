#include "utils.h"

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

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
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

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = buffer;
	return paquete;
}

t_instruccion* recibir_instruccion(int cod_op, int socket){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	int size;
	void* buffer;
	uint32_t len;
	int desplazamiento = 0;

	buffer = recibir_buffer(&size, socket);

	/*memcpy(&instruccion->pid, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);*/

	switch(cod_op){
		case S_CREATE:
			memcpy(&(instruccion->pid), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(instruccion->valor_semaforo), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			instruccion->nombre_sem_io = malloc(len);
			memcpy((instruccion->nombre_sem_io), buffer+desplazamiento, len);
			desplazamiento += len;
			break;

		case S_WAIT:
			memcpy(&(instruccion->pid), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			instruccion->nombre_sem_io = malloc(len);
			memcpy((instruccion->nombre_sem_io), buffer+desplazamiento, len);
			desplazamiento += len;
			break;

		case S_POST:
			memcpy(&(instruccion->pid), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			instruccion->nombre_sem_io = malloc(len);
			memcpy((instruccion->nombre_sem_io), buffer+desplazamiento, len);
			desplazamiento += len;
			break;

		case DELETE:
			memcpy(&(instruccion->pid), buffer+desplazamiento, sizeof(int32_t));
			desplazamiento+=sizeof(int32_t);
			break;

		case CALL_IO:
			memcpy(&(instruccion->pid), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);

			instruccion->nombre_sem_io = malloc(len);
			memcpy((instruccion->nombre_sem_io), buffer+desplazamiento, len);
			desplazamiento += len;
			break;

		default:
			break;
	}
	return instruccion;
}

t_buffer* deserializar_parcial(int cop, int socket){
	t_op_memoria* mem = malloc(sizeof(t_op_memoria));

	switch(cop){
	case MEMALLOC:
			mem = recibir_memalloc(socket);
			return serializar_memalloc(mem->pid, mem->size);
	case MEMFREE:
			mem = recibir_memfree(socket);
			return serializar_memfree(mem->pid, mem->dir);
	case MEMREAD:
			mem =  recibir_memread(socket);
			return serializar_memread(mem->pid, mem->dir, mem->size);
	case MEMWRITE:
			mem =  recibir_memwrite(socket);
			return serializar_memwrite(mem->pid, mem->dir, mem->size, mem->data);
	default:
			return serializar_respuesta(ERROR_DE_CONEXION);
	}
}

void responder(int cod_op, int respuesta, int socket){
	t_buffer* buffer = serializar_respuesta(respuesta);
	t_paquete* paquete = crear_mensaje(buffer, RESPUESTA);
	enviar_paquete(paquete, socket);
}

void responder_memalloc(int cod_op, int32_t respuesta, int socket){
	t_buffer* buffer = serializar_respuesta_memalloc(respuesta);
	t_paquete* paquete = crear_mensaje(buffer, 20); //TODO rta memalloc de conexiones
	enviar_paquete(paquete, socket);
}

double timestamp(){
	time_t fin;
	time(&fin);
	struct tm *local_fin = localtime(&fin);
	return (local_fin->tm_sec + local_fin->tm_min * 60 + local_fin->tm_hour * 3600);
}

t_buffer* serializar_respuesta(int rta){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(int)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//INT
	memcpy(stream + desplazamiento, (void*)(&rta), sizeof(int));
	desplazamiento += sizeof(int);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_respuesta_memalloc(int32_t rta){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(int32_t)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&rta), sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_suspender(uint32_t pid){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

int deserializar_rta(int socket_carpincho){
	int size;
	void* buffer;
	//declarar las var para almacenar lo deserializado
	int desplazamiento = 0;
	int rta;

	buffer = recibir_buffer(&size, socket_carpincho);

	//INT
	memcpy(&rta, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);
	return rta;
}

int32_t deserializar_rta_memalloc(int socket_carpincho){
	int size;
	void* buffer;
	//declarar las var para almacenar lo deserializado
	int desplazamiento = 0;
	int32_t rta;

	buffer = recibir_buffer(&size, socket_carpincho);

	//INT
	memcpy(&rta, buffer+desplazamiento, sizeof(int32_t));
	desplazamiento+=sizeof(int32_t);

	free(buffer);
	return rta;
}

uint32_t deserializar_iniciar(int socket_carpincho){
	int size;
	void* buffer;
	//declarar las var para almacenar lo deserializado
	int desplazamiento = 0;
	int32_t rta;

	buffer = recibir_buffer(&size, socket_carpincho);

	//INT
	memcpy(&rta, buffer+desplazamiento, sizeof(int32_t));
	desplazamiento+=sizeof(int32_t);

	free(buffer);
	return rta;
}

t_buffer* serializar_id(char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	uint32_t len = strlen(nombre) + 1;

	//CHAR* + LARGO DE CHAR*
	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);

	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_op_memoria* recibir_memalloc(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_op_memoria* des = malloc(sizeof(t_op_memoria));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(int);

	free(buffer);
	return des;
}

t_op_memoria* recibir_memfree(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_op_memoria* des = malloc(sizeof(t_op_memoria));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	free(buffer);
	return des;
}

t_op_memoria* recibir_memwrite(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_op_memoria* des = malloc(sizeof(t_op_memoria));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(des->data), buffer+desplazamiento, des->size);
	desplazamiento += sizeof(des->data);

	free(buffer);
	return des;
}

t_op_memoria* recibir_memread(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_op_memoria* des = malloc(sizeof(t_op_memoria));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(int);

	free(buffer);
	return des;
}
