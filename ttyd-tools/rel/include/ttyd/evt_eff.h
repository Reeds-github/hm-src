#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_eff {

extern "C" {

// .text
// evt_eff_fukidashi
// evt_eff_delete_ptr
// evt_eff_softdelete_ptr
// evt_eff_delete
// evt_eff_softdelete
EVT_DECLARE_USER_FUNC(evt_eff64, 14)
EVT_DECLARE_USER_FUNC(evt_eff, 14)

// .data
// dat_ptrarr_8030bf08
// dat_ptrarr_8030c0a4

}

}