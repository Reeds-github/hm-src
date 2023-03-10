#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::fadedrv {

namespace FadeType {
	enum e {
		NONE,
		BLACK_MARIO_HEAD_FADE_IN,
		BLACK_MARIO_HEAD_FADE_OUT,
		BLACK_CIRCLE_FADE_IN,
		BLACK_CIRCLE_FADE_OUT,
		BLACK_CIRCLE_FADE_IN2,
		BLACK_CIRCLE_FADE_OUT2,
		BLACK_CIRCLE_FADE_IN3,
		BLACK_CIRCLE_FADE_OUT3,
		BLACK_FADE_IN,
		BLACK_FADE_OUT,
		BLACK_FADE_IN2,
		BLACK_FADE_OUT2,
		BLACK_STATIC,
		BLACK_FADE_IN3,
		BLACK_FADE_OUT3,
		WHITE_FADE_IN,
		WHITE_FADE_OUT,
		WHITE_FADE_IN2,
		WHITE_FADE_OUT2,
		WHITE_FADE_IN3,
		WHITE_FADE_OUT3,
		CURTAIN_PULL_DOWN,
		CURTAIN_PULL_UP,
		CURTAIN_PULL_LEFT,
		CURTAIN_PULL_RIGHT,
		CURTAIN_STATIC,
		CURTAIN_PULL_LEFT2,
		CURTAIN_PULL_RIGHT2,
		TITLESCREEN_CURTAIN_OPEN,
		TITLESCREEN_CURTAIN_CLOSE,
		TITLESCREEN_CURTAIN_OPEN_STATIC,
		TITLESCREEN_CURTAIN_CLOSED_STATIC,
		TITLESCREEN_CURTAIN_LEAVE_GRADUAL,
		TITLESCREEN_CURTAIN_APPEAR_GRADUAL,
		TITLESCREEN_CURTAIN_OPEN2,
		TITLESCREEN_CURTAIN_CLOSE2,
		TITLESCREEN_CURTAIN_OPEN_STATIC2,
		TITLESCREEN_CURTAIN_CLOSED_STATIC2,
		TITLESCREEN_CURTAIN_LEAVE_GRADUAL2,
		TITLESCREEN_CURTAIN_APPEAR_GRADUAL2,
		ENTER_PIPE_FROM_UP,
		EXIT_PIPE_FROM_UP,
		ENTER_PIPE_FROM_DOWN,
		EXIT_PIPE_FROM_DOWN,
		ENTER_PIPE_FROM_LEFT,
		EXIT_PIPE_FROM_LEFT,
		ENTER_PIPE_FROM_RIGHT,
		EXIT_PIPE_FROM_RIGHT,
		FREEZE_SCREEN,
		UNFREEZE_SCREEN_PULL_TOP_RIGHT,
		DEMO_TITLE_FADE_OUT,
		MARIO_HEAD_FADE_IN,
		MARIO_HEAD_FADE_OUT,
		PEACH_HEAD_FADE_IN,
		PEACH_HEAD_FADE_OUT,
		BOWSER_HEAD_FADE_IN,
		BOWSER_HEAD_FADE_OUT,
		MARIO_HEAD_STATIC_BLINKING,
		PEACH_HEAD_STATIC,
		BOWSER_HEAD_STATIC,
		CURTAIN_PULL_LEFT3,
		CURTAIN_PULL_RIGHT3,
	};
};

extern "C" {

// fadeSetNarrowFast
// fadeGetTpl
void fadeReset(uint32_t unk);
// fadeIsFinish
// disp_dummy_black
// disp_tec
// disp_maku
// zFill
// z
// disp_texture
// fadeMain
// fadeSetOffscreenCallback
// fadeSetAnimVirtualPos
// fadeSetAnimOfsPos
// fadeSetSpotPos
// fadeWhiteOutOff
// fadeWhiteOutOn
// fadeSoftFocusOff
// fadeSoftFocusOn
// fadeTecSoftOff
// fadeTecSoftOn
// fadeTecOff
// fadeTecOn
void fadeEntry(uint32_t fadeType, int32_t duration, gc::color4* color);
// fadeTexSetup
// fadeInit
// _callback

}

}