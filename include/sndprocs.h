/* NetHack 5.0	sndprocs.h	$NHDT-Date: 1781973088 2026/06/20 16:31:28 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.28 $ */
/* Copyright (c) Michael Allison, 2022                                */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sndprocs.h
 * @brief The interface between the game and whatever makes sound.
 *
 * Arranged exactly like the display interface, and for the same reason: the game says what happened and something else decides what that sounds like. A table of function pointers
 * is installed, and which table decides whether the game is silent, plays samples, speaks, or emits console codes for an external program to interpret.
 *
 * What the game reports is not sounds but occasions -- an achievement, an effect, an ambience, something spoken, a tune the hero played. That division is the point: the core knows
 * a door was kicked and knows nothing about audio files.
 *
 * As with the display, a sound library declares what it can manage. The core checks before reporting, because a library given an occasion it cannot handle would be a silence the
 * game thinks was a sound.
 *
 * @note There is always a table installed, including one that does nothing. That is why no caller tests whether sound exists before reporting to it.
 * @note Speech carries a good deal beyond the words -- who is speaking, in what tone, how loudly. A speaking library needs that to sound like the monster rather than like a
 *       narrator.
 * @warning The two instrument lists are not alternatives to choose between. One is the subset the game actually uses and the other is the full standard set, disabled -- kept for
 *          reference, so a new tune has names to reach for.
 */

/**
 * @file sndprocs.h
 * @brief 게임과 소리를 내는 것 사이의 인터페이스.
 *
 * 표시부 인터페이스와 똑같이 짜여 있고 이유도 같다. 게임은 무슨 일이 있었는지 말하고 다른 것이 그것이 어떤 소리인지 정한다. 함수 포인터의 표가 설치되며, 어느 표인지가 게임이 조용한지, 샘플을 재생하는지, 말하는지, 외부 프로그램이 해석할 콘솔 코드를 내보내는지를 정한다.
 *
 * 게임이 알리는 것은 소리가 아니라 계기다. 업적, 효과, 분위기, 말해진 것, 영웅이 연주한 곡조. 그 분업이 요점이다. 코어는 문이 차였다는 것을 알고 오디오 파일에 대해서는 아무것도 모른다.
 *
 * 표시부와 마찬가지로 사운드 라이브러리가 자신이 무엇을 감당할 수 있는지 선언한다. 코어는 알리기 전에 확인한다. 다룰 수 없는 계기를 받은 라이브러리는 게임이 소리였다고 여기는 침묵이 되기 때문이다.
 *
 * @note 아무것도 하지 않는 것을 포함해 항상 표가 설치되어 있다. 그래서 어느 호출자도 알리기 전에 소리가 존재하는지 검사하지 않는다.
 * @note 말은 낱말 이상을 상당히 지닌다. 누가 말하는지, 어떤 어조로, 얼마나 크게. 말하는 라이브러리는 해설자가 아니라 그 몬스터처럼 들리기 위해 그것을 필요로 한다.
 * @warning 두 악기 목록은 골라야 할 대안이 아니다. 하나는 게임이 실제로 쓰는 부분집합이고 다른 하나는 비활성화된 전체 표준 묶음이다. 참고용으로 남겨져 있어, 새 곡조가 손을 뻗을 이름을 갖는다.
 */

#ifndef SNDPROCS_H
#define SNDPROCS_H

/**
 * @brief Which sound library this is.
 * @note The list is conditional on what was built in, so its values differ between builds. That is safe because they are only ever compared with each other, never saved or shown.
 * @note The silent entry is first and always present, so there is always something installable.
 */
/**
 * @brief 이것이 어느 사운드 라이브러리인지.
 * @note 이 목록은 무엇이 빌드에 포함되었는지에 조건적이므로 그 값들이 빌드마다 다르다. 그것들이 오직 서로 비교되기만 하고 저장되거나 보여지지 않으므로 안전하다.
 * @note 조용한 항목이 첫 번째이고 항상 존재하므로, 설치할 수 있는 것이 항상 있다.
 */
