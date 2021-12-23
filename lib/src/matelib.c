/*
 * matelib.c
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#include "matelib.h"
//#include <biblioteca_compartida.h>


//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config){
	config = config_create(config);
	lib_ref->group_info->ip = config_get_string_value(config,"IP");
	lib_ref->group_info->puerto = config_get_string_value(config,"PUERTO");
	//lib_ref->group_info->socket_cliente =

	//lib_ref->group_info->kernel = TODO hacer conexion y ver si es kernel o memoria, identificar proceso
	//lib_ref->group_info->pid =
    logear(NUEVO_PROCESO,lib_ref->group_info->pid, "");
    return 0;
}

int mate_close(mate_instance *lib_ref){

	config_destroy(config);
	close(lib_ref->group_info->socket_cliente);
    logear(FIN_PROCESO,lib_ref->group_info->pid, "");
    return 0;
}

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){
    if(lib_ref->group_info->kernel == 0){
        return -1;
    }

    logear(SEM_CREATE,lib_ref->group_info->pid, "");
    return 0;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
        return -1;
    }

    logear(SEM_WAIT,lib_ref->group_info->pid, "");
    return 0;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
        return -1;
    }

    logear(SEM_POST,lib_ref->group_info->pid, "");
    return 0;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){
    if(lib_ref->group_info->kernel == 0){
        return -1;
    }

    logear(SEM_DELETE,lib_ref->group_info->pid, "");
    return 0;
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){

    logear(IO,lib_ref->group_info->pid, msg);
    return 0;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){

    logear(MEM_ALLOC,lib_ref->group_info->pid, "");
    return lib_ref->group_info->pid;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){

    logear(MEM_FREE,lib_ref->group_info->pid, "");
    return 0;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){

    logear(MEM_READ,lib_ref->group_info->pid, "");
    return 0;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){

    logear(MEM_WRITE,lib_ref->group_info->pid, "");
    return 0;
}

