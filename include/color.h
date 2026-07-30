/* NetHack 5.0	color.h	$NHDT-Date: 1781973077 2026/06/20 16:31:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.27 $ */
/* Copyright (c) Steve Linhart, Eric Raymond, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file color.h
 * @brief The sixteen colours the game describes things in.
 *
 * The palette is the original PC one -- eight colours and their bright
 * counterparts -- and it stays that way because object appearances, monster
 * colours and the data files all name these values. It is a vocabulary the game
 * is written in, not a display setting.
 *
 * Interfaces map these onto whatever the terminal actually has, so a build with
 * more colours available still receives these and widens them itself.
 *
 * @note The exceptions matter: bright black is meaningless on a black screen, so
 *       that slot is the default foreground instead, and low-intensity yellow is
 *       named brown because that is what it looked like.
 */

/**
 * @file color.h
 * @brief 게임이 사물을 서술하는 데 쓰는 열여섯 색.
 *
 * 이 팔레트는 원래의 PC 팔레트다. 여덟 색과 그 밝은 짝이며, 물건의 겉모습과 몬스터의
 * 색, 데이터 파일이 모두 이 값들을 이름으로 부르기 때문에 그대로 유지된다. 표시 설정이
 * 아니라 게임이 쓰여 있는 어휘다.
 *
 * 인터페이스는 이것을 터미널이 실제로 가진 것에 대응시킨다. 그래서 더 많은 색을 쓸 수
 * 있는 빌드도 이 값을 받아 스스로 넓힌다.
 *
 * @note 예외들이 중요하다. 검은 화면에서 밝은 검정은 의미가 없으므로 그 자리는 기본
 *       전경색이 되었고, 낮은 밝기의 노랑은 그렇게 보였기 때문에 갈색이라 불린다.
 */

#ifndef COLOR_H
#define COLOR_H

/*
 * The color scheme used is tailored for an IBM PC.  It consists of the
 * standard 8 colors, followed by their bright counterparts.  There are
 * exceptions, these are listed below.	Bright black doesn't mean very
 * much, so it is used as the "default" foreground color of the screen.
 */
#define CLR_BLACK 0
#define CLR_RED 1
#define CLR_GREEN 2
#define CLR_BROWN 3 /* on IBM, low-intensity yellow is brown */
#define CLR_BLUE 4
#define CLR_MAGENTA 5
#define CLR_CYAN 6
#define CLR_GRAY 7 /* low-intensity white */
#define NO_COLOR 8
#define CLR_ORANGE 9
#define CLR_BRIGHT_GREEN 10
#define CLR_YELLOW 11
#define CLR_BRIGHT_BLUE 12
#define CLR_BRIGHT_MAGENTA 13
#define CLR_BRIGHT_CYAN 14
#define CLR_WHITE 15
#define CLR_MAX 16

/* The "half-way" point for tty-based color systems.  This is used in */
/* the tty color setup code.  (IMHO, it should be removed - dean).    */
#define BRIGHT 8

/* color aliases used in monsters.h and display.c  */
#define HI_DOMESTIC CLR_WHITE /* for player + pets */
#define HI_LORD CLR_MAGENTA /* for high-end monsters */
#define HI_OVERLORD CLR_BRIGHT_MAGENTA /* for few uniques */

/* these can be configured */
#define HI_OBJ CLR_MAGENTA
#define HI_METAL CLR_CYAN
#define HI_COPPER CLR_YELLOW
#define HI_SILVER CLR_GRAY
#define HI_GOLD CLR_YELLOW
#define HI_LEATHER CLR_BROWN
#define HI_CLOTH CLR_BROWN
#define HI_ORGANIC CLR_BROWN
#define HI_WOOD CLR_BROWN
#define HI_PAPER CLR_WHITE
#define HI_GLASS CLR_BRIGHT_CYAN
#define HI_MINERAL CLR_GRAY
#define DRAGON_SILVER CLR_BRIGHT_CYAN
#define HI_ZAP CLR_BRIGHT_BLUE

#define NH_BASIC_COLOR  0x1000000
#define NH_ALTPALETTE   0x2000000
#define COLORVAL(x) ((x) & 0xFFFFFF)

enum nhcolortype { no_color, nh_color, rgb_color };

struct nethack_color {
    enum nhcolortype colortyp;
    int tableindex;
    int rgbindex;
    const char *name;
    long r, g, b;
};

typedef struct color_and_attr {
           int color, attr;
} color_attr;

extern const struct nethack_color colortable[];

#endif /* COLOR_H */

/*color.h*/
