/* NetHack 5.0	selvar.h	$NHDT-Date: 1709677544 2024/03/05 22:25:44 $  $NHDT-Branch: keni-mdlib-followup $:$NHDT-Revision: 1.0 $ */
/* Copyright (c) 2024 by Pasi Kallinen */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file selvar.h
 * @brief The callback shape used to walk a set of map squares.
 *
 * A selection is a set of squares (see nhlsel.c), and acting on one means doing
 * something at each member. Rather than expose the set's representation, the
 * caller supplies a function and the selection calls it per square -- so the
 * bit-per-square storage stays private.
 *
 * @note Declared in a header of its own because both the selection code and its
 *       callers need the type, but neither should need the other's headers.
 */

/**
 * @file selvar.h
 * @brief 맵 칸 집합을 순회할 때 쓰는 콜백 형태.
 *
 * 선택(selection)은 칸들의 집합이며(nhlsel.c 참고), 그것에 작용한다는 것은 각
 * 원소에서 무언가를 한다는 뜻이다. 집합의 표현 방식을 드러내는 대신 호출자가 함수를
 * 제공하고 선택이 칸마다 그것을 호출한다. 그래서 칸당 1비트 저장 방식이 감춰진 채로
 * 남는다.
 *
 * @note 선택 코드와 그 호출자 양쪽이 이 타입을 필요로 하지만 서로의 헤더는 필요하지
 *       않으므로, 별도 헤더로 선언한다.
 */

#ifndef SELVAR_H
#define SELVAR_H

/**
 * @brief Called once for each square in a selection.
 * @param x    Column of the square.
 * @param y    Row of the square.
 * @param arg  Caller's context, passed through untouched.
 * @note Receives no indication of position within the set, so an iteration that
 *       needs to count or accumulate must do so through @p arg.
 */
/**
 * @brief 선택 안의 각 칸마다 한 번 호출된다.
 * @param x    그 칸의 열.
 * @param y    그 칸의 행.
 * @param arg  호출자의 문맥. 손대지 않고 그대로 전달된다.
 * @note 집합 안에서 몇 번째인지는 전달되지 않는다. 세거나 누적해야 하는 순회는
 *       @p arg 를 통해 해야 한다.
 */
typedef void (*select_iter_func)(coordxy, coordxy, genericptr);

#endif /* SELVAR_H */

