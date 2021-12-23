#include "paginacion.h"

extern t_config_memoria* cfg;

int tlb_hit_totales;
int tlb_miss_totales;

void iniciar_mutex(){
	pthread_mutex_init(&MUTEX_MP, NULL);
	pthread_mutex_init(&MUTEX_FRAME, NULL);
	pthread_mutex_init(&MUTEX_TLB, NULL);
	pthread_mutex_init(&MUTEX_TABLAS_DE_PAGINAS, NULL);
}

void finalizar_mutex(){
	pthread_mutex_destroy(&MUTEX_MP);
	pthread_mutex_destroy(&MUTEX_FRAME);
	pthread_mutex_destroy(&MUTEX_TLB);
	pthread_mutex_destroy(&MUTEX_TABLAS_DE_PAGINAS);
}
//MEMS
uint32_t memalloc(uint32_t pid, uint32_t size){
	puts("1");
	pthread_mutex_lock(&MUTEX_MP);
	puts("2");
	t_tabla_de_paginas* tabla = list_find_by_pid(pid);
	puts("3");
	if(tabla == NULL){ //carpincho nuevo
		if(entra_en_mp(size+2*sizeof(HeapMetadata))){//si ademas entra en memoria
			puts("3.1");
			crear_tabla_de_paginas(pid);
			puts("3.2");
			tabla = list_find_by_pid(pid);
			puts("3.3");
			printf("PAGINAS: %d \n", list_size(tabla->paginas));
			puts("3.3");
			t_frame* frame = encontrar_frame_vacio();
			puts("3.4");
			crear_pagina(pid,tabla, frame->nroFrame);
			puts("3.5");
			HeapMetadata *meta = malloc(sizeof(HeapMetadata));
			puts("3.6");
			meta->isFree=1;
			meta->prevAlloc=(uint32_t)NULL;
			meta->nextAlloc=(uint32_t)NULL;
			puts("3.7");
			/*memcpy_pagina_en_frame(frame->nroFrame,0,(void*) meta->prevAlloc,sizeof(uint32_t));
			memcpy_pagina_en_frame(frame->nroFrame,4,(void*) meta->nextAlloc,sizeof(uint32_t));
			memcpy_pagina_en_frame(frame->nroFrame,8,(void*) meta->isFree,sizeof(uint8_t));
			*/

			memcpy(inicio_memoria + frame->nroFrame*cfg->TAMANIO_PAGINA + 0, (void*)(&meta->prevAlloc), sizeof(uint32_t) );
			memcpy(inicio_memoria + frame->nroFrame*cfg->TAMANIO_PAGINA + 4, (void*)(&meta->nextAlloc), sizeof(uint32_t) );
			memcpy(inicio_memoria + frame->nroFrame*cfg->TAMANIO_PAGINA + 8, (void*)(&meta->isFree), sizeof(uint8_t) );
			puts("3.8");
			free(frame);
			free(meta);
		}else{
			return -1;
		}
	}
	puts("4");
	uint32_t dirLog = alloc_disponible(pid,tabla,size);
	puts("5");
	free(tabla);
	pthread_mutex_unlock(&MUTEX_MP);
	printf("TLB HIT: %d \n", tabla->tlbHit);
	printf("TLB MISS: %d \n", tabla->tlbMiss);
	printf("SIZE TABLA: %d \n", list_size(tabla->paginas));
	puts("6");
	return dirLog;
}

