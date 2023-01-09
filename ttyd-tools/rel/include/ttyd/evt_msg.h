#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_msg {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_msg_fill_num, 4)  // unk_800d0998
// _evt_msg_fill_num / unk_US_EU_06_800d0a4c
EVT_DECLARE_USER_FUNC(evt_msg_fill_item, 4)  // unk_800d12b0
// _evt_msg_fill_item / unk_US_EU_08_800d1364
// evt_msg_npc
// evt_msg_pri
// evt_msg_togelr
EVT_DECLARE_USER_FUNC(evt_msg_toge, 4)
EVT_DECLARE_USER_FUNC(evt_msg_select, 2)
// evt_msg_close
// evt_msg_repeat
EVT_DECLARE_USER_FUNC(evt_msg_continue, 0)
// evt_msg_print_add_insert
EVT_DECLARE_USER_FUNC(evt_msg_print_add, 2)
EVT_DECLARE_USER_FUNC(evt_msg_print_insert, -1)
EVT_DECLARE_USER_FUNC(evt_msg_print_battle_party, 1)
// evt_msg_print_party_add
EVT_DECLARE_USER_FUNC(evt_msg_print_party, 1)
EVT_DECLARE_USER_FUNC(evt_msg_print, 4)
// _evt_msg_print
// evt_msg_init

}

}