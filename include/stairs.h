/* NetHack 5.0	stairs.h	$NHDT-Date: 1781973088 2026/06/20 16:31:28 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $ */
/* Copyright (c) 2024 by Pasi Kallinen */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file stairs.h
 * @brief A way between levels, and where it leads.
 *
 * Stairs are not a property of the square they stand on. A level may hold
 * several, leading to different branches, so each is a record in a list with its
 * own destination -- which is what lets the Mines and the main dungeon both
 * descend from the same level.
 *
 * @note Ladders are stairs with a flag rather than a separate kind, since they
 *       differ in description and in a few effects but not in what they do.
 */

/**
 * @file stairs.h
 * @brief 레벨 사이를 잇는 통로와, 그것이 이어지는 곳.
 *
 * 계단은 자신이 놓인 칸의 속성이 아니다. 한 레벨이 서로 다른 분기로 이어지는 여러
 * 계단을 가질 수 있으므로, 각각은 자기 목적지를 지닌 목록의 항목이다. 광산과 본
 * 던전이 같은 레벨에서 함께 내려갈 수 있는 이유가 그것이다.
 *
 * @note 사다리는 별도의 종류가 아니라 플래그가 붙은 계단이다. 설명과 몇 가지 효과가
 *       다를 뿐, 하는 일은 같기 때문이다.
 */

#ifndef STAIRS_H
#define STAIRS_H

/**
 * @brief One stairway or ladder, with its location and destination.
 * @note @c u_traversed records that the hero has used it, which the game needs
 *       for level feeling and for describing places already visited.
 * @warning @c tolev names a level, not a square. Where the traveller arrives is
 *          decided on arrival, not stored here.
 */
/**
 * @brief 계단 또는 사다리 하나. 위치와 목적지를 지닌다.
 * @note @c u_traversed 는 영웅이 그것을 이용했음을 기록한다. 레벨에 대한 감각과
 *       이미 다녀온 곳을 서술하는 데 게임이 필요로 하는 정보다.
 * @warning @c tolev 는 칸이 아니라 레벨을 가리킨다. 이동자가 정확히 어디에 도착하는지는
 *          여기에 저장되지 않고 도착 시점에 결정된다.
 */
typedef struct stairway { /* basic stairway identifier */
    coordxy sx, sy;         /* x / y location of the stair */
    d_level tolev;        /* where does it go */
    boolean up;           /* up or down? */
    boolean isladder;     /* ladder or stairway? */
    boolean u_traversed;  /* hero has traversed this stair */
    struct stairway *next;
} stairway;

#endif /* STAIRS_H */

