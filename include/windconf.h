/* NetHack 5.0	windconf.h	$NHDT-Date: 1781973091 2026/06/20 16:31:31 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.133 $ */
/* Copyright (c) NetHack PC Development Team 1993, 1994.  */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file windconf.h
 * @brief What the game needs to know when it is built for Windows.
 *
 * One of the platform family, and the one this fork is built with. Shorter than the Unix header because Windows is one system rather than a family, so most of the differences it
 * would otherwise have to resolve simply do not arise.
 *
 * What is here divides into three. Where the game finds itself and its files -- including the option to take its own location as the game directory, which is what makes an
 * installation portable. What it does when a game ends badly: locking so an interrupted game is not overwritten without asking, and the ability to repair one rather than lose it.
 * And the site configuration, enabled here, so one installation can be shared.
 *
 * @note The one option left disabled has its reason recorded beside it: running a shell from inside the game hung. That is a note about a specific failure rather than a policy, and
 *       it is why the option is commented rather than removed.
 * @note This is a modified copy of NetHack, and the real-time work is built through this platform's display. That work is not conditional on anything here -- the switch is in the
 *       shared configuration -- but this is the header that decides which display it runs under.
 */

/**
 * @file windconf.h
 * @brief 게임이 Windows 를 위해 빌드될 때 알아야 하는 것.
 *
 * 플랫폼 가족의 하나이며, 이 포크가 그것으로 빌드되는 것이다. Unix 헤더보다 짧은 것은 Windows 가 가족이 아니라 하나의 시스템이기 때문이다. 그래서 그러지 않으면 해결해야 했을 차이 대부분이 애초에 생기지 않는다.
 *
 * 여기 있는 것은 셋으로 나뉜다. 게임이 자신과 자기 파일을 어디서 찾는지 -- 자기 위치를 게임 디렉토리로 삼는 선택지를 포함하며, 그것이 설치본을 휴대 가능하게 만드는 것이다. 게임이 나쁘게 끝났을 때 무엇을 하는지. 중단된 게임이 묻지 않고 덮어써지지 않도록 하는 잠금과,
 * 그것을 잃는 대신 복구할 수 있는 능력. 그리고 여기서 켜지는 사이트 설정. 그래서 하나의 설치본을 공유할 수 있다.
 *
 * @note 꺼진 채로 남은 하나의 선택지에는 그 이유가 곁에 기록되어 있다. 게임 안에서 셸을 돌리면 멈춰 버렸다. 그것은 방침이 아니라 특정한 실패에 대한 메모이며, 그 선택지가 없애지지 않고 주석 처리된 이유다.
 * @note 이것은 NetHack 의 수정된 사본이며, 실시간 작업은 이 플랫폼의 표시부를 통해 빌드된다. 그 작업은 여기의 어떤 것에도 조건적이지 않다 -- 그 스위치는 공유 설정에 있다 -- 그러나 그것이 어느 표시부 아래에서 돌아가는지를 정하는 것이 이 헤더다.
 */

#ifndef WINDCONF_H
#define WINDCONF_H

/* #define SHELL */    /* nt use of pcsys routines caused a hang */

#define EXEPATH              /* Allow .exe location to be used as HACKDIR */
#define TRADITIONAL_GLYPHMAP /* Store glyph mappings at level change time */

#define LAN_FEATURES /* Include code for lan-aware features. Untested in \
                        3.4.0*/

#define PC_LOCKING /* Prevent overwrites of aborted or in-progress games */
/* without first receiving confirmation. */

#define SELF_RECOVER /* Allow the game itself to recover from an aborted \
                        game */

#define SYSCF                /* Use a global configuration */
#define SYSCF_FILE "sysconf" /* Use a file to hold the SYSCF configuration */

#ifdef DUMPLOG
#define DUMPLOG_FILE "%TEMP%/nethack-%n-%d.log"
#endif

/*#define CHANGE_COLOR*/ /* allow palette changes */

#define QWERTZ_SUPPORT  /* when swap_yz is True, numpad 7 is 'z' not 'y' */

#define OPTIONS_AT_RUNTIME  /* build info done at runtime not text file */

#define EARLY_CONFIGFILE_PASS
#define TTY_PERM_INVENT

#ifdef WIN32CON
#define IDLECHECKPOINT
#endif

#define TIMED_DELAY

