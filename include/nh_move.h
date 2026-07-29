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

/**
 * @brief How far the hero intends to keep going in one command.
 *
 * Walking takes one step and stops; running and rushing continue until
 * something interesting interrupts them, differing in how readily they stop.
 *
 * @note @c MV_ANY is a wildcard for lookups, not a mode a move can be in.
 */
/**
 * @brief 한 번의 명령으로 영웅이 얼마나 계속 나아갈 작정인지.
 *
 * 걷기는 한 걸음만에 멈추고, 달리기와 돌진은 흥미로운 무언가가 가로막을 때까지
 * 이어지며, 둘은 얼마나 쉽게 멈추는지가 다르다.
 *
 * @note @c MV_ANY 는 조회용 와일드카드이며, 이동이 실제로 가질 수 있는 방식이
 *       아니다.
 */
enum movemodes {
    MV_ANY = -1,
    MV_WALK,
    MV_RUN,
    MV_RUSH,

    N_MOVEMODES
};

/**
 * @brief The compass directions, ordered so that rotation is arithmetic.
 *
 * The eight horizontal directions come first and in rotational order, which is
 * what lets the @c DIR_ macros turn a direction by adding modulo @c N_DIRS.
 * Up and down follow, deliberately outside that range.
 *
 * @warning Rotating a vertical direction is meaningless: @c N_DIRS excludes
 *          @c DIR_UP and @c DIR_DOWN, so passing them to the rotation macros
 *          produces a horizontal direction unrelated to the input.
 */
/**
 * @brief 회전이 산술로 이루어지도록 정렬된 방위들.
 *
 * 수평 8방향이 회전 순서대로 먼저 온다. 그래서 @c DIR_ 매크로들이 @c N_DIRS 를
 * 법으로 더하여 방향을 돌릴 수 있다. 위와 아래는 그 뒤에, 의도적으로 그 범위
 * 밖에 놓인다.
 *
 * @warning 수직 방향을 회전시키는 것은 의미가 없다. @c N_DIRS 는 @c DIR_UP 과
 *          @c DIR_DOWN 을 제외하므로, 이들을 회전 매크로에 넘기면 입력과 무관한
 *          수평 방향이 나온다.
 */
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
/** @brief Count of horizontal directions, excluding up and down. */
/** @brief 위·아래를 제외한 수평 방향의 개수. */
#define N_DIRS (N_DIRS_Z - 2)

/**
 * @brief Turn a horizontal direction by a fixed amount.
 * @param dir A horizontal direction from @c enum movementdirs.
 * @return The rotated direction, wrapped into range.
 * @note @c DIR_180 reverses; @c DIR_LEFT / @c DIR_RIGHT turn one step;
 *       @c DIR_LEFT2 / @c DIR_RIGHT2 turn two; @c DIR_CLAMP only normalizes.
 * @warning Argument is evaluated more than once, and must be horizontal --
 *          see the warning on @c enum movementdirs.
 */
/**
 * @brief 수평 방향을 정해진 만큼 회전시킨다.
 * @param dir @c enum movementdirs 의 수평 방향.
 * @return 회전된 방향. 범위 안으로 순환된다.
 * @note @c DIR_180 은 반대 방향, @c DIR_LEFT / @c DIR_RIGHT 는 한 칸,
 *       @c DIR_LEFT2 / @c DIR_RIGHT2 는 두 칸 회전하며, @c DIR_CLAMP 는 정규화만
 *       한다.
 * @warning 인자가 여러 번 평가되며 반드시 수평 방향이어야 한다.
 *          @c enum movementdirs 의 경고를 참고할 것.
 */
/* direction adjustments */
#define DIR_180(dir) (((dir) + 4) % N_DIRS)
#define DIR_LEFT(dir) (((dir) + 7) % N_DIRS)
#define DIR_RIGHT(dir) (((dir) + 1) % N_DIRS)
#define DIR_LEFT2(dir) (((dir) + 6) % N_DIRS)
#define DIR_RIGHT2(dir) (((dir) + 2) % N_DIRS)
#define DIR_CLAMP(dir) (((dir) + N_DIRS) % N_DIRS)

/**
 * @brief Whether @c test_move() should perform a move or merely evaluate one.
 *
 * Travel and running need to know what lies ahead before committing, so the
 * same routine that performs a step is reused to ask about one. The test modes
 * differ in how far ahead they look and whether a trap counts as a refusal.
 *
 * @warning Only @c DO_MOVE has effects. The others must leave the world
 *          unchanged, since they run repeatedly while plotting a route.
 */
/**
 * @brief @c test_move() 가 이동을 실제로 수행할지, 평가만 할지.
 *
 * 여행과 달리기는 확정하기 전에 앞에 무엇이 있는지 알아야 하므로, 한 걸음을
 * 수행하는 바로 그 루틴을 질의에도 재사용한다. 시험 모드들은 얼마나 앞을 보는지,
 * 함정을 거부 사유로 볼지가 서로 다르다.
 *
 * @warning 부작용이 있는 것은 @c DO_MOVE 뿐이다. 나머지는 경로를 계산하는 동안
 *          반복 실행되므로 세계를 바꾸어서는 안 된다.
 */
/* Flags to control test_move in hack.c */
#define DO_MOVE 0   /* really doing the move */
#define TEST_MOVE 1 /* test a normal move (move there next) */
#define TEST_TRAV 2 /* test a future travel location */
#define TEST_TRAP 3 /* check if a future travel loc is a trap */

/**
 * @brief What became of a monster that was asked to move.
 *
 * A move attempt has more outcomes than success or failure: the monster may
 * have died in the attempt, exhausted its actions for the turn, or found
 * itself with nowhere legal to go.
 *
 * @warning After @c MMOVE_DIED the monster has been freed. The caller must not
 *          touch it again.
 */
/**
 * @brief 이동을 요청받은 몬스터가 어떻게 되었는지.
 *
 * 이동 시도의 결과는 성공과 실패만이 아니다. 몬스터는 그 과정에서 죽었을 수도,
 * 이번 턴의 행동력을 다 썼을 수도, 갈 수 있는 곳이 없었을 수도 있다.
 *
 * @warning @c MMOVE_DIED 이후 몬스터는 이미 해제되었다. 호출자는 다시 접근해서는
 *          안 된다.
 */
/* m_move return values */
#define MMOVE_NOTHING 0
#define MMOVE_MOVED   1 /* monster moved */
#define MMOVE_DIED    2 /* monster died */
#define MMOVE_DONE    3 /* monster used up all actions */
#define MMOVE_NOMOVES 4 /* monster has no valid locations to move to */

#endif /* NH_MOVE_H */
