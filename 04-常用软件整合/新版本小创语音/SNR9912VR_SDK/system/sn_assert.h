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

#include <stdio.h>
#include "sn_log.h"

#ifndef mprintf
#if USE_STD_PRINTF
#define mprintf(fmt, args...) printf(fmt, ## args)
#else
#define mprintf(fmt, args...) _printf(fmt, ## args)
#endif
#endif

#define SN_ASSERT(x,msg)                                                                                                    \
    if( ( x ) == 0 )                                                                                                        \
    {                                                                                                                       \
        mprintf("%s",msg);                                                                                                   \
        mprintf("ASSERT: %d in %s\n",__LINE__,__FUNCTION__);                                                                \
        while(1)  asm volatile ("ebreak");                                                                                  \
    }

#endif /* _SN_ASSERT_H_ */
