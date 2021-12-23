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
	int dato;

	uint32_t direccionLogica, tamanioData=0, pid;
	void* data;

	t_memalloc* alloc;
	t_memread* read;
	t_memwrite* write;
	t_memfree* mfree;
	op_code cop;
	int manejoErrores;

	while(cliente_socket !=-1){
/*
		if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)){
			log_info(logger, STR(DISCONECT));
			return;
		}*/
		cop = recibir_operacion(cliente_socket);
		switch(cop){
			case MEMALLOC:
				alloc = malloc(sizeof(t_memalloc));
				puts("0");
				alloc = recibir_memalloc(cliente_socket);
				printf("RECIBO MEMALLOC PID: %d  -- SIZE: %d  --- \n", alloc->pid, alloc->size);
				puts("0");
				direccionLogica = memalloc(alloc->pid, alloc->size);
				puts("1");
				if(direccionLogica==-1){
					responder_dirLogica(RMEMALLOC, (int32_t) NULL, cliente_socket);
				}else{
					responder_dirLogica(RMEMALLOC, (int32_t) direccionLogica, cliente_socket);
				}
				logear(PEDIDO_ALLOC,alloc->pid, alloc->size, 1);
				free(alloc);
				break;

			case MEMFREE:
				mfree = malloc(sizeof(t_memfree));
				puts("3");
				mfree = recibir_memfree(cliente_socket);
				printf("PID: %d  -- DIR_LOG: %d  ---", mfree->pid, mfree->dir_log);
				puts("4");
				manejoErrores=memfree(mfree->pid, mfree->dir_log);
				if(manejoErrores==-1){
					responder(MEMFREE,-10,cliente_socket);
				}else{
					responder(MEMFREE,1,cliente_socket);
				}
				logear(PEDIDO_FREE,mfree->pid, mfree->dir_log, 1);
				free(mfree);
				break;

			case MEMREAD:
				read = malloc(sizeof(t_memread));
				puts("3");
				read = recibir_memread(cliente_socket);
				printf("RECIBIDO MEMREAD PID: %d  -- SIZE: %d  --- DIR: %d ", read->pid, read->size, read->dir_log);
				puts("5");
				void* destino = malloc(read->size);
				manejoErrores=memread(read->pid, read->dir_log, read->size, destino);
				if(manejoErrores==-1){
					responder(MEMREAD, -11,cliente_socket);
				}else{
					responder_data_leida(MEMREAD, data, tamanioData, cliente_socket);
				}
				logear(PEDIDO_READ,read->pid, read->dir_log, 1);
				free(destino);
				free(read);
				break;

			case MEMWRITE:
				write = malloc(sizeof(t_memwrite));
				puts("RECIBIENDO WRITE");
				puts("3");
				write = recibir_memwrite(cliente_socket);
				printf("RECIBIDO WRITE - PID: %d  -- SIZE: %d  --- DIR: %d \n", write->pid, write->size, write->dir_log);
				puts("4");
				manejoErrores=memwrite(write->pid, write->dir_log, write->size, write->data);
				if(manejoErrores==-1){
					responder(MEMWRITE, -12, cliente_socket);
				}else{
					puts("5");
					responder(MEMWRITE,1,cliente_socket);
				}
				logear(PEDIDO_WRITE,write->pid, write->dir_log, 1);
				free(write->data);
				free(write);
				break;

			case SUSPENDIDO:
				pid = recibir_suspender_carpincho(cliente_socket);
				suspender_carpincho(pid);
				responder(SUSPENDED,1,cliente_socket);
				logear(SUSPENDED,pid,1, 1);
				break;

			case INIT:
				printf("INICIADO \n");
				puts("2");

				int id = deserializar_iniciar(cliente_socket);

			    char *pid = string_new();
			    string_append(&pid, "M");
			    char *nro_id = string_itoa(ultimo_id);
			    string_append(&pid, nro_id);
			    ultimo_id++;

				t_buffer* buffer = serializar_id(pid);
				t_paquete* paquete = crear_mensaje(buffer, ENVIO_PID);
				enviar_paquete(paquete, cliente_socket);

				logear(NUEVO_PROCESO,ultimo_id-1,1, 1);
				free(buffer->stream);
				free(buffer);
				free(paquete);
				free(pid);
				free(nro_id);
				break;

			case DELETE:
				dato = recibir_suspender_carpincho(cliente_socket);
				eliminar_carpincho(dato);
				//TODO avisar a swap
				logear(CARPINCHO_ELIMINADO,alloc->pid, alloc->size, 1);
				break;

			case -7:
				log_error(logger, "Cliente desconectado de memoria...");
				return;

			default:
				log_error(logger, "Algo anduvo mal en el server de memoria");
				return;
		}
		puts("====================================================================================");
	}

	log_warning(logger, "El cliente se desconecto de %s server", server_name);
	return;
}

/*void liberar_conexion(int* socket_cliente){
    close(*socket_cliente);
    *socket_cliente = -1;
}*/

int server_escuchar(char* server_name, int server_socket){
	printf("SOCKET: %d \n", server_socket);
	int cliente_socket = esperar_cliente(server_socket);
	printf("CLIENTE SOCKET: %d \n ", cliente_socket);
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

t_buffer* serializar_write_swap(uint32_t pid, uint32_t nro_pag, uint32_t data_size, void* data) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)*3 + cfg->TAMANIO_PAGINA);
	int desplazamiento = 0;

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

	uint32_t pid;
	int32_t dirLog;
	int size_void;
	void* data;

	buffer = recibir_buffer(&size, socket_carpincho);

	memcpy(&(pid), buffer+desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(dirLog), buffer+desplazamiento, sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	memcpy(&(size_void), buffer+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	data = malloc(size_void);
	puts("3.1");
	memcpy((data), buffer+desplazamiento, size_void);
	puts("3.2");


	t_memwrite* des = malloc(sizeof(pid) + sizeof(dirLog) + sizeof(size_void) + sizeof(data));
	puts("3.3");
	des->pid = pid;
	des->dir_log = dirLog;
	des->size = size_void;
	des->data = data;
	puts("3.4");
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

	memcpy(&(des->size), buffer+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

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
	t_buffer* buffer = serializar_dirLogica(dirLog);
	t_paquete* paquete_dirLog = crear_mensaje(buffer, 20);
	enviar_paquete(paquete_dirLog, socket);
	printf("DIRLOG: %d \n", dirLog);

	free(buffer->stream);
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

t_buffer* serializar_delete(uint32_t pid, uint32_t nro_pag) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t)*3 + cfg->TAMANIO_PAGINA);
	int desplazamiento = 0;

	memcpy(stream + desplazamiento, &pid, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(stream + desplazamiento, &nro_pag, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_asignacion(){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(2*sizeof(uint32_t) + strlen(cfg->TIPO_ASIGNACION) + 1); /*size de todos los datos a enviar +1 si hay strings*/
	int desplazamiento = 0;

	uint32_t len = strlen(cfg->TIPO_ASIGNACION) + 1;
	memcpy(stream + desplazamiento, (void*)(&len), sizeof(uint32_t));
	desplazamiento += sizeof(strlen(cfg->TIPO_ASIGNACION) + 1);

	memcpy(stream + desplazamiento, cfg->TIPO_ASIGNACION, strlen(cfg->TIPO_ASIGNACION) + 1);
	desplazamiento += strlen(cfg->TIPO_ASIGNACION) + 1;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}
