#ifndef __ES7243_H
#define __ES7243_H

#include <stdint.h>
#include "snr991x_system.h"

#ifdef __cplusplus
 extern "C" {
#endif

void es7243_test(void *p_arg);
void es7243_test1(void);
void es7243_aec_hw_init(void);

#ifdef __cplusplus
}
#endif

#endif