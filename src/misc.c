/*
 * Apple // emulator for Linux: Miscellaneous support
 *
 * Copyright 1994 Alexander Jean-Claude Bottema
 * Copyright 1995 Stephen Lee
 * Copyright 1997, 1998 Aaron Culliney
 * Copyright 1998, 1999, 2000 Michael Deutschmann
 *
 * This software package is subject to the GNU General Public License
 * version 2 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * THERE ARE NO WARRANTIES WHATSOEVER.
 *
 */

#include "common.h"

/* ----------------------------------
    internal apple2 variables
   ---------------------------------- */

static unsigned char apple_ii_rom[12288];
static unsigned char apple_iie_rom[32768];              /* //e */

bool do_logging = true; // also controlled by NDEBUG
FILE *error_log = NULL;

GLUE_BANK_READ(read_ram_bank,base_d000_rd)
GLUE_BANK_MAYBEWRITE(write_ram_bank,base_d000_wrt)
GLUE_BANK_READ(read_ram_lc,base_e000_rd)
GLUE_BANK_MAYBEWRITE(write_ram_lc,base_e000_wrt)


GLUE_BANK_READ(iie_read_ram_default,base_ramrd)
GLUE_BANK_WRITE(iie_write_ram_default,base_ramwrt)
GLUE_BANK_READ(iie_read_ram_text_page0,base_textrd)
GLUE_BANK_WRITE(iie_write_screen_hole_text_page0,base_textwrt)
GLUE_BANK_READ(iie_read_ram_hires_page0,base_hgrrd)
GLUE_BANK_WRITE(iie_write_screen_hole_hires_page0,base_hgrwrt)

GLUE_BANK_READ(iie_read_ram_zpage_and_stack,base_stackzp)
GLUE_BANK_WRITE(iie_write_ram_zpage_and_stack,base_stackzp)

GLUE_BANK_READ(iie_read_slot3,base_c3rom)
GLUE_BANK_MAYBEREAD(iie_read_slot4,base_c4rom)
GLUE_BANK_MAYBEREAD(iie_read_slot5,base_c5rom)
GLUE_BANK_READ(iie_read_slotx,base_cxrom)


uint32_t softswitches;

uint8_t *base_ramrd;
uint8_t *base_ramwrt;
uint8_t *base_textrd;
uint8_t *base_textwrt;
uint8_t *base_hgrrd;
uint8_t *base_hgrwrt;

uint8_t *base_stackzp;
uint8_t *base_d000_rd;
uint8_t *base_e000_rd;
uint8_t *base_d000_wrt;
uint8_t *base_e000_wrt;

uint8_t *base_c3rom;
void *base_c4rom;
void *base_c5rom;
uint8_t *base_cxrom;


/* -------------------------------------------------------------------------
    c_debug_illegal_bcd - illegal BCD (decimal mode) computation
   ------------------------------------------------------------------------- */
void c_debug_illegal_bcd()
{
    ERRLOG("Illegal/undefined BCD operation encountered, debug break on c_debug_illegal_bcd to debug...");
    char *ptr = (char*)0x0bad;
    *ptr = 0x1337; // segfault
}


/* -------------------------------------------------------------------------
    c_set_altchar() - set alternate character set
   ------------------------------------------------------------------------- */
void c_set_altchar()
{
    video_loadfont(0x40,0x20,mousetext_glyphs,1);
    video_loadfont(0x60,0x20,lcase_glyphs,2);

    video_redraw();
}

/* -------------------------------------------------------------------------
    c_set_primary_char() - set primary character set
   ------------------------------------------------------------------------- */
void c_set_primary_char()
{
    video_loadfont(0x40,0x40,ucase_glyphs,3);

    video_redraw();
}


/* -------------------------------------------------------------------------
    c_initialize_font():     Initialize ROM character table to primary char set
   ------------------------------------------------------------------------- */
