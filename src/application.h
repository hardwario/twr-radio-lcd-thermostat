#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef ROTATE_SUPPORT
#define ROTATE_SUPPORT 0
#endif

#ifndef CORE_R
#define CORE_R 2
#endif

#include <bcl.h>
#include <twr.h>

typedef struct
{
    uint8_t number;
    float value;
    twr_tick_t next_pub;

} event_param_t;

#endif