enum soundlib_ids {
    soundlib_nosound,
#ifdef SND_LIB_PORTAUDIO
    soundlib_portaudio,
#endif
#ifdef SND_LIB_OPENAL
    soundlib_openal,
#endif
#ifdef SND_LIB_SDL_MIXER
    soundlib_sdl_mixer,
#endif
#ifdef SND_LIB_MINIAUDIO
    soundlib_miniaudio,
#endif
#ifdef SND_LIB_FMOD
    soundlib_fmod,
#endif
#ifdef SND_LIB_SOUND_ESCCODES
    soundlib_sound_esccodes,
#endif
#ifdef SND_LIB_VISSOUND
    soundlib_vissound,
#endif
#ifdef SND_LIB_WINDSOUND
    soundlib_windsound,
#endif
#ifdef SND_LIB_MACSOUND
    soundlib_macsound,
#endif
#ifdef SND_LIB_QTSOUND
    soundlib_qtsound,
#endif
    soundlib_notused
};

/**
 * @brief One complete sound library: what it is called, what it can do, and every routine it provides.
 *
 * The six routines correspond to the six kinds of occasion the game reports. They are separate rather than one general routine because they carry different information -- an
 * achievement is identified by what it was, an effect by a description and a volume, speech by the words and the speaker.
 *
 * @note @c sound_verbal is the one that makes accessibility possible: a library implementing it can speak what would otherwise only be printed.
 * @note @c sound_ambience takes a proximity as well as an identity, so a library can place a continuous sound rather than only start and stop it.
 */
/**
 * @brief 하나의 완전한 사운드 라이브러리. 무엇이라 불리는지, 무엇을 할 수 있는지, 그리고 제공하는 모든 루틴.
 *
 * 여섯 루틴이 게임이 알리는 여섯 종류의 계기에 대응한다. 하나의 일반 루틴이 아니라 따로 있는 것은 서로 다른 정보를 지니기 때문이다. 업적은 그것이 무엇이었는지로 식별되고, 효과는 기술과 음량으로, 말은 낱말과 말하는 이로.
 *
 * @note @c sound_verbal 이 접근성을 가능하게 하는 것이다. 그것을 구현하는 라이브러리는 그러지 않으면 인쇄만 될 것을 말할 수 있다.
 * @note @c sound_ambience 는 정체와 함께 근접도도 받는다. 그래서 라이브러리가 지속되는 소리를 시작하고 멈추는 것만이 아니라 그것을 배치할 수 있다.
 */
struct sound_procs {
    /**
     * @brief The name a player writes to ask for this library.
     */
    /**
     * @brief 플레이어가 이 라이브러리를 요청하기 위해 적는 이름.
     */
    const char *soundname;
    /**
     * @brief Which library this is, for code that must ask.
     */
    /**
     * @brief 이것이 어느 라이브러리인지. 물어야 하는 코드를 위해.
     */
    enum soundlib_ids soundlib_id;
    /**
     * @brief Which kinds of occasion this library can actually handle.
     * @note Checked before reporting, so an unsupported occasion is not reported rather than being reported and dropped -- the difference matters because the core sometimes does
     *       something else instead, such as printing what it would have spoken.
     */
    /**
     * @brief 이 라이브러리가 실제로 다룰 수 있는 계기의 종류가 무엇인지.
     * @note 알리기 전에 확인되므로, 지원되지 않는 계기는 알려진 뒤 버려지는 것이 아니라 알려지지 않는다. 그 차이가 중요한 것은, 코어가 때때로 대신 다른 일을 하기 때문이다. 예컨대 말했을 것을 인쇄하는 것.
     */
    unsigned long sound_triggers; /* capabilities in the port */
    void (*sound_init_nhsound)(void);
    void (*sound_exit_nhsound)(const char *);
    void (*sound_achievement)(schar, schar, int32_t);
    void (*sound_soundeffect)(char *desc, int32_t, int32_t volume);
    void (*sound_hero_playnotes)(int32_t instrument, const char *str,
                                 int32_t volume);
    void (*sound_play_usersound)(char *filename, int32_t volume, int32_t idx);
    void (*sound_ambience)(int32_t ambience_action, int32_t ambienceid,
                           int32_t proximity);
    void (*sound_verbal)(char *text, int32_t gender, int32_t tone,
                         int32_t vol, int32_t moreinfo);
};