void c_initialize_font()
{
    video_loadfont(0x00,0x40,ucase_glyphs,2);
    video_loadfont(0x40,0x40,ucase_glyphs,3);
    video_loadfont(0x80,0x40,ucase_glyphs,0);
    video_loadfont(0xC0,0x20,ucase_glyphs,0);
    video_loadfont(0xE0,0x20,lcase_glyphs,0);
    video_redraw();
}



/* -------------------------------------------------------------------------
    c_initialize_tables()
   ------------------------------------------------------------------------- */

void c_initialize_tables() {

    int i;

    /* reset everything */
    for (i = 0; i < 0x10000; i++)
    {
        cpu65_vmem[i].r = iie_read_ram_default;
        cpu65_vmem[i].w = iie_write_ram_default;
    }

    /* language card read/write area */
    for (i = 0xD000; i < 0xE000; i++)
    {
        {
            cpu65_vmem[i].w =
                write_ram_bank;
            cpu65_vmem[i].r =
                read_ram_bank;
        }
    }

    for (i = 0xE000; i < 0x10000; i++)
    {
        {
            cpu65_vmem[i].w =
                write_ram_lc;
            cpu65_vmem[i].r =
                read_ram_lc;
        }
    }

    /* done common initialization */

    /* initialize zero-page, //e specific */
    for (i = 0; i < 0x200; i++)
    {
        cpu65_vmem[i].r =
            iie_read_ram_zpage_and_stack;
        cpu65_vmem[i].w =
            iie_write_ram_zpage_and_stack;
    }

    /* initialize first text & hires page, which are specially bank switched
     *
     * video_set() substitutes it's own hooks for all visible write locations
     * affect the display, leaving our write-functions in place only at the
     * `screen holes', hence the name.
     */
    for (i = 0x400; i < 0x800; i++)
    {
        cpu65_vmem[i].r =
            iie_read_ram_text_page0;
        cpu65_vmem[i].w =
            iie_write_screen_hole_text_page0;
    }

    for (i = 0x2000; i < 0x4000; i++)
    {
        cpu65_vmem[i].r =
            iie_read_ram_hires_page0;
        cpu65_vmem[i].w =
            iie_write_screen_hole_hires_page0;
    }

    /* softswich rom */
    for (i = 0xC000; i < 0xC100; i++)
    {
        cpu65_vmem[i].r =
            read_unmapped_softswitch;
        cpu65_vmem[i].w =
            write_unmapped_softswitch;
    }

    /* slot rom defaults */
    for (i = 0xC100; i < 0xD000; i++)
    {
        cpu65_vmem[i].r =
            iie_read_ram_default;
        cpu65_vmem[i].w =
            ram_nop;
    }

    /* keyboard data and strobe (READ) */
    for (i = 0xC000; i < 0xC010; i++)
    {
        cpu65_vmem[i].r =
            read_keyboard;
    }

    for (i = 0xC010; i < 0xC020; i++)
    {
        cpu65_vmem[i].r =
            cpu65_vmem[i].w =
                read_keyboard_strobe;
    }

    /* RDBNK2 switch */
    cpu65_vmem[0xC011].r =
        iie_check_bank;

    /* RDLCRAM switch */
    cpu65_vmem[0xC012].r =
        iie_check_lcram;

    /* 80STORE switch */
    cpu65_vmem[0xC000].w = iie_80store_off;
    cpu65_vmem[0xC001].w = iie_80store_on;
    cpu65_vmem[0xC018].r = iie_check_80store;

    /* RAMRD switch */
    cpu65_vmem[0xC002].w = iie_ramrd_main;
    cpu65_vmem[0xC003].w = iie_ramrd_aux;
    cpu65_vmem[0xC013].r = iie_check_ramrd;

    /* RAMWRT switch */
    cpu65_vmem[0xC004].w = iie_ramwrt_main;
    cpu65_vmem[0xC005].w = iie_ramwrt_aux;
    cpu65_vmem[0xC014].r = iie_check_ramwrt;

    /* ALTZP switch */
    cpu65_vmem[0xC008].w = iie_altzp_main;
    cpu65_vmem[0xC009].w = iie_altzp_aux;
    cpu65_vmem[0xC016].r = iie_check_altzp;

    /* 80COL switch */
    cpu65_vmem[0xC00C].w = iie_80col_off;
    cpu65_vmem[0xC00D].w = iie_80col_on;
    cpu65_vmem[0xC01F].r = iie_check_80col;

    /* ALTCHAR switch */
    cpu65_vmem[0xC00E].w = iie_altchar_off;
    cpu65_vmem[0xC00F].w = iie_altchar_on;
    cpu65_vmem[0xC01E].r = iie_check_altchar;

    /* SLOTC3ROM switch */
    cpu65_vmem[0xC00A].w = iie_c3rom_internal;
    cpu65_vmem[0xC00B].w = iie_c3rom_peripheral;
    cpu65_vmem[0xC017].r = iie_check_c3rom;

    /* SLOTCXROM switch */
    cpu65_vmem[0xC006].w = iie_cxrom_peripheral;
    cpu65_vmem[0xC007].w = iie_cxrom_internal;
    cpu65_vmem[0xC015].r = iie_check_cxrom;

    /* RDVBLBAR switch */
    cpu65_vmem[0xC019].r =
        iie_check_vbl;

    /* random number generator */
    for (i = 0xC020; i < 0xC030; i++)
    {
        cpu65_vmem[i].r =
            cpu65_vmem[i].w =
                read_random;
    }

    /* TEXT switch */
    cpu65_vmem[0xC050].r =
        cpu65_vmem[0xC050].w =
            read_switch_graphics;
    cpu65_vmem[0xC051].r =
        cpu65_vmem[0xC051].w =
            read_switch_text;

    cpu65_vmem[0xC01A].r =
        iie_check_text;

    /* MIXED switch */
    cpu65_vmem[0xC052].r =
        cpu65_vmem[0xC052].w =
            read_switch_no_mixed;
    cpu65_vmem[0xC053].r =
        cpu65_vmem[0xC053].w =
            read_switch_mixed;

    cpu65_vmem[0xC01B].r =
        iie_check_mixed;

    /* PAGE2 switch */
    cpu65_vmem[0xC054].r =
        cpu65_vmem[0xC054].w =
            iie_page2_off;

    cpu65_vmem[0xC01C].r =
        iie_check_page2;

    /* PAGE2 switch */
    cpu65_vmem[0xC055].r =
        cpu65_vmem[0xC055].w =
            iie_page2_on;

    /* HIRES switch */
    cpu65_vmem[0xC01D].r =
        iie_check_hires;
    cpu65_vmem[0xC056].r =
        cpu65_vmem[0xC056].w =
            iie_hires_off;
    cpu65_vmem[0xC057].r =
        cpu65_vmem[0xC057].w =
            iie_hires_on;

    /* game I/O switches */
    cpu65_vmem[0xC061].r =
        cpu65_vmem[0xC069].r =
            read_button0;
    cpu65_vmem[0xC062].r =
        cpu65_vmem[0xC06A].r =
            read_button1;
    cpu65_vmem[0xC063].r =
        cpu65_vmem[0xC06B].r =
            read_button2;
    cpu65_vmem[0xC064].r =
        cpu65_vmem[0xC06C].r =
            read_gc0;
    cpu65_vmem[0xC065].r =
        cpu65_vmem[0xC06D].r =
            read_gc1;
    cpu65_vmem[0xC066].r =
        iie_read_gc2;
    cpu65_vmem[0xC067].r =
        iie_read_gc3;

    for (i = 0xC070; i < 0xC080; i++)
    {
        cpu65_vmem[i].r =
            cpu65_vmem[i].w =
                read_gc_strobe;
    }

    /* IOUDIS switch & read_gc_strobe */
    cpu65_vmem[0xC07E].w =
        iie_ioudis_on;
    cpu65_vmem[0xC07F].w =
        iie_ioudis_off;
    cpu65_vmem[0xC07E].r =
        iie_check_ioudis;
    cpu65_vmem[0xC07F].r =
        iie_check_dhires;

    /* DHIRES/Annunciator switches */
    cpu65_vmem[0xC05E].w =
        cpu65_vmem[0xC05E].r =
            iie_dhires_on;
    cpu65_vmem[0xC05F].w =
        cpu65_vmem[0xC05F].r =
            iie_dhires_off;

    /* language card softswitches */
    cpu65_vmem[0xC080].r = cpu65_vmem[0xC080].w =
                               cpu65_vmem[0xC084].r = cpu65_vmem[0xC084].w = iie_c080;
    cpu65_vmem[0xC081].r = cpu65_vmem[0xC081].w =
                               cpu65_vmem[0xC085].r = cpu65_vmem[0xC085].w = iie_c081;
    cpu65_vmem[0xC082].r = cpu65_vmem[0xC082].w =
                               cpu65_vmem[0xC086].r = cpu65_vmem[0xC086].w = lc_c082;
    cpu65_vmem[0xC083].r = cpu65_vmem[0xC083].w =
                               cpu65_vmem[0xC087].r = cpu65_vmem[0xC087].w = iie_c083;

    cpu65_vmem[0xC088].r = cpu65_vmem[0xC088].w =
                               cpu65_vmem[0xC08C].r = cpu65_vmem[0xC08C].w = iie_c088;
    cpu65_vmem[0xC089].r = cpu65_vmem[0xC089].w =
                               cpu65_vmem[0xC08D].r = cpu65_vmem[0xC08D].w = iie_c089;
    cpu65_vmem[0xC08A].r = cpu65_vmem[0xC08A].w =
                               cpu65_vmem[0xC08E].r = cpu65_vmem[0xC08E].w = lc_c08a;
    cpu65_vmem[0xC08B].r = cpu65_vmem[0xC08B].w =
                               cpu65_vmem[0xC08F].r = cpu65_vmem[0xC08F].w = iie_c08b;

    /* slot i/o area */
    for (i = 0xC100; i < 0xC300; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slotx;             /* slots 1 & 2 (x) */
    }

    for (i = 0xC300; i < 0xC400; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slot3;             /* slot 3 (80col) */
    }

    for (i = 0xC400; i < 0xC500; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slot4;             /* slot 4 - MB or Phasor */
    }

    for (i = 0xC500; i < 0xC600; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slot5;             /* slot 5 - MB #2 */
    }

    for (i = 0xC600; i < 0xC800; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slotx;             /* slots 6 - 7 (x) */
    }

    for (i = 0xC800; i < 0xD000; i++)
    {
        cpu65_vmem[i].r =
            iie_read_slot_expansion;    /* expansion rom */
    }

    cpu65_vmem[0xCFFF].r =
        cpu65_vmem[0xCFFF].w =
            iie_disable_slot_expansion;

    video_set(0);

    // Peripheral card slot initializations ...

    // HACK TODO FIXME : this needs to be tied to the UI/configuration system (once we have more/conflicting options)

#ifdef AUDIO_ENABLED
    mb_io_initialize(4, 5); /* Mockingboard(s) and/or Phasor in slots 4 & 5 */
#endif
    disk_io_initialize(6); /* Put a Disk ][ Controller in slot 6 */
}

