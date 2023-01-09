#include "mod.h"
#include "patch.h"

#include <gc/OSLink.h>

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_detect.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_unit.h>
#include <ttyd/dvdmgr.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/evtmgr.h>
#include <ttyd/memory.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sac_genki.h>
#include <ttyd/seqdrv.h>
#include <ttyd/sound.h>

#include <hm/common.h>
#include <hm/heromode.h>
#include <hm/hookgloom_fakeout.h>

#include "common_types.h"
#include "evt_cmd.h"

#include <cstring>

using namespace mod;
using namespace ttyd;
using namespace battle;
using namespace ttyd::battle_unit;
using namespace ttyd::battle_database_common;

using gc::OSLink::OSModuleInfo;
using pmario_sound::PSndBgmEntry;

extern "C" {
	extern BattleUnitSetup gon_battle_database_gon_btlparty_gon_gon_11_01_off_1;

	extern BattleUnitKind gon_unit_boss_gonbaba_unit_boss_gonbaba;
	extern int32_t gon_unit_boss_gonbaba_negotiation_event[];
	extern int32_t gon_unit_boss_gonbaba_dead_event[];
	extern int32_t gon_unit_boss_gonbaba_init_event[];

	extern BattleUnitKind las_unit_boss_bunbaba_unit_boss_bunbaba;
	extern int32_t las_unit_boss_bunbaba_init_event[];
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_poison_attack;
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_fumituke_attack;
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_kamituki_attack;
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_eq_attack;
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_charge;
	extern BattleWeapon las_unit_boss_bunbaba_weapon_bunbaba_mega_attack;
	extern int32_t las_unit_boss_bunbaba_battle_entry_event[];
	extern int32_t* las_unit_boss_bunbaba_data_table[];
	extern int32_t las_unit_boss_bunbaba_dead_event[];
}

EVT_DECLARE_USER_FUNC(hgf_btlseqend_bullshit, 0)
EVT_DEFINE_USER_FUNC(hgf_btlseqend_bullshit) {
	auto* mario = BattleGetMarioPtr(g_BattleWork);
	auto* party = BattleGetPartyPtr(g_BattleWork);

	if (isFirstCall) {
		BattleWorkUnit* unit = mario;
		int8_t times = 2;
		while (times > 0) {
			if (unit != nullptr) {
				void* winReaction = BtlUnit_GetData(unit, 0x40);
				evtmgr::EvtEntry* entry = evtmgr::evtEntry(winReaction, 10, 0);
				if (entry != nullptr) {
					entry->wActorThisPtr = unit->unit_id;
					unit->attack_evt_tid = entry->threadId;
				}
			}
			if (times == 2) unit = party;
			times--;
		}
	}

	bool unfinished = evtmgr::evtCheckID(mario->attack_evt_tid) || evtmgr::evtCheckID(party->attack_evt_tid);
	if (!unfinished) {
		mario->attack_evt_tid = 0;
		party->attack_evt_tid = 0;
	}

	return unfinished ? 0 : 2;
}

EVT_DECLARE_USER_FUNC(evt_snd_bgmon_f_d, 4)
EVT_DEFINE_USER_FUNC(evt_snd_bgmon_f_d) {
	pmario_sound::psndBGMOn_f_d(
		evt->currentCommandArguments[0],
		(const char *)evt->currentCommandArguments[1],
		evt->currentCommandArguments[2],
		evt->currentCommandArguments[3]
	);
	return 2;
}

PSndBgmEntry* bgmEntryBackup = nullptr;
EVT_DECLARE_USER_FUNC(bkp_bgm1, 0)
EVT_DEFINE_USER_FUNC(bkp_bgm1) {
	bgmEntryBackup = (PSndBgmEntry *)memory::__memAlloc(0, sizeof(PSndBgmEntry));
	memcpy(bgmEntryBackup, &pmario_sound::psbgm[1], sizeof(PSndBgmEntry));
	pmario_sound::psbgm[1].field_0x0 = -1;
	return 2;
}

