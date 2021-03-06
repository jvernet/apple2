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

// Modified sample code from https://developer.apple.com/library/mac/samplecode/GLEssentials/Introduction/Intro.html

#ifndef __IMAGE_UTIL_H__
#define __IMAGE_UTIL_H__

#include "glUtil.h"

typedef struct demoImageRec {
    GLubyte *data;
    GLsizei size;
    GLuint width;
    GLuint height;
    GLenum format;
    GLenum type;
    GLuint rowByteSize;
} demoImage;

demoImage *imgLoadImage(const char *filepathname, int flipVertical);

void imgDestroyImage(demoImage *image);

#endif //__IMAGE_UTIL_H__

