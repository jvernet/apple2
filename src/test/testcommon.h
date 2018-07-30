/*
 * Apple // emulator for *ix
 *
 * This software package is subject to the GNU General Public License
 * version 3 or later (your choice) as published by the Free Software
 * Foundation.
 *
 * Copyright 2013-2015 Aaron Culliney
 *
 */

#ifndef _TESTCOMMON_H_
#define _TESTCOMMON_H_

#include "common.h"
#include "greatest.h"

#ifdef __APPLE__
#   include <CommonCrypto/CommonDigest.h>
#   define SHA_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH
#   define SHA1 CC_SHA1
#else
#   include "test/sha1.h"
#endif

#define TEST_FINISHED 0xff
#define MIXSWITCH_ADDR 0x1f32   // PEEK(7986) -- NOTE : value is hardcoded in various places
#define WATCHPOINT_ADDR 0x1f33  // PEEK(7987) -- NOTE : value is hardcoded in various places
#define TESTOUT_ADDR 0x1f43     // PEEK(8003) -- NOTE : value is hardcoded in various places

#define BLANK_SCREEN "6C8ABA272F220F00BE0E76A8659A1E30C2D3CDBE"
#define BOOT_SCREEN  "F8D6C781E0BB7B3DDBECD69B25E429D845506594"

extern char mdstr[(SHA_DIGEST_LENGTH*2)+1];

extern bool test_do_reboot;
void test_breakpoint(void *arg);
void test_common_init(void);
void test_common_setup(void);
void test_type_input(const char *input);
void test_type_input_deterministically(const char *input);
char **test_copy_disk_paths(const char *fileName);
int test_setup_boot_disk(const char *fileName, int readonly);
void sha1_to_str(const uint8_t * const md, char *buf);

static inline bool _matchFramebufferSHA(const char *SHA_STR, bool is_old) {
    uint8_t md[SHA_DIGEST_LENGTH];

    bool matches = false;
    for (unsigned int i=0; i<2; i++) { // HACK: pump for at least 2 video frames to accommodate testing loading state
        uint8_t *fb = NULL;
        if (is_old) {
            extern uint8_t *display_renderStagingFramebuffer(void);
            fb = display_renderStagingFramebuffer();
        } else {
            fb = display_waitForNextCompleteFramebuffer();
        }
        SHA1(fb, SCANWIDTH*SCANHEIGHT, md);

        sha1_to_str(md, mdstr);
        matches = strcasecmp(mdstr, SHA_STR) == 0;
        if (matches) {
            break;
        }
    }

    return matches;
}

static inline int _assertFramebufferSHA(const char *SHA_STR, bool is_old) {
    bool matches = _matchFramebufferSHA(SHA_STR, is_old);
    ASSERT(matches && "check global mdstr if failed...");
    PASS();
}

#define ASSERT_SHA(SHA_STR) \
do { \
    int ret = _assertFramebufferSHA(SHA_STR, /*is_old:*/false); \
    if (ret != 0) { \
        return ret; \
    } \
} while (0);

#define ASSERT_SHA_OLD(SHA_STR) \
do { \
    int ret = _assertFramebufferSHA(SHA_STR, /*is_old:*/true); \
    if (ret != 0) { \
        return ret; \
    } \
} while (0);

#define WAIT_FOR_FB_SHA(SHA_STR) \
do { \
    unsigned int matchAttempts = 0; \
    const unsigned int maxMatchAttempts = 10; \
    do { \
        bool matches = _matchFramebufferSHA(SHA_STR, /*is_old:*/false); \
        if (matches) { \
            break; \
        } \
    } while (matchAttempts++ < maxMatchAttempts); \
    if (matchAttempts >= maxMatchAttempts) { \
        fprintf(GREATEST_STDOUT, "DID NOT FIND SHA %s...\n", SHA_STR); \
        ASSERT(0); \
    } \
} while (0);

static inline int ASSERT_SHA_MEM(const char *SHA_STR, uint16_t ea, uint16_t len) {
    uint8_t md[SHA_DIGEST_LENGTH];
    const uint8_t * const mem = &apple_ii_64k[0][ea];
    SHA1(mem, len, md);
    sha1_to_str(md, mdstr);
    ASSERT(strcasecmp(mdstr, SHA_STR) == 0);
    return 0;
}

static inline int ASSERT_SHA_BIN(const char *SHA_STR, const uint8_t * const buf, unsigned long len) {
    uint8_t md[SHA_DIGEST_LENGTH];
    SHA1(buf, len, md);
    sha1_to_str(md, mdstr);
    ASSERT(strcasecmp(mdstr, SHA_STR) == 0);
    return 0;
}

static inline int BOOT_TO_DOS(void) {
    if (test_do_reboot) {
        ASSERT(apple_ii_64k[0][WATCHPOINT_ADDR] != TEST_FINISHED);
        c_debugger_go();
        ASSERT(apple_ii_64k[0][WATCHPOINT_ADDR] == TEST_FINISHED);
        apple_ii_64k[0][WATCHPOINT_ADDR] = 0x00;
    }
    return 0;
}

static inline void REBOOT_TO_DOS(void) {
    apple_ii_64k[0][WATCHPOINT_ADDR] = 0x00;
    apple_ii_64k[0][TESTOUT_ADDR] = 0x00;
    run_args.joy_button0 = 0xff;
    cpu65_interrupt(ResetSig);
}

#endif // whole file
