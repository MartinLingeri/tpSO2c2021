/*
 * logs.c
 *
 *  Created on: 15 dic. 2021
 *      Author: utnso
 */

#ifndef SRC_LOGS_H_
#define SRC_LOGS_H_

#include "logs.h"

void logear(int caso, int dato1, int dato2, int dato3){
	switch(caso){
			case CARPINCHO_ELIMINADO:
				log_info(logger,"CARPINCHO %d - Proceso terminado", dato1);
				break;

			case READ_SWAP:
				log_info(logger,"Pagina %d pedida a SWAP", dato1);
				break;

			case WRITE_SWAP:
				log_info(logger,"Pagina %d escrita a SWAP", dato1);
				break;

			case PEDIDO_ALLOC:
				log_info(logger,"CARPINCHO %d - Hace alloc de memoria de tamaño %d", dato1, dato2);
				break;

			case PEDIDO_FREE:
				log_info(logger,"CARPINCHO %d - Libera memoria en la dirección %d",dato1, dato2);
				break;

			case PEDIDO_WRITE:
				log_info(logger,"CARPINCHO %d - Escribe en la dirección %d", dato1, dato2);
				break;

			case PEDIDO_READ:
				log_info(logger,"CARPINCHO %d - Lee de la dirección %d", dato1, dato2);
				break;

			case NUEVO_PROCESO:
				log_info(logger,"NUEVO PROCESO - INICIADO CARPINCHO %d", dato1);
				break;

			case SUSPENDED:
				log_info(logger,"CARPINCHO %d - Pasa a suspendido", dato1);
				break;


			case TLB_HIT:
				log_info(logger,"TLB HIT! Proceso: %d - Pagina: %d - Marco: %d", dato1, dato2, dato3);
				break;

			case TLB_MISS:
				log_info(logger,"TLB MISS! Proceso: %d - Pagina: %d", dato1, dato2);
				break;

			case REEMPLAZO_ENTRA:
				log_info(logger,"NUEVA ENTRADA EN TLB: Proceso: %d - Pagina: %d - Marco: %d", dato1, dato2, dato3);
				break;

			case REEMPLAZO_SALE:
				log_info(logger,"SALE DE TLB: Proceso: %d - Pagina: %d - Marco: %d", dato1, dato2, dato3);
				break;

			case ENVIO_A_SWAP:
				log_info(logger,"SELECCIONADA PARA ENVIAR A SWAP: Proceso: %d - Pagina: %d - Marco: %d", dato1, dato2, dato3);
				break;

			case TRAIGO_DE_SWAP:
				log_info(logger,"NUEVA ENTRADA EN TP: Proceso: %d - Pagina: %d - Marco: %d", dato1, dato2, dato3);
				break;

			default:
				break;
		}
		return;
}

#endif /* SRC_LOGS_H_ */
