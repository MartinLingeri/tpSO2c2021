/*
 * testing.h
 *
 *  Created on: 7 nov. 2021
 *      Author: utnso
 */
#ifndef SRC_TESTING_H_
#define SRC_TESTING_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/config.h>
#include<commons/string.h>
#include<conexiones.h>
#include<matelib.h>

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <commons/log.h>

void* carpincho4_func(void* config);
void* carpincho3_func(void* config);
void* carpincho2_func(void* config);
void* carpincho1_func(void* config);

void free_all();

void init_sems();

#endif /* SRC_TESTING_H_ */
