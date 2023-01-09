#include "mod.h"
#include "patch.h"

#include <gc/types.h>
#include <gc/mtx.h>
#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_menu_disp.h>
#include <ttyd/battle_seq_end.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/cardmgr.h>
#include <ttyd/dispdrv.h>
#include <ttyd/dvdmgr.h>
#include <ttyd/eff_starstone.h>
#include <ttyd/event.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_memcard.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/evtmgr.h>
#include <ttyd/fadedrv.h>
#include <ttyd/filemgr.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mario.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sac_genki.h>
#include <ttyd/seq_battle.h>
#include <ttyd/seq_gameover.h>
#include <ttyd/seq_load.h>
#include <ttyd/seq_mapchange.h>
#include <ttyd/seq_title.h>
#include <ttyd/seqdrv.h>
#include <ttyd/statuswindow.h>
#include <ttyd/system.h>
#include <ttyd/unit_mario.h>
#include <ttyd/unit_party_christine.h>

#include <hm/blooper_rematch.h>
#include <hm/bugpatches.h>
#include <hm/common.h>
#include <hm/gon_09_ceilings.h>
#include <hm/heromode.h>
#include <hm/hookgloom_fakeout.h>
#include <hm/koopie_koo_battle.h>
#include <hm/larger_battles.h>
#include <hm/patches_evt.h>
#include <hm/room_objects.h>
#include <hm/skin_selector.h>

#include "common_types.h"
#include "evt_cmd.h"
#include "util.h"

#include <cstring>
#include <cstdio>

using namespace mod;
using namespace ttyd;
using namespace ttyd::battle_database_common;
using namespace ttyd::battle_unit;
using namespace ttyd::evtmgr_cmd;
using namespace ttyd::item_data;

namespace FadeType = fadedrv::FadeType;

using gc::OSLink::OSModuleInfo;
using battle::BattleWork;
using evtmgr::EvtEntry;
using filemgr::filemgr__File;
using mariost::g_MarioSt;
using seqdrv::SeqInfo;

struct CookingRecipe {
	char* ingredient1;
	char* ingredient2;
	char* result;
} __attribute__((__packed__));

static_assert(sizeof(CookingRecipe) == 0xC);

extern "C" {
	extern int32_t aji_aji_17_peach_evt_stg1[];
	extern int32_t dmo_dmo_00_demo_evt[];
	extern int32_t gon_gon_10_gon_10_init_evt[];
	extern int32_t gon_gon_11_gonbaba_event[];
	extern BattleUnitSetup gon_battle_database_gon_btlparty_gon_gon_10_01_off_1[];
	extern BattleGroupSetup gon_battle_database_gon_btlgrp_gon_gon_10_01_off_1;
	extern BattleSetupData gon_battle_database_gon_btlsetup_gon_tbl[];
	extern int32_t gon_unit_honenoko_attack_event[];
	extern int32_t gon_unit_red_honenoko_attack_event[];
	extern int32_t gor_gor_01_christine_nakama[];
	extern int32_t hei_hei_00_ki_data_evt_02[];
	extern uint32_t hei_hei_07_quiz_data[];
	extern BattleSetupData hei_battle_database_hei_btlsetup_hei_tbl[];
	extern int32_t tik_tik_01_gorotuki_sanninngumi[];
	extern int32_t tik_tik_02_geso_battle[];
	extern int32_t tik_tik_05_evt_mapwrite[];
	extern CookingRecipe gor_gor_01_R_cookbook_recipe_table[292];
}

const char* GON_10_NOP_BGM = "BGM_BOSS_STG8_KOOPA";
const char* ERROR_NO_DOUBLE_TATTLE = "btl_msg_cmderror_no_double_tattle";

// both arrays must have the same amount of lines
const char* FSTRIKE_SHITA_MSGS[] = {
	"You struck first!",
	"No holds barred!",
	"Lights out!",
	"They're outta here!",
	"Good Job!",
};
const char* FSTRIKE_SARETA_MSGS[] = {
	"Your foe struck first!",
	"Right in the kisser!",
	"Try jumping next time!",
	"That's gotta hurt!",
	"Too Bad!",
};
static_assert(sizeof(FSTRIKE_SHITA_MSGS) == sizeof(FSTRIKE_SARETA_MSGS));

static HMWork* hm;

EVT_DECLARE_USER_FUNC(status_gauge_force_close_clear, 0)
EVT_DEFINE_USER_FUNC(status_gauge_force_close_clear) {
	statuswindow::statusWinForceCloseClear();
	return 2;
}

EVT_DECLARE_USER_FUNC(toggle_partner_hp_display, 1)
EVT_DEFINE_USER_FUNC(toggle_partner_hp_display) {
	hm->hidePartnerHpInStatusWindow = !evt->currentCommandArguments[0];
	return 2;
}

EVT_DECLARE_USER_FUNC(toggle_ether_flower, 2)
EVT_DEFINE_USER_FUNC(toggle_ether_flower) {
	if (evtGetValue(evt, evt->currentCommandArguments[0]) != (uint32_t)hm->etherFlowerWeapon)
		return 2;

	auto ballIdLi = (uint8_t *)0x801be84c;
	auto flowerIdLi = (uint8_t *)0x801beb18;

	bool isEther = !!evt->currentCommandArguments[1];
	ballIdLi[3] = isEther ? 0x95 : 0x36;
	flowerIdLi[3] = isEther ? 0x96 : 0x37;

	gc::os::DCFlushRange(ballIdLi, sizeof(uint32_t));
	gc::os::DCFlushRange(flowerIdLi, sizeof(uint32_t));
	gc::os::ICInvalidateRange(ballIdLi, sizeof(uint32_t));
	gc::os::ICInvalidateRange(flowerIdLi, sizeof(uint32_t));

	return 2;
}


EVT_BEGIN(fire_ether_flower_init_patch)
	USER_FUNC(battle_event_cmd::btlevtcmd_CommandGetWeaponAddress, -2, LW(12)) // replacing orig
	USER_FUNC(toggle_ether_flower, LW(12), 1)
	RETURN()
EVT_END()

EVT_BEGIN(fire_ether_flower_exit_patch)
	RUN_CHILD_EVT(battle_item_data::ItemEvent_Fire_Flower_Core) // orig
	USER_FUNC(toggle_ether_flower, LW(12), 0)
	RETURN()
EVT_END()

EVT_BEGIN(gon_10_init_evt_bgm_hook)
	USER_FUNC(evt_snd::evt_snd_bgmon, 512, PTR("BGM_EVT_DANGER3"))
	RETURN()
EVT_END()

