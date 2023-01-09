#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_damage.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_movefloor.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mobjdrv.h>

#include <hm/common.h>
#include <hm/room_objects.h>

#include "common_types.h"
#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace ttyd::evtmgr_cmd;
using namespace ttyd::item_data;

using gc::OSLink::OSModuleInfo;
using ttyd::evt_bero::BeroInfo;
using ttyd::evt_bero::BeroInfoType;
using ttyd::evt_bero::BeroInfoDirectionId;

extern int32_t gon_gon_04_evt_gon_04flowblock_updown1_init[];
extern int32_t gon_gon_04_evt_gon_04flowblock_updown1[];
extern int32_t gon_gon_04_evt_gon_04flowblock_updown2_init[];
extern int32_t gon_gon_04_evt_gon_04flowblock_updown2[];
extern int32_t gon_gon_04_evt_hashi[];
extern int32_t gon_gon_05_main_1[];

extern int32_t gon_gon_00_gon_00_init_evt[];
extern int32_t gon_gon_01_gon_01_init_evt[];
extern int32_t gon_gon_02_gon_02_init_evt[];
extern int32_t gon_gon_03_gon_03_init_evt[];
extern int32_t gon_gon_04_gon_04_init_evt[];
extern int32_t gon_gon_05_gon_05_init_evt[];
extern int32_t gon_gon_06_gon_06_init_evt[];
extern int32_t gon_gon_07_gon_07_init_evt[];
extern int32_t gon_gon_08_gon_08_init_evt[];
extern int32_t gon_gon_09_gon_09_init_evt[];
extern int32_t gor_gor_01_gor_01_init_evt[];
extern int32_t gor_gor_02_gor_02_init_evt[];
extern int32_t hei_hei_00_hei_00_init_evt[];
extern int32_t hei_hei_01_hei_01_init_evt[];
extern int32_t hei_hei_02_hei_02_init_evt[];
extern int32_t hei_hei_04_hei_04_init_evt[];
extern int32_t hei_hei_06_hei_06_init_evt[];
extern int32_t hei_hei_10_hei_10_init_evt[];
extern int32_t hei_hei_13_hei_13_init_evt[];
extern int32_t nok_nok_00_nok_00_init_evt[];
extern int32_t nok_nok_01_nok_01_init_evt[];
extern int32_t tik_tik_00_tik_00_init_evt[];
extern int32_t tik_tik_01_tik_01_init_evt[];
extern int32_t tik_tik_04_tik_04_init_evt[];
extern int32_t tik_tik_05_tik_05_init_evt[];
extern int32_t tik_tik_13_tik_13_init_evt[];
extern int32_t tik_tik_15_tik_15_init_evt[];
extern int32_t tik_tik_16_tik_16_init_evt[];
extern int32_t tik_tik_17_tik_17_init_evt[];
extern int32_t tik_tik_19_tik_19_init_evt[];

// I don't understand the point of this one, and I'm not gonna try
// (based on las_21::set_mobj_y_position, which gets used in floatblk_evt)
EVT_DECLARE_USER_FUNC(las_21_set_mobj_position, 4)
EVT_DEFINE_USER_FUNC(las_21_set_mobj_position) {
	auto mobjName = (char *)evtGetValue(evt, evt->currentCommandArguments[0]);
	auto mobjPtr = mobjdrv::mobjNameToPtr(mobjName);
	auto posVec = (float *)((uintptr_t)mobjPtr + 0x38);

	posVec[0] = evtGetFloat(evt, evt->currentCommandArguments[1]);
	posVec[1] = evtGetFloat(evt, evt->currentCommandArguments[2]);
	posVec[2] = evtGetFloat(evt, evt->currentCommandArguments[3]);

	mobjdrv::mobjCalcMtx(mobjPtr);
	return 2;
}

