/*
 * dumps.c
 *
 *  Created on: 16 dic. 2021
 *      Author: utnso
 */

#include "dumps.h"

void dump_tlb_sigusr1(){
	char* path = string_new();
	char* tiempo = temporal_get_string_time("%H:%M:%S");
	string_append(&path,cfg->PATH_DUMP_TLB);
	string_append(&path,"Dump_");
	string_append(&path,tiempo);
	string_append(&path,".dmp");

	FILE* dump = fopen(path,"w");
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");

	time_t now;
	now=time(NULL);
	struct tm *local = localtime(&now);
	fprintf(dump,"Dump: %02d/%02d/%d %02d:%02d:%02d\n",local->tm_mday,local->tm_mon+1,local->tm_year+1900,local->tm_hour,local->tm_min,local->tm_sec);

	t_pagina_de_tlb* paginaDeTlb = malloc(sizeof(t_pagina));
	for(int i=0; i<cfg->CANTIDAD_ENTRADAS_TLB;i++){
		paginaDeTlb=list_get(tlb,i);
		if(paginaDeTlb!=NULL){
			fprintf(dump,"Entrada: %d		Estado:%2s		Carpincho: %d		Pagina: %d		Marco:%d\n",i,"Ocupado",paginaDeTlb->pid,paginaDeTlb->nroPagina,paginaDeTlb->nroFrame);
		}else{
			fprintf(dump,"Entrada: %d		Estado:%2s		Carpincho: -		Pagina: -		Marco: -\n",i,"Libre");
		}
	}
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");
}

void dump_tlb_sigint(){
	char* path = string_new();
	char* tiempo = temporal_get_string_time("%H:%M:%S");
	string_append(&path,cfg->PATH_DUMP_TLB);
	string_append(&path,"Dump_");
	string_append(&path,tiempo);
	string_append(&path,".dmp");

	FILE* dump = fopen(path,"w");
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");
	time_t now;
	now=time(NULL);
	struct tm *local = localtime(&now);
	fprintf(dump,"Dump: %02d/%02d/%d %02d:%02d:%02d\n",local->tm_mday,local->tm_mon+1,local->tm_year+1900,local->tm_hour,local->tm_min,local->tm_sec);

	fprintf(dump,"TLB HIT TOTALES: %d\n", tlb_hit_totales);
	fprintf(dump,"TLB MISS TOTALES: %d\n", tlb_miss_totales);
	t_tabla_de_paginas* tabla = malloc(sizeof(t_tabla_de_paginas));
	for(int i=0; i<lista_de_tablas_de_paginas->elements_count;i++){
		tabla=list_get(lista_de_tablas_de_paginas,i);
		fprintf(dump,"Carpincho: %d		TLB HIT: %d		TLB MISS: %d",tabla->pid,tabla->tlbHit,tabla->tlbMiss);
	}
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");
}

void dump_tlb_sigusr2(){
	char* path = string_new();
	char* tiempo = temporal_get_string_time("%H:%M:%S");
	string_append(&path,cfg->PATH_DUMP_TLB);
	string_append(&path,"Dump_");
	string_append(&path,tiempo);
	string_append(&path,".dmp");

	FILE* dump = fopen(path,"w");
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");

	time_t now;
	now=time(NULL);
	struct tm *local = localtime(&now);
	fprintf(dump,"Dump: %02d/%02d/%d %02d:%02d:%02d\n",local->tm_mday,local->tm_mon+1,local->tm_year+1900,local->tm_hour,local->tm_min,local->tm_sec);

	list_clean(tlb);
	fprintf(dump,"TLB vaciada");
	fprintf(dump,"\n--------------------------------------------------------------------------------\n");
}
