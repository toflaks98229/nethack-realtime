/* NetHack 5.0	align.h	$NHDT-Date: 1781973076 2026/06/20 16:31:16 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.24 $ */
/* Copyright (c) Mike Stephenson, Izchak Miller  1991.            */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file align.h
 * @brief Which side someone is on, and how well they have kept to it.
 *
 * Alignment is two separate things. Which one -- lawful, neutral, chaotic -- is
 * fixed; how faithfully the hero has behaved is a running record that rises and
 * falls with their conduct, and it is that record, not the side, which decides
 * whether a god is willing to help.
 *
 * Abuse is counted apart from the record, because some transgressions are
 * remembered permanently even after the record has been repaired.
 *
 * @note Monsters and objects carry an alignment too, which is what makes a
 *       creature's reaction and an artifact's willingness to be wielded depend
 *       on whose side the hero is on.
 * @warning The record's limit grows with elapsed game time, so the same value
 *          means less late in a game than early -- comparisons must use
 *          @c ALIGNLIM rather than a constant.
 */

/**
 * @file align.h
 * @brief 누가 어느 편인지, 그리고 그 편을 얼마나 잘 지켰는지.
 *
 * 성향은 서로 다른 두 가지다. 어느 편인지 -- 질서, 중립, 혼돈 -- 는 고정되어 있고,
 * 영웅이 얼마나 충실히 행동했는지는 처신에 따라 오르내리는 기록이다. 신이 도울 마음이
 * 있는지를 정하는 것은 편이 아니라 바로 그 기록이다.
 *
 * 남용(abuse)은 기록과 별도로 세어진다. 어떤 위반은 기록을 회복한 뒤에도 영구히
 * 기억되기 때문이다.
 *
 * @note 몬스터와 물건도 성향을 지닌다. 생물의 반응과 아티팩트가 휘둘리기를 받아들이는지가
 *       영웅이 어느 편인지에 달려 있는 이유다.
 * @warning 기록의 상한은 경과한 게임 시간과 함께 커진다. 그래서 같은 값이 게임 후반에는
 *          초반보다 덜한 의미를 가진다. 비교는 상수가 아니라 @c ALIGNLIM 을 써야 한다.
 */

#ifndef ALIGN_H
#define ALIGN_H

/** @brief Which side: see the @c A_ values below. */
/** @brief 어느 편인지. 아래의 @c A_ 값들 참고. */
typedef schar aligntyp; /* basic alignment type */

/**
 * @brief A side, together with how faithfully it has been kept.
 * @note @c record moves with conduct; @c abuse counts transgressions that are
 *       remembered regardless of later repair.
 */
/**
 * @brief 어느 편인지와, 그 편을 얼마나 충실히 지켰는지.
 * @note @c record 는 처신에 따라 움직이고, @c abuse 는 나중에 회복해도 기억되는
 *       위반의 횟수를 센다.
 */
typedef struct align { /* alignment & record */
    aligntyp type;
    int record;
    unsigned abuse;
} align;

/**
 * @brief Upper bound on the alignment record at this point in the game.
 * @note Grows with elapsed turns, so a record cannot be banked early and relied
 *       on forever; the starting value of 10 is what the bound respects.
 */
/**
 * @brief 현재 시점에서 성향 기록의 상한.
 * @note 경과 턴과 함께 커진다. 그래서 초반에 쌓아 둔 기록을 끝까지 믿을 수는 없다.
 *       이 상한이 존중하는 것이 초기값 10 이다.
 */
/* bounds for "record" -- respect initial alignments of 10 */
#define ALIGNLIM (10L + (svm.moves / 200L))

#define A_NONE (-128) /* the value range of type */

#define A_CHAOTIC (-1)
#define A_NEUTRAL 0
#define A_LAWFUL 1

#define A_COALIGNED 1
#define A_OPALIGNED (-1)

/* align masks */
#define AM_NONE         0x00
#define AM_CHAOTIC      0x01
#define AM_NEUTRAL      0x02
#define AM_LAWFUL       0x04
#define AM_MASK         0x07 /* mask for "normal" alignment values */

/* Some altars are considered shrines, add a flag for that
   for the altarmask field of struct rm. */
#define AM_SHRINE       0x08
/* High altar on Astral plane or Moloch's sanctum */
#define AM_SANCTUM      0x10

/* special level flags, gone by the time the level has been loaded */
#define AM_SPLEV_CO     0x20 /* co-aligned: force alignment to match hero's  */
#define AM_SPLEV_NONCO  0x40 /* non-co-aligned: force alignment to not match */
#define AM_SPLEV_RANDOM 0x80

#define Amask2align(x) \
    ((aligntyp) ((((x) & AM_MASK) == 0) ? A_NONE                \
                 : (((x) & AM_MASK) == AM_LAWFUL) ? A_LAWFUL    \
                   : ((int) ((x) & AM_MASK)) - 2)) /* 2 => 0, 1 => -1 */
#define Align2amask(x) \
    ((unsigned) (((x) == A_NONE) ? AM_NONE                      \
                 : ((x) == A_LAWFUL) ? AM_LAWFUL                \
                   : ((x) + 2))) /* -1 => 1, 0 => 2 */

/* Because clearly Nethack needs more ways to specify alignment...
   Amask2msa(): 1, 2, 4 converted to 1, 2, 3 to fit within a width 2 bitfield;
   Msa2amask(): 1, 2, 3 converted back to 1, 2, 4;
   For Amask2msa(), 'x' might have the shrine bit set so strip that off. */
#define Amask2msa(x) ((((x) & AM_MASK) == 4) ? 3 : (x) & AM_MASK)
#define Msa2amask(x) (((x) == 3) ? 4 : (x))
#define MSA_NONE    0  /* unaligned or multiple alignments */

/* alignment change reasons for uchangealign(attrib.c) */
enum uchangealign_reasons {
    A_CG_CONVERT  = 0, /* permanently converted */
    A_CG_HELM_ON  = 1, /* donned helm of opposite alignment */
    A_CG_HELM_OFF = 2, /* doffed helm of opposite alignment */
};

#endif /* ALIGN_H */