/**
 * @brief How one speaker should sound, so that the same monster sounds the same each time.
 *
 * A speaking library needs a voice, not just words. Keeping it per monster means a shopkeeper sounds like that shopkeeper on every visit rather than being reassigned a voice each
 * time it speaks -- which is what makes speech identify who is talking.
 *
 * @note The serial number exists so a voice can be recognised as the same one across a save, where a pointer cannot be.
 * @note The name is kept alongside the monster because a monster may die and still need to have been someone.
 */
/**
 * @brief 한 명의 말하는 이가 어떻게 들려야 하는지. 같은 몬스터가 매번 같게 들리도록.
 *
 * 말하는 라이브러리는 낱말만이 아니라 목소리를 필요로 한다. 그것을 몬스터마다 보관하면 상점 주인이 방문할 때마다 그 상점 주인처럼 들린다. 말할 때마다 목소리가 다시 배정되는 것이 아니다. 그것이 말이 누가 이야기하는지 알려 주게 하는 것이다.
 *
 * @note 일련번호가 있는 것은, 포인터가 그럴 수 없는 저장을 건너서도 목소리가 같은 것으로 인식될 수 있게 하기 위함이다.
 * @note 이름이 몬스터 곁에 보관되는 것은, 몬스터가 죽어도 여전히 누군가였어야 하기 때문이다.
 */
struct sound_voice {
    int32_t serialno;
    int32_t gender;
    int32_t tone;
    int32_t volume;
    int32_t moreinfo;
    struct monst *mon;
    const char *nameid;
};

extern struct sound_procs sndprocs;

/**
 * @def SOUNDID
 * @brief Fill in a sound library's name and identifier from one token.
 * @param soundname the library's short name
 * @note The same device as the display interface's equivalent, and for the same reason: written separately, the name and the identifier could disagree.
 */
/**
 * @def SOUNDID
 * @brief 사운드 라이브러리의 이름과 식별자를 하나의 토큰에서 채운다.
 * @param soundname 그 라이브러리의 짧은 이름
 * @note 표시부 인터페이스의 대응물과 같은 장치이며 이유도 같다. 따로 적으면 이름과 식별자가 어긋날 수 있다.
 */
#define SOUNDID(soundname) \
    #soundname, ((enum soundlib_ids) soundlib_##soundname)

/**
 * @name Kinds of occasion
 * @brief What a sound library may declare itself able to handle.
 * @note These are what the game reports, not sounds. Each corresponds to one routine in the table above, so the capability and the routine are two halves of one statement -- a
 *       library that supplies a routine without declaring the capability will never be called.
 * @{
 */
/**
 * @name 계기의 종류
 * @brief 사운드 라이브러리가 자신이 다룰 수 있다고 선언할 수 있는 것.
 * @note 이들은 소리가 아니라 게임이 알리는 것이다. 각각이 위 표의 한 루틴에 대응하므로, 그 능력과 그 루틴은 하나의 선언의 두 절반이다. 능력을 선언하지 않고 루틴을 제공하는 라이브러리는 결코 호출되지 않는다.
 * @{
 */
/*
 * Types of triggers
 */
#define SOUND_TRIGGER_USERSOUNDS   0x0001L
#define SOUND_TRIGGER_HEROMUSIC    0x0002L
#define SOUND_TRIGGER_ACHIEVEMENTS 0x0004L
#define SOUND_TRIGGER_SOUNDEFFECTS 0x0008L
#define SOUND_TRIGGER_AMBIENCE     0x0010L
#define SOUND_TRIGGER_VERBAL       0x0020L
                            /* 26 free bits */
/** @} */

extern struct sound_procs soundprocs;

/**
 * @brief The instruments the game's tunes are written for.
 *
 * A subset of the standard instrument set, and the values are the standard's -- so they are not consecutive, and the gaps are instruments the game has no use for. That is why this
 * is a list of named values rather than a numbered enumeration.
 *
 * @warning Not a range. Anything iterating over these will read numbers that are valid instruments but not ones the game uses.
 * @note The full standard set is kept below, disabled, so a new tune can find a name for an instrument this subset lacks.
 */
