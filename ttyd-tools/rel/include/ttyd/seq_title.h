#pragma once

#include <ttyd/filemgr.h>

#include <cstdint>

namespace ttyd::seq_title {

extern "C" {

// pressStartGX
// disp2
// disp1
// titleMain
void L_titleInit();
// N_getDebugMode
// seq_titleMain
// seq_titleExit
// seq_titleInit
// DbgBtlSel_GetMsgDataPtr

extern filemgr::filemgr__File **seqTitleWorkPointer2;

}

}