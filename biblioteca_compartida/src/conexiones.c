#include "conexiones.h"

void unaFuncion() {
	printf("Holaa: %d", PAQUETE);
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

	int socket_cliente = 0;
	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_servidor(char* ip, char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);
    socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
    listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente = 0;
	socket_cliente = accept(socket_servidor, NULL, NULL);
	return socket_cliente;
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

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_mensaje(char* mensaje, int socket_cliente, op_code codigo_operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo_operacion;
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

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	printf("Me llego el mensaje %s", buffer);
	free(buffer);
}

t_paquete* crear_mensaje(t_buffer* buffer, op_code codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = buffer;
	return paquete;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}


t_buffer* serializar_memalloc(uint32_t pid, int size){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int)); /*size de todos los datos a enviar +1 si hay strings*/

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
	void* stream = malloc(sizeof(uint32_t) + sizeof(int32_t)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&direccion), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}

t_buffer* serializar_memread(uint32_t pid, int32_t origen, int size){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	void* stream = malloc(sizeof(uint32_t) + sizeof(int32_t) + sizeof(int)); /*size de todos los datos a enviar +1 si hay strings*/

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&origen), sizeof(uint32_t));
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
	void* stream = malloc(sizeof(uint32_t) + sizeof(uint32_t)  + sizeof(int) + size);

	int desplazamiento = 0;

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&pid), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	//UINT32_t
	memcpy(stream + desplazamiento, (void*)(&destino), sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	//INT
	memcpy(stream + desplazamiento, (void*)(&size), sizeof(int));
	desplazamiento += sizeof(int);

	printf("SIZE = %d \n", sizeof(origen));
	printf("SIZE PARAMETRO= %d \n", size);

	memcpy(stream + desplazamiento, (void*)(origen), size);
	desplazamiento += size;

	buffer->size = desplazamiento;
	buffer->stream = stream;
	return buffer;
}
