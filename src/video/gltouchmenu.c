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

#include "video/glhudmodel.h"
#include "video/glnode.h"

#if !INTERFACE_TOUCH
#error this is a touch interface module, possibly you mean to not compile this at all?
#endif

#define MODEL_DEPTH -1/32.f

#define MENU_TEMPLATE_COLS 10
#define MENU_TEMPLATE_ROWS 2

#define MENU_FB_WIDTH (MENU_TEMPLATE_COLS * FONT80_WIDTH_PIXELS)
#define MENU_FB_HEIGHT (MENU_TEMPLATE_ROWS * FONT_HEIGHT_PIXELS)

#define MENU_OBJ_W 2.0
#define MENU_OBJ_H_LANDSCAPE (2.0/4.0) // NOTE : intent is to match touch keyboard height in landscape mode
#define MENU_OBJ_H_PORTRAIT (1.0/4.0)

static bool isAvailable = false; // Were there any OpenGL/memory errors on initialization?
static bool isEnabled = true;    // Does player want this enabled?

// NOTE : intent is to match touch keyboard width
static uint8_t topMenuTemplate[MENU_TEMPLATE_ROWS][MENU_TEMPLATE_COLS+1] = {
    "++      ++",
    "++      ++",
};

// touch viewport

static struct {
    int width;
    int height;

    // top left hitbox
    int topLeftX;
    int topLeftXHalf;
    int topLeftXMax;
    int topLeftY;
    int topLeftYHalf;
    int topLeftYMax;

    // top right hitbox
    int topRightX;
    int topRightXHalf;
    int topRightXMax;
    int topRightY;
    int topRightYHalf;
    int topRightYMax;

    GLfloat modelHeight;
} touchport = { 0 };

// touch menu variables

static struct {
    GLModel *model;
    unsigned int glyphMultiplier;
    bool topLeftShowing;
    bool topRightShowing;

    struct timespec timingBegin;
    float minAlpha; // Minimum alpha value of components (at zero, will not render)
    float maxAlpha;

    // pending changes requiring reinitialization
    bool prefsChanged;
} menu = { 0 };

static void gltouchmenu_applyPrefs(void);

// ----------------------------------------------------------------------------

static inline void _present_menu(GLModel *parent) {
    GLModelHUDElement *hudMenu = (GLModelHUDElement *)parent->custom;
    memcpy(hudMenu->tpl, topMenuTemplate, sizeof(topMenuTemplate));
    glhud_setupDefault(parent);
}

static inline void _show_top_left(void) {
    topMenuTemplate[0][0]  = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[0][1]  = MOUSETEXT_RIGHT;

    long lVal = 0;
    interface_device_t screenOwner = prefs_parseLongValue(PREF_DOMAIN_TOUCHSCREEN, PREF_SCREEN_OWNER, &lVal, /*base:*/10) ? (interface_device_t)lVal : TOUCH_DEVICE_NONE;

    if (screenOwner == TOUCH_DEVICE_JOYSTICK || screenOwner == TOUCH_DEVICE_JOYSTICK_KEYPAD) {
        topMenuTemplate[1][0] = ICONTEXT_UPPERCASE;
        if (screenOwner == TOUCH_DEVICE_JOYSTICK) {
            topMenuTemplate[1][1] = ICONTEXT_MENU_TOUCHJOY_KPAD;
        } else {
            topMenuTemplate[1][1] = ICONTEXT_MENU_TOUCHJOY;
        }
    } else {
        topMenuTemplate[1][0] = ICONTEXT_MENU_TOUCHJOY;
        topMenuTemplate[1][1] = ICONTEXT_MENU_TOUCHJOY_KPAD;
    }

    menu.topLeftShowing = true;
    _present_menu(menu.model);
}

static inline void _hide_top_left(void) {
    topMenuTemplate[0][0] = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[0][1] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][0] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][1] = ICONTEXT_NONACTIONABLE;
    menu.topLeftShowing = false;
    _present_menu(menu.model);
}

