/*
 * Apple // emulator for *nix 
 *
 * This software package is subject to the GNU General Public License
 * version 2 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * THERE ARE NO WARRANTIES WHATSOEVER.
 *
 */

/*
 * 65c02 CPU Timing Support. Some source derived from AppleWin.
 *
 * Copyleft 2013 Aaron Culliney
 *
 */

#include "timing.h"
#include "misc.h"
#include "cpu.h"
#include "speaker.h"
#include "keys.h"
#include "mockingboard.h"

#define EXECUTION_PERIOD_NSECS 1000000  // AppleWin: nExecutionPeriodUsec

double g_fCurrentCLK6502 = CLK_6502;
bool g_bFullSpeed = false; // HACK TODO FIXME : prolly shouldn't be global anymore -- don't think it's necessary for speaker/soundcore/etc anymore ...
uint64_t g_nCumulativeCycles = 0;       // cumulative cycles since emulator (re)start
int g_nCpuCyclesFeedback = 0;

static bool alt_speed_enabled = false;
double cpu_scale_factor = 1.0;
double cpu_altscale_factor = 1.0;

uint8_t emul_reinitialize;

static unsigned int g_nCyclesExecuted; // # of cycles executed up to last IO access

// -----------------------------------------------------------------------------

struct timespec timespec_diff(struct timespec start, struct timespec end, bool *negative) {
    struct timespec t;

    if (*negative)
    {
        *negative = false;
    }

    // if start > end, swizzle...
    if ( (start.tv_sec > end.tv_sec) || ((start.tv_sec == end.tv_sec) && (start.tv_nsec > end.tv_nsec)) )
    {
        t=start;
        start=end;
        end=t;
        if (negative)
        {
            *negative = true;
        }
    }

