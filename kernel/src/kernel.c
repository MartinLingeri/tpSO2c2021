#include "kernel.h"

int conexion;
char* ip;
char* puerto;

void atenderSocket(void* arg){
	int socketCliente = *((int *)arg);
	int cod_op = recibir_operacion(socketCliente);
	if(cod_op != -1){
		switch (cod_op) {
			case INIT:
				planificador_largo_plazo(socketCliente);
				break;
			default:
				log_warning(logger, "Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

int esperarClientes(int server_fd){
	if (server_fd < 0) {
		log_error(logger, "NO SE PUDO CREAR SOCKET ESCUCHA");
		return -1;
	}

	log_info(logger, "KERNEL LISTO PARA RECIBIR PETICIONES");

	//printf("SERVER FD: %d --------\n", server_fd);
	int cliente_fd = esperar_cliente(server_fd);
	//printf("CLIENTE FD: %d --------\n", cliente_fd);
	if(cliente_fd != -1){
		pthread_t hilo;
		pthread_create(&hilo, NULL, (void*) atenderSocket, &cliente_fd);
		pthread_detach(hilo);
		return 1;
	}else{
		log_error(logger, "El cliente se desconecto. Terminando servidor");
	}
	return -1;
}

int main(void){
	logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_INFO);
	log_info(logger, "Iniciando kernel");
	config = config_create("kernel.config");
	array_dispositivos = config_get_array_value(config, "DISPOSITIVOS_IO");
	array_duraciones = config_get_array_value(config, "DURACIONES_IO");

	int gradomp = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");

	lista_semaforos = list_create();
	lista_dispositivos = list_create();

	for(int i = 0; array_dispositivos[i]!=NULL; i++){
		t_io* un_io = malloc(sizeof(t_io));
		un_io->duracion = atoi(array_duraciones[i]);
		un_io->nombre = array_dispositivos[i];
		un_io->bloqueado = 0;
		list_add(lista_dispositivos, un_io);
	}

	l_new = list_create();
	l_ready = list_create();
	l_exec = list_create();
	l_exit = list_create();
	l_blocked = list_create();
	l_suspended_ready = list_create();
	l_suspended_blocked = list_create();

	sem_init(&proceso_ready,0,1);
	sem_init(&proceso_susp,0,1);
	sem_init(&proceso_bloqueado,0,1);
	sem_init(&proceso_nuevo,0,1);
	sem_init(&multiproc,0,gradomp);
	sem_init(&listo_para_trabajar,0,0);
	sem_init(&listo_para_trabajar_susp,0,0);
	sem_init(&listo_para_trabajar_ready,0,0);

	pthread_t planif_corto;
	pthread_create(&planif_corto, NULL, (void*) planificador_corto_plazo, NULL);
	pthread_detach((pthread_t) planif_corto);

	pthread_t planif_medio_suspender;
	pthread_create(&planif_medio_suspender, NULL, (void*) planificador_mediano_plazo_suspender, NULL);
	pthread_detach((pthread_t) planif_medio_suspender);

	pthread_t planif_medio_ready;
	pthread_create(&planif_medio_ready, NULL, (void*) planificador_mediano_plazo_ready, NULL);
	pthread_detach((pthread_t) planif_medio_ready);

	pthread_t deadlock;
	pthread_create(&deadlock, NULL, (void*) detectar_deadlock, NULL);
	pthread_detach((pthread_t) deadlock);

	int server_fd = iniciar_servidor("127.0.0.1", config_get_string_value(config, "PUERTO_KERNEL"));
	while(esperarClientes(server_fd) != -1);

	log_info(logger, "Finalizando Kernel");
	terminar_programa(conexion, logger, config);
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(config);
}

void cambiar_estado(int estado_nuevo, t_carpincho* carpincho) {
   bool es_el_carpincho(void* carpincho_en_lista) {
		return ((t_carpincho*)carpincho_en_lista)->pid == carpincho->pid;
	}

   pthread_mutex_lock(&estados[carpincho->estado]);
   switch(carpincho->estado){
    case NEW:
        list_remove_by_condition(l_new, es_el_carpincho);
        break;
    case READY:
        list_remove_by_condition(l_ready, es_el_carpincho);
        break;
    case EXEC:
        list_remove_by_condition(l_exec, es_el_carpincho);
        //sem_post(&proceso_ready);
        //sem_post(&proceso_ready);
		sem_post(&multiproc);
        break;
    case EXIT:
        list_remove_by_condition(l_exit, es_el_carpincho);
        break;
    case BLOCKED:
        list_remove_by_condition(l_blocked, es_el_carpincho);
        break;
    case SUSPENDED_READY:
        list_remove_by_condition(l_suspended_ready, es_el_carpincho);
        break;
    case SUSPENDED_BLOCKED:
        list_remove_by_condition(l_suspended_blocked, es_el_carpincho);
        break;
   }

   logear(CAMBIO_ESTADO,carpincho->pid,estado_nuevo,"");
   pthread_mutex_unlock(&estados[carpincho->estado]);
   carpincho->estado = estado_nuevo;
   pthread_mutex_lock(&estados[estado_nuevo]);

   switch(estado_nuevo){
    case NEW:
        list_add(l_new, carpincho);
        break;
    case READY:
        list_add(l_ready, carpincho);
       // sem_post(&proceso_ready);
     //   sem_post(&proceso_ready);
        sem_post(&listo_para_trabajar);
		carpincho->tiempo_llegada = timestamp();
        break;
    case EXEC:
        list_add(l_exec, carpincho);
        break;
    case EXIT:
        list_add(l_exit, carpincho);
        break;
    case BLOCKED:
        list_add(l_blocked, carpincho);
        //sem_post(&proceso_bloqueado);
        //sem_post(&proceso_bloqueado);
		sem_post(&listo_para_trabajar_susp);
        break;
    case SUSPENDED_READY:
        list_add(l_suspended_ready, carpincho);
		//sem_post(&proceso_susp);
		//sem_post(&proceso_susp);
		sem_post(&listo_para_trabajar_ready);
        break;
    case SUSPENDED_BLOCKED:
        list_add(l_suspended_blocked, carpincho);
        break;
    }
   pthread_mutex_unlock(&estados[estado_nuevo]);
}

void planificador_largo_plazo(int socket_cliente){
	int multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

	int id = deserializar_iniciar(socket_cliente);
	t_carpincho* llegado = malloc(sizeof(t_carpincho));

	llegado->pid = ultimo_id + id*0;
	ultimo_id++;
	llegado->estado = NEW;
	llegado->instruccion = malloc(sizeof(t_instruccion));
	llegado->ultima_rafaga = -1;
	llegado->ultimo_estimado = -1;
	llegado->tiempo_llegada = timestamp();
	llegado->esperando = "VACIO";
	llegado->socket_carpincho = socket_cliente;
	sem_t sem;
	llegado->semaforo = sem;
	llegado->tomando = list_create();
	sem_init(&llegado->semaforo,0,1);

    char *pid = string_new();
    string_append(&pid, "K");
    string_append(&pid, string_itoa(llegado->pid));

	t_buffer* buffer = serializar_id(pid);
	t_paquete* paquete = crear_mensaje(buffer, ENVIO_PID);
	enviar_paquete(paquete, socket_cliente);
	free(buffer);
	free(paquete);
	printf("CARPINCHO %d INICIADO \n", llegado->pid);
	if((list_size(l_ready) + list_size(l_exec) + list_size(l_blocked)) >= multiprog){
		list_add(l_new, llegado);
		printf("SE agrega carpincho %d \n", llegado->pid);
	}else{
		list_add(l_new, llegado);
		cambiar_estado(READY,llegado);
	}
}

void planificador_corto_plazo(){
	int grado_multiproc = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
	char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

	while(1){
		sem_wait(&listo_para_trabajar);
		/*printf("READY: %d \n", list_size(l_ready));
		printf("ECEC: %d \n", list_size(l_exec));
		puts("lISTO P TRABAJAR");*/
		while(list_size(l_ready) > 0 && list_size(l_exec) < grado_multiproc && list_size(l_suspended_ready) == 0) {
			sem_wait(&multiproc);
			if(list_size(l_ready) != 0){
				t_carpincho* turno = buscar_carpincho(algoritmo);
				cambiar_estado(EXEC,turno);
				//printf(" -----------------> SELECCIONA CARPINCHO: %d \n", turno->pid);
				pthread_t tarea;
				pthread_create(&tarea, NULL, (void*) hacer_tarea, turno);
				pthread_detach((pthread_t) tarea);
			}
		}
		sem_post(&listo_para_trabajar);
	}
}

void planificador_mediano_plazo_suspender(){
	int multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

	while(true){
		sem_wait(&listo_para_trabajar_susp);
		while(list_size(l_ready) == 0 && list_size(l_blocked) != 0 && list_size(l_new) != 0){
			t_carpincho* objetivo = list_get(l_blocked,list_size(l_blocked)-1);
			cambiar_estado(SUSPENDED_BLOCKED, objetivo);
			puts("1");
			if((list_size(l_ready) + list_size(l_exec) + list_size(l_blocked)) < multiprog){
				//puts("Selecciona uno \n");
				if(list_size(l_new) > 0){
					cambiar_estado(READY,list_get(l_new,0));
				}
			}
			puts("2");
			conexion = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA")); //conexion a memoria
			puts("3");
			if(conexion != -1){
				t_buffer* buffer = serializar_suspender(objetivo->pid);
				t_paquete* paquete = crear_mensaje(buffer, 17); //TODO cod op de conexiones.h no lo toma
				enviar_paquete(paquete, conexion);
				free(buffer);
				free(paquete);
			}
			close(conexion);
		}
		sem_post(&listo_para_trabajar_susp);
	}
}

void planificador_mediano_plazo_ready(){
	int multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

	while(true){
		sem_wait(&listo_para_trabajar_ready);
		while(list_size(l_suspended_ready) > 0 && (list_size(l_ready) + list_size(l_exec) + list_size(l_blocked)) < multiprog){
			cambiar_estado(READY, list_get(l_suspended_ready,0));
		}
		sem_post(&listo_para_trabajar_ready);
	}
}

t_carpincho* buscar_carpincho(char* algoritmo){
    void* t_sjf(t_carpincho* c1, t_carpincho* c2){
    	return (sjf(c1) <= sjf(c2)) ? c1 : c2;
    }

    void* t_hrrn(t_carpincho* c1, t_carpincho* c2){
        return hrrn(c1) >= hrrn(c2) ? c1 : c2;
    }

	if(strcmp(algoritmo,"SJF") == 0){
		if(list_size(l_ready) == 1){
			sjf(list_get(l_ready,0));
			return list_get(l_ready,0);
		}
	    return list_get_minimum(l_ready,(void*)t_sjf);
	}else if(strcmp(algoritmo,"HRRN") == 0){
		if(list_size(l_ready) == 1){
			hrrn(list_get(l_ready,0));
			return list_get(l_ready,0);
		}

		return list_get_maximum(l_ready,(void*)t_hrrn);
	}else{
		return list_get(l_ready,0);
	}
}

double sjf(t_carpincho* c){
	double alfa = config_get_double_value(config, "ALFA");
	/*puts("==========================================");*/
	if(c->ultimo_estimado == -1){
		double t_est = config_get_double_value(config, "ESTIMACION_INICIAL");
		c->ultimo_estimado = (alfa*0 + (1-alfa)*t_est);
		return c->ultimo_estimado;
	}if(c->ultima_rafaga == -1){
		return c->ultimo_estimado;
	}

	c->ultimo_estimado = (alfa*c->ultima_rafaga + (1-alfa)*c->ultimo_estimado);
	//printf("CARPINCHO %d - ESTIMADO: %f \n", c->pid, c->ultimo_estimado);
	return c->ultimo_estimado;
}

double hrrn(t_carpincho* c){
	double s;
	double alfa = config_get_double_value(config, "ALFA");
	double w = timestamp() - c->tiempo_llegada;

	//puts("==========================================");

	if(c->ultimo_estimado == -1 && c->ultima_rafaga == -1){
		double t_est = config_get_double_value(config, "ESTIMACION_INICIAL");
		c->ultimo_estimado = (alfa*0 + (1-alfa)*t_est);
	}else if(c->ultimo_estimado != -1 && c->ultima_rafaga == -1){

	}else{
		c->ultimo_estimado = (alfa*c->ultima_rafaga + (1-alfa)*c->ultimo_estimado);
	}
	s = c->ultimo_estimado;

	//printf("CARPINCHO %d - ESTIMADO: %f \n", c->pid, (w+s)/s);
	return (w+s)/s;
}

void hacer_tarea(void* args){
	t_carpincho* carpincho = args;
	int retorno = -1, cod_op;
	double inicio_exec = timestamp();
	int32_t rta_memalloc = -1;

	bool buscar_semaforo(void* sem_en_lista) {return string_equals_ignore_case(((t_semaforo*)sem_en_lista)->nombre,carpincho->instruccion->nombre_sem_io);}
	bool buscar_dispositivo(void* io_en_lista) {return string_equals_ignore_case(((t_io*)io_en_lista)->nombre,carpincho->instruccion->nombre_sem_io);}

	while(carpincho->estado == EXEC){
		if(string_equals_ignore_case(carpincho->esperando, "VACIO")){
			cod_op = recibir_operacion(carpincho->socket_carpincho);
			if(cod_op == DELETE || cod_op == S_CREATE || cod_op == S_WAIT || cod_op == S_POST || cod_op == S_DELETE || cod_op == CALL_IO){
				free(carpincho->instruccion);
				carpincho->instruccion = NULL;
				carpincho->instruccion = recibir_instruccion(cod_op,carpincho->socket_carpincho);
			}else if(cod_op == MEMFREE || cod_op == MEMREAD || cod_op == MEMWRITE || cod_op == MEMALLOC){
				retorno = (int) atender_op_memoria(retorno, rta_memalloc, cod_op, carpincho);
			}else if(cod_op == MEMALLOC){
				rta_memalloc = (int32_t) atender_op_memoria(retorno, rta_memalloc, cod_op, carpincho);
				printf("RETORNO %d \n", rta_memalloc);
			}else{
				return;
			}
		}else{
			responder(carpincho->instruccion->instruccion, EXITO, carpincho->socket_carpincho);
			carpincho->esperando = "VACIO";
			carpincho->ultima_rafaga = carpincho->ultima_rafaga + (inicio_exec - carpincho->ultima_rafaga) + (timestamp() - inicio_exec);
			cambiar_estado(READY,carpincho);
			return;
		}

		switch(cod_op){
			case DELETE:
				retorno = informar_finalizado(carpincho);
				break;

			case S_CREATE:
				pthread_mutex_lock(&existencia_semaforo);
				if(!list_any_satisfy(lista_semaforos,buscar_semaforo)){
					retorno = crear_semaforo(carpincho);
				}
				pthread_mutex_unlock(&existencia_semaforo);
				break;

			case S_WAIT:
				pthread_mutex_lock(&existencia_semaforo);
				if(list_any_satisfy(lista_semaforos,buscar_semaforo)){
					t_semaforo* objetivo = list_find(lista_semaforos,buscar_semaforo);
					objetivo->valor--;
					logear(SEM_WAITED,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);

					if(objetivo->valor < 0){
						pthread_mutex_unlock(&existencia_semaforo);
						cambiar_estado(BLOCKED,carpincho);
						carpincho->esperando = carpincho->instruccion->nombre_sem_io;
						carpincho->ultima_rafaga = inicio_exec - timestamp();
						return;
					}
					list_add(carpincho->tomando,objetivo);
				}else{
					logear(SEM_NOEXISTE,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					retorno = NO_EXISTE_SEMAFORO;
				}
				pthread_mutex_unlock(&existencia_semaforo);
				break;

			case S_POST:
				pthread_mutex_lock(&existencia_semaforo);
				if(list_any_satisfy(lista_semaforos,buscar_semaforo)){
					t_semaforo* objetivo = list_find(lista_semaforos,buscar_semaforo);
					objetivo->valor++;
					list_remove_by_condition(carpincho->tomando,buscar_semaforo);
					logear(SEM_POSTED,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					retorno = desbloquear_en_espera_sem(carpincho);
				}else{
					logear(SEM_NOEXISTE,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					retorno = NO_EXISTE_SEMAFORO;
				}
				pthread_mutex_unlock(&existencia_semaforo);
				break;

			case S_DELETE:
				pthread_mutex_lock(&existencia_semaforo);
				if(list_any_satisfy(lista_semaforos,buscar_semaforo)){
					list_remove_by_condition(lista_semaforos,buscar_semaforo);
					logear(SEM_DELETED,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					desbloquear_todos(carpincho);
					retorno = EXITO;
				}else{
					logear(SEM_NOEXISTE,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					retorno = NO_EXISTE_SEMAFORO;
				};
				pthread_mutex_unlock(&existencia_semaforo);
				break;

			case CALL_IO:
				if(list_any_satisfy(lista_dispositivos,buscar_dispositivo)){
					puts("1");
					t_io* objetivo = list_find(lista_dispositivos,buscar_dispositivo);
					puts("2");
					cambiar_estado(BLOCKED,carpincho);
					pthread_mutex_lock(&(objetivo->semaforo));

					if(objetivo->bloqueado == 1){
						pthread_mutex_unlock(&objetivo->semaforo);
						carpincho->esperando = carpincho->instruccion->nombre_sem_io;
						sem_wait(&carpincho->semaforo);
						log_info(logger, "CARPINCHO %d - Esperando semaforo", carpincho->pid);
						sem_wait(&carpincho->semaforo);
						pthread_mutex_lock(&objetivo->semaforo);
						log_info(logger, "CARPINCHO %d - Termina espera", carpincho->pid);
					}
					pthread_mutex_unlock(&(objetivo->semaforo));

					usar_io(objetivo,carpincho);
					carpincho->ultima_rafaga = timestamp() - inicio_exec;
					return;
				}else{
					logear(IO_NOEXISTE,carpincho->pid,1,carpincho->instruccion->nombre_sem_io);
					retorno = NO_EXISTE_IO;
				}
				break;

			default:
				break;
		}
		if(cod_op == MEMALLOC){
			responder_memalloc(cod_op, rta_memalloc, carpincho->socket_carpincho);
		}else{
			responder(cod_op, retorno, carpincho->socket_carpincho);
		}
		carpincho->esperando = "VACIO";
		carpincho->ultima_rafaga = inicio_exec - timestamp();
	}
	return;
}

void* atender_op_memoria(int retorno, uint32_t rta_memalloc, int cod_op, t_carpincho* carpincho){
	t_buffer* buffer = deserializar_parcial(cod_op,carpincho->socket_carpincho);
	t_paquete* paquete = crear_mensaje(buffer, cod_op);

	conexion = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA")); //conexion a memoria

	if(conexion != -1){
		enviar_paquete(paquete, conexion);
		free(buffer);
		free(paquete);
		logear(MEM_ENVIO,carpincho->pid,1,"");

		int cod_mem = recibir_operacion(conexion);
		printf("CODIGO_MEMORIA %d \n", cod_mem);
		logear(LOPMEM,carpincho->pid,1,"");
		close(conexion);

		if(cod_mem == 20){ //TODO poner cod de conexions q no lo toma
			return deserializar_rta_memalloc(conexion);
		}else if(cod_mem == RESPUESTA){
			return deserializar_rta(conexion);
		}else{
			return -1;
		}

	}else{
		free(buffer);
		free(paquete);
		log_error(logger, "Se perdió la conexión a memoria");
		return ERROR_DE_CONEXION;
	}

}

int desbloquear_en_espera_sem(t_carpincho* salido_de_io){
	bool desbloquear(void* carpincho_en_lista) {return string_equals_ignore_case(((t_carpincho*)carpincho_en_lista)->esperando,salido_de_io->instruccion->nombre_sem_io);}

	if(list_any_satisfy(l_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_blocked, desbloquear);
		cambiar_estado(READY,desbloqueado);
	}else if(list_any_satisfy(l_suspended_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_blocked, desbloquear);
		cambiar_estado(SUSPENDED_READY,desbloqueado);
	}

	return EXITO;
}

void desbloquear_en_espera_io(t_carpincho* carpincho){
	bool desbloquear(void* carpincho_en_lista) {return string_equals_ignore_case(((t_carpincho*)carpincho_en_lista)->esperando,carpincho->instruccion->nombre_sem_io);}

	if(list_any_satisfy(l_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_blocked, desbloquear);
		sem_post(&desbloqueado->semaforo);
		sem_post(&desbloqueado->semaforo);
	}else if(list_any_satisfy(l_suspended_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_suspended_blocked, desbloquear);
		sem_post(&desbloqueado->semaforo);
		sem_post(&desbloqueado->semaforo);
	}
}

void desbloquear_todos(t_carpincho* carpincho){
	bool desbloquear(void* carpincho_en_lista) {return string_equals_ignore_case(((t_carpincho*)carpincho_en_lista)->esperando,carpincho->instruccion->nombre_sem_io);}

	while(list_any_satisfy(l_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_blocked, desbloquear);
		list_remove_by_condition(desbloqueado->tomando,desbloquear);
		cambiar_estado(READY,desbloqueado);
	}
	while(list_any_satisfy(l_suspended_blocked, desbloquear)){
		t_carpincho* desbloqueado = list_find(l_blocked, desbloquear);
		list_remove_by_condition(desbloqueado->tomando,desbloquear);
		cambiar_estado(SUSPENDED_READY,desbloqueado);
	}
}

int crear_semaforo(t_carpincho* carpincho){
	t_semaforo* sem = malloc(sizeof(t_semaforo));
	sem->id = semaforo_id++;
	sem->nombre = carpincho->instruccion->nombre_sem_io;
	sem->valor = carpincho->instruccion->valor_semaforo;
	list_add(lista_semaforos,sem);
	logear(SEM_CREATED,carpincho->pid,1,sem->nombre);
	return EXITO;
}

void usar_io(t_io* objetivo, t_carpincho* carpincho){
	objetivo->bloqueado = 1;

	logear(IO_EN_USO,carpincho->pid,0,objetivo->nombre);
	sleep(objetivo->duracion);
	objetivo->bloqueado = 0;
	logear(IO_LIBERADO,carpincho->pid,0,objetivo->nombre);
	desbloquear_en_espera_io(carpincho);
	responder(carpincho->instruccion->instruccion, EXITO, carpincho->socket_carpincho);
	carpincho->esperando = "VACIO";
	cambiar_estado(READY,carpincho);
}

int informar_finalizado(t_carpincho* carpincho){
	conexion = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA")); //conexion a memoria

	if(conexion != -1){
		t_buffer* buffer = serializar_suspender(carpincho->pid);
		t_paquete* paquete = crear_mensaje(buffer, DELETE);
		enviar_paquete(paquete, conexion);
		close(conexion);
	}

	while(list_size(carpincho->tomando) > 0){
		t_semaforo* sem = list_remove(carpincho->tomando, 0);
		sem->valor++;
	}

	cambiar_estado(EXIT,carpincho);
	free(carpincho->instruccion);
	list_destroy(carpincho->tomando);
	logear(FINALIZADO,carpincho->pid,1,"");
	return EXITO;
}

void finalizar_por_dl(t_carpincho* carpincho){
	conexion = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA")); //conexion a memoria

	if(conexion != -1){
		t_buffer* buffer = serializar_suspender(carpincho->pid);
		t_paquete* paquete = crear_mensaje(buffer, DELETE);
		enviar_paquete(paquete, conexion);
		close(conexion);
	}

	while(list_size(carpincho->tomando) > 0){
		t_semaforo* sem = list_remove(carpincho->tomando, 0);
		sem->valor++;
	}
	cambiar_estado(EXIT,carpincho);
	free(carpincho->instruccion);
	list_destroy(carpincho->tomando);
	logear(FINALIZADO,carpincho->pid,1,"");

	responder(RESPUESTA, FINALIZADO_POR_DEADLOCK, carpincho->socket_carpincho);
}

void detectar_deadlock(){
	int tiempo = config_get_int_value(config, "DURACIONES_IO"); //TODO eliminar, esperar y volver a chequear o eliminar chequear hasta que no haya?

    void* a_eliminar(t_carpincho* c1, t_carpincho* c2){
        return (c1->pid) >= (c2->pid) ? c1 : c2;
    }
    bool bloq_sem(void* carpincho_en_lista) {
    	return !string_equals_ignore_case(((t_carpincho*)carpincho_en_lista)->esperando,"VACIO");
    }

	t_list* posible_dl = list_create();
	while(1){
		sleep(tiempo); //TODO espera activa
		if(list_size(l_blocked) > 1){
			t_list* aux1 = list_filter(l_blocked, bloq_sem);
			while(list_size(aux1) > 0){
				list_add(posible_dl, list_remove(aux1, 0));
			}
		}
		if(list_size(l_suspended_blocked) > 1){
			t_list* aux2 = list_filter(l_blocked, bloq_sem);
			while(list_size(aux2) > 0){
				list_add(posible_dl, list_remove(aux2, 0));
			}
		}
		if(list_size(posible_dl) > 0){
			buscar_deadlock(posible_dl);
		}
	}
}

void buscar_deadlock(t_list* lista){
	t_list* espera_circular = list_create();
	t_carpincho* aux;
	t_semaforo* aux_s;
/*
	bool buscar_semaforo(void* sem_en_lista) {return string_equals_ignore_case(((t_semaforo*)sem_en_lista)->nombre,objetivo->esperando);}

	bool bloqueado_por(void* c_en_lista) {return string_equals_ignore_case(((t_carpincho*)c_en_lista)->esperando,objetivo->esperando);}*/

	bool buscar_carpincho(void* c_en_lista) {return ((t_carpincho*)c_en_lista)->pid == aux->pid;}

	//t_semaforo* sem = list_find(lista_semaforos,buscar_semaforo);

}

