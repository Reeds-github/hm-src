#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_memcard {

extern "C" {

// .text
// memcard_write
// memcard_file_existance
// memcard_ipl
// memcard_header_write
// memcard_copy
// memcard_delete
// memcard_create
// memcard_format
EVT_DECLARE_USER_FUNC(memcard_load, 0)
// unk_8025c380
// unk_8025c3a4
// unk_8025c40c
// unk_8025c430
EVT_DECLARE_USER_FUNC(memcard_code, 1)

// .data
// evt_memcard_nosave
// evt_memcard_check
// evt_memcard_start
// evt_memcard_delete
// evt_memcard_copy
// evt_memcard_save
// unk_evt_803bac3c

}

}