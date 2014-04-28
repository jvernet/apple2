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

#ifndef _TESTCOMMON_H_
#define _TESTCOMMON_H_

#include "common.h"
#include "greatest.h"

#define TEST_FINISHED 0xff
#define MIXSWITCH_ADDR 0x1f32   // PEEK(7986)
#define WATCHPOINT_ADDR 0x1f33  // PEEK(7987)
#define TESTOUT_ADDR 0x1f43     // PEEK(8003)

#define BLANK_SCREEN "6C8ABA272F220F00BE0E76A8659A1E30C2D3CDBE"
#define BOOT_SCREEN  "F8D6C781E0BB7B3DDBECD69B25E429D845506594"

#define ASSERT_SHA(SHA_STR) \
    do { \
        uint8_t md[SHA_DIGEST_LENGTH]; \
        char mdstr[(SHA_DIGEST_LENGTH*2)+1]; \
        const uint8_t * const fb = video_current_framebuffer(); \
        SHA1(fb, SCANWIDTH*SCANHEIGHT, md); \
        sha1_to_str(md, mdstr); \
        ASSERT(strcmp(mdstr, SHA_STR) == 0); \
    } while(0);

#define BOOT_TO_DOS() \
    if (test_do_reboot) { \
        ASSERT(apple_ii_64k[0][WATCHPOINT_ADDR] != TEST_FINISHED); \
        c_debugger_go(); \
        ASSERT(apple_ii_64k[0][WATCHPOINT_ADDR] == TEST_FINISHED); \
        ASSERT_SHA(BOOT_SCREEN); \
        apple_ii_64k[0][WATCHPOINT_ADDR] = 0x00; \
    }

void test_breakpoint(void *arg);
void test_common_init(bool do_cputhread);
void test_common_setup();
void test_type_input(const char *input);

#endif // whole file