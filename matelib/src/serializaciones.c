/*
 * serializaciones.c
 *
 *  Created on: 8 oct. 2021
 *      Author: utnso
 */


#include "serializaciones.h"

t_buffer* serializar_iniciar(uint32_t pid){
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

t_buffer* serializar_delete(uint32_t pid){
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

t_buffer* serializar_crear_sem(uint32_t pid, unsigned int valor, char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(3*sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;
	uint32_t  el_Valor = valor;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UNSIGNED INT
	memcpy(stream + desplazamiento, (void*)(&el_Valor), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//CHAR* + LARGO DE CHAR*
	uint32_t len = strlen(nombre) + 1;

	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);
	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_wait_sem(uint32_t pid, char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(2*sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//CHAR* + LARGO DE CHAR*
	uint32_t len = strlen(nombre) + 1;

	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);
	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_post_sem(uint32_t pid, char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(2*sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//CHAR* + LARGO DE CHAR*
	uint32_t len = strlen(nombre) + 1;

	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);
	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_delete_sem(uint32_t pid, char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(2*sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//CHAR* + LARGO DE CHAR*
	uint32_t len = strlen(nombre) + 1;

	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);
	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_usar_io(uint32_t pid, char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(2*sizeof(uint32_t) + strlen(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//CHAR* + LARGO DE CHAR*
	uint32_t len = strlen(nombre) + 1;

	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(nombre) + 1);
	memcpy(stream + desplazamiento, nombre, strlen(nombre) + 1);
	desplazamiento += strlen(nombre) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}
/*
t_buffer* serializar_memalloc(uint32_t pid, int size){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int));

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//INT
	memcpy(stream + desplazamiento, (void*)(&size), sizeof(int));
	desplazamiento += sizeof(int);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_memfree(uint32_t pid, int32_t direccion){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int32_t));

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_memread(uint32_t pid, int32_t origen, int size){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int32_t) + sizeof(int));

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//INT
	memcpy(stream + desplazamiento, (void*)(&size), sizeof(int));
	desplazamiento += sizeof(int);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_memwrite(uint32_t pid, int32_t destino, int size, void* origen){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int32_t)  + sizeof(int) + sizeof(origen));

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//INT
	memcpy(stream + desplazamiento, (void*)(&size), sizeof(int));
	desplazamiento += sizeof(int);

	//printf("VOID* = %s \n", origen);
	printf("SIZE = %d \n", sizeof(origen));
	printf("SIZE PARAMETRO= %d \n", size);
	//VOID* TODO ver si anda
	memcpy(stream + desplazamiento, (void*)(&origen), sizeof(origen));
	desplazamiento += sizeof(origen);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}*/

char* deserializar_id(int socket_cliente){ //HACER QUE RETORNE LO Q CORRESPONDA
	int size;
	int desplazamiento = 0;
	void* buffer;
	uint32_t len;
	char* id;

	buffer = recibir_buffer(&size, socket_cliente);

	memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	id = malloc(len);
	memcpy((id), buffer+desplazamiento, len);
	desplazamiento += len;

	free(buffer);
	return id;
}

int deserializar_rta(int socket_cliente){ //HACER QUE RETORNE LO Q CORRESPONDA
	int size;
	void* buffer;
	//declarar las var para almacenar lo deserializado
	int desplazamiento = 0;
	int rta;

	buffer = recibir_buffer(&size, socket_cliente);

	//INT
	memcpy(&rta, buffer+desplazamiento, sizeof(int));
	desplazamiento+=sizeof(int);

	free(buffer);
	return rta;
}

int32_t deserializar_rta_memalloc(int socket_cliente){ //HACER QUE RETORNE LO Q CORRESPONDA
	int size;
	void* buffer;
	//declarar las var para almacenar lo deserializado
	int desplazamiento = 0;
	int32_t rta;

	buffer = recibir_buffer(&size, socket_cliente);

	//INT
	memcpy(&rta, buffer+desplazamiento, sizeof(int32_t));
	desplazamiento+=sizeof(int32_t);

	free(buffer);
	return rta;
}
