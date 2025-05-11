/**
 * @file sn_assert.h
 * @brief assert
 * @version 1.0
 * @date 2019-02-21
 * 
 * @copyright Copyright (c) 2019   
 * 
 */
#ifndef _SN_ASSERT_H_
#define _SN_ASSERT_H_

#include "sn_log.h"
#include "sn_debug_config.h"

#if CONFIG_DEBUG_EN
#define SN_ASSERT(x,msg)                                                                                                    \
    if( ( x ) == 0 )                                                                                                        \
    {                                                                                                                       \
        sn_logerr(LOG_ASSERT,"%s",msg);                                                                                     \
        sn_logassert(LOG_ASSERT,"ASSERT: %d in %s\n",__LINE__,__FUNCTION__);                                                \
        while(1)  asm volatile ("ebreak");                                                                                  \
    }
#else
#define SN_ASSERT(x,msg) do{}while(0)
#endif

#endif /* _SN_ASSERT_H_ */
