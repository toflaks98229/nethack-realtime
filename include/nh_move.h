/* NetHack 5.0	nh_move.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Consolidated 2026-07 from hack.h for the real-time fork (see
   MODIFICATIONS.md).  Content is unchanged; gathered at the position of the
   earliest constituent block, so every definition is available no later than
   before. */

/**
 * @file nh_move.h
 * @brief Directions, movement styles, and the outcomes of attempting a move.
 *
 * Movement is asked about before it is done. The hero's travel and run logic
 * probes squares ahead of time -- is this passable, is it a trap -- using the
 * same routine that performs the real move, distinguished only by a test flag.
 * Monster movement reports back which of several things happened, since a
 * monster may move, die, or simply run out of actions.
 *
 * @note The compass directions are ordered so that arithmetic works on them:
 *       the @c DIR_ macros rotate a direction by adding modulo @c N_DIRS.
 * @warning @c N_DIRS excludes up and down, which sit at the end of the
 *          enumeration; rotating a vertical direction is meaningless.
 */

/**
 * @file nh_move.h
 * @brief 방향, 이동 방식, 그리고 이동 시도의 결과.
 *
 * 이동은 실행되기 전에 먼저 질의된다. 영웅의 여행·달리기 로직은 앞쪽 칸을 미리
 * 검사하며 -- 지나갈 수 있는가, 함정인가 -- 실제 이동을 수행하는 것과 같은
 * 루틴을 쓰되 시험 플래그로만 구분한다. 몬스터 이동은 여러 결과 중 무엇이
 * 일어났는지 보고한다. 몬스터는 이동할 수도, 죽을 수도, 그저 행동력을 다 쓸
 * 수도 있기 때문이다.
 *
 * @note 방위는 산술이 성립하도록 정렬되어 있다. @c DIR_ 매크로들은 @c N_DIRS
 *       를 법으로 더하여 방향을 회전시킨다.
 * @warning @c N_DIRS 는 위·아래를 제외한다. 이들은 열거의 끝에 있으며, 수직
 *          방향을 회전시키는 것은 의미가 없다.
 */

#ifndef NH_MOVE_H
#define NH_MOVE_H

enum movemodes {
    MV_ANY = -1,
    MV_WALK,
    MV_RUN,
    MV_RUSH,

    N_MOVEMODES
};

enum movementdirs {
    DIR_ERR = -1,
    DIR_W,
    DIR_NW,
    DIR_N,
    DIR_NE,
    DIR_E,
    DIR_SE,
    DIR_S,
    DIR_SW,
    DIR_DOWN,
    DIR_UP,

    N_DIRS_Z
};
/* N_DIRS_Z, minus up & down */
#define N_DIRS (N_DIRS_Z - 2)
/* direction adjustments */
#define DIR_180(dir) (((dir) + 4) % N_DIRS)
#define DIR_LEFT(dir) (((dir) + 7) % N_DIRS)
#define DIR_RIGHT(dir) (((dir) + 1) % N_DIRS)
#define DIR_LEFT2(dir) (((dir) + 6) % N_DIRS)
#define DIR_RIGHT2(dir) (((dir) + 2) % N_DIRS)
#define DIR_CLAMP(dir) (((dir) + N_DIRS) % N_DIRS)

/* Flags to control test_move in hack.c */
#define DO_MOVE 0   /* really doing the move */
#define TEST_MOVE 1 /* test a normal move (move there next) */
#define TEST_TRAV 2 /* test a future travel location */
#define TEST_TRAP 3 /* check if a future travel loc is a trap */

/* m_move return values */
#define MMOVE_NOTHING 0
#define MMOVE_MOVED   1 /* monster moved */
#define MMOVE_DIED    2 /* monster died */
#define MMOVE_DONE    3 /* monster used up all actions */
#define MMOVE_NOMOVES 4 /* monster has no valid locations to move to */

#endif /* NH_MOVE_H */
