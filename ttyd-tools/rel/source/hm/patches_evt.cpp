#include "mod.h"
#include "patch.h"

#include <ttyd/battle_camera.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evtmgr.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/unit_mario.h>

#include <hm/common.h>
#include <hm/patches_evt.h>

#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;

const int32_t EVT_NULL = 0xF1194D80; // -250'000'000

EVT_BEGIN(mario_dead_event)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 0, 7)
	USER_FUNC(battle_camera::evt_btl_camera_set_homing_unit, 0, -2, -1)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveSpeedLv, 0, 2)
	USER_FUNC(battle_camera::evt_btl_camera_set_zoom, 0, 470)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 1)
	USER_FUNC(battle_event_cmd::btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_DOWN1"), EVT_NULL, 0, EVT_NULL)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_I_S"))
	WAIT_MSEC(2066)
	USER_FUNC(battle_event_cmd::btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE2"), EVT_NULL, 0, EVT_NULL)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_D_2"))
	WAIT_MSEC(1000)
	USER_FUNC(battle_event_cmd::btlevtcmd_KillUnit, LW(10), 1)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 0)
	RETURN()
EVT_END()

EVT_BEGIN(mario_emergency_revival_event)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 0, 7)
	USER_FUNC(battle_camera::evt_btl_camera_set_homing_unit, 0, -2, -1)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveSpeedLv, 0, 2)
	USER_FUNC(battle_camera::evt_btl_camera_set_zoom, 0, 470)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 1)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_I_S"))
	WAIT_MSEC(666)
	USER_FUNC(battle_event_cmd::btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_LAND_DAMAGE2"), EVT_NULL, 0, EVT_NULL)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_D_2"))
	WAIT_MSEC(1100)
	USER_FUNC(battle_event_cmd::btlevtcmd_ConsumeItemReserve, LW(10), 151)
	BROTHER_EVT()
		USER_FUNC(battle_event_cmd::btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
		ADD(LW(1), 40)
		USER_FUNC(battle_event_cmd::btlevtcmd_DispItemIcon, LW(0), LW(1), LW(2), 151, 60)
	END_BROTHER()
	WAIT_FRM(60)
	USER_FUNC(battle_event_cmd::btlevtcmd_OnOffStatus, LW(10), 27, 0, 0, 0)
	USER_FUNC(battle_event_cmd::btlevtcmd_SetHp, LW(10), 0)
	USER_FUNC(battle_event_cmd::btlevtcmd_RecoverHp, LW(10), LW(11), 10)
	USER_FUNC(battle_event_cmd::btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
	ADD(LW(1), 30)
	USER_FUNC(evt_eff::evt_eff, 0, PTR("recovery"), 0, LW(0), LW(1), LW(2), 10, 0, 0, 0, 0, 0, 0, 0)
	WAIT_MSEC(1700)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_U_3"))
	WAIT_MSEC(50)
	USER_FUNC(battle_event_cmd::btlevtcmd_snd_se, -2, PTR("SFX_VOICE_MARIO_HAPPY2"), EVT_NULL, 0, EVT_NULL)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_V_2"))
	WAIT_MSEC(1000)
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("M_V_1"))
	WAIT_MSEC(1050)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 0, 0)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveSpeedLv, 0, 2)
	USER_FUNC(battle_camera::evt_btl_camera_set_zoom, 0, 0)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 0)
	USER_FUNC(battle_event_cmd::btlevtcmd_StartWaitEvent, -2)
	RETURN()
EVT_END()

EVT_BEGIN(mario_emergency_revival_event_hook)
	RUN_CHILD_EVT(mario_emergency_revival_event)
	RETURN()
EVT_END()

EVT_BEGIN(pow_block_guard_hook)
	IF_EQUAL(LW(6), 0)
		USER_FUNC(battle_event_cmd::btlevtcmd_ResultACDefence, LW(10), PTR(&battle_item_data::ItemWeaponData_Pow_Block))
		SET(LW(6), 1)
	END_IF()
	USER_FUNC(battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(10), LW(11), PTR(&battle_item_data::ItemWeaponData_Pow_Block), 256, LW(5)) // orig
	RETURN()
EVT_END()

EVT_BEGIN(generic_guard_hook)
	USER_FUNC(battle_event_cmd::btlevtcmd_ResultACDefence, LW(10), LW(12))
	USER_FUNC(battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(10), LW(11), LW(12), 256, LW(5)) // orig
	RETURN()
EVT_END()

EVT_BEGIN(generic_once_guard_hook)
	IF_EQUAL(LW(6), 0)
		USER_FUNC(battle_event_cmd::btlevtcmd_ResultACDefence, LW(10), LW(12))
		SET(LW(6), 1)
	END_IF()
	USER_FUNC(battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(10), LW(11), LW(12), 256, LW(5)) // orig
	RETURN()
EVT_END()

EVT_BEGIN(support_sub_effect_guard_hook)
	SET(LF(0), 0)
	IF_EQUAL(LW(12), &battle_item_data::ItemWeaponData_Nemure_Yoikoyo) // Sleepy Sheep
		SET(LF(0), 1)
	ELSE()
		IF_EQUAL(LW(12), &battle_item_data::ItemWeaponData_Guruguru_Memawashi) // Dizzy Dial
			SET(LF(0), 1)
		END_IF()
	END_IF()
	
	IF_EQUAL(LF(0), 1)
		IF_EQUAL(LW(6), 0)
			USER_FUNC(battle_event_cmd::btlevtcmd_ResultACDefence, LW(10), LW(12))
			SET(LW(6), 1)
		END_IF()
	END_IF()
	USER_FUNC(battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(10), LW(11), LW(12), 256, LW(5)) // orig
	RETURN()
EVT_END()

void patches_evt_init() {
	memcpy(&unit_mario::mario_dead_event,
		&mario_dead_event, sizeof(mario_dead_event));
	
	memcpy(&unit_mario::mario_emergency_revival_event,
		&mario_emergency_revival_event_hook, sizeof(mario_emergency_revival_event_hook));
	
	// make some items guardable
	writeEvtBranch(&battle_item_data::ItemEvent_Pow_Block[387] /*80366214*/, pow_block_guard_hook); // POW Block
	writeEvtBranch(&battle_item_data::ItemEvent_Fire_Flower_Core[162] /*803665D8*/, generic_once_guard_hook); // Fire Flower
	writeEvtBranch(&battle_item_data::ItemEvent_Kaminari_Core[172] /*80366A3C*/, generic_guard_hook); // Thunder {Rage,Bolt}
	writeEvtBranch(&battle_item_data::ItemEvent_Kirakira_Otoshi_Core[409] /*8036742C*/, generic_once_guard_hook); // Shooting Star
	writeEvtBranch(&battle_item_data::ItemEvent_Koorino_Ibuki[270] /*80367A40*/, generic_once_guard_hook); // Ice Storm
	writeEvtBranch(&battle_item_data::ItemEvent_Support_Sub_Effect[40] /*80365228*/, support_sub_effect_guard_hook); // Sleepy Sheep, Dizzy Dial
}