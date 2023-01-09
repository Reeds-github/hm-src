#include "mod.h"
#include "patch.h"

#include <gc/types.h>

#include <ttyd/animdrv.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/evtmgr.h>
#include <ttyd/filemgr.h>
#include <ttyd/item_data.h>
#include <ttyd/mapdata.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mario.h>
#include <ttyd/memory.h>
#include <ttyd/seqdrv.h>
#include <ttyd/system.h>

#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_window.h>

#include <hm/common.h>
#include <hm/skin_selector.h>

#include "common_types.h"
#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::item_data;

using evtmgr::EvtEntry;
using evtmgr_cmd::evtGetValue;
using filemgr::filemgr__File;
using mario_pouch::PouchData;
using seqdrv::SeqIndex;
using seqdrv::SeqInfo;

void (*g_marioSetFamicomMode_trampoline)(uint32_t) = nullptr;
void (*g_pouchReviseMarioParam_trampoline)() = nullptr;
void (*g_seqSetSeq_trampoline)(SeqIndex, char *, char *) = nullptr;

const uint32_t ORIG_SELECT_DESC_COUNT = 19;
const uint32_t SELECT_DESC_ENTRY_SIZE = 0x8;
const uint32_t SELECT_DESC_TBL_SIZE = SELECT_DESC_ENTRY_SIZE * ORIG_SELECT_DESC_COUNT;
const uint32_t SKINSEL_ENTRY_IDX = ORIG_SELECT_DESC_COUNT;
static const char* MSG_WHICH_SKIN = "Which skin?";
static const char* MSG_SKINS = "Skins";

// shhh. pretend you don't see it
#define _SKIN_PATH_R(s, ...) __VA_ARGS__ + 0 > 0 ? ("a/a_mario_" s "r") : ("a/a_mario_" s "_r")
#define SKIN_PATHS(s, ...) \
	{"a/a_mario_" s, _SKIN_PATH_R(s, __VA_ARGS__), "a/b_mario_" s, "a/e_mario_" s}

// i don't actually remember why i'm replacing the models as well as the textures,
// but surely there was a good reason when i initially wrote the code. surely.
constexpr static Skin SKINS[] = {
	{
		"Default", 422,
		"Default",
		{"a/a_mario", "a/a_mario_r", "a/b_mario", "a/e_mario"},
	}, {
		"Luigi Emblem", 533,
		"Cowardly but fearless brother!\nShow him some love! <icon HM 0.8 -4 5 0>",
		SKIN_PATHS("l", true),
	}, {
		"Wario Emblem", 534,
		"The infamous gold digger!\nDon't leave your coins out!",
		SKIN_PATHS("w", true),
	}, {
		"Waluigi Emblem", 535,
		"A weird but charming person.\nUse this emblem to flex on 'em.",
		SKIN_PATHS("wl", true),
	}, {
		"Fire Emblem", 536,
		"Is it me or has it gotten hotter?\nOh, it's literally just me.",
		SKIN_PATHS("fire"),
	}, {
		"Ice Emblem", 538,
		"Is it me or has it gotten colder?\nOh, it's literally just me.",
		SKIN_PATHS("ice"),
	}, {
		"Star Emblem", 541,
		"Space-like emblem that reminds\nyou of the galaxy.",
		SKIN_PATHS("red"),
	}, {
		"Admiral Emblem", 542,
		"A salty sailor emblem.",
		SKIN_PATHS("adm"),
	}, {
		"Bowser Emblem", 543,
		"An emblem with a rough design.",
		SKIN_PATHS("bow"),
	}, {
		"Toadette Emblem", 544,
		"The dominating queen herself.",
		SKIN_PATHS("toad"),
	}, {
		"Dia Emblem", 550,
		"hi",
		SKIN_PATHS("dia"),
	}, {
		"Jdaster Emblem", 552,
		"The man, the myth, the one who\ndocumented the battle system.",
		SKIN_PATHS("jda"),
	}, {
		"Aki Emblem", 553,
		"An aspiring Vtuber.",
		SKIN_PATHS("aki"),
	}, {
		"More to come!", 554,
		"More Emblems in the final game!",
		SKIN_PATHS("htl"),
	},
};

constexpr static uint32_t SKIN_COUNT = sizeof(SKINS) / sizeof(SKINS[0]);

