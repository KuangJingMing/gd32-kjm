#ifndef __REMOTE_API_FOR_BNPU_H__
#define __REMOTE_API_FOR_BNPU_H__
#if CORE_ID == 1

#include "snr991x_nuclear_com.h"

extern uint32_t RC_BNPU_CALLER(get_current_model_addr)(uint32_t *p_dnn_addr, uint32_t *p_dnn_size, uint32_t *p_asr_addr, uint32_t *p_addr_size);
extern uint32_t RC_BNPU_CALLER(malloc_in_host)(size_t size);
extern void RC_BNPU_CALLER(free_in_host)(void* pv);


#endif
#endif

