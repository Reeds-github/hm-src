#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_detect.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_unit.h>
#include <ttyd/dvdmgr.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evtmgr.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/npcdrv.h>

#include <hm/common.h>
#include <hm/heromode.h>
#include <hm/koopie_koo_battle.h>

#include "common_types.h"
#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::battle_unit;
using namespace ttyd::battle_database_common;

using gc::OSLink::OSModuleInfo;
using ttyd::mariost::g_MarioSt;
using ttyd::npcdrv::NpcEntry;

extern "C" {
	extern int32_t hei_hei_01_evt_nokorin[];

	extern BattleStageData hei_battle_database_hei_hei_btlstage_data;
	extern BattleSetupData hei_battle_database_hei_btlsetup_hei_tbl[];

	extern BattleUnitKind hei_unit_gold_chorobon_unit_gold_chorobon;
	extern int32_t hei_unit_gold_chorobon_damage_event[];

	EVT_DECLARE_USER_FUNC(tou2_tou_03__firstattack_setting, 0)

	extern BattleUnitKind tou2_unit_ura_pata_unit_ura_pata;
	extern int32_t tou2_unit_ura_pata_init_event_patapata[];
	extern int32_t tou2_unit_ura_pata_attack_event_patapata[];
	extern BattleWeapon tou2_unit_ura_pata_weapon_patapata_attack;
	extern int8_t tou2_unit_ura_pata_defence_turn[5];
	extern int32_t tou2_unit_ura_pata_attack_event[];
	extern int32_t tou2_unit_ura_pata_ura_attack_event[];
	extern BattleWeapon tou2_unit_ura_pata_weapon_ura_noko_ura_attack;
	extern BattleWeapon tou2_unit_ura_pata_weapon_nokonoko_attack;
}

// SJIS npc names
const char* KK_NPC_NAME = "\x83\x6d\x83\x52\x83\x8a\x83\x93"; // ノコリン
const char* GOLDFZ_NPC_NAME = "\x83\x53\x81\x5b\x83\x8b\x83\x68\x83\x60\x83\x87\x83\x8d\x83\x7b\x83\x93"; // ゴールドチョロボン

EVT_BEGIN(evt_nokorin_hook_setpos_gold_fuzzy)
	USER_FUNC(evt_npc::evt_npc_set_position, PTR(KK_NPC_NAME), 700, 0, 0) // orig
	USER_FUNC(evt_npc::evt_npc_set_position, PTR(GOLDFZ_NPC_NAME), 740, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(evt_nokorin_hook_move_gold_fuzzy)
	USER_FUNC(evt_npc::evt_npc_move_position, PTR(KK_NPC_NAME), 550, 0, 0, FLOAT(80.f), 1) // orig
	WAIT_MSEC(300)

	// bring in the gold fuzzy
	USER_FUNC(evt_npc::evt_npc_set_walk_anim, PTR(GOLDFZ_NPC_NAME))
	USER_FUNC(evt_npc::evt_npc_jump_position_nohit, PTR(GOLDFZ_NPC_NAME), 590, 0, 0, 550, 40)
	USER_FUNC(evt_npc::evt_npc_set_stay_anim, PTR(GOLDFZ_NPC_NAME))
	USER_FUNC(evt_npc::evt_npc_get_position, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2))
	USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_BOSS_G_CHORO_LANDING1"), LW(0), LW(1), LW(2), 0)
	RETURN()
EVT_END()

EVT_BEGIN(evt_nokorin_hook_flag_on_mario_hit)
	USER_FUNC(evt_mario::evt_mario_set_pose, PTR("M_D_1")) // orig
	SET(GF(16), 1)
	RETURN()
EVT_END()