static inline void _show_top_right(void) {
    topMenuTemplate[0][MENU_TEMPLATE_COLS-2] = MOUSETEXT_LEFT;
    topMenuTemplate[0][MENU_TEMPLATE_COLS-1] = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-2] = MOUSETEXT_CURSOR1;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-1] = MOUSETEXT_CHECKMARK;
    menu.topRightShowing = true;
    _present_menu(menu.model);
}

static inline void _hide_top_right(void) {
    topMenuTemplate[0][MENU_TEMPLATE_COLS-2] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[0][MENU_TEMPLATE_COLS-1] = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-2] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-1] = ICONTEXT_NONACTIONABLE;
    menu.topRightShowing = false;
    _present_menu(menu.model);
}

static inline bool _is_point_on_left_menu(float x, float y) {
    if (menu.topLeftShowing) {
        return (x >= touchport.topLeftX && x <= touchport.topLeftXMax && y >= touchport.topLeftY && y <= touchport.topLeftYMax);
    } else {
        return (x >= touchport.topLeftX && x <= touchport.topLeftXHalf && y >= touchport.topLeftY && y <= touchport.topLeftYHalf);
    }
}

static inline bool _is_point_on_right_menu(float x, float y) {
    if (menu.topRightShowing) {
        return (x >= touchport.topRightX && x <= touchport.topRightXMax && y >= touchport.topRightY && y <= touchport.topRightYMax);
    } else {
        return (x >= touchport.topRightXHalf && x <= touchport.topRightXMax && y >= touchport.topRightY && y <= touchport.topRightYHalf);
    }
}

#warning FIXME TODO : make this function generic _screen_to_model() ?
static inline void _screen_to_menu(float x, float y, OUTPARM int *col, OUTPARM int *row) {

    GLModelHUDElement *hudMenu = (GLModelHUDElement *)(menu.model->custom);
    const unsigned int keyW = touchport.width / hudMenu->tplWidth;
    const unsigned int keyH = touchport.topLeftYMax / hudMenu->tplHeight;

    *col = x / keyW;
    if (*col < 0) {
        *col = 0;
    } else if (*col >= hudMenu->tplWidth) {
        *col = hudMenu->tplWidth-1;
    }
    *row = y / keyH;
    if (*row < 0) {
        *row = 0;
    } else if (*row >= hudMenu->tplHeight) {
        *row = hudMenu->tplWidth-1;
    }

    //LOG("SCREEN TO MENU : menuX:%d menuXMax:%d menuW:%d keyW:%d ... scrn:(%f,%f)->kybd:(%d,%d)", touchport.topLeftX, touchport.topLeftXMax, touchport.width, keyW, x, y, *col, *row);
}

static bool _sprout_menu(float x, float y) {

    if (! (_is_point_on_left_menu(x, y) || _is_point_on_right_menu(x, y)) ) {
        return false;
    }

    int col = -1;
    int row = -1;

    _screen_to_menu(x, y, &col, &row);
    bool isTopLeft = (col <= 1);
    bool isTopRight = (col >= MENU_TEMPLATE_COLS-2);

    if (isTopLeft) {

        // hide other
        _hide_top_right();

        // maybe show this one
        if (menu.topLeftShowing) {
            if (col == 0 && row == 0) {
                _hide_top_left();
            }
        } else {
            if (col == 0 && row == 0) {
                _show_top_left();
            }
        }

        return menu.topLeftShowing;
    } else if (isTopRight) {

        // hide other
        _hide_top_left();

        // maybe show this one
        if (menu.topRightShowing) {
            if (col == MENU_TEMPLATE_COLS-1 && row == 0) {
                _hide_top_right();
            }
        } else {
            if (col == MENU_TEMPLATE_COLS-1 && row == 0) {
                _show_top_right();
            }
        }
        return menu.topRightShowing;
    } else {
        LOG("This should not happen");
        assert(false);
        return false;
    }
}

