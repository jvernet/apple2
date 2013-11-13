/*
 * OpenAL Helpers
 *
 * Copyright (c) 2011 by Chris Robinson <chris.kcat@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* This file contains routines to help with some menial OpenAL-related tasks,
 * such as opening a device and setting up a context, closing the device and
 * destroying its context, converting between frame counts and byte lengths,
 * finding an appropriate buffer format, and getting readable strings for
 * channel configs and sample types. */

#include <stdio.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

#include "alhelpers.h"
#include "common.h"


/* InitAL opens the default device and sets up a context using default
 * attributes, making the program ready to call OpenAL functions. */
ALCcontext* InitAL(void)
{
    ALCdevice *device;
    ALCcontext *ctx;

    /* Open and initialize a device with default settings */
    device = alcOpenDevice(NULL);
    if(!device)
    {
        ERRLOG("Could not open a device!");
        return NULL;
    }

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
        {
            alcDestroyContext(ctx);
        }
        alcCloseDevice(device);
        ERRLOG("Could not set a context!");
        return NULL;
    }

    LOG("Opened \"%s\"", alcGetString(device, ALC_DEVICE_SPECIFIER));
    return ctx;
}

/* CloseAL closes the device belonging to the current context, and destroys the
 * context. */
void CloseAL(void)
{
    ALCdevice *device;
    ALCcontext *ctx;

    ctx = alcGetCurrentContext();
    if(ctx == NULL)
    {
        return;
    }

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}


/* GetFormat retrieves a compatible buffer format given the channel config and
 * sample type. If an alIsBufferFormatSupportedSOFT-compatible function is
 * provided, it will be called to find the closest-matching format from
 * AL_SOFT_buffer_samples. Returns AL_NONE (0) if no supported format can be
 * found. */