EVT_BEGIN(gor_01_christine_nakama_kiss_hook)
	INLINE_EVT()
		WAIT_MSEC(300)
		// orig
		USER_FUNC(evt_party::evt_party_move_pos2, 0, 25, 500, FLOAT(120.00))
		USER_FUNC(evt_party::evt_party_set_dir, 0, 270, 200)
	END_INLINE()

	USER_FUNC(toggle_partner_hp_display, 0)
	USER_FUNC(evt_sub::evt_sub_status_gauge_force_open)
	USER_FUNC(evt_mario::evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
	USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_STG8_MARIO_SHINE1"), LW(0), LW(1), LW(2), 0)
	USER_FUNC(evt_eff::evt_eff64, 0, PTR("broken_barrier_n64"), 2, LW(0), LW(1), LW(2), 1, 60, 0, 0, 0, 0, 0, 0)
	WAIT_MSEC(900)
	USER_FUNC(evt_eff::evt_eff, 0, PTR("stardust"), 4, LW(0), LW(1), LW(2), 30, 40, 12, 120, 0, 0, 0, 0)
	WAIT_MSEC(200)
	USER_FUNC(evt_eff::evt_eff, 0, PTR("stardust"), 4, LW(0), LW(1), LW(2), 30, 40, 12, 120, 0, 0, 0, 0)
	USER_FUNC(evt_pouch::evt_pouch_mario_recovery)
	DO(0)
		USER_FUNC(evt_sub::evt_sub_status_gauge_check_update, LW(0))
		IF_EQUAL(LW(0), 0)
			DO_BREAK()
		END_IF()
		WAIT_FRM(1)
	WHILE()
	WAIT_MSEC(500)
	USER_FUNC(evt_sub::evt_sub_status_gauge_force_close)
	INLINE_EVT()
		WAIT_MSEC(2000)
		USER_FUNC(toggle_partner_hp_display, 1)
		USER_FUNC(status_gauge_force_close_clear)
	END_INLINE()

	RETURN()
EVT_END()

EVT_BEGIN(gor_01_christine_nakama_nod_hook)
	WAIT_MSEC(500) // orig
	USER_FUNC(evt_mario::evt_mario_set_pose, PTR("M_B_1"))
	WAIT_FRM(6)
	USER_FUNC(evt_mario::evt_mario_set_pose, PTR("M_B_2"))
	WAIT_MSEC(1000)
	RETURN()
EVT_END()

EVT_BEGIN(gor_01_christine_nakama_somethingidk_hook)
	WAIT_MSEC(1000) // orig
	USER_FUNC(evt_mario::evt_mario_set_pose, PTR("M_I_2"))
	WAIT_MSEC(1000)
	RETURN()
EVT_END()

EVT_BEGIN(tik_tik_01_gorotuki_sanninngumi_hafta_hook)
	USER_FUNC(evt_msg::evt_msg_print, 0, PTR("mac_0_090"), 0, PTR("\x83\x4E\x83\x8A\x83\x7B\x81\x5B\x82\x50")) // orig ("クリボー１")
	USER_FUNC(evt_cam::evt_cam3d_evt_set, -140, 65, 310, -140, 33, 37, 0, 11)
	WAIT_MSEC(200)
	USER_FUNC(evt_msg::evt_msg_print_party, PTR("tik_01_hafta"))
	USER_FUNC(evt_cam::evt_cam3d_evt_set, 125, 148, 360, 125, 110, 37, 0, 11)
	WAIT_MSEC(200)
	RETURN()
EVT_END()

EVT_BEGIN(tik_02_geso_battle_intro_bgm_hook)
	WAIT_MSEC(1500)
	USER_FUNC(evt_snd::evt_snd_bgmon, 512, PTR("BGM_FF_MAIL_RECEPTION3"))
	RETURN()
EVT_END()

EVT_BEGIN(tik_05_evt_mapwrite_first_star_color_fix)
	WAIT_MSEC(500)

	SET(LW(2), GW(15))
	IF_EQUAL(LW(2), 0)
		SET(LW(2), 2)
	END_IF()

	RETURN()
EVT_END()

EVT_BEGIN(evt_gameover_resume)
	SET(GF(0), 0)
	USER_FUNC(evt_memcard::memcard_load)
	USER_FUNC(evt_memcard::memcard_code, LW(0))
	IF_EQUAL(LW(0), 0)
		USER_FUNC(evt_fade::evt_fade_end_wait)
		USER_FUNC(evt_msg::evt_msg_print, 0, PTR("msg_gameover_resume"), 0, 0)
		USER_FUNC(evt_msg::evt_msg_select, 0, PTR("msg_mcard_yesno"))
		USER_FUNC(evt_msg::evt_msg_continue)
		IF_EQUAL(LW(0), 0)
			SET(GF(0), 1)
		END_IF()
	END_IF()
	USER_FUNC(evt_fade::evt_fade_entry, FadeType::TITLESCREEN_CURTAIN_CLOSED_STATIC2, 0, 0, 0, 0) // prevent the black border flash
	RETURN()
EVT_END()

EVT_BEGIN(evt_gameover_resume_fadeout_end)
	USER_FUNC(evt_fade::evt_fade_end_wait)
	USER_FUNC(evt_fade::evt_fade_entry, FadeType::TITLESCREEN_CURTAIN_LEAVE_GRADUAL2, 0, 0, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(set_seq_56_hook)
	SET(GSW(0), 56) // original instr
	SET(GSWF(5677), 1) // for mobjs

	// set mario's base normal defense to 1
	GET_RAM(LW(0), unit_mario::mario_defence)
	ORI(LW(0), 1 << 24)
	SET_RAM(LW(0), unit_mario::mario_defence)
	RETURN()
EVT_END()

bool (*g_OSLink_trampoline)(OSModuleInfo *, void *) = nullptr;
bool (*g_OSUnlink_trampoline)(OSModuleInfo *) = nullptr;
void (*g_seq_mapChangeExit_trampoline)(SeqInfo *) = nullptr;
filemgr__File* (*g__fileAlloc_trampoline)(const char *, uint32_t) = nullptr;
const char* (*g_msgSearch_trampoline)(const char*) = nullptr;
void (*g_L_titleInit_trampoline)(void) = nullptr;
void (*g_stg0_00_init_trampoline)(void) = nullptr;
void (*g_pouchInit_trampoline)(void) = nullptr;
void (*g_seq_battleInit_trampoline)(SeqInfo *) = nullptr;
void (*g_loadDraw_trampoline)(void) = nullptr;
void (*g_seq_loadExit_trampoline)(SeqInfo *) = nullptr;
void (*g_seq_loadMain_trampoline)(SeqInfo *) = nullptr;
void (*g_fadeEntry_trampoline)(uint32_t, int32_t, gc::color4 *) = nullptr;
void (*g_statusWinDisp_trampoline)(void) = nullptr;
void (*g_seq_gameOverInit_trampoline)(SeqInfo *) = nullptr;
void (*g_BattleSetSeq_trampoline)(BattleWork *, uint32_t, uint32_t) = nullptr;
void (*g_BtlUnit_PayWeaponCost_trampoline)(BattleWorkUnit *, BattleWeapon *) = nullptr;
uint32_t (*g_BtlUnit_CheckWeaponCost_trampoline)(BattleWorkUnit *, BattleWeapon *) = nullptr;
void (*g_DrawSubIconSub_trampoline)(int32_t, int32_t, int32_t, int32_t, uint16_t, const char*, int32_t, int32_t, int32_t, int32_t) = nullptr;
BattleWorkUnit* (*g_BtlUnit_Entry_trampoline)(BattleUnitSetup *) = nullptr;
int32_t (*g_BattleCalculateDamage_trampoline)(BattleWorkUnit *, BattleWorkUnit *, BattleWorkUnitPart *, BattleWeapon *, uint32_t *, uint32_t) = nullptr;
int32_t (*g__get_flower_suitoru_point_trampoline)(EvtEntry *, bool) = nullptr;
int32_t (*g__get_heart_suitoru_point_trampoline)(EvtEntry *, bool) = nullptr;
void (*g_main_mario_trampoline)(void) = nullptr;

// allow to continue the game from the game over screen
void patch_resume_from_gameover() {
	// TITLESCREEN_CURTAIN_CLOSE2 seems to be the exact same as TITLESCREEN_CURTAIN_CLOSE,
	// except that's it on another layer that also happens to be above message windows.
	// we're patching seq_gameOverMain to use the latter instead, so that we can display messages.
	*(uint8_t *)0x800788cb = FadeType::TITLESCREEN_CURTAIN_CLOSE;

	// add a new state before the end of seq_gameover, to start the evt
	// asking whether to continue, then wait for it and all that
	patch::hookInstruction(
		(void *)0x80078998, // seq_gameover::seq_gameOverMain, if state > 6
		[](gc::os::OSContext* context, void* user) {
			auto seqInfo = (SeqInfo *)context->gpr[31];
			if (seqInfo->state == 7) {
				if (hm->askResumeEvt == nullptr) {
					hm->askResumeEvt = evtmgr::evtEntryType((void *)evt_gameover_resume, 0, 0, 0);
				} else if (!evtmgr::evtCheckID(hm->askResumeEvt->threadId)) { // if the evt has finished running
					// if the memcard loading hasn't crashed and the player has selected yes,
					// set the flag that will be checked to know whether to directly resume or not
					hm->resumingFromGameover = evtGetValue((EvtEntry *)0x0, GF(0));
					hm->askResumeEvt = nullptr;
					seqInfo->state = 6; // jump back to the last state of seq_gameOverMain
				}
			}
		}
	);

	// in seq_gameOverMain, jump from state 5 to 7, as the latter now handles going to 6
	*(uint8_t *)0x800788ef = 2;

	// jump to seq_load instead of seq_title if we're resuming
	patch::hookInstruction(
		(void *)0x80078990, // seq_gameover::seq_gameOverMain, right before the seqSetSeq(kTitle)
		[](gc::os::OSContext* context, void* user) {
			if (hm->resumingFromGameover) {
				context->gpr[3] = (uint32_t)seqdrv::SeqIndex::kLoad;
			}
		}
	);

	// skip all the gui init & textboxes when resuming
	g_seq_loadMain_trampoline = patch::hookFunction(
		seq_load::seq_loadMain, [](SeqInfo* info) {
			if (hm->resumingFromGameover) {
				info->state = 4;
				if (seq_load::seqLoadWp->state2 == 0) {
					seq_load::seqLoadWp->state2 = 0x17;
					seq_load::seqLoadWp->hoveredDtEntryIx = g_MarioSt->saveFileNumber; // resume the save we were using
				}
			}

			g_seq_loadMain_trampoline(info);
		}
	);

	// nop out the loading menu drawing code when resuming
	// (trying to draw while also skipping the texture init in seq_loadMain would crash)
	g_loadDraw_trampoline = patch::hookFunction(
		seq_load::loadDraw, []() {
			if (!hm->resumingFromGameover) {
				g_loadDraw_trampoline();
			}
		}
	);

	// fix the fadeout type when resuming (replacing CURTAIN_PULL_RIGHT2)
	patch::hookInstruction(
		(void *)0x800f3aa4, // seq_load::continueGame
		[](gc::os::OSContext* context, void* user) {
			if (hm->resumingFromGameover) {
				// using variant 2 to be on the layer over the black borders
				context->gpr[0] = FadeType::TITLESCREEN_CURTAIN_OPEN2;
			}
		}
	);

	g_fadeEntry_trampoline = patch::hookFunction(
		fadedrv::fadeEntry,
		[](uint32_t fadeType, int32_t duration, gc::color4* color) {
			g_fadeEntry_trampoline(fadeType, duration, color);

			if (hm->resumingFromGameover && fadeType == FadeType::TITLESCREEN_CURTAIN_OPEN2) {
				// when reaching the fadeout point of the resuming,
				// stop it and queue the second part of the curtain fadeout
				hm->resumingFromGameover = false;
				evtmgr::evtEntryType((void *)evt_gameover_resume_fadeout_end, 0, 0, 0);
			}
		}
	);
}

void tornado_jump_hp_cost_patch() {
	// i could probably fit this somewhere in a register inside DrawSubIconSub,
	// but i don't really feel like looking for one when this works fine
	static bool currentlyDrawingTornadoJump = false;

	// debit hp instead of fp on use
	// FIXME: make it countdown the second it's used rather than
	//        having it show up only after the end of the attack
	g_BtlUnit_PayWeaponCost_trampoline = patch::hookFunction(
		battle_unit::BtlUnit_PayWeaponCost,
		[](BattleWorkUnit* unit, BattleWeapon* weapon) {
			if (weapon == &battle_mario::badgeWeapon_TatsumakiJump) {
				// unit->current_hp -= weapon->base_fp_cost; // w/out damage star
				battle_damage::BattleDamageDirect(-5, unit, nullptr, weapon->base_fp_cost, 0, 0x128, 0, 1); // w/ damage star
    			battle_actrecord::BtlActRec_AddCount(&battle::g_BattleWork->act_record_work.mario_times_move_used);
				return;
			}

			return g_BtlUnit_PayWeaponCost_trampoline(unit, weapon);
		}
	);

	g_DrawSubIconSub_trampoline = patch::hookFunction(
		battle_menu_disp::DrawSubIconSub,
		[](int32_t unk1, int32_t unk2, int32_t unk3, int32_t unk4, uint16_t iconId, const char* name, int32_t unk7, int32_t unk8, int32_t unk9, int32_t unk10) {
			currentlyDrawingTornadoJump = iconId == 0x21;
			return g_DrawSubIconSub_trampoline(unk1, unk2, unk3, unk4, iconId, name, unk7, unk8, unk9, unk10);
		}
	);

	// display "HP" instead of "FP" in the attack menu
	patch::hookInstruction(
		(void *)0x801189f4, // battle_menu_disp::DrawSubIconSub
		[](gc::os::OSContext* context, void* user) {
			if (currentlyDrawingTornadoJump) {
				context->gpr[3] = (uint32_t)"HP";
			}
		}
	);
}

void replace_firepop_with_etherflower() {
	// replace Fire Pop with the Ether Flower
	auto etherFlower = &item_data::itemDataTable[ItemType::FIRE_POP];
	etherFlower->fp_restored = 10;

	hm->etherFlowerWeapon = (BattleWeapon *)memory::__memAlloc(0, sizeof(BattleWeapon));
	memcpy(hm->etherFlowerWeapon, item_data::itemDataTable[ItemType::FIRE_FLOWER].weapon_params, sizeof(BattleWeapon));
	etherFlower->weapon_params = hm->etherFlowerWeapon;
	etherFlower->weapon_params->item_id = ItemType::FIRE_POP;
	etherFlower->weapon_params->damage_function_params[0] = 5;
	etherFlower->weapon_params->element = WeaponElementType::EXPLOSIVE;
	etherFlower->weapon_params->damage_pattern = 0xd; // Inked
	etherFlower->weapon_params->burn_chance = 25;
	etherFlower->weapon_params->burn_time = 3;
	etherFlower->weapon_params->allergic_chance = 100;
	etherFlower->weapon_params->allergic_time = 4;

	// generalize the Fire Flower events
	writeEvtBranch(&battle_item_data::ItemEvent_Fire_Flower[17], fire_ether_flower_init_patch);
	writeEvtBranch(&battle_item_data::ItemEvent_Fire_Flower[27], fire_ether_flower_exit_patch);
	battle_item_data::ItemEvent_Fire_Flower_Core[119] = LW(12); // 8036652C
	battle_item_data::ItemEvent_Fire_Flower_Core[137] = LW(12); // 80366574
}

void swap_crystal_stars() {
	// this is, in fact, pretty disgusting. we're casting to BattleWeapon** so that
	// the SAT assignments don't complain (it would with void**), and it's not an issue
	// for color_tbl. it's not an issue for the float sets either, since a float is the
	// size of a pointer (on GC at least :p), so we can just multiply the index.
	// (we're using the size of edge_color since it's the biggest array we're handling)
	// I think using a temp working section might've been a bad call compared to
	// simply using a single temp var, but I'd argue it makes it more readable.
	// either way, overall this is a mess, but hey, it works!
	auto temp = (BattleWeapon**) memory::__memAlloc(0, sizeof(eff_starstone::edge_color));

	// swap the pointers in superActionTable (1-indexed because index 0 is Sweet Treat)
	memcpy(temp, battle_mario::superActionTable, sizeof(battle_mario::superActionTable));
	battle_mario::superActionTable[1] = temp[4]; // Diamond Star (ch1) = Ruby Star    (ch4)
	battle_mario::superActionTable[2] = temp[6]; // Emerald Star (ch2) = Garnet Star  (ch6)
	battle_mario::superActionTable[3] = temp[1]; // Gold Star    (ch3) = Diamond Star (ch1)
	battle_mario::superActionTable[4] = temp[2]; // Ruby Star    (ch4) = Emerald Star (ch2)
	battle_mario::superActionTable[6] = temp[3]; // Garnet Star  (ch6) = Gold Star    (ch3)

	// swap the icon ids in the BattleWeapons (reverses the tpl changes)
	int16_t icon1 = temp[1]->icon;
	int16_t icon2 = temp[2]->icon;
	temp[1]->icon = temp[3]->icon;
	temp[2]->icon = temp[4]->icon;
	temp[3]->icon = temp[6]->icon;
	temp[4]->icon = icon1;
	temp[6]->icon = icon2;

	// swap the pointers in color_tbl (back to 0-indexed)
	memcpy(temp, eff_starstone::color_tbl, sizeof(eff_starstone::color_tbl));
	eff_starstone::color_tbl[0] = temp[3];
	eff_starstone::color_tbl[1] = temp[5];
	eff_starstone::color_tbl[2] = temp[0];
	eff_starstone::color_tbl[3] = temp[1];
	eff_starstone::color_tbl[5] = temp[2];

	// swap the float sets in edge_color
	memcpy(temp, eff_starstone::edge_color, sizeof(eff_starstone::edge_color));
	const uint32_t setSize = 3 * sizeof(float);
	memcpy(&eff_starstone::edge_color[0], &temp[3 * 3], setSize);
	memcpy(&eff_starstone::edge_color[1], &temp[5 * 3], setSize);
	memcpy(&eff_starstone::edge_color[2], &temp[0 * 3], setSize);
	memcpy(&eff_starstone::edge_color[3], &temp[1 * 3], setSize);
	memcpy(&eff_starstone::edge_color[5], &temp[2 * 3], setSize);

	memory::__memFree(0, temp);

	// patch instructions in the sac_* star functions to undo the model changes in attacks
	// AFAIK those are the only places where the star model is shown depending on a user
	// selection, and therefore where there's an expectation of the star color matching
	// the selected attack. we can afford doing a blanket change for all star models in
	// the game, except for those ones (which, without this, get swapped around too much)
	*(uint8_t *)0x8022fb3f = 3; // sac_bakugame::main_star (Clock Out)
	*(uint16_t *)0x80232786 = 0x19f; // sac_scissor::star_disp2D_sub (Art Attack, 2D icon)
	*(uint8_t *)0x802328db = 0; // sac_scissor::star_control (Art Attack)
	*(uint8_t *)0x8024a4a7 = 2; // sac_deka::main_star (Earth Tremor)
	*(uint8_t *)0x8024dbb7 = 5; // sac_muki::main_star (Power Lift)
	*(uint8_t *)0x8024eea7 = 12; // sac_muki::main_muki (Power Lift, is +7)
	*(uint8_t *)0x802501a7 = 1; // sac_suki::main_star (Showstopper)

	// patch drawing code to make the diamond star the 3rd one
	// (the game has special behavior for it/index 0)
	*(uint8_t *)0x8020f0df = 2; // eff_starstone::effStarStoneDraw
	*(uint8_t *)0x8020feef = 2; // eff_starstone::effStarStoneDisp_1, x4
	*(uint8_t *)0x80210507 = 2;
	*(uint8_t *)0x80210587 = 2;
	*(uint8_t *)0x802107f7 = 2;
	*(uint8_t *)0x80211113 = 2; // eff_starstone::effStarStoneMain

	// if the player doesn't have the first star yet, show the diamond star
	// instead to hide the fact that the stars have been swapped around
	patch::hookInstruction(
		(void *)0x8016e674, // win_mario::winMarioDisp, after `li r3,0x19f`
		[](gc::os::OSContext* context, void* user) {
			if ((mario_pouch::pouchGetPtr()->star_powers_obtained & 2) == 0) {
				context->gpr[3] = 0x1a2;
			}
		}
	);

	// same thing, show the diamond star instead of the ruby star on the 2D map
	patch::hookInstruction(
		(void *)0x80173434, // win_log::mapGX, `li r26,0x19f`
		[](gc::os::OSContext* context, void* user) {
			if ((mario_pouch::pouchGetPtr()->star_powers_obtained & 2) == 0) {
				context->gpr[26] = 0x1a2;
				context->srr0 += 4;
			}
		}
	);

	// additional code for the 3D map is in the `tik` case in the OSLink trampoline
}

// https://github.com/jdaster64/ttyd-infinite-pit/blob/9fd2df7/ttyd-tools/rel/source/patches_item.cpp#L161-L187
int32_t GetDrainRestoration(EvtEntry* evt, bool hp_drain) {
	int32_t id = battle_sub::BattleTransID(evt, evt->currentCommandArguments[0]);
	auto unit = battle::BattleGetUnitPtr(battle::g_BattleWork, id);

	int32_t drain = 0;
	if (unit) {
		int32_t num_badges = hp_drain ? unit->badges_equipped.hp_drain : unit->badges_equipped.fp_drain;
		drain = unit->total_damage_dealt_this_attack * num_badges;
		if (drain > 5) drain = 5;
	}

	evtSetValue(evt, evt->currentCommandArguments[1], drain);
	return 2;
}

// Patches the HP/FP Drain badges so that they have the PM64 behavior
// (implementation from Jdaster64's Infinite Pit mod)
void drain64_patch() {
	// https://github.com/jdaster64/ttyd-infinite-pit/blob/9fd2df7/ttyd-tools/rel/source/patches_item.cpp#L530-L534
	*(uint32_t *)0x800fe058 = 0x60000000;

	// https://github.com/jdaster64/ttyd-infinite-pit/blob/21228a1/ttyd-tools/rel/source/patches_enemy.cpp#L135-L136
	g_BattleCalculateDamage_trampoline = patch::hookFunction(
		battle_damage::BattleCalculateDamage,
		[](BattleWorkUnit* attacker, BattleWorkUnit* target, BattleWorkUnitPart* target_part, BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1) {
			if (weapon->damage_function)
				attacker->total_damage_dealt_this_attack++;

			return g_BattleCalculateDamage_trampoline(attacker, target, target_part, weapon, unk0, unk1);
		}
	);

	// https://github.com/jdaster64/ttyd-infinite-pit/blob/9fd2df7/ttyd-tools/rel/source/patches_item.cpp#L517-L529
	g__get_heart_suitoru_point_trampoline = patch::hookFunction(
        battle_event_default::_get_heart_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, true);
        }
	);

    g__get_flower_suitoru_point_trampoline = patch::hookFunction(
        battle_event_default::_get_flower_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, false);
        }
	);
}