EVT_BEGIN(evt_floatblk_shake)
	USER_FUNC(evt_sub::evt_sub_intpl_init, 11, 10, 3, 4)
	LBL(0)
	USER_FUNC(evt_sub::evt_sub_intpl_get_value)
	DIVF(LW(0), FLOAT(1000.00))
	USER_FUNC(evt_cam::evt_cam_shake, 4, FLOAT(0.00), LW(0), 100)
	IF_EQUAL(LW(1), 1)
		GOTO(0)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(evt_floatblk_stop_se)
	USER_FUNC(evt_mobj::evt_mobj_get_position, LW(15), LW(0), LW(1), LW(2))
	USER_FUNC(evt_mobj::evt_mobj_get_kindname, LW(15), LW(3))
	SET(LF(0), 0)
	IF_STR_EQUAL(LW(3), PTR("MOBJ_green_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_STR_EQUAL(LW(3), PTR("MOBJ_purple_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_STR_EQUAL(LW(3), PTR("MOBJ_orange_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_EQUAL(LF(1), 1)
		USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_BLOCK_B_FLOAT_STOP1"), LW(0), LW(1), LW(2), 0)
	END_IF()
	RETURN()
EVT_END()

// from las_21::floatblk_evt, with edits to support X and Z
// it's very bad, but don't worry about it :)
// LF(2-4) are used for toggling X/Y/Z
// UWs are used because I'm too tired to make everything fit in less LWs
// UW(0-1) are the begin/end values for the X axis
// UW(2-3) are the same for the Z axis
// UW(4-5) are the speed params for the X axis (same as LW(10-9))
// UW(6-7) are the same for the Z axis
// UW(8-9) are temps for evt_mobj_get_position
// UW(10-12) contain the travel distance for X/Y/Z
// UW(13) contains the index of the axis with the largest travel distance
// UW(14-15) are used as temp vars for speed ratio calculation
// UW(16-17) contain the max speed, pos/neg in that order
// (yes this should very much be a C func instead of an evt mess)
EVT_BEGIN(evt_floatblk)
	SETF(UW(17), UW(16))
	MULF(UW(17), FLOAT(-1.00))

	SET(LW(15), LW(0))
	SET(LW(14), LW(1))
	SET(LW(13), LW(2))
	SET(LW(12), LW(3))
	SET(LW(11), LW(4))
	IF_SMALL(UW(0), UW(1))
		SETF(UW(4), FLOAT(0.10))
		SETF(UW(5), FLOAT(0.05))
		SET(LF(5), 0)
	ELSE()
		SETF(UW(4), FLOAT(-0.10))
		SETF(UW(5), FLOAT(-0.05))
		SET(LF(5), 1)
	END_IF()
	IF_SMALL(LW(12), LW(11))
		SETF(LW(10), FLOAT(0.10))
		SETF(LW(9), FLOAT(0.05))
		SET(LF(1), 0)
	ELSE()
		SETF(LW(10), FLOAT(-0.10))
		SETF(LW(9), FLOAT(-0.05))
		SET(LF(1), 1)
	END_IF()
	IF_SMALL(UW(2), UW(3))
		SETF(UW(6), FLOAT(0.10))
		SETF(UW(7), FLOAT(0.05))
		SET(LF(6), 0)
	ELSE()
		SETF(UW(6), FLOAT(-0.10))
		SETF(UW(7), FLOAT(-0.05))
		SET(LF(6), 1)
	END_IF()
	RUN_EVT(evt_floatblk_shake)
	WAIT_MSEC(500)
	USER_FUNC(evt_mobj::evt_mobj_get_position, LW(15), LW(0), LW(1), LW(2))
	USER_FUNC(evt_mobj::evt_mobj_get_kindname, LW(15), LW(3))
	SET(LF(0), 0)
	IF_STR_EQUAL(LW(3), PTR("MOBJ_green_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_STR_EQUAL(LW(3), PTR("MOBJ_purple_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_STR_EQUAL(LW(3), PTR("MOBJ_orange_big02_FlowBlock"))
		SET(LF(0), 1)
	END_IF()
	IF_EQUAL(LF(1), 0)
		IF_EQUAL(LF(0), 1)
			USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_BLOCK_B_FLOAT_MOVE1"), LW(0), LW(1), LW(2), LW(8))
		ELSE()
			USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_BLOCK_S_FLOAT_MOVE1"), LW(0), LW(1), LW(2), LW(8))
		END_IF()
	ELSE()
		IF_EQUAL(LF(0), 1)
			USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_BLOCK_B_FLOAT_DOWN1"), LW(0), LW(1), LW(2), 0)
		ELSE()
			USER_FUNC(evt_snd::evt_snd_sfxon_3d, PTR("SFX_MOBJ_BLOCK_S_FLOAT_DOWN1"), LW(0), LW(1), LW(2), 0)
		END_IF()
		SET(LW(8), -1)
	END_IF()
	USER_FUNC(evt_mobj::evt_mobj_get_y_position, LW(15), LW(7))
	DO(0)
		SET(LF(0), 0)
		IF_EQUAL(LF(1), 1)
			USER_FUNC(evt_mario::evt_mario_get_name_hitobj_head, LW(0))
			IF_STR_EQUAL(LW(0), LW(14))
				SET(LF(0), 1)
			END_IF()
			USER_FUNC(evt_mario::evt_mario_get_party, LW(0))
			IF_NOT_EQUAL(LW(0), 0)
				USER_FUNC(evt_party::evt_party_get_name_hitobj_head, 0, LW(0))
				IF_STR_EQUAL(LW(0), LW(14))
					SET(LF(0), 1)
				END_IF()
			END_IF()
		END_IF()
		IF_EQUAL(LF(0), 0)
			GOTO(50)
		END_IF()
		USER_FUNC(evt_mobj::evt_mobj_exec_cancel, LW(13))
		SET(LW(0), UW(0))
		SET(UW(0), UW(1))
		SET(UW(1), LW(0))
		SET(LW(0), LW(12))
		SET(LW(12), LW(11))
		SET(LW(11), LW(0))
		SET(LW(0), UW(2))
		SET(UW(2), UW(3))
		SET(UW(3), LW(0))
		IF_EQUAL(LF(5), 0)
			SETF(UW(4), FLOAT(-0.10))
			SETF(UW(5), FLOAT(-0.05))
			SET(LF(5), 1)
		ELSE()
			SETF(UW(4), FLOAT(0.10))
			SETF(UW(5), FLOAT(0.10))
			SET(LF(5), 0)
		END_IF()
		IF_EQUAL(LF(1), 0)
			SETF(LW(10), FLOAT(-0.10))
			SETF(LW(9), FLOAT(-0.05))
			SET(LF(1), 1)
		ELSE()
			SETF(LW(10), FLOAT(0.10))
			SETF(LW(9), FLOAT(0.10))
			SET(LF(1), 0)
		END_IF()
		IF_EQUAL(LF(6), 0)
			SETF(UW(6), FLOAT(-0.10))
			SETF(UW(7), FLOAT(-0.05))
			SET(LF(6), 1)
		ELSE()
			SETF(UW(6), FLOAT(0.10))
			SETF(UW(7), FLOAT(0.10))
			SET(LF(6), 0)
		END_IF()
LBL(50)
		USER_FUNC(evt_mobj::evt_mobj_get_position, LW(15), UW(8), LW(0), UW(9))
		IF_EQUAL(LF(2), 1)
			SETF(UW(14), UW(4))
			SETF(UW(15), UW(10))
			IF_EQUAL(UW(13), 1)
				DIVF(UW(15), UW(11))
				MULF(UW(14), UW(15))
			ELSE()
				IF_EQUAL(UW(13), 2)
					DIVF(UW(15), UW(12))
					MULF(UW(14), UW(15))
				END_IF()
			END_IF()
			ADDF(UW(8), UW(14))
		END_IF()
		IF_EQUAL(LF(3), 1)
			SETF(UW(14), LW(10))
			SETF(UW(15), UW(11))
			IF_EQUAL(UW(13), 0)
				DIVF(UW(15), UW(10))
				MULF(UW(14), UW(15))
			ELSE()
				IF_EQUAL(UW(13), 2)
					DIVF(UW(15), UW(12))
					MULF(UW(14), UW(15))
				END_IF()
			END_IF()
			ADDF(LW(0), UW(14))
		END_IF()
		IF_EQUAL(LF(4), 1)
			SETF(UW(14), UW(6))
			SETF(UW(15), UW(12))
			IF_EQUAL(UW(13), 0)
				DIVF(UW(15), UW(10))
				MULF(UW(14), UW(15))
			ELSE()
				IF_EQUAL(UW(13), 1)
					DIVF(UW(15), UW(11))
					MULF(UW(14), UW(15))
				END_IF()
			END_IF()
			ADDF(UW(9), UW(14))
		END_IF()
		USER_FUNC(las_21_set_mobj_position, LW(15), UW(8), LW(0), UW(9))
		SET(LF(7), 0)
		SET(LF(0), 0)
		SET(LF(8), 0)
		IF_EQUAL(LF(2), 1)
			IF_EQUAL(LF(5), 0)
				IF_LARGE_EQUAL(UW(8), UW(1))
					USER_FUNC(evt_mobj::evt_mobj_set_x_position, LW(15), UW(1))
					SET(LF(7), 1)
				END_IF()
			ELSE()
				IF_SMALL_EQUAL(UW(8), UW(1))
					USER_FUNC(evt_mobj::evt_mobj_set_x_position, LW(15), UW(1))
					SET(LF(7), 1)
				END_IF()
			END_IF()
		ELSE()
			SET(LF(7), 1)
		END_IF()
		IF_EQUAL(LF(3), 1)
			IF_EQUAL(LF(1), 0)
				IF_LARGE_EQUAL(LW(0), LW(11))
					USER_FUNC(evt_mobj::evt_mobj_set_y_position, LW(15), LW(11))
					SET(LF(0), 1)
				END_IF()
			ELSE()
				IF_SMALL_EQUAL(LW(0), LW(11))
					USER_FUNC(evt_mobj::evt_mobj_set_y_position, LW(15), LW(11))
					SET(LF(0), 1)
				END_IF()
			END_IF()
		ELSE()
			SET(LF(0), 1)
		END_IF()
		IF_EQUAL(LF(4), 1)
			IF_EQUAL(LF(6), 0)
				IF_LARGE_EQUAL(UW(9), UW(3))
					USER_FUNC(evt_mobj::evt_mobj_set_z_position, LW(15), UW(3))
					SET(LF(8), 1)
				END_IF()
			ELSE()
				IF_SMALL_EQUAL(UW(9), UW(3))
					USER_FUNC(evt_mobj::evt_mobj_set_z_position, LW(15), UW(3))
					SET(LF(8), 1)
				END_IF()
			END_IF()
		ELSE()
			SET(LF(8), 1)
		END_IF()

		// when all axes are done
		IF_EQUAL(LF(7), 0)
			SET(LF(0), 0)
		END_IF()
		IF_EQUAL(LF(8), 0)
			SET(LF(0), 0)
		END_IF()

		USER_FUNC(evt_mobj::evt_mobj_get_position, LW(15), LW(0), LW(1), LW(2))
		USER_FUNC(evt_snd::evt_snd_sfx_pos, LW(8), LW(0), LW(1), LW(2))
		IF_NOT_EQUAL(LF(1), 0)
			USER_FUNC(evt_mario::evt_mario_get_name_hitobj_ride, LW(1))
			IF_STR_EQUAL(LW(1), LW(14))
				USER_FUNC(evt_mario::evt_mario_set_hosei_xyz, FLOAT(0.00), LW(10), FLOAT(0.00))
			END_IF()
			USER_FUNC(evt_mario::evt_mario_get_party, LW(0))
			IF_NOT_EQUAL(LW(0), 0)
				USER_FUNC(evt_party::evt_party_get_name_hitobj_ride, 0, LW(0))
				IF_STR_EQUAL(LW(0), LW(14))
					USER_FUNC(evt_party::evt_party_set_hosei_xyz, 0, FLOAT(0.00), LW(10), FLOAT(0.00))
				END_IF()
			END_IF()
		END_IF()

		IF_EQUAL(LF(0), 1)
			DO_BREAK()
		END_IF()

		ADDF(UW(4), UW(5))
		IF_SMALL_EQUAL(UW(4), UW(17))
			SETF(UW(4), UW(17))
		END_IF()
		IF_LARGE_EQUAL(UW(4), UW(16))
			SETF(UW(4), UW(16))
		END_IF()
		ADDF(LW(10), LW(9))
		IF_SMALL_EQUAL(LW(10), UW(17))
			SETF(LW(10), UW(17))
		END_IF()
		IF_LARGE_EQUAL(LW(10), UW(16))
			SETF(LW(10), UW(16))
		END_IF()
		ADDF(UW(6), UW(7))
		IF_SMALL_EQUAL(UW(6), UW(17))
			SETF(UW(6), UW(17))
		END_IF()
		IF_LARGE_EQUAL(UW(6), UW(16))
			SETF(UW(6), UW(16))
		END_IF()

		WAIT_FRM(1)
	WHILE()
	USER_FUNC(evt_snd::evt_snd_sfxoff, LW(8))
	RUN_EVT(evt_floatblk_stop_se)
	IF_EQUAL(LF(1), 1)
		RUN_EVT(evt_floatblk_shake)
		WAIT_MSEC(1000)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(gon_00_toggle_floatblk_in_gon_01)
	IF_EQUAL(LSWF(8), 0)
		SET(LSWF(8), 1)
	ELSE()
		SET(LSWF(8), 0)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(gon_00_floatblk)
	SET_TYPE(239)
	SET_USER_WRK(GW(15))
	SET(LW(0), PTR("mobj_fltbl01"))
	SET(LW(1), PTR("MOBJ_mobj_fltbl01"))
	SET(LW(2), PTR("mobj_fltsw01"))
	IF_EQUAL(LSWF(8), 0)
		SET(LW(3), 0)
		SET(LW(4), 125)
		SET(LSWF(8), 1)
	ELSE()
		SET(LW(3), 125)
		SET(LW(4), 0)
		SET(LSWF(8), 0)
	END_IF()

	SET(UW(11), 125) // Y distance
	SET(UW(13), 1) // axis with largest distance
	SET(LF(3), 1) // Y
	SETF(UW(16), FLOAT(2.00)) // speed

	RUN_CHILD_EVT(evt_floatblk)
	USER_FUNC(evt_mobj::evt_mobj_get_y_position, PTR("mobj_fltbl01"), LW(0))
	IF_LARGE_EQUAL(LW(0), 125)
		SET(LSWF(8), 1)
	ELSE()
		SET(LSWF(8), 0)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(gon_01_floatblk)
	SET_TYPE(239)
	SET_USER_WRK(GW(15))
	SET(LW(0), PTR("mobj_fltbl01"))
	SET(LW(1), PTR("MOBJ_mobj_fltbl01"))
	SET(LW(2), PTR("mobj_fltsw01"))
	IF_EQUAL(LSWF(8), 0)
		SET(LW(3), 0)
		SET(LW(4), 125)
		SET(LSWF(8), 1)
	ELSE()
		SET(LW(3), 125)
		SET(LW(4), 0)
		SET(LSWF(8), 0)
	END_IF()

	SET(UW(11), 125) // Y distance
	SET(UW(13), 1) // axis with largest distance
	SET(LF(3), 1) // Y
	SETF(UW(16), FLOAT(2.00)) // speed

	RUN_CHILD_EVT(evt_floatblk)
	USER_FUNC(evt_mobj::evt_mobj_get_y_position, PTR("mobj_fltbl01"), LW(0))
	IF_LARGE_EQUAL(LW(0), 125)
		SET(LSWF(8), 1)
	ELSE()
		SET(LSWF(8), 0)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(gon_04_floatblk)
	SET_TYPE(239)
	SET_USER_WRK(GW(15))
	SET(LW(0), PTR("mobj_fltbl03"))
	SET(LW(1), PTR("MOBJ_mobj_fltbl03"))
	SET(LW(2), PTR("mobj_fltbl03"))
	IF_EQUAL(LSWF(9), 0)
		SET(LW(3), 150)
		SET(LW(4), 350)
		SET(LSWF(9), 1)
	ELSE()
		SET(LW(3), 350)
		SET(LW(4), 150)
		SET(LSWF(9), 0)
	END_IF()

	SET(UW(11), 350) // Y distance
	SET(UW(13), 1) // axis with largest distance
	SET(LF(3), 1) // Y
	SETF(UW(16), FLOAT(3.00)) // speed

	RUN_CHILD_EVT(evt_floatblk)
	USER_FUNC(evt_mobj::evt_mobj_get_y_position, PTR("mobj_fltbl03"), LW(0))
	IF_LARGE_EQUAL(LW(0), 350)
		SET(LSWF(9), 1)
	ELSE()
		SET(LSWF(9), 0)
	END_IF()
	RETURN()
EVT_END()

EVT_BEGIN(gon_09_floatblk)
	SET_TYPE(239)
	SET_USER_WRK(GW(15))
	SET(LW(0), PTR("mobj_fltbl01"))
	SET(LW(1), PTR("MOBJ_mobj_fltbl01"))
	SET(LW(2), PTR("mobj_fltsw01"))
	IF_EQUAL(LSWF(13), 0)
		SET(LW(3), 30)
		SET(LW(4), 130)
		SET(LSWF(13), 1)
	ELSE()
		SET(LW(3), 130)
		SET(LW(4), 30)
		SET(LSWF(13), 0)
	END_IF()

	SET(UW(11), 100) // Y distance
	SET(UW(13), 1) // axis with largest distance
	SET(LF(3), 1) // Y
	SETF(UW(16), FLOAT(2.00)) // speed

	RUN_CHILD_EVT(evt_floatblk)
	USER_FUNC(evt_mobj::evt_mobj_get_y_position, PTR("mobj_fltbl01"), LW(0))
	IF_LARGE_EQUAL(LW(0), 130)
		SET(LSWF(13), 1)
	ELSE()
		SET(LSWF(13), 0)
	END_IF()
	RETURN()
EVT_END()

// switches don't light up properly in most places,
// but i'm not feeling up to messing with that (and reed doesn't care much)
EVT_BEGIN(gon_00_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk00"), 0, 260, 0, ItemType::CLOSE_CALL_P, 0, GSWF(5693), 1)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk04"), 0, 60, 0, ItemType::POW_BLOCK, 1, 0, GSWF(5712))
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl01"), 700, LW(0), 0, 3, 2, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl01"), 128)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk01"), 700, -25, 0, ItemType::COIN, 1, 0, GSWF(5710))
	USER_FUNC(evt_mobj::evt_mobj_switch_float_blk, PTR("mobj_fltsw01"), -150, 200, 0, PTR("mobj_fltbl01"), 3, PTR(gon_00_floatblk), LSWF(8))
	RETURN()
EVT_END()

EVT_BEGIN(gon_01_init_hook)
	RUN_CHILD_EVT(evt_damage::evt_gazigazi_entry) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 380, 60, -123, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover"), 4, 430, 60, -123, 0, 0)

	ALLOC_USER_WRK(18, GW(15))
	SET(LW(0), 0)
	IF_EQUAL(LSWF(8), 1)
		ADD(LW(0), 125)
	END_IF()
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl01"), 154, LW(0), 0, 3, 2, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl01"), 128)
	USER_FUNC(evt_mobj::evt_mobj_switch_float_blk, PTR("mobj_fltsw01"), 330, -150, 0, PTR("mobj_fltbl01"), 3, PTR(gon_01_floatblk), LSWF(8))
	RETURN()
EVT_END()

EVT_BEGIN(gon_02_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk01"), 25, 60, 50, ItemType::COIN, 1, 0, GSWF(5701))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk02"), 75, 60, 50, ItemType::COIN, 1, 0, GSWF(5702))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk03"), 200, 60, 0, 0, 3, 0, GSWF(5703))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk04"), 325, 60, -45, ItemType::COIN, 1, 0, GSWF(5704))
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk05"), 350, 60, -45, ItemType::FIRE_POP, 0, GSWF(5705), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk06"), 375, 60, -45, ItemType::COIN, 1, 0, GSWF(5706))
	RETURN()