/**
 * @brief 게임의 곡조가 쓰인 악기들.
 *
 * 표준 악기 묶음의 부분집합이며, 그 값은 표준의 것이다. 그래서 연속하지 않고, 그 틈들은 게임이 쓸 일이 없는 악기다. 이것이 번호가 붙은 열거가 아니라 이름 붙은 값의 목록인 이유가 그것이다.
 *
 * @warning 범위가 아니다. 이것을 순회하는 것은 유효한 악기이지만 게임이 쓰지 않는 번호를 읽게 된다.
 * @note 전체 표준 묶음이 아래에 비활성화된 채로 보관되어 있어, 새 곡조가 이 부분집합에 없는 악기의 이름을 찾을 수 있다.
 */
/* subset for NetHack */
enum instruments {
    ins_cello = 43, ins_orchestral_harp = 47, ins_choir_aahs = 53,
    ins_trumpet = 57, ins_trombone = 58, ins_french_horn = 61,
    ins_baritone_sax = 68, ins_english_horn = 70, ins_piccolo = 73,
    ins_flute = 74, ins_pan_flute = 76, ins_blown_bottle = 77,
    ins_whistle = 79, ins_tinkle_bell = 113, ins_woodblock = 116,
    ins_taiko_drum = 117, ins_melodic_tom = 118, ins_seashore = 123,
    ins_no_instrument
};

#if 0
enum instruments_broad {
    ins_acoustic_grand_piano = 1, ins_bright_acoustic_piano = 2,
    ins_electric_grand_piano = 3, ins_honkytonk_piano = 4,
    ins_electric_piano_1 = 5, ins_electric_piano_2 = 6,
    ins_harpsichord = 7, ins_clavinet = 8, ins_celesta = 9,
    ins_glockenspiel = 10, ins_music_box = 11, ins_vibraphone = 12,
    ins_marimba = 13, ins_xylophone = 14, ins_tubular_bells = 15,
    ins_dulcimer = 16, ins_drawbar_organ = 17,
    ins_percussive_organ = 18, ins_rock_organ = 19, ins_church_organ = 20,
    ins_reed_organ = 21, ins_french_accordion = 22, ins_harmonica = 23,
    ins_tango_accordion = 24, ins_acoustic_guitar__nylon = 25,
    ins_acoustic_guitar_steel = 26, ins_electric_guitar_jazz = 27,
    ins_electric_guitar_clean = 28, ins_electric_guitar_muted = 29,
    ins_overdriven_guitar = 30, ins_distortion_guitar = 31,
    ins_guitar_harmonics = 32, ins_acoustic_bass = 33,
    ins_electric_bass__fingered = 34, ins_electric_bass_picked = 35,
    ins_fretless_bass = 36, ins_slap_bass_1 = 37, ins_slap_bass_2 = 38,
    ins_synth_bass_1 = 39, ins_synth_bass_2 = 40, ins_violin = 41,
    ins_viola = 42, ins_cello = 43, ins_contrabass = 44,
    ins_tremolo_strings = 45, ins_pizzicato_strings = 46,
    ins_orchestral_harp = 47, ins_timpani = 48,
    ins_string_ensemble_1 = 49, ins_string_ensemble_2 = 50,
    ins_synthstrings_1 = 51, ins_synthstrings_2 = 52, ins_choir_aahs = 53,
    ins_voice_oohs = 54, ins_synth_voice = 55, ins_orchestra_hit = 56,
    ins_trumpet = 57, ins_trombone = 58, ins_tuba = 59, ins_muted_trumpet = 60,
    ins_french_horn = 61, ins_brass_section = 62, ins_synthbrass_1 = 63,
    ins_synthbrass_2 = 64, ins_soprano_sax = 65, ins_alto_sax = 66,
    ins_tenor_sax = 67, ins_baritone_sax = 68, ins_oboe = 69,
    ins_english_horn = 70, ins_bassoon = 71, ins_clarinet = 72,
    ins_piccolo = 73, ins_flute = 74, ins_recorder = 75,
    ins_pan_flute = 76, ins_blown_bottle = 77, ins_shakuhachi = 78,
    ins_whistle = 79, ins_ocarina = 80, ins_sitar = 105, ins_banjo = 106,
    ins_shamisen = 107, ins_koto = 108, ins_kalimba = 109, ins_bag_pipe = 110,
    ins_fiddle = 111, ins_shanai = 112, ins_tinkle_bell = 113, ins_agogo = 114,
    ins_steel_drums = 115, ins_woodblock = 116, ins_taiko_drum = 117,
    ins_melodic_tom = 118, ins_synth_drum = 119, ins_reverse_cymbal = 120,
    ins_guitar_fret_noise = 121, ins_breath_noise = 122, ins_seashore = 123,
    ins_bird_tweet = 124, ins_telephone_ring = 125, ins_helicopter = 126,
    ins_applause = 127, ins_gunshot = 128,
    ins_no_instrument
};
#endif