EVT_BEGIN(evt_nokorin_hook_start_battle)
	INLINE_EVT()
		USER_FUNC(evt_npc::evt_npc_move_position, PTR(KK_NPC_NAME), 200, 0, 0, FLOAT(450.f), 1) // orig
	END_INLINE()
	INLINE_EVT()
		// jump onto mario
		USER_FUNC(evt_npc::evt_npc_get_position, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2))
		USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_BOSS_G_CHORO_ATTACK1"), LW(0), LW(1), LW(2), 0)
		USER_FUNC(evt_mario::evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
		SUB(LW(0), 10)
		USER_FUNC(evt_npc::evt_npc_jump_position_nohit, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2), 280, 40)
	END_INLINE()

	// wait until mario gets hit
	DO(0)
		IF_EQUAL(GF(16), 1)
			DO_BREAK()
		END_IF()
		WAIT_FRM(1)
	WHILE()
	SET(GF(16), 0)

	// wait until mario faces right to start the battle
	// if this is missing, koops will be on the wrong side when the battle is over
	// could be replaced with a manual set_dispdir?
	WAIT_FRM(2)

	// start the battle as as first strike
	USER_FUNC(evt_npc::evt_npc_set_attack_mode, PTR(KK_NPC_NAME), 0x10)
	USER_FUNC(tou2_tou_03__firstattack_setting)
	USER_FUNC(evt_npc::evt_npc_set_battle_info, PTR(KK_NPC_NAME), -1)
	USER_FUNC(evt_npc::evt_npc_battle_start, PTR(KK_NPC_NAME))
	USER_FUNC(evt_npc::evt_npc_wait_battle_end)

	// force the party member back to koops in case it was changed during the battle
	USER_FUNC(evt_mario::evt_mario_get_party, LW(0))
	IF_NOT_EQUAL(LW(0), 2)
		USER_FUNC(evt_mario::evt_mario_kill_party, 0)
		USER_FUNC(evt_mario::evt_mario_set_party, 0, 2)
	END_IF()

	// fix koops not having the correct animation when getting out of the battle
	USER_FUNC(evt_party::evt_party_stop, 0) // what the fuck is runMode and why do I need to change it
	USER_FUNC(evt_party::evt_party_set_pose, 0, PTR("PNK_D_1")) // doesn't work but i'm over it. at least the next hit works

	// set gold fuzzy idle position
	USER_FUNC(evt_mario::evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
	ADD(LW(0), 40)
	USER_FUNC(evt_npc::evt_npc_set_position, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2))
	USER_FUNC(evt_npc::evt_npc_set_anim, PTR(GOLDFZ_NPC_NAME), PTR("CBN_N_1"))

	INLINE_EVT()
		// wait until koopie koo hits the fuzzy
		DO(0)
			USER_FUNC(evt_npc::evt_npc_get_position, PTR(KK_NPC_NAME), LW(0), LW(1), LW(2))
			USER_FUNC(evt_npc::evt_npc_get_position, PTR(GOLDFZ_NPC_NAME), LW(3), LW(4), LW(5))
			IF_LARGE_EQUAL(LW(0), LW(3))
				DO_BREAK()
			END_IF()
			WAIT_FRM(1)
		WHILE()

		// kick the fuzzy into the camera
		SET(GF(16), 1)
		USER_FUNC(evt_cam::evt_cam_get_pos, 4, LW(0), LW(1), LW(2))
		ADD(LW(2), 30)
		USER_FUNC(evt_npc::evt_npc_set_anim, PTR(GOLDFZ_NPC_NAME), PTR("CBN_Q_1"))
		USER_FUNC(evt_npc::evt_npc_jump_position_nohit, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2), 200, 0)
	END_INLINE()

	// handle sfx when the gold fuzzy gets hit
	INLINE_EVT()
		DO(0)
			IF_EQUAL(GF(16), 1)
				DO_BREAK()
			END_IF()
			WAIT_FRM(1)
		WHILE()

		USER_FUNC(evt_npc::evt_npc_get_position, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2))
		USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_BOSS_G_CHORO_DAMAGED1"), LW(0), LW(1), LW(2), LW(3))
		DO(0)
			USER_FUNC(evt_snd::evt_snd_sfxchk, LW(3), LW(0))
			IF_EQUAL(LW(0), -1)
				DO_BREAK()
			END_IF()
			USER_FUNC(evt_npc::evt_npc_get_position, PTR(GOLDFZ_NPC_NAME), LW(0), LW(1), LW(2))
			USER_FUNC(evt_snd::evt_snd_sfx_pos, LW(3), LW(0), LW(1), LW(2))
			WAIT_FRM(1)
		WHILE()

		// hide the fuzzy when we're done
		WAIT_MSEC(200)
		USER_FUNC(evt_npc::evt_npc_set_position, PTR(GOLDFZ_NPC_NAME), FLOAT(0), FLOAT(-1000), FLOAT(0))
	END_INLINE()

	RETURN()
