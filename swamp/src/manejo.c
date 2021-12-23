#include "main.h"

bool pagina_tiene_nro_en_mem(void* x) {
	t_pagina* elem = x;
	return elem->nro_pagina_en_mem == static_nro_pagina;
}

bool frame_tiene_nro(void* x) {
	t_frame* elem = x;
	return elem->numero_frame == static_frame->numero_frame;
}

bool frame_vacio(void* x) {
	t_frame* elem = x;
	return elem->pid == -1;
}

bool frame_del_proc(void* x) {
	t_frame* elem = x;
	return elem->pid == static_pid;
}

bool pagina_tiene_nro(void* x) {
	t_pagina* elem = x;
	return elem->nro_pagina == static_nro_pagina;
}

void pag_destroy(void* x) {
	t_pagina* elem = x;
	free(elem);
}

bool frame_vacio_para_pid(void* x) {
	t_frame* elem = x;
	return elem->pid == -1 && elem->path_archivo == static_path;
}

void* read_pagina(uint32_t pid, uint32_t nro_pagina_en_mem, void* swap) {
	static_nro_pagina = nro_pagina_en_mem;
	t_pagina* pagina = list_find(lista_paginas, &pagina_tiene_nro_en_mem);

	if(pagina != NULL) {
		static_frame = pagina->frame;
		t_frame* frame = list_find(lista_frames, &frame_tiene_nro);
		if(frame != NULL) {
			memcpy(swap, frame->inicio, t_pagina_size);
			printf("EL TEXTO: %s, %d", (char*) swap, strlen((char*) swap));
			puts("DESP TEXTO");
			memset(frame->inicio, '\n', t_pagina_size);
			frame->espacio_libre = t_pagina_size;
			frame->pid = -1;
			static_nro_pagina = pagina->nro_pagina;
			logear(PAGINA_SOLICITADA, nro_pagina_en_mem, pid, "");
			list_remove_and_destroy_by_condition(lista_paginas, pagina_tiene_nro, pag_destroy);
		} else {
			logear(FRAME_NULL, pagina->frame->numero_frame, nro_pagina_en_mem, "");
		}
	} else {
		logear(PAGINA_NULL, nro_pagina_en_mem, pid, "");
	}
	return swap;
}

int write_pagina(uint32_t pid, uint32_t nro_pagina_en_mem, void* data, uint32_t size) {
	if(size <= t_pagina_size) {
		static_pid = pid;
		t_list* frames_del_pid = list_filter(lista_frames, (void*)frame_del_proc);
		t_frame* frame;
		if(list_size(frames_del_pid) == 0) {
			//cualquier pagina xq no hay importa en q archivo dejarlo
			frame = list_find(lista_frames, &frame_vacio);
		} else {
			static_pid = ((t_frame*)list_get(frames_del_pid, 0))->path_archivo;
			frame = list_find(lista_frames, &frame_vacio_para_pid);
		}

		if(string_equals_ignore_case(tipo_asignacion,"FIJA")){
			if(list_size(frames_del_pid) <= marcos_por_carpincho) {
				if(frame != NULL) {
					t_pagina* pagina = malloc(sizeof(t_pagina));
					pagina->nro_pagina_en_mem = nro_pagina_en_mem;
					frame->pid = pid;
					frame->espacio_libre = frame->espacio_libre - size;
					memcpy(frame->inicio, data, t_pagina_size);
					puts("desp de memcopy");
					logear(PAGINA_GUARDADA, nro_pagina_en_mem, pid, "");
					return 1;
				} else {
					logear(SIN_FRAMES, pid, 1, "");
					return -12;//SWAP_FRAME_NULL;
				}
			} else {
				printf("FRAMES PID: %d \n", frames_del_pid);
				printf("FRAMES PID: %d \n", marcos_por_carpincho);
				printf("FRAMES PID: %s \n", tipo_asignacion);
				logear(LIMITE_FRAMES, pid, 1, "");
				return -15; //SWAP_LIMITE_FRAMES;
			}
		}else{
			if(frame != NULL) {
				t_pagina* pagina = malloc(sizeof(t_pagina));
				pagina->nro_pagina_en_mem = nro_pagina_en_mem;
				frame->pid = pid;
				frame->espacio_libre = frame->espacio_libre - size;
				memcpy(frame->inicio, data, t_pagina_size);
				puts("desp de memcopy");
				logear(PAGINA_GUARDADA, nro_pagina_en_mem, pid, "");
				return 1;
			} else {
				logear(SIN_FRAMES, pid, 1, "");
				return -12;//SWAP_FRAME_NULL;
			}
		}
	} else {
		logear(OVERFLOW, 1, 1, "");
		return -14;//SWAP_OVERFLOW;
	}
}

void finalizar_carpincho(uint32_t pid, uint32_t nro_pagina_en_mem){
	static_nro_pagina = nro_pagina_en_mem;
	t_pagina* pagina = list_find(lista_paginas, &pagina_tiene_nro_en_mem);

	if(pagina != NULL) {
		static_frame = pagina->frame;
		t_frame* frame = list_find(lista_frames, &frame_tiene_nro);
		if(frame != NULL) {
			frame->espacio_libre = t_pagina_size;
			frame->pid = -1;
			static_nro_pagina = pagina->nro_pagina;
			list_remove_and_destroy_by_condition(lista_paginas, pagina_tiene_nro, pag_destroy);
		} else {
			logear(FRAME_NULL, pagina->frame->numero_frame, nro_pagina_en_mem, "");
		}
	} else {
		logear(PAGINA_NULL, nro_pagina_en_mem, pid, "");
	}

	t_list* frames_del_pid = list_filter(lista_frames, (void*)frame_del_proc);
	if(list_size(frames_del_pid) == 0) {
		logear(CARPINCHO_FINALIZADO, pid, 1, "");
	}
}