uint32_t alloc_disponible(uint32_t pid,t_tabla_de_paginas* tabla, uint32_t size){
	double nroPag=0;
	double desplazamiento=0;
	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina=list_get(tabla->paginas, (int)nroPag);

	//pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid); TODO

	if(!pagina->bit_presencia){//si la pagina no esta en mp
		//traer_a_mp()
	}

	puts("4.1");
	void* data = malloc(cfg->TAMANIO_PAGINA*tabla->paginas->elements_count);
	puts("4.2");
	memcpy(data,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
	puts("4.2.1");
	uint32_t* nextAlloc = (uint32_t*)malloc(sizeof(uint32_t));
	puts("4.3");
	memcpy(nextAlloc, data+4, sizeof(uint32_t));
	uint8_t* isFree = (uint8_t*)malloc(sizeof(uint8_t));;
	puts("4.4");
	memcpy(isFree, data+8, sizeof(uint8_t));
	uint32_t allocActual = (uint32_t)malloc(sizeof(uint32_t));
	allocActual=9;
	uint32_t espacioDisponible = (uint32_t)malloc(sizeof(uint32_t));
	puts("4.5");
	if((*nextAlloc)==(uint32_t)NULL){
		espacioDisponible=size;
	}else{
		espacioDisponible=(*nextAlloc)-allocActual;
	}
	printf("ESPACIO: %d \n", espacioDisponible);
	printf("SIZE: %d \n", size);
	printf("ISFREE: %d \n", (*isFree));
	puts("4.6");
	//RECORRO LOS ALLOCS PARA ENCONTRAR UNO DISPONIBLE
	int a = cfg->TAMANIO_PAGINA;
	while(((*isFree) == 0) || size > espacioDisponible){//si no esta libre o no hay es espacio suficiente hay que ir a buscar otro alloc
		puts("4.7");
		allocActual=(*nextAlloc)+9;
		puts("4.8");
		desplazamiento = (*nextAlloc)%a;
		puts("4.9");
		if(allocActual-((int)nroPag+1)*cfg->TAMANIO_PAGINA>=0){	//si el alloc siguiente esta en otra pagina
			if((int)(*nextAlloc)/a>(int)nroPag){//si el nextAlloc esta en otra pagina
				nroPag = (*nextAlloc)/a;
				pagina = list_get(tabla->paginas, (int)nroPag);
				pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

				if(!(pagina->bit_presencia)){
					//si no esta cargada en memoria traer_a_mp()
				}
				memcpy(data+(int)nroPag*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
				if((((int)nroPag+1)*cfg->TAMANIO_PAGINA-(*nextAlloc))<9){//si el metadata esta cortado
					pagina = list_get(tabla->paginas, (int)nroPag+1);

					pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

					if(!(pagina->bit_presencia)){
						//si no esta cargada en memoria traer_a_mp()
					}
					memcpy(data+((int)nroPag+1)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
					memcpy(nextAlloc, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+4, sizeof(uint32_t));
					memcpy(isFree, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+8, sizeof(uint8_t));
				}else{//como el metadata no esta cortado puedo leer tranquilo
					memcpy(nextAlloc, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+4, sizeof(uint32_t));
					memcpy(isFree, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+8, sizeof(uint8_t));
				}
			}else{//si el nextAlloc esta en la misma pagina en la que estoy pero el alloc esta en la otra
				pagina = list_get(tabla->paginas, (int)nroPag+1);

				pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

				if(!(pagina->bit_presencia)){
					//si no esta cargada en memoria traer_a_mp()
				}
				memcpy(data+((int)nroPag+1)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
				memcpy(nextAlloc, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+4, sizeof(uint32_t));
				memcpy(isFree, data+(int)nroPag*cfg->TAMANIO_PAGINA+(int)desplazamiento+8, sizeof(uint8_t));
			}
		}else{//si el alloc esta en la misma pagina en la que estoy
			memcpy(nextAlloc, data+(int)desplazamiento+4, sizeof(uint32_t));
			memcpy(isFree, data+(int)desplazamiento+8, sizeof(uint8_t));
		}
		if((*nextAlloc)==(uint32_t)NULL){
			espacioDisponible=size;
		}else{
			espacioDisponible = (*nextAlloc)-allocActual;
		}
	}

	//Pongo el free en 0
	puts("4.7");
	if(cfg->TAMANIO_PAGINA-((int)desplazamiento+8)==0){//si el free no esta en la pagina del nextalloc+8 sino en la sigueinte
		puts("4.8");
		memcpy_pagina_en_frame(pagina->nro_frame,(int)desplazamiento+8,(void*)0,sizeof(uint8_t));
	}else{//el free esta en la misma pagina del nextalloc+8, y puedo actualizar el free tranquilo
		puts("4.8B");
		pagina = list_get(tabla->paginas, (int)nroPag);
		uint8_t libre = 0;
		memcpy(inicio_memoria + pagina->nro_frame*cfg->TAMANIO_PAGINA + (int)desplazamiento + 8, (void*)(&libre), sizeof(uint8_t));
		//memcpy_pagina_en_frame(pagina->nro_frame,(int)desplazamiento+8,(void*)0,sizeof(uint8_t));
	}
	puts("4.9");
	if((*nextAlloc)==(uint32_t)NULL){//ES el ULTIMO ALLOC
		puts("4.10");
		int cantCachosACrear=(int)(((size+9-(cfg->TAMANIO_PAGINA-(((int)nroPag+1)*cfg->TAMANIO_PAGINA)-allocActual)))/a)+1;//el +1 es porque el (int) redondea para abajo
		puts("4.11");
		if(string_equals_ignore_case(cfg->TIPO_ASIGNACION,"FIJA") && tabla->paginas->elements_count+cantCachosACrear>cfg->MARCOS_POR_PROCESO){//SI TENGO QUE CREAR MAS PAGINAS DE LAS QUE ME DEJA LA ASIGNACION FIJA
			return -1;
		}
		puts("4.12");
		if(cfg->TAMANIO_PAGINA-(int)desplazamiento+4 < 5){//es decir que el nextalloc del metadata del allocActual esta cortado
			puts("4.13");
			pagina=list_get(tabla->paginas,(int)nroPag);
			puts("4.14");
			memcpy_pagina_en_frame(pagina->nro_frame, (int)desplazamiento+4, (void*)(allocActual+size),cfg->TAMANIO_PAGINA-(int)desplazamiento+4);
			pagina=list_get(tabla->paginas,(int)nroPag+1);
			puts("4.15");
			memcpy_pagina_en_frame(pagina->nro_frame, 0, (void*)(allocActual+size),4-(cfg->TAMANIO_PAGINA-(int)desplazamiento+4));
		}else{//puedo consolidar el nextAlloc tranquilo
			puts("4.16");
			//memcpy_pagina_en_frame(pagina->nro_frame, (int)desplazamiento+4, (void*)(allocActual+size),sizeof(uint32_t));
			uint32_t valor = allocActual + size;
			memcpy(inicio_memoria + pagina->nro_frame*cfg->TAMANIO_PAGINA + (int)desplazamiento + 4, (void*)(&valor), sizeof(uint32_t));
		}
		puts("4.17");
		t_frame* frame = malloc(sizeof(t_frame));
		for(int i=0; i<cantCachosACrear;i++){//tengo que crear paginas y poner en al ultima la metadata
			puts("4.18");
			frame = encontrar_frame_vacio();
			puts("4.18.1");
			crear_pagina(pid,tabla, frame->nroFrame);
			puts("4.18.2");
		}
		puts("4.19");
		desplazamiento = (size+allocActual) % cfg->TAMANIO_PAGINA;
		HeapMetadata *meta = malloc(sizeof(HeapMetadata));
		meta->isFree=1;
		meta->prevAlloc=allocActual-9;
		meta->nextAlloc=(uint32_t)NULL;
		puts("4.20");
		if(cfg->TAMANIO_PAGINA - (int)desplazamiento < 9){//si tengo que meter la metadata de a cachos
			puts("4.21");
			memcpy_pagina_en_frame(frame->nroFrame,(int)desplazamiento,(void*)meta,cfg->TAMANIO_PAGINA-(int)desplazamiento);
			puts("4.22");
			frame = encontrar_frame_vacio();
			crear_pagina(pid,tabla, frame->nroFrame);
			puts("4.23");
			memcpy_pagina_en_frame(frame->nroFrame,(int)desplazamiento,(void*)meta+(cfg->TAMANIO_PAGINA-(int)desplazamiento),9-(cfg->TAMANIO_PAGINA-(int)desplazamiento));
		}
		else{//si puedo meter la metadata de una
			puts("4.24");
			memcpy_pagina_en_frame(frame->nroFrame,(int)desplazamiento,(void*)meta,sizeof(HeapMetadata));
			puts("4.25");
		}
		puts("4.26");
		free(frame);
		free(meta);
	}

	free(pagina);
	free(data);
	free(nextAlloc);
	free(isFree);
	return allocActual;
}

void memfree(uint32_t pid, uint32_t dir_log){
	/*
	pthread_mutex_lock(&MUTEX_MP);
	t_tabla_de_paginas* tabla = list_find_by_pid(pid);
	double nroPag;
	double desplazamiento;
	desplazamiento = modf(dir_log/cfg->TAMANIO_PAGINA,&nroPag);
	desplazamiento = desplazamiento*cfg->TAMANIO_PAGINA;

	t_pagina *pagina = malloc(sizeof(t_pagina));
	pagina->bitMod = 0; //TODO está bien esto así? ver csasos libero toda la pagina vs no toda
	void* data = malloc(cfg->TAMANIO_PAGINA*tabla->paginas->elements_count);
	uint32_t* prevAlloc = (uint32_t*)malloc(sizeof(uint32_t));
	uint32_t* nextAlloc = (uint32_t*)malloc(sizeof(uint32_t));
	uint8_t* isFree = (uint8_t*)malloc(sizeof(uint8_t));
	uint32_t tamanioDatosABorrar;
	if(desplazamiento<5){//no puedo leer el nextAlloc para saber el tamaño del alloc porque esta cortado
		pagina = list_get(tabla->paginas, (int)nroPag-1);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data+((int)nroPag-1)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);

		pagina = list_get(tabla->paginas, (int)nroPag);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data+(int)nroPag*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
		desplazamiento+=(int)nroPag*cfg->TAMANIO_PAGINA;

		//Con esto ya tengo las dos paginas cargadas en la data y puedo leer el prev alloc y el next alloc tranquilo
		memcpy(prevAlloc, data+(int)desplazamiento-9, sizeof(uint32_t));
		memcpy(nextAlloc, data+(int)desplazamiento-5, sizeof(uint32_t));
		tamanioDatosABorrar = (*nextAlloc)-dir_log;

	}else{//puedo leer el nextAlloc
		pagina = list_get(tabla->paginas, (int)nroPag);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
		memcpy(prevAlloc, data+(int)desplazamiento-9, sizeof(uint32_t));
		memcpy(nextAlloc, data+(int)desplazamiento-5, sizeof(uint32_t));
		tamanioDatosABorrar = (*nextAlloc)-dir_log;
	}

	//Leo si el prevalloc esta libre o no
	double nroPagPrevAlloc;
	double desplazamientoPrevAlloc = 0;
	desplazamientoPrevAlloc = modf((*prevAlloc)/cfg->TAMANIO_PAGINA,&nroPagPrevAlloc);

	t_pagina* paginaPrevAlloc = malloc(sizeof(t_pagina));
	paginaPrevAlloc = list_get(tabla->paginas, (int)nroPagPrevAlloc);

	pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

	if(!paginaPrevAlloc->bit_presencia){//si la pagina no esta en mp
		//traer_a_mp()
	}
	memcpy(data+(int)nroPagPrevAlloc*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);

	if(((int)nroPagPrevAlloc+1)*cfg->TAMANIO_PAGINA-(int)desplazamientoPrevAlloc<9){//el metadata esta cortado
		paginaPrevAlloc = list_get(tabla->paginas, (int)nroPagPrevAlloc+1);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!paginaPrevAlloc->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data+((int)nroPagPrevAlloc+1)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);

		memcpy(isFree, data+((int)nroPagPrevAlloc+1)*cfg->TAMANIO_PAGINA+(8-(((int)nroPagPrevAlloc+1)*cfg->TAMANIO_PAGINA-(int)desplazamientoPrevAlloc)), sizeof(uint8_t));
	}else{
		memcpy(isFree, data+(int)nroPagPrevAlloc*cfg->TAMANIO_PAGINA+(int)desplazamientoPrevAlloc, sizeof(uint8_t));
	}

	//Si el alloc anterior esta libre hay que liberar y consolidar con ese anterior
	if(isFree){

	}

	int tamanioDatosBorrados=0;
	double cantidadDePedazosABorrar;
	cantidadDePedazosABorrar=tamanioDatosABorrar/cfg->TAMANIO_PAGINA;
	for(int i =0; i<=(int)cantidadDePedazosABorrar;i++){
		if(((*nextAlloc)-nroPag*cfg->TAMANIO_PAGINA)/cfg->TAMANIO_PAGINA >1){//si el alloc esta en otra pagina tengo que leer desde donde estoy hasta el fin de la pagina
			if(i==0){//tengo que leer desde donde estoy hasta el fin de la pagina
				memcpy(datosLeidos+tamanioDatosBorrados,data+dir_log-cfg->TAMANIO_PAGINA*((int)nroPag),cfg->TAMANIO_PAGINA*((int)nroPag+1)-dir_log);
				tamanioDatosBorrados+=cfg->TAMANIO_PAGINA*((int)nroPag+1)-dir_log;
				tamanioDatosABorrar-=tamanioDatosBorrados;
			}else{//tengo que leer la pagina entera
				memcpy(datosLeidos+tamanioDatosBorrados,data+dir_log-cfg->TAMANIO_PAGINA*((int)nroPag)+tamanioDatosBorrados,cfg->TAMANIO_PAGINA);
				tamanioDatosBorrados+=cfg->TAMANIO_PAGINA;
				tamanioDatosABorrar-=tamanioDatosBorrados;
			}
			nroPag+=1;
			pagina = list_get(tabla->paginas, (int)nroPag);
			if(!pagina->bit_presencia){//si la pagina no esta en mp
				//traer_a_mp()
			}
			memcpy(data+dir_log-cfg->TAMANIO_PAGINA*((int)nroPag)+tamanioDatosBorrados,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
			desplazamiento=0;
		}else{//si el nextalloc esta en la misma pagina significa que puedo terminar de leer los datos
			memcpy(datosLeidos+tamanioDatosBorrados, data+dir_log-cfg->TAMANIO_PAGINA*((int)nroPag)+tamanioDatosBorrados,tamanioDatosABorrar);
			tamanioDatosBorrados+=tamanioDatosABorrar;
			tamanioDatosABorrar-=tamanioDatosBorrados;
		}
	}

	free(nextAlloc);
	//free(data)
	pthread_mutex_unlock(&MUTEX_MP);
	*/
}

void* memread(uint32_t pid, uint32_t direccionLogica, uint32_t *tamanioData){
	pthread_mutex_lock(&MUTEX_MP);
	t_tabla_de_paginas* tabla = list_find_by_pid(pid);
	double nroPag;
	double desplazamiento;
	int a = cfg->TAMANIO_PAGINA;
	desplazamiento = modf(direccionLogica/a,&nroPag);
	desplazamiento = desplazamiento*cfg->TAMANIO_PAGINA;

	t_pagina* pagina = malloc(sizeof(t_pagina));
	void* data = malloc(cfg->TAMANIO_PAGINA*tabla->paginas->elements_count);
	void* datosLeidos;
	uint32_t* nextAlloc = (uint32_t*)malloc(sizeof(uint32_t));
	uint32_t tamanioDatosALeer;
	if(desplazamiento<5){//no puedo leer el nextAlloc para saber el tamaño del alloc porque esta cortado
		pagina = list_get(tabla->paginas, (int)nroPag-1);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
		desplazamiento+=(int)nroPag*cfg->TAMANIO_PAGINA;

		pagina = list_get(tabla->paginas, (int)nroPag);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);

		//Con esto ya tengo las dos paginas cargadas en la data y puedo leer el next alloc tranquilo
		memcpy(nextAlloc, data+(int)desplazamiento-5, sizeof(uint32_t));
		tamanioDatosALeer = (*nextAlloc)-direccionLogica;

	}else{//puedo leer el nextAlloc
		pagina = list_get(tabla->paginas, (int)nroPag);

		pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

		if(!pagina->bit_presencia){//si la pagina no esta en mp
			//traer_a_mp()
		}
		memcpy(data,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
		memcpy(nextAlloc, data+(int)desplazamiento-5, sizeof(uint32_t));
		tamanioDatosALeer = (*nextAlloc)-direccionLogica;
	}

	double cantidadDePedazosALeer;
	cantidadDePedazosALeer=tamanioDatosALeer/cfg->TAMANIO_PAGINA;

	for(int i =0; i<=(int)cantidadDePedazosALeer;i++){
		if(((*nextAlloc)-nroPag*cfg->TAMANIO_PAGINA)/cfg->TAMANIO_PAGINA >1){//si el alloc esta en otra pagina tengo que leer desde donde estoy hasta el fin de la pagina
			if(i==0){//tengo que leer desde donde estoy hasta el fin de la pagina
				memcpy(datosLeidos+(*tamanioData),data+direccionLogica-cfg->TAMANIO_PAGINA*((int)nroPag),cfg->TAMANIO_PAGINA*((int)nroPag+1)-direccionLogica);
				(*tamanioData)+=cfg->TAMANIO_PAGINA*((int)nroPag+1)-direccionLogica;
				tamanioDatosALeer-=(*tamanioData);
			}else{//tengo que leer la pagina entera
				memcpy(datosLeidos+(*tamanioData),data+direccionLogica-cfg->TAMANIO_PAGINA*((int)nroPag)+(*tamanioData),cfg->TAMANIO_PAGINA);
				(*tamanioData)+=cfg->TAMANIO_PAGINA;
				tamanioDatosALeer-=(*tamanioData);
			}
			nroPag+=1;
			pagina = list_get(tabla->paginas, (int)nroPag);

			pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

			if(!pagina->bit_presencia){//si la pagina no esta en mp
				//traer_a_mp()
			}
			memcpy(data+direccionLogica-cfg->TAMANIO_PAGINA*((int)nroPag)+(*tamanioData),get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
			desplazamiento=0;
		}else{//si el nextalloc esta en la misma pagina significa que puedo terminar de leer los datos
			memcpy(datosLeidos+(*tamanioData), data+direccionLogica-cfg->TAMANIO_PAGINA*((int)nroPag)+(*tamanioData),tamanioDatosALeer);
			(*tamanioData)+=tamanioDatosALeer;
			tamanioDatosALeer-=(*tamanioData);
		}
	}
	free(tabla);
	free(pagina);
	free(data);
	free(nextAlloc);
	pthread_mutex_unlock(&MUTEX_MP);
	return datosLeidos;
}
/*
void memread(uint32_t pid, uint32_t direccionLogica, uint32_t tamanioData, void* destino){
	pthread_mutex_lock(&MUTEX_MP);
	t_tabla_de_paginas* tabla = malloc(sizeof(t_tabla_de_paginas));
	tabla = list_find_by_pid(pid);
	double nroPag;
	double desplazamiento;
	int a = cfg->TAMANIO_PAGINA;
	desplazamiento = modf(direccionLogica/a,&nroPag);
	desplazamiento = desplazamiento*cfg->TAMANIO_PAGINA;

	t_pagina* pagina = malloc(sizeof(t_pagina));
	pagina = list_get(tabla->paginas,(int)nroPag);
	void* data = malloc(cfg->TAMANIO_PAGINA*tabla->paginas->elements_count);
	void* datosLeidos = malloc(tamanioData);

	int dataLeida = 0;
	int hastaDondeLeer = direccionLogica+tamanioData;
	double cantidadDePedazosALeer = hastaDondeLeer/a;
	for(int i =0; i<=(int)cantidadDePedazosALeer;i++){
		if((hastaDondeLeer-(int)nroPag*cfg->TAMANIO_PAGINA)/a >1){//si el alloc esta en otra pagina tengo que leer desde donde estoy hasta el fin de la pagina
			if(i==0){//tengo que leer desde donde estoy hasta el fin de pagina
				memcpy(data+((int)nroPag)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
				memcpy(datosLeidos,data,cfg->TAMANIO_PAGINA*((int)nroPag+1)-direccionLogica);
				dataLeida+=cfg->TAMANIO_PAGINA*((int)nroPag+1)-direccionLogica;
			}else{//tengo que leer una pagina entera
				memcpy(data+((int)nroPag)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
				memcpy(datosLeidos+dataLeida,data+((int)nroPag)*cfg->TAMANIO_PAGINA,cfg->TAMANIO_PAGINA);
				dataLeida+=cfg->TAMANIO_PAGINA;
			}
			nroPag+=1;
			pagina = list_get(tabla->paginas, (int)nroPag);

			pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

			if(!pagina->bit_presencia){//si la pagina no esta en mp
				//traer_a_mp()
			}
			desplazamiento=0;
		}else{//si el nextalloc esta en la misma pagina significa que puedo terminar de leer los datos
			memcpy(data+((int)nroPag)*cfg->TAMANIO_PAGINA,get_pagina_data(pagina->nro_frame),cfg->TAMANIO_PAGINA);
			memcpy(datosLeidos+dataLeida,data+((int)nroPag)*cfg->TAMANIO_PAGINA,tamanioData-dataLeida);
		}
	}

	memcpy(destino,datosLeidos, tamanioData);

	free(tabla);
	free(pagina);
	free(data);
	free(datosLeidos);
	pthread_mutex_unlock(&MUTEX_MP);
}
*/
void memwrite(uint32_t pid, uint32_t dir_log, uint32_t size, void* data){
	pthread_mutex_lock(&MUTEX_MP);
	t_tabla_de_paginas* tabla = malloc(sizeof(t_tabla_de_paginas));

	tabla = list_find_by_pid(pid);

	if(tabla->paginas != NULL){
		puts("NO ES NULO");
		//printf("SIZE TABLA: %d \n", list_size( (list_find_by_pid(pid)))->paginas   );
	}

	double nroPag;
	double desplazamiento;
	int a = cfg->TAMANIO_PAGINA;

	puts("4.1");
	desplazamiento = modf(dir_log/a,&nroPag);
	puts("4.2");
	desplazamiento = desplazamiento*cfg->TAMANIO_PAGINA;
	printf("NRO PAG: %d \n", (int)nroPag);
	printf("ID TABLA: %d \n", tabla->pid);
	printf("ELEM COUNT: %d \n", list_size(tabla->paginas->elements_count));
	printf("SIZE TABLA: %d \n", list_size(tabla->paginas));
	int nro_pag = nroPag;
	puts("4.3");

	t_pagina* pagina = list_get(tabla->paginas, nro_pag);
	pagina->bitMod = 0;
	puts("4.4");
	//pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);
	puts("4.5");
	if(!pagina->bit_presencia){//si la pagina no esta en mp
		//traer_a_mp()
	}
	puts("4.6");
	int dataLeida=0;
	int hastaDondeLeer = dir_log+size;
	double cantidadDePedazosALeer;

	cantidadDePedazosALeer=hastaDondeLeer/a;
	puts("4.7");
	for(int i =0; i<=(int)cantidadDePedazosALeer; i++){
		puts("4.8");
		if((hastaDondeLeer-(int)nroPag*cfg->TAMANIO_PAGINA)/a >1){//si el alloc esta en otra pagina tengo que escribir desde donde estoy hasta el fin de la pagina
			puts("4.9");

			if(i==0){//tengo que escribir desde donde estoy hasta el fin de pagina
				puts("4.9.1");
				memcpy_pagina_en_frame(pagina->nro_frame,dir_log-cfg->TAMANIO_PAGINA*((int)nroPag),data,cfg->TAMANIO_PAGINA*((int)nroPag+1)-dir_log);
				puts("4.9.2");
				dataLeida+=cfg->TAMANIO_PAGINA*((int)nroPag+1)-dir_log;
				puts("4.9.3");
			}else{//tengo que esribir una pagina entera
				memcpy_pagina_en_frame(pagina->nro_frame,0,data+dataLeida,cfg->TAMANIO_PAGINA);
				dataLeida+=cfg->TAMANIO_PAGINA;
			}

			puts("4.10");
			nroPag+=1;
			pagina = list_get(tabla->paginas, (int)nroPag);

			//pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);

			if(!pagina->bit_presencia){//si la pagina no esta en mp
				//traer_a_mp()
			}
			desplazamiento=0;
		}else{//si el nextalloc esta en la misma pagina significa que puedo terminar de escribir los datos
			memcpy_pagina_en_frame(pagina->nro_frame,(int)desplazamiento,data+dataLeida,size-dataLeida);
		}
	}

	free(tabla);
	free(pagina);
	pthread_mutex_unlock(&MUTEX_MP);
}

void suspender_carpincho(uint32_t pid){
	pthread_mutex_lock(&MUTEX_MP);
	t_tabla_de_paginas* tabla = list_find_by_pid(pid);
	for(int i=0; i<tabla->paginas->elements_count;i++){
		t_pagina* pagina = list_get(tabla->paginas, i);
		mandar_a_swap(pid,pagina);
	}
	pthread_mutex_unlock(&MUTEX_MP);
}

void traer_a_mp(uint32_t pid, t_pagina*pagina){

}

void mandar_a_swap(uint32_t pid, t_pagina* pagina){

}

/*
void LRU(){

}

void CLOCK_MODIFICADO();
*/
//
void crear_tabla_de_paginas(uint32_t pid){
	tlb_hit_totales = 0;
	tlb_miss_totales = 0;
	t_tabla_de_paginas *tablaDePaginas = malloc(sizeof(t_tabla_de_paginas));
	tablaDePaginas->pid=pid;
	tablaDePaginas->paginas=list_create();
	printf("SIZE TABLA: %d \n", list_size(tablaDePaginas->paginas));
	puts("B");
	tablaDePaginas->tlbHit = 0;
	tablaDePaginas->tlbMiss = 0;
	list_add(lista_de_tablas_de_paginas,tablaDePaginas);
}

void crear_pagina(uint32_t pid, t_tabla_de_paginas *tablaDePaginas, uint32_t nroFrame){
	t_pagina *pagina = malloc(sizeof(t_pagina));
	pagina->nro_pagina = tablaDePaginas->paginas->elements_count;
	pagina->bit_presencia = 1;
	pagina->nro_frame = nroFrame;
	pagina->uso = time(NULL);
	pagina->bitUso = 1; //TODO Donde se modifican estos dos bits
	pagina->bitMod = 0; //TODO Donde se modifican estos dos bits
	puts("3.4.1");
	list_add(tablaDePaginas->paginas,pagina);
	puts("3.4.2");
	//pagina_en_tlb(pagina->nro_pagina,pagina->nro_frame,pid);
	puts("3.4.3");
}

t_frame *encontrar_frame_vacio(){
	bool _frame_vacio(void *frame){
		printf("FRAME ID: %d - VACIO: %d \n", ((t_frame *)frame)->nroFrame ,((t_frame *)frame)->estaLibre);
		return (((t_frame *)frame)->estaLibre == 1);
	}

	//t_list *framesVacios=list_filter(lista_de_frames, _frame_vacio);
	t_list* framesVacios = list_create();
	for(int i = 0; i< list_size(lista_de_frames); i++){
		t_frame* aux = list_get(lista_de_frames,i);
		if(aux->estaLibre == 1){
			//printf("FRAME ID: %d - VACIO: %d \n", aux->nroFrame , aux->estaLibre);
			//printf("I: %d \n", i);
			list_add(framesVacios, aux);
		}
	}

	if(framesVacios==NULL){
		puts("F0");
		//TODO preguntar al swap si hay espacio disponible, si hay hay que enviar una pagina y devolver ese frame
	}
	t_frame *frame = malloc(sizeof(t_frame));
	printf("CANTIDAD FRAMES: %d \n", list_size(lista_de_frames));
	printf("CANTIDAD FRAMES VACIOS: %d \n", list_size(framesVacios));
	puts("F1");
	frame =	list_get(framesVacios,0);
	printf("FRAME ASIGNADO: %d \n", frame->nroFrame);
	frame->estaLibre=0;
	return frame;
}

void* get_pagina_data(uint32_t nro_frame){
	void* data = malloc(cfg->TAMANIO_PAGINA);
	memcpy(data, inicio_memoria+nro_frame*cfg->TAMANIO_PAGINA, cfg->TAMANIO_PAGINA);
	return data;
}

void memcpy_pagina_en_frame(uint32_t nro_frame, uint32_t desplazamiento, void* data, uint32_t size){
	puts("3.7.0");
	printf("SIZE HEAPDATA %d \n", size);
	puts("3.7.1");
	memcpy(inicio_memoria + nro_frame*cfg->TAMANIO_PAGINA + desplazamiento, data, size);
	puts("3.7.2");
}

void clear_frame(uint32_t nro_frame){
	memset(inicio_memoria+nro_frame*cfg->TAMANIO_PAGINA,0,cfg->TAMANIO_PAGINA);
}

uint32_t cant_frames_libres(){
	bool _frame_vacio(void *frame){
		return ((t_frame *)frame)->estaLibre;
	}

	t_list *framesVacios=list_filter(lista_de_frames, _frame_vacio);
	//TODO preguntarlea swap cuantos frames libres tiene
	return framesVacios->elements_count;
}

bool entra_en_mp(uint32_t tamanio){
	int cantPaginasNecesarias = (int)(ceil(tamanio/cfg->TAMANIO_PAGINA)+1);
	if(cantPaginasNecesarias<=cant_frames_libres()){
		return 1;
	}
	return -1;
}

void pagina_en_tlb(uint32_t nroPagina, uint32_t nroFrame, uint32_t pid){
	bool _tlb_tiene_pagina(void* x){
		t_pagina_de_tlb* elem = (t_pagina_de_tlb*) x;
		return elem->pid==pid && elem->nroPagina == nroPagina;
	}

	t_pagina_de_tlb* paginaDeTlb = malloc(sizeof(t_pagina_de_tlb));
	t_tabla_de_paginas* tabla = malloc(sizeof(t_tabla_de_paginas));

	puts("3.4.2.1");
	tabla=list_find_by_pid(pid);
	puts("3.4.2.2");

	printf("TLB ELEMENTS COUNT: %d \n", tlb->elements_count);
	printf("ENTRADAS TLB: %d \n", cfg->CANTIDAD_ENTRADAS_TLB);

	if(tlb->elements_count <= cfg->CANTIDAD_ENTRADAS_TLB){
		puts("3.4.2.4");
		paginaDeTlb = list_find(tlb, _tlb_tiene_pagina); //todo no encuentra
		puts("3.4.2.5");
		if(paginaDeTlb != NULL){//si encuentra la pagina le actualiza el uso
			tlb_hit_totales+=1;
			tabla->tlbHit+=1;
			paginaDeTlb->uso=time(NULL);
		}else{//si no encuentra la pagina la agrega
			puts("3.4.2.6");
			tlb_miss_totales+=1;
			tabla->tlbMiss+=1;
			puts("3.4.2.6.1");
			/*paginaDeTlb->pid = pid;
			puts("3.4.2.6.2");
			paginaDeTlb->nroPagina = nroPagina;
			puts("3.4.2.7");
			paginaDeTlb->nroFrame = nroFrame;
			paginaDeTlb->uso=time(NULL);*/
			puts("3.4.2.8");
			list_add(tlb, paginaDeTlb);
			puts("3.4.2.9");
		}
	}else{//la tlb esta llena asi que hay que reemplazar
		//FIFO
		printf("TLB ELEMENTS COUNT: %d \n", tlb->elements_count);
		printf("TLB SIZE: %d \n", list_size(tlb));
		puts("ALGORITMOS \n");
		if(string_equals_ignore_case(cfg->ALGORITMO_REEMPLAZO_TLB, "FIFO")){
			if(list_size(tlb) > 0){
				puts("3.4.2.2A");
				paginaDeTlb = list_find(tlb, _tlb_tiene_pagina);
				puts("3.4.2.2B");
			}else{
				puts("TLB VACIA");
			}
			if(paginaDeTlb!=NULL){//si encuentra la pagina le actualiza el uso
				tlb_hit_totales+=1;
				tabla->tlbHit+=1;
				paginaDeTlb->uso=time(NULL);
			}else{//si no encuentra la pagina la agrega
				tlb_miss_totales+=1;
				tabla->tlbMiss+=1;
				paginaDeTlb->nroPagina=nroPagina;
				paginaDeTlb->nroFrame = nroFrame;
				paginaDeTlb->uso=time(NULL);
				list_remove(tlb,0);//saco la primera pagina de la lista
				list_add(tlb, paginaDeTlb);//agrego la pagina a la lista
			}
		}
		//LRU
		else{//ALGORITMO_REEMPLAZO_TLB=="LRU"
			paginaDeTlb = list_find(tlb, _tlb_tiene_pagina);
			if(paginaDeTlb!=NULL){//si encuentra la pagina le actualiza el uso
				tlb_hit_totales+=1;
				tabla->tlbHit+=1;
				paginaDeTlb->uso=time(NULL);
			}else{//si no encuentra la pagina la agrega
				tlb_miss_totales+=1;
				tabla->tlbMiss+=1;
				t_pagina_de_tlb* masVieja=malloc(sizeof(t_pagina_de_tlb));
				masVieja = list_get(tlb,0);
				int masViejaIndex = 0;
				for(int i=0; i<tlb->elements_count;i++){
					paginaDeTlb = list_get(tlb,i);
					if(masVieja->uso<=paginaDeTlb->uso){
						masVieja=paginaDeTlb;
						masViejaIndex=i;
					}
				}
				list_remove(tlb,masViejaIndex);//saco la primera pagina mas vieja lista
				paginaDeTlb->nroPagina=nroPagina;
				paginaDeTlb->nroFrame = nroFrame;
				paginaDeTlb->uso=time(NULL);
				list_add(tlb, paginaDeTlb);//agrego la pagina a la lista
			}
		}
	}
}

void dump_tlb_sigusr1(){
	printf("\n--------------------------------------------------------------------------------\n");
	dump_fecha();
	t_pagina_de_tlb* paginaDeTlb = malloc(sizeof(t_pagina));
	for(int i=0; i<cfg->CANTIDAD_ENTRADAS_TLB;i++){
		paginaDeTlb=list_get(tlb,i);
		if(paginaDeTlb!=NULL){
			printf("Entrada: %d		Estado:%2s		Carpincho: %d		Pagina: %d		Marco:%d\n",i,"Ocupado",paginaDeTlb->pid,paginaDeTlb->nroPagina,paginaDeTlb->nroFrame);
		}else{
			printf("Entrada: %d		Estado:%2s		Carpincho: -		Pagina: -		Marco: -\n",i,"Libre");
		}
	}
	printf("\n--------------------------------------------------------------------------------\n");
}

void dump_tlb_sigint(){
	printf("\n--------------------------------------------------------------------------------\n");
	dump_fecha();
	printf("TLB HIT TOTALES: %d\n", tlb_hit_totales);
	printf("TLB MISS TOTALES: %d\n", tlb_miss_totales);
	t_tabla_de_paginas* tabla = malloc(sizeof(t_tabla_de_paginas));
	for(int i=0; i<lista_de_tablas_de_paginas->elements_count;i++){
		tabla=list_get(lista_de_tablas_de_paginas,i);
		printf("Carpincho: %d		TLB HIT: %d		TLB MISS: %d",tabla->pid,tabla->tlbHit,tabla->tlbMiss);
	}
	printf("\n--------------------------------------------------------------------------------\n");
}

void dump_fecha(){
	time_t now;
	now=time(NULL);
	struct tm *local = localtime(&now);
	printf("Dump: %02d/%02d/%d %02d:%02d:%02d\n",local->tm_mday,local->tm_mon+1,local->tm_year+1900,local->tm_hour,local->tm_min,local->tm_sec);
}

void dump_tlb_sigusr2(){
	printf("\n--------------------------------------------------------------------------------\n");
	dump_fecha();
	list_clean(tlb);
	printf("TLB vaciada");
	printf("\n--------------------------------------------------------------------------------\n");
}
//MANEJO DE TABLAS
void crear_lista_de_frames(void *inicio_memoria){
	lista_de_frames = list_create();
	//t_frame *frameVacio = malloc(sizeof(t_frame));

	int cant_frames = (cfg->TAMANIO/cfg->TAMANIO_PAGINA);
	printf("CANTIDAD FRAMES: %d \n", cant_frames);
	puts("A");

	for(int i=0; i< cant_frames; i++){
		t_frame *frameVacio = malloc(sizeof(t_frame));
		frameVacio->nroFrame=i;
		frameVacio->estaLibre=1;
		list_add(lista_de_frames, frameVacio);
	}
}

void list_add_tabla_de_paginas(t_tabla_de_paginas* elem){
	pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
	list_add(lista_de_tablas_de_paginas, (void*) elem);
	pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
}

bool tabla_de_paginas_tiene_pid(void* x){
	t_tabla_de_paginas* elem = x;
	return elem->pid == static_pid;
}

t_tabla_de_paginas* list_find_by_pid(uint32_t pid){
	pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
	static_pid = pid;
	t_tabla_de_paginas* elem = list_find(lista_de_tablas_de_paginas, &tabla_de_paginas_tiene_pid);
	pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
	puts("0");
	if(elem != NULL){
		puts("0.1");
		if(elem->paginas != NULL){
			puts("NO ES NULL");
			printf("PID: %d \n", elem->pid);
			printf("TLB HIT: %d \n", elem->tlbHit);
			printf("TLB MISS: %d \n", elem->tlbMiss); //TODO este valor está mal
			puts("SIZE ELEM");
			printf("SIZE ELEM: %d \n", list_size(elem->paginas));
		}
	}
	puts("1");
	return elem;
}

t_tabla_de_paginas* list_remove_by_pid(uint32_t pid){
	pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
	static_pid = pid;
	t_tabla_de_paginas* elem = list_remove_by_condition(lista_de_tablas_de_paginas, &tabla_de_paginas_tiene_pid);
	pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
	return elem;
}

void free_tabla_de_paginas(void* x){
	if(x == NULL)
		return;
	t_tabla_de_paginas* elem = (t_tabla_de_paginas*) x;
	list_destroy_and_destroy_elements(elem->paginas, (void*) free);
	free(elem);
}

void finalizar_lista_de_tablas_de_paginas(){
	pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
	list_destroy_and_destroy_elements(lista_de_tablas_de_paginas, &free_tabla_de_paginas);
	pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
}

void finalizar_lista_de_frames(){
	pthread_mutex_lock(&MUTEX_FRAME);
	list_destroy_and_destroy_elements(lista_de_frames, free);
	pthread_mutex_unlock(&MUTEX_FRAME);
}

void finalizar_tlb(){
	pthread_mutex_lock(&MUTEX_TLB);
	list_destroy(tlb);
	pthread_mutex_unlock(&MUTEX_TLB);
}

t_pagina* pagina_a_remover(t_list* paginas_en_memoria){
    void* lru(t_pagina* t1, t_pagina* t2){
        return t1->uso < t2->uso ? t1 : t2;
    }

	bool buscar_doble_cero(void* t1) {
		return ((t_pagina*)t1)->bitUso == 0 && ((t_pagina*)t1)->bitMod == 0;
	}

	bool buscar_cero_uno(void* t1) {
		if(((t_pagina*)t1)->bitUso == 1){
			((t_pagina*)t1)->bitUso = 0;
		}
		return ((t_pagina*)t1)->bitUso == 0 && ((t_pagina*)t1)->bitMod == 1;
	}

	if(list_size(paginas_en_memoria) > 0){
		if((strcmp(cfg->ALGORITMO_REEMPLAZO_MMU,"LRU") == 0)){
			t_pagina* objetivo = list_get_minimum(paginas_en_memoria, (void*)lru);
			return objetivo;

		}else if((strcmp(cfg->ALGORITMO_REEMPLAZO_MMU,"CLOCK_MODIFICADO") == 0)){
			if(list_any_satisfy(paginas_en_memoria, buscar_doble_cero)){
				t_pagina* objetivo = list_find(paginas_en_memoria, buscar_doble_cero);
				return objetivo;
			}else if(list_any_satisfy(paginas_en_memoria, buscar_cero_uno)){
				t_pagina* objetivo = list_find(paginas_en_memoria, buscar_cero_uno);
				return objetivo;
			}else if(list_any_satisfy(paginas_en_memoria, buscar_cero_uno)){
				t_pagina* objetivo = list_find(paginas_en_memoria, buscar_cero_uno);
				return objetivo;
			}else{
				return list_get(paginas_en_memoria,0);
			}
		}else{
			return NULL;
		}
	}else{
		return NULL;
	}
}