// there are too many references to dotMarioPose to easily fix them all,
// so this is my workaround: editing the model file directly in the game.
void updateDotMarioTextureIndices() {
	uintptr_t animWorkPtr = (uintptr_t)animdrv::animGetPtr();
	uintptr_t animFilesPtr = *(uintptr_t *)animWorkPtr;
	uint32_t animFileCapacity = *(uint32_t *)(animWorkPtr + 0x4);

	for (uint32_t i = 0; i < animFileCapacity; i++) {
		uintptr_t animFile = animFilesPtr + (0x10 * i);
		uint32_t hasData = *(uint32_t *)animFile;
		if (hasData != 0) {
			filemgr__File* file = *(filemgr__File **)(animFile + 0x8);
			uintptr_t fileDataPtr = (uintptr_t)*file->mpFileData;

			char* filename = (char *)(fileDataPtr + 0x4);
			if (!strcmp(filename, "d_mario")) {
				// it might be enough to just adjust the sampler(s?), but this can't have
				// unintended consequences, so I'm keeping it, for my peace of mind
				const uint32_t texArrayOffset = 0x4F0;
				const uint32_t texEntrySize = 0x40;
				for (uint32_t j = 2; j <= 8; j++) {
					uintptr_t texEntryPtr = fileDataPtr + texArrayOffset + (texEntrySize * j);
					uint32_t* tplIndex = (uint32_t *)(texEntryPtr + 0x4);

					// 8-bit mario has 7 sprites per skin.
					// default mario's sprites start at tpl index 2,
					// while skin sprites start at index 35 and are then sequential
					// (i.e. luigi's sprites are directly followed by wario's)
					uint8_t curSkin = getHMSaveData()->selectedSkin;
					if (curSkin == 0) {
						*tplIndex = j;
					} else {
						*tplIndex = j + 33 + ((curSkin - 1) * 7);
					}
				}
			}
		}
	}
}

// patch file allocs on the fly to use the current skin
const char* skinselector_filealloc_hook(const char* path) {
	const uint8_t skinFileCount = 4;
	for (uint8_t i = 0; i < skinFileCount; i++) {
		if (!strcmp(path, SKINS[0].filePaths[i])) {
			path = SKINS[getHMSaveData()->selectedSkin].filePaths[i];
			break;
		}
	}

	return path;
}