EVT_END()

EVT_BEGIN(gold_chorobon_damage_event_replacement)
	SET(LW(10), -2)
	SET(LW(11), 1)
	RUN_CHILD_EVT(&battle_event_default::btldefaultevt_Damage)
	RETURN()
EVT_END()

EVT_BEGIN(koopie_koo_entry_event)
	// wait until the attack starts, then wait until it ends
	WAIT_FRM(40)
	USER_FUNC(battle_event_cmd::btlevtcmd_WaitAttackEnd)

	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 1)
	USER_FUNC(battle_camera::evt_btl_camera_wait_move_end)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 1, 3)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveto, 1, 0, 130, 335, -2, 93, 25, 20, 0)
	WAIT_MSEC(400)
	USER_FUNC(battle_event_cmd::btlevtcmd_StatusWindowOnOff, 0)

	USER_FUNC(evt_msg::evt_msg_print, 2, PTR("kkb_entry"), 0, -2)

	USER_FUNC(battle_event_cmd::btlevtcmd_StartWaitEvent, -2)
	USER_FUNC(battle_event_cmd::btlevtcmd_StatusWindowOnOff, 1)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 0)
	RETURN()
EVT_END()

EVT_BEGIN(ura_pata_init_event_hook_add_setevententry)
	USER_FUNC(battle_event_cmd::btlevtcmd_SetEventEntry, -2, PTR(koopie_koo_entry_event))
	USER_FUNC(battle_event_cmd::btlevtcmd_StartWaitEvent, -2) // orig
	RETURN()
EVT_END()

EVT_BEGIN(ura_attack_event_camera_hook)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 0, 0)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveSpeedLv, 0, 1)
	USER_FUNC(battle_camera::evt_btl_camera_set_zoom, 0, 200)
	RETURN()
EVT_END()

void (*g_BattleGetFirstAttackUnit_trampoline)(battle::BattleWork *, BattleWorkUnit **, BattleWorkUnit **);

OSModuleInfo* kkbRel = nullptr;
void* kkbRelBss = nullptr;
BattleSetupData* kkbBSD = nullptr;

bool inKKB() {
	return kkbBSD != nullptr && inBattleWithGroupSetup(kkbBSD->flag_off_loadouts->group_data);
}

void kkb_init() {
	patch::hookInstruction(
		(void *)0x80043da4, // npcdrv::npcSetBattleInfo, if (battleInfoId == -1) {
		[](gc::os::OSContext* context, void* user) {
			NpcEntry* npc = (NpcEntry *)context->gpr[31];

			// if this is Koopie Koo, use our custom battle
			if (!strcmp(npc->name, KK_NPC_NAME)) {
				context->gpr[4] = (uint32_t)kkbBSD;
				context->srr0 += 4; // skip `li r4,0`
			}
		}
	);

	// force Koopie Koo to be the one to first strike, regardless of her position
	g_BattleGetFirstAttackUnit_trampoline = patch::hookFunction(
		battle_detect::BattleGetFirstAttackUnit,
		[](battle::BattleWork* battleWork, BattleWorkUnit** unit1, BattleWorkUnit** unit2) {
			g_BattleGetFirstAttackUnit_trampoline(battleWork, unit1, unit2);

			if (inKKB()) {
				// Koopie Koo is guaranteed to be at ix 4, because Mario will always have Koops on his team
				// (therefore the unit index cannot change depending on whether Mario has a partner or not)
				*unit1 = battleWork->battle_units[4];
			}
		}
	);

	// enemies that first strike usually don't play their entry events,
	// so here we're forcing koopie koo to run it anyway
	patch::hookInstruction(
		(void *)0x8011e850, // battle_seq::btlseqFirstAct, `cmpwi r31,0x7`
		[](gc::os::OSContext* context, void* user) {
			// r31 is battleWork->mFbatInformation->wFirstAttack
			if (context->gpr[31] == 0x10 && inKKB()) {
				auto unitWork = battle::g_BattleWork->battle_units[4];
				auto evtEntry = evtmgr::evtEntry(unitWork->entry_evt_code, 10, 0);
				unitWork->entry_evt_tid = evtEntry->threadId;
				evtEntry->wActorThisPtr = unitWork->unit_id;
			}
		}
	);
}