#define SEFFECTS_ENUM
enum sound_effect_entries {
    se_zero_invalid = 0,
#include "seffects.h"
    number_of_se_entries
};
#undef SEFFECTS_ENUM

enum ambience_actions {
    ambience_nothing, ambience_begin, ambience_end, ambience_update
};

enum ambiences {
    amb_noambience,
};

enum voice_moreinfo {
    voice_nothing_special,
    voice_audioassistant   = 0x0001,  /* accessibility */
    voice_talking_artifact = 0x0002,
    voice_deity            = 0x0004,
    voice_oracle           = 0x0008,
    voice_throne           = 0x0010,
    voice_death            = 0x0020
};

enum achievements_arg2 {
    sa2_zero_invalid, sa2_splashscreen, sa2_newgame_nosplash, sa2_restoregame,
    sa2_xplevelup, sa2_xpleveldown, number_of_sa2_entries
};

/*
Arguments for sound_achievement(schar arg1, schar arg2, int32_t aflags)

Arguments for actual achievements, those in you.h,
        arg1 = the achievement value.
        arg2 = 0 (irrelevant).
      aflags = 0 for first time, 1 for repeat.

These next ones make use of arg2, and aflags may be
filled with additional int values dependent on arg2.
arg1 must always be 0 for these.

SoundAchievement(0, sa2_splashscreen, 0);
SoundAchievement(0, sa2_newgame_nosplash, 0);
SoundAchievement(0, sa2_restoregame, 0);
SoundAchievement(0, sa2_levelup, level);
SoundAchievement(0, sa2_xpleveldown, level);
*/

#if defined(SND_LIB_QTSOUND) || defined(SND_LIB_PORTAUDIO) \
        || defined(SND_LIB_OPENAL) || defined(SND_LIB_SDL_MIXER) \
        || defined(SND_LIB_MINIAUDIO) || defined(SND_LIB_FMOD) \
        || defined(SND_LIB_SOUND_ESCCODES) || defined(SND_LIB_VISSOUND) \
        || defined(SND_LIB_WINDSOUND) || defined(SND_LIB_MACSOUND)

/* shortcut for conditional code in other files */
#define SND_LIB_INTEGRATED

#define Play_usersound(filename, vol, idx) \
    do {                                                                      \
        if (iflags.sounds && !Deaf && soundprocs.sound_play_usersound         \
            && ((soundprocs.sound_triggers & SOUND_TRIGGER_USERSOUNDS) != 0)) \
            (*soundprocs.sound_play_usersound)((filename), (vol), (idx));     \
    } while(0)

#define Soundeffect(seid, vol) \
    do {                                                                      \
        if (iflags.sounds && !Deaf && soundprocs.sound_soundeffect            \
          && ((soundprocs.sound_triggers & SOUND_TRIGGER_SOUNDEFFECTS) != 0)) \
            (*soundprocs.sound_soundeffect)(emptystr, (seid), (vol));         \
    } while(0)

/* Player's perspective, not the hero's; no Deaf suppression */
#define SoundeffectEvenIfDeaf(seid, vol) \
    do {                                                                      \
        if (iflags.sounds && !soundprocs.sound_soundeffect                    \
          && ((soundprocs.sound_triggers & SOUND_TRIGGER_SOUNDEFFECTS) != 0)) \
            (*soundprocs.sound_soundeffect)(emptystr, (seid), (vol));         \
    } while(0)

#define Hero_playnotes(instrument, str, vol) \
    do {                                                                     \
        if (iflags.sounds && !Deaf && soundprocs.sound_hero_playnotes        \
            && ((soundprocs.sound_triggers & SOUND_TRIGGER_HEROMUSIC) != 0)) \
            (*soundprocs.sound_hero_playnotes)((instrument), (str), (vol));  \
    } while(0)

