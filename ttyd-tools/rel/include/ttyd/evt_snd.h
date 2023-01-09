#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_snd {

extern "C" {

// evt_snd_sfx_all_stop
// evt_snd_set_rev_mode
// evt_snd_env_lpf
// evt_snd_envoff_f
// evt_snd_envoff
// evt_snd_envon_f
// evt_snd_envon
// evt_snd_sfx_dist
EVT_DECLARE_USER_FUNC(evt_snd_sfx_pos, 4)
EVT_DECLARE_USER_FUNC(evt_snd_sfx_vol, 2)
// evt_snd_sfx_pit
EVT_DECLARE_USER_FUNC(evt_snd_sfxchk, 2)
EVT_DECLARE_USER_FUNC(evt_snd_sfxoff, 1)
// evt_snd_sfxon_3d_ex
EVT_DECLARE_USER_FUNC(evt_snd_sfxon_3d, 5)
// evt_snd_sfxon_
// evt_snd_sfxon__
EVT_DECLARE_USER_FUNC(evt_snd_sfxon, 2)
// evt_snd_bgm_scope
// evt_snd_bgm_freq
// evt_snd_bgmoff_f
EVT_DECLARE_USER_FUNC(evt_snd_bgmoff, 1)
// unk_801524c8
// evt_snd_bgmon_f
EVT_DECLARE_USER_FUNC(evt_snd_bgmon, 2)

}

}