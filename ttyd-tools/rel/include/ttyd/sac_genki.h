#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_genki {

extern "C" {

// .text
// recover_cmd_disable
// star_stone_appear
// star_appear
EVT_DECLARE_USER_FUNC(status_on, 0)
// get_score
// wait_game_end
// start_game
// object_get_num
// object_entry
// weapon_entry
// get_ptr
// disp_3D_alpha
// disp_3D
// disp_2D
// end_genki
// main_star1
// main_star0
// main_star
// main_object
// main_weapon
// main_target
void main_mario();
// main_base
// main_genki

// .data
// genki_evt_common
// status_recover_evt

// .sdata
// marioAttackEvent_Genki0
// marioAttackEvent_Genki1

}

}