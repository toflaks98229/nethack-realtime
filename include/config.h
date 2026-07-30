/* NetHack 5.0	config.h	$NHDT-Date: 1710344316 2024/03/13 15:38:36 $  $NHDT-Branch: keni-staticfn $:$NHDT-Revision: 1.188 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2016. */
/* NetHack may be freely redistributed.  See license for details. */
/* MODIFIED 2026-07 (real-time fork): added the REALTIME_PROTO / RT_TURN_MS
   real-time build switch; see MODIFICATIONS.md.  This file differs from the
   upstream NetHack distribution. */

/**
 * @file config.h
 * @brief What to build, decided before anything is compiled.
 *
 * The first header read, and it decides what the rest of the game is. Which platform, which display, which optional features, how large the shared files are allowed to
 * be. Everything downstream is conditional on what is chosen here.
 *
 * The file is meant to be edited. It is organised in numbered sections with prose explaining each choice, and most lines are commented-out options with a note on when to
 * enable them -- so it reads as instructions to a person rather than as declarations to a compiler.
 *
 * That is worth keeping in mind while reading it. A commented-out definition here is not dead code; it is an option not taken, and the comment beside it is the reason.
 *
 * @note Platform detection happens in an included header, so the platform sections here mostly need no change -- the compiler is asked what it is rather than told.
 * @note This is a modified copy of NetHack. The real-time switch at the top of the file is this fork's, and the file header above records that; the modifications document
 *       has the details.
 * @warning Choices here change the save format and the data files. A saved game or a data file from a build with different options may not be readable, and the version
 *          check exists to catch that rather than to prevent it.
 */

/**
 * @file config.h
 * @brief 무엇을 빌드할지. 무엇이 컴파일되기 전에 정해진다.
 *
 * 가장 먼저 읽히는 헤더이며, 게임의 나머지가 무엇인지를 정한다. 어느 플랫폼, 어느 표시부, 어떤 선택 기능, 공유 파일이 얼마나 커도 되는지. 하류의 모든 것이 여기서 골라진 것에 조건적이다.
 *
 * 이 파일은 편집되도록 의도되어 있다. 번호가 붙은 절로 조직되어 있고 각 선택을 설명하는 산문이 딸려 있으며, 대부분의 줄이 주석 처리된 선택지와 그것을 언제 켜야 하는지에 대한 메모다. 그래서 컴파일러를 향한 선언이 아니라 사람을 향한 지침으로 읽힌다.
 *
 * 그것을 읽는 동안 염두에 둘 만하다. 여기의 주석 처리된 정의는 죽은 코드가 아니다. 택하지 않은 선택지이며, 그 곁의 주석이 그 이유다.
 *
 * @note 플랫폼 감지는 포함된 헤더에서 일어나므로, 여기의 플랫폼 절은 대부분 손댈 필요가 없다. 컴파일러에게 말해 주는 것이 아니라 무엇인지 물어본다.
 * @note 이것은 NetHack 의 수정된 사본이다. 파일 맨 위의 실시간 스위치가 이 포크의 것이며, 위의 파일 머리말이 그것을 기록한다. 자세한 것은 수정 문서에 있다.
 * @warning 여기의 선택은 저장 형식과 데이터 파일을 바꾼다. 옵션이 다른 빌드의 저장 게임이나 데이터 파일은 읽히지 않을 수 있으며, 버전 검사는 그것을 막기 위해서가 아니라 잡아내기 위해 존재한다.
 */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define CONFIG_H

/**
 * @name Real-time conversion switch
 * @brief This fork's own option: run the world on a clock rather than on the player's input.
 *
 * In stock NetHack the world is frozen while the game waits for a keystroke. With this defined it is not: a shared wall clock advances the world one turn every so many
 * real milliseconds whether or not anything was typed, so monsters keep moving and timeouts keep running while the player thinks.
 *
 * The turn length is the pace of the game and the poll interval is how finely input is checked while waiting. The second is not a game setting -- it trades responsiveness
 * against how much processor time is spent waiting.
 *
 * @note Two ports implement it, as the existing comment records: the console loop and the Windows tile display. A port that does not is unaffected and stays turn-based.
 * @note Commenting the switch out restores stock behaviour, which is the point of having it as a switch rather than a rewrite.
 * @warning Not part of upstream NetHack. Everything conditional on it is this fork's, and the surrounding game was written on the assumption that the world waits.
 * @{
 */