/* Player's perspective, not the hero's; no Deaf suppression */
#define SoundAchievement(arg1, arg2, avals) \
    do {                                                                      \
        if (iflags.sounds && soundprocs.sound_achievement                     \
          && ((soundprocs.sound_triggers & SOUND_TRIGGER_ACHIEVEMENTS) != 0)) \
            (*soundprocs.sound_achievement)((arg1), (arg2), (avals));         \
    } while(0)

/* sound_speak is in sound.c */
#define SoundSpeak(text) \
    do {                                                                     \
        if ((gp.pline_flags & (PLINE_VERBALIZE | PLINE_SPEECH)) != 0         \
            && soundprocs.sound_verbal && iflags.voices                      \
            && ((soundprocs.sound_triggers & SOUND_TRIGGER_VERBAL) != 0))    \
            sound_speak(text);                                               \
    } while(0)

/* set_voice is in sound.c */
#define SetVoice(mon, tone, vol, moreinfo) \
    do {                                                                     \
        set_voice(mon, tone, vol, moreinfo);                                 \
    } while(0)

/*  void (*sound_achievement)(schar, schar, int32_t); */

#ifdef SOUNDLIBONLY
#undef SOUNDLIBONLY
#endif
#define SOUNDLIBONLY
#ifdef SND_SPEECH
#define VOICEONLY
#else
#define VOICEONLY UNUSED
#endif

#else  /*  NO SOUNDLIB IS INTEGRATED AFTER THIS */

#ifdef SND_LIB_INTEGRATED
#undef SND_LIB_INTEGRATED
#endif
#define Play_usersound(filename, vol, idx)
#define Soundeffect(seid, vol)
#define Hero_playnotes(instrument, str, vol)
#define SoundAchievement(arg1, arg2, avals)
#define SoundSpeak(text)
#define SetVoice(mon, tone, vol, moreinfo)
#ifdef SOUNDLIBONLY
#undef SOUNDLIBONLY
#endif
#define SOUNDLIBONLY UNUSED
#ifdef SND_SPEECH
#undef SND_SPEECH
#endif
#ifdef VOICEONLY
#undef VOICEONLY
#endif
#define VOICEONLY UNUSED

#endif  /* No SOUNDLIB */

/**
 * @brief Where a sound comes from.
 * @note Two possibilities and they need different handling: a sound compiled into the library and one read from a file. The second can fail and the first cannot.
 */
/**
 * @brief 소리가 어디서 오는지.
 * @note 두 가능성이 있고 서로 다른 처리를 필요로 한다. 라이브러리에 컴파일되어 들어간 소리와 파일에서 읽는 소리. 두 번째는 실패할 수 있고 첫 번째는 그럴 수 없다.
 */
enum findsound_approaches {
    findsound_embedded,
    findsound_soundfile
};

/**
 * @brief How much of a sound file's name the caller has already worked out.
 *
 * Assembling the name means joining a directory, a base name and a suffix, and a caller may already have some of those. So it says which parts it is supplying and which it wants
 * added, rather than every caller assembling the whole name and getting the separators wrong differently.
 *
 * @note @c sff_base_only asks for no assembly at all, which is what a library that manages its own file layout wants.
 */
/**
 * @brief 사운드 파일 이름의 얼마만큼을 호출자가 이미 알아냈는지.
 *
 * 이름을 조립하는 것은 디렉토리, 기본 이름, 접미사를 잇는 일이며, 호출자가 그 중 일부를 이미 가지고 있을 수 있다. 그래서 호출자가 자신이 무엇을 제공하고 무엇이 더해지기를 원하는지 말한다. 호출자마다 전체 이름을 조립하며 구분자를 서로 다르게 틀리는 대신.
 *
 * @note @c sff_base_only 는 조립을 전혀 요청하지 않는다. 자기 파일 배치를 스스로 관리하는 라이브러리가 원하는 것이다.
 */
enum sound_file_flags {
    sff_default,            /* add dir prefix + '/' + sound + suffix */
    sff_base_only,          /* base sound name only, no dir, no suffix */
    sff_havedir_append_rest, /* dir provided, append base sound name+suffix */
    sff_baseknown_add_rest /* base is already known, add dir and suffix */
};

#endif /* SNDPROCS_H */