EVT_DECLARE_USER_FUNC(restore_bgm1, 0)
EVT_DEFINE_USER_FUNC(restore_bgm1) {
	memcpy(&pmario_sound::psbgm[1], bgmEntryBackup, sizeof(PSndBgmEntry));
	memory::__memFree(0, bgmEntryBackup);
	bgmEntryBackup = nullptr;
	return 2;
}

EVT_DECLARE_USER_FUNC(fadeout_bgm1_bkp, 0)
EVT_DEFINE_USER_FUNC(fadeout_bgm1_bkp) {
	auto vol = (bgmEntryBackup->field_0xe_volume * bgmEntryBackup->field_0x24_remainingFadeoutFramesCounter) / bgmEntryBackup->field_0x22_fadeoutFrames;
	sound::SoundSSSetVolCh(bgmEntryBackup->field_0x4_streamId, vol);
	bgmEntryBackup->field_0x24_remainingFadeoutFramesCounter -= 1;

	// 0 volume seems to unload the file which is an issue, and you can't even hear anything at vol 1
	// alternatively, run restore in a thread? but i'd honestly rather do this
	return vol == 1 ? 2 : 0;
}

// i'm skipping a lot of the victory stuff in here because it really doesn't need to be perfect
EVT_BEGIN(hgf_fakeout)
	// run death animation by jumping to the middle of the death event
	RUN_CHILD_EVT(&las_unit_boss_bunbaba_dead_event[114]) // 8061E34C

	// setup cam from btlseqEnd for fake victory (btlseqEnd state 0x7000008)
	WAIT_MSEC(2000)
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 1)
	USER_FUNC(battle_camera::evt_btl_camera_wait_move_end)
	USER_FUNC(battle_camera::evt_btl_camera_set_moveSpeedLv, 1, 2)
	USER_FUNC(battle_camera::evt_btl_camera_set_mode, 1, 0x11)
	USER_FUNC(battle_camera::evt_btl_camera_set_posoffset, 1, 0, 0, 0)

	USER_FUNC(evt_snd::evt_snd_bgmoff, 0x401) // pause battle bgm
	USER_FUNC(bkp_bgm1)
	INLINE_EVT()
		USER_FUNC(fadeout_bgm1_bkp)
	END_INLINE()
	WAIT_MSEC(100)
	USER_FUNC(evt_snd::evt_snd_bgmon, 1, PTR("BGM_BATTLE_WIN1")) // victory fanfare

	USER_FUNC(sac_genki::status_on)
	USER_FUNC(battle_camera::evt_btl_camera_wait_move_end)
	WAIT_MSEC(1100)
	USER_FUNC(hgf_btlseqend_bullshit) // let's be real this kinda sucks, but uh, ✨ it works ✨
	WAIT_MSEC(1300)

	// pretty much 1:1 to the logic in btlseqEnd (0x700001a)
	USER_FUNC(battle_event_cmd::btlevtcmd_GetBodyId, -3, LW(1))
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePose, -3, LW(1), PTR("M_V_1"))
	WAIT_FRM(60)
	// set anims to running away
	USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePoseType, -3, LW(1), 0x2a)
	USER_FUNC(battle_event_cmd::btlevtcmd_GetPartnerId, -3, LW(1))
	IF_NOT_EQUAL(LW(1), -1) // has a party member battling
		USER_FUNC(battle_event_cmd::btlevtcmd_GetUnitKind, -4, LW(1))
		IF_NOT_EQUAL(LW(1), 0xE1) // is not koops
			SET(LW(1), -2)
		ELSE()
			USER_FUNC(battle_event_cmd::btlevtcmd_GetOverTurnCount, -4, LW(1))
			IF_SMALL(LW(1), 1)
				SET(LW(1), -2)
			END_IF()
		END_IF()

		IF_EQUAL(LW(1), -2)
			USER_FUNC(battle_event_cmd::btlevtcmd_GetBodyId, -4, LW(1))
			USER_FUNC(battle_event_cmd::btlevtcmd_AnimeChangePoseType, -4, LW(1), 0x2a)
		END_IF()
	END_IF()

	USER_FUNC(evt_fade::evt_fade_out, 200) // from 0x700001d
	USER_FUNC(battle_camera::evt_btl_camera_set_prilimit, 0)
	// set anims back to idle
	USER_FUNC(battle_event_cmd::btlevtcmd_StartWaitEvent, -3)
	USER_FUNC(battle_event_cmd::btlevtcmd_StartWaitEvent, -4)
	WAIT_MSEC(4000)
	USER_FUNC(restore_bgm1)
	USER_FUNC(evt_snd::evt_snd_sfxon, PTR("SFX_EVT_NPC_QUESTION_MARK1"), LW(1))
	USER_FUNC(evt_snd::evt_snd_sfx_vol, LW(1), 0xFF)
	WAIT_MSEC(900)
	USER_FUNC(evt_snd_bgmon_f_d, 0x121, 0, 0, 0) // resume battle bgm (no fade)
	USER_FUNC(evt_fade::evt_fade_in, 0)

	RUN_CHILD_EVT(gon_unit_boss_gonbaba_negotiation_event) // run eating animation
	RETURN()
