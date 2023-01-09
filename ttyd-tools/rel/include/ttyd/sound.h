#pragma once

#include <cstdint>

namespace ttyd::sound {

extern "C" {

// .text
// SoundAIDMACallback
// SoundSSMainInt
// SoundSSMain
// SoundSSMuteOff
// SoundSSMuteOn
// SoundSSCheck
// SoundSSSetPlayFreqCh
// SoundSSSetSrndPanCh
// SoundSSGetVolCh
void SoundSSSetVolCh(int32_t stream_id, uint8_t vol);
// SoundSSSetPanCh
// SoundSSContinueCh
// SoundSSFadeoutCh
// SoundSSStopCh
// SoundSSPlayChEx_main
// SoundSSPlayChEx
// SoundSSPlayCh
// _sscallback
// ssDecodeDPCM
// _ssDVDReadAsync_cache_next
// _ssDVDReadAsync_cache_aram
// cache_flush
// _ssDVDReadAsync_activeChk
// SoundDVDMain
// SoundEfxMain
// SoundEfxCheck
// SoundEfxSetLPF
// SoundEfxSetSrndPan
// SoundEfxSetPan
// SoundEfxSetAux1
// SoundEfxGetVolume
// SoundEfxSetVolume
// SoundEfxSetPitch
// SoundEfxStop
// SoundEfxPlayEx
// SoundSongCheck
// SoundSongGetVolCh
// SoundSongSetVolCh
// SoundSongFadeinCh
// SoundSongFadeoutCh
// SoundSongStopCh
// SoundSongContinueCh
// SoundSongPlayCh
// SoundCloseCover
// SoundOpenCover
// SoundSetOutputMode
// SoundDropData
// SoundSLibLoadDVD
// SoundLoadDVD2Free
// SoundLoadDVD2PushGroup
// SoundLoadDVD2
// loadDVD_callback
// SoundSetFadeTime
// SoundMainInt
// SoundMain
// SoundInit
// sndFree
// sndMalloc

// .sdata
// dtbl

// .bss
// str$436
// sound
// dly
// cho
// revS
// revH

// .sbss
// owner_no$1419
// aramMemArray

}

}