/**
 * @name 실시간 전환 스위치
 * @brief 이 포크 자체의 선택지. 세계를 플레이어의 입력이 아니라 시계로 돌린다.
 *
 * 원래의 NetHack 에서는 게임이 키 입력을 기다리는 동안 세계가 멈춰 있다. 이것이 정의되면 그렇지 않다. 공유된 실제 시계가 무엇이 입력되었는지와 무관하게 실제 밀리초 단위마다 세계를 한 턴 진행시키므로, 플레이어가 생각하는 동안에도 몬스터는 계속 움직이고
 * 남은 시간은 계속 흐른다.
 *
 * 턴 길이가 게임의 속도이고, 폴링 간격은 기다리는 동안 입력을 얼마나 촘촘히 확인하는지다. 두 번째는 게임 설정이 아니다. 반응성과 기다리는 데 쓰이는 프로세서 시간을 절충한다.
 *
 * @note 기존 주석이 기록하듯 두 포트가 이것을 구현한다. 콘솔 루프와 Windows 타일 표시부. 그러지 않는 포트는 영향을 받지 않고 턴제로 남는다.
 * @note 이 스위치를 주석 처리하면 원래 동작으로 돌아간다. 그것이 이것을 다시 쓰기가 아니라 스위치로 두는 요점이다.
 * @warning 상류 NetHack 의 일부가 아니다. 이것에 조건적인 모든 것이 이 포크의 것이며, 둘레의 게임은 세계가 기다린다는 전제 위에 쓰였다.
 * @{
 */
/*
 * Real-time prototype switch (turn-based -> real-time conversion).
 * When defined, the hero no longer freezes the world while waiting for
 * input.  A shared wall-clock (rt_world_tick_ready) advances the world one
 * game turn every RT_TURN_MS of real time, regardless of input, so monsters
 * and timeouts keep running and the pace stays constant no matter how fast
 * keys are pressed.  Handled in allmain.c (console) and mswproc.c (win32
 * tile/GUI port).  Comment out to restore stock turn-based play.
 */
#define REALTIME_PROTO
#ifdef REALTIME_PROTO
#define RT_TURN_MS 150 /* real milliseconds per game turn at normal speed;
                        * smaller = faster action, larger = calmer pace */
#define RT_POLL_MS 10  /* input-poll / CPU-yield granularity while waiting */
#endif
/** @} */

/**
 * @name Section 1: platform and display
 * @brief Which operating system and which display this build is for.
 *
 * Mostly nothing to change. The platform is detected by the included header rather than declared here, and the prose alongside each option says when it would need
 * enabling -- generally when a compiler or terminal misbehaves in a specific way.
 *
 * @note More than one display may be built in at once. The choice between them is then made when the game runs, which is why they are not mutually exclusive here.
 * @{
 */
/**
 * @name 1절: 플랫폼과 표시부
 * @brief 이 빌드가 어느 운영 체제와 어느 표시부를 위한 것인지.
 *
 * 대부분 바꿀 것이 없다. 플랫폼은 여기서 선언되는 것이 아니라 포함된 헤더가 감지하며, 각 선택지 곁의 산문이 그것을 언제 켜야 하는지 밝힌다. 대체로 컴파일러나 터미널이 특정한 방식으로 잘못 동작할 때다.
 *
 * @note 여러 표시부가 한 번에 빌드에 포함될 수 있다. 그 사이의 선택은 게임이 돌아갈 때 이뤄지며, 그래서 여기서 그것들이 서로 배타적이지 않다.
 * @{
 */
/*
 * Section 1:   Operating and window systems selection.
 *              Select the version of the OS you are using.
 *              For "UNIX" select BSD, ULTRIX, SYSV, or HPUX in unixconf.h.
 *              A "VMS" option is not needed since the VMS C-compilers
 *              provide it (no need to change sec#1, vmsconf.h handles it).
 *              MacOSX uses the UNIX configuration, not the old MAC one.
 */

#define UNIX /* delete if no fork(), exec() available */

/* #define MSDOS */ /* in case it's not auto-detected */

/* #define OS2 */ /* define for OS/2 */

/* #define TOS */ /* define for Atari ST/TT */

/* #define STUPID */ /* avoid some complicated expressions if
                      * your C compiler chokes on them */
/* #define MINIMAL_TERM */
/* if a terminal handles highlighting or tabs poorly,
   try this define, used in pager.c and termcap.c */
/* #define ULTRIX_CC20 */
/* define only if using cc v2.0 on a DECstation */
/* #define ULTRIX_PROTO */
/* define for Ultrix 4.0 (or higher) on a DECstation;
 * if you get compiler errors, don't define this. */
/* Hint: if you're not developing code, don't define
   ULTRIX_PROTO. */

#include "config1.h" /* should auto-detect MSDOS, MAC68K, AMIGA, and WIN32 */

/*
 * Consolidated version, patchlevel, development status.
 */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif


/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */
#if !defined(NOTTYGRAPHICS)
#define TTY_GRAPHICS /* good old tty-based graphics */
#endif
/* #define CURSES_GRAPHICS *//* Curses interface - Karl Garrison*/
/* #define X11_GRAPHICS */   /* X11 interface */
/* #define QT_GRAPHICS */    /* Qt interface */
/* #define MSWIN_GRAPHICS */ /* Windows NT, CE, Graphics */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system (see defines above).  Known window systems are:
 *
 *      tty, X11, mac, amii, BeOS, Qt, Gem, Gnome, shim
 */

/* MAC68K also means MAC windows */
#ifdef MAC68K
#ifndef AUX
#define DEFAULT_WINDOW_SYS "mac"
#endif
#endif

/* Amiga supports AMII_GRAPHICS and/or TTY_GRAPHICS */
#ifdef AMIGA
#define AMII_GRAPHICS             /* (optional) */
#define DEFAULT_WINDOW_SYS "amii" /* "amii", "amitile" or "tty" */
#endif