uint32_t findScreenshotIxFromSequence(uint32_t gsw0) {
	const uint32_t sequenceSteps[] = {
		3,	// After the boat arrives in Rogueport
		7,	// After the cutscene after Goombella joining Mario
		33,	// After Koops joining Mario (Before Koopie Koo arriving)
		62,	// After the cutscene of Mario and Koops preparing to leave
		87, // After the cutscene of Flurrie joining Mario
	};

	for (uint32_t i = sizeof(sequenceSteps) / sizeof(uint32_t); i > 0; i--) {
		if (gsw0 >= sequenceSteps[i - 1]) {
			return i - 1;
		}
	}

	return -2;
}

MOD_INIT_FUNCTION() {
	hm = (HMWork *)memory::__memAlloc(0, sizeof(HMWork));
	memset(hm, 0, sizeof(HMWork));
	hm->lastSavefileSequence = -1;

	// force-enable the game's debug mode
	*(uint32_t *)0x80009b2c = 0x3800ffff; // li r0,-1

	// update the build-time strings to use the rel build time
	strcpy(system::R_build_date, __DATE__);
	strcpy(system::R_build_time, __TIME__);

	#ifdef DEBUG
	// always show the build date on the title screen
    *(uint32_t *)0x80008fe8 = 0x60000000; // nop

	// display tattle hp bar for all enemies
	*(uint32_t *)0x8012ee64 = 0x60000000; // nop
	#else
	// change savefile name if in release mode (prevent reusing normal saves)
	strcpy(cardmgr::str_mariost_save_file_802cb0e0, "heromode_save_v03");

	// replace the icon&banner textures used for the savefile
	// we store these on disk to save on memory (the file is 7.5KB)
	// (they're stored consecutively in memory, hence the offset being
	// icon_tex's. the order is icon_tex -> icon_tlut -> banner_tex)
	dvdmgr::DvdMgrFile* gfxEntry = dvdmgr::DVDMgrOpen("/mod/savedata_gfx.bin", 2, 0);
	dvdmgr::DVDMgrRead(gfxEntry, &cardmgr::_mariost_icon_tex, 0x1e00, 0);
	dvdmgr::DVDMgrClose(gfxEntry);
	#endif

	// patch Pity Flower's recovery chance to be 100%
	// (from Jdaster64's Infinite Pit mod)
	*(uint32_t *)0x800fe500 = 0x2c030064;

	// change the rank up level thresholds
	battle_seq_end::_rank_up_data[1].level = 20;
	battle_seq_end::_rank_up_data[2].level = 40;
	battle_seq_end::_rank_up_data[3].level = 50;

	patch_resume_from_gameover();
	tornado_jump_hp_cost_patch();
	replace_firepop_with_etherflower();
	swap_crystal_stars();
	drain64_patch();

	bugpatches_init();
	patches_evt_init();
	kkb_init();
	robjects_init();
	skinselector_init();

	// adjust the starting stats, unless the savefile name is "Level 0"
	g_pouchInit_trampoline = patch::hookFunction(
		mario_pouch::pouchInit, []() {
			g_pouchInit_trampoline();
			if (!strcmp(g_MarioSt->saveFileName, "Level 0")) return;

			auto pouchPtr = mario_pouch::pouchGetPtr();
			pouchPtr->max_fp = 0;
			pouchPtr->base_max_fp = 0;
			pouchPtr->unallocated_bp = 0;
			pouchPtr->total_bp = 0;
			pouchPtr->current_fp = pouchPtr->max_fp;
		}
	);

	g_stg0_00_init_trampoline = patch::hookFunction(
		event::stg0_00_init, []() {
			g_stg0_00_init_trampoline();
			if (!strcmp(g_MarioSt->saveFileName, "Level 0")) return;

			mario_pouch::pouchSetCoin(50);
		}
	);

	g_seq_loadExit_trampoline = patch::hookFunction(
		seq_load::seq_loadExit, [](SeqInfo* info) {
			g_seq_loadExit_trampoline(info);

			skinselector_loadexit_hook();

			if (!strcmp(g_MarioSt->saveFileName, "Level 0")) {
				for (uint32_t i = 0; i < 0x153; i++) {
					if (item_data::itemDataTable[i].bp_cost > 0) {
						item_data::itemDataTable[i].bp_cost = 1;
					}
				}

				mario_pouch::pouchReviseMarioParam();
			}

			unit_mario::mario_defence[0] = g_MarioSt->gsw0 >= 56;
		}
	);

	// loading memcard to know which savefile was loaded last
	cardmgr::cardBufReset();
	cardmgr::cardReadAll();

	// I think it might be possible to entirely drop this hook and
	// find the index in the L_titleInit hook, because the memcard
	// buffer will always be full at that point (including the first
	// time, because we ran cardReadAll)? You'd have to check codes
	// and stuff, and it'd be pretty nice but right now I'm tired so
	g_seq_gameOverInit_trampoline = patch::hookFunction(
		seq_gameover::seq_gameOverInit, [](SeqInfo* info) {
			hm->lastSavefileSequence = findScreenshotIxFromSequence(g_MarioSt->gsw0);
			g_seq_gameOverInit_trampoline(info);
		}
	);

	// patching the title screen to show the image we want based on
	// the story sequence of the last savefile that was saved to
	g_L_titleInit_trampoline = patch::hookFunction(
		seq_title::L_titleInit, []() {
			g_L_titleInit_trampoline();

			if (hm->lastSavefileSequence >= 0) {
				filemgr__File* mariostTplFile = *seq_title::seqTitleWorkPointer2;
				uintptr_t mariostTplDataPtr = (uintptr_t)*mariostTplFile->mpFileData;

				// XXX: this might change if mariost gets edited too much
				void* titlescreenTexPtr = (void *)(mariostTplDataPtr + 0x6D2A0);

				dvdmgr::DvdMgrFile* titlescreensEntry = dvdmgr::DVDMgrOpen("/mod/titlescreens.bin", 2, 0);
				dvdmgr::DVDMgrRead(titlescreensEntry, titlescreenTexPtr, 0x11D000, 0x11D000 * hm->lastSavefileSequence);
				dvdmgr::DVDMgrClose(titlescreensEntry);
			}
		}
	);

	g_OSLink_trampoline = patch::hookFunction(
		gc::OSLink::OSLink,
		[](OSModuleInfo* new_module, void* bss) {
			bool result = g_OSLink_trampoline(new_module, bss);
			if (new_module != nullptr && result) {
				battles_oslink_hook(new_module);
				br_oslink_hook(new_module);
				ceilings_oslink_hook(new_module);
				hgf_oslink_hook(new_module);
				kkb_oslink_hook(new_module);
				robjects_oslink_hook(new_module);

				switch (new_module->id) {
					case ModuleId::AJI: {
						// skip bowser intermission after ch1
						aji_aji_17_peach_evt_stg1[1067] = 61; // set gsw0 from after bowser intermission (805F3500)
						aji_aji_17_peach_evt_stg1[1102] = 3; // change gsw4 value, used in seq_load::continueGame (805F358C)
						aji_aji_17_peach_evt_stg1[1107] = 0; // set mario to mario, not bowser (805F35A0)
						aji_aji_17_peach_evt_stg1[1127] = PTR("nok_01"); // change fade-to map name (805F35F0)
						aji_aji_17_peach_evt_stg1[1128] = PTR("nokotarou");
					} break;
					case ModuleId::DMO:
						// in the intro cutscene, skip to the titlescreen after the book fadeout
						// works by threading the zoom-in code, then jumping to the titlescreen
						// fade code directly. somewhat hacky, but it works.
						// (works fine without the setii and i'm not even sure why it's set in
						// the buttonCheck evt, but it's straightforward to add and i'm not taking risks)
						dmo_dmo_00_demo_evt[708] = 0x0000006B; // begin_thread (805BD884)
						dmo_dmo_00_demo_evt[709] = 0x00020032; // setii, 2 args [setii GF(10) 1]
						dmo_dmo_00_demo_evt[710] = GF(10); // setii arg 1
						dmo_dmo_00_demo_evt[711] = 1; // setii arg 2
						dmo_dmo_00_demo_evt[712] = assembleEvtNop(1);
						dmo_dmo_00_demo_evt[755] = 0x0000006D; // end_thread (805BD940)
						dmo_dmo_00_demo_evt[756] = 0x00010004; // goto, 1 arg
						dmo_dmo_00_demo_evt[757] = PTR(&dmo_dmo_00_demo_evt[1463] /*805BE448*/);
						dmo_dmo_00_demo_evt[758] = assembleEvtNop(3);
						break;
					case ModuleId::GON: {
						// if in release mode, prevent fleeing in the banzai bill battle
						#ifndef DEBUG
						gon_battle_database_gon_btlsetup_gon_tbl[17].battle_setup_flags |= 0x10;
						#endif

						// set the bgm for all three battles in gon_10
						// TODO: use mapdata for those eventually
						gon_battle_database_gon_btlsetup_gon_tbl[17].music_name = GON_10_NOP_BGM;
						gon_battle_database_gon_btlsetup_gon_tbl[18].music_name = GON_10_NOP_BGM;
						gon_battle_database_gon_btlsetup_gon_tbl[19].music_name = GON_10_NOP_BGM;

						// hook to a custom event in gon_10's init event to set the BGM
						writeEvtBranch(&gon_gon_10_gon_10_init_evt[7], gon_10_init_evt_bgm_hook);

						// when reaching seq 56/obtaining the diamond star, set gswf 5677 and add the defense point
						writeEvtBranch(&gon_gon_11_gonbaba_event[1386], set_seq_56_hook); // 805CF16C

						// disable call_friend_event by making the rng roll for the normal attack always succeed
						gon_unit_honenoko_attack_event[59] = 200; // 805D781C
						gon_unit_red_honenoko_attack_event[59] = 200; // 805D9DF4

						// patch the banzai bill's init event imported from another rel to change its forced spawn height
						const int32_t super_killer_height = 50;
						int32_t* super_killer_init = gon_battle_database_gon_btlgrp_gon_gon_10_01_off_1.enemy_data[3].unit_kind_params->init_evt_code;
						super_killer_init[29] = super_killer_height; // 806202B4
						super_killer_init[35] = super_killer_height; // 806202CC
						} break;
					case ModuleId::GOR:
						writeEvtBranch(&gor_gor_01_christine_nakama[196], gor_01_christine_nakama_kiss_hook); // 805F2108
						writeEvtNop(&gor_gor_01_christine_nakama[202]); // 805F2120
						writeEvtBranch(&gor_gor_01_christine_nakama[226], gor_01_christine_nakama_nod_hook); // 805F2180
						writeEvtBranch(&gor_gor_01_christine_nakama[428], gor_01_christine_nakama_somethingidk_hook); // 805F24A8

						// auto-generated
						gor_gor_01_christine_nakama[427] = PTR("M_I_N"); // 805F24A4
						gor_gor_01_christine_nakama[460] = PTR("M_J_1B"); // 805F2528
						gor_gor_01_christine_nakama[480] = PTR("SFX_VOICE_MARIO_FLD_JUMP1"); // 805F2578
						gor_gor_01_christine_nakama[495] = 0; // 805F25B4

						// edit the recipes for the Ether Flower (replacing Fire Pop/ファイアキャンディ)
						gor_gor_01_R_cookbook_recipe_table[0x8c].ingredient1 = (char *)0x805d2988; // replace Cake Mix with Coconut Bomb (ココナッツボム)
						memcpy(&gor_gor_01_R_cookbook_recipe_table[0xa2], &gor_gor_01_R_cookbook_recipe_table[0x8c], sizeof(CookingRecipe)); // erase the other recipe by duplicating the good one
						break;
					case ModuleId::HEI: {
						// replace the Mushroom in the tree with a coin
						hei_hei_00_ki_data_evt_02[10] = ItemType::COIN; // 805C6820

						// yes, those battles already use that song originally. don't question it.
						for (uint8_t i = 15; i <= 23; i++) {
							if (i == 19) continue;
							hei_battle_database_hei_btlsetup_hei_tbl[i].music_name = "BGM_ZAKO_BATTLE1";
						}

						// replace quiz answer indices
						const uint32_t quizAnswers[] = {0, 3, 1, 2, 1, 0, 3};
						for (uint8_t i = 0; i < 7; i++) {
							hei_hei_07_quiz_data[(4 * i) + 3] = quizAnswers[i];
						}
						} break;
					case ModuleId::TIK:
						writeEvtBranch(&tik_tik_01_gorotuki_sanninngumi[210], tik_tik_01_gorotuki_sanninngumi_hafta_hook); // 805CE7C4

						// add some audio to the intro of both blooper fights and delay the curtain drop
						writeEvtBranch(&tik_tik_02_geso_battle[268], tik_02_geso_battle_intro_bgm_hook); // 805D05C8
						tik_tik_02_geso_battle[354] = 14; // 805D071C

						// make the ruby star (1st) on the 3d map appear as the diamond map
						writeEvtBranch(&tik_tik_05_evt_mapwrite[1027], tik_05_evt_mapwrite_first_star_color_fix);
						tik_tik_05_evt_mapwrite[1033] = LW(2);
						break;
				}
			}

			return result;
		}
	);

	g_OSUnlink_trampoline = patch::hookFunction(
		gc::OSLink::OSUnlink,
		[](OSModuleInfo* module) {
			battles_osunlink_hook(module);
			ceilings_osunlink_hook(module);
			hgf_osunlink_hook(module);
			kkb_osunlink_hook(module);

			return g_OSUnlink_trampoline(module);
		}
	);

	// force reload mario model in aaa_00 to make pajamas work in the intro
	g_seq_mapChangeExit_trampoline = patch::hookFunction(
		seq_mapchange::seq_mapChangeExit, [](SeqInfo* seqInfo) {
			g_seq_mapChangeExit_trampoline(seqInfo);

			if (!strcmp(g_MarioSt->currentMapName, "aaa_00")) {
				mario::marioSetCharMode(0);
			}
		}
	);

	g__fileAlloc_trampoline = patch::hookFunction(
		filemgr::_fileAlloc, [](const char* path, uint32_t unk2) {
			if (isPointerValid((void *)path)) {
				static char buf[23]; // current largest possible string is "a/MOBJ_SaveBlock_pyra-" (22 + 1)
				const char* suffix = nullptr;
				if (!strcmp(path, "a/MOBJ_SaveBlock-")) {
					const char* sbType = nullptr;
					if (g_MarioSt->gsw0 >= 25 && system::irand(100) == 0) {
						sbType = "pyra";
					} else {
						switch (g_MarioSt->pMapAlloc->id) {
							case ModuleId::GON:
							case ModuleId::HEI:
							case ModuleId::MRI:
							case ModuleId::NOK:
							case ModuleId::WIN:
								sbType = g_MarioSt->currentAreaName;
								break;
						}
					}

					if (sbType != nullptr) {
						sprintf(buf, "a/MOBJ_SaveBlock_%s-", sbType);
						path = buf;
					}
				} else if (!strcmp(g_MarioSt->currentMapName, "gon_10") && !strcmp(path, "a/c_kilr_g-")) {
					// set the alternate texture for the banzai bill
					path = "a/c_kilr_bg-";
				} else if (!strcmp(g_MarioSt->currentMapName, "aaa_00") && (!strcmp(path, "a/a_mario-") || !strcmp(path, "a/a_mario_r-") || !strcmp(path, "a/c_luigi-"))) {
					// pajamas pog
					suffix = "pjs";
				} else if (!strcmp(g_MarioSt->currentMapName, "gon_06") && (!strcmp(path, "a/OFF_d_kusya-") || !strcmp(path, "a/c_p_majin-"))) {
					// alt black chest
					suffix = "2";
				} else if (g_MarioSt->gsw0 >= 56 && (!strcmp(path, "a/c_kinopio-") || !strcmp(path, "a/c_kokinopio-") || !strcmp(path, "a/c_nokonoko-") || !strcmp(path, "a/c_nokorin-"))) {
					// uncurse colors after diamond star
					suffix = "56";
				} else {
					path = br_filealloc_hook(path);
					path = kkb_filealloc_hook(path);
					path = skinselector_filealloc_hook(path);
				}

				if (suffix != nullptr) {
					strcpy(buf, path);
					strcat(buf, suffix);
					path = buf;
				}
			}

			return g__fileAlloc_trampoline(path, unk2);
		}
	);

	g_msgSearch_trampoline = patch::hookFunction(
		msgdrv::msgSearch, [](const char* msgKey) {
			if (!strcmp(msgKey, ERROR_NO_DOUBLE_TATTLE)) {
				return "Cannot tattle twice\nwithin the same turn!";
			}

			const char* msg;
			msg = br_msgsearch_hook(msgKey);
			if (msg != nullptr)
				return msg;

			msg = kkb_msgsearch_hook(msgKey);
			if (msg != nullptr)
				return msg;

			if (!strcmp(msgKey, "fb_sensei_shita")) {
				return FSTRIKE_SHITA_MSGS[hm->lastRandFirstStrikeMsgIx];
			} else if (!strcmp(msgKey, "fb_sensei_sareta")) {
				return FSTRIKE_SARETA_MSGS[hm->lastRandFirstStrikeMsgIx];
			} else if (!strcmp(msgKey, "msg_gameover_resume")) {
				return "<system><p>Would you like to continue\nfrom your previous save file?<o>";
			} else if (!strcmp(msgKey, "tik_01_hafta_kur")) {
				return "What?<wait 500> Cat got your tongue?\n<k>";
			} else if (g_MarioSt->pMapAlloc->id == ModuleId::GON) {
				// set name&tattle for alt enemies in gon
				if (!strcmp(msgKey, "btl_un_togekuri")) {
					return "C. Spikey Bigloomba";
				} else if (!strcmp(msgKey, "btl_hlp_togekuri")) {
					return "That's a C. Spiky Bigloomba!\n"
							"<wait 500><col c00000ff>Climax</col> variants of enemies\n"
							"are generally overtuned.\n"
							"<k>\n<p>\n"
							"It's literally just a Gloomba,\n"
							"<wait 300>but bigger,<wait 300> nothing of note.\n"
							"<wait 500>Here's the stat-page!\n"
							"<k>\n<p>\n"
							"<speed 0>\n"
							"<pos -25 0 0>*\n"
							"<scale 0.85>\n"
							"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 05\n"
							"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 006\n"
							"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 00\n"
							"</scale>\n"
							"<pos 450 0 0>*\n"
							"<pos 0 35 0><scale 0.85>Headbonk <col c00000ff>Attack Power</col>: 05\n"
							"<pos 0 55 0><scale 0.8><col 40BF9F>Superguardable</col>: No\n"
							"</speed></scale></pos><dkey><wait 500></dkey>\n"
							"<k>";
				} else if (!strcmp(msgKey, "btl_un_kuriboo")) {
					return "Climax Gloomba";
				} else if (!strcmp(msgKey, "btl_hlp_kuriboo")) {
					return "That's a Climax Gloomba.\n"
							"<wait 500><col c00000ff>Climax</col> variants of enemies\n"
							"are generally overtuned.\n"
							"<k>\n<p>\n"
							"It may just look like your\n"
							"usual Gloomba,<wait 500> but it's not.\n"
							"<wait 500>Here's the stat-page!\n"
							"<k>\n<p>\n"
							"<speed 0>\n"
							"<pos -25 0 0>*\n"
							"<scale 0.85>\n"
							"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 05\n"
							"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 008\n"
							"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 00\n"
							"</scale>\n"
							"<pos 450 0 0>*\n"
							"<pos 0 24 0><scale 0.85>Headbonk <col c00000ff>Attack Power</col>: 04\n"
							"<pos 0 48 0><scale 0.85>Goom-up <col c00000ff>Attack Buff</col>: +03 ATK\n"
							"<pos 0 69 0><scale 0.8><col 40BF9F>Superguardable</col>: Yes\n"
							"</speed></scale></pos><dkey><wait 500></dkey>\n"
							"<k>";
				} else if (!strcmp(msgKey, "btl_un_patakuri")) {
					return "C. Paragloomba";
				} else if (!strcmp(msgKey, "btl_hlp_patakuri")) {
					return "That's a Climax Paragloomba!\n"
							"<wait 500><col c00000ff>Climax</col> variants of enemies\n"
							"are generally overtuned.\n"
							"<k>\n<p>\n"
							"It has the same advantages\n"
							"and disadvantages seen before.\n"
							"Here's its stat-page!\n"
							"<k>\n<p>\n"
							"<speed 0>\n"
							"<pos -25 0 0>*\n"
							"<scale 0.85>\n"
							"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 05\n"
							"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 008\n"
							"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 00\n"
							"</scale>\n"
							"<pos 450 0 0>*\n"
							"<pos 0 24 0><scale 0.85>Airstrike <col c00000ff>Attack Power</col>: 04\n"
							"<pos 0 48 0><scale 0.85>Goom-up <col c00000ff>Attack Buff</col>: +03 ATK\n"
							"<pos 0 69 0><scale 0.8><col 40BF9F>Superguardable</col>: Yes\n"
							"</speed></scale></pos><dkey><wait 500></dkey>\n"
							"<k>";
				} else if (!strcmp(msgKey, "menu_monosiri_las") || !strcmp(msgKey, "menu_monosiri_dou") || !strcmp(msgKey, "menu_monosiri_eki") || !strcmp(msgKey, "menu_monosiri_jin") || !strcmp(msgKey, "menu_monosiri_tou")) {
					return "Hooktail Castle";
				} else if (inBattleWithGroupSetup(&gon_battle_database_gon_btlgrp_gon_gon_10_01_off_1)) {
					// set name&tattle for banzai bill
					if (!strcmp(msgKey, "btl_un_super_killer")) {
						return "Banzai Bill";
					} else if (!strcmp(msgKey, "btl_hlp_super_killer")) {
						return "That's Banzai Bill!\n"
								"<wait 250>\n"
								"That thing is...humongous!\n"
								"<k>\n<p>\n"
								"It appears to have a time\n"
								"limit for when it can hit us.\n"
								"<wait 400>But once it does,<wait 300> it's gonezo.\n"
								"<k>\n<p>\n"
								"But...don't let it do that,<wait 300> OK?\n"
								"when that timer's done,<wait 400> one of\n"
								"us is guaranteed to flatline!\n"
								"<k>\n<p>\n"
								"<speed 0>\n"
								"<pos -25 0 0>*\n"
								"<scale 0.85>\n"
								"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 06\n"
								"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 010\n"
								"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 03\n"
								"</scale>\n"
								"<pos 450 0 0>*\n"
								"<pos 0 35 0><scale 0.85>Bullet Crush <col c00000ff>Attack Power</col>: 07\n"
								"<pos 0 55 0><scale 0.8><col 40BF9F>Superguardable</col>: No\n"
								"</speed></scale></pos><dkey><wait 500></dkey>\n"
								"<k>";
					}
				}
			}

			return g_msgSearch_trampoline(msgKey);
		}
	);

	g_BtlUnit_Entry_trampoline = patch::hookFunction(
		ttyd::battle_unit::BtlUnit_Entry, [](BattleUnitSetup *unit_setup) {
			kkb_unitentry_hook(unit_setup);

			auto* kind = unit_setup->unit_kind_params;
			if (kind->unit_type <= BattleUnitType::BONETAIL) {
				// make all enemies invulnerable to Gale Force
				kind->status_vulnerability->gale_force = 0;

				// prevent enemies from dropping experience in Level 0 mode
				if (!strcmp(g_MarioSt->saveFileName, "Level 0")) {
					kind->level = 0;
				}
			}

			auto result = g_BtlUnit_Entry_trampoline(unit_setup);

			// set the scale for spiky goombas if we're in gon
			if (kind->unit_type == BattleUnitType::SPIKY_GOOMBA && g_MarioSt->pMapAlloc->id == ModuleId::GON) {
				result->base_scale.x = 1.5;
				result->base_scale.y = 1.5;
				result->base_scale.z = 1.5;
			} else if (inBattleWithGroupSetup(&gon_battle_database_gon_btlgrp_gon_gon_10_01_off_1) && kind->unit_type == BattleUnitType::BOMBSHELL_BILL) {
				result->base_scale.x = 3.5;
				result->base_scale.y = 3.5;
				result->base_scale.z = 3.5;
				result->slow_turns = 3;
			}

			return result;
		});

	// pick a random first strike message once per battle, so that it doesn't change on each msgSearch call
	g_seq_battleInit_trampoline = patch::hookFunction(
		seq_battle::seq_battleInit, [](SeqInfo* info) {
			hm->lastRandFirstStrikeMsgIx = system::irand(sizeof(FSTRIKE_SHITA_MSGS) / sizeof(const char*));

			g_seq_battleInit_trampoline(info);
		}
	);

	// skip head-holding anim in sweet treat after hitting a poison shroom
	*(uint16_t *)0x802481c2 = 0xe94; // replace M_X_1 with M_I_Y in the condition handling counter == 0x1e
	g_main_mario_trampoline = patch::hookFunction(
		sac_genki::main_mario, []() {
			g_main_mario_trampoline();

			auto extraWorkAddr = (uintptr_t)&battle::g_BattleWork->ac_manager_work + 0x2bc;
			auto genkiMarioState = (uint32_t *)(extraWorkAddr + 0x44);
			auto genkiMarioCounter = (uint32_t *)(extraWorkAddr + 0x48);

			if (*genkiMarioState == 4 && *genkiMarioCounter == 0x1e) {
				*genkiMarioState = 1;
				*genkiMarioCounter = 0;
			}
		}
	);

	// in sweet treat, make poison shrooms remove 1 from all gained stats
	patch::hookInstruction(
		(void *)0x80247c98, // sac_genki::main_weapon, on poison shroom hit
		[](gc::os::OSContext* context, void* user) {
			uintptr_t extraWorkAddr = context->gpr[30];
			auto fpCounter = (int32_t *)(extraWorkAddr + 0x10);
			auto marioHpCounter = (int32_t *)(extraWorkAddr + 0x14);
			auto partyHpCounter = (int32_t *)(extraWorkAddr + 0x18);

			if (*fpCounter > 0) (*fpCounter)--;
			if (*marioHpCounter > 0) (*marioHpCounter)--;
			if (*partyHpCounter > 0) (*partyHpCounter)--;
		}
	);

	// add hook to disable the partner hp display during the goombella kiss
	patch::hookInstruction(
		(void *)0x8013cf1c, // statuswindow::statusWinDisp, `extsh. r0,r4` for partner check
		[](gc::os::OSContext* context, void* user) {
			if (hm->hidePartnerHpInStatusWindow) {
				context->gpr[4] = 0;
			}
		}
	);

	// patch pmario_sound stuff to allow the BGM in gon_10 to continue without interruption
	// when going into and coming out of a battle (based on similar preexisting conditions)
	patch::hookInstruction(
		(void *)0x800db21c, // pmario_sound::psndBGMOn_f_d
		[](gc::os::OSContext* context, void* user) {
			if (user == nullptr) {
				// branch to the end of the function
				patch::writeBranch((void *)(context->srr0 + 0xc), (void *)0x800db764);
			}

			if (!strcmp(g_MarioSt->currentMapName, "gon_10")) {
				auto param_2 = context->gpr[24];

				bool ret0 = false;
				if (param_2 == 0x8c) { // id for BGM_BOSS_STG8_KOOPA
					ret0 = true;
				} else if (pmario_sound::psbgm[0].field_0x0 == 0xa3) { // id for BGM_EVT_DANGER3
					auto param_1 = context->gpr[30];
					if (param_2 == 0x90 || param_2 == 0x91 || (param_2 == 0x0 && (param_1 & 0x20) == 0)) {
						ret0 = true;
					} else if (param_2 == 0x92) {
						pmario_sound::psndBGMOff_f_d(0x400, 0x2ee, 1);
					}
				}

				if (ret0) {
					context->gpr[3] = 0;
					context->srr0 += 0xc;
				}
			}
		}
	);

	patch::hookInstruction(
		(void *)0x800dac58, // pmario_sound::psndBGMOff_f_d
		[](gc::os::OSContext* context, void* user) {
			if (user == nullptr) {
				// branch to the end of the function
				patch::writeBranch((void *)(context->srr0 + 0xc), (void *)0x800daee8);
			}

			if (pmario_sound::psbgm[0].field_0x0 == 0xa3 // id for BGM_EVT_DANGER3
				&& !strcmp(g_MarioSt->currentMapName, "gon_10")) {
					auto prm1bgm = (pmario_sound::PSndBgmEntry *)context->gpr[31];
					if (prm1bgm->field_0x0 != 0x92) {
						// return 0
						context->gpr[3] = 0;
						context->srr0 += 0xc;
					} else {
						pmario_sound::psndBGMOn_f_d(0x120, nullptr, 0x2ee, 0x2ee);
					}
			}
		}
	);

	patch::hookInstruction(
		(void *)0x800db7b0, // pmario_sound::psndBGMOn
		[](gc::os::OSContext* context, void* user) {
			if (context->gpr[3] == 0x120
				&& context->gpr[4] == 0x0
				&& pmario_sound::psbgm[0].field_0x0 == 0xa3 // id for BGM_EVT_DANGER3
				&& !strcmp(g_MarioSt->currentMapName, "gon_10")) {
					// disable weird fading when getting out of battle
					context->gpr[5] = 0;
					context->gpr[6] = 0;
			}
		}
	);

	// Displays the Star Power in 0.01 units numerically below the status window.
	// (from Jdaster64's Infinite Pit mod)
	g_statusWinDisp_trampoline = patch::hookFunction(
        statuswindow::statusWinDisp, []() {
            g_statusWinDisp_trampoline();

			// Don't display SP if no Star Powers have been unlocked yet.
			if (mario_pouch::pouchGetMaxAP() <= 0) return;

			// Don't try to display SP if the status bar is not on-screen.
			float menu_height = *reinterpret_cast<float*>(
				reinterpret_cast<uintptr_t>(statuswindow::g_StatusWindowWork)
				+ 0x24);
			if (menu_height < 100.f || menu_height > 330.f) return;

			gc::mat3x4 matrix;
			int32_t unknown_param = -1;
			int32_t current_AP = mario_pouch::pouchGetAP();
			gc::mtx::PSMTXTrans(&matrix, 192.f, menu_height - 100.f, 0.f);
			icondrv::iconNumberDispGx(
				&matrix, current_AP, /* is_small = */ 1, &unknown_param);
        });


	// make tattle not use a turn and set it as used when appropriate
	patch::hookInstruction(
		(void *)0x80120568, // battle_seq_command::_btlcmd_SetAttackEvent
		[](gc::os::OSContext* context, void* user) {
			BattleWeapon* weapon = (BattleWeapon *)context->gpr[3];
			if (weapon == &unit_party_christine::partyWeapon_ChristineMonosiri) {
				BattleWorkUnit* unit = (BattleWorkUnit *)context->gpr[28];
				unit->move_state = 3;
				hm->usedTattleThisTurn = true;
			}
		}
	);

	// re-enable tattle when a turn ends or a battle starts
	g_BattleSetSeq_trampoline = patch::hookFunction(
		battle::BattleSetSeq,
		[](BattleWork* battleWork, uint32_t seqLevel, uint32_t seqState) {
			if ((seqLevel == 3 && seqState == 0x3000001) || (seqLevel == 0 && seqState == 1)) {
				hm->usedTattleThisTurn = false;
			}

			g_BattleSetSeq_trampoline(battleWork, seqLevel, seqState);
		}
	);

	// disable tattle in the menu if it's already been used this turn
	// also check hp instead of fp for tornado jump
	g_BtlUnit_CheckWeaponCost_trampoline = patch::hookFunction(
		battle_unit::BtlUnit_CheckWeaponCost,
		[](BattleWorkUnit* unit, BattleWeapon* weapon) {
			if (weapon == &unit_party_christine::partyWeapon_ChristineMonosiri) {
				return (uint32_t)!hm->usedTattleThisTurn;
			} else if (weapon == &battle_mario::badgeWeapon_TatsumakiJump) {
				return (uint32_t)(unit->current_hp > weapon->base_fp_cost);
			}

			return g_BtlUnit_CheckWeaponCost_trampoline(unit, weapon);
		}
	);

	// fix the error message when you try using tattle while it's disabled
	// (this way is fine only because tattle is never disabled usually:
	// with a move that uses FP, this would incorrectly display our message
	// even if the player is genuinely out of FP)
	patch::hookInstruction(
		(void *)0x80123e78, // battle_seq_command::_btlcmd_UpdateSelectWeaponTable
		[](gc::os::OSContext* context, void* user) {
			BattleWeapon* weapon = (BattleWeapon *)context->gpr[5];
			if (weapon == &unit_party_christine::partyWeapon_ChristineMonosiri) {
				context->gpr[0] = 7; // mUnk_0x18 / disabled message type
			}
		}
	);

	patch::hookInstruction(
		(void *)0x8011f508, // battle_seq_command::_get_msg
		[](gc::os::OSContext* context, void* user) {
			if (context->gpr[0] == 7) {
				context->gpr[5] = (uint32_t)ERROR_NO_DOUBLE_TATTLE;
				context->srr0 += 0x4; // skip fallback
			}
		}
	);
}