/* -------------------------------------------------------------------------
    c_initialize_apple_ii_memory()
   ------------------------------------------------------------------------- */

void c_initialize_apple_ii_memory()
{
    FILE       *f;
    int i;
    static int ii_rom_loaded = 0;
    static int iie_rom_loaded = 0;

    for (i = 0; i < 0x10000; i++)
    {
        apple_ii_64k[0][i] = 0;
        apple_ii_64k[1][i] = 0;
    }

    for (i = 0; i < 8192; i++)
    {
        language_card[0][i] = language_card[1][i] = 0;
    }

    for (i = 0; i < 8192; i++)
    {
        language_banks[0][i] = language_banks[1][i] = 0;
    }

    if (!ii_rom_loaded)
    {
        snprintf(temp, TEMPSIZE, "%s/apple_II.rom", system_path);
        if ((f = fopen(temp, "r")) == NULL) {
            printf("OOPS!\n");
            printf("Cannot find file '%s'.\n",temp);
            //exit(0);
        } else {
            if (fread(apple_ii_rom, 0x3000, 1, f) != 0x3000)
            {
                // ERROR ...
            }
            fclose(f);
        }
        ii_rom_loaded = 1;
    }

    if (!iie_rom_loaded)
    {
        snprintf(temp, TEMPSIZE, "%s/apple_IIe.rom", system_path);
        if ((f = fopen(temp, "r")) == NULL)
        {
            printf("Cannot find file '%s'.\n",temp);
            exit(0);
        }

        if (fread(apple_iie_rom, 32768, 1, f) != 32768)
        {
            // ERROR ...
        }

        fclose(f);
        iie_rom_loaded = 1;
    }

    for (i = 0xD000; i < 0x10000; i++)
    {
        apple_ii_64k[0][i] = apple_ii_rom[i - 0xD000];
    }

    for (i = 0; i < 0x1000; i++)
    {
        language_banks[0][i] = apple_ii_rom[i];
    }

    for (i = 0; i < 0x2000; i++)
    {
        language_card[0][i] = apple_ii_rom[i + 0x1000];
    }

    /* load the rom from 0xC000, slot rom main, internal rom aux */
    for (i = 0xC000; i < 0x10000; i++)
    {
        apple_ii_64k[0][i] = apple_iie_rom[i - 0xC000];
        apple_ii_64k[1][i] = apple_iie_rom[i - 0x8000];
    }

    for (i = 0; i < 0x1000; i++)
    {
        language_banks[0][i] = apple_iie_rom[i + 0x1000];
        language_banks[1][i] = apple_iie_rom[i + 0x5000];
    }

    for (i = 0; i < 0x2000; i++)
    {
        language_card[0][i] = apple_iie_rom[i + 0x2000];
        language_card[1][i] = apple_iie_rom[i + 0x6000];
    }

    apple_ii_64k[0][0xC000] = 0x00;
    apple_ii_64k[1][0xC000] = 0x00;
}

