#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/evtmgr.h>
#include <ttyd/item_data.h>
#include <ttyd/mariost.h>

#include <hm/blooper_rematch.h>
#include <hm/common.h>
#include <hm/heromode.h>

#include "common_types.h"
#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::battle_unit;
using namespace ttyd::battle_database_common;
using namespace ttyd::evtmgr_cmd;
using namespace ttyd::item_data;

using gc::OSLink::OSModuleInfo;
using ttyd::mariost::g_MarioSt;

extern "C" {
	extern int32_t tik_tik_02_geso_battle[];
	extern int32_t tik_tik_02_tik_02_init_evt[];

	EVT_DECLARE_USER_FUNC(tik_unit_boss_gesso__ikasumi_effect, 2)

	extern BattleUnitKind tik_unit_boss_gesso_unit_boss_gesso;
	extern int8_t tik_unit_boss_gesso_defence_attr[5];
	extern int8_t tik_unit_boss_gesso_defence_attr_ceil[5];
	extern int8_t tik_unit_boss_gesso_R_defence_attr_turn[5];
	extern int32_t tik_unit_boss_gesso_battle_entry_event[];
	extern int32_t tik_unit_boss_gesso_phase_event[];
	extern BattleWeapon tik_unit_boss_gesso_weapon_gesso_ikasumi_attack;
	extern int32_t tik_unit_boss_gesso_gesso_ikasumi_attack[];

	extern BattleUnitKind tik_unit_boss_gesso_right_arm_unit_boss_gesso_right_arm;
	extern int8_t tik_unit_boss_gesso_right_arm_defence_attr[5];
	extern BattleWeapon tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack;
	extern BattleWeapon tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack;
	extern int32_t tik_unit_boss_gesso_right_arm_attack_event[];

	extern BattleUnitKind tik_unit_boss_gesso_left_arm_unit_boss_gesso_left_arm;
	extern int8_t tik_unit_boss_gesso_left_arm_defence_attr[5];
	extern BattleWeapon tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack;

	extern BattleGroupSetup tik_battle_database_tik_btlgrp_tik_tik_gesso;
	extern BattleSetupData tik_battle_database_tik_btlsetup_tik_tbl[];
}

EVT_DECLARE_USER_FUNC(evt_set_held_item, 1)
EVT_DEFINE_USER_FUNC(evt_set_held_item) {
	auto unitPtr = battle::BattleGetUnitPtr(battle::g_BattleWork, (int32_t)evt->wActorThisPtr);
	unitPtr->held_item = evtGetValue(evt, evt->currentCommandArguments[0]);
	return 2;
}

EVT_DECLARE_USER_FUNC(ikasumi_huge_toggle, 1)
EVT_DEFINE_USER_FUNC(ikasumi_huge_toggle) {
	auto gessoUnitPtr = battle::BattleGetUnitPtr(battle::g_BattleWork, (int32_t)evt->wActorThisPtr);
	if (gessoUnitPtr->size_change_turns > 0) {
		bool isOn = !!evtGetValue(evt, evt->currentCommandArguments[0]);

		// edit the floats used to initialize effOpukuJetwN64Entry in _ikasumi_effect
		*(float *)0x805c5ce4 = isOn ? 115 : 95;
		*(float *)0x805c5ce8 = isOn ? 180 : 75;
		*(float *)0x805c5cec = isOn ? 3 : 2;
		*(float *)0x805c5cf0 = isOn ? 150 : 135;
	}

	return 2;
}

