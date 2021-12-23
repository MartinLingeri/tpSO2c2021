/*
 * matelib.c
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#include "matelib.h"

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){
	struct mate_instance carpincho;
	carpincho = *lib_ref;
	carpincho.group_info = malloc(sizeof(t_carpincho));

	config_carpincho = config_create(config);
	carpincho.group_info->ip = config_get_string_value(config_carpincho,"IP");
	carpincho.group_info->puerto = config_get_string_value(config_carpincho,"PUERTO");

	//printf("IP: %s \n", carpincho.group_info->ip);
	//printf("PUERTO: %s \n", carpincho.group_info->puerto);

	carpincho.group_info->socket_cliente = crear_conexion(carpincho.group_info->ip,carpincho.group_info->puerto);

	t_buffer* buffer = serializar_iniciar(1); //pid no está iniciado, mando un 1 por las dudas que rompa
	t_paquete* paquete = crear_mensaje(buffer, INIT);
	pthread_mutex_lock(&carpincho.group_info->semaforo);
	enviar_paquete(paquete, carpincho.group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&carpincho.group_info->semaforo);

	int cod_op = recibir_operacion(carpincho.group_info->socket_cliente);
	char* id;

	if(cod_op == ENVIO_PID){
		id = deserializar_id(carpincho.group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,carpincho.group_info->pid, "");
		return ERROR_DE_CONEXION;
	}

	if(string_starts_with(id, "K")){
		carpincho.group_info->kernel = 1;
	}else{
		carpincho.group_info->kernel = 0;
	}
	carpincho.group_info->pid = atoi(string_substring_from(id,1));

	printf("KERNEL: %d \n", carpincho.group_info->kernel);
	printf("ID: %d \n", carpincho.group_info->pid);

    logear(NUEVO_PROCESO,carpincho.group_info->pid, "");

    *lib_ref = carpincho;
    return 0;
}

int mate_close(mate_instance *lib_ref){
	struct mate_instance carpincho;
	carpincho = *lib_ref;
	carpincho.group_info = malloc(sizeof(t_carpincho));

	t_buffer* buffer = serializar_delete(lib_ref->group_info->pid);
	t_paquete* paquete = crear_mensaje(buffer, DELETE);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;

	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	//printf("COD: %d \n", cod_op);

	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}

    logear(FIN_PROCESO,lib_ref->group_info->pid, "");

	close(carpincho.group_info->socket_cliente);
	free(carpincho.group_info);
    *lib_ref = carpincho;
    return rta;
}

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
	printf("NOMBRE SEMAFORO: %s \n", sem);
    if(lib_ref->group_info->kernel == 0){
    	logear(FUNCION_INHABILITADA,lib_ref->group_info->pid, "");
		return KERNEL_INHABILITADO;
    }

	t_buffer* buffer = serializar_crear_sem(lib_ref->group_info->pid, value, sem);
	t_paquete* paquete = crear_mensaje(buffer, S_CREATE);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}

    logear(SEM_CREATE,lib_ref->group_info->pid, sem);
    return rta;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
	    logear(FUNCION_INHABILITADA,lib_ref->group_info->pid, "");
        return KERNEL_INHABILITADO;
    }

	t_buffer* buffer = serializar_wait_sem(lib_ref->group_info->pid, sem);
	t_paquete* paquete = crear_mensaje(buffer, S_WAIT);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;;
	}

    logear(SEM_WAIT,lib_ref->group_info->pid, sem);
    return rta;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
	    logear(FUNCION_INHABILITADA,lib_ref->group_info->pid, "");
        return KERNEL_INHABILITADO;
    }

	t_buffer* buffer = serializar_post_sem(lib_ref->group_info->pid, sem);
	t_paquete* paquete = crear_mensaje(buffer, S_POST);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);

	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}

    logear(SEM_POST,lib_ref->group_info->pid, sem);
    return rta;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
	    logear(FUNCION_INHABILITADA,lib_ref->group_info->pid, "");
        return KERNEL_INHABILITADO;
    }

	t_buffer* buffer = serializar_delete_sem(lib_ref->group_info->pid, sem);
	t_paquete* paquete = crear_mensaje(buffer, S_DELETE);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	free(buffer);
	free(paquete);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}

    logear(SEM_DELETE,lib_ref->group_info->pid, sem);
    return rta;
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){
    if(lib_ref->group_info->kernel == 0){
	    logear(FUNCION_INHABILITADA,lib_ref->group_info->pid, "");
        return KERNEL_INHABILITADO;
    }

	t_buffer* buffer = serializar_usar_io(lib_ref->group_info->pid, io);
	t_paquete* paquete = crear_mensaje(buffer, CALL_IO);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);

	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	    printf("%s \n", msg);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}
    logear(IO,lib_ref->group_info->pid, io);

    return rta;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
puts("1");
	t_buffer* buffer = serializar_memalloc(lib_ref->group_info->pid, size);
	t_paquete* paquete = crear_mensaje(buffer, MEMALLOC);
	puts("2");
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	puts("3");
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);
	puts("4");
	int32_t rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	puts("5");
	if(cod_op == RMEMALLOC){
		puts("6");
		rta = deserializar_rta_memalloc(lib_ref->group_info->socket_cliente);
		puts("7");
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}
    logear(MEM_ALLOC,lib_ref->group_info->pid, "");
    return rta;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

	t_buffer* buffer = serializar_memfree(lib_ref->group_info->pid, addr);
	t_paquete* paquete = crear_mensaje(buffer, MEMFREE);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}
    logear(MEM_FREE,lib_ref->group_info->pid, "");
    return rta;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

	t_buffer* buffer = serializar_memread(lib_ref->group_info->pid, origin, size);
	t_paquete* paquete = crear_mensaje(buffer, MEMREAD);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);
	//TODO aca el dest es donde tengo que guardar el dato que recibo ???
	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
		//dest = deserilizar_memread(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}
    logear(MEM_READ,lib_ref->group_info->pid, "");
    return rta;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

	t_buffer* buffer = serializar_memwrite(lib_ref->group_info->pid, dest, size, origin);
	t_paquete* paquete = crear_mensaje(buffer, MEMWRITE);
	pthread_mutex_lock(&lib_ref->group_info->semaforo);
	enviar_paquete(paquete, lib_ref->group_info->socket_cliente);

	pthread_mutex_unlock(&lib_ref->group_info->semaforo);

	int rta;
	int cod_op = recibir_operacion(lib_ref->group_info->socket_cliente);
	if(cod_op == RESPUESTA){
		rta = deserializar_rta(lib_ref->group_info->socket_cliente);
	}else{
	    logear(ERROR_OP,lib_ref->group_info->pid, "");
		return ERROR_DE_CONEXION;
	}
    logear(MEM_WRITE,lib_ref->group_info->pid, "");
    return rta;
}