/*
 * -----------------------------------------------------------------
 *  The remaining code shouldn't need modification.
 * -----------------------------------------------------------------
 */
/* #define SHORT_FILENAMES */ /* All NT filesystems support long names now
 */

#ifdef DLB
#define VERSION_IN_DLB_FILENAME     /* Append version digits to nhdat */
#endif

#ifdef MICRO
#undef MICRO /* never define this! */
#endif

#define NOCWD_ASSUMPTIONS /* Always define this. There are assumptions that \
                             it is defined for WIN32.                       \
                             Allow paths to be specified for HACKDIR,       \
                             LEVELDIR, SAVEDIR, BONESDIR, DATADIR,          \
                             SCOREDIR, LOCKDIR, CONFIGDIR, and TROUBLEDIR */
#define NO_TERMS
#define ASCIIGRAPH

#ifdef OPTIONS_USED
#undef OPTIONS_USED
#endif
#define OPTIONS_USED "options"
#define OPTIONS_FILE OPTIONS_USED

#define PORT_HELP "porthelp"

#define PORT_DEBUG /* include ability to debug international keyboard issues \
                      */

#define RUNTIME_PORT_ID /* trigger run-time port identification for \
                         * identification of exe CPU architecture   \
                         */
#define RUNTIME_PASTEBUF_SUPPORT


#define SAFERHANGUP /* Define SAFERHANGUP to delay hangup processing   \
                     * until the main command loop. 'safer' because it \
                     * avoids certain cheats and also avoids losing    \
                     * objects being thrown when the hangup occurs.    \
                     */

#define CONFIG_FILE ".nethackrc"
#define CONFIG_TEMPLATE "nethackrc.template"
#define SYSCF_TEMPLATE "sysconf.template"
#define SYMBOLS_TEMPLATE "symbols.template"
#define GUIDEBOOK_FILE "Guidebook.txt"

/* Stuff to help the user with some common, yet significant errors */
#define INTERJECT_PANIC 0
#define INTERJECTION_TYPES (INTERJECT_PANIC + 1)
extern void interject_assistance(int, int, genericptr_t, genericptr_t);
extern void interject(int);
extern char *windows_exepath(void);

/*
 *===============================================
 * Compiler-specific adjustments
 *===============================================
 */

#ifdef __GNUC__
#define MD_USE_TMPFILE_S
#
#ifdef strncasecmp
#undef strncasecmp
#endif
#ifdef strcasecmp
#undef strcasecmp
/* https://sourceforge.net/p/mingw-w64/wiki2/gnu%20printf/ */
#endif
/* extern int getlock(void); */
#endif   /* __GNUC__ */

#ifdef _MSC_VER
#define MD_USE_TMPFILE_S
#define HAS_STDINT
#if (_MSC_VER > 1000)
/* Visual C 8 warning elimination */
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#pragma warning(disable : 4996) /* VC8 deprecation warnings */
#pragma warning(disable : 4142) /* benign redefinition */
#pragma warning(disable : 4267) /* conversion from 'size_t' to XX */
#if (_MSC_VER > 1600)
#pragma warning(disable : 4459) /* hide global declaration */
#endif                          /* _MSC_VER > 1600 */
#endif                          /* _MSC_VER > 1000 */
#pragma warning(disable : 4761) /* integral size mismatch in arg; conv \
                                   supp*/
#ifdef YYPREFIX
#pragma warning(disable : 4102) /* unreferenced label */
#endif
#ifdef __cplusplus
/* suppress a warning in cppregex.cpp */
#pragma warning(disable : 4101) /* unreferenced local variable */
#endif
#ifndef HAS_STDINT_H
#define HAS_STDINT_H    /* force include of stdint.h in integer.h */
#endif
/* Turn on some additional warnings */
#pragma warning(3:4389)

/* supply ssize_t */
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

#endif /* _MSC_VER */

/* The following is needed for prototypes of certain functions */
#if defined(_MSC_VER)
#include <process.h> /* Provides prototypes of exit(), spawn()      */
#endif

#include <string.h> /* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#define strncmpi(a, b, c) strnicmp(a, b, c)
#endif


#include <sys/types.h>
#ifdef __BORLANDC__
#undef randomize
#undef random
#endif

#define PATHLEN BUFSZ  /* maximum pathlength */
#define FILENAME BUFSZ /* maximum filename length (conservative) */