static inline void _step_cpu_speed(int delta) {
    bool wasPaused = cpu_isPaused();

    if (!wasPaused) {
        cpu_pause();
    }

    // TODO FIXME : consolidate with other CPU stepping code in interface.c/timing.c/glalert.c animation =D
    float scale = roundf(cpu_scale_factor * 100.f);
    if (delta < 0) {
        if (scale > 400.f) {
            scale = 375.f;
        } else if (scale > 100.f) {
            scale -= 25.f;
        } else {
            scale -= 5.f;
        }
    } else {
        if (scale >= 100.f) {
            scale += 25.f;
        } else {
            scale += 5.f;
        }
    }

    prefs_setFloatValue(PREF_DOMAIN_VM, PREF_CPU_SCALE, scale);
    prefs_sync(PREF_DOMAIN_VM);

    if (video_getAnimationDriver()->animation_showCPUSpeed) {
        video_getAnimationDriver()->animation_showCPUSpeed();
    }

    timing_initialize();

    if (!wasPaused) {
        cpu_resume();
    }
}

static int64_t _tap_menu_item(float x, float y) {
    if (! (_is_point_on_left_menu(x, y) || _is_point_on_right_menu(x, y)) ) {
        return 0x0LL;
    }

    int col = -1;
    int row = -1;

    _screen_to_menu(x, y, &col, &row);

    uint8_t selectedItem = topMenuTemplate[row][col];

    int64_t flags = TOUCH_FLAGS_KEY_TAP | TOUCH_FLAGS_HANDLED;
    switch (selectedItem) {

        case MOUSETEXT_LEFT:
            LOG("decreasing cpu speed...");
            _step_cpu_speed(-1);
            break;

        case MOUSETEXT_RIGHT:
            LOG("increasing cpu speed...");
            _step_cpu_speed(1);
            break;

        case MOUSETEXT_CHECKMARK:
            LOG("showing main menu...");
            flags |= TOUCH_FLAGS_REQUEST_HOST_MENU;
            _hide_top_right();
            prefs_save();
            break;

        case MOUSETEXT_CURSOR0:
        case MOUSETEXT_CURSOR1:
            LOG("showing system keyboard...");
            flags |= TOUCH_FLAGS_REQUEST_SYSTEM_KBD;
            _hide_top_right();
            prefs_save();
            break;

        case ICONTEXT_MENU_TOUCHJOY:
            LOG("switching to joystick  ...");
            flags |= TOUCH_FLAGS_INPUT_DEVICE_CHANGE;
            flags |= TOUCH_FLAGS_JOY;
            _hide_top_left();
            prefs_setLongValue(PREF_DOMAIN_TOUCHSCREEN, PREF_SCREEN_OWNER, TOUCH_DEVICE_JOYSTICK);
            prefs_sync(PREF_DOMAIN_TOUCHSCREEN);
            break;

        case ICONTEXT_MENU_TOUCHJOY_KPAD:
            LOG("switching to keypad joystick  ...");
            flags |= TOUCH_FLAGS_INPUT_DEVICE_CHANGE;
            flags |= TOUCH_FLAGS_JOY_KPAD;
            _hide_top_left();
            prefs_setLongValue(PREF_DOMAIN_TOUCHSCREEN, PREF_SCREEN_OWNER, TOUCH_DEVICE_JOYSTICK_KEYPAD);
            prefs_sync(PREF_DOMAIN_TOUCHSCREEN);
            break;

        case ICONTEXT_UPPERCASE:
            LOG("switching to keyboard  ...");
            flags |= TOUCH_FLAGS_INPUT_DEVICE_CHANGE;
            flags |= TOUCH_FLAGS_KBD;
            _hide_top_left();
            prefs_setLongValue(PREF_DOMAIN_TOUCHSCREEN, PREF_SCREEN_OWNER, TOUCH_DEVICE_KEYBOARD);
            prefs_sync(PREF_DOMAIN_TOUCHSCREEN);
            break;

        case ICONTEXT_MENU_SPROUT:
            LOG("sprout ...");
            break;

        case ICONTEXT_NONACTIONABLE:
        default:
            LOG("nonactionable ...");
            flags = 0x0LL;
            _hide_top_left();
            _hide_top_right();
            break;
    }

    return flags;
}

