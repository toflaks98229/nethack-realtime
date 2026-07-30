/* NetHack 5.0	dgn_file.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.15 $ */
/* Copyright (c) 1989 by M. Stephenson                            */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DGN_FILE_H
#define DGN_FILE_H

#ifndef ALIGN_H
#include "align.h"
#endif

/**
 * @file dgn_file.h
 * @brief The dungeon's description as the compiler and the loader both see it.
 *
 * The shape of the dungeon -- its branches, how deep each runs, where they join,
 * which levels are fixed and which are random -- is written as a description,
 * compiled into a data file, and read back at the start of a game. This header is
 * what the two ends agree on.
 *
 * The structures are deliberately provisional: a description says "somewhere
 * between here and there" rather than a depth, so a @c couple is a base plus a
 * random spread, and the actual dungeon is settled when a game begins.
 *
 * @note The names carry @c tmp because they are the compiler's working form, not
 *       the dungeon the game plays in; that is built from them.
 */

/**
 * @file dgn_file.h
 * @brief 던전의 기술. 컴파일러와 로더가 함께 보는 형태.
 *
 * 던전의 형태 -- 분기들, 각 분기의 깊이, 어디서 이어지는지, 어떤 레벨이 고정이고 어떤
 * 것이 무작위인지 -- 는 기술문으로 쓰여 데이터 파일로 컴파일되고, 게임 시작 시 다시
 * 읽힌다. 이 헤더가 양쪽이 합의하는 내용이다.
 *
 * 구조체들은 의도적으로 잠정적이다. 기술문은 깊이 대신 "여기서 저기 사이 어딘가"를
 * 말하므로 @c couple 이 기준값과 무작위 범위로 되어 있고, 실제 던전은 게임이 시작될 때
 * 확정된다.
 *
 * @note 이름에 @c tmp 가 붙은 것은 그것이 컴파일러의 작업 형태이지 게임이 실제로
 *       플레이하는 던전이 아니기 때문이다. 후자는 이것들로부터 만들어진다.
 */

/*
 * Structures manipulated by the dungeon loader & compiler
 */

struct couple {
    short base, rand;
};

struct tmpdungeon {
    char *name, *protoname;
    struct couple lev;
    int flags, chance, levels, branches,
        entry_lev; /* entry level for this dungeon */
    char boneschar;
    int align;
};

struct tmplevel {
    char *name;
    char *chainlvl;
    struct couple lev;
    int chance, rndlevs, chain, flags;
    char boneschar;
};

struct tmpbranch {
    char *name; /* destination dungeon name */
    struct couple lev;
    int chain; /* index into tmplevel array (chained branch)*/
    int type;  /* branch type (see below) */
    int up;    /* branch is up or down */
};

/*
 *    Values for type in tmpbranch structure.
 */
#define TBR_STAIR 0   /* connection with both ends having a staircase */
#define TBR_NO_UP 1   /* connection with no up staircase */
#define TBR_NO_DOWN 2 /* connection with no down staircase */
#define TBR_PORTAL 3  /* portal connection */

/*
 *    Flags that map into the dungeon flags bitfields.
 */
#define TOWN        0x01 /* levels only */
#define HELLISH     0x02
#define MAZELIKE    0x04
#define ROGUELIKE   0x08
#define UNCONNECTED 0x10

#define D_ALIGN_NONE 0
#define D_ALIGN_CHAOTIC (AM_CHAOTIC << 4)
#define D_ALIGN_NEUTRAL (AM_NEUTRAL << 4)
#define D_ALIGN_LAWFUL (AM_LAWFUL << 4)

#define D_ALIGN_MASK 0x70

/*
 *    Max number of prototype levels and branches.
 */
#define LEV_LIMIT 50
#define BRANCH_LIMIT 32

#endif /* DGN_FILE_H */
