#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/memory.h>

#include <hm/gon_09_ceilings.h>

#include "common_types.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::battle_unit;
using namespace ttyd::battle_database_common;

using gc::OSLink::OSModuleInfo;

extern BattleSetupWeightedLoadout gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_01_off;
extern BattleSetupWeightedLoadout gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_02_off;
extern BattleSetupWeightedLoadout gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_04_off;

BattleStageData* ceiling_btlstage = nullptr;
BattleStageObjectData* ceiling_btlstage_objects = nullptr;

// TODO: save space by not cloning the BSD, setting the objects ptr
// unconditionally and adjusting the objcount on battle setup
void ceilings_oslink_hook(OSModuleInfo* new_module) {
	if (new_module->id == ModuleId::GON) {
		ceiling_btlstage = (BattleStageData *)memory::__memAlloc(0, sizeof(BattleStageData));
		memcpy(ceiling_btlstage, gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_01_off.stage_data, sizeof(BattleStageData));

		uint32_t objdata_len = sizeof(BattleStageObjectData) * ceiling_btlstage->num_props;
		ceiling_btlstage_objects = (BattleStageObjectData *)memory::__memAlloc(0, objdata_len + (sizeof(BattleStageObjectData) * 2));
		memcpy(ceiling_btlstage_objects, ceiling_btlstage->props, objdata_len);

		ceiling_btlstage->props = ceiling_btlstage_objects;
		ceiling_btlstage_objects[ceiling_btlstage->num_props++] = {"if_wire", 5, 6, 0, 130, 0, 0, 0};
		ceiling_btlstage_objects[ceiling_btlstage->num_props++] = {"if_body", 5, 6, 0, 130, 0, 0, 0};

		// all three battles use gon_btlstage_data[2] originally
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_01_off.stage_data = ceiling_btlstage;
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_02_off.stage_data = ceiling_btlstage;
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_04_off.stage_data = ceiling_btlstage;

		// make some units start on the ceiling
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_01_off.group_data->enemy_data[2].unit_work[0] = 1;
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_02_off.group_data->enemy_data[2].unit_work[0] = 1;
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_04_off.group_data->enemy_data[1].unit_work[0] = 1;
		gon_battle_database_gon_btlgrpsel_gon_btlno_gon_09_04_off.group_data->enemy_data[2].unit_work[0] = 1;
	}
}

void ceilings_osunlink_hook(OSModuleInfo* module) {
	if (module->id == ModuleId::GON && ceiling_btlstage != nullptr) {
		memory::__memFree(0, ceiling_btlstage);
		memory::__memFree(0, ceiling_btlstage_objects);
		ceiling_btlstage = nullptr;
		ceiling_btlstage_objects = nullptr;
	}
}