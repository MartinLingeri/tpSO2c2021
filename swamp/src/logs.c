/*
 * logs.c
 *
 *  Created on: 12 dic. 2021
 *      Author: utnso
 */


#include "logs.h"

void logear(int caso, int dato1, int dato2, char* dato3){
	switch(caso){
		case CARPINCHO_FINALIZADO:
			log_info(logger,"CARPINCHO %d - Proceso terminado", dato1);
			break;

		case PAGINA_GUARDADA:
			log_info(logger,"Pagina %d del carpincho %d guardada en SWAP", dato1, dato2);
			break;

		case PAGINA_SOLICITADA:
			log_info(logger,"Pagina %d del carpincho %d solicitada por MEMORIA", dato1, dato2);
			break;

		case PAGINA_NULL:
			log_error(logger,"ERROR - Pagina %d no encontrada", dato1);
			break;

		case FRAME_NULL:
			log_error(logger,"ERROR - Frame %d para la página %d no encontrado", dato1);
			break;

		case OVERFLOW:
			log_error(logger,"ERROR - Se intentó guardar en SWAP un tamaño mayor a 1 página");
			break;

		case LIMITE_FRAMES:
			log_error(logger,"ERROR - Carpincho %d superó el límite de frames por proceso", dato1);
			break;

		case SIN_FRAMES:
			log_error(logger,"ERROR - No se encontraron frames para escribir datos de carpincho %d", dato1);
			break;

		default:
			break;
	}
	return;
}
