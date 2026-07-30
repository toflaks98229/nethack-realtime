/* NetHack 5.0  mdlib.c  $NHDT-Date: 1781973053 2026/06/20 16:30:53 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.74 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* Copyright (c) M. Stephenson, 1990, 1991.                       */
/* Copyright (c) Dean Luick, 1990.                                */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This can be linked into a binary to provide the functionality
 * via the contained functions, or it can be #included directly
 * into util/makedefs.c to provide it there.
 */

/**
 * @file mdlib.c
 * @brief Build-time facts about this NetHack, shared with the tools that
 *        generate its data.
 *
 * The version, the build settings, and the checks a save file is measured
 * against have to be known in two places: inside the game, and inside the
 * makedefs tool that writes the data files the game will read. Keeping them in
 * one file is what stops the two from disagreeing.
 *
 * That is also why this can either be linked in or included directly into
 * makedefs.c -- the tool cannot link against the game.
 *
 * @warning Compiled in both contexts, so it must not depend on the full game
 *          declarations; the includes here are deliberately narrow.
 */

/**
 * @file mdlib.c
 * @brief 이 NetHack 의 빌드 시점 정보. 데이터를 생성하는 도구와 공유한다.
 *
 * 버전, 빌드 설정, 저장 파일을 견주는 검사 기준은 두 곳에서 알고 있어야 한다.
 * 게임 안에서, 그리고 게임이 읽을 데이터 파일을 쓰는 makedefs 도구 안에서다.
 * 이것들을 한 파일에 두는 것이 둘이 서로 어긋나지 않게 하는 방법이다.
 *
 * 이 파일을 링크해 쓸 수도 있고 makedefs.c 에 직접 포함해 쓸 수도 있는 이유도
 * 같다. 그 도구는 게임과 링크할 수 없기 때문이다.
 *
 * @warning 두 맥락 모두에서 컴파일되므로 게임 전체 선언에 의존해서는 안 된다.
 *          여기의 include 목록이 의도적으로 좁은 이유다.
 */

#ifndef MAKEDEFS_C
#define MDLIB_C
#include "config.h"
#include "permonst.h"
#include "objclass.h"
#include "wintype.h"
#include "sym.h"
#include "artilist.h"
#include "dungeon.h"
#include "sndprocs.h"
#include "obj.h"
#include "monst.h"
#include "you.h"
#include "context.h"
#include "flag.h"
#include "dlb.h"
#include "hacklib.h"

/* version information */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif
#define Fprintf (void) fprintf
#define Fclose (void) fclose
#define Unlink (void) unlink
#if !defined(AMIGA) || defined(AZTEC_C)
#define rewind(fp) fseek((fp), 0L, SEEK_SET) /* guarantee a return value */
#endif  /* AMIGA || AZTEC_C */
#else
#ifndef GLOBAL_H
#include "global.h"
#endif
#endif  /* !MAKEDEFS_C */

/* shorten up some lines */
#define FOR_RUNTIME

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)
#include <stdarg.h>
/* REPRODUCIBLE_BUILD will change this to TRUE */
static boolean date_via_env = FALSE;

extern unsigned long md_ignored_features(void);
extern const char *datamodel(int);
char *version_id_string(char *, size_t, const char *) NONNULL NONNULLPTRS;
char *bannerc_string(char *, size_t, const char *) NONNULL NONNULLPTRS;
int case_insensitive_comp(const char *, const char *) NONNULLPTRS;

#ifndef SFCTOOL
staticfn
#endif /* SFCTOOL */
void make_version(void);

#ifndef HAS_NO_MKSTEMP
#ifdef _MSC_VER
static int mkstemp(char *);
#endif
#endif

#endif /* MAKEDEFS_C || FOR_RUNTIME */

#if !defined(MAKEDEFS_C) && defined(WIN32)
extern int GUILaunched;
#endif

/* these are in extern.h but we don't include hack.h */
/* XXX move to new file mdlib.h? */
extern void populate_nomakedefs(struct version_info *) NONNULLARG1; /*date.c*/
extern void free_nomakedefs(void); /* date.c */
void runtime_info_init(void);
const char *do_runtime_info(int *) NO_NNARGS;
void release_runtime_info(void);
char *mdlib_version_string(char *, const char *) NONNULL NONNULLPTRS;

staticfn void build_options(void);
staticfn int count_and_validate_winopts(void);
staticfn void opt_out_words(char *, int *) NONNULLPTRS;
staticfn void build_savebones_compat_string(void);

/**
 * @name Storage for the composed build-options report
 * @{
 */

/** How many lines have been stored, and whether the report has been composed at all. The second doubles as the guard that keeps composition from happening twice, since the report cannot change while the program runs. */
/** 몇 줄이 저장되었는지, 그리고 보고가 아예 조립되었는지. 두 번째 것은 조립이 두 번 일어나지 않게 하는 방벽을 겸한다. 프로그램이 실행되는 동안 보고는 바뀔 수 없기 때문이다. */
static int idxopttext, done_runtime_opt_init_once = 0;

/**
 * @def MAXOPT
 * @brief How many report lines can be held.
 * @note Generously above what is used, as the accompanying comment records -- around forty at present. The margin is there because options are added over time and nothing reports when the limit is approached.
 */
/**
 * @def MAXOPT
 * @brief 몇 줄의 보고를 담을 수 있는지.
 * @note 딸린 주석이 기록하는 대로, 쓰이는 것보다 넉넉히 위다. 현재 사십 줄쯤이다. 그 여유가 있는 것은 선택지가 시간이 지나며 더해지고, 한계에 가까워질 때 알려주는 것이 없기 때문이다.
 */
#define MAXOPT 60 /* 5.0: currently 40 lines get inserted into opttext[] */

/** The report itself, one line per entry, each separately allocated. */
/** 보고 자체. 항목마다 한 줄이며, 각각 따로 할당된다. */
static char *opttext[MAXOPT] = { 0 };

/**
 * @def STOREOPTTEXT
 * @brief Append one finished line to the report, keeping a copy of it.
 * @warning Does nothing once the limit is reached, and says nothing about having done nothing. A report that has quietly lost its last lines still looks like a complete report, which is the failure mode worth knowing about here.
 */
/**
 * @def STOREOPTTEXT
 * @brief 끝난 한 줄을 보고에 덧붙이면서 그것의 사본을 지닌다.
 * @warning 한계에 이르면 아무것도 하지 않으며, 아무것도 하지 않았다는 것에 대해 아무것도 말하지 않는다. 마지막 줄들을 조용히 잃은 보고도 여전히 완전한 보고처럼 보인다. 그것이 여기서 알아 둘 만한 고장 방식이다.
 */
#define STOREOPTTEXT(line) \
    ((void) ((idxopttext < MAXOPT)                      \
             ? (opttext[idxopttext++] = dupstr(line))   \
             : 0))

/** The line currently being assembled. Shared rather than local because assembly is spread across build_options() and opt_out_words(). */
/** 지금 조립되고 있는 줄. 조립이 build_options()와 opt_out_words()에 걸쳐 퍼져 있으므로 지역 변수가 아니라 공유된다. */
static char optbuf[COLBUFSZ];