static bool skinMenuIsOpen = false;
void skinselector_init() {
	// replace all calls to pouchEquipCheckBadge(kL/WEmblem) with `li r3,0`
	// this effectively disables the effect of the emblems
	*(uint32_t *)0x8005a84c = 0x38600000; // marioMain
	*(uint32_t *)0x8005a858 = 0x38600000;
	*(uint32_t *)0x8005da14 = 0x38600000; // marioSetCharMode
	*(uint32_t *)0x8005da20 = 0x38600000;
	*(uint32_t *)0x80145468 = 0x38600000; // _mario_super_emblem_anim_set
	*(uint32_t *)0x8014547c = 0x38600000;
	*(uint32_t *)0x801696ec = 0x38600000; // mario_change
	*(uint32_t *)0x801696f8 = 0x38600000;
	
	// replace select_desc_tbl with a larger one to add an entry for SKINSEL_ENTRY_IDX
	uintptr_t newSelectDescTbl = (uintptr_t)memory::__memAlloc(0, (SKINSEL_ENTRY_IDX + 1) * SELECT_DESC_ENTRY_SIZE);
	memcpy((void *)newSelectDescTbl, (void *)0x803b6174, SELECT_DESC_TBL_SIZE);

	// copy first entry to new one, because we use the same descs
	uintptr_t newEntryPtr = newSelectDescTbl + (SKINSEL_ENTRY_IDX * SELECT_DESC_ENTRY_SIZE);
	memcpy((void *)newEntryPtr, (void *)newSelectDescTbl, SELECT_DESC_ENTRY_SIZE);

	// fix tbl ptr load in winMgrSelectEntry
	*(uint16_t *)(0x8023d6b4 + 0x2) = newSelectDescTbl >> 16; // tbl ptr high half load data (lis)
	*(uint16_t *)(0x8023d6bc) = 0x6065; // tbl ptr low half load instr (addi -> ori)
	*(uint16_t *)(0x8023d6bc + 0x2) = newSelectDescTbl & 0xFFFF; // tbl ptr low half load data

	// add case for allocating the rowData when using our new entry type
	patch::hookInstruction(
		(void *)0x8023e414, // winMgrSelectEntry, first line of default:
		[](gc::os::OSContext* context, void* user) {
			if (context->gpr[26] == SKINSEL_ENTRY_IDX) {
				uint16_t* skinRowDataTable = (uint16_t *)memory::__memAlloc(0, SKIN_COUNT * 0x4);

				for (uint8_t i = 0; i < SKIN_COUNT; i++) {
					// add kHasBadgeEquipped flag for current skin
					if (i == getHMSaveData()->selectedSkin) {
						skinRowDataTable[i * 2] = 0x8;
					}

					skinRowDataTable[(i * 2) + 1] = i;
				}

				uintptr_t selEntry = context->gpr[31];
				*(void **)(selEntry + 0x30) = skinRowDataTable;
				*(int32_t *)(selEntry + 0x34) = SKIN_COUNT;
			}
		}
	);

	// directly set the current skin when the user selects one
	patch::hookInstruction(
		(void *)0x8023cfb0, // winMgrSelectOther, right after load of cur row value in r28 (before switch)
		[](gc::os::OSContext* context, void* user) {
			uintptr_t selEntry = context->gpr[27];
			uint32_t selectType = *(uint32_t *)(selEntry + 0x4);
			if (selectType == SKINSEL_ENTRY_IDX) {
				EvtEntry* evt = (EvtEntry *)context->gpr[31];
				evt->lwData[1] = *(uint16_t *)(context->gpr[4] + 0x2); // cursor row value (row index, essentially)
			}
		}
	);

	// fix the row names
	patch::hookInstruction(
		(void *)0x8023c078, // select_disp, right after the name msgSearch
		[](gc::os::OSContext* context, void* user) {
			uintptr_t selEntry = context->gpr[25];
			uint32_t selectType = *(uint32_t *)(selEntry + 0x4);

			if (selectType == SKINSEL_ENTRY_IDX) {
				uintptr_t rowData = *(uintptr_t *)(selEntry + 0x30);
				uintptr_t curRow = rowData + context->gpr[27];
				uint16_t curRowValue = *(uint16_t *)(curRow + 0x2);

				context->gpr[3] = (uint32_t)SKINS[curRowValue].name;
			}
		}
	);

	// fix the row icons
	patch::hookInstruction(
		(void *)0x8023bf68, // select_disp, after getting the icon id for the row
		[](gc::os::OSContext* context, void* user) {
			uintptr_t selEntry = context->gpr[25];
			uint32_t selectType = *(uint32_t *)(selEntry + 0x4);

			if (selectType == SKINSEL_ENTRY_IDX) {
				uintptr_t rowData = *(uintptr_t *)(selEntry + 0x30);
				uintptr_t curRow = rowData + context->gpr[27];
				uint16_t curRowValue = *(uint16_t *)(curRow + 0x2);

				context->gpr[3] = SKINS[curRowValue].icon;
			}
		}
	);

	// fix the text in the select menu's blue header
	patch::hookInstruction(
		(void *)0x8023c434, // select_disp, right after the msgSearch switch
		[](gc::os::OSContext* context, void* user) {
			uintptr_t selEntry = context->gpr[25];
			uint32_t selectType = *(uint32_t *)(selEntry + 0x4);

			if (selectType == SKINSEL_ENTRY_IDX) {
				context->gpr[22] = (uint32_t)MSG_SKINS;
			}
		}
	);
	
	// fix the text in the "Which X?" window to the left of the screen
	patch::hookInstruction(
		(void *)0x80239bfc, // select_disp2, right after the msgSearch switch
		[](gc::os::OSContext* context, void* user) {
			if (context->gpr[4] == SKINSEL_ENTRY_IDX) {
				context->gpr[29] = (uint32_t)MSG_WHICH_SKIN;
			}
		}
	);

	// fix the skin descriptions
	patch::hookInstruction(
		(void *)0x802399e0, // select_disp3
		[](gc::os::OSContext* context, void* user) {
			uintptr_t selEntry = *(uintptr_t *)(context->gpr[31] + 0x2c);
			uint32_t selectType = *(uint32_t *)(selEntry + 0x4);
			if (selectType == SKINSEL_ENTRY_IDX) {
				uint32_t cursorIndex = *(uint32_t *)(selEntry + 0xc);
				uint16_t* rowData = *(uint16_t **)(selEntry + 0x30);
				uint16_t curRowValue = rowData[(cursorIndex * 2) + 1];
				context->gpr[3] = (uint32_t)SKINS[curRowValue].description;
			}
		}
	);

	// handle skins for 8-bit mario
	g_marioSetFamicomMode_trampoline = patch::hookFunction(
		mario::marioSetFamicomMode, [](uint32_t param_1) {
			g_marioSetFamicomMode_trampoline(param_1);
			updateDotMarioTextureIndices();
		}
	);

	// pouchReviseMarioParam is called at most(all?) places at which the badge may get
	// unequipped, sold, or lost, so this should disable/reset the skin when necessary
	g_pouchReviseMarioParam_trampoline = patch::hookFunction(
		mario_pouch::pouchReviseMarioParam, []() {
			g_pouchReviseMarioParam_trampoline();

			if (getHMSaveData()->selectedSkin != 0 && mario_pouch::pouchEquipCheckBadge(ItemType::L_EMBLEM) == 0) {
				getHMSaveData()->selectedSkin = 0;
				mario::marioSetCharMode(0);
			}
		}
	);

	// if the player e.g. moves to the game over sequence with the menu open,
	// the menu event will end before it sets skinMenuIsOpen back to false.
	// this is a workaround, filtering kGame in case the current sequence is
	// set again while in the menu. this *might* leave a 1-frame window in
	// which the menu can be opened a second time, but I really don't care.
	// definitely not my fault if they fuck it up this bad
	g_seqSetSeq_trampoline = patch::hookFunction(
		seqdrv::seqSetSeq, [](SeqIndex seq, char* map, char* bero) {
			g_seqSetSeq_trampoline(seq, map, bero);

			if (seq != SeqIndex::kGame) {
				skinMenuIsOpen = false;
			}
		}
	);
}