// when loading hei.rel and if the player has not met Koopie Koo yet,
// allocate our custom battle setup, load our custom rel, and patch the event
void kkb_oslink_hook(OSModuleInfo* new_module) {
	if (new_module->id == ModuleId::HEI && g_MarioSt->gsw0 < 34) {
		kkb_load_custom_rel();
		kkb_patch();
	}
}

void kkb_osunlink_hook(OSModuleInfo* module) {
	if (module->id == ModuleId::HEI && kkbRel != nullptr) {
		kkb_free();
	}
}

const char* kkb_filealloc_hook(const char* path) {
	if (inKKB() && !strcmp(path, "a/c_uranoko")) {
		return "a/c_koopiekoo";
	}

	return path;
}

const char* kkb_msgsearch_hook(const char* msgKey) {
	if (!inKKB())
		return nullptr;

	if (!strcmp(msgKey, "btl_hlp_gold_chorobon")) {
		return "That's Dark Fuzzy.\n"
				"<wait 550>It teamed with Koopie Koo\n"
				"to stop you in your tracks.\n"
				"<k>\n<p>\n"
				"Now that the Fuzzy Hoard is\n"
				"out of the equation,<wait 300> we can\n"
				"pummel this fool to a pulp.\n"
				"<k>\n<p>\n"
				"Be careful though,<wait 50> Mario.\n"
				"<wait 400>In combination,<wait 200> they work\n"
				"well off of each other!\n"
				"<k>\n<p>\n"
				"<speed 0>"
				"<pos -25 0 0><col c00000ff>*"
				"</col><scale 0.85>"
				"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 05"
				"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 025"
				"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 00"
				"</scale>"
				"<pos 450 0 0><col c00000ff>*"
				"</col><pos 0 24 0><scale 0.85>Head Bop <col c00000ff>Attack Power</col>: 03"
				"<pos 0 48 0><scale 0.8><col 40BF9F>Superguardable</col>: Yes <pos 230 48 0><col 40BF9F>Status</col>: Slow"
				"<pos 0 68 0><scale 0.8><col 40BF9F>Chance</col>: 100%   <pos 230 68 0><col 40BF9F>Turns</col>: 2"
				"</speed></scale></pos><dkey><wait 500></dkey>"
				"<k>";
	} else if (!strcmp(msgKey, "btl_hlp_ura_pata") || !strcmp(msgKey, "btl_hlp_ura_noko")) {
		return "That's Koopie Koo!\n"
				"<wait 350>Boy,<wait 100> she seems infuriated.\n"
				"<wait 350>Don't get too close, yeah?\n"
				"<k>\n<p>\n"
				"Koopie Koo has <col c00000ff>two places\n"
				"</col>she'll fly to,<wait 300> that being the\n"
				"far <col c00000ff>left</col> or <col c00000ff>right</col> of here.\n"
				"<k>\n<p>\n"
				"If you can strike her <col c00000ff>without\n"
				"</col>her wings falling off,<wait 300> she will\n"
				"move to the far <col c00000ff>right!\n"
				"<k>\n<p>\n"
				"</col>That's your chance to hit\n"
				"her down to ground level!\n"
				"<k>\n<p>\n"
				"This'll make her attacks much\n"
				"more foreseeable for us!\n"
				"<wait 300>Here's her stat-page!\n"
				"<k>\n<p>\n"
				"<speed 0>"
				"<pos -25 0 0><col c00000ff>*"
				"</col><scale 0.85>"
				"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 05"
				"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 020"
				"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 01"
				"</scale>"
				"<pos 450 0 0><col c00000ff>*"
				"</col><pos 0 24 0><scale 0.85>Shell Shot/Toss <col c00000ff>Attack Power</col>: 04"
				"<pos 0 48 0><scale 0.8><col 40BF9F>Superguardable</col>: No <pos 230 48 0><col 40BF9F>Status</col>: Fire"
				"<pos 0 68 0><scale 0.8><col 40BF9F>Chance</col>: 50%   <pos 230 68 0><col 40BF9F>Turns</col>: 2"
				"</speed></scale></pos><dkey><wait 500></dkey>"
				"<k>";
	} else if (!strcmp(msgKey, "btl_un_ura_pata") || !strcmp(msgKey, "btl_un_ura_noko")) {
		return "Koopie Koo";
	} else if (!strcmp(msgKey, "menu_monosiri_tou")) {
		// TODO: might be better to just change the pointer that uses the menu_monosiri_tou msgKey?
		return "Petalburg";
	} else if (!strcmp(msgKey, "kkb_entry")) {
		return "<dynamic 2>Why won't you listen to me!?\n"
				"<wait 200>You'll just end up dead like\n"
				"the rest of them out there!\n"
				"<k>";
	}

	return nullptr;
}