/* Atari supports GEM_GRAPHICS and/or TTY_GRAPHICS */
#ifdef TOS
#define GEM_GRAPHICS             /* Atari GEM interface (optional) */
#define DEFAULT_WINDOW_SYS "Gem" /* "Gem" or "tty" */
#endif

#ifdef __BEOS__
#define BEOS_GRAPHICS             /* (optional) */
#define DEFAULT_WINDOW_SYS "BeOS" /* "tty" */
#ifndef HACKDIR                   /* override the default hackdir below */
#define HACKDIR "/boot/apps/NetHack"
#endif
#endif

#ifdef QT_GRAPHICS
#ifndef DEFAULT_WC_TILED_MAP
#define DEFAULT_WC_TILED_MAP /* Default to tiles if users doesn't request
                              * wc_ascii_map */
#endif
#ifndef USE_XPM
#define USE_XPM           /* Use XPM format for images (required) */
#endif
#ifndef GRAPHIC_TOMBSTONE
#define GRAPHIC_TOMBSTONE /* Use graphical tombstone (rip.ppm) */
#endif
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "Qt"
#endif
#endif

#ifdef GNOME_GRAPHICS
#ifndef USE_XPM
#define USE_XPM           /* Use XPM format for images (required) */
#endif
#ifndef GRAPHIC_TOMBSTONE
#define GRAPHIC_TOMBSTONE /* Use graphical tombstone (rip.ppm) */
#endif
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "Gnome"
#endif
#endif

#ifdef MSWIN_GRAPHICS
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "mswin"
#endif
#define HACKDIR "\\nethack"
#endif

#ifdef TTY_GRAPHICS
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "tty"
#endif
#endif

#ifdef CURSES_GRAPHICS
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "curses"
#endif
#endif

#ifdef SHIM_GRAPHICS
#ifndef DEFAULT_WINDOW_SYS
#define DEFAULT_WINDOW_SYS "shim"
#endif
#endif

#ifdef X11_GRAPHICS
/*
 * There are two ways that X11 tiles may be defined:
 * (1) using a custom format loaded by NetHack code.
 * (2) using the XPM format loaded by the free XPM library.
 * The second option allows you to then use other programs to
 * generate tiles files.  For example, the PBMPlus tools
 * would allow:
 *  xpmtoppm <x11tiles.xpm | pnmscale 1.25 | ppmquant 90 | \
 *      ppmtoxpm >x11tiles_big.xpm
 */
/* # define USE_XPM */ /* Disable if you do not have the XPM library */
#ifdef USE_XPM
#ifndef GRAPHIC_TOMBSTONE
#define GRAPHIC_TOMBSTONE /* Use graphical tombstone (rip.xpm) */
#endif
#endif
#ifndef DEFAULT_WC_TILED_MAP
#define DEFAULT_WC_TILED_MAP /* Default to tiles */
#endif
#endif

/** @} */

/**
 * @name Section 2: shared files and site configuration
 * @brief Where the game keeps its shared state, and what a site administrator may control.
 *
 * The first half names the files several players share -- the score record, the log of finished games, the log of what happened during them. Commenting one out removes that
 * feature rather than breaking it, which is what makes them optional.
 *
 * The second half is the site configuration, and it exists for a specific situation: one installation played by many people. An administrator can then say who is allowed to
 * use debug mode, how many may play at once, how large the score file may grow, and how to reach support. The compiled-in values become defaults which that configuration
 * overrides.
 *
 * @note The prose here is the reference for what a site configuration file may contain. It is documentation for an administrator rather than for a programmer, which is why
 *       it is unusually long.
 * @note Debug mode is always built in now, as the existing comment records. What the configuration controls is who may enter it, not whether it exists.
 * @warning The list of who may debug is one setting that does @e not fall back to the compiled-in value, as the comment states explicitly. An installation that enables site
 *          configuration and omits it allows nobody.
 * @{
 */
/**
 * @name 2절: 공유 파일과 사이트 설정
 * @brief 게임이 공유 상태를 어디에 보관하는지, 그리고 사이트 관리자가 무엇을 통제할 수 있는지.
 *
 * 전반부는 여러 플레이어가 공유하는 파일들의 이름을 붙인다. 점수 기록, 끝난 게임의 로그, 그 동안 무슨 일이 있었는지의 로그. 하나를 주석 처리하면 그 기능이 깨지는 것이 아니라 없어진다. 그것이 그것들을 선택적으로 만드는 것이다.
 *
 * 후반부는 사이트 설정이며, 특정한 상황을 위해 존재한다. 여러 사람이 플레이하는 하나의 설치본. 그러면 관리자가 누가 디버그 모드를 쓸 수 있는지, 몇 명이 동시에 플레이할 수 있는지, 점수 파일이 얼마나 커질 수 있는지, 지원을 어떻게 받는지를 말할 수 있다.
 * 컴파일에 포함된 값들이 그 설정이 덮어쓰는 기본값이 된다.
 *
 * @note 여기의 산문이 사이트 설정 파일이 무엇을 담을 수 있는지에 대한 참고 자료다. 프로그래머가 아니라 관리자를 위한 문서이며, 그래서 유난히 길다.
 * @note 기존 주석이 기록하듯 디버그 모드는 이제 항상 빌드에 포함된다. 설정이 통제하는 것은 그것이 존재하는지가 아니라 누가 그것에 들어갈 수 있는지다.
 * @warning 누가 디버그할 수 있는지의 목록은 컴파일에 포함된 값으로 @e 돌아가지 않는 유일한 설정이며, 그 주석이 그것을 명시한다. 사이트 설정을 켜고 그것을 빠뜨린 설치본은 아무에게도 허용하지 않는다.
 * @{
 */