#if defined(_MAX_PATH) && defined(_MAX_FNAME)
#if (_MAX_PATH < BUFSZ) && (_MAX_FNAME < BUFSZ)
#undef PATHLEN
#undef FILENAME
#define PATHLEN _MAX_PATH
#define FILENAME _MAX_FNAME
#endif
#endif

#define NO_SIGNAL
#define USE_STDARG

/* Use the high quality random number routines. */
#ifdef USE_ISAAC64
#undef RANDOM
#else
#define RANDOM
#define Rand() random()
#endif

/* Fall back to C's if nothing else, but this really isn't acceptable */
#if !defined(USE_ISAAC64) && !defined(RANDOM)
#define Rand() rand()
#endif

#include <sys/stat.h>
#define FCMASK (_S_IREAD | _S_IWRITE) /* file creation mask */
#define regularize nt_regularize
#define HLOCK "NHPERM"

#ifndef M
#define M(c) ((char) (0x80 | (c)))
/* #define M(c) ((c) - 128) */
#endif

#ifndef C
#define C(c) (0x1f & (c))
#endif

#if defined(DLB) || defined(_MSC_VER)
#define FILENAME_CMP stricmp /* case insensitive */
#endif

/* this was part of the MICRO stuff in the past */
extern const char *alllevels, *allbones;
#define ABORT C('a')
#define getuid() 1
#define getlogin() ((char *) 0)
extern void win32_abort(void);
extern void consoletty_preference_update(const char *);
extern void toggle_mouse_support(void);
extern void map_subkeyvalue(char *);
extern void set_altkeyhandling(const char *);
extern void raw_clear_screen(void);

#include <fcntl.h>
#ifndef __BORLANDC__
#include <io.h>
#include <direct.h>
#else
int _RTLENTRY _EXPFUNC access(const char _FAR *__path, int __amode);
int _RTLENTRY _EXPFUNC _chdrive(int __drive);
int _RTLENTRYF _EXPFUNC32 chdir(const char _FAR *__path);
char _FAR *_RTLENTRY _EXPFUNC getcwd(char _FAR *__buf, int __buflen);
int _RTLENTRY _EXPFUNC
write(int __handle, const void _FAR *__buf, unsigned __len);
int _RTLENTRY _EXPFUNC creat(const char _FAR *__path, int __amode);
int _RTLENTRY _EXPFUNC close(int __handle);
int _RTLENTRY _EXPFUNC _close(int __handle);
int _RTLENTRY _EXPFUNC
open(const char _FAR *__path, int __access, ... /*unsigned mode*/);
long _RTLENTRY _EXPFUNC lseek(int __handle, long __offset, int __fromwhere);
int _RTLENTRY _EXPFUNC read(int __handle, void _FAR *__buf, unsigned __len);
#endif
#undef kbhit /* Use our special NT kbhit */
#define kbhit (*nt_kbhit)

#ifdef LAN_FEATURES
#define MAX_LAN_USERNAME 20
#endif

#ifndef alloca
#define ALLOCA_HACK /* used in util/panic.c */
#endif

extern int set_win32_option(const char *, const char *);
#define LEFTBUTTON FROM_LEFT_1ST_BUTTON_PRESSED
#define RIGHTBUTTON RIGHTMOST_BUTTON_PRESSED
#define MIDBUTTON FROM_LEFT_2ND_BUTTON_PRESSED
#define MOUSEMASK (LEFTBUTTON | RIGHTBUTTON | MIDBUTTON)
#ifdef CHANGE_COLOR
extern int alternative_palette(char *);
#endif

#define nethack_enter(argc, argv) nethack_enter_windows()
extern boolean file_exists(const char *);
extern boolean file_newer(const char *, const char *);
#ifndef SYSTEM_H
/* #include "system.h" */
#endif

#if defined(WIN_CE)
#define QSORTCALLBACK __cdecl
#endif

/* Override the default version of nhassert.  The default version is unable
 * to generate a string form of the expression due to the need to be
 * compatible with compilers which do not support macro stringization (i.e.
 * #x to turn x into its string form).
 */
extern void nt_assert_failed(const char *, const char *, int);
#define nhassert(expression) (void)((!!(expression)) || \
        (nt_assert_failed(#expression, __FILE__, __LINE__), 0))

#endif /* WINDCONF_H */
