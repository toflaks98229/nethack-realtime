/* NetHack 5.0	pcconf.h	$NHDT-Date: 1781973085 2026/06/20 16:31:25 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.40 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file pcconf.h
 * @brief What the game needs to know when it is built for MS-DOS.
 *
 * One of the platform family. What makes this one distinctive is not the operating system but the constraints of the machines it ran on: little memory, short file names, no ability
 * to run another program while the game is loaded.
 *
 * A good deal of what is here is a consequence of that. The file names differ from every other platform's because eight characters and three were the limit. The graphics options
 * are about detecting and switching video hardware, which no other platform does from inside the game. And the compiler detection at the top exists because several quite different
 * compilers targeted this platform and each announces itself differently.
 *
 * @note The comment listing the automatic definitions is the useful part of that detection: it records which symbol comes from which compiler, so a new one can be recognised by
 *       following the pattern.
 * @note One of the listed compilers is noted as not having been verified for some years, which is honest about what "supported" means here.
 * @warning The short file names are not a preference. A name longer than the limit is silently truncated by the system, and two names that differ only past it become the same file.
 */

/**
 * @file pcconf.h
 * @brief 게임이 MS-DOS 를 위해 빌드될 때 알아야 하는 것.
 *
 * 플랫폼 가족의 하나. 이것을 특징적으로 만드는 것은 운영 체제가 아니라 그것이 돌아갔던 기계의 제약이다. 적은 메모리, 짧은 파일 이름, 게임이 적재된 동안 다른 프로그램을 돌릴 수 없음.
 *
 * 여기 있는 것의 상당 부분이 그 결과다. 파일 이름이 다른 모든 플랫폼과 다른 것은 여덟 자와 세 자가 한계였기 때문이다. 그래픽 선택지는 비디오 하드웨어를 감지하고 전환하는 것에 관한 것인데, 다른 어느 플랫폼도 게임 안에서 그것을 하지 않는다. 그리고 위쪽의 컴파일러
 * 감지가 있는 것은 꽤 다른 여러 컴파일러가 이 플랫폼을 대상으로 했고 각각이 자신을 다르게 알리기 때문이다.
 *
 * @note 자동 정의를 나열하는 주석이 그 감지의 쓸모 있는 부분이다. 어느 심볼이 어느 컴파일러에서 오는지 기록하므로, 새 컴파일러를 그 방식을 따라 인식시킬 수 있다.
 * @note 나열된 컴파일러 중 하나는 몇 해 동안 검증되지 않았다고 적혀 있으며, 그것은 여기서 "지원"이 무엇을 뜻하는지에 대해 정직하다.
 * @warning 짧은 파일 이름은 선호가 아니다. 한계보다 긴 이름은 시스템이 조용히 잘라 내며, 그 지점 뒤에서만 다른 두 이름은 같은 파일이 된다.
 */

#ifndef PCCONF_H
#define PCCONF_H

#define MICRO /* always define this! */

#ifdef MSDOS /* some of this material is MS-DOS specific */

/*
 *  Automatic Defines:
 *
 *     __GO32__ is defined automatically by the djgpp port of gcc.
 *     __DJGPP__ is defined automatically by djgpp version 2 and above.
 *     _MSC_VER is defined automatically by Microsoft C.
 *     __BORLANDC__ is defined automatically by Borland C.
 *     __SC__ is defined automatically by Symantec C.
 *     Note: 3.6.x was not verified with Symantec C.
 */

#define CONFIG_FILE "defaults.nh"
#define GUIDEBOOK_FILE "Guidebook.txt"

/*
 *  The following options are somewhat configurable depending on
 *  your compiler.
 */

/*
 *  For pre-V7.0 Microsoft Compilers only, manually define OVERLAY here.
 */

/*#define OVERLAY */ /* Manual overlay definition (MSC 6.0ax only) */

#ifndef CROSS_TO_AMIGA
#define SHELL /* via exec of COMMAND.COM */
#endif

/*
 * Screen control options
 *
 * You may uncomment:
 *    ANSI_DEFAULT
 *    or   TERMLIB
 *    or   ANSI_DEFAULT and TERMLIB
 *    or   NO_TERMS
 */

