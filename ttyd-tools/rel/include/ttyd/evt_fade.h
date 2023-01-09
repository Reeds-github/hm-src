#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_fade {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(evt_fade_reset, 1)
// evt_fade_softfocus_onoff
// evt_fade_tec_onoff
// evt_fade_set_mapchange_type
// evt_fade_set_anim_virtual_pos
// evt_fade_set_anim_ofs_pos
// evt_fade_set_spot_pos
EVT_DECLARE_USER_FUNC(evt_fade_end_wait, 0)
EVT_DECLARE_USER_FUNC(evt_fade_entry, 5)
EVT_DECLARE_USER_FUNC(evt_fade_out, 1)
EVT_DECLARE_USER_FUNC(evt_fade_in, 1)

}

}