EVT_END()

EVT_BEGIN(gon_03_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_item::evt_item_entry, PTR("item_00"), ItemType::STAR_PIECE, -250, 0, -160, 16, GSWF(5694), 0)
	RETURN()
EVT_END()

EVT_BEGIN(gon_04_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig

	ALLOC_USER_WRK(18, GW(15))
	SET(LW(0), 150)
	IF_EQUAL(LSWF(9), 1)
		ADD(LW(0), 200)
	END_IF()
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl03"), 573, LW(0), -125, 1, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl03"), 128)
	USER_FUNC(evt_mobj::evt_mobj_switch_float_blk, PTR("mobj_fltsw03"), -200, 400, -125, PTR("mobj_fltbl03"), 1, PTR(gon_04_floatblk), LSWF(9))
	USER_FUNC(evt_mobj::evt_mobj_jumpstand_blue, 0, PTR("mobj_jump00"), 150, 373, 0, -30, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block03"), 75, 0, -75, 0, GSWF(5707), PTR("MOBJ_Lv3BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block04"), 75, 50, -75, 0, GSWF(5708), PTR("MOBJ_Lv3BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block05"), 536, 150, -131, 0, GSWF(5709), PTR("MOBJ_Lv3Block"))
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk05"), 550, 960, 100, ItemType::SUPER_SHROOM, 0, GSWF(5717), 0)
	RETURN()
EVT_END()

// turns out that the game has a 16 mobj entry limit. who knew!
// (patching the dol to increase it to 30 seemed to have no ill effects, so we did that)
EVT_BEGIN(gon_05_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save1"), -220, 1950, -120, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover2"), 1, -170, 1950, -120, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover1"), 4, 50, 60, 0, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(gon_06_init_hook)
	WAIT_FRM(1) // orig
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk05"), -555, 85, -120, ItemType::HP_DRAIN_ITEM, 0, GSWF(5711), 0)
	RETURN()
EVT_END()

EVT_BEGIN(gon_07_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig

	SET(LW(0), 300) // red off
	SET(LW(1), -100) // yellow off
	SET(LW(2), 50) // purple off
	SET(LW(3), 90) // purple off
	SET(LW(4), 40) // purple off
	IF_EQUAL(LSWF(8), 1)
		SUB(LW(0), 50) // red on
	END_IF()
	IF_EQUAL(LSWF(7), 1)
		SUB(LW(1), 150) // yellow on
	END_IF()
	IF_EQUAL(LSWF(9), 1)
		SUB(LW(2), 100) // purple on
	END_IF()
	IF_EQUAL(LSWF(9), 1)
		SUB(LW(3), 180) // purple on
	END_IF()
	IF_EQUAL(LSWF(9), 1)
		SUB(LW(4), 80) // purple on
	END_IF()
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl01"), LW(0), 0, 100, 3, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl02"), LW(0), 50, 100, 3, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl03"), LW(1), 0, 249, 2, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl04"), LW(1), 50, 249, 2, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl05"), 0, 0, -250, 1, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl06"), 0, 50, -250, 1, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl07"), LW(2), 0, -250, 1, 1, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl08"), LW(3), 0, -248, 1, 0, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl09"), LW(4), 50, -248, 1, 0, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl01"), 128) // still don't know what this flag does btw lmao
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl02"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl03"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl04"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl05"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl06"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl07"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl08"), 128)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl09"), 128)
	RETURN()
