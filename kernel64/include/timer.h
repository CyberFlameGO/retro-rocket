#ifndef TIMER_H
#define TIMER_H

void sleep_one_tick();
void init_timer(u32 frequency);
void beep(u32 pitch);
void stopbeep();
void sleep(u64 secs);
void sleep_one_tick();
u64 get_ticks();

#endif

