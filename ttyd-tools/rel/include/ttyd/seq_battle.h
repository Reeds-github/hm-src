#pragma once

#include "seqdrv.h"

#include <cstdint>

namespace ttyd::seq_battle {

extern "C" {

// battle_exit
// battle_init
void seq_battleMain(seqdrv::SeqInfo *info);
void seq_battleExit(seqdrv::SeqInfo *info);
void seq_battleInit(seqdrv::SeqInfo *info);

}

}