/** This build's three identifying numbers, as computed by make_version(). */
/** make_version()이 계산한, 이 빌드의 세 식별 숫자. */
static struct version_info version;

/** The leading space every report line carries, so the report reads as indented under its heading. */
/** 모든 보고 줄이 지니는 앞쪽 공백. 보고가 제 제목 아래 들여쓰인 것으로 읽히게 한다. */
static const char opt_indent[] = "    ";

/** @} */

struct win_information {
    const char *id, /* windowtype value */
        *name;      /* description, often same as id */
    boolean valid;
};

static struct win_information window_opts[] = {
#ifdef TTY_GRAPHICS
    { "tty",
      /* testing TILES_IN_GLYPHMAP here would bring confusion because it could
         apply to another interface such as X11, so check MSDOS explicitly
         instead; even checking TTY_TILES_ESCCODES would probably be
         confusing to most users (and it will already be listed separately
         in the compiled options section so users aware of it can find it) */
#ifdef MSDOS
      "traditional text with optional 'tiles' graphics",
#else
      /* assume that one or more of IBMgraphics, DECgraphics
         can be enabled; we can't tell from here whether that is accurate */
      "traditional text with optional line-drawing",
#endif
      TRUE
    },
#endif /*TTY_GRAPHICS */
#ifdef CURSES_GRAPHICS
    { "curses", "terminal-based graphics", TRUE },
#endif
#ifdef X11_GRAPHICS
    { "X11", "X11", TRUE },
#endif
#ifdef QT_GRAPHICS /* too vague; there are multiple incompatible versions */
    { "Qt", "Qt", TRUE },
#endif
#ifdef MSWIN_GRAPHICS /* win32 */
    { "mswin", "Windows GUI", TRUE },
#endif
#ifdef SHIM_GRAPHICS
    { "shim", "NetHack Library Windowing Shim", TRUE },
#endif
#ifdef AMIGA_INTUITION
    { "amii", "Amiga Intuition (text)", TRUE },
    { "amiv", "Amiga Intuition (tiles)", TRUE },
#endif

#if 0  /* remainder have been retired */
#ifdef GNOME_GRAPHICS /* unmaintained/defunct */
    { "Gnome", "Gnome", TRUE },
#endif
#ifdef MAC68K /* defunct OS 9 interface */
    { "mac", "Mac", TRUE },
#endif
#ifdef GEM_GRAPHICS /* defunct Atari interface */
    { "Gem", "Gem", TRUE },
#endif
#ifdef BEOS_GRAPHICS /* unmaintained/defunct */
    { "BeOS", "BeOS InterfaceKit", TRUE },
#endif
#endif  /* 0 => retired */
    { 0, 0, FALSE }
};

#if !defined(MAKEDEFS_C)
staticfn int count_and_validate_soundlibopts(void);

struct soundlib_information {
    enum soundlib_ids id;
    const char *const text_id;
    const char *const Url;
    boolean valid;
};

/*
 * soundlibs
 *
 * None of these are endorsements or recommendations of one library
 * or another, in any way. They are just preprocessor conditionals
 * in the event that glue code for such a library is ever added into
 * NetHack.
 */
static struct soundlib_information soundlib_opts[] = {
    { soundlib_nosound, "soundlib_nosound", "", FALSE },
#ifdef SND_LIB_PORTAUDIO
    { soundlib_portaudio, "soundlib_portaudio",
        "http://www.portaudio.com/", FALSE },
#endif
#ifdef SND_LIB_OPENAL
    { soundlib_openal, "soundlib_openal",
        "https://www.openal.org/", FALSE },
#endif
#ifdef SND_LIB_SDL_MIXER
    { soundlib_sdl_mixer, "soundlib_sdl_mixer",
        "https://github.com/libsdl-org/SDL_mixer/", FALSE },
#endif
#ifdef SND_LIB_MINIAUDIO
    { soundlib_miniaudio, "soundlib_miniaudio",
        "https://miniaud.io/", FALSE },
#endif
#ifdef SND_LIB_FMOD
    /* proprietary, though a Free Indie License exists.
     * https://www.fmod.com/licensing#indie-note
     */
    { soundlib_fmod, "soundlib_fmod",
        "https://www.fmod.com/", FALSE },
#endif
#ifdef SND_LIB_SOUND_ESCCODES
    { soundlib_sound_esccodes, "sound_esccodes", "", FALSE },
#endif
#ifdef SND_LIB_VISSOUND
    { soundlib_vissound, "soundlib_vissound", "", FALSE },
#endif
#ifdef SND_LIB_WINDSOUND
    /* Uses Windows WIN32 API */
    { soundlib_windsound, "soundlib_windsound",
  "https://learn.microsoft.com/en-us/windows/win32/multimedia/waveform-audio",
        FALSE },
#endif
#ifdef SND_LIB_MACSOUND
    /* Uses AppKit NSSound */
    { soundlib_macsound, "soundlib_macsound",
        "https://developer.apple.com/documentation/appkit/nssound",
        FALSE },
#endif
#ifdef SND_LIB_QTSOUND
    { soundlib_qtsound, "soundlib_qtsound",
        "https://doc.qt.io/qt-5/qsoundeffect.html", FALSE },
#endif
    { 0, 0, 0, FALSE },
};
#endif  /* !MAKEDEFS_C */

/**
 * @brief Which build options do not affect whether a file can be read.
 *
 * A file records the options its build was made with, and a mismatch normally refuses it. But a few options change nothing about how anything is stored -- whether the score appears on the status line does not alter a save file -- and
 * refusing a file over one of those would be an obstruction with no purpose.
 *
 * @return the options to disregard when comparing
 * @note One of the entries is not an option at all but the mark saying a file came from the format-conversion tool. It is ignored here because that mark is examined and removed separately, so leaving it in the comparison would make
 *       every converted file mismatch.
 * @warning An option added here becomes invisible to the compatibility check. That is right only when nothing about the option changes what is stored, and nothing verifies that -- the consequence of getting it wrong is a file
 *          accepted that should not have been.
 */
/**
 * @brief 어떤 빌드 선택지가 파일을 읽을 수 있는지에 영향을 주지 않는지.
 *
 * 파일은 자기 빌드가 어떤 선택지로 만들어졌는지 기록하고, 불일치는 보통 그것을 거부한다. 그런데 몇몇 선택지는 무엇이 어떻게 저장되는지에 대해 아무것도 바꾸지 않는다. 점수가 상태줄에 나타나는지는 저장 파일을 바꾸지 않는다. 그런 것 때문에 파일을 거부하는 것은 목적 없는 방해가 된다.
 *
 * @return 비교할 때 무시할 선택지
 * @note 항목 중 하나는 선택지가 전혀 아니라 파일이 형식 변환 도구에서 왔다고 말하는 표시다. 그 표시가 따로 살펴지고 제거되므로 여기서 무시된다. 비교에 남겨 두면 변환된 모든 파일이 불일치하게 된다.
 * @warning 여기에 더해진 선택지는 호환성 검사에 보이지 않게 된다. 그것은 그 선택지에 관한 무엇도 저장되는 것을 바꾸지 않을 때만 옳으며, 그것을 검증하는 것은 없다. 틀렸을 때의 결과는 받아들여져서는 안 되는 파일이 받아들여지는 것이다.
 */
