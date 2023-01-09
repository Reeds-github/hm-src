#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/memory.h>

#include <hm/common.h>
#include <hm/larger_battles.h>

#include "common_types.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::battle_unit;
using namespace ttyd::battle_database_common;

using gc::OSLink::OSModuleInfo;

extern BattleGroupSetup gor_battle_database_gor_btlgrp_gor_gor_00_01_off_1;
extern BattleSetupData gor_battle_database_gor_btlsetup_gor_tbl[];
extern BattleUnitKind gor_unit_gundan_zako_unit_gundan_zako;
extern BattleWeapon gor_unit_gundan_zako_weapon_gundan_zako_attack;
extern BattleWeapon gor_unit_gundan_zako_weapon_gundan_zako_jump_attack;
extern int32_t gor_unit_gundan_zako_attack_event[];

// i do have code to make this stuff properly dynamic,
// but reed asked me to do this quick, so eh
BattleUnitSetup* gor_00_01_units = nullptr;

void battles_oslink_hook(OSModuleInfo* new_module) {
	if (new_module->id == ModuleId::GOR) {
		gor_unit_gundan_zako_unit_gundan_zako.max_hp = 1;
		gor_unit_gundan_zako_unit_gundan_zako.level = 11;
		gor_unit_gundan_zako_weapon_gundan_zako_attack.damage_function_params[0] = 1;
		gor_unit_gundan_zako_weapon_gundan_zako_jump_attack.damage_function_params[0] = 1;

		// disable the dekadeka attack
		writeEvtGoto(&gor_unit_gundan_zako_attack_event[69], &gor_unit_gundan_zako_attack_event[89] /*8061DFAC*/); // 8061DF5C

		// fill unit setup with copies of the same one enemy to use as a base
		gor_00_01_units = (BattleUnitSetup *)memory::__memAlloc(0, sizeof(BattleUnitSetup) * 3);
		for (uint8_t i = 0; i < 3; i++) {
			memcpy(&gor_00_01_units[i], gor_battle_database_gor_btlgrp_gor_gor_00_01_off_1.enemy_data, sizeof(BattleUnitSetup));
		}

		gor_00_01_units[1].unit_kind_params = &gor_unit_gundan_zako_unit_gundan_zako;
		gor_00_01_units[2].unit_kind_params = &gor_unit_gundan_zako_unit_gundan_zako;

		gor_00_01_units[0].position.x = 60;
		gor_00_01_units[0].position.z = 10;
		gor_00_01_units[2].position.x = 120;
		gor_00_01_units[2].position.z = -10;

		gor_battle_database_gor_btlgrp_gor_gor_00_01_off_1.num_enemies = 3;
		gor_battle_database_gor_btlgrp_gor_gor_00_01_off_1.enemy_data = gor_00_01_units;

		gor_battle_database_gor_btlsetup_gor_tbl[2].music_name = "BGM_EVT_MARIO_HOUSE2";
	}
}

void battles_osunlink_hook(OSModuleInfo* module) {
	if (module->id == ModuleId::GOR && gor_00_01_units != nullptr) {
		memory::__memFree(0, gor_00_01_units);
		gor_00_01_units = nullptr;
	}
}