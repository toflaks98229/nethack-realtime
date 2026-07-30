/* NetHack 5.0	mfndpos.h	$NHDT-Date: 1781973082 2026/06/20 16:31:22 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.19 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2005. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file mfndpos.h
 * @brief What a monster is permitted to do in order to get somewhere.
 *
 * Asking where a monster may move is not asking which squares are empty. A
 * creature that opens doors reaches places one that cannot does not; a digger
 * treats rock as passable; one that pushes boulders has routes a weaker one
 * lacks. These flags say which of those abilities to assume, so the same
 * search serves every kind of creature.
 *
 * The answer comes back as candidate squares each carrying the flags that would
 * apply on arriving there -- so the caller learns not only where it may go but
 * what going there would entail.
 *
 * @note Some flags describe reluctance rather than ability: avoiding the hero's
 *       line of sight, or disliking garlic, restrict an otherwise legal move.
 * @warning The values are specific bits within one word shared with other
 *          movement flags, which is why they are written out rather than
 *          numbered sequentially.
 */

/**
 * @file mfndpos.h
 * @brief 몬스터가 어딘가로 가기 위해 해도 되는 일.
 *
 * 몬스터가 어디로 갈 수 있는지 묻는 것은 어떤 칸이 비었는지 묻는 것이 아니다. 문을
 * 여는 생물은 열지 못하는 생물이 닿지 못하는 곳에 닿고, 파는 생물에게는 암반이
 * 통행 가능하며, 바위를 미는 생물은 약한 생물에게 없는 경로를 갖는다. 이 플래그들이
 * 그 능력 중 무엇을 가정할지 알려 주므로, 같은 탐색이 모든 종류의 생물에게 쓰인다.
 *
 * 답은 후보 칸들로 돌아오며 각 칸에는 그리로 갔을 때 적용될 플래그가 함께 담긴다.
 * 그래서 호출자는 어디로 갈 수 있는지뿐 아니라 그리로 가는 일이 무엇을 수반하는지도
 * 알게 된다.
 *
 * @note 일부 플래그는 능력이 아니라 꺼림을 나타낸다. 영웅의 시선을 피하거나 마늘을
 *       싫어하는 것은 그 밖에는 적법한 이동을 제한한다.
 * @warning 이 값들은 다른 이동 플래그와 공유하는 한 워드 안의 특정 비트다. 순차적으로
 *          번호를 매기지 않고 값을 직접 적어 둔 이유가 그것이다.
 */

#ifndef MFNDPOS_H
#define MFNDPOS_H

/* clang-format off */
#define ALLOW_MDISP 0x00001000L  /* can displace a monster out of its way */
#define ALLOW_TRAPS 0x00020000L  /* can enter traps */
#define ALLOW_U     0x00040000L  /* can attack you */
#define ALLOW_M     0x00080000L  /* can attack other monsters */
#define ALLOW_TM    0x00100000L  /* can attack tame monsters */
#define ALLOW_ALL (ALLOW_U | ALLOW_M | ALLOW_TM | ALLOW_TRAPS)
#define NOTONL      0x00200000L  /* avoids direct line to player */
#define OPENDOOR    0x00400000L  /* opens closed doors */
#define UNLOCKDOOR  0x00800000L  /* unlocks locked doors */
#define BUSTDOOR    0x01000000L  /* breaks any doors */
#define ALLOW_ROCK  0x02000000L  /* pushes rocks */
#define ALLOW_WALL  0x04000000L  /* walks through walls */
#define ALLOW_DIG   0x08000000L  /* digs */
#define ALLOW_BARS  0x10000000L  /* may pass through iron bars */
#define ALLOW_SANCT 0x20000000L  /* enters temples */
#define ALLOW_SSM   0x40000000L  /* ignores scare monster */
#ifdef NHSTDC
#define NOGARLIC    0x80000000UL /* hates garlic */
#else
#define NOGARLIC    0x80000000L  /* hates garlic */
#endif
/* clang-format on */

/**
 * @brief The squares a monster could move to, and what each would involve.
 * @note Nine entries because a creature considers its own square along with the
 *       eight around it -- staying put is a candidate move.
 * @warning @c poss and @c info are parallel arrays; the flags for a square are
 *          at the same index as the square, and only the first @c cnt entries
 *          of either are meaningful.
 */
/**
 * @brief 몬스터가 이동할 수 있는 칸들과, 각각이 수반하는 일.
 * @note 항목이 아홉 개인 것은 생물이 주변 여덟 칸과 함께 자기 칸도 고려하기 때문이다.
 *       제자리에 머무는 것도 후보 이동이다.
 * @warning @c poss 와 @c info 는 병렬 배열이다. 어떤 칸의 플래그는 그 칸과 같은
 *          색인에 있으며, 양쪽 모두 앞의 @c cnt 개 항목만 유효하다.
 */
struct mfndposdata {
    int cnt;
    coord poss[9];
    long info[9];
};

#endif /* MFNDPOS_H */