unsigned long
md_ignored_features(void)
{
    return (0UL
            | (1UL << 19) /* SCORE_ON_BOTL */
            | SFCTOOL_BIT /* stored by SFCTOOL, not NetHack itself */
            );
}

#ifndef SFCTOOL
staticfn
#endif
/**
 * @brief Compute the three numbers that identify this build.
 *
 * The single place all three are produced, which is what stops the game and the data-generating tool from computing them differently. Each answers a different question, and understanding which is which is the point of reading this.
 *
 * The version is the four parts packed into one value. The feature set is a bit per build option that affects storage. The entity count packs the number of artifacts, objects and monsters together -- so a build with a monster added
 * has a different count and its files are refused, which is right because monster numbers are stored.
 *
 * @warning The bit positions in the feature set are arbitrary but fixed. The accompanying comment states the rule: changing or reassigning one means incrementing the edit level at the same time, because a file written before the
 *          change records that bit meaning something else. The category groupings are only for convenience and carry no meaning.
 * @note The artifact count is arrived at by walking the names until they run out rather than from a constant, because the count is not available as one here.
 * @note Colour support contributes a bit unconditionally. It is not an option any more, and the bit is kept set so that files remain comparable with those written when it was.
 */
/**
 * @brief 이 빌드를 식별하는 세 숫자를 계산한다.
 *
 * 그 셋 모두가 만들어지는 단 하나의 장소이며, 그것이 게임과 데이터 생성 도구가 그것들을 다르게 계산하는 것을 막는 것이다. 각각이 다른 질문에 답하며, 어느 것이 어느 것인지 이해하는 것이 이것을 읽는 요점이다.
 *
 * 판본은 네 부분을 하나의 값에 압축한 것이다. 기능 묶음은 저장에 영향을 주는 빌드 선택지마다 한 비트다. 개체 수는 아티팩트, 물건, 몬스터의 수를 함께 압축한다. 그래서 몬스터가 더해진 빌드는 다른 개수를 가지고 그 파일이 거부된다. 몬스터 번호가 저장되므로 그것이 옳다.
 *
 * @warning 기능 묶음의 비트 위치는 임의적이지만 고정되어 있다. 딸린 주석이 그 규칙을 밝힌다. 하나를 바꾸거나 다시 배정하는 것은 동시에 편집 단계를 올린다는 뜻이다. 그 변경 전에 쓰인 파일은 그 비트가 다른 것을 뜻하는 것으로 기록하기 때문이다. 범주별 묶음은 편의를 위한 것일 뿐 의미가
 *          없다.
 * @note 아티팩트 수는 상수가 아니라 이름이 다할 때까지 훑어서 얻는다. 여기서 그 개수를 상수로 쓸 수 없기 때문이다.
 * @note 색 지원이 조건 없이 한 비트를 기여한다. 그것은 더는 선택지가 아니며, 그것이 선택지였을 때 쓰인 파일과 비교 가능하도록 그 비트가 켜진 채로 유지된다.
 */
void
make_version(void)
{
    int i;

    /*
     * integer version number
     */
    version.incarnation = ((unsigned long) VERSION_MAJOR << 24)
                          | ((unsigned long) VERSION_MINOR << 16)
                          | ((unsigned long) PATCHLEVEL << 8)
                          | ((unsigned long) EDITLEVEL);
    /*
     * encoded feature list
     * Note:  if any of these magic numbers are changed or reassigned,
     * EDITLEVEL in patchlevel.h should be incremented at the same time.
     * The actual values have no special meaning, and the category
     * groupings are just for convenience.
     */
    version.feature_set = (unsigned long) (0L
/* levels and/or topology (0..4) */
/* monsters (5..9) */
#ifdef MAIL_STRUCTURES
                                           | (1L << 6)
#endif
/* objects (10..14) */
/* flag bits and/or other global variables (15..26) */
 /* color support always*/                 | (1L << 17)
#ifdef INSURANCE
                                           | (1L << 18)
#endif
#ifdef SCORE_ON_BOTL
                                           | (1L << 19)
#endif
                                               );
    /*
     * Value used for object & monster sanity check.
     *    (NROFARTIFACTS<<24) | (NUM_OBJECTS<<12) | (NUMMONS<<0)
     */
    for (i = 1; artifact_names[i]; i++)
        continue;
    version.entity_count = (unsigned long) (i - 1);
    i = NUM_OBJECTS;
    version.entity_count = (version.entity_count << 12) | (unsigned long) i;
    i = NUMMONS;
    version.entity_count = (version.entity_count << 12) | (unsigned long) i;
/* free bits in here */
    return;
}

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)

/**
 * @brief The version's three public parts, joined by whatever separator the caller wants.
 * @param outbuf where to write it
 * @param delim what to put between the parts
 * @return the buffer
 * @note The separator is an argument because the same three numbers appear as "5.0.0" to a player and as "5_0_0" in a file name, and composing them twice would let the two drift.
 * @note The edit level is appended only in a build that is not a release. It is a private counter, and showing it to a player of a released version would be showing them a number they cannot use.
 */
/**
 * @brief 판본의 공개된 세 부분. 호출자가 원하는 구분자로 이어서.
 * @param outbuf 그것을 쓸 곳
 * @param delim 부분들 사이에 무엇을 둘지
 * @return 그 버퍼
 * @note 구분자가 인자인 것은, 같은 세 숫자가 플레이어에게는 "5.0.0"으로, 파일 이름에서는 "5_0_0"으로 나타나기 때문이다. 그것들을 두 번 조립하면 그 둘이 어긋날 수 있다.
 * @note 편집 단계는 릴리스가 아닌 빌드에서만 덧붙는다. 그것은 내부 계수기이며, 릴리스된 판본의 플레이어에게 그것을 보이는 것은 그가 쓸 수 없는 숫자를 보이는 일이다.
 */
char *
mdlib_version_string(char *outbuf, const char *delim)
{
    Sprintf(outbuf, "%d%s%d%s%d", VERSION_MAJOR, delim, VERSION_MINOR, delim,
            PATCHLEVEL);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    Sprintf(eos(outbuf), "-%d", EDITLEVEL);
#endif
    return outbuf;
}

#define Snprintf(str, size, ...) \
    nh_snprintf(__func__, __LINE__, str, size, __VA_ARGS__)
extern void nh_snprintf(const char *func, int line, char *str, size_t size,
                        const char *fmt, ...);

/**
 * @brief The one-line identification a player sees, and that a bug report should carry.
 * @param outbuf where to write it
 * @param bufsz its size
 * @param build_date when this was built, as text
 * @return the buffer
 * @note It names the port as well as the version, because the same version behaves differently between ports and a report that omits the port is a report that cannot be reproduced.
 * @note It says "revision" rather than "build" when the date came from the environment rather than from compilation. The distinction matters: a date fixed by the environment is a reproducible-build date and does not tell you when
 *       the binary was made.
 * @note A development build says so. That is deliberate -- a player reporting from a work-in-progress build should not have to know they were.
 */