// ----------------------------------------------------------------------------
// GLCustom functions

static void *_create_touchmenu_hud(GLModel *parent) {
    parent->custom = glhud_createCustom(sizeof(GLModelHUDElement));
    GLModelHUDElement *hudMenu = (GLModelHUDElement *)parent->custom;

    if (!hudMenu) {
        return NULL;
    }

    hudMenu->blackIsTransparent = true;
    hudMenu->opaquePixelHalo = true;
    hudMenu->glyphMultiplier = menu.glyphMultiplier;

    hudMenu->tplWidth  = MENU_TEMPLATE_COLS;
    hudMenu->tplHeight = MENU_TEMPLATE_ROWS;
    hudMenu->pixWidth  = MENU_FB_WIDTH;
    hudMenu->pixHeight = MENU_FB_HEIGHT;

    topMenuTemplate[0][0]  = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[0][1]  = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][0]  = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][1]  = ICONTEXT_NONACTIONABLE;

    topMenuTemplate[0][MENU_TEMPLATE_COLS-2] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[0][MENU_TEMPLATE_COLS-1] = ICONTEXT_MENU_SPROUT;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-2] = ICONTEXT_NONACTIONABLE;
    topMenuTemplate[1][MENU_TEMPLATE_COLS-1] = ICONTEXT_NONACTIONABLE;

    for (unsigned int row=0; row<MENU_TEMPLATE_ROWS; row++) {
        for (unsigned int col=2; col<MENU_TEMPLATE_COLS-2; col++) {
            topMenuTemplate[row][col] = ICONTEXT_NONACTIONABLE;
        }
    }

    const unsigned int size = sizeof(topMenuTemplate);
    hudMenu->tpl = CALLOC(size, 1);
    hudMenu->pixels = CALLOC(MENU_FB_WIDTH * MENU_FB_HEIGHT * PIXEL_STRIDE, 1);

    _present_menu(parent);

    return hudMenu;
}

// ----------------------------------------------------------------------------
// GLNode functions

static void gltouchmenu_shutdown(void) {
    LOG("gltouchmenu_shutdown ...");
    if (!isAvailable) {
        return;
    }

    isAvailable = false;

    menu.topLeftShowing = false;
    menu.topRightShowing = false;

    mdlDestroyModel(&menu.model);
}

static void gltouchmenu_setup(void) {
    LOG("gltouchmenu_setup ...");

    gltouchmenu_shutdown();

    if (menu.prefsChanged) {
        gltouchmenu_applyPrefs();
    }

    menu.model = mdlCreateQuad((GLModelParams_s){
            .skew_x = -1.0,
            .skew_y = 1.0-touchport.modelHeight,
            .z = MODEL_DEPTH,
            .obj_w = MENU_OBJ_W,
            .obj_h = touchport.modelHeight,
            .positionUsageHint = GL_STATIC_DRAW, // positions don't change
            .tex_w = MENU_FB_WIDTH * menu.glyphMultiplier,
            .tex_h = MENU_FB_HEIGHT * menu.glyphMultiplier,
            .texcoordUsageHint = GL_DYNAMIC_DRAW, // but menu texture does
        }, (GLCustom){
            .create = &_create_touchmenu_hud,
            .destroy = &glhud_destroyDefault,
        });
    if (!menu.model) {
        LOG("gltouchmenu initialization problem");
        return;
    }
    if (!menu.model->custom) {
        LOG("gltouchmenu HUD initialization problem");
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &menu.timingBegin);

    isAvailable = true;

    GL_MAYBELOG("gltouchmenu_setup");
}