void skinselector_loadexit_hook() {
	#ifdef DEBUG
	// DEBUG: give L emblem if missing
	if (mario_pouch::pouchCheckItem(ItemType::L_EMBLEM) == 0) {
		mario_pouch::pouchGetItem(ItemType::L_EMBLEM);
		mario_pouch::pouchEquipBadgeID(ItemType::L_EMBLEM);
	}
	#endif
}

EVT_DECLARE_USER_FUNC(evt_skinsel_set_skin, 1)
EVT_DEFINE_USER_FUNC(evt_skinsel_set_skin) {
	getHMSaveData()->selectedSkin = evtGetValue(evt, evt->currentCommandArguments[0]);
	return 2;
}

EVT_DECLARE_USER_FUNC(evt_skinsel_menu_block, 1)
EVT_DEFINE_USER_FUNC(evt_skinsel_menu_block) {
	skinMenuIsOpen = (bool)evtGetValue(evt, evt->currentCommandArguments[0]);
	return 2;
}

EVT_BEGIN(evt_open_skin_menu)
	USER_FUNC(evt_skinsel_menu_block, 1)
	USER_FUNC(evt_window::evt_win_other_select, SKINSEL_ENTRY_IDX)
	IF_EQUAL(LW(0), 1)
		USER_FUNC(evt_skinsel_set_skin, LW(1)) // LW(1) set from winMgrSelectOther
		USER_FUNC(evt_mario::evt_mario_set_mode, 0)
	END_IF()
	USER_FUNC(evt_skinsel_menu_block, 0)
	RETURN()
EVT_END()

void skinselector_update() {
	const uint32_t buttonCombo = ButtonId::Z | ButtonId::DPAD_UP;
	if (system::keyGetButton(0) == buttonCombo && system::keyGetButtonTrg(0) & buttonCombo && // button combo pressed?
		!skinMenuIsOpen && mario::marioChkKey() != 0 && mario::marioChkCtrl() != 0 && // allowed to open the menu?
		mario_pouch::pouchEquipCheckBadge(ItemType::L_EMBLEM) > 0) { // has L Emblem?
		evtmgr::evtEntry((void *)evt_open_skin_menu, 0, 0);
	}
}