EVT_BEGIN(tik_02_init_evt_geso_hook)
	USER_FUNC(evt_hm_seq_get_bit, 0x1, LW(0))
	IF_EQUAL(LW(0), 0) // if the rematch hasn't been beaten yet
		RUN_EVT(tik_tik_02_geso_battle)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(geso_battle_wait_position_hook)
	DO(0)
		USER_FUNC(evt_mario::evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
		IF_SMALL(LW(0), -475)
			DO_BREAK()
		END_IF()
		WAIT_FRM(1)
	WHILE()

	SET(GF(0), 0)
	USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_STG0_GESO_LEG_MOVE2"), 0)
	RETURN()
EVT_END()

EVT_BEGIN(geso_battle_set_hm_seq_hook)
	USER_FUNC(evt_hm_seq_set_bit, 0x1, 1) // set blooper rematch as complete
	SET(GF(0), 0) // reset the bit used to check whether the half hp event has been triggered
	RETURN()
EVT_END()

EVT_BEGIN(gesso_ikasumi_attack_item_event_hook)
	IF_EQUAL(GF(0), 0) // if the half hp event hasn't been triggered yet
		// don't need to check if it's on ground, as it always is when doing this attack
		USER_FUNC(battle_event_cmd::btlevtcmd_SetStayPose, -2, PTR("GSO_S2_1"))
		USER_FUNC(battle_event_cmd::btlevtcmd_SetTalkPose, -2, PTR("GSO_S2_1")) // there's no upright talking pose, so idle it is
		USER_FUNC(battle_event_cmd::btlevtcmd_GetHp, -2, LW(0))
		USER_FUNC(battle_event_cmd::btlevtcmd_GetMaxHp, -2, LW(1))
		DIV(LW(1), 2)
		IF_SMALL_EQUAL(LW(0), LW(1)) // if half HP or less
			SET(GF(0), 1) // prevent the evt from triggering again
			USER_FUNC(evt_msg::evt_msg_print, 2, PTR("br_halfhp"), 0, -2)
			USER_FUNC(evt_set_held_item, ItemType::POWER_PUNCH)
			RUN_CHILD_EVT(battle_item_data::ItemEvent_Dekadeka_Drink)
		END_IF()
	END_IF()

	SET(LW(12), PTR(&tik_unit_boss_gesso_weapon_gesso_ikasumi_attack)) // orig
	RETURN()
EVT_END()

EVT_BEGIN(ikasumi_attack_hugeink_hook)
	USER_FUNC(ikasumi_huge_toggle, 1)
	USER_FUNC(tik_unit_boss_gesso__ikasumi_effect, -2, LW(7))
	USER_FUNC(ikasumi_huge_toggle, 0)
	RETURN()
EVT_END()

bool inBR() {
	return g_MarioSt->gsw0 == 63 && inBattleWithGroupSetup(&tik_battle_database_tik_btlgrp_tik_tik_gesso);
}

void br_oslink_hook(OSModuleInfo* new_module) {
	if (new_module->id == ModuleId::TIK && g_MarioSt->gsw0 == 63) {
		br_patch();
	}
}

const char* br_filealloc_hook(const char* path) {
	if (g_MarioSt->gsw0 == 63 && !strcmp(g_MarioSt->currentMapName, "tik_02")) {
		if (!strcmp(path, "a/c_gesso-")) {
			return "a/c_gesso5-";
		} else if (!strcmp(path, "a/c_gesso2-")) {
			return "a/c_gesso6-";
		} else if (!strcmp(path, "a/c_gesso3-")) {
			return "a/c_gesso7-";
		} else if (!strcmp(path, "a/c_gesso4-")) {
			return "a/c_gesso8-";
		}
	}

	return path;
}

void br_patch() {
	// start the geso_battle script on seq 63 too
	writeEvtNop(&tik_tik_02_tik_02_init_evt[171]); // 805D117C
	writeEvtBranch(&tik_tik_02_tik_02_init_evt[174], tik_02_init_evt_geso_hook); // 805D1188
	writeEvtNop(&tik_tik_02_tik_02_init_evt[176]); // 805D1190

	// run a script that starts the battle when mario passes a certain position
	writeEvtBranch(&tik_tik_02_geso_battle[0], geso_battle_wait_position_hook, true); // 805D0198

	// skip the tentacle stuff when it's the rematch
	writeEvtGoto(&tik_tik_02_geso_battle[6], &tik_tik_02_geso_battle[41] /*805D023C*/); // 805D01B0
	writeEvtGoto(&tik_tik_02_geso_battle[85], &tik_tik_02_geso_battle[113] /*805D035C*/); // 805D02EC
	writeEvtNop(&tik_tik_02_geso_battle[147]); // nop out the tentacle sfx (805D03E4)

	// skip the water platforms' spawn animation after the rematch
	writeEvtGoto(&tik_tik_02_geso_battle[608], &tik_tik_02_geso_battle[656] /*805D0BD8*/); // 805D0B18

	// on rematch completion, set hm seq bit instead of editing the global seq
	writeEvtBranch(&tik_tik_02_geso_battle[656], geso_battle_set_hm_seq_hook); // 805D0BD8

	// never go back to the ceiling/respawn the tentacles
	writeEvtGoto(&tik_unit_boss_gesso_phase_event[129], &tik_unit_boss_gesso_phase_event[495] /*805DFE8C*/); // 805DF8D4

	// disable the """random""" dialogue event
	writeEvtGoto(&tik_unit_boss_gesso_right_arm_attack_event[96], &tik_unit_boss_gesso_right_arm_attack_event[159] /*805E4140*/); // 805E4044

	// add the item use event
	writeEvtBranch(&tik_unit_boss_gesso_gesso_ikasumi_attack[0], gesso_ikasumi_attack_item_event_hook); // 805E0014

	// fix the ink jet position when he's huge
	writeEvtBranch(&tik_unit_boss_gesso_gesso_ikasumi_attack[86], ikasumi_attack_hugeink_hook); // 805E016C

	// fix koops' talk&stay poses by just not setting any in the entry evt
	writeEvtGoto(&tik_unit_boss_gesso_battle_entry_event[287], &tik_unit_boss_gesso_battle_entry_event[295] /*805DF3BC*/); // 805DF39C

	// make the blooper attack twice with reed's extended rel bullshit
	// doing this makes me sad
	*(uint32_t *)0x805dfedc = 0x2;

	tik_battle_database_tik_btlsetup_tik_tbl[3].music_name = "BGM_STG6_EKI1"; // index 3 is this battle

	// unit changes
	tik_unit_boss_gesso_unit_boss_gesso.max_hp = 40;
	tik_unit_boss_gesso_unit_boss_gesso.level = 48;
	tik_unit_boss_gesso_unit_boss_gesso.parts->counter_attribute_flags |= 0x800; // ElectricStatus
	tik_unit_boss_gesso_defence_attr[WeaponElementType::EXPLOSIVE] = BattleUnitDefenseAttr_Type::WEAKNESS;
	tik_unit_boss_gesso_defence_attr_ceil[WeaponElementType::EXPLOSIVE] = BattleUnitDefenseAttr_Type::WEAKNESS;
	tik_unit_boss_gesso_R_defence_attr_turn[WeaponElementType::EXPLOSIVE] = BattleUnitDefenseAttr_Type::WEAKNESS;

	tik_unit_boss_gesso_right_arm_unit_boss_gesso_right_arm.max_hp = 15;
	tik_unit_boss_gesso_right_arm_unit_boss_gesso_right_arm.parts->counter_attribute_flags |= 0x800; // ElectricStatus
	tik_unit_boss_gesso_right_arm_defence_attr[WeaponElementType::EXPLOSIVE] = BattleUnitDefenseAttr_Type::WEAKNESS;

	tik_unit_boss_gesso_left_arm_unit_boss_gesso_left_arm.max_hp = 15;
	tik_unit_boss_gesso_left_arm_unit_boss_gesso_left_arm.parts->counter_attribute_flags |= 0x800; // ElectricStatus
	tik_unit_boss_gesso_left_arm_defence_attr[WeaponElementType::EXPLOSIVE] = BattleUnitDefenseAttr_Type::WEAKNESS;

	// weapon changes
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.damage_function_params[0] = 3;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.element = WeaponElementType::ELECTRIC;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.special_property_flags |= 0x40; // PiercesDefense
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.stop_chance = 50;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.stop_time = 1;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.poison_chance = 50;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.poison_time = 1;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.poison_strength = 2;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.electric_chance = 100;
	tik_unit_boss_gesso_weapon_gesso_ikasumi_attack.electric_time = 8;

	tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack.damage_function_params[0] = 5;
	tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack.element = WeaponElementType::ELECTRIC;
	tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack.damage_pattern = 0x6; // Knocked into next target
	tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack.electric_chance = 100;
	tik_unit_boss_gesso_left_arm_weapon_gesso_left_arm_attack.electric_time = 8;

	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack.damage_function_params[0] = 5;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack.element = WeaponElementType::ELECTRIC;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack.damage_pattern = 0xa; // Squashed
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack.electric_chance = 100;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_front_attack.electric_time = 8;

	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack.damage_function_params[0] = 5;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack.element = WeaponElementType::ELECTRIC;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack.damage_pattern = 0xa; // Squashed
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack.electric_chance = 100;
	tik_unit_boss_gesso_right_arm_weapon_gesso_right_arm_rear_attack.electric_time = 8;
}

const char* br_msgsearch_hook(const char* msgKey) {
	if (!inBR())
		return nullptr;

	if (!strcmp(msgKey, "btl_un_gesso")) {
		return "Electro Blooper";
	} else if (!strcmp(msgKey, "btl_hlp_gesso")) {
		return "That's Electro Blooper!\n"
				"<wait 300>This breed of Blooper emits\n"
				"electricity around its body!\n"
				"<k><p>\n"
				"Touching it with your bare\n"
				"hands isn't the best idea.\n"
				"<wait 300>So use <col c00000ff>disjointed</col> attacks!\n"
				"<k><p>\n"
				"And if we make contact with\n"
				"its tentacles,<wait 200> they'll grant us\n"
				"the <col c00000ff>electrify</col> status.\n"
				"<k>\n<p>\n"
				"Oh,<wait 400> it's also weak to any\n"
				"<col c00000ff>explosive</col> related attacks.\n"
				"<wait 400>Here are the stats!\n"
				"<k>\n<p>\n"
				"<speed 0>"
				"<pos -25 0 0><col c00000ff>*"
				"</col><scale 0.85>"
				"<pos -1 0 0><col 202060ff>Lv</col>.<pos 28 0 0> 08"
				"<pos 83 0 0><col 202060ff>Health Points</col>:<pos 238 0 0> 040"
				"<pos 305 0 0><col 202060ff>Defense</col>:<pos 400 0 0> 00"
				"</scale>"
				"<pos 450 0 0><col c00000ff>*"
				"</col><pos 0 24 0><scale 0.85>Electro Ink <col c00000ff>Pierce Attack</col>: 03"
				"<pos 0 48 0><scale 0.8><col 40BF9F>Superguardable</col>: Yes <pos 230 48 0><col 40BF9F>Status</col>: Poison/Stop"
				"<pos 0 68 0><scale 0.8><col 40BF9F>Chance</col>: 50%   <pos 230 68 0><col 40BF9F>Turns</col>: 1"
				"</speed></scale></pos><dkey><wait 500></dkey>"
				"<k>";
	} else if (!strcmp(msgKey, "tik_02_gs_01")) {
		return "Bloobloobloobloop!\n"
				"<wait 100>\n"
				"(So,<wait 300> you're the one who\n"
				" was messin' with my bro?)\n"
				"<k>";
	} else if (!strcmp(msgKey, "tik_02_gs_02")) {
		return "Bloobloop! <wait 250>Bloobloobloop!\n"
				"<wait 100>\n"
				"(Enough of the chit-chatting!\n"
				"<wait 300> You're donezo, y'hear?!)\n"
				"<k>";
	} else if (!strcmp(msgKey, "tik_02_gs_03")) {
		return "BlooblooblooblooblooBLOOP!\n"
				"<wait 100>\n"
				"(It's time to say goodnight!)\n"
				"<k>";
	} else if (!strcmp(msgKey, "tik_02_gs_03_01")) {
		auto partyUnit = battle::BattleGetPartyPtr(battle::g_BattleWork);
		if (partyUnit != nullptr) {
			auto partyUnitType = partyUnit->unit_kind_params->unit_type;
			if (partyUnitType == BattleUnitType::GOOMBELLA) {
				return "Look here buddy,<wait 400> it's not our\n"
						"problem your little brother\n"
						"can't hold a relationship!\n"
						"<k>";
			} else if (partyUnitType == BattleUnitType::KOOPS) {
				return "<dynamic 2>That Blooper is huge!\n"
						"</dynamic><wait 400>But...<wait 400>do we have to fight?\n"
						"<wait 500>He seems way too strong...\n"
						"<k>\n<p>\n"
						"<wait 600><dynamic 2>Bleh!</dynamic><wait 400> You got this Koops!\n"
						"<wait 500>This is for Koopie Koo!\n"
						"<k>";
			}
		}
	} else if (!strcmp(msgKey, "br_halfhp")) {
		return "<dynamic 2>\n"
				"Blooblooop!<wait 250> BLOOOP!<wait 100>\n"
				"</dynamic> \n"
				"(<dynamic 2>Alright!</dynamic><wait 500> I'm done with your\n"
				" super hero shenanigans!)\n"
				"<k>\n<p>\n"
				"Bloop!<wait 100> BLOOOOOOOOOOOOP!\n"
				"<wait 100>(It's time to meet your maker!)\n"
				"<k>";
	}

	return nullptr;
}