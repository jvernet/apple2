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

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

#include "cpu.h"
#include "win-shim.h"

typedef enum eIRQSRC {
    IS_6522=0x08, // NOTE : matches IRQ... defines in cpu.h
    IS_SPEECH=0x10,
    IS_SSC=0x20,
    IS_MOUSE=0x40
} eIRQSRC;

typedef enum SS_CARDTYPE
{
    CT_Empty = 0,
    CT_Disk2,           // Apple Disk][
    CT_SSC,             // Apple Super Serial Card
    CT_MockingboardC,   // Soundcard
    CT_GenericPrinter,
    CT_GenericHDD,      // Hard disk
    CT_GenericClock,
    CT_MouseInterface,
    CT_Z80,
    CT_Phasor,          // Soundcard
    CT_Echo,            // Soundcard
    CT_SAM,             // Soundcard: Software Automated Mouth
} SS_CARDTYPE;

typedef struct
{
    DWORD dwLength;     // Byte length of this unit struct
    DWORD dwVersion;
} SS_UNIT_HDR;

typedef struct
{
    SS_UNIT_HDR UnitHdr;
    DWORD dwType;        // SS_CARDTYPE
    DWORD dwSlot;        // [1..7]
} SS_CARD_HDR;

void CpuIrqAssert(eIRQSRC irq_source);
void CpuIrqDeassert(eIRQSRC irq_source);

#endif