/* -------------------------------------------------------------------------
    void c_initialize_sound_hooks()
   ------------------------------------------------------------------------- */

void c_initialize_sound_hooks()
{
#ifdef AUDIO_ENABLED
    SpkrSetVolume(sound_volume * (SPKR_DATA_INIT/10));
#endif
    for (int i = 0xC030; i < 0xC040; i++)
    {
        cpu65_vmem[i].r = cpu65_vmem[i].w =
#ifdef AUDIO_ENABLED
            (sound_volume > 0) ? read_speaker_toggle_pc :
#endif
            ram_nop;
    }
}

void c_disable_sound_hooks()
{
    for (int i = 0xC030; i < 0xC040; i++)
    {
        cpu65_vmem[i].r = ram_nop;
    }
}

/* -------------------------------------------------------------------------
    c_initialize_iie_switches
   ------------------------------------------------------------------------- */
void c_initialize_iie_switches() {

    base_stackzp = apple_ii_64k[0];
    base_d000_rd = apple_ii_64k[0];
    base_d000_wrt = language_banks[0] - 0xD000;
    base_e000_rd = apple_ii_64k[0];
    base_e000_wrt = language_card[0] - 0xE000;

    base_ramrd = apple_ii_64k[0];
    base_ramwrt = apple_ii_64k[0];
    base_textrd = apple_ii_64k[0];
    base_textwrt = apple_ii_64k[0];
    base_hgrrd = apple_ii_64k[0];
    base_hgrwrt= apple_ii_64k[0];

    base_c3rom = apple_ii_64k[1];               /* c3rom internal */
    base_c4rom = apple_ii_64k[1];               /* c4rom internal */
    base_c5rom = apple_ii_64k[1];               /* c5rom internal */
    c8rom_offset = 0x10000;                     /* c8rom internal */
    base_cxrom = apple_ii_64k[0];               /* cxrom peripheral */
}