/*
 * Section 2:   Some global parameters and filenames.
 *
 *              LOGFILE, XLOGFILE, LIVELOGFILE, NEWS and PANICLOG refer to
 *              files in the playground directory.  Commenting out LOGFILE,
 *              XLOGFILE, NEWS or PANICLOG removes that feature from the game.
 *
 *              Building with debugging features enabled is now unconditional;
 *              the old WIZARD setting for that has been eliminated.
 *              If SYSCF is enabled, WIZARD_NAME will be overridden at
 *              runtime by the SYSCF WIZARDS value.
 *
 *              SYSCF:  (not supported by all ports)
 *            If SYSCF is defined, the following configuration info is
 *            available in a global config space, with the compiled-in
 *            entries as defaults:
 *              WIZARDS      (a space-separated list of usernames of users who
 *                           can run the game in debug mode, aka wizard mode;
 *                           a value of * allows anyone to debug;
 *                           this does NOT default to compiled-in value)
 *              EXPLORERS    (who can use explore mode, aka discover mode)
 *              SHELLERS     (who can use ! to execute a shell subprocess)
 *              MAXPLAYERS   (see MAX_NR_OF_PLAYERS below and nethack.sh)
 *              SUPPORT      (how to get local support) [no default]
 *              RECOVER      (how to recover a game at your site) [no default]
 *            For the record file (see topten.c):
 *              PERSMAX      (max entries for one person)
 *              ENTRYMAX     (max entries in the record file)
 *              POINTSMIN    (min points to get an entry)
 *              PERS_IS_UID  (0 or 1 - person is name or (numeric) userid)
 *            Can force incubi/succubi behavior to be toned down to nymph-like:
 *              SEDUCE       (0 or 1 - runtime disable/enable SEDUCE option)
 *            Can hide the entry for displaying command line usage from
 *            the help menu if players don't have access to command lines:
 *              HIDEUSAGE    (0 or 1 - runtime show/hide command line usage)
 *            The following options pertain to crash reporting:
 *              GREPPATH     (the path to the system grep(1) utility)
 *              GDBPATH      (the path to the system gdb(1) program)
 *              CRASHREPORT  (use CRASHREPORTURL if defined in syscf; this
 *                           define specifies the name of the helper program
 *                           used to launch the browser and enables the
 *                           feature))
 *            Regular nethack options can also be specified in order to
 *            provide system-wide default values local to your system:
 *              OPTIONS      (same as in users' .nethackrc or defaults.nh)
 *
 *              In the future there may be other ways to supply SYSCF
 *              information (Windows registry, Apple resource forks, etc)
 *              but at present the only supported method is via a text file.
 *              If the program is built with SYSCF enabled, the file *must*
 *              exist and be readable, otherwise the game will complain and
 *              refuse to start.
 *              SYSCF_FILE:  file containing the SYSCF options shown above;
 *              default is 'sysconf' in nethack's playground.
 */

#ifndef WIZARD_NAME /* allow for compile-time or Makefile changes */
#define WIZARD_NAME "wizard" /* value is ignored if SYSCF is enabled */
#endif

#ifndef SYSCF
#define SYSCF                /* use a global configuration */
#define SYSCF_FILE "sysconf" /* global configuration is in a file */
#endif

#ifndef GDBPATH
#define GDBPATH "/usr/bin/gdb"
#endif
#ifndef GREPPATH
#define GREPPATH "/bin/grep"
#endif

#ifndef NOCRASHREPORT
# ifndef CRASHREPORT
#  ifdef MACOS
#   define CRASHREPORT "/usr/bin/open"
#  endif
#  ifdef __linux__
#   define CRASHREPORT "/usr/bin/xdg-open"
       /* Define this if the terminal is filled with useless error messages
        * when the browser launches. */
#   define CRASHREPORT_EXEC_NOSTDERR
#  endif
#  ifdef WIN32
#   define CRASHREPORT /* builtin helper */
#  endif
# endif
#else
# ifdef CRASHREPORT
#  undef CRASHREPORT
# endif
# if defined(MSDOS) || defined(NOPANICTRACE)
#  undef PANICTRACE
# endif
#endif

#ifdef CRASHREPORT
# ifndef DUMPLOG_CORE
#  define DUMPLOG_CORE     // required to get ^P info
# endif
# ifdef MACOS
#  define PANICTRACE
# endif
# ifdef __linux__
#  define PANICTRACE
#  ifndef NOSTATICFN       // may be defined on command line
#   define NOSTATICFN
#  endif
# endif
// This test isn't quite right: CNG is only available from Windows 2000 on.
// But we'll check that at runtime.
# ifdef WIN32
#  define PANICTRACE
#  define NOSTATICFN
# endif
#endif

