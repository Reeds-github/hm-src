#pragma once

#include <gc/OSLink.h>

#include <ttyd/battle_database_common.h>
#include <ttyd/evtmgr.h>

using namespace ttyd;

struct HMWork {
	int32_t lastSavefileSequence;
	bool usedTattleThisTurn;
	bool hidePartnerHpInStatusWindow;
	uint8_t lastRandFirstStrikeMsgIx;
	battle_database_common::BattleWeapon* etherFlowerWeapon;

	// gameover resuming
	evtmgr::EvtEntry* askResumeEvt;
	bool resumingFromGameover;
};

// this feels absolutely terrible and certainly cannot be the solution,
// but I don't care enough to properly learn C++ or use classes
extern bool (*g_OSLink_trampoline)(gc::OSLink::OSModuleInfo *, void *);