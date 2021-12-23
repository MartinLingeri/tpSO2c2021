#include "logs.h"

void logear(int caso, int dato1, int dato2, char* dato3){
	switch(caso){
		case CAMBIO_ESTADO:
			log_info(logger,"CARPINCHO %d - Cambia de estado a %s", dato1, estado_a_string(dato2));
			break;

		case INICIO_PROCESO:
			log_info(logger,"CARPINCHO %d - Proceso iniciado", dato1);
			break;

		case DEADLOCK_ELIMINA:
			log_info(logger,"CARPINCHO %d - Eliminado por deadlock", dato1);
			break;

		case DL_DETECTADO:
			log_info(logger,"Deadlock detectado entre carpinchos %d y %d", dato1, dato2);
			break;

		case IO_EN_USO:
			log_info(logger,"CARPINCHO %d - Utilizando dispositivo %s",dato1, dato3);
			break;

		case IO_LIBERADO:
			log_info(logger,"CARPINCHO %d - Libera dispositivo %s", dato1, dato3);
			break;

		case SEM_NOEXISTE:
			log_info(logger,"CARPINCHO %d - Operación sobre semáforo %s inexistente", dato1, dato3);
			break;

		case IO_NOEXISTE:
			log_info(logger,"CARPINCHO %d - Operación sobre dispositivo IO %s inexistente", dato1, dato3);
			break;

		case SEM_POSTED:
			log_info(logger,"CARPINCHO %d - POST de semáforo %s", dato1, dato3);
			break;

		case SEM_WAITED:
			log_info(logger,"CARPINCHO %d - WAIT de semáforo %s", dato1, dato3);
			break;

		case SEM_CREATED:
			log_info(logger,"CARPINCHO %d - Crea semáforo %s", dato1, dato3);
			break;

		case SEM_DELETED:
			log_info(logger,"CARPINCHO %d - Elimina semáforo %s", dato1, dato3);
			break;

		case INICIADO:
			log_info(logger,"CARPINCHO %d - Iniciado correctamente", dato1, dato3);
			break;

		case FINALIZADO:
			log_info(logger,"CARPINCHO %d - Finalizado. Hasta pronto!", dato1);
			break;

		case LMEMALLOC:
			log_info(logger,"CARPINCHO %d - Realiza MEMALLOC", dato1);
			break;

		case LOPMEM:
			log_info(logger,"CARPINCHO %d - Realiza operación de memoria", dato1);
			break;

		case MEM_ENVIO:
			log_info(logger,"CARPINCHO %d - Se comunica con módulo Memoria", dato1);
			break;

		default:
			break;
	}
	return;
}

char* estado_a_string(int dato){
	switch(dato){
	    case NEW: return "NUEVO";
	    case READY: return "READY";
	    case EXEC:return "EXEC";
	    case BLOCKED: return "BLOQUEADO";
	    case SUSPENDED_READY: return "LISTO_SUSPENDIDO";
	    case SUSPENDED_BLOCKED: return "BLOQUEADO_SUSPENDIDO";
	    case EXIT: return "EXIT";
	    default: return "???";
	}
}