/**
 * @brief 플레이어가 보는, 그리고 버그 보고가 담아야 하는 한 줄 식별.
 * @param outbuf 그것을 쓸 곳
 * @param bufsz 그 크기
 * @param build_date 이것이 언제 빌드되었는지, 글로
 * @return 그 버퍼
 * @note 판본뿐 아니라 이식판도 이름 짓는다. 같은 판본이 이식판 사이에서 다르게 행동하며, 이식판을 빠뜨린 보고는 재현할 수 없는 보고이기 때문이다.
 * @note 날짜가 컴파일이 아니라 환경에서 온 경우 "build"가 아니라 "revision"이라고 말한다. 그 구별은 중요하다. 환경이 고정한 날짜는 재현 가능 빌드의 날짜이며 이 실행 파일이 언제 만들어졌는지 알려주지 않는다.
 * @note 개발 빌드는 그렇다고 말한다. 그것은 의도된 것이다. 작업 중인 빌드에서 보고하는 플레이어가 자기가 그랬다는 것을 알아야 할 필요는 없어야 한다.
 */
char *
version_id_string(char *outbuf, size_t bufsz, const char *build_date)
{
    char subbuf[64], versbuf[64];
    char statusbuf[64];

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
    Strcpy(statusbuf, " Beta");
#else
#if (NH_DEVEL_STATUS == NH_STATUS_WIP)
    Strcpy(statusbuf, " Work-in-progress");
#else
    Strcpy(statusbuf, " post-release");
#endif
#endif
#else
    statusbuf[0] = '\0';
#endif
    subbuf[0] = '\0';
#ifdef PORT_SUB_ID
    subbuf[0] = ' ';
    Strcpy(&subbuf[1], PORT_SUB_ID);
#endif

    Snprintf(outbuf, bufsz, "%s NetHack%s Version %s%s - last %s %s.",
             PORT_ID, subbuf, mdlib_version_string(versbuf, "."), statusbuf,
             date_via_env ? "revision" : "build", build_date);
    return outbuf;
}

/* still within #if MAKDEFS_C || FOR_RUNTIME */

/**
 * @brief The same identification, formatted for the startup banner.
 * @param outbuf where to write it
 * @param bufsz its size
 * @param build_date when this was built, as text
 * @return the buffer
 * @note It exists separately from version_id_string() because the banner is centred under a title and wants leading space and a different word order, not because it says anything different.
 * @warning The development-status text is written into the same buffer that would otherwise hold the port sub-identification, overwriting it. In a development build with a sub-identification, the sub-identification does not appear.
 *          version_id_string() keeps both in separate buffers and does not have this problem, so the two lines can disagree about what this build is.
 */
/**
 * @brief 같은 식별을, 시작 배너를 위한 형식으로.
 * @param outbuf 그것을 쓸 곳
 * @param bufsz 그 크기
 * @param build_date 이것이 언제 빌드되었는지, 글로
 * @return 그 버퍼
 * @note version_id_string()과 따로 존재하는 것은, 배너가 제목 아래에 가운데 맞춰지며 앞쪽 공백과 다른 어순을 원하기 때문이지, 다른 무엇을 말하기 때문이 아니다.
 * @warning 개발 상태 글이, 그러지 않았다면 이식판 하위 식별을 담았을 같은 버퍼에 쓰여서 그것을 덮어쓴다. 하위 식별이 있는 개발 빌드에서 하위 식별은 나타나지 않는다. version_id_string()은 그 둘을 따로 된 버퍼에 두어 이 문제가 없으므로, 두 줄이 이 빌드가 무엇인지에 대해
 *          서로 어긋날 수 있다.
 */
char *
bannerc_string(char *outbuf, size_t bufsz, const char *build_date)
{
    char subbuf[64], versbuf[64];

    subbuf[0] = '\0';
#ifdef PORT_SUB_ID
    subbuf[0] = ' ';
    Strcpy(&subbuf[1], PORT_SUB_ID);
#endif

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
    Strcpy(subbuf, " Beta");
#elif (NH_DEVEL_STATUS == NH_STATUS_WIP)
    Strcpy(subbuf, " Work-in-progress");
#elif (NH_DEVEL_STATUS == NH_STATUS_POSTRELEASE)
    Strcpy(subbuf, " post-release");
#endif
#endif  /* !NH_STATUS_RELEASED */
    Snprintf(outbuf, bufsz, "         Version %s %s%s, %s %s.",
            mdlib_version_string(versbuf, "."), PORT_ID, subbuf,
            date_via_env ? "revised" : "built", build_date);
    return outbuf;
}

#ifndef HAS_NO_MKSTEMP
#ifdef _MSC_VER
/**
 * @brief A temporary-file creator for the compiler that does not provide one.
 * @param template a name pattern ending in the placeholder characters, modified in place to the name chosen
 * @return an open descriptor, or -1
 * @note Present only because this compiler lacks the standard function, and matching its interface is the whole point -- callers must not need to know which one they got.
 * @note The file is opened as delete-on-close, so it disappears when the last descriptor for it goes even if the program does not get to clean up. That is a deliberate improvement over the standard behaviour and the reason no caller
 *       removes the file by name.
 */
/**
 * @brief 그것을 제공하지 않는 컴파일러를 위한 임시 파일 생성기.
 * @param template 자리표 문자로 끝나는 이름 형태. 선택된 이름으로 제자리에서 수정된다
 * @return 열린 서술자, 또는 -1
 * @note 이 컴파일러가 표준 함수를 갖지 않기 때문에만 존재하며, 그 인터페이스를 맞추는 것이 요점 전부다. 호출자가 어느 것을 얻었는지 알 필요가 없어야 한다.
 * @note 파일은 닫을 때 지워지도록 열린다. 그래서 프로그램이 정리할 기회를 얻지 못해도 그것에 대한 마지막 서술자가 사라질 때 파일이 없어진다. 그것은 표준 동작보다 의도적으로 나은 것이며, 어떤 호출자도 이름으로 파일을 지우지 않는 이유다.
 */
int
mkstemp(char *template)
{
    int err;

    err = _mktemp_s(template, strlen(template) + 1);
    if( err != 0 )
        return -1;
    return _open(template,
                 _O_RDWR | _O_BINARY | _O_TEMPORARY | _O_CREAT,
                 _S_IREAD | _S_IWRITE);
}
#endif /* _MSC_VER */
#endif /* HAS_NO_MKSTEMP */
#endif /* MAKEDEFS_C || FOR_RUNTIME */

static char save_bones_compat_buf[BUFSZ];

/**
 * @brief Compose the sentence saying which versions' save and bones files this build will read.
 *
 * It is worded as a range when the oldest accepted version differs from this one, and as "this version only" when it does not. The two wordings exist because a player who reads "accepted from version 5.0.0 through 5.0.0" would
 * reasonably wonder what the range was for.
 *
 * @note Writes into a file-scope buffer that is already an entry in the build-options table. The table cannot hold a computed string any other way, which is why this must run before the options are listed rather than being called
 *       from where the text is needed.
 * @warning The oldest accepted version is packed the same way as the current one for the comparison, and the packing has to match or the two are never equal and every build claims a range.
 */