EVT_END()

EVT_BEGIN(gon_08_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 150, 760, -30, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(gon_09_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig

	ALLOC_USER_WRK(18, GW(15))
	SET(LW(0), 30)
	IF_EQUAL(LSWF(13), 1)
		ADD(LW(0), 100)
	END_IF()
	USER_FUNC(evt_mobj::evt_mobj_float_blk, PTR("mobj_fltbl01"), 550, LW(0), 0, 3, 2, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_flag_onoff, 1, PTR("mobj_fltbl01"), 128)
	USER_FUNC(evt_mobj::evt_mobj_switch_float_blk, PTR("mobj_fltsw01"), -580, 200, -115, PTR("mobj_fltbl01"), 3, PTR(gon_09_floatblk), LSWF(13))
	USER_FUNC(evt_item::evt_item_entry, PTR("item_01"), ItemType::SLOW_SHROOM, -570, 200, 114, 16, GSWF(5713), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk04"), -200, 60, 0, ItemType::COIN, 1, 0, GSWF(5714))
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk05"), -150, 60, 0, ItemType::POW_BLOCK, 0, GSWF(5715), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk06"), -100, 60, 0, ItemType::COIN, 1, 0, GSWF(5716))
	RETURN()
EVT_END()

EVT_BEGIN(gor_01_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk00"), -435, 234, -185, 0, 3, 0, GSWF(5667))
	RETURN()
EVT_END()

EVT_BEGIN(gor_02_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), 175, -50, 350, 0, GSWF(5668), PTR("MOBJ_Lv2BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk00"), 50, 287, -495, 0, 3, 0, GSWF(5669))
	RETURN()
EVT_END()

EVT_BEGIN(hei_00_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk00"), -440, 135, 52, ItemType::MUSHROOM, 0, GSWF(5675), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk01"), -516, 135, -52, ItemType::COIN, 1, 0, GSWF(5676))
	RETURN()