#ifdef NONOSTATICFN
# define staticfn static
#else
# ifdef NOSTATICFN
#  define staticfn
# else
#  define staticfn static
# endif
#endif

/* note: "larger" is in comparison with 'record', the high-scores file
   (whose name can be overridden via #define in global.h if desired) */
#define LOGFILE  "logfile"  /* larger file for debugging purposes */
#define XLOGFILE "xlogfile" /* even larger logfile */
#define NEWS     "news"     /* the file containing the latest hack news */
#define PANICLOG "paniclog" /* log of panic and impossible events */

/* alternative paniclog format, better suited for public servers with
   many players, as it saves the player name and the game start time */
/* #define PANICLOG_FMT2 */

/*
 *      When building the program, whether the 'makedefs' utility
 *      checks for non-ASCII or non-printable (control) characters
 *      in various data files (data.base, rumors.tru, rumors.fal,
 *      {oracles,epitaphs,engravings,bogusmons}.txt and warns about them.
 *      They also get changed to '#' instead of possibly remaining
 *      unprintable.
 *
 *      If you modify the data files to intentionally add accented
 *      letters or something comparable, comment this out.  (Such things
 *      won't necessarily work as intended within nethack but at least
 *      makedefs wouldn't reject them.)
 */
#define MAKEDEFS_FILTER_NONASCII

/*
 *      PERSMAX, POINTSMIN, ENTRYMAX, PERS_IS_UID:
 *      These control the contents of 'record', the high-scores file.
 *      They used to be located in topten.c rather than config.h, and
 *      their values can be overridden at runtime (to increase ENTRYMAX, the
 *      maximum number of scores to keep, for example) if SYSCF is enabled.
 */
#ifndef PERSMAX
#define PERSMAX 3 /* entries per name/uid per char. allowed */
#endif
#ifndef POINTSMIN
#define POINTSMIN 1 /* must be > 0 */
#endif
#ifndef ENTRYMAX
#define ENTRYMAX 100 /* must be >= 10 */
#endif
#ifndef PERS_IS_UID
#if !defined(MICRO) && !defined(MAC68K) && !defined(WIN32)
#define PERS_IS_UID 1 /* delete for PERSMAX per name; now per uid */
#else
#define PERS_IS_UID 0
#endif
#endif

/*
 *      NODUMPENUMS
 *      If there are memory constraints and you don't want to store information
 *      about the internal enum values for monsters and objects, this can be
 *      uncommented to define NODUMPENUMS. Doing so will disable the
 *          nethack --dumpenums
 *      command line option.
 *      Note:  the extra memory is also used when ENHANCED_SYMBOLS is
 *      defined, so defining both ENHANCED_SYMBOLS and NODUMPENUMS will limit
 *      the amount of memory and code reduction offered by the latter.
 */
/* #define NODUMPENUMS */

/*
 *      ENHANCED_SYMBOLS
 *      Support the enhanced display of symbols by utilizing utf8.
 *      Enabled by default, but it can be disabled by commenting it out.
 */

#define ENHANCED_SYMBOLS

/*
 *      If COMPRESS is defined, it should contain the full path name of your
 *      'compress' program.
 *
 *      If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *      as the extension your compressor appends to filenames after
 *      compression. Currently, only UNIX fully implements
 *      COMPRESS; other ports should be able to uncompress save files a
 *      la unixmain.c if so inclined.
 *
 *      Defining ZLIB_COMP builds in support for zlib compression. If you
 *      define ZLIB_COMP, you must link with a zlib library. Not all ports
 *      support ZLIB_COMP.
 *
 *      COMPRESS and ZLIB_COMP are mutually exclusive.
 *
 */

#if defined(UNIX) && !defined(ZLIB_COMP) && !defined(COMPRESS)
/* path and file name extension for compression program */
#define COMPRESS "/usr/bin/compress" /* Lempel-Ziv compression */
#define COMPRESS_EXTENSION ".Z"      /* compress's extension */
/* An example of one alternative you might want to use: */
/* #define COMPRESS "/usr/local/bin/gzip" */ /* FSF gzip compression */
/* #define COMPRESS_EXTENSION ".gz" */       /* normal gzip extension */
#endif

#ifndef COMPRESS
/* # define ZLIB_COMP */            /* ZLIB for compression */
#endif

/*
 *      Data librarian.  Defining DLB places most of the support files into
 *      a tar-like file, thus making a neater installation.  See *conf.h
 *      for detailed configuration.
 */
/* #define DLB */ /* not supported on all platforms */

/*
 *      Defining REPRODUCIBLE_BUILD causes 'util/makedefs -v' to construct
 *      date+time in include/date.h (to be shown by nethack's 'v' command)
 *      from SOURCE_DATE_EPOCH in the build environment rather than use
 *      current date+time when makedefs is run.
 *
 *      [The version string will show "last revision <date><time>" instead
 *      of "last build <date><time>" if SOURCE_DATE_EPOCH has a value
 *      which seems valid at the time date.h is generated.  The person
 *      building the program is responsible for setting it correctly,
 *      and the value should be in UTC rather than local time.  NetHack
 *      normally uses local time and doesn't display timezone so toggling
 *      REPRODUCIBLE_BUILD on or off might yield a date+time that appears
 *      to be incorrect relative to what the other setting produced.]
 *
 *      Intent is to be able to rebuild the program with the same value
 *      and obtain an identical copy as was produced by a previous build.
 *      Not necessary for normal game play....
 */