EVT_END()

OSModuleInfo* hgfRel = nullptr;
void* hgfRelBss = nullptr;
BattleUnitKindPart* hgfParts = nullptr;

void hgf_oslink_hook(OSModuleInfo* new_module) {
	if (new_module->id == ModuleId::GON && !strcmp(seqdrv::nextMap, "gon_11")) {
		hgf_load_custom_rel();
		hgf_patch();
	}
}

void hgf_osunlink_hook(OSModuleInfo* module) {
	if (module->id == ModuleId::GON && hgfRel != nullptr) {
		hgf_free();
	}
}

void hgf_patch() {
	las_unit_boss_bunbaba_unit_boss_bunbaba.max_hp = 70;
	gon_unit_boss_gonbaba_negotiation_event[927] = 40; // change respawn hp (805D3D40)

	las_unit_boss_bunbaba_unit_boss_bunbaba.parts->defense[0] = 0;
	las_unit_boss_bunbaba_unit_boss_bunbaba.level = 68;
	las_unit_boss_bunbaba_unit_boss_bunbaba.pb_soft_cap = 5;

	memset(las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability, 0, sizeof(StatusVulnerability));
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->attack_up = 100;
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->defense_up = 100;
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->allergic = 100;
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->fast = 100;
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->poison = 100;
	las_unit_boss_bunbaba_unit_boss_bunbaba.status_vulnerability->invisible = 100;

	las_unit_boss_bunbaba_weapon_bunbaba_poison_attack.superguards_allowed = 0;
	las_unit_boss_bunbaba_weapon_bunbaba_poison_attack.damage_function_params[0] = 4;
	las_unit_boss_bunbaba_weapon_bunbaba_poison_attack.poison_chance = 100;
	las_unit_boss_bunbaba_weapon_bunbaba_poison_attack.poison_time = 2;
	las_unit_boss_bunbaba_weapon_bunbaba_poison_attack.poison_strength = 1;

	las_unit_boss_bunbaba_weapon_bunbaba_fumituke_attack.superguards_allowed = 0;
	las_unit_boss_bunbaba_weapon_bunbaba_fumituke_attack.damage_function_params[0] = 4;

	las_unit_boss_bunbaba_weapon_bunbaba_kamituki_attack.superguards_allowed = 0;
	las_unit_boss_bunbaba_weapon_bunbaba_kamituki_attack.damage_function_params[0] = 4;
	las_unit_boss_bunbaba_weapon_bunbaba_kamituki_attack.damage_pattern = 0x05; // Knocked onto ground

	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.superguards_allowed = 0;
	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.damage_function_params[0] = 4;
	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.special_property_flags |= 0x40; // PiercesDefense
	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.slow_chance = 100;
	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.slow_time = 2;
	las_unit_boss_bunbaba_weapon_bunbaba_eq_attack.damage_pattern = 0x05; // Knocked onto ground

	las_unit_boss_bunbaba_weapon_bunbaba_charge.charge_strength = 2;
	las_unit_boss_bunbaba_weapon_bunbaba_charge.fast_chance = 100;
	las_unit_boss_bunbaba_weapon_bunbaba_charge.fast_time = 2;
	las_unit_boss_bunbaba_weapon_bunbaba_charge.def_change_chance = 100;
	las_unit_boss_bunbaba_weapon_bunbaba_charge.def_change_time = 2;
	las_unit_boss_bunbaba_weapon_bunbaba_charge.def_change_strength = 2;

	las_unit_boss_bunbaba_weapon_bunbaba_mega_attack.superguards_allowed = 0;
	las_unit_boss_bunbaba_weapon_bunbaba_mega_attack.damage_function_params[0] = 4;

	// replace hooktail with gloomtail
	gon_battle_database_gon_btlparty_gon_gon_11_01_off_1.unit_kind_params = &las_unit_boss_bunbaba_unit_boss_bunbaba;

	// copy hooktail's eaten toad part to gloomtail
	auto og_part_count = las_unit_boss_bunbaba_unit_boss_bunbaba.num_parts;
	hgfParts = (BattleUnitKindPart *)memory::__memAlloc(0, sizeof(BattleUnitKindPart) * (og_part_count + 1));
	memcpy(hgfParts, las_unit_boss_bunbaba_unit_boss_bunbaba.parts, sizeof(BattleUnitKindPart) * og_part_count);
	memcpy(&hgfParts[og_part_count], &gon_unit_boss_gonbaba_unit_boss_gonbaba.parts[2], sizeof(BattleUnitKindPart));
	las_unit_boss_bunbaba_unit_boss_bunbaba.parts = hgfParts;
	las_unit_boss_bunbaba_unit_boss_bunbaba.num_parts += 1;

	// branch from the gloomtail init event to part of the hooktail init event to init the audience, then return early
	writeEvtBranch(&las_unit_boss_bunbaba_init_event[24] /*8061E034*/, &gon_unit_boss_gonbaba_init_event[20] /*805D5E9C*/);
	writeEvtReturn(&gon_unit_boss_gonbaba_init_event[115]); // 805D6018

	// replace gloomtail's death event with hooktail's for the nego evt check + og movement
	las_unit_boss_bunbaba_data_table[3] = gon_unit_boss_gonbaba_dead_event;

	// replace negotiation event with the fakeout
	writeEvtBranch(&gon_unit_boss_gonbaba_dead_event[8] /*805D5A00*/, hgf_fakeout);

	// return early from the normal death event to allow using it for the fakeout
	writeEvtReturn(&las_unit_boss_bunbaba_dead_event[239]); // 8061E540

	// skip talk in negotiation event
	writeEvtGoto(&gon_unit_boss_gonbaba_negotiation_event[13] /*805D2EF8*/, &gon_unit_boss_gonbaba_negotiation_event[30] /*805D2F3C*/);
	writeEvtGoto(&gon_unit_boss_gonbaba_negotiation_event[43] /*805D2F70*/, &gon_unit_boss_gonbaba_negotiation_event[618] /*805D386C*/);
	writeEvtGoto(&gon_unit_boss_gonbaba_negotiation_event[627] /*805D3890*/, &gon_unit_boss_gonbaba_negotiation_event[755] /*805D3A90*/);
}

// TODO: dedup this and the kkb one
void hgf_load_custom_rel() {
	// load bunbaba unit rel into memory
	hgfRel = (OSModuleInfo *)readFileFromDvd("/mod/boss_bunbaba.rel");

	// allocate bss and link the rel
	hgfRelBss = memory::__memAlloc(0, hgfRel->bss_size);
	g_OSLink_trampoline(hgfRel, hgfRelBss);
}

void hgf_free() {
	memory::__memFree(0, hgfParts);
	gc::OSLink::OSUnlink(hgfRel);
	memory::__memFree(0, hgfRelBss);
	memory::__memFree(0, hgfRel);
	hgfParts = nullptr;
	hgfRelBss = nullptr;
	hgfRel = nullptr;
}