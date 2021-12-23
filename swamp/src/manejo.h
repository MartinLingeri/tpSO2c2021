#ifndef MANEJO_H_
#define MANEJO_H_

#include "global.h"
#include "logs.h"
#include <conexiones.h>

void* read_pagina(uint32_t pid, uint32_t nro_pagina_en_mem, void* swap);
int write_pagina(uint32_t pid, uint32_t nro_pagina_en_mem, void* data, uint32_t size);
void finalizar_carpincho(uint32_t pid, uint32_t);

#endif /* MANEJO_H_ */
