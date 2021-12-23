#include "comunicacion.h"

int ultimo_id = 1;
extern t_config_memoria* cfg;

#define STR(_) #_

typedef struct{
	int fd;
	char* server_name;
}t_procesar_conexion_args;

static void procesar_conexion(void* void_args){
	t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
	int cliente_socket = args->fd;
	char* server_name = args->server_name;
	free(args);

	t_memalloc* alloc = malloc(sizeof(t_memalloc));
	t_memread* read = malloc(sizeof(t_memread));
	t_memwrite* write = malloc(sizeof(t_memwrite));
	t_memfree* mfree = malloc(sizeof(t_memfree));
	uint32_t direccionLogica;
	void* data;
	uint32_t tamanioData=0;
	uint32_t pid;

	op_code cop;
	while(cliente_socket !=-1){
		/*if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)){
			log_info(logger, STR(DISCONECT));
			return;
		}*/
		cop = recibir_operacion(cliente_socket);
		//puts("RECV OP");
		//printf("RECIBIDO %d \n", cop);
		switch(cop){
			case MEMALLOC:
				puts("0");
				alloc = recibir_memalloc(cliente_socket);
				puts("RECIBIDO MEMALLOC \n");
				printf("PID: %d  -- SIZE: %d  --- \n", alloc->pid, alloc->size);
				direccionLogica = memalloc(alloc->pid, alloc->size);
				puts("1");
				responder_dirLogica(20, (int32_t) direccionLogica, cliente_socket);
				puts("2");
				break;

			case MEMFREE:
				puts("3");
				mfree = recibir_memfree(cliente_socket);
				printf("PID: %d  -- DIR_LOG: %d  ---", mfree->pid, mfree->dir_log);
				puts("4");
				memfree(mfree->pid, mfree->dir_log);
				puts("5");
				responder(MEMFREE,1,cliente_socket);
				//TODO devolver todo ok
				break;

			case MEMREAD:
				puts("RECIBIENDO MEMREAD");
				puts("3");
				read = recibir_memread(cliente_socket);
				printf("PID: %d  -- SIZE: %d  --- DIR: %d ", read->pid, read->size, read->dir_log);
				puts("4");
				responder(MEMREAD,1,cliente_socket);
				//data = memread(read->pid, read->dir_log, &tamanioData);
				//responder_data_leida(MEMREAD, data, tamanioData, cliente_socket);
				break;

			case MEMWRITE:
				puts("RECIBIENDO WRITE");
				puts("3");
				write = recibir_memwrite(cliente_socket);
				printf("PID: %d  -- SIZE: %d  --- DIR: %d \n", write->pid, write->size, write->dir_log);
				puts("4");
				memwrite(write->pid, write->dir_log, write->size, write->data);
				puts("5");
				responder(MEMWRITE,1,cliente_socket);
				//TODO devolver todo ok
				break;

			case SUSPENDIDO:
				pid = recibir_suspender_carpincho(cliente_socket);
				suspender_carpincho(pid);
				//TODO devolver todo ok
				break;

			case INIT:
				printf("INICIADO \n");
				puts("2");

				int id = deserializar_iniciar(cliente_socket);

			    char *pid = string_new();
			    string_append(&pid, "M");
			    string_append(&pid, string_itoa(ultimo_id));
			    ultimo_id++;

				t_buffer* buffer = serializar_id(pid);
				t_paquete* paquete = crear_mensaje(buffer, ENVIO_PID);
				enviar_paquete(paquete, cliente_socket);
				free(buffer);
				free(paquete);
				break;

			case DELETE:
				//TODO avisar a swap

			case -7:
				log_error(logger, "Cliente desconectado de memoria...");
				return;

			default:
				log_error(logger, "Algo anduvo mal en el server de memoria");
				return;
		}
		puts("A");
	}

	log_warning(logger, "El cliente se desconecto de %s server", server_name);
	return;
}

/*void liberar_conexion(int* socket_cliente){
    close(*socket_cliente);
    *socket_cliente = -1;
}*/

int server_escuchar(char* server_name, int server_socket){
	puts("3");
	printf("SOCKET: %d \n", server_socket);
	int cliente_socket = esperar_cliente(server_socket);
	printf("CLIENTE SOCKET: %d \n ", cliente_socket);
	puts("4");
	if (cliente_socket != -1){
		pthread_t hilo;
		t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
		args->fd = cliente_socket;
		args->server_name = server_name;
		pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
		pthread_detach(hilo);
		return 1;
	}
	return 0;
}

