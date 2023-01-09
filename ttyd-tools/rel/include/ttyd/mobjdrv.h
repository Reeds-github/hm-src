#pragma once

#include <cstdint>

namespace ttyd::mobjdrv {

extern "C" {

// .text
// mobjNearDistCheck2
// mobjCheckItemboxOpen
// mobjCheckExec
// mobjGetHint
// mobjCalcMtx2
void mobjCalcMtx(void* mobjPtr);
// mobjRunEvent
// mobjHitObjPtrToPtr
// mobjNameToPtrNoAssert
void* mobjNameToPtr(const char* name);
// mobjMain
// mobjSetPosition
// mobjDelete
// mobjEntry
// mobjHitEntry
// mobjReset
// mobjInit
// mobjDisp_OffscreenXLU
// mobjDisp
// mobjDispXLU
// mobjKoopaOn

// .data
// cubeDVP
// cubeDVN
// cubeDDT
// cubeDN
// cubeDP
// cubeDJ
// headDP
// headDJ
// frontDP
// frontDJ
// front2DP
// front2DJ

// .sdata
// cubeMSJ
// front2MSJ

// .bss
// work

// .sbss
// koopaRunFlag

}

}