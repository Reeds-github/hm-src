#pragma once

#include "seqdrv.h"

#include <cstdint>

namespace ttyd::seq_load {

struct SeqLoadWork {
	uint16_t flags;
	uint8_t unk_0x2[0x18 - 0x2];
	uint32_t state2;
	uint32_t unk_0x1c; // stores a card code
	uint32_t hoveredDtEntryIx;
	uint8_t unk_0x24[0x4c - 0x24];
} __attribute__((__packed__));

static_assert(sizeof(SeqLoadWork) == 0x4c);

extern "C" {

void loadDraw();
void seq_loadInit(seqdrv::SeqInfo *info);
void seq_loadExit(seqdrv::SeqInfo *info);
void seq_loadMain(seqdrv::SeqInfo *info);

extern SeqLoadWork* seqLoadWp;

}

}