EVT_END()

EVT_BEGIN(hei_13_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block00"), -250, 0, -100, 0, GSWF(5677), PTR("MOBJ_Lv3BigBigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), -250, 0, 90, 0, GSWF(5677), PTR("MOBJ_Lv3Block"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block02"), 205, 0, 0, 0, GSWF(5677), PTR("MOBJ_Lv3BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block03"), 205, 0, 50, 0, GSWF(5677), PTR("MOBJ_Lv3BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block04"), -250, -1000, -100, 0, GSWF(5699), PTR("MOBJ_Lv2Block"))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk01"), -250, 75, -100, ItemType::VOLT_SHROOM, 1, 0, GSWF(5698))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk02"), 300, 60, -100, ItemType::COIN, 1, 0, GSWF(5681))
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk03"), 325, 60, -100, ItemType::COIN, 0, GSWF(5682), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk04"), 350, 60, -100, ItemType::COIN, 1, 0, GSWF(5683))
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk05"), 325, 135, -100, ItemType::HOT_SAUCE, 0, GSWF(5684), 0)
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk06"), -25, 75, -100, ItemType::COIN, 0, GSWF(5685), 0)
	RETURN()
EVT_END()

EVT_BEGIN(hei_01_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_item::evt_item_entry, PTR("item_02"), ItemType::STAR_PIECE, -570, 0, -77, 16, GSWF(5686), 0)
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk00"), -495, 0, 80, ItemType::HONEY_SYRUP, 0, GSWF(5687), 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("blk01"), 275, 0, 118, ItemType::COIN, 1, 0, GSWF(5688))
	RETURN()
