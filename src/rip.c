/* NetHack 5.0	rip.c	$NHDT-Date: 1781973064 2026/06/20 16:31:04 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.49 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file rip.c
 * @brief 게임 종료 시 텍스트 묘비(tombstone) 화면을 구성하는 모듈.
 *
 * 플레이어 이름, 소지 금액, 사망 원인, 사망 연도를 ASCII 아트 묘비 그림에
 * 채워 넣어 출력한다. 텍스트 묘비를 사용하는 윈도우 포트가 빌드에 포함될
 * 때(@c TEXT_TOMBSTONE)에만 컴파일된다.
 */

#include "hack.h"

/* Defining TEXT_TOMBSTONE causes genl_outrip() to exist, but it doesn't
   necessarily have to be used by a binary with multiple window-ports */

#if defined(TTY_GRAPHICS) || defined(X11_GRAPHICS) || defined(GEM_GRAPHICS) \
    || defined(DUMPLOG) || defined(CURSES_GRAPHICS) || defined(SHIM_GRAPHICS) \
    || defined(AMII_GRAPHICS)
#define TEXT_TOMBSTONE
#endif
#if defined(mac) || defined(__BEOS__)
#ifndef TEXT_TOMBSTONE
#define TEXT_TOMBSTONE
#endif
#endif

#ifdef TEXT_TOMBSTONE
staticfn void center(int, char *);

#ifndef NH320_DEDICATION
/** @brief 게임 종료 화면에 쓰이는 표준 묘비 ASCII 아트 템플릿. */
/* A normal tombstone for end of game display. */
static const char *const rip_txt[] = {
    "                       ----------",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |", /* Name of player */
    "                  |                  |", /* Amount of $ */
    "                  |                  |", /* Type of death */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |       1001       |", /* Real year of death */
    "                 *|     *  *  *      | *",
    "        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______", 0
};
#define STONE_LINE_CENT 28 /* char[] element of center of stone face */
#else                      /* NH320_DEDICATION */
/** @brief Izchak Miller 추모용 이중 묘비 ASCII 아트 템플릿(NH 3.2.x 헌정). */
/* NetHack 3.2.x displayed a dual tombstone as a tribute to Izchak. */
static const char *const rip_txt[] = {
    "              ----------                      ----------",
    "             /          \\                    /          \\",
    "            /    REST    \\                  /    This    \\",
    "           /      IN      \\                /  release of  \\",
    "          /     PEACE      \\              /   NetHack is   \\",
    "         /                  \\            /   dedicated to   \\",
    "         |                  |            |  the memory of   |",
    "         |                  |            |                  |",
    "         |                  |            |  Izchak Miller   |",
    "         |                  |            |   1935 - 1994    |",
    "         |                  |            |                  |",
    "         |                  |            |     Ascended     |",
    "         |       1001       |            |                  |",
    "      *  |     *  *  *      | *        * |      *  *  *     | *",
    (" _____)/\\|\\__//(\\/(/\\)/\\//\\/|_)___"
     "_____)/|\\\\_/_/(\\/(/\\)/\\/\\/|_)____"),
    0
};
#define STONE_LINE_CENT 19 /* char[] element of center of stone face */
#endif                     /* NH320_DEDICATION */
#define STONE_LINE_LEN  16 /* # chars that fit on one line
                            * (note 1 ' ' border)           */
#define NAME_LINE  6 /* *char[] line # for player name */
#define GOLD_LINE  7 /* *char[] line # for amount of gold */
#define DEATH_LINE 8 /* *char[] line # for death description */
#define YEAR_LINE 12 /* *char[] line # for year */

/**
 * @brief 지정한 묘비 줄의 중앙에 텍스트를 배치한다(범용 텍스트 묘비 출력).
 *
 * 사망 화면용 묘비 그림을 동적으로 복제한 뒤 이름/금액/사망 원인/연도를
 * 각 줄 중앙에 채워 넣고 지정한 윈도우에 출력한다.
 *
 * @param[in] tmpwin 출력 대상 윈도우 식별자.
 * @param[in] how    사망 원인 코드(@c formatkiller 로 설명 문자열을 만든다).
 * @param[in] when   사망 시각(연도 표시에 사용).
 * @note 함수 내에서 묘비 버퍼를 할당하고 사용 후 모두 해제한다(누수 없음).
 */
void
genl_outrip(winid tmpwin, int how, time_t when)
{
    char **dp;
    char *dpx;
    char buf[BUFSZ];
    int x;
    int line, year;
    long cash;

    gr.rip = dp = (char **) alloc(sizeof(rip_txt));
    for (x = 0; rip_txt[x]; ++x)
        dp[x] = dupstr(rip_txt[x]);
    dp[x] = (char *) 0;

    /* Put name on stone */
    Sprintf(buf, "%.*s", (int) STONE_LINE_LEN, svp.plname);
    center(NAME_LINE, buf);

    /* Put $ on stone */
    cash = max(gd.done_money, 0L);
    /* arbitrary upper limit; practical upper limit is quite a bit less */
    if (cash > 999999999L)
        cash = 999999999L;
    Sprintf(buf, "%ld Au", cash);
    center(GOLD_LINE, buf);

    /* Put together death description */
    formatkiller(buf, sizeof buf, how, FALSE);

    /* Put death type on stone */
    for (line = DEATH_LINE, dpx = buf; line < YEAR_LINE; line++) {
        char tmpchar;
        int i, i0 = (int) strlen(dpx);

        if (i0 > STONE_LINE_LEN) {
            for (i = STONE_LINE_LEN; (i > 0) && (i0 > STONE_LINE_LEN); --i)
                if (dpx[i] == ' ')
                    i0 = i;
            if (!i)
                i0 = STONE_LINE_LEN;
        }
        tmpchar = dpx[i0];
        dpx[i0] = 0;
        center(line, dpx);
        if (tmpchar != ' ') {
            dpx[i0] = tmpchar;
            dpx = &dpx[i0];
        } else
            dpx = &dpx[i0 + 1];
    }

    /* Put year on stone */
    year = (int) ((yyyymmdd(when) / 10000L) % 10000L);
    Sprintf(buf, "%4d", year);
    center(YEAR_LINE, buf);

#ifdef DUMPLOG
    if (tmpwin == 0)
        dump_forward_putstr(0, 0, "Game over:", TRUE);
    else
#endif
        putstr(tmpwin, 0, "");

    for (; *dp; dp++)
        putstr(tmpwin, 0, *dp);

    putstr(tmpwin, 0, "");
#ifdef DUMPLOG
    if (tmpwin != 0)
#endif
        putstr(tmpwin, 0, "");

    for (x = 0; rip_txt[x]; x++) {
        free((genericptr_t) gr.rip[x]);
    }
    free((genericptr_t) gr.rip);
    gr.rip = 0;
}

/**
 * @brief 묘비의 한 줄 중앙에 텍스트를 복사해 넣는다.
 *
 * @param[in] line 텍스트를 채울 묘비 줄 번호.
 * @param[in] text 해당 줄 중앙에 배치할 널 종료 문자열.
 * @warning 대상 버퍼 @c gr.rip[line] 의 폭을 넘지 않는 짧은 문자열이어야 하며,
 *          길이 검사는 호출자가 책임진다.
 */
staticfn void
center(int line, char *text)
{
    char *ip, *op;
    ip = text;
    op = &gr.rip[line][STONE_LINE_CENT - ((strlen(text) + 1) >> 1)];
    while (*ip)
        *op++ = *ip++;
}

#endif /* TEXT_TOMBSTONE */

/*rip.c*/