static void gltouchmenu_render(void) {
    if (!isAvailable) {
        return;
    }
    if (UNLIKELY(menu.prefsChanged)) {
        gltouchmenu_setup(); // fully set up again on prefs change
    }
    if (!isEnabled) {
        return;
    }

    float alpha = glhud_getTimedVisibility(menu.timingBegin, menu.minAlpha, menu.maxAlpha);
    if (alpha < menu.minAlpha) {
        alpha = menu.minAlpha;
    }
    if (alpha <= 0.0) {
        return;
    }

    glViewport(0, 0, touchport.width, touchport.height); // NOTE : show these HUD elements beyond the A2 framebuffer dimensions
    glUniform1f(alphaValue, alpha);

    // render top sprouting menu(s)

    glActiveTexture(TEXTURE_ACTIVE_TOUCHMENU);
    glBindTexture(GL_TEXTURE_2D, menu.model->textureName);
    if (menu.model->texDirty) {
        menu.model->texDirty = false;
        _HACKAROUND_GLTEXIMAGE2D_PRE(TEXTURE_ACTIVE_TOUCHMENU, menu.model->textureName);
        glTexImage2D(GL_TEXTURE_2D, /*level*/0, TEX_FORMAT_INTERNAL, menu.model->texWidth, menu.model->texHeight, /*border*/0, TEX_FORMAT, TEX_TYPE, menu.model->texPixels);
    }
    glUniform1i(texSamplerLoc, TEXTURE_ID_TOUCHMENU);
    glhud_renderDefault(menu.model);

    GL_MAYBELOG("gltouchmenu_render");
}

static void gltouchmenu_reshape(int w, int h, bool landscape) {
    LOG("w:%d h:%d landscape:%d", w, h, landscape);
    assert(video_isRenderThread());

    touchport.topLeftX = 0;
    touchport.topLeftY = 0;
    touchport.topRightY = 0;

    swizzleDimensions(&w, &h, landscape);
    touchport.width = w;
    touchport.height = h;

    touchport.modelHeight = landscape ? MENU_OBJ_H_LANDSCAPE : MENU_OBJ_H_PORTRAIT;;

    const unsigned int keyW = touchport.width / MENU_TEMPLATE_COLS;
    touchport.topLeftXHalf  = keyW;
    touchport.topLeftXMax   = keyW*2;
    touchport.topRightX     = w - (keyW*2);
    touchport.topRightXHalf = w - keyW;
    touchport.topRightXMax  = w;

    const unsigned int menuH = h * (touchport.modelHeight/2.0);
    touchport.topLeftYHalf  = menuH/2;
    touchport.topLeftYMax   = menuH;
    touchport.topRightYHalf = menuH/2;
    touchport.topRightYMax  = menuH;
}

static int64_t gltouchmenu_onTouchEvent(interface_touch_event_t action, int pointer_count, int pointer_idx, float *x_coords, float *y_coords) {

    if (!isAvailable) {
        return 0x0;
    }
    if (!isEnabled) {
        return 0x0;
    }
    if (UNLIKELY(menu.prefsChanged)) {
        return 0x0;
    }

    //LOG("gltouchmenu_onTouchEvent ...");

    float x = x_coords[pointer_idx];
    float y = y_coords[pointer_idx];

    int flags = TOUCH_FLAGS_MENU;

    static int trackingIndex = TRACKING_NONE;

    switch (action) {
        case TOUCH_DOWN:
        case TOUCH_POINTER_DOWN:
            if (_is_point_on_left_menu(x, y) || _is_point_on_right_menu(x, y)) {
                trackingIndex = pointer_idx;
                _sprout_menu(x, y);
                flags |= TOUCH_FLAGS_HANDLED;
            }
            break;

        case TOUCH_MOVE:
            flags |= ((pointer_idx == trackingIndex) ? TOUCH_FLAGS_HANDLED : 0);
            break;

        case TOUCH_UP:
        case TOUCH_POINTER_UP:
            if (trackingIndex == pointer_idx) {
                flags |= _tap_menu_item(x, y);
                trackingIndex = TRACKING_NONE;
            }
            break;

        case TOUCH_CANCEL:
            trackingIndex = TRACKING_NONE;
            LOG("---MENU TOUCH CANCEL");
            return 0x0;

        default:
            trackingIndex = TRACKING_NONE;
            LOG("!!!MENU UNKNOWN TOUCH EVENT : %d", action);
            return 0x0;
    }

    if (flags & TOUCH_FLAGS_HANDLED) {
        clock_gettime(CLOCK_MONOTONIC, &menu.timingBegin);
    }

    return flags;
}