/* # define TERMLIB */ /* enable use of termcap file /etc/termcap */
                       /* or ./termcap for MSDOS (SAC) */
                       /* compile and link in Fred Fish's termcap library, */
                       /* enclosed in TERMCAP.ARC, to use this */

/* # define ANSI_DEFAULT */ /* allows NetHack to run without a ./termcap */

#define NO_TERMS /* Allows Nethack to run without ansi.sys by linking */
                 /* screen routines into the .exe     */

#ifdef NO_TERMS     /* if NO_TERMS select one screen package below */
#define SCREEN_BIOS /* Use bios calls for all screen control */
/* #define SCREEN_DJGPPFAST */ /* Use djgpp fast screen routines */
#endif

/* # define PC9800 */ /* Allows NetHack to run on NEC PC-9800 machines */
/* Yamamoto Keizo */

/*
 * PC video hardware support options (for graphical tile support)
 *
 * You may uncomment any/all of the options below.
 *
 */
#ifndef SUPPRESS_GRAPHICS
#if (defined(SCREEN_BIOS) || defined(SCREEN_DJGPPFAST)) && !defined(PC9800)
#ifdef TILES_IN_GLYPHMAP
#define SCREEN_VGA /* Include VGA graphics routines in the build */
#define SCREEN_VESA
#endif
#endif
#else
#undef NO_TERMS
#undef SCREEN_BIOS
#undef SCREEN_DJGPPFAST
#undef SCREEN_VGA
#undef SCREEN_VESA
#undef TERMLIB
#define ANSI_DEFAULT
#endif

#ifndef CROSS_TO_AMIGA
#define RANDOM /* have Berkeley random(3) */
#endif

#define MAIL /* Allows for fake mail daemon to deliver mail */
             /* in the MSDOS version.  (For AMIGA MAIL see  */
             /* amiconf.h).  In the future this will be the */
             /* hook for mail reader implementation.        */

/* The following is needed for prototypes of certain functions */

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__SC__)
#include <process.h> /* Provides prototypes of exit(), spawn()      */
#endif

#ifdef CROSS_TO_AMIGA
#include <spawn.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 7)
#include <sys/types.h>
#ifdef strcmpi
#undef strcmpi
#endif
#include <conio.h>
#include <io.h>
#include <direct.h>
#define SIG_RET_TYPE void(__cdecl *)(int)
#define vprintf printf
#define vfprintf fprintf
#define vsprintf sprintf
#endif

#ifndef M
#define M(c) ((char) (0x80 | (c)))
#endif

/*
 * On the VMS and unix, this option controls whether a delay is done by
 * the clock, or whether it is done by excess output.  On the PC, however,
 * there is always a clock to use for the delay.  The TIMED_DELAY option
 * on MSDOS (without the termcap routines) is used to determine whether to
 * include the delay routines in the code (and thus, provides a compile time
 * method to turn off napping for visual effect).  However, it is also used
 * in the music code to wait between different notes.  So it is needed in that
 * case as well.

 * Whereas on the VMS and unix, flags.nap is a run-time option controlling
 * whether there is a delay by clock or by excess output, on MSDOS it is
 * simply a flag to turn on or off napping for visual effects at run-time.
 */

#define TIMED_DELAY /* enable the `timed_delay' run-time option */

#define NOCWD_ASSUMPTIONS /* Allow paths to be specified for HACKDIR,      \
                             LEVELDIR, SAVEDIR, BONESDIR, DATADIR,         \
                             SCOREDIR, LOCKDIR, CONFIGDIR, and TROUBLEDIR. \
                             */

#endif /* MSDOS configuration stuff */

#ifndef PATHLEN
#define PATHLEN 64  /* maximum pathlength */
#endif
#define FILENAME 80 /* maximum filename length (conservative) */
#ifndef MICRO_H
#include "micro.h" /* contains necessary externs for [os_name].c */
#endif

/* ===================================================
 *  The remaining code shouldn't need modification.
 */

#ifndef SYSTEM_H
#if !defined(_MSC_VER)
/* #include "system.h" */
#endif
#endif

#ifdef __DJGPP__
#include <unistd.h> /* close(), etc. */
/* lock() in io.h interferes with lock[] in decl.h */
#define lock djlock
#include <io.h>
#undef lock
#include <pc.h> /* kbhit() */
#define PC_LOCKING
#define SELF_RECOVER /* NetHack itself can recover games */
#endif