/**
 * @brief 이 빌드가 어느 판본의 저장 파일과 유골 파일을 읽을지 말하는 문장을 조립한다.
 *
 * 받아들이는 가장 오래된 판본이 이것과 다를 때는 범위로, 다르지 않을 때는 "이 판본만"으로 표현된다. 두 표현이 있는 것은, "5.0.0부터 5.0.0까지 받아들임"을 읽는 플레이어가 그 범위가 무엇을 위한 것인지 당연히 의아해할 것이기 때문이다.
 *
 * @note 이미 빌드 선택지 표의 한 항목인 파일 범위 버퍼에 쓴다. 그 표는 계산된 문자열을 다른 방법으로는 담을 수 없다. 그것이 이것이 그 글이 필요한 곳에서 불리는 대신 선택지가 나열되기 전에 실행되어야 하는 이유다.
 * @warning 받아들이는 가장 오래된 판본은 비교를 위해 현재 것과 같은 방식으로 압축되며, 그 압축이 일치해야 한다. 그러지 않으면 그 둘은 결코 같지 않고 모든 빌드가 범위를 주장한다.
 */
staticfn void
build_savebones_compat_string(void)
{
#ifdef VERSION_COMPATIBILITY
    unsigned long uver = VERSION_COMPATIBILITY,
                  cver  = (((unsigned long) VERSION_MAJOR << 24)
                         | ((unsigned long) VERSION_MINOR << 16)
                         | ((unsigned long) PATCHLEVEL    <<  8));
#endif

    Strcpy(save_bones_compat_buf,
           "save and bones files accepted from version");
#ifdef VERSION_COMPATIBILITY
    if (uver != cver)
        Sprintf(eos(save_bones_compat_buf), "s %lu.%lu.%lu through %d.%d.%d",
                ((uver >> 24) & 0x0ffUL),
                ((uver >> 16) & 0x0ffUL),
                ((uver >>  8) & 0x0ffUL),
                VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
    else
#endif
        Sprintf(eos(save_bones_compat_buf), " %d.%d.%d only",
                VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}

static const char *const build_opts[] = {
#ifdef AMIGA_WBENCH
    "Amiga WorkBench support",
#endif
#ifdef ANSI_DEFAULT
    "ANSI default terminal",
#endif
    "color",
#ifdef TTY_GRAPHICS
#ifdef TTY_TILES_ESCCODES
    "console escape codes for tile hinting",
#endif
#endif
#ifdef LIFE
    "Conway's Game of Life",
#endif
#ifdef COMPRESS
    "data file compression",
#endif
#ifdef ZLIB_COMP
    "ZLIB data file compression",
#endif
#ifdef DLB
#ifndef VERSION_IN_DLB_FILENAME
    "data librarian",
#else
    "data librarian with a version-dependent name",
#endif
#endif
#ifdef EDIT_GETLIN
    "edit getlin - some prompts remember previous response",
#endif
#ifdef DUMPLOG
    "end-of-game dumplogs",
#endif
#ifdef HOLD_LOCKFILE_OPEN
    "exclusive lock on level 0 file",
#endif
#if defined(HANGUPHANDLING) && !defined(NO_SIGNAL)
#ifdef SAFERHANGUP
    "deferred handling of hangup signal",
#else
    "immediate handling of hangup signal",
#endif
#endif
#ifdef INSURANCE
    "insurance files for recovering from crashes",
#endif
#ifdef LIVELOG
    "live logging support",
#endif
#ifdef LOGFILE
    "log file",
#endif
#ifdef XLOGFILE
    "extended log file",
#endif
#ifdef PANICLOG
    "errors and warnings log file",
#endif
#ifdef MAIL
    "mail daemon",
#endif
#ifdef MONITOR_HEAP
    "monitor heap - record memory usage for later analysis",
#endif
#if defined(GNUDOS) || defined(__DJGPP__)
    "MSDOS protected mode",
#endif
#ifdef NEWS
    "news file",
#endif
#ifdef OVERLAY
#ifdef MOVERLAY
    "MOVE overlays",
#else
#ifdef VROOMM
    "VROOMM overlays",
#else
    "overlays",
#endif
#endif
#endif
#ifdef UNIX
#if defined(DEF_PAGER) && !defined(DLB)
    "external pager used for viewing help files",
#else
    "internal pager used for viewing help files",
#endif
#endif /* UNIX */
    /* pattern matching method will be substituted by nethack at run time */
    "pattern matching via :PATMATCH:",
#ifdef USE_ISAAC64
    "pseudo random numbers generated by ISAAC64",
#ifdef DEV_RANDOM
    /* include which specific one */
    "strong PRNG seed from " DEV_RANDOM,
#else
#ifdef WIN32
    "strong PRNG seed from CNG BCryptGenRandom()",
#endif
#endif  /* DEV_RANDOM */
#else   /* ISAAC64 */
#ifdef RANDOM
    "pseudo random numbers generated by random()",
#else
    "pseudo random numbers generated by C rand()",
#endif
#endif /* ISAAC64 */
#ifdef SELECTSAVED
    "restore saved games via menu",
#endif
#ifdef SCORE_ON_BOTL
    "score on status line",
#endif
#ifdef CLIPPING
    "screen clipping",
#endif
#ifdef NO_TERMS
#ifdef MAC68K
    "screen control via mactty",
#endif
#ifdef SCREEN_BIOS
    "screen control via BIOS",
#endif
#ifdef SCREEN_DJGPPFAST
    "screen control via DJGPP fast",
#endif
#ifdef SCREEN_VGA
    "screen control via VGA graphics",
#endif
#ifdef WIN32CON
    "screen control via WIN32 console I/O",
#endif
#endif /* NO_TERMS */
#ifdef SHELL
    "shell command",
#endif
    "traditional status display",
#ifdef STATUS_HILITES
    "status via windowport with highlighting",
#else
    "status via windowport without highlighting",
#endif
#ifdef SUSPEND
    "suspend command",
#endif
#ifdef TTY_GRAPHICS
#ifdef TERMINFO
    "terminal info library",
#else
#if defined(TERMLIB) || (!defined(MICRO) && !defined(WIN32))
    "terminal capability library",
#endif
#endif
#endif /*TTY_GRAPHICS*/
#ifdef USE_XPM
    "tiles file in XPM format",
#endif
#ifdef GRAPHIC_TOMBSTONE
    "graphical RIP screen",
#endif
#ifdef TIMED_DELAY
    "timed wait for display effects",
#endif
#ifdef PREFIXES_IN_USE
    "variable playground",
#endif
#ifdef VISION_TABLES
    "vision tables",
#endif
#ifdef SYSCF
    "system configuration at run-time",
#endif
#ifdef PANICTRACE
    "show stack trace on error",
#endif
#ifdef CRASHREPORT
    "launch browser to report issues",
#endif
    save_bones_compat_buf,
    "and basic NetHack features"
};

/**
 * @brief Count the window interfaces this build has, marking each as offerable or not.
 *
 * Counting and marking are one operation because they answer the same question, and separating them would let the count disagree with the marks.
 *
 * @return how many are offerable
 * @note "Compiled in" and "usable right now" are different things. On Windows the answer depends on how the program was started: a graphical launch cannot offer the terminal interfaces, and a console launch cannot offer the graphical
 *       one, because neither has the kind of window the other needs. So the same binary reports different interfaces depending on how it was run, which is correct but surprising.
 * @note The table's last entry is a sentinel and is skipped, as the existing comment records.
 * @warning In the makedefs tool this cannot consult how the program was started, so every compiled-in interface is marked offerable. That is right for the tool -- it is describing the build, not this run -- but it means the tool's
 *          list and the running game's list legitimately differ.
 */
/**
 * @brief 이 빌드가 가진 창 인터페이스를 세면서, 각각을 제공 가능한지 아닌지 표시한다.
 *
 * 세기와 표시하기가 하나의 작업인 것은 그것들이 같은 질문에 답하기 때문이며, 그것들을 나누면 개수가 표시와 어긋날 수 있다.
 *
 * @return 몇 개가 제공 가능한지
 * @note "컴파일에 포함됨"과 "지금 쓸 수 있음"은 다른 것이다. 윈도우에서 그 답은 프로그램이 어떻게 시작되었는지에 달려 있다. 그래픽 실행은 터미널 인터페이스를 제공할 수 없고, 콘솔 실행은 그래픽 인터페이스를 제공할 수 없다. 어느 쪽도 다른 쪽이 필요한 종류의 창을 갖지 않기
 *       때문이다. 그래서 같은 실행 파일이 어떻게 실행되었는지에 따라 다른 인터페이스를 보고한다. 그것은 옳지만 놀랍다.
 * @note 표의 마지막 항목은 파수꾼이며 건너뛰어진다. 기존 주석이 기록하는 대로다.
 * @warning makedefs 도구에서는 프로그램이 어떻게 시작되었는지 물을 수 없으므로, 컴파일에 포함된 모든 인터페이스가 제공 가능으로 표시된다. 그것은 도구에게 옳다. 그것은 이 실행이 아니라 빌드를 서술하고 있다. 그러나 그것은 도구의 목록과 실행 중인 게임의 목록이 정당하게 다르다는 뜻이다.
 */
staticfn int
count_and_validate_winopts(void)
{
    int i, cnt = 0;

    /* window_opts has a fencepost entry at the end */
    for (i = 0; i < SIZE(window_opts) - 1; i++) {
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
#ifdef WIN32
        window_opts[i].valid = FALSE;
        if ((GUILaunched
             && case_insensitive_comp(window_opts[i].id, "curses") != 0
             && case_insensitive_comp(window_opts[i].id, "mswin") != 0)
            || (!GUILaunched
                && case_insensitive_comp(window_opts[i].id, "mswin") == 0))
            continue;
#endif
#endif /* !MAKEDEFS_C && FOR_RUNTIME */
        ++cnt;
        window_opts[i].valid = TRUE;
    }
    return cnt;
}

#if !defined(MAKEDEFS_C)
/**
 * @brief Count the sound libraries this build has, marking each as offerable.
 * @return how many
 * @note The counterpart of count_and_validate_winopts(), but without any of its conditions: every compiled-in sound library is offerable, because none of them depends on how the program was started. The parallel shape is kept anyway
 *       so that a future library which does have such a condition has an obvious place to state it.
 * @note Not built into the makedefs tool at all -- the tool has no need to describe sound support.
 */
/**
 * @brief 이 빌드가 가진 소리 라이브러리를 세면서, 각각을 제공 가능으로 표시한다.
 * @return 몇 개인지
 * @note count_and_validate_winopts()의 대응물이지만 그것의 조건은 하나도 없다. 컴파일에 포함된 모든 소리 라이브러리가 제공 가능하다. 그중 무엇도 프로그램이 어떻게 시작되었는지에 달려 있지 않기 때문이다. 그럼에도 나란한 모양이 유지되는 것은, 그런 조건을 실제로 갖는 미래의
 *       라이브러리가 그것을 밝힐 뻔한 자리를 갖게 하기 위해서다.
 * @note makedefs 도구에는 전혀 빌드되지 않는다. 그 도구는 소리 지원을 서술할 필요가 없다.
 */
staticfn int
count_and_validate_soundlibopts(void)
{
    int i, cnt = 0;

    /* soundlib_opts has a fencepost entry at the end */
    for (i = 0; i < SIZE(soundlib_opts) - 1; i++) {
        ++cnt;
        soundlib_opts[i].valid = TRUE;
    }
    return cnt;
}
#endif

/**
 * @brief Append words to the option text being built, wrapping to a new line where they would not fit.
 *
 * Word wrapping done here rather than by the display because the option list is also written to a file and printed by the tool, neither of which has a display to do it for them.
 *
 * @param str the words to append; modified during processing, as the existing comment concedes
 * @param length_p how long the current line already is, updated as words are added
 * @warning The string is chopped up in place -- each space is overwritten with a terminator to isolate a word. A caller passing a string it intends to use afterwards gets back only its first word. Callers pass a scratch copy for this
 *          reason.
 * @note A word longer than a line is not broken; it simply overruns. Option descriptions are written by hand and none is that long, so the case is not handled rather than being handled badly.
 * @note There is disabled code that would have treated " (" as unbreakable, keeping a parenthetical with the word it belongs to. It is left in place as a record of the intent.
 */
/**
 * @brief 조립되고 있는 선택지 글에 단어를 덧붙이면서, 들어가지 않을 곳에서 새 줄로 넘긴다.
 *
 * 단어 줄바꿈이 화면이 아니라 여기서 이루어지는 것은, 선택지 목록이 파일에도 쓰이고 도구에 의해 인쇄되기도 하며, 그 어느 쪽도 그것을 대신 해 줄 화면을 갖지 않기 때문이다.
 *
 * @param str 덧붙일 단어들. 처리 중에 수정된다. 기존 주석이 인정하는 대로다
 * @param length_p 현재 줄이 이미 얼마나 긴지. 단어가 더해질 때 갱신된다
 * @warning 문자열이 제자리에서 잘린다. 단어를 떼어내기 위해 각 공백이 종료 문자로 덮어쓰인다. 나중에 쓸 뜻으로 문자열을 넘긴 호출자는 그 첫 단어만 돌려받는다. 호출자들이 그 이유로 임시 사본을 넘긴다.
 * @note 한 줄보다 긴 단어는 쪼개지지 않고 그냥 넘쳐 흐른다. 선택지 설명은 손으로 쓰이며 그렇게 긴 것은 없으므로, 그 경우가 나쁘게 다뤄지는 대신 다뤄지지 않는다.
 * @note " ("를 쪼갤 수 없는 것으로 다루어 괄호구를 그것이 속한 단어와 붙여 두었을 비활성화된 코드가 있다. 그 의도의 기록으로 남겨져 있다.
 */
staticfn void
opt_out_words(
    char *str,     /* input, but modified during processing */
    int *length_p) /* in/out */
{
    char *word;

    while (*str) {
        word = strchr(str, ' ');
#if 0
        /* treat " (" as unbreakable space */
        if (word && *(word + 1) == '(')
            word = strchr(word + 1,  ' ');
#endif
        if (word)
            *word = '\0';
        if (*length_p + (int) strlen(str) > COLNO - 5) {
            STOREOPTTEXT(optbuf);
            Sprintf(optbuf, "%s", opt_indent),
                *length_p = (int) strlen(opt_indent);
        } else {
            Sprintf(eos(optbuf), " "), (*length_p)++;
        }
        Sprintf(eos(optbuf), "%s", str), *length_p += (int) strlen(str);
        str += strlen(str) + (word ? 1 : 0);
    }
}

/**
 * @brief Compose the whole build-options report, line by line, into the stored text.
 *
 * The report a player pastes into a bug issue and a maintainer reads to know what they are looking at. Its value is entirely in being complete, which is why it lists things nobody would think to ask about.
 *
 * Four parts, in order: the version, then the features compiled in, then the window interfaces (marking which is the default), then the sound libraries. Each is wrapped to the terminal width as it is added.
 *
 * @note The compatibility sentence must already have been composed, because it is a table entry rather than something this function formats -- so build_savebones_compat_string() is called first here rather than being left to the
 *       caller.
 * @note The default window interface is marked in the list rather than stated separately, so that a player cannot report the list and omit which one was in use.
 * @warning Each line is copied to freshly allocated memory as it is finished, and the count of lines is bounded by a fixed maximum. Adding enough options to exceed it truncates the report silently -- the report would then be reassuring
 *          and wrong, which is worse than absent.
 */
/**
 * @brief 빌드 선택지 보고 전체를 한 줄씩 저장된 글로 조립한다.
 *
 * 플레이어가 버그 이슈에 붙여 넣고, 관리자가 자기가 무엇을 보고 있는지 알기 위해 읽는 보고다. 그 가치는 온전히 완전함에 있다. 그것이 아무도 물어볼 생각을 하지 않을 것들을 나열하는 이유다.
 *
 * 순서대로 네 부분이다. 판본, 그다음 컴파일에 포함된 기능들, 그다음 창 인터페이스들(어느 것이 기본값인지 표시하면서), 그다음 소리 라이브러리들. 각각이 더해질 때 터미널 너비에 맞춰 줄바꿈된다.
 *
 * @note 호환성 문장이 이미 조립되어 있어야 한다. 그것은 이 함수가 형식화하는 것이 아니라 표의 항목이기 때문이다. 그래서 build_savebones_compat_string()이 호출자에게 맡겨지는 대신 여기서 먼저 불린다.
 * @note 기본 창 인터페이스는 따로 진술되는 대신 목록 안에 표시된다. 플레이어가 목록을 보고하면서 어느 것이 쓰이고 있었는지 빠뜨릴 수 없게 하기 위해서다.
 * @warning 각 줄은 끝날 때 새로 할당된 메모리에 복사되며, 줄의 개수는 고정된 최댓값으로 제한된다. 그것을 넘을 만큼 선택지를 더하면 보고가 조용히 잘린다. 그러면 보고는 안심시키면서 틀린 것이 되며, 그것은 없는 것보다 나쁘다.
 */
staticfn void
build_options(void)
{
    char buf[COLBUFSZ];
    int i, length, winsyscnt, cnt = 0;
    const char *defwinsys = DEFAULT_WINDOW_SYS;
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
    int soundlibcnt;

#ifdef WIN32
    defwinsys = GUILaunched ? "mswin" : "tty";
#endif
#endif
    build_savebones_compat_string();
    STOREOPTTEXT(optbuf);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
#define STATUS_ARG " [beta]"
#else
#define STATUS_ARG " [work-in-progress]"
#endif
#else
#define STATUS_ARG ""
#endif /* NH_DEVEL_STATUS == NH_STATUS_RELEASED */
    Sprintf(optbuf, "%sNetHack version %d.%d.%d%s\n",
            opt_indent, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, STATUS_ARG);
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "Options compiled into this edition:");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */
    Strcat(strcpy(buf, datamodel(0)), " data model,");
    opt_out_words(buf, &length);
    for (i = 0; i < SIZE(build_opts); i++) {
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
#ifdef WIN32
        /* ignore the console entry if GUI version */
        if (GUILaunched
            && !strcmp("screen control via WIN32 console I/O", build_opts[i]))
            continue;
#endif
#endif /* !MAKEDEFS_C && FOR_RUNTIME */
        Strcat(strcpy(buf, build_opts[i]),
               (i < SIZE(build_opts) - 1) ? "," : ".");
        opt_out_words(buf, &length);
    }
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    winsyscnt = count_and_validate_winopts();
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "Supported windowing system%s:",
            (winsyscnt > 1) ? "s" : "");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */

    for (i = 0; i < SIZE(window_opts) - 1; i++) {
        if (!window_opts[i].valid)
            continue;
        Sprintf(buf, "\"%s\"", window_opts[i].id);
        if (strcmp(window_opts[i].name, window_opts[i].id))
            Sprintf(eos(buf), " (%s)", window_opts[i].name);
        /*
         * 1 : foo.
         * 2 : foo and bar,
         * 3+: for, bar, and quux,
         *
         * 2+ will be followed by " with a default of..."
         */
        Strcat(buf, (winsyscnt == 1) ? "." /* no 'default' */
                    : (winsyscnt == 2 && cnt == 0) ? " and"
                      : (cnt == winsyscnt - 2) ? ", and"
                        : ",");
        opt_out_words(buf, &length);
        cnt++;
    }
    if (cnt > 1) {
        /* loop ended with a comma; opt_out_words() will insert a space */
        Sprintf(buf, "with a default of \"%s\".", defwinsys);
        opt_out_words(buf, &length);
    }

#if !defined(MAKEDEFS_C)
    cnt = 0;
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    soundlibcnt = count_and_validate_soundlibopts();
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "Supported soundlib%s:", (soundlibcnt > 1) ? "s" : "");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */

#ifdef USER_SOUNDS
    soundlibcnt += 1;
#endif
    for (i = 0; i < SIZE(soundlib_opts) - 1; i++) {
        const char *soundlib;

        if (!soundlib_opts[i].valid)
            continue;
        soundlib = soundlib_opts[i].text_id;
        if (!strncmp(soundlib, "soundlib_", 9))
            soundlib += 9;
        Sprintf(buf, "\"%s\"", soundlib);
        /*
         * 1 : foo.
         * 2 : foo and bar.
         * 3+: for, bar, and quux.
         */
        Strcat(buf, (soundlibcnt == 1 || cnt == soundlibcnt - 1)
                    ? "." /* no 'with default' */
                    : (soundlibcnt == 2 && cnt == 0) ? " and"
                      : (cnt == soundlibcnt - 2) ? ", and"
                        : ",");
        opt_out_words(buf, &length);
        cnt++;
    }
#ifdef USER_SOUNDS
    if (cnt > 1) {
        /* loop ended with a comma; opt_out_words() will insert a space */
        Sprintf(buf, "user sounds.");
        opt_out_words(buf, &length);
    }
#endif
#endif  /* !MAKEDEFS_C */

    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)
    {
        static const char *const lua_info[] = {
 "", "NetHack 5.0.* uses the 'Lua' interpreter to process some data:", "",
 "    :LUACOPYRIGHT:", "",
 /*        1         2         3         4         5         6         7
  1234567890123456789012345678901234567890123456789012345678901234567890123456
  */
 ("    \"Permission is hereby granted, free of charge,"
  " to any person obtaining"),
 "     a copy of this software and associated documentation files (the ",
 "     \"Software\"), to deal in the Software without restriction including",
 "     without limitation the rights to use, copy, modify, merge, publish,",
 "     distribute, sublicense, and/or sell copies of the Software, and to ",
 "     permit persons to whom the Software is furnished to do so, subject to",
 "     the following conditions:",
 "     The above copyright notice and this permission notice shall be",
 "     included in all copies or substantial portions of the Software.\"",
            (const char *) 0
        };

        /* add lua copyright notice;
           ":TAG:" substitutions are deferred to caller */
        for (i = 0; lua_info[i]; ++i) {
            STOREOPTTEXT(lua_info[i]);
        }
    }