void mensaje_read_swap() {
	t_config* cfg_file = config_create("memoria.config");
	int conexion_swap = crear_conexion(config_get_string_value(cfg_file, "IP_SWAP"), config_get_string_value(cfg_file, "PUERTO_SWAP"));
	config_destroy(cfg_file);
	t_buffer* buffer = serializar_read_swap();

	t_paquete* paquete_read_swap = crear_mensaje(buffer, SWAP_READ);
	enviar_paquete(paquete_read_swap, conexion_swap);
	free(buffer);
	free(paquete_read_swap);

	int codigo_operacion = recibir_operacion(conexion_swap);
	printf("OPCODE: %d", codigo_operacion);
	int size;
	int desplazamiento = 0;
	void* buffer_recibido;

	uint32_t data_size;

	buffer_recibido = recibir_buffer(&size, conexion_swap);
	memcpy(&(data_size), (buffer_recibido+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	void* data = malloc(data_size);
	memcpy(data, (buffer_recibido+desplazamiento), data_size);
	desplazamiento += data_size;
	printf("LA DATA: %s", (char*)data);
	close(conexion_swap);
	sleep(10);
}

void mensaje_write_swap() {
	t_config* cfg_file = config_create("memoria.config");
	int conexion_swap = crear_conexion(config_get_string_value(cfg_file, "IP_SWAP"), config_get_string_value(cfg_file, "PUERTO_SWAP"));
	config_destroy(cfg_file);
	t_buffer* buffer = serializar_write_swap();

	t_paquete* paquete_write_swap = crear_mensaje(buffer, SWAP_WRITE);
	enviar_paquete(paquete_write_swap, conexion_swap);
	free(buffer);
	free(paquete_write_swap);

	int codigo_operacion = recibir_operacion(conexion_swap);
	printf("OPCODE: %d", codigo_operacion);
	int size;
	int desplazamiento = 0;
	void* buffer_recibido;

	uint32_t pid;
	uint32_t nro_pag;

	buffer_recibido = recibir_buffer(&size, conexion_swap);
	memcpy(&(pid), (buffer_recibido+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(nro_pag), (buffer_recibido+desplazamiento), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	printf("EL PID: %d", pid);
	close(conexion_swap);
	sleep(10);
}

t_buffer* serializar_read_swap() {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)*2);
	int desplazamiento = 0;

	uint32_t pid = 0;
	uint32_t nro_pag = 0;

	memcpy(stream + desplazamiento, &pid, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, &nro_pag, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_write_swap() {
	void* data = "ladata";
	uint32_t data_size = strlen(data) + 1;

	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)*3 + data_size);
	int desplazamiento = 0;

	uint32_t pid = 20;
	uint32_t nro_pag = 0;

	memcpy(stream + desplazamiento, &pid, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, &nro_pag, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, &data_size, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, data, data_size);
	desplazamiento += data_size;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_memalloc* recibir_memalloc(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_memalloc* des = malloc(sizeof(t_memalloc));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(int));

	free(buffer);
	return des;
}

t_memfree* recibir_memfree(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_memfree* des = malloc(sizeof(t_memfree));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir_log), buffer+desplazamiento, sizeof(uint32_t));

	free(buffer);
	return des;
}

t_memwrite* recibir_memwrite(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_memwrite* des = malloc(sizeof(t_memwrite));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir_log), buffer+desplazamiento, sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	puts("3.1");
	memcpy(&(des->data), buffer+desplazamiento, des->size);
	puts("3.2");

	free(buffer);
	return des;
}

t_memread* recibir_memread(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	t_memread* des = malloc(sizeof(t_memread));

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(des->pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->dir_log), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(des->size), buffer+desplazamiento, sizeof(int)); //TODO fijate el tipo de datos de size que es int y lo estás recibiendo com oint32, no se si romperá pero x las dudas
	desplazamiento += sizeof(int); //tambien el orden en el q se reciben las cosas si coincide con el q las mando de lib todo

	free(buffer);
	return des;
}

uint32_t recibir_suspender_carpincho(int socket_carpincho){
	int size;
	void* buffer;
	int desplazamiento = 0;
	uint32_t pid;

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&pid,buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento+=sizeof(uint32_t);

	free(buffer);
	return pid;
}

void responder_dirLogica(int cod_op, int32_t dirLog, int socket){
	printf("DIRLOG: %d \n", dirLog);
	t_buffer* buffer = serializar_dirLogica(dirLog);
	printf("DIRLOG: %d \n", dirLog);
	t_paquete* paquete_dirLog = crear_mensaje(buffer, 20);
	printf("DIRLOG: %d \n", dirLog);
	enviar_paquete(paquete_dirLog, socket);
	printf("DIRLOG: %d \n", dirLog);
	free(buffer);
	free(paquete_dirLog);
}

t_buffer* serializar_dirLogica(int32_t dirLog){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(int32_t)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&dirLog), sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

void responder_data_leida(int cod_op, void* data, uint32_t tamanioData, int socket){
	t_buffer* buffer = serializar_data_leida(data, tamanioData);
	t_paquete* paquete_data_leida = crear_mensaje(buffer, MEMREAD); //MEMREAD  O TENGO QUE CREAR OTRO OPCODE?
	enviar_paquete(paquete_data_leida, socket);
	free(buffer);
	free(paquete_data_leida);
}

t_buffer* serializar_data_leida(void* data, uint32_t tamanioData){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t));
	int desplazamiento = 0;

	memcpy(stream+desplazamiento, &tamanioData, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream+desplazamiento, data, tamanioData);
	desplazamiento += tamanioData;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_id(char* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(nombre) + 1); /*size de todos los datos a enviar +1 si hay strings*/

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

void responder(int cod_op, int respuesta, int socket){
	t_buffer* buffer = serializar_respuesta(respuesta);
	t_paquete* paquete = crear_mensaje(buffer, RESPUESTA);
	enviar_paquete(paquete, socket);
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