#ifdef MSDOS
#ifndef EXEPATH
#define EXEPATH /* HACKDIR is .exe location if not explicitly defined */
#endif
#endif

#if defined(_MSC_VER) && defined(MSDOS)
#if (_MSC_VER >= 700) && !defined(FUNCTION_LEVEL_LINKING)
#ifndef MOVERLAY
#define MOVERLAY /* Microsoft's MOVE overlay system (MSC >= 7.0) */
#endif
#endif
#define PC_LOCKING
#endif

/* Borland Stuff */
#if defined(__BORLANDC__)
#if defined(__OVERLAY__) && !defined(VROOMM)
/* __OVERLAY__ is automatically defined by Borland C if overlay option is on
 */
#define VROOMM /* Borland's VROOMM overlay system */
#endif
#if !defined(STKSIZ)
#define STKSIZ 5 * 1024 /* Use a default of 5K stack for Borland C      */
                        /* This macro is used in any file that contains */
                        /* a main() function.                           */
#endif
#define PC_LOCKING
#endif

#ifdef PC_LOCKING
#define HLOCK "NHPERM"
#endif

/* the high quality random number routines */
#ifndef USE_ISAAC64
# ifdef RANDOM
#  define Rand() random()
# else
#  define Rand() rand()
# endif
#endif

#ifndef TOS
#define FCMASK 0660 /* file creation mask */
#endif

#include <fcntl.h>

#ifdef MSDOS
#define PORT_HELP "msdoshlp.txt" /* msdos port specific help file */
#endif

/* Sanity check, do not modify these blocks. */

#if defined(MSDOS) && defined(NO_TERMS)
#ifdef TERMLIB
#if defined(_MSC_VER) || defined(__SC__)
#pragma message("Warning -- TERMLIB defined with NO_TERMS in pcconf.h")
#pragma message("           Forcing undef of TERMLIB")
#endif
#undef TERMLIB
#endif
#ifdef ANSI_DEFAULT
#if defined(_MSC_VER) || defined(__SC__)
#pragma message("Warning -- ANSI_DEFAULT defined with NO_TERMS in pcconf.h")
#pragma message("           Forcing undef of ANSI_DEFAULT")
#endif
#undef ANSI_DEFAULT
#endif
/* only one screen package is allowed */
#if defined(SCREEN_BIOS) && defined(SCREEN_DJGPPFAST)
#if defined(_MSC_VER) || defined(__SC__)
#pragma message("Warning -- More than one screen package defined in pcconf.h")
#endif
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__SC__)
#if defined(SCREEN_DJGPPFAST)
#if defined(_MSC_VER) || defined(__SC__)
#pragma message("           Forcing undef of SCREEN_DJGPPFAST")
#endif
#undef SCREEN_DJGPPFAST /* Can't use djgpp fast with other compilers anyway \
                           */
#endif
#else
/* djgpp C compiler */
#if defined(SCREEN_BIOS)
#undef SCREEN_BIOS
#endif
#endif
#endif
#define ASCIIGRAPH
#define VIDEOSHADES
/* SCREEN_8514, SCREEN_VESA are only placeholders presently - sub VGA instead
 */
#if defined(SCREEN_8514)
#undef SCREEN_8514
#define SCREEN_VGA
#endif
/* Graphical tile sanity checks */
#ifdef SCREEN_VGA
#define SIMULATE_CURSOR
#define POSITIONBAR
/* Select appropriate tile file format, and map size */
#define PLANAR_FILE
#define SMALL_MAP
#endif
#endif /* End of sanity check block */

#if defined(MSDOS) && defined(DLB)
#define FILENAME_CMP stricmp /* case insensitive */
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 7)
#pragma warning(disable : 4131)
#pragma warning(disable : 4135)
#pragma warning(disable : 4309)
#pragma warning(disable : 4746)
#pragma warning(disable : 4761)
#endif

#ifdef TIMED_DELAY
#ifdef __DJGPP__
#define msleep(k) (void) usleep((k) *1000)
#endif
#ifdef __BORLANDC__
#define msleep(k) delay(k)
#endif
#ifdef __SC__
#define msleep(k) (void) usleep((long)((k) *1000))
#endif
#endif

#endif /* PCCONF_H */