EVT_END()

EVT_BEGIN(hei_02_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover"), 2, 735, 110, -80, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 595, 110, 100, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), -620, 50, 130, 0, GSWF(5677), PTR("MOBJ_Lv3BigBlock"))
	USER_FUNC(evt_item::evt_item_entry, PTR("item_01"), ItemType::FIRE_POP, -620, 50, 130, 16, GSWF(5700), 0)
	RETURN()
EVT_END()

EVT_BEGIN(hei_04_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_item::evt_item_entry, PTR("item_00"), ItemType::STAR_PIECE, -250, 0, 122, 16, GSWF(5689), 0)
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover"), 2, 280, 135, 70, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 425, 135, -85, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("mobj_hidden1"), 410, 75, 170, ItemType::THUNDER_RAGE, 11, 0, GSWF(5690))
	RETURN()
EVT_END()

EVT_BEGIN(hei_06_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_item::evt_item_entry, PTR("item_01"), ItemType::STAR_PIECE, -440, 0, 110, 16, GSWF(5697), 0)
	RETURN()
EVT_END()

EVT_BEGIN(hei_10_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 0, 46, -75, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_recovery_blk, PTR("mobj_recover"), 2, 100, 46, -75, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(nok_00_init_hook)
	RUN_CHILD_EVT(evt_damage::evt_gazigazi_entry) // orig
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("mobj_hidden1"), 48, 70, -273, ItemType::FIRE_POP, 11, 0, GSWF(5691))
	RETURN()
EVT_END()

EVT_BEGIN(nok_01_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save"), 390, 110, -190, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_badgeblk, PTR("blk01"), 420, 0, 150, ItemType::HAMMER_THROW, 0, GSWF(5692), 1)
	RETURN()
EVT_END()

EVT_BEGIN(tik_00_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), 432, 170, -170, 0, GSWF(5670), PTR("MOBJ_Lv1BigBlock"))
	RETURN()
EVT_END()

EVT_BEGIN(tik_01_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save2"), 585, 85, 130, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(tik_04_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save1"), 25, 95, -130, 0, 0)
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), -425, 0, 0, 0, GSWF(5677), PTR("MOBJ_Lv3BigBigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block02"), 150, 0, 90, 0, GSWF(5695), PTR("MOBJ_Lv1BigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("mobj_hidden1"), 150, 115, 90, ItemType::FIRE_FLOWER, 11, 0, GSWF(5696))
	RETURN()
EVT_END()

EVT_BEGIN(tik_05_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_save_blk, PTR("mobj_save1"), 350, 60, 175, 0, 0)
	RETURN()
EVT_END()

EVT_BEGIN(tik_13_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("mobj_hidden1"), -64, 2, 25, ItemType::BOOS_SHEET, 11, 0, GSWF(5671))
	RETURN()
EVT_END()

EVT_BEGIN(tik_15_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block02"), -55, 0, -190, 0, GSWF(5672), PTR("MOBJ_Lv1BigBigBlock"))
	USER_FUNC(evt_mobj::evt_mobj_brick, PTR("mobj_hidden1"), -394, 110, -23, ItemType::LIFE_SHROOM, 11, 0, GSWF(5673))
	RETURN()
EVT_END()

EVT_BEGIN(tik_16_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), -300, 0, 0, 0, GSWF(5674), PTR("MOBJ_Lv2BigBlock"))
	RETURN()
EVT_END()

EVT_BEGIN(tik_17_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_mobj::evt_mobj_lv_blk, PTR("block01"), 300, 0, 0, 0, GSWF(5674), PTR("MOBJ_Lv2BigBlock"))
	RETURN()
EVT_END()

EVT_BEGIN(tik_19_init_hook)
	RUN_CHILD_EVT(evt_bero::evt_bero_info_run) // orig
	USER_FUNC(evt_item::evt_item_entry, PTR("blackkey"), ItemType::BLACK_KEY_0021, 0, 10, -87, 16, GSWF(1337), 0)
	RETURN()
EVT_END()

void robjects_oslink_hook(OSModuleInfo* new_module) {
	switch (new_module->id) {
		case ModuleId::GON:
			writeEvtBranch(&gon_gon_00_gon_00_init_evt[8],   gon_00_init_hook); // 805C2338
			writeEvtBranch(&gon_gon_01_gon_01_init_evt[58],  gon_01_init_hook); // 805C3218
			writeEvtBranch(&gon_gon_02_gon_02_init_evt[5],   gon_02_init_hook); // 805C352C
			writeEvtBranch(&gon_gon_03_gon_03_init_evt[8],   gon_03_init_hook); // 805C4A38
			writeEvtBranch(&gon_gon_04_gon_04_init_evt[5],   gon_04_init_hook); // 805C71F8
			writeEvtBranch(&gon_gon_05_gon_05_init_evt[5],   gon_05_init_hook); // 805C8CF4
			writeEvtBranch(&gon_gon_06_gon_06_init_evt[306], gon_06_init_hook); // 805CA728
			writeEvtBranch(&gon_gon_07_gon_07_init_evt[5],   gon_07_init_hook); // 805CB2B4
			writeEvtBranch(&gon_gon_08_gon_08_init_evt[5],   gon_08_init_hook); // 805CC350
			writeEvtBranch(&gon_gon_09_gon_09_init_evt[5],   gon_09_init_hook); // 805CD060

			// in gon_04, remove mobj_hashi0 to avoid going past the mobj limit
			writeEvtGoto(&gon_gon_04_evt_hashi[0], &gon_gon_04_evt_hashi[24] /*805C70DC*/); // 805C707C

			// in gon_04, remove the recovery block
			writeEvtNop(&gon_gon_04_gon_04_init_evt[351]); // 805C7760

			// in gon_05, remove the yellow FlowBlock
			writeEvtGoto(&gon_gon_05_gon_05_init_evt[186], &gon_gon_05_gon_05_init_evt[213] /*805C9034*/); // 805C8FC8

			// fix the height for the purple FlowBlock in gon_04
			gon_gon_04_evt_gon_04flowblock_updown1_init[12] = FLOAT(50.00); // 805C55E8
			gon_gon_04_evt_gon_04flowblock_updown1_init[28] = FLOAT(50.00); // 805C5628
			gon_gon_04_evt_gon_04flowblock_updown1[51] = FLOAT(50.00); // 805C5718
			gon_gon_04_evt_gon_04flowblock_updown1[68] = FLOAT(50.00); // 805C575C

			// fix the height for the yellow FlowBlock in gon_04
			gon_gon_04_evt_gon_04flowblock_updown2_init[12] = FLOAT(250.00); // 805C5DC0
			gon_gon_04_evt_gon_04flowblock_updown2_init[28] = FLOAT(250.00); // 805C5E00
			gon_gon_04_evt_gon_04flowblock_updown2[51] = FLOAT(250.00); // 805C5EF0
			gon_gon_04_evt_gon_04flowblock_updown2[68] = FLOAT(250.00); // 805C5F34

			// (autogen) increase the speed for the big green block in gon_05
			gon_gon_05_main_1[57] = FLOAT(3.00); // 805C8554
			gon_gon_05_main_1[248] = FLOAT(3.00); // 805C8850
			gon_gon_05_main_1[280] = FLOAT(3.00); // 805C88D0
			gon_gon_05_main_1[283] = FLOAT(3.00); // 805C88DC
			break;
		case ModuleId::GOR:
			writeEvtBranch(&gor_gor_01_gor_01_init_evt[113], gor_01_init_hook); // 805F6D64
			writeEvtBranch(&gor_gor_02_gor_02_init_evt[43],  gor_02_init_hook); // 80607FA0
			break;
		case ModuleId::HEI:
			writeEvtBranch(&hei_hei_00_hei_00_init_evt[14],  hei_00_init_hook); // 805C6FC4
			writeEvtBranch(&hei_hei_01_hei_01_init_evt[5],   hei_01_init_hook); // 805C97BC
			writeEvtBranch(&hei_hei_02_hei_02_init_evt[5],   hei_02_init_hook); // 805C9D70
			writeEvtBranch(&hei_hei_04_hei_04_init_evt[5],   hei_04_init_hook); // 805CACC8
			writeEvtBranch(&hei_hei_06_hei_06_init_evt[5],   hei_06_init_hook); // 805CB990
			writeEvtBranch(&hei_hei_10_hei_10_init_evt[5],   hei_10_init_hook); // 805CE928
			writeEvtBranch(&hei_hei_13_hei_13_init_evt[5],   hei_13_init_hook); // 805CF32C
			break;
		case ModuleId::NOK:
			writeEvtBranch(&nok_nok_00_nok_00_init_evt[192], nok_00_init_hook); // 805C023C
			writeEvtBranch(&nok_nok_01_nok_01_init_evt[5],   nok_01_init_hook); // 805C3E9C
			break;
		case ModuleId::TIK:
			writeEvtBranch(&tik_tik_00_tik_00_init_evt[31],  tik_00_init_hook); // 805CDD14
			writeEvtBranch(&tik_tik_01_tik_01_init_evt[32],  tik_01_init_hook); // 805CF4B8
			writeEvtBranch(&tik_tik_04_tik_04_init_evt[16],  tik_04_init_hook); // 805D3268
			writeEvtBranch(&tik_tik_05_tik_05_init_evt[5],   tik_05_init_hook); // 805D96A4
			writeEvtBranch(&tik_tik_13_tik_13_init_evt[16],  tik_13_init_hook); // 805DBBC0
			writeEvtBranch(&tik_tik_15_tik_15_init_evt[16],  tik_15_init_hook); // 805E637C
			writeEvtBranch(&tik_tik_16_tik_16_init_evt[20],  tik_16_init_hook); // 805E6A64
			writeEvtBranch(&tik_tik_17_tik_17_init_evt[20],  tik_17_init_hook); // 805E7224

			// move the black key to behind the dark chest
			writeEvtNop(&tik_tik_04_tik_04_init_evt[133]); // 805D343C
			writeEvtBranch(&tik_tik_19_tik_19_init_evt[16],  tik_19_init_hook); // 805E83E8
			break;
	}
}

void robjects_init() {
	// add support for a medium-size red block
	patch::hookInstruction(
		(void *)0x80069360, // evt_mobj::evt_mobj_float_blk
		[](gc::os::OSContext* context, void* user) {
			if (context->gpr[27] == 1) { // size is medium
				context->gpr[27] = (uint32_t)"MOBJ_red_big_FlowBlock";
				context->srr0 += 4; // skip instr that sets r27 to the big02 variant ptr
			}
		}
	);
}