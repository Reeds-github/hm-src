#pragma once

#include <cstdint>

namespace ttyd::evt_movefloor {

extern "C" {

// .text
// evt_moving_floor_delete
// moving_floor_halt_se
// moving_floor_stop_se
// moving_floor_set_se
// moving_floor_reverse
// moving_floor_getf_xz_scale
// moving_floor_get_stmsec
// moving_floor_getf_now_bpoint
// moving_floor_getf_pos
// moving_floor_update_turn
// moving_floor_get_mode
// moving_floor_get_kind
// moving_floor_get_pause
// moving_floor_get_end
// evt_moving_floor_main2
// evt_moving_floor_main
// evt_moving_floor_init
// evt_moving_floor_free
// evt_moving_floor_alloc

// .data
// sintbl
// moving_floor_bero_check
extern int32_t moving_floor[];
// ridedown_floor
// dat_ptrarr_803acf6c

}

}