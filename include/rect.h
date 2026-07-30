/* NetHack 5.0	rect.h	$NHDT-Date: 1781973086 2026/06/20 16:31:26 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.13 $ */
/* Copyright (c) 1990 by Jean-Christophe Collet                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file rect.h
 * @brief A rectangular area of the map.
 *
 * Level generation works by carving space out of what remains: the free area is
 * kept as a set of rectangles, a room is placed inside one, and the leftovers
 * become rectangles again. This is the shape that bookkeeping is done in.
 *
 * @note Bounds are inclusive on both corners, so a single square is a valid
 *       rectangle with @c lx equal to @c hx.
 */

/**
 * @file rect.h
 * @brief 지도의 직사각형 영역.
 *
 * 레벨 생성은 남은 공간을 깎아 내며 진행된다. 빈 영역을 직사각형들의 집합으로
 * 유지하고, 그중 하나 안에 방을 놓으면 남은 부분이 다시 직사각형이 된다. 그
 * 장부를 기록하는 단위가 이 형태다.
 *
 * @note 양쪽 모서리를 모두 포함한다. 그래서 한 칸도 @c lx 와 @c hx 가 같은 유효한
 *       직사각형이다.
 */

#ifndef RECT_H
#define RECT_H

/**
 * @brief A rectangle given by its low and high corners, both included.
 * @note "low" and "high" are in map order, so @c ly is the upper edge on screen.
 */
/**
 * @brief 낮은 모서리와 높은 모서리로 주어지는 직사각형. 양쪽 모두 포함된다.
 * @note "낮음"과 "높음"은 지도 좌표 순서이므로, 화면상 위쪽 변이 @c ly 다.
 */
typedef struct nhrect {
    coordxy lx, ly;
    coordxy hx, hy;
} NhRect;

#endif /* RECT_H */