/* #define REPRODUCIBLE_BUILD */ /* use getenv("SOURCE_DATE_EPOCH") instead
                                    of current time when creating date.h */

/*
 *      Defining INSURANCE slows down level changes, but allows games that
 *      died due to program or system crashes to be resumed from the point
 *      of the last level change, after running a utility program.
 */
#define INSURANCE /* allow crashed game recovery */

#if !defined(MAC68K) && !defined(SHIM_GRAPHICS)
#define CHDIR /* delete if no chdir() available */
#endif

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
#ifndef HACKDIR
#define HACKDIR "/usr/games/lib/nethackdir"
#endif

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.  In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE */ /* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS (or use
 * MAXPLAYERS under SYSCF).
 * #define MAX_NR_OF_PLAYERS 6
 */
#endif /* CHDIR */


/** @} */

/**
 * @name Section 3: what the compiler and machine can do
 * @brief Accommodations for compilers and machines that differ from the assumed one.
 *
 * The options here are not preferences but workarounds, each for a specific failing: a compiler that does not understand a basic type, one that claims to follow the standard
 * without doing so, a machine on which the game's small integer types must be wider than a byte.
 *
 * @note Almost all of them are for hardware and compilers long out of use. They are kept because removing one is only safe if nobody is building on such a machine, and that
 *       cannot be established -- the cost of keeping them is a few conditionals.
 * @warning The type widths chosen here are part of the save format. A machine that needs wider small integers cannot read another's saves.
 * @{
 */
/**
 * @name 3절: 컴파일러와 기계가 무엇을 할 수 있는지
 * @brief 전제된 것과 다른 컴파일러와 기계를 위한 조치.
 *
 * 여기의 선택지들은 선호가 아니라 우회책이며, 각각이 특정한 결함을 위한 것이다. 기본 타입을 이해하지 못하는 컴파일러, 표준을 따르지 않으면서 따른다고 주장하는 컴파일러, 게임의 작은 정수 타입이 한 바이트보다 넓어야 하는 기계.
 *
 * @note 거의 전부가 오래전에 쓰이지 않게 된 하드웨어와 컴파일러를 위한 것이다. 남겨 두는 것은, 하나를 없애는 것이 그런 기계에서 아무도 빌드하지 않을 때만 안전하고 그것을 확인할 수 없기 때문이다. 남겨 두는 비용은 조건문 몇 개다.
 * @warning 여기서 골라진 타입 폭은 저장 형식의 일부다. 더 넓은 작은 정수를 필요로 하는 기계는 다른 기계의 저장을 읽을 수 없다.
 * @{
 */
/*
 * Section 3:   Definitions that may vary with system type.
 *              For example, both schar and uchar should be short ints on
 *              the AT&T 3B2/3B5/etc. family.
 */

/*
 * Uncomment the following line if your compiler doesn't understand the
 * 'void' type (and thus would give all sorts of compile errors without
 * this definition).
 */
/* #define NOVOID */ /* define if no "void" data type. */

/*
 * Uncomment the following line if your compiler falsely claims to be
 * a standard C compiler (i.e., defines __STDC__ without cause).
 * Examples are Apollo's cc (in some versions) and possibly SCO UNIX's rcc.
 */
/* #define NOTSTDC */ /* define for lying compilers */

#include "tradstdc.h"

/*
 * type schar:
 * small signed integers (8 bits suffice) (eg. TOS)
 *      typedef char schar;
 * will do when you have signed characters; otherwise use
 *      typedef short int schar;
 */
#ifdef AZTEC
#define schar char
#else
typedef signed char schar;
#endif

/*
 * type uchar:
 * small unsigned integers (8 bits suffice - but 7 bits do not)
 *      typedef unsigned char uchar;
 * will be satisfactory if you have an "unsigned char" type; otherwise use
 *      typedef unsigned short int uchar;
 */
#ifndef _AIX32 /* identical typedef in system file causes trouble */
typedef unsigned char uchar;
#endif

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS /* Good bitfield handling */

/* #define STRNCMPI */ /* compiler/library has the strncmpi function */

/*
 * Vision choices.
 *
 * Things will be faster if you can use MACRO_CPATH.  Some cpps, however,
 * cannot deal with the size of the functions that have been macroized.
 */

#ifndef NO_MACRO_CPATH
#define MACRO_CPATH /* use clear_path macros instead of functions */
#endif

#if !defined(MAC68K)
#if !defined(NOCLIPPING)
#define CLIPPING /* allow smaller screens -- ERS */
#endif
#endif

/* CONFIG_ERROR_SECURE: If user makes NETHACKOPTIONS point to a file ...
 *  TRUE: Show the first error, nothing else.
 *  FALSE: Show all errors as normal, with line numbers and context.
 */
