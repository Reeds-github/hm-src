#pragma once

#include "seqdrv.h"

#include <cstdint>

namespace ttyd::seq_gameover {

extern "C" {

void seq_gameOverInit(seqdrv::SeqInfo *info);
void seq_gameOverExit(seqdrv::SeqInfo *info);
void seq_gameOverMain(seqdrv::SeqInfo *info);

}

}