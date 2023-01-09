#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_party {

extern "C" {

// N_evt_party_cloud_lock_animations_on_off
// evt_party_cloud_breathout
// evt_party_nokotaro_get_status
// evt_party_nokotaro_hold_cancel
// evt_party_nokotaro_kick_hold
// evt_party_nokotaro_kick
// evt_party_yoshi_fly
// evt_party_yoshi_ride
// L_evt_party_vivian_tail
// evt_party_vivian_unhold
// evt_party_vivian_hold
// evt_party_thunders_use
EVT_DECLARE_USER_FUNC(evt_party_get_name_hitobj_ride, 2)
EVT_DECLARE_USER_FUNC(evt_party_get_name_hitobj_head, 2)
// evt_party_get_status
// L_evt_party_dokan
// evt_party_set_breed_pose
// evt_party_sleep_off
// evt_party_sleep_on
EVT_DECLARE_USER_FUNC(evt_party_set_pose, 2)
// evt_party_set_homing_dist
// evt_party_move_beside_mario
// evt_party_move_behind_mario
// evt_party_jump_pos
// evt_party_wait_landon
EVT_DECLARE_USER_FUNC(evt_party_move_pos2, 4)
// evt_party_move_pos
// evt_party_run
EVT_DECLARE_USER_FUNC(evt_party_stop, 1)
// evt_party_set_dispdir
// evt_party_get_dispdir
// evt_party_set_dir_pos
// evt_party_set_dir_npc
EVT_DECLARE_USER_FUNC(evt_party_set_dir, 3)
// evt_party_get_pos
EVT_DECLARE_USER_FUNC(evt_party_set_hosei_xyz, 4)
// evt_party_set_pos
// evt_party_outofscreen
// evt_party_force_reset_outofscreen
// evt_party_set_camid
// evt_party_init_camid
// evt_party_cont_onoff
// evt_party_dispflg_onoff
// evt_party_flg_onoff

}

}