/* -------------------------------------------------------------------------
    void c_initialize_vm()
   ------------------------------------------------------------------------- */
void c_initialize_vm() {
    c_initialize_font();                /* font already read in */
    c_initialize_apple_ii_memory();     /* read in rom memory */
    c_initialize_tables();              /* read/write memory jump tables */
    c_initialize_sound_hooks();         /* sound system */
    c_init_6();                         /* drive ][, slot 6 */
    c_initialize_iie_switches();        /* set the //e softswitches */
}

/* -------------------------------------------------------------------------
    void c_initialize_firsttime()
   ------------------------------------------------------------------------- */

void reinitialize(void)
{
    int i;

    c_initialize_vm();

    softswitches = SS_TEXT | SS_IOUDIS | SS_C3ROM | SS_LCWRT | SS_LCSEC;

    video_setpage( 0 );

    video_redraw();

    cpu65_set(CPU65_C02);

    timing_initialize();

#ifdef AUDIO_ENABLED
    MB_Reset();
#endif 
}

void c_initialize_firsttime()
{
#ifdef INTERFACE_CLASSIC
    /* read in system files and calculate system defaults */
    c_load_interface_font();
#endif

    /* initialize the video system */
    video_init();

    // TODO FIXME : sound system never released ...
#ifdef AUDIO_ENABLED
    DSInit();
    SpkrInitialize();
    MB_Initialize();
#endif

#ifdef DEBUGGER
    c_debugger_init();
#endif

    reinitialize();
}

