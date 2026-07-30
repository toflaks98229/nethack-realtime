/* NetHack 3.6	amiconf.h	$NHDT-Date: 1432512775 2015/05/25 00:12:55 $  $NHDT-Branch: master $:$NHDT-Revision: 1.12 $ */
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990, 1991, 1992, 1993.
 */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file amiconf.h
 * @brief What the game needs to know when it is built for the Amiga.
 *
 * One of the platform family, and the one that reaches furthest into the system. It includes the operating system's own headers directly, which no other platform header does, and
 * it declares its own settings structure and colour handling -- so it is not only a set of choices but a piece of the port.
 *
 * It begins by undefining three names before anything else, and the reason is in the comments: the system's headers define them as macros, and the game wants the functions or its
 * own versions. That has to happen first, before those headers are read.
 *
 * @note The settings structure at the end is this platform's alone. Its members are each conditional on the feature they serve, so the structure's size differs with what was built
 *       in -- which is acceptable only because nothing saves it.
 * @note The file ends by withdrawing the site configuration. That is the platform saying it cannot support a shared installation, which is the same pattern of a platform header
 *       overriding the shared configuration seen in the VMS one.
 * @warning It is read last of the platform headers, as the shared configuration notes, because it needs everything else settled before it undoes what it must.
 */

/**
 * @file amiconf.h
 * @brief 게임이 Amiga 를 위해 빌드될 때 알아야 하는 것.
 *
 * 플랫폼 가족의 하나이며, 시스템 안으로 가장 깊이 손을 뻗는 것이다. 운영 체제 자신의 헤더를 직접 포함하는데 다른 어느 플랫폼 헤더도 그러지 않으며, 자기 설정 구조체와 색 처리를 선언한다. 그래서 그것은 선택의 묶음만이 아니라 포트의 한 조각이다.
 *
 * 다른 무엇보다 먼저 세 이름을 정의 해제하며 시작하는데, 그 이유가 주석에 있다. 시스템의 헤더가 그것들을 매크로로 정의하고, 게임은 함수나 자기 판본을 원한다. 그것은 그 헤더들이 읽히기 전에 먼저 일어나야 한다.
 *
 * @note 끝의 설정 구조체는 이 플랫폼만의 것이다. 그 멤버들이 각각 자신이 맡는 기능에 조건적이므로, 구조체의 크기가 무엇이 빌드에 포함되었는지에 따라 다르다. 그것을 저장하는 것이 아무것도 없기 때문에만 받아들일 만하다.
 * @note 이 파일은 사이트 설정을 철회하며 끝난다. 그것은 플랫폼이 공유 설치본을 지원할 수 없다고 말하는 것이며, VMS 헤더에서 본 것과 같은, 플랫폼 헤더가 공유 설정을 덮어쓰는 방식이다.
 * @warning 공유 설정이 밝히듯 플랫폼 헤더 중 마지막으로 읽힌다. 되돌려야 할 것을 되돌리기 전에 다른 모든 것이 정해져 있어야 하기 때문이다.
 */

#ifndef AMICONF_H
#define AMICONF_H

#undef abs /* avoid using macro form of abs */
#undef min /* this gets redefined */
#undef max /* this gets redefined */

#include <time.h> /* get time_t defined before use! */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <proto/dos.h>

#define MICRO /* must be defined to allow some inclusions */

#define NOCWD_ASSUMPTIONS /* Allow paths to be specified for HACKDIR, \
                             LEVELDIR, SAVEDIR, BONESDIR, DATADIR,    \
                             SCOREDIR, LOCKDIR, CONFIGDIR, and TROUBLEDIR */

#define PATHLEN 130

/* data librarian defs */
#define DLBFILE "nhdat"   /* main library */
/* nhsdat sound library not used in 5.0 */
#undef DLBFILE2

#define FILENAME_CMP strcmpi /* case insensitive */
#define O_BINARY 0

#define MFLOPPY /* You'll probably want this; provides assistance \
                 * for typical personal computer configurations   \
                 */

/* ### amidos.c ### */

extern void nethack_exit(int);

/* ### winreq.c ### */

extern void amii_setpens(int);

extern void getlind(const char *, char *, const char *);
extern void CleanUp(void);
extern void Abort(long) NORETURN;
extern int getpid(void);
extern int kbhit(void);
extern int WindowGetchar(void);
extern void ami_wininit_data(int);

#ifndef MICRO_H
#include "micro.h"
#endif

#ifndef PCCONF_H
#include "pcconf.h" /* remainder of stuff is almost same as the PC */
#endif

#define remove(x) unlink(x)
#define rewind(f) fseek(f, 0, 0)

/*
 *  (Possibly) configurable Amiga options:
 */

#define HACKFONT  /* Use special hack.font */
#define MAIL      /* Get mail at unexpected occasions */
#define AMIFLUSH /* toss typeahead (select flush in .cnf) */

/* new window system options */
/* WRONG - AMIGA_INTUITION should go away */
#ifdef AMII_GRAPHICS
#define AMIGA_INTUITION /* high power graphics interface (amii) */
#endif

#define CHANGE_COLOR 1
#define DEPTH 6 /* Maximum depth of the screen allowed */
#define AMII_MAXCOLORS (1L << DEPTH)
/* Number of palette entries actually populated in amii_init_map[] (AMII text
 * mode) and amiv_init_map[] (AMIV tile mode).  Indices beyond these read 0. */
#define AMII_PALETTE_SIZE 8
#define AMIV_PALETTE_SIZE 32
typedef unsigned short AMII_COLOR_TYPE;

#define PORT_HELP "amii.hlp"

#undef TERMLIB

#ifdef AMII_GRAPHICS
extern int amii_numcolors;
void amii_setpens(int);
#endif

/**
 * @brief This platform's own settings, which no other platform has.
 * @note Almost every member is conditional on the feature it belongs to, so the structure differs between builds. That is safe here because it is session state and is not saved.
 * @note The identifier at the front is a name held as text -- a way of recognising the structure at run time, which matters on a platform where a settings block might be inspected
 *       outside the program.
 */
/**
 * @brief 이 플랫폼만의 설정. 다른 어느 플랫폼도 갖지 않는 것.
 * @note 거의 모든 멤버가 자신이 속한 기능에 조건적이므로, 이 구조체는 빌드마다 다르다. 여기서 그것이 안전한 것은 세션 상태이고 저장되지 않기 때문이다.
 * @note 앞쪽의 식별자는 글로 보관된 이름이다. 실행 중에 이 구조체를 알아보는 방법이며, 설정 블록이 프로그램 밖에서 살펴질 수도 있는 플랫폼에서는 그것이 중요하다.
 */
struct ami_sysflags {
    char sysflagsid[10];
#ifdef AMIFLUSH
    boolean altmeta;  /* use ALT keys as META */
    boolean amiflush; /* kill typeahead */
#endif
#ifdef AMII_GRAPHICS
    int numcols;
    unsigned short amii_dripens[20]; /* DrawInfo Pens currently there are 13 in v39 */
    AMII_COLOR_TYPE amii_curmap[AMII_MAXCOLORS]; /* colormap */
#endif
#ifdef MFLOPPY
    boolean asksavedisk;
#endif
};
extern struct ami_sysflags sysflags;

#undef SYSCF
#undef SYSCF_FILE

#endif /* AMICONF_H */
