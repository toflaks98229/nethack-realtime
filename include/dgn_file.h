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

/**
 * @brief How the two ends of a branch are joined.
 * @note A branch is not symmetrical: it may be enterable from one side only,
 *       which is what the one-way values express, and a portal connects places
 *       with no staircase at either end.
 */
/**
 * @brief 분기의 양쪽 끝이 어떻게 이어지는지.
 * @note 분기는 대칭이 아니다. 한쪽에서만 들어갈 수 있을 수도 있으며 일방통행 값들이
 *       그것을 나타낸다. 포탈은 양쪽 어디에도 계단이 없는 곳들을 잇는다.
 */
/*
 *    Values for type in tmpbranch structure.
 */
#define TBR_STAIR 0   /* connection with both ends having a staircase */
#define TBR_NO_UP 1   /* connection with no up staircase */
#define TBR_NO_DOWN 2 /* connection with no down staircase */
#define TBR_PORTAL 3  /* portal connection */

/**
 * @brief Properties a dungeon or level is declared to have.
 *
 * @c TOWN marks a level containing a settlement, and is meaningful for levels
 * only. @c HELLISH makes a place part of Gehennom, which changes generation and
 * what may be created there. @c MAZELIKE and @c ROGUELIKE choose how a level is
 * built rather than merely how it looks. @c UNCONNECTED declares that a level is
 * not reachable by ordinary stairs, so the loader must not expect a path to it
 * and must not warn about its absence.
 *
 * @note These land in the dungeon's own flag bitfields, so the values are shared
 *       with that structure rather than private to the compiler.
 */
/**
 * @brief 던전이나 레벨이 지녔다고 선언되는 성질들.
 *
 * @c TOWN 은 마을이 있는 레벨을 표시하며 레벨에만 의미가 있다. @c HELLISH 는 그곳을
 * 게헨놈의 일부로 만들어 생성 방식과 거기서 만들어질 수 있는 것을 바꾼다.
 * @c MAZELIKE 와 @c ROGUELIKE 는 겉모습이 아니라 레벨이 지어지는 방식을 고른다.
 * @c UNCONNECTED 는 그 레벨에 평범한 계단으로 닿을 수 없다고 선언한다. 그래서 로더가
 * 그곳으로의 경로를 기대해서도, 경로가 없다고 경고해서도 안 된다.
 *
 * @note 이 값들은 던전 자신의 플래그 비트필드로 들어간다. 따라서 컴파일러 전용이 아니라
 *       그 구조체와 공유되는 값이다.
 */
/*
 *    Flags that map into the dungeon flags bitfields.
 */
#define TOWN        0x01 /* levels only */
#define HELLISH     0x02
#define MAZELIKE    0x04
#define ROGUELIKE   0x08
#define UNCONNECTED 0x10

/**
 * @brief A dungeon's alignment, stored in the same word as its flags.
 * @note Built by shifting the ordinary alignment masks up out of the flag bits,
 *       so one field carries both; @c D_ALIGN_MASK is how the alignment is
 *       recovered from it.
 */
/**
 * @brief 던전의 성향. 플래그와 같은 워드에 저장된다.
 * @note 평범한 성향 마스크를 플래그 비트 위로 올려 만든다. 그래서 한 필드가 둘을 함께
 *       담으며, @c D_ALIGN_MASK 가 그것에서 성향을 되찾는 수단이다.
 */
#define D_ALIGN_NONE 0
#define D_ALIGN_CHAOTIC (AM_CHAOTIC << 4)
#define D_ALIGN_NEUTRAL (AM_NEUTRAL << 4)
#define D_ALIGN_LAWFUL (AM_LAWFUL << 4)

#define D_ALIGN_MASK 0x70

/**
 * @brief How many prototype levels and branches a description may declare.
 * @warning Fixed limits on the compiler's working arrays, not on the dungeon
 *          itself; exceeding one is a failure to compile the description rather
 *          than a smaller dungeon.
 */
/**
 * @brief 기술문이 선언할 수 있는 원형 레벨과 분기의 개수.
 * @warning 던전 자체가 아니라 컴파일러 작업 배열에 대한 고정 한계다. 초과하면 던전이
 *          작아지는 것이 아니라 기술문 컴파일이 실패한다.
 */
/*
 *    Max number of prototype levels and branches.
 */
#define LEV_LIMIT 50
#define BRANCH_LIMIT 32

#endif /* DGN_FILE_H */
