#ifndef __SYSTEM_HOOK_H__
#define __SYSTEM_HOOK_H__

#include "command_info.h"


void sys_power_on_hook();

void sys_weakup_hook();

void sys_sleep_hook();

void sys_asr_result_hook(cmd_handle_t cmd_handle, uint8_t asr_score);
void _key_deal_hook(uint8_t key_num,uint8_t key_event);
#if _TIMER_TMR0_INT == Enable
void _timer0_isr_handle(void);
#endif
#if _TIMER_TMR1_INT == Enable
void _timer1_isr_handle(void);
#endif
#if _TIMER_TMR2_INT == Enable
void _timer2_isr_handle(void);
#endif
#endif