#ifndef CONFIG_ERROR_SECURE
# define CONFIG_ERROR_SECURE TRUE
#endif

/** @} */

/**
 * @name Section 4: experimental options
 * @brief Features that are finished enough to build but not enough to be relied on.
 *
 * The warning above is not boilerplate. These are enabled at the builder's own risk and the existing comment says outright that bugs are expected to remain -- so an option
 * here failing is not necessarily a defect to be reported.
 *
 * @note Each option's prose says both what it does and when to turn it off again -- typically when the code it depends on does not compile or does not work on a given
 *       platform. That is the practical use of this section: it is where a feature waits until it works everywhere.
 * @note Some options here are enabled by default despite the section's warning, which means they have been through enough use to be trusted on the common platforms but not
 *       to be promoted out of the section.
 * @{
 */
/**
 * @name 4절: 실험적 선택지
 * @brief 빌드될 만큼은 완성되었으나 의존할 만큼은 아닌 기능들.
 *
 * 위의 경고는 형식적인 문구가 아니다. 이들은 빌드하는 사람의 책임으로 켜지며, 기존 주석은 버그가 남아 있을 것으로 예상된다고 노골적으로 말한다. 그래서 여기의 어떤 선택지가 실패하는 것이 반드시 보고할 결함은 아니다.
 *
 * @note 각 선택지의 산문이 그것이 무엇을 하는지와 언제 다시 끄면 되는지를 함께 밝힌다. 대체로 그것이 의존하는 코드가 어떤 플랫폼에서 컴파일되지 않거나 작동하지 않을 때다. 그것이 이 절의 실질적인 용도다. 어떤 기능이 어디서나 작동하기까지 기다리는 곳.
 * @note 이 절의 경고에도 불구하고 여기의 몇몇 선택지는 기본으로 켜져 있다. 흔한 플랫폼에서 믿을 만큼은 쓰였지만 이 절 밖으로 승격될 만큼은 아니라는 뜻이다.
 * @{
 */
/*
 * Section 4:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

/* SELECTSAVED: Enable the 'selectsaved' run-time option, allowing it
 * to be set in user's config file or NETHACKOPTIONS.  When set, if
 * player is about to be given the "who are you?" prompt, check for
 * save files and if any are found, put up a menu of them for choosing
 * one to restore (plus extra menu entries "new game" and "quit").
 *
 * Not useful if players are forced to use a specific character name
 * such as their user name.  However in some cases, players can set
 * their character name to one which is classified as generic in the
 * sysconf file (such as "player" or "games")
 *  nethack -u player
 * to force the "who are you?" prompt in which case 'selectsaved' will
 * be honored.
 *
 * Comment out if the wildcard file name lookup in files.c doesn't
 * compile or doesn't work as intended.
 */
#define SELECTSAVED /* support for restoring via menu */

/* TTY_TILES_ESCCODES: Enable output of special console escape codes
 * which act as hints for external programs such as EbonHack or hterm.
 *
 * TTY_SOUND_ESCCODES: Enable output of special console escape codes
 * which act as hints for theoretical external programs to play sound effect.
 *
 * Only for TTY_GRAPHICS.
 *
 * All of the escape codes are in the format ESC [ N z, where N can be
 * one or more positive integer values, separated by semicolons.
 * For example ESC [ 1 ; 0 ; 120 z
 *
 * Possible TTY_TILES_ESCCODES codes are:
 *  ESC [ 1 ; 0 ; n ; m z   Start a glyph (aka a tile) number n, with flags m
 *  ESC [ 1 ; 1 z           End a glyph.
 *  ESC [ 1 ; 2 ; n z       Select a window n to output to.
 *  ESC [ 1 ; 3 z           End of data. NetHack has finished sending data,
 *                          and is waiting for input.
 * Possible TTY_SOUND_ESCCODES codes are:
 *  ESC [ 1 ; 4 ; n ; m z   Play specified sound n, volume m
 *
 * Whenever NetHack outputs anything, it will first output the "select window"
 * code. Whenever NetHack outputs a tile, it will first output the "start
 * glyph" code, then the escape codes for color and the glyph character
 * itself, and then the "end glyph" code.
 *
 * To compile NetHack with this, add tile.c to WINSRC and tile.o to WINOBJ in
 * the hints file or Makefile.  Set boolean option vt_tiledata and/or
 * vt_sounddata in your config file to turn either of these on.  Note that some
 * terminals (e.g. old versions of gnome-terminal) don't work with this. */
/* #define TTY_TILES_ESCCODES */
/* #define TTY_SOUND_ESCCODES */

/* An experimental minimalist inventory list capability under tty if you have
 * at least 28 additional rows beneath the status window on your terminal  */
/* #define TTY_PERM_INVENT */

/* enable status highlighting via STATUS_HILITE directives in run-time
   config file and the 'statushilites' option */
#define STATUS_HILITES         /* support hilites of status fields */

/* #define WINCHAIN */              /* stacked window systems */

#if defined(DEBUG) && !defined(DEBUG_MIGRATING_MONS)
#define DEBUG_MIGRATING_MONS  /* add a wizard-mode command to help debug
                               * migrating monsters */