// Read Game Controller (paddle) strobe ...
// From _Understanding the Apple IIe_ :
//  * 7-29, discussing PREAD : "The timer duration will vary between 2 and 3302 usecs"
//  * 7-30, timer reset : "But the timer pulse may still be high from the previous [strobe access] and the timers are
//  not retriggered by C07X' if they have not yet reset from the previous trigger"
#define JOY_STEP_USEC (3300.0 / 256.0)
#define CYCLES_PER_USEC (CLK_6502 / 1000000)
#define JOY_STEP_CYCLES (JOY_STEP_USEC / CYCLES_PER_USEC)
extern short joy_x;
extern short joy_y;
GLUE_C_READ(read_gc_strobe)
{
    if (gc_cycles_timer_0 <= 0)
    {
        gc_cycles_timer_0 = (int)(joy_x * JOY_STEP_CYCLES) + 2;
    }
    if (gc_cycles_timer_1 <= 0)
    {
        gc_cycles_timer_1 = (int)(joy_y * JOY_STEP_CYCLES) + 2;
    }
    // NOTE: unimplemented GC2 and GC3 timers since they were not wired on the //e ...
    return 0;
}

GLUE_C_READ(read_gc0)
{
    if (gc_cycles_timer_0 <= 0)
    {
        gc_cycles_timer_0 = 0;
        return 0;
    }
    return 0xFF;
}

GLUE_C_READ(read_gc1)
{
    if (gc_cycles_timer_1 <= 0)
    {
        gc_cycles_timer_1 = 0;
        return 0;
    }
    return 0xFF;
}

// HACK FIXME TODO : candidate for GLUE_C_READ(...)
void c_read_random() {
    static time_t seed=0;
    if (!seed) {
        seed = time(NULL);
        srandom(seed);
    }
    random_value = (unsigned char)random();
}

#if !defined(TESTING)
static void main_thread(void *dummyptr) {
    struct timespec sleeptime = { .tv_sec=0, .tv_nsec=8333333 }; // 120Hz

    c_keys_set_key(kF8); // show credits
    do
    {
        video_sync(0);
        nanosleep(&sleeptime, NULL);
    } while (1);
}

extern void cpu_thread(void *dummyptr);

int main(int sargc, char *sargv[])
{
    argc = sargc;
    argv = sargv;

    load_settings();                    /* user prefs */
    c_initialize_firsttime();           /* init svga graphics and vm */

    // spin off cpu thread
    pthread_create(&cpu_thread_id, NULL, (void *) &cpu_thread, (void *)NULL);

    // continue with main render thread
    main_thread(NULL);
}
#endif // TESTING

