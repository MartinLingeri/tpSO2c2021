#include "mensajes.h"

void recibir_mensaje_read(int socket_memoria) {
	int size;
	int desplazamiento = 0;
	void* buffer;

	uint32_t pid;
	uint32_t nro_pag;

	buffer = recibir_buffer(&size, socket_memoria);

	memcpy(&(pid), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(nro_pag), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	void* data_a_enviar = malloc(t_pagina_size);
	data_a_enviar = read_pagina(pid, nro_pag, data_a_enviar);

	free(buffer);
	//oid* data_a_enviar = "ladata";

	if(data_a_enviar != NULL){
		buffer = serializar_rta_read(data_a_enviar);
		t_paquete* paquete_read_rta = crear_mensaje(buffer, SWAP_READ_RTA);
		enviar_paquete(paquete_read_rta, socket_memoria);
		free(paquete_read_rta);
	}else{
		buffer = serializar_rta_write(-16); //TODO Codigo de conexiones
		t_paquete* paquete_err_rta = crear_mensaje(buffer, 21); //TODO Codigo de conexiones
		enviar_paquete(paquete_err_rta, socket_memoria);
		free(paquete_err_rta);
	}

	free(buffer);
	free(data_a_enviar);
	//close(socket_memoria);
}

t_buffer* serializar_rta_read(void* data_a_enviar) {
	uint32_t data_size = strlen(data_a_enviar) + 1;

	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + data_size);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &data_size, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, data_a_enviar, data_size);
	desplazamiento += data_size;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

void recibir_mensaje_write(int socket_memoria) {
	int size;
	int desplazamiento = 0;
	void* buffer;

	uint32_t pid;
	uint32_t nro_pag;
	uint32_t data_size;
	buffer = recibir_buffer(&size, socket_memoria);
	memcpy(&(pid), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(nro_pag), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(data_size), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	void* data = malloc(data_size);
	memcpy(data, (buffer+desplazamiento), data_size);
	desplazamiento += data_size;

	//data = "bbbbbbbbbbbbbbbbbbbb";
	uint32_t rta = write_pagina(0, 0, data, data_size);

	//uint32_t pid_a_enviar = 0;
	//uint32_t nro_pag_a_enviar = 0;

	free(buffer);
	buffer = serializar_rta_write(rta);
	t_paquete* paquete_write_rta = crear_mensaje(buffer, SWAP_WRITE_RTA);
	enviar_paquete(paquete_write_rta, socket_memoria);
	free(buffer);
	free(paquete_write_rta);
	//close(socket_memoria);
}

t_buffer* serializar_rta_write(uint32_t rta) {

	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)*2);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &rta, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

void recibir_eliminar(int socket_memoria) {
	int size;
	int desplazamiento = 0;
	void* buffer;

	uint32_t pid;
	uint32_t pag;
	buffer = recibir_buffer(&size, socket_memoria);

	memcpy(&(pid), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(pag), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	finalizar_carpincho(pid, pag);

	free(buffer);
	//close(socket_memoria);
}

char* recibir_tipo_asignacion(int socket_memoria){
	int size;
	void* buffer;
	uint32_t len;
	int desplazamiento = 0;

	buffer = recibir_buffer(&size, socket_memoria);

	memcpy(&(len), (buffer+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	char* valor = malloc(len);
	memcpy((valor), buffer+desplazamiento, len);
	desplazamiento += len;

	return valor;
}