MOD_UPDATE_FUNCTION() {
	skinselector_update();

	// as soon as the memcard is loaded, find the correct sequence index
	if (hm->lastSavefileSequence == -1 && !cardmgr::cardIsExec()) {
		hm->lastSavefileSequence = -2;

		uint32_t code = cardmgr::cardGetCode();
		if (code == 0) {
			uintptr_t cardPtr = (uintptr_t)cardmgr::cardGetFilePtr();
			uint16_t lastSavefileIndex = *(uint16_t *)(cardPtr + 0x1e44);
			uintptr_t lastSavefilePtr = cardPtr + 0x2000 + lastSavefileIndex * 0x4000;
			uint32_t gsw0 = *(uint32_t *)(lastSavefilePtr + 0x8 + 0x174);

			hm->lastSavefileSequence = findScreenshotIxFromSequence(gsw0);
		}
	}

	#ifdef DEBUG
	if (seqdrv::seqGetSeq() == seqdrv::SeqIndex::kGame) {
		// draw mario's coordinates on screen
		// (taken from the practice codes, thanks Zephiles)
		dispdrv::dispEntry(
			dispdrv::CameraId::kDebug3d, 2, 0.f,
			[](dispdrv::CameraId layerId, void *user) {
				auto* player = mario::marioGetPtr();

				char marioPosStrBuffer[36];
				sprintf(
					marioPosStrBuffer, "Pos: %.2f  %.2f  %.2f",
					player->playerPosition[0], player->playerPosition[1], player->playerPosition[2]
				);

				uint32_t color = 0xFFFFFFFF;
				float scale = 0.65f;
				gc::mat3x4 mtx;
				gc::mtx::PSMTXScale(&mtx, scale, scale, scale);
				gc::mtx::PSMTXTransApply(&mtx, &mtx, -232, -160, 0);

				fontmgr::FontDrawStart_alpha(0xFF);
				fontmgr::FontDrawEdge();
				fontmgr::FontDrawColor((uint8_t *)(&color));
				fontmgr::FontDrawMessageMtx(&mtx, marioPosStrBuffer);
			}, nullptr);

		// levitate, also from the practice codes
		uint32_t combo = ButtonId::L | ButtonId::A;
		if ((system::keyGetButton(0) & combo) == combo) {
			mario::marioGetPtr()->wJumpVelocityY = 4.540000915527344f;
		}
	}
	#endif
}