ALenum GetFormat(ALenum channels, ALenum type, LPALISBUFFERFORMATSUPPORTEDSOFT palIsBufferFormatSupportedSOFT)
{
    ALenum format = AL_NONE;

    /* If using AL_SOFT_buffer_samples, try looking through its formats */
    if(palIsBufferFormatSupportedSOFT)
    {
        /* AL_SOFT_buffer_samples is more lenient with matching formats. The
         * specified sample type does not need to match the returned format,
         * but it is nice to try to get something close. */
        if(type == AL_UNSIGNED_BYTE_SOFT || type == AL_BYTE_SOFT)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO8_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO8_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD8_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_8_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_8_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_8_SOFT;
        }
        else if(type == AL_UNSIGNED_SHORT_SOFT || type == AL_SHORT_SOFT)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO16_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO16_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD16_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_16_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_16_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_16_SOFT;
        }
        else if(type == AL_UNSIGNED_BYTE3_SOFT || type == AL_BYTE3_SOFT ||
                type == AL_UNSIGNED_INT_SOFT || type == AL_INT_SOFT ||
                type == AL_FLOAT_SOFT || type == AL_DOUBLE_SOFT)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO32F_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO32F_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD32F_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_32F_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_32F_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_32F_SOFT;
        }

        if(format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
            format = AL_NONE;

        /* A matching format was not found or supported. Try 32-bit float. */
        if(format == AL_NONE)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO32F_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO32F_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD32F_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_32F_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_32F_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_32F_SOFT;

            if(format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
                format = AL_NONE;
        }
        /* 32-bit float not supported. Try 16-bit int. */
        if(format == AL_NONE)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO16_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO16_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD16_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_16_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_16_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_16_SOFT;

            if(format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
                format = AL_NONE;
        }
        /* 16-bit int not supported. Try 8-bit int. */
        if(format == AL_NONE)
        {
            if(channels == AL_MONO_SOFT) format = AL_MONO8_SOFT;
            else if(channels == AL_STEREO_SOFT) format = AL_STEREO8_SOFT;
            else if(channels == AL_QUAD_SOFT) format = AL_QUAD8_SOFT;
            else if(channels == AL_5POINT1_SOFT) format = AL_5POINT1_8_SOFT;
            else if(channels == AL_6POINT1_SOFT) format = AL_6POINT1_8_SOFT;
            else if(channels == AL_7POINT1_SOFT) format = AL_7POINT1_8_SOFT;

            if(format != AL_NONE && !palIsBufferFormatSupportedSOFT(format))
                format = AL_NONE;
        }

        return format;
    }

    /* We use the AL_EXT_MCFORMATS extension to provide output of Quad, 5.1,
     * and 7.1 channel configs, AL_EXT_FLOAT32 for 32-bit float samples, and
     * AL_EXT_DOUBLE for 64-bit float samples. */
    if(type == AL_UNSIGNED_BYTE_SOFT)
    {
        if(channels == AL_MONO_SOFT)
            format = AL_FORMAT_MONO8;
        else if(channels == AL_STEREO_SOFT)
            format = AL_FORMAT_STEREO8;
        else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
        {
            if(channels == AL_QUAD_SOFT)
                format = alGetEnumValue("AL_FORMAT_QUAD8");
            else if(channels == AL_5POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_51CHN8");
            else if(channels == AL_6POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_61CHN8");
            else if(channels == AL_7POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_71CHN8");
        }
    }
    else if(type == AL_SHORT_SOFT)
    {
        if(channels == AL_MONO_SOFT)
            format = AL_FORMAT_MONO16;
        else if(channels == AL_STEREO_SOFT)
            format = AL_FORMAT_STEREO16;
        else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
        {
            if(channels == AL_QUAD_SOFT)
                format = alGetEnumValue("AL_FORMAT_QUAD16");
            else if(channels == AL_5POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_51CHN16");
            else if(channels == AL_6POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_61CHN16");
            else if(channels == AL_7POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_71CHN16");
        }
    }
    else if(type == AL_FLOAT_SOFT && alIsExtensionPresent("AL_EXT_FLOAT32"))
    {
        if(channels == AL_MONO_SOFT)
            format = alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
        else if(channels == AL_STEREO_SOFT)
            format = alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
        else if(alIsExtensionPresent("AL_EXT_MCFORMATS"))
        {
            if(channels == AL_QUAD_SOFT)
                format = alGetEnumValue("AL_FORMAT_QUAD32");
            else if(channels == AL_5POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_51CHN32");
            else if(channels == AL_6POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_61CHN32");
            else if(channels == AL_7POINT1_SOFT)
                format = alGetEnumValue("AL_FORMAT_71CHN32");
        }
    }
    else if(type == AL_DOUBLE_SOFT && alIsExtensionPresent("AL_EXT_DOUBLE"))
    {
        if(channels == AL_MONO_SOFT)
            format = alGetEnumValue("AL_FORMAT_MONO_DOUBLE");
        else if(channels == AL_STEREO_SOFT)
            format = alGetEnumValue("AL_FORMAT_STEREO_DOUBLE");
    }

    /* NOTE: It seems OSX returns -1 from alGetEnumValue for unknown enums, as
     * opposed to 0. Correct it. */
    if(format == -1)
        format = 0;

    return format;
}


void AL_APIENTRY wrap_BufferSamples(ALuint buffer, ALuint samplerate,
                                    ALenum internalformat, ALsizei samples,
                                    ALenum channels, ALenum type,
                                    const ALvoid *data)
{
    alBufferData(buffer, internalformat, data,
                 FramesToBytes(samples, channels, type),
                 samplerate);
}


const char *ChannelsName(ALenum chans)
{
    switch(chans)
    {
    case AL_MONO_SOFT: return "Mono";
    case AL_STEREO_SOFT: return "Stereo";
    case AL_REAR_SOFT: return "Rear";
    case AL_QUAD_SOFT: return "Quadraphonic";
    case AL_5POINT1_SOFT: return "5.1 Surround";
    case AL_6POINT1_SOFT: return "6.1 Surround";
    case AL_7POINT1_SOFT: return "7.1 Surround";
    }
    return "Unknown Channels";
}

const char *TypeName(ALenum type)
{
    switch(type)
    {
    case AL_BYTE_SOFT: return "S8";
    case AL_UNSIGNED_BYTE_SOFT: return "U8";
    case AL_SHORT_SOFT: return "S16";
    case AL_UNSIGNED_SHORT_SOFT: return "U16";
    case AL_INT_SOFT: return "S32";
    case AL_UNSIGNED_INT_SOFT: return "U32";
    case AL_FLOAT_SOFT: return "Float32";
    case AL_DOUBLE_SOFT: return "Float64";
    }
    return "Unknown Type";
}

const char *FormatName(ALenum format)
{
    switch (format)
    {
        case AL_FORMAT_MONO8: return "MONO8";
        case AL_FORMAT_MONO16: return "MONO16";
        case AL_FORMAT_STEREO8: return "STEREO8";
        case AL_FORMAT_STEREO16: return "STEREO16";

        case AL_FORMAT_QUAD8: return "QUAD8";
        case AL_FORMAT_QUAD16: return "QUAD16";
        case AL_FORMAT_QUAD32: return "QUAD32";
        case AL_FORMAT_REAR8: return "REAR8";
        case AL_FORMAT_REAR16: return "REAR16";
        case AL_FORMAT_REAR32: return "REAR32";
        case AL_FORMAT_51CHN8: return "51CHN8";
        case AL_FORMAT_51CHN16: return "51CHN16";
        case AL_FORMAT_51CHN32: return "51CHN32";
        case AL_FORMAT_61CHN8: return "61CHN8";
        case AL_FORMAT_61CHN16: return "61CHN16";
        case AL_FORMAT_61CHN32: return "61CHN32";
        case AL_FORMAT_71CHN8: return "71CHN8";
        case AL_FORMAT_71CHN16: return "71CHN16";
        case AL_FORMAT_71CHN32: return "71CHN32";
        case AL_FORMAT_WAVE_EXT: return "WAVE_EXT";
        case AL_FORMAT_IMA_ADPCM_MONO16_EXT: return "IMA_ADPCM_MONO16_EXT";
        case AL_FORMAT_IMA_ADPCM_STEREO16_EXT: return "IMA_ADPCM_STEREO16_EXT";
        case AL_FORMAT_VORBIS_EXT: return "VORBIS_EXT";
        case AL_FORMAT_MONO_FLOAT32: return "MONO_FLOAT32";
        case AL_FORMAT_STEREO_FLOAT32: return "STEREO_FLOAT32";
        case AL_FORMAT_MONO_DOUBLE_EXT: return "MONO_DOUBLE_EXT";
        case AL_FORMAT_STEREO_DOUBLE_EXT: return "STEREO_DOUBLE_EXT";
        case AL_FORMAT_MONO_MULAW_EXT: return "MONO_MULAW(_EXT)";
        //case AL_FORMAT_MONO_MULAW: return "MONO_MULAW";
        case AL_FORMAT_STEREO_MULAW_EXT: return "STEREO_MULAW(_EXT)";
        //case AL_FORMAT_STEREO_MULAW: return "STEREO_MULAW";
        case AL_FORMAT_QUAD8_LOKI: return "QUAD8_LOKI";
        case AL_FORMAT_QUAD16_LOKI: return "QUAD16_LOKI";
        case AL_FORMAT_QUAD_MULAW: return "QUAD_MULAW";
        case AL_FORMAT_REAR_MULAW: return "REAR_MULAW";
        case AL_FORMAT_51CHN_MULAW: return "51CHN_MULAW";
        case AL_FORMAT_61CHN_MULAW: return "61CHN_MULAW";
        case AL_FORMAT_71CHN_MULAW: return "71CHN_MULAW";
        case AL_FORMAT_MONO_IMA4: return "MONO_IMA4";
        case AL_FORMAT_STEREO_IMA4: return "STEREO_IMA4";
    }
    return "Unknown Format";
}

ALsizei FramesToBytes(ALsizei size, ALenum channels, ALenum type)
{
    switch(channels)
    {
    case AL_MONO_SOFT:    size *= 1; break;
    case AL_STEREO_SOFT:  size *= 2; break;
    case AL_REAR_SOFT:    size *= 2; break;
    case AL_QUAD_SOFT:    size *= 4; break;
    case AL_5POINT1_SOFT: size *= 6; break;
    case AL_6POINT1_SOFT: size *= 7; break;
    case AL_7POINT1_SOFT: size *= 8; break;
    }

    switch(type)
    {
    case AL_BYTE_SOFT:           size *= sizeof(ALbyte); break;
    case AL_UNSIGNED_BYTE_SOFT:  size *= sizeof(ALubyte); break;
    case AL_SHORT_SOFT:          size *= sizeof(ALshort); break;
    case AL_UNSIGNED_SHORT_SOFT: size *= sizeof(ALushort); break;
    case AL_INT_SOFT:            size *= sizeof(ALint); break;
    case AL_UNSIGNED_INT_SOFT:   size *= sizeof(ALuint); break;
    case AL_FLOAT_SOFT:          size *= sizeof(ALfloat); break;
    case AL_DOUBLE_SOFT:         size *= sizeof(ALdouble); break;
    }

    return size;
}

ALsizei BytesToFrames(ALsizei size, ALenum channels, ALenum type)
{
    return size / FramesToBytes(1, channels, type);
}
