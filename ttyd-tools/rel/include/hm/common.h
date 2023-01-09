#pragma once

#include <ttyd/battle.h>
#include <ttyd/evtmgr.h>
#include <ttyd/npcdrv.h>

#include <cstdint>

// could we actually move all of this to GSW(F)s? might be the more sensible choice lmao
struct HMSaveData {
	uint8_t pad[9];
	// 0x1 - Blooper rematch
	uint8_t hmSeqBitfield;
	uint8_t selectedSkin;
} __attribute__((__packed__));

static_assert(sizeof(HMSaveData) == 11);

extern "C" {

bool isPointerValid(void* ptr);

}

bool inBattleWithGroupSetup(ttyd::battle_database_common::BattleGroupSetup* group_setup);
void* readFileFromDvd(const char* path);

constexpr uint32_t assembleEvtNop(uint16_t nopCount) {
	return 0x00000072 | ((nopCount - 1) << 16);
}

void writeEvtNop(int32_t* instr);
void writeEvtReturn(int32_t* instr);
void writeEvtGoto(int32_t* instr, const int32_t* destination);
void writeEvtBranch(int32_t* instr, const int32_t* destination, bool async = false);

HMSaveData* getHMSaveData();
EVT_DECLARE_USER_FUNC(evt_hm_seq_get_bit, 2)
EVT_DECLARE_USER_FUNC(evt_hm_seq_set_bit, 2)