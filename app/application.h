#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#ifndef ROTATE_SUPPORT
#define ROTATE_SUPPORT 0
#endif

#ifndef CORE_R
#define CORE_R 1
#endif

#include <bcl.h>

typedef struct
{
    uint8_t number;
    float value;
    bc_tick_t next_pub;

} event_param_t;

#endif