    // assuming time_t is signed ...
    if (end.tv_nsec < start.tv_nsec)
    {
        t.tv_sec  = end.tv_sec - start.tv_sec - 1;
        t.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        t.tv_sec  = end.tv_sec  - start.tv_sec;
        t.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return t;
}

static inline struct timespec timespec_add(struct timespec start, unsigned long nsecs) {

    start.tv_nsec += nsecs;
    if (start.tv_nsec > NANOSECONDS)
    {
        start.tv_sec += (start.tv_nsec / NANOSECONDS);
        start.tv_nsec %= NANOSECONDS;
    }

    return start;
}

static void _timing_initialize(double scale)
{
    if (g_bFullSpeed)
    {
        LOG("timing_initialize() emulation at fullspeed ...");
        return;
    }

    g_fCurrentCLK6502 = CLK_6502 * scale;
    // this is extracted out of SetClksPerSpkrSample -- speaker.c
    g_fClksPerSpkrSample = (double) (UINT) (g_fCurrentCLK6502 / (double)SPKR_SAMPLE_RATE);
    SpkrReinitialize();

    LOG("timing_initialize() ... ClockRate:%0.2lf  ClockCyclesPerSpeakerSample:%0.2lf", g_fCurrentCLK6502, g_fClksPerSpkrSample);
}

static void _switch_to_fullspeed(double scale)
{
    if (!g_bFullSpeed)
    {
        g_bFullSpeed = true;
        c_disable_sound_hooks();
    }
}

static void _switch_to_regular_speed(double scale)
{
    if (g_bFullSpeed)
    {
        g_bFullSpeed = false;
        c_initialize_sound_hooks();
    }
    _timing_initialize(scale);
}

void timing_toggle_cpu_speed()
{
    alt_speed_enabled = !alt_speed_enabled;

    if (alt_speed_enabled)
    {
        if (cpu_altscale_factor >= CPU_SCALE_FASTEST)
        {
            _switch_to_fullspeed(cpu_altscale_factor);
        }
        else
        {
            _switch_to_regular_speed(cpu_altscale_factor);
        }
    }
    else
    {
        if (cpu_scale_factor >= CPU_SCALE_FASTEST)
        {
            _switch_to_fullspeed(cpu_scale_factor);
        }
        else
        {
            _switch_to_regular_speed(cpu_scale_factor);
        }
    }
}

void timing_initialize()
{
    _timing_initialize(cpu_scale_factor);
}

void cpu_thread(void *dummyptr) {
    struct timespec deltat;
    struct timespec t0;         // the target timer
    struct timespec ti, tj;     // actual time samples
    bool negative = false;
    long drift_adj_nsecs = 0;         // generic drift adjustment between target and actual

    unsigned long dbg_ticks = 0;

    do
    {
        g_nCumulativeCycles = 0;
        static int16_t cycles_adjust = 0;
        LOG("cpu_thread : begin main loop ...");

        clock_gettime(CLOCK_MONOTONIC, &t0);

        emul_reinitialize = 1;
        do {
            // -LOCK----------------------------------------------------------------------------------------- SAMPLE ti
            pthread_mutex_lock(&interface_mutex);
            clock_gettime(CLOCK_MONOTONIC, &ti);

            deltat = timespec_diff(t0, ti, &negative);
            if (deltat.tv_sec)
            {
                // TODO FIXME : this is innocuous when coming out of interface menus, but are there any other edge cases?
                LOG("NOTE : serious divergence from target time ...");
                t0 = ti;
                deltat = timespec_diff(t0, ti, &negative);
            }
            t0 = timespec_add(t0, EXECUTION_PERIOD_NSECS); // expected interval
            drift_adj_nsecs = negative ? ~deltat.tv_nsec : deltat.tv_nsec;

            // set up increment & decrement counters
            cpu65_cycles_to_execute = (g_fCurrentCLK6502 / 1000); // g_fCurrentCLK6502 * EXECUTION_PERIOD_NSECS / NANOSECONDS
            cpu65_cycles_to_execute += g_nCpuCyclesFeedback;
            cpu65_cycles_to_execute -= cycles_adjust;
            if (cpu65_cycles_to_execute < 0)
            {
                cpu65_cycles_to_execute = 0;
            }

            cpu65_cycle_count = 0;
            g_nCyclesExecuted = 0;

            MB_StartOfCpuExecute();

            cpu65_run(); // run emulation for cpu65_cycles_to_execute cycles ...

            cycles_adjust = cpu65_cycles_to_execute; // counter is decremented in cpu65_run()
            if (cycles_adjust < 0)
            {
                cycles_adjust = ~cycles_adjust +1; // cycles_adjust *= -1
            }
            unsigned int uExecutedCycles = cpu65_cycle_count;

            MB_UpdateCycles(uExecutedCycles);   // Update 6522s (NB. Do this before updating g_nCumulativeCycles below)

            // N.B.: IO calls that depend on accurate timing will update g_nCyclesExecuted
            const unsigned int nRemainingCycles = uExecutedCycles - g_nCyclesExecuted;
            g_nCumulativeCycles += nRemainingCycles;

            if (!g_bFullSpeed)
            {
                SpkrUpdate(uExecutedCycles); // play audio
            }

            // N.B.: technically this is not the end of the video frame...
            MB_EndOfVideoFrame();

            clock_gettime(CLOCK_MONOTONIC, &tj);
            pthread_mutex_unlock(&interface_mutex);
            // -UNLOCK--------------------------------------------------------------------------------------- SAMPLE tj

            deltat = timespec_diff(ti, tj, &negative);
            assert(!negative);
            long sleepfor = 0;
            if (!deltat.tv_sec && !g_bFullSpeed)
            {
                sleepfor = EXECUTION_PERIOD_NSECS - drift_adj_nsecs - deltat.tv_nsec;
            }

            if (sleepfor <= 0)
            {
                // lagging ...
                static time_t throttle_warning = 0;
                if (t0.tv_sec - throttle_warning > 0)
                {
                    LOG("lagging... %ld . %ld", deltat.tv_sec, deltat.tv_nsec);
                    throttle_warning = t0.tv_sec;
                }
            }
            else
            {
                deltat.tv_sec = 0;
                deltat.tv_nsec = sleepfor;
                nanosleep(&deltat, NULL);
            }

#ifndef NDEBUG
            dbg_ticks += EXECUTION_PERIOD_NSECS;
            if ((dbg_ticks % NANOSECONDS) == 0)
            {
                dbg_ticks = 0;
                LOG("tick (%ld . %ld) real: (%ld . %ld)", t0.tv_sec, t0.tv_nsec, ti.tv_sec, ti.tv_nsec);
            }
#endif
        } while (!emul_reinitialize);

        reinitialize();
    } while (1);
}

// From AppleWin...
//      Called when an IO-reg is accessed & accurate cycle info is needed
void CpuCalcCycles(const unsigned long nExecutedCycles)
{
    // Calc # of cycles executed since this func was last called
    const long nCycles = nExecutedCycles - g_nCyclesExecuted;
    assert(nCycles >= 0);
    g_nCumulativeCycles += nCycles;

    g_nCyclesExecuted = nExecutedCycles;
}

