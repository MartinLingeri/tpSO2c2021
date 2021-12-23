/*
 * dumps.h
 *
 *  Created on: 16 dic. 2021
 *      Author: utnso
 */

#ifndef DUMPS_H_
#define DUMPS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <commons/string.h>
#include <math.h>
#include <time.h>

#include "init_memoria.h"
#include "client.h"
#include "logs.h"
#include "comunicacion.h"
#include "paginacion.h"

void dump_tlb_sigusr1();
void dump_tlb_sigint();
void dump_tlb_sigusr2();

#endif /* DUMPS_H_ */