#endif /* MAKEDEFS_C || FOR_RUNTIME */

    /* end with a blank line */
    STOREOPTTEXT("");
    return;
}

#undef STOREOPTTEXT

/**
 * @brief Compose the build-options report if it has not been composed yet.
 *
 * Done on demand rather than at startup because most games never ask for it, and composing it allocates a line at a time.
 *
 * @note The order here is a dependency chain, not a preference: the compatibility sentence has to exist before the options table is read, the version numbers before anything reports them, and the build date is filled in from a
 *       separately generated file before the report can name it.
 * @note Safe to call repeatedly. The guard is what makes do_runtime_info() able to call this itself, so no caller has to remember to initialise first.
 */
/**
 * @brief 빌드 선택지 보고가 아직 조립되지 않았다면 조립한다.
 *
 * 시작할 때가 아니라 요청될 때 이루어지는 것은, 대부분의 게임이 그것을 결코 요청하지 않으며 그것을 조립하는 것이 한 줄씩 메모리를 할당하기 때문이다.
 *
 * @note 여기의 순서는 취향이 아니라 의존 사슬이다. 호환성 문장이 선택지 표가 읽히기 전에 존재해야 하고, 판본 숫자가 무엇이든 그것을 보고하기 전에 있어야 하며, 빌드 날짜가 보고가 그것을 이름 짓기 전에 따로 생성된 파일에서 채워져야 한다.
 * @note 되풀이해 호출해도 안전하다. 그 방벽이 do_runtime_info()가 이것을 스스로 호출할 수 있게 만드는 것이며, 그래서 어떤 호출자도 먼저 초기화하는 것을 기억해야 할 필요가 없다.
 */
