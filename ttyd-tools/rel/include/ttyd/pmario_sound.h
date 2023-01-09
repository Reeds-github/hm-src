#pragma once

#include <cstdint>

namespace ttyd::pmario_sound {

struct PSndBgmEntry {
	int32_t field_0x0;
	int32_t field_0x4_streamId;
	uint8_t field_0x8[0xe - 0x8];
	uint8_t field_0xe_volume;
	uint8_t field_0xf[0x22 - 0xf];
	uint16_t field_0x22_fadeoutFrames;
	uint16_t field_0x24_remainingFadeoutFramesCounter;
	uint8_t field_0x26[0x38 - 0x26];
} __attribute__((__packed__));

static_assert(sizeof(PSndBgmEntry) == 0x38);

extern "C" {

// .text
// psndENV_LPF
// psndENVOff_f_d
// psndENVOff
// psndENVOn_f_d
// psndENVOn
// psndENVMain
// psndENV_stop
// psndSFX_get_vol
// psndSFXChk
// psndSFXOff
// psndSFX_dist
// psndSFX_pos
// psndSFX_pit
// psndSFX_vol
// psndSFXOnEx_3D
// psndSFXOn_3D
// psndSFXOnVol
// psndSFXOn
// psndSFXOn_
// __psndSFXOn
// psndSFXMain
// psndBGMPlayTime
// psndBGMStartCheck
// psndBGMScope
// psndBGMChkSilent
// psndBGMChk
bool psndBGMOff_f_d(int32_t flags, uint16_t fadetime, uint32_t unk_force);
// psndBGMOff
bool psndBGMOn_f_d(int32_t flags, const char* name, uint16_t fadetime1, uint16_t fadetime2);
// unk_800db778
// psndBGMOn
// psndBGMMain
// psndBGM_rate
// L_psndBGM_stop
// psndSFXAllOff
// psndMapChange
// psndGetFlag
// psndClearFlag
// psndSetFlag
// psndSetReverb
// psndStopAllFadeOut
// psndSetPosDirListener
// psndPushGroup
// psndExit
// psndMainInt
// psndMain
// psndInit
// searchPSSFXList
// calc3D
// angleABf

// .data
// psbgmlist
// pssfxlist
// psenvlist
// psenvlistname
// angleABTBL

// .sdata
// silent_tbl

// .bss
// _buf
// psnd
// psenv
// pssfx
extern PSndBgmEntry psbgm[2];

// .sbss
// init_f
// _psndPushGroup_trg
// group$4682
// tbl$4683
// PAN$4553

}

}