void kkb_unitentry_hook(BattleUnitSetup* unit_setup) {
	BattleUnitKind *unitKind = unit_setup->unit_kind_params;

	if (inKKB() && unitKind->unit_type == BattleUnitType::GOLD_FUZZY) {
		unitKind->max_hp = 25;
		memset(unitKind->parts->defense, 0x00, 5);

		// we do this here because it's the easiest way to not make it happen before the OG fight is over
		memcpy(&hei_unit_gold_chorobon_damage_event,
			&gold_chorobon_damage_event_replacement, sizeof(gold_chorobon_damage_event_replacement));
	}
}

// TODO: dedup this and the hgf one
void kkb_load_custom_rel() {
	// load custom KKB rel into memory
	kkbRel = (OSModuleInfo *)readFileFromDvd("/mod/ura_pata.rel");

	// allocate bss and link the rel
	kkbRelBss = memory::__memAlloc(0, kkbRel->bss_size);
	g_OSLink_trampoline(kkbRel, kkbRelBss);
}

void kkb_patch() {
	// fix bgm used for orig gold fuzzy battle
	hei_battle_database_hei_btlsetup_hei_tbl[19].music_name = "BGM_FILE_MENU1";

	// fix stats for ura_pata
	tou2_unit_ura_pata_unit_ura_pata.max_hp = 20;
	tou2_unit_ura_pata_unit_ura_pata.level = 25;
	memset(tou2_unit_ura_pata_unit_ura_pata.status_vulnerability, 0, sizeof(StatusVulnerability));
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->sleep = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->poison = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->confuse = 100;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->electric = 100;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->huge = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->attack_up = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->attack_down = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->defense_up = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->defense_down = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->fast = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->slow = 50;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->dodgy = 200;
	tou2_unit_ura_pata_unit_ura_pata.status_vulnerability->invisible = 200;
	memset(tou2_unit_ura_pata_unit_ura_pata.parts->defense, 0x01, 5);
	memset(tou2_unit_ura_pata_unit_ura_pata.parts->defense_attr, BattleUnitDefenseAttr_Type::NORMAL, 5);
	memset(&tou2_unit_ura_pata_defence_turn, 0x01, 5);

	tou2_unit_ura_pata_weapon_patapata_attack.superguards_allowed = 0;
	tou2_unit_ura_pata_weapon_patapata_attack.damage_function_params[0] = 4;
	tou2_unit_ura_pata_weapon_patapata_attack.special_property_flags |= 0x1000; // FlipsShellEnemies
	tou2_unit_ura_pata_weapon_patapata_attack.target_weighting_flags |= 0x800; // PreferHigherHP
	tou2_unit_ura_pata_weapon_patapata_attack.burn_chance = 50;
	tou2_unit_ura_pata_weapon_patapata_attack.burn_time = 2;

	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.superguards_allowed = 0;
	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.damage_function_params[0] = 5;
	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.special_property_flags |= 0x1000 | 0x40; // FlipsShellEnemies | PiercesDefense
	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.burn_chance = 100;
	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.burn_time = 1;
	tou2_unit_ura_pata_weapon_ura_noko_ura_attack.damage_pattern = 0x05; // Knocked onto ground

	tou2_unit_ura_pata_weapon_nokonoko_attack.superguards_allowed = 0;
	tou2_unit_ura_pata_weapon_nokonoko_attack.damage_function_params[0] = 4;
	tou2_unit_ura_pata_weapon_nokonoko_attack.special_property_flags |= 0x1000; // FlipsShellEnemies
	tou2_unit_ura_pata_weapon_nokonoko_attack.burn_chance = 50;
	tou2_unit_ura_pata_weapon_nokonoko_attack.burn_time = 2;

	// fix stats for gold_chorobon
	hei_unit_gold_chorobon_unit_gold_chorobon.status_vulnerability->dizzy = 100;
	hei_unit_gold_chorobon_unit_gold_chorobon.status_vulnerability->sleep = 100;
	hei_unit_gold_chorobon_unit_gold_chorobon.status_vulnerability->poison = 100;

	// allocate and setup custom battle
	BattleUnitSetup* kkbBUS = new BattleUnitSetup[2];
	kkbBUS[0].unit_kind_params = &hei_unit_gold_chorobon_unit_gold_chorobon;
	kkbBUS[0].alliance = 1;
	kkbBUS[0].attack_phase = 0x4000004;
	kkbBUS[0].position = {70, 0, 0};
	kkbBUS[0].item_drop_table = &battle_database_common::battle_item_NONE;

	kkbBUS[1].unit_kind_params = &tou2_unit_ura_pata_unit_ura_pata;
	kkbBUS[1].alliance = 1;
	kkbBUS[1].attack_phase = 0x4000004;
	kkbBUS[1].position = {120, 40, 10};
	kkbBUS[1].item_drop_table = &battle_database_common::battle_item_NONE;

	BattleGroupSetup* kkbBGS = new BattleGroupSetup;
	kkbBGS->num_enemies = 2;
	kkbBGS->enemy_data = kkbBUS;
	kkbBGS->no_item_weight = 100;
	kkbBGS->hp_drop_table = &battle_database_common::battle_heart_drop_param_default; //probably drops, fix later
	kkbBGS->fp_drop_table = &battle_database_common::battle_flower_drop_param_default; //probably drops, fix later
	kkbBGS->unk_1c = 5; // FIXME/CHECK

	BattleSetupWeightedLoadout* kkbBSWL = new BattleSetupWeightedLoadout[2]; // null-terminated array
	kkbBSWL[0].weight = 1;
	kkbBSWL[0].group_data = kkbBGS;
	kkbBSWL[0].stage_data = &hei_battle_database_hei_hei_btlstage_data; // check pour les trucs qui tombent ou non?

	kkbBSD = new BattleSetupData;
	kkbBSD->battle_name = "\x89\xbd\x82\xb5\x82\xc4\x82\xe9\x82\xcc";
	kkbBSD->different_loadouts_flag = -1;
	kkbBSD->flag_off_loadouts = kkbBSWL;
	#ifndef DEBUG // only prevent fleeing in release mode
	kkbBSD->battle_setup_flags = 0x10;
	#endif
	kkbBSD->audience_type_weights[0] = {5, 5}; // Toad
	kkbBSD->audience_type_weights[5] = {5, 5}; // Shy Guy
	kkbBSD->audience_type_weights[8] = {5, 5}; // Koopa
	kkbBSD->audience_type_weights[10] = {5, 5}; // Goomba
	kkbBSD->audience_type_weights[12] = {1, 1}; // aud_{min,max}_wt_12
	kkbBSD->music_name = "BGM_FILE_MENU1";

	writeEvtBranch(&hei_hei_01_evt_nokorin[51], evt_nokorin_hook_setpos_gold_fuzzy); // set gold fuzzy initial pos (805c8468)
	writeEvtBranch(&hei_hei_01_evt_nokorin[143], evt_nokorin_hook_move_gold_fuzzy); // move gold fuzzy with KK (805c85d8)
	writeEvtBranch(&hei_hei_01_evt_nokorin[338], evt_nokorin_hook_flag_on_mario_hit); // set a flag when mario gets hit (805c88e4)
	writeEvtBranch(&hei_hei_01_evt_nokorin[509], evt_nokorin_hook_start_battle); // start the battle when the flag is set (805c8b90)

	// add an entry event to koopie koo, for the intro text (80606364)
	writeEvtBranch(&tou2_unit_ura_pata_init_event_patapata[21], ura_pata_init_event_hook_add_setevententry);

	// patch Koopie's attack events to use Reed's edited versions
	writeEvtBranch(&tou2_unit_ura_pata_ura_attack_event[235], ura_attack_event_camera_hook);

	// the following was autogenerated using ttyd-stuff/gen_ttydasm_patch.py
	tou2_unit_ura_pata_attack_event_patapata[99] = LW(0); // 80606844
	tou2_unit_ura_pata_attack_event_patapata[100] = LW(3); // 80606848
	tou2_unit_ura_pata_attack_event_patapata[127] = FLOAT(8.00); // 806068B4
	tou2_unit_ura_pata_attack_event_patapata[131] = LW(2); // 806068C4
	tou2_unit_ura_pata_attack_event_patapata[133] = LW(4); // 806068CC
	tou2_unit_ura_pata_attack_event_patapata[165] = FLOAT(3.00); // 8060694C
	tou2_unit_ura_pata_attack_event_patapata[188] = LW(2); // 806069A8
	tou2_unit_ura_pata_attack_event_patapata[190] = LW(4); // 806069B0
	tou2_unit_ura_pata_attack_event_patapata[297] = FLOAT(3.00); // 80606B5C
	tou2_unit_ura_pata_attack_event_patapata[300] = LW(0); // 80606B68
	tou2_unit_ura_pata_attack_event_patapata[301] = LW(3); // 80606B6C
	tou2_unit_ura_pata_attack_event_patapata[429] = FLOAT(3.00); // 80606D6C
	tou2_unit_ura_pata_attack_event_patapata[487] = LW(2); // 80606E54
	tou2_unit_ura_pata_attack_event_patapata[489] = LW(4); // 80606E5C
	tou2_unit_ura_pata_attack_event_patapata[513] = FLOAT(3.00); // 80606EBC
	tou2_unit_ura_pata_attack_event_patapata[523] = LW(2); // 80606EE4
	tou2_unit_ura_pata_attack_event_patapata[525] = LW(4); // 80606EEC

	tou2_unit_ura_pata_attack_event[84] = 2; // 80604F64
	tou2_unit_ura_pata_attack_event[88] = 350; // 80604F74
	tou2_unit_ura_pata_attack_event[129] = FLOAT(1.00); // 80605018
	tou2_unit_ura_pata_attack_event[259] = FLOAT(11.50); // 80605220
	tou2_unit_ura_pata_attack_event[522] = FLOAT(5.00); // 8060563C

	tou2_unit_ura_pata_ura_attack_event[44] = 2; // 80605754
	tou2_unit_ura_pata_ura_attack_event[48] = 350; // 80605764
	tou2_unit_ura_pata_ura_attack_event[90] = FLOAT(0.00); // 8060580C
	tou2_unit_ura_pata_ura_attack_event[108] = 10; // 80605854
	tou2_unit_ura_pata_ura_attack_event[140] = 10; // 806058D4
	tou2_unit_ura_pata_ura_attack_event[186] = 10; // 8060598C
	tou2_unit_ura_pata_ura_attack_event[189] = 50; // 80605998
	tou2_unit_ura_pata_ura_attack_event[216] = 150; // 80605A04
	tou2_unit_ura_pata_ura_attack_event[223] = 150; // 80605A20
	tou2_unit_ura_pata_ura_attack_event[230] = 150; // 80605A3C
	tou2_unit_ura_pata_ura_attack_event[285] = FLOAT(13.00); // 80605B18
	tou2_unit_ura_pata_ura_attack_event[316] = FLOAT(13.00); // 80605B94
	tou2_unit_ura_pata_ura_attack_event[462] = FLOAT(10.00); // 80605DDC
	tou2_unit_ura_pata_ura_attack_event[473] = 200; // 80605E08
	tou2_unit_ura_pata_ura_attack_event[495] = FLOAT(10.00); // 80605E60
}

void kkb_free() {
	delete[] kkbBSD->flag_off_loadouts->group_data->enemy_data;
	delete kkbBSD->flag_off_loadouts->group_data;
	delete[] kkbBSD->flag_off_loadouts;
	delete kkbBSD;
	kkbBSD = nullptr;

	gc::OSLink::OSUnlink(kkbRel);
	memory::__memFree(0, kkbRelBss);
	memory::__memFree(0, kkbRel);
	kkbRelBss = nullptr;
	kkbRel = nullptr;
}