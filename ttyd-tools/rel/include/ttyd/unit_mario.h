#pragma once

#include <cstdint>

namespace ttyd::unit_mario {

extern "C" {

// .text
// _get_mario_hammer_lv
// _mario_super_emblem_anim_set
// _mario_makkuro_set
// faker_mario_makkuro_set
// callback
// mario_pinch_pose_sound_callback
// _get_local_frame

// .data
// pose_table_mario_stay
// data_table_mario
// unitpartsdata_Mario
// unitdata_Mario
// battle_entry_event
// mario_ride_btl_entry_event
// init_event
extern int32_t mario_dead_event[];
extern int32_t mario_emergency_revival_event[];
// mario_appeal_event
// attack_audience_event
// btlataudevtMarioJump
// btlataudevtMarioHammer

// .sdata
extern int8_t mario_defence[5];
// defence_attr
// regist
// pose_table_object
// pose_sound_timing_table
// damage_event
// attack_event
// wait_event
// mario_win_reaction

}

}