#endif

/* SCORE_ON_BOTL is neither experimental nor inadequately tested,
   but doesn't seem to fit in any other section... */
/* #define SCORE_ON_BOTL */         /* enable the 'showscore' option to
                                     * show estimated score on status line */

/* FREE_ALL_MEMORY is neither experimental nor inadequately tested,
   but it isn't necessary for successful operation of the program */
#define FREE_ALL_MEMORY             /* free all memory at exit */

/* EXTRA_SANITY_CHECKS adds extra impossible calls,
 * probably not useful for normal play */
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#define EXTRA_SANITY_CHECKS
#endif

/* BREADCRUMBS employs the use of predefined compiler macros
 * __FUNCTION__ and __LINE__ to store some caller breadcrumbs
 * for use during heavy debugging sessions. Only define if your
 * compiler supports those predefined macros and you are debugging */
/* #define BREADCRUMBS */

/* EDIT_GETLIN makes the string input in TTY, curses, Qt4, and X11
   for some prompts be pre-loaded with previously input text (from
   a previous instance of the same prompt) as the default response.
   In some cases, the previous instance can only be within the same
   session; in others, such as #annotate, the previous input can be
   from any session because the response is saved and restored with
   the map.  The 'edit' capability is just <delete> or <backspace>
   to strip off characters at the end, or <escape> to discard the
   whole thing, then type a new end for the text. */
/* #define EDIT_GETLIN */

#ifndef NO_CHRONICLE
/* CHRONICLE - enable #chronicle command, a log of major game events.
   The logged messages will also appear in DUMPLOG. */
#define CHRONICLE
#ifdef CHRONICLE
/* LIVELOG - log CHRONICLE events into LIVELOGFILE as they happen. */
/* #define LIVELOG */
#endif /* CHRONICLE */
#else
#undef LIVELOG
#endif /* NO_CHRONICLE */

/* #define DUMPLOG */  /* End-of-game dump logs */

#define USE_ISAAC64 /* Use cross-platform, bundled RNG */

/* TEMPORARY - MAKE UNCONDITIONAL BEFORE RELEASE */
/* undef this to check if sandbox breaks something */
#define NHL_SANDBOX

#ifdef NHL_SANDBOX
#ifdef CHRONICLE
    /* LIVELOG (and therefore CHRONICLE)  is needed for --loglua */
#ifndef LIVELOG
#define LIVELOG
#endif
#endif
#endif

/* experimential; if the platform/window-port supports it; when the game has
 * started to wait for player input, and the wait lasts longer than
 * IDLECHECKPOINT_WAIT_TIME seconds (defined in hack.h or *conf.h), the game
 * will perform an update to the checkpoint file.
 * Currently has support in:
 *     WIN32CON
 *     Qt
 *     curses
 */

/* #define IDLECHECKPOINT */

/* End of Section 4 */

#ifdef TTY_TILES_ESCCODES
# ifndef TILES_IN_GLYPHMAP
#  define TILES_IN_GLYPHMAP
# endif
#endif

#include "cstd.h"
#include "integer.h"
#include "global.h" /* Define everything else according to choices above */

/* Place the following after #include [platform]conf.h in global.h so that
   overrides are possible in there, for things like unix-specific file
   paths. */

#ifdef LIVELOG
#ifndef LIVELOGFILE
#define LIVELOGFILE "livelog" /* in-game events recorded, live */
#endif /* LIVELOGFILE */
#endif /* LIVELOG */

#ifdef DUMPLOG
#define DUMPLOG_CORE
#ifndef DUMPLOG_FILE
#define DUMPLOG_FILE        "/tmp/nethack.%n.%d.log"
/* DUMPLOG_FILE allows following placeholders:
   %% literal '%'
   %v version (eg. "3.6.3-0")
   %u game UID
   %t game start time, UNIX timestamp format
   %T current time, UNIX timestamp format
   %d game start time, YYYYMMDDhhmmss format
   %D current time, YYYYMMDDhhmmss format
   %n player name
   %N first character of player name
   DUMPLOG_FILE is not used if SYSCF is defined
*/
#endif /* DUMPLOG_FILE */
#endif /* DUMPLOG */
/**
 * @def DUMPLOG_MSG_COUNT
 * @brief How many of the last messages the end-of-game dump includes.
 * @note A window rather than the whole history, because the dump is meant to show how the game ended -- the messages leading up to the death are the interesting ones and the
 *       rest is noise.
 */
/**
 * @def DUMPLOG_MSG_COUNT
 * @brief 게임 종료 시의 덤프가 마지막 메시지 몇 개를 포함하는지.
 * @note 전체 이력이 아니라 창인 것은, 그 덤프가 게임이 어떻게 끝났는지를 보이기 위한 것이기 때문이다. 죽음에 이르는 메시지들이 흥미로운 것이고 나머지는 소음이다.
 */
#ifdef DUMPLOG_CORE
#ifndef DUMPLOG_MSG_COUNT
#define DUMPLOG_MSG_COUNT   50
#endif /* DUMPLOG_MSG_COUNT */
#endif
/** @} */

#endif /* CONFIG_H */