void
runtime_info_init(void)
{
    if (!done_runtime_opt_init_once) {
        done_runtime_opt_init_once = 1;
        build_savebones_compat_string();
        /* construct the current version number */
        make_version();
        populate_nomakedefs(&version);          /* date.c */
        idxopttext = 0;
        build_options();
    }
}

/**
 * @brief Hand back the report one line at a time.
 *
 * Shaped this way because the callers differ in what they do with the lines -- one puts them in a menu, another writes them to a file -- and a function that returned the whole report would have to choose a joining that suited neither.
 *
 * @param rtcontext the caller's place in the report, advanced by one on each successful call; the caller starts it at zero
 * @return the next line, or null when there are no more
 * @note Composes the report itself on the first call, so a caller need only start the counter at zero.
 * @warning The returned string belongs to the report, not to the caller. It stays valid until release_runtime_info() frees the report, and a caller that keeps the pointer past that point is holding freed memory.
 * @note An out-of-range counter yields null rather than being an error, which lets a caller stop by seeing null instead of having to know the line count in advance.
 */
/**
 * @brief 보고를 한 줄씩 돌려준다.
 *
 * 이런 모양인 것은, 호출자들이 그 줄들로 하는 일이 다르기 때문이다. 하나는 그것들을 메뉴에 넣고, 다른 하나는 파일에 쓴다. 보고 전체를 돌려주는 함수는 어느 쪽에도 맞지 않는 이어붙임을 골라야 했을 것이다.
 *
 * @param rtcontext 보고 안에서 호출자의 자리. 성공하는 호출마다 하나씩 나아간다. 호출자가 그것을 영에서 시작한다
 * @return 다음 줄, 또는 더 없을 때 널
 * @note 첫 호출에서 보고를 스스로 조립하므로, 호출자는 계수기를 영에서 시작하기만 하면 된다.
 * @warning 돌려준 문자열은 호출자가 아니라 보고의 것이다. release_runtime_info()가 보고를 해제할 때까지 유효하며, 그 시점을 넘겨 그 포인터를 지니는 호출자는 해제된 메모리를 붙들고 있는 것이다.
 * @note 범위를 벗어난 계수기는 오류가 되는 대신 널을 낸다. 그것이 호출자가 줄 수를 미리 알아야 하는 대신 널을 봄으로써 멈출 수 있게 한다.
 */
const char *
do_runtime_info(int *rtcontext)
{
    const char *retval = (const char *) 0;

    if (!done_runtime_opt_init_once)
        runtime_info_init();
    if (idxopttext && rtcontext)
        if (*rtcontext >= 0 && *rtcontext < MAXOPT) {
            retval = opttext[*rtcontext];
            *rtcontext += 1;
        }
    return retval;
}

/**
 * @brief Discard the composed report and everything it allocated.
 * @note Clears the composed-once flag as well, so this is a reset rather than only a teardown: a later request composes the report again. That is why it is safe to call before the game ends and not only at exit.
 * @note Also releases the separately generated build-date strings, because they were filled in as part of composing this and nothing else owns them.
 * @warning Any line handed out by do_runtime_info() is freed here. A caller that stored one rather than copying it is left with a dangling pointer, and nothing detects that.
 */
/**
 * @brief 조립된 보고와 그것이 할당한 모든 것을 버린다.
 * @note 조립 완료 표시도 지우므로, 이것은 해체일 뿐 아니라 초기화다. 나중의 요청은 보고를 다시 조립한다. 그것이 게임이 끝날 때만이 아니라 그 전에 호출해도 안전한 이유다.
 * @note 따로 생성된 빌드 날짜 문자열도 해제한다. 그것들이 이것을 조립하는 일부로 채워졌고, 다른 무엇도 그것들을 소유하지 않기 때문이다.
 * @warning do_runtime_info()가 내준 어떤 줄이든 여기서 해제된다. 그것을 복사하는 대신 저장한 호출자는 매달린 포인터를 갖게 되며, 그것을 알아채는 것은 없다.
 */
void
release_runtime_info(void)
{
    while (idxopttext > 0) {
        --idxopttext;
        free((genericptr_t) opttext[idxopttext]), opttext[idxopttext] = 0;
    }
    done_runtime_opt_init_once = 0;
    free_nomakedefs();
}

/*mdlib.c*/
