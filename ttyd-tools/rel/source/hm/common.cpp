#include <ttyd/dvdmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/memory.h>

#include <hm/common.h>

using namespace ttyd;

bool isPointerValid(void* ptr) {
	const uintptr_t rawPtr = (uintptr_t)ptr;
	return rawPtr >= 0x80000000 && rawPtr <= 0x817FFFFF;
}

bool inBattleWithGroupSetup(battle_database_common::BattleGroupSetup* group_setup) {
	return battle::g_BattleWork != nullptr && battle::g_BattleWork->fbat_info->pNpcBattleInfo->pGroupSetup == group_setup;
}

void* readFileFromDvd(const char* path) {
	dvdmgr::DvdMgrFile* entry = dvdmgr::DVDMgrOpen(path, 2, 0);
	uint32_t length = *(uint32_t *)((uintptr_t)entry + 0x74);
	length = (length + 31) & ~31; // round up 32b
	void* buf = memory::__memAlloc(0, length);
	dvdmgr::DVDMgrRead(entry, buf, length, 0);
	dvdmgr::DVDMgrClose(entry);
	return buf;
}

// the evt system only reads args it needs, so we can just
// use the original arg count and let it skip over the extra ones
void replaceEvtInstr(int32_t* instr, uint16_t opc) {
	*(uint16_t *)((uintptr_t)instr + sizeof(uint16_t)) = opc;
}

// replaces the instruction and its args with nops
void writeEvtNop(int32_t* instr) {
	replaceEvtInstr(instr, 0x0072);
}

void writeEvtReturn(int32_t* instr) {
	replaceEvtInstr(instr, 0x0002);
}

// the following two require the replaced instruction to have at least 1 arg
void writeEvtGoto(int32_t* instr, const int32_t* destination) {
	replaceEvtInstr(instr, 0x0004);
	instr[1] = (uintptr_t)destination; // arg 1
}

void writeEvtBranch(int32_t* instr, const int32_t* destination, bool async) {
	replaceEvtInstr(instr, async ? 0x5C : 0x5E); // calls{a,s}
	instr[1] = (uintptr_t)destination; // arg 1
}

// okay, so, hear me out. yoshi's name field is 20 chars long,
// to support shift_jis (8 SJIS characters + null terminator padded to 4B).
// but we're modding the US version of the game, which doesn't support
// kanji in names, and the last three null bytes are useless. therefore,
// we can use the last 11* chars of the array for custom storage, to avoid
// having to create a whole second settings file like the practice codes do.
// *: not 12: the 9th one is reserved for the C-string null byte.
HMSaveData* getHMSaveData() {
	return (HMSaveData *)&mario_pouch::pouchGetPtr()->yoshi_name[9];
}

EVT_DEFINE_USER_FUNC(evt_hm_seq_get_bit) {
	evtmgr_cmd::evtSetValue(evt, evt->currentCommandArguments[1], getHMSaveData()->hmSeqBitfield & evtmgr_cmd::evtGetValue(evt, evt->currentCommandArguments[0]));
	return 2;
}

EVT_DEFINE_USER_FUNC(evt_hm_seq_set_bit) {
	uint8_t bit = evtmgr_cmd::evtGetValue(evt, evt->currentCommandArguments[0]);
	uint32_t isSet = evtmgr_cmd::evtGetValue(evt, evt->currentCommandArguments[1]);
	
	if (isSet != 0) {
		getHMSaveData()->hmSeqBitfield |= bit;
	} else {
		getHMSaveData()->hmSeqBitfield &= ~bit;
	}

	return 2;
}