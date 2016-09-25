#ifndef __NBA_TIMER_H__
#define __NBA_TIMER_H__

namespace GBA
{
    struct Timer
    {
        u16 count;
        u16 reload;
        int clock;
        int ticks;
        bool enable;
        bool countup;
        bool interrupt;
        bool overflow;
    };
}

#endif // __NBA_TIMER_H__
