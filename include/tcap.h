/* NetHack 5.0	tcap.h	$NHDT-Date: 1781973089 2026/06/20 16:31:29 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.17 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

/* not named termcap.h because it may conflict with a system header */

/**
 * @file tcap.h
 * @brief What the terminal can do, as short names the tty code uses directly.
 *
 * Terminals differ in the sequences that move the cursor, highlight text or
 * switch to a line-drawing font, so those are looked up at startup and kept in
 * one place. The abbreviations here are the traditional termcap names, which is
 * why they are two letters rather than descriptive.
 *
 * Whether any of this exists depends on the build: platforms without a termcap
 * library are handled by leaving @c TERMLIB undefined, so the same tty code
 * compiles with or without it.
 *
 * @note The global block is also declared in decl.h and defined in decl.c; the
 *       copy here is guarded so that including both is harmless.
 * @warning @c tc_ul_hack records a terminal that underlines by overwriting
 *          rather than by attribute -- a workaround, not a capability, and one
 *          that changes what drawing is safe.
 */

/**
 * @file tcap.h
 * @brief 터미널이 무엇을 할 수 있는지. tty 코드가 그대로 쓰는 짧은 이름들.
 *
 * 터미널마다 커서를 옮기고, 글자를 강조하고, 선 그리기 글꼴로 바꾸는 시퀀스가 다르다.
 * 그래서 그것들을 시작 시 조회해 한곳에 보관한다. 여기의 약어들은 전통적인 termcap
 * 이름이며, 그래서 설명적이지 않고 두 글자다.
 *
 * 이것이 존재하는지 자체가 빌드에 달려 있다. termcap 라이브러리가 없는 플랫폼은
 * @c TERMLIB 를 정의하지 않는 방식으로 처리되므로, 같은 tty 코드가 그것이 있든 없든
 * 컴파일된다.
 *
 * @note 전역 블록은 decl.h 에도 선언되고 decl.c 에서 정의된다. 여기 사본은 가드로
 *       감싸여 있어 둘을 함께 포함해도 무해하다.
 * @warning @c tc_ul_hack 은 속성이 아니라 덮어쓰기로 밑줄을 그리는 터미널을 기록한다.
 *          기능이 아니라 우회책이며, 어떤 그리기가 안전한지를 바꾼다.
 */

#ifndef TCAP_H
#define TCAP_H

#ifndef MICRO
#define TERMLIB /* include termcap code */
#endif

/* might display need graphics code? */
#if !defined(AMIGA) && !defined(TOS) && !defined(MAC68K)
#if defined(TERMLIB) || defined(OS2) || defined(MSDOS)
#define ASCIIGRAPH
#endif
#endif

#ifndef DECL_H
extern struct tc_gbl_data { /* also declared in decl.h; defined in decl.c */
    char *tc_AS, *tc_AE;    /* graphics start and end (tty font swapping) */
    int tc_LI, tc_CO;       /* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

extern struct tc_lcl_data { /* defined and set up in termcap.c */
    char *tc_CM, *tc_ND, *tc_CD;
    char *tc_HI, *tc_HE, *tc_US, *tc_UE;
    boolean tc_ul_hack;
} tc_lcl_data;
/* some curses.h declare CM etc. */
#define nh_CM tc_lcl_data.tc_CM
#define nh_ND tc_lcl_data.tc_ND
#define nh_CD tc_lcl_data.tc_CD
#define nh_HI tc_lcl_data.tc_HI
#define nh_HE tc_lcl_data.tc_HE
#define nh_US tc_lcl_data.tc_US
#define nh_UE tc_lcl_data.tc_UE
#define ul_hack tc_lcl_data.tc_ul_hack

extern short ospeed; /* set up in termcap.c */

#ifdef TOS
extern const char *hilites[CLR_MAX];
#else
extern NEARDATA char *hilites[CLR_MAX];
#endif

#endif /* TCAP_H */
