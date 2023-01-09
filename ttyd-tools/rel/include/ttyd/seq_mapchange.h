#pragma once

#include <ttyd/seqdrv.h>

#include <cstdint>

namespace ttyd::seq_mapchange {

extern "C" {

// .text
// _relUnLoad
// _load
// _unload
// seq_mapChangeMain
void seq_mapChangeExit(seqdrv::SeqInfo* seqInfo);
// seq_mapChangeInit

// .data
// dat_ptrarr_80304388

// .sdata
// key_off
// dir_str$719

// .bss
// rel_bss

// .sbss
// _next_bero
// _next_map
// _next_area

// .sbss2
// unk_80429500

}

}