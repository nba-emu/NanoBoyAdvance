#ifndef __NBA_TIMER_H__
#define __NBA_TIMER_H__

namespace GBA
{
    struct Timer
    {
        u16 count  {0};
        u16 reload {0};
        int clock  {0};
        int ticks  {0};
        bool enable    {false};
        bool countup   {false};
        bool interrupt {false};
        bool overflow  {false};
    };
}

#endif // __NBA_TIMER_H__