// ----------------------------------------------------------------------------
// Animation and settings handling

static void _animation_showTouchMenu(void) {
    clock_gettime(CLOCK_MONOTONIC, &menu.timingBegin);
}

static void _animation_hideTouchMenu(void) {
    _hide_top_left();
    _hide_top_right();
    menu.timingBegin = (struct timespec){ 0 };
}

static void gltouchmenu_applyPrefs(void) {
    assert(video_isRenderThread());

    menu.prefsChanged = false;

    bool bVal = false;
    float fVal = 0.f;
    long lVal = 0;

    isEnabled            = prefs_parseBoolValue (PREF_DOMAIN_KEYBOARD, PREF_TOUCHMENU_ENABLED, &bVal) ? bVal : true;

    menu.minAlpha        = prefs_parseFloatValue(PREF_DOMAIN_KEYBOARD, PREF_MIN_ALPHA,        &fVal) ? fVal : 1/4.f;
    menu.maxAlpha        = prefs_parseFloatValue(PREF_DOMAIN_KEYBOARD, PREF_MAX_ALPHA,        &fVal) ? fVal : 1.f;
    menu.glyphMultiplier = prefs_parseLongValue (PREF_DOMAIN_KEYBOARD, PREF_GLYPH_MULTIPLIER, &lVal, /*base:*/10) ? lVal : 2;
    if (menu.glyphMultiplier == 0) {
        menu.glyphMultiplier = 1;
    }
    if (menu.glyphMultiplier > 4) {
        menu.glyphMultiplier = 4;
    }

    long width            = prefs_parseLongValue (PREF_DOMAIN_INTERFACE, PREF_DEVICE_WIDTH,      &lVal, 10) ? lVal : (long)(SCANWIDTH*1.5);
    long height           = prefs_parseLongValue (PREF_DOMAIN_INTERFACE, PREF_DEVICE_HEIGHT,     &lVal, 10) ? lVal : (long)(SCANHEIGHT*1.5);
    bool isLandscape      = prefs_parseBoolValue (PREF_DOMAIN_INTERFACE, PREF_DEVICE_LANDSCAPE,  &bVal)     ? bVal : true;

    glhud_currentColorScheme = prefs_parseLongValue(PREF_DOMAIN_INTERFACE, PREF_SOFTHUD_COLOR, &lVal, 10) ? (interface_colorscheme_t)lVal : RED_ON_BLACK;

    gltouchmenu_reshape(width, height, isLandscape);
}

static void gltouchmenu_prefsChanged(const char *domain) {
    menu.prefsChanged = true;
}

// ----------------------------------------------------------------------------
// Constructor

static void _init_gltouchmenu(void) {
    LOG("Registering OpenGL software touch menu");

    video_getAnimationDriver()->animation_showTouchMenu = &_animation_showTouchMenu;
    video_getAnimationDriver()->animation_hideTouchMenu = &_animation_hideTouchMenu;

    menu.prefsChanged = true;

    glnode_registerNode(RENDER_TOP, (GLNode){
        .type = TOUCH_DEVICE_TOPMENU,
        .setup = &gltouchmenu_setup,
        .shutdown = &gltouchmenu_shutdown,
        .render = &gltouchmenu_render,
        .onTouchEvent = &gltouchmenu_onTouchEvent,
    });

    prefs_registerListener(PREF_DOMAIN_KEYBOARD, &gltouchmenu_prefsChanged);
    prefs_registerListener(PREF_DOMAIN_TOUCHSCREEN, &gltouchmenu_prefsChanged);
    prefs_registerListener(PREF_DOMAIN_INTERFACE, &gltouchmenu_prefsChanged);
}

static __attribute__((constructor)) void __init_gltouchmenu(void) {
    emulator_registerStartupCallback(CTOR_PRIORITY_LATE, &_init_gltouchmenu);
}

