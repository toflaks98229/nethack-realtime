/* NetHack 5.0	coord.h	$NHDT-Date: 1781973078 2026/06/20 16:31:18 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.14 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file coord.h
 * @brief A position on the map, as a single value.
 *
 * Exists so a location can be passed and returned as one thing rather than a
 * pair, which is why it appears throughout the interfaces that answer "where?".
 *
 * @note Declared separately from everything else because nearly every header
 *       needs it; it depends on nothing but the coordinate type itself.
 */

/**
 * @file coord.h
 * @brief 지도 위의 위치를 하나의 값으로.
 *
 * 위치를 두 개의 값이 아니라 하나로 전달하고 반환할 수 있게 하려고 존재한다.
 * "어디?"에 답하는 인터페이스 전반에 이것이 나타나는 이유다.
 *
 * @note 거의 모든 헤더가 필요로 하므로 다른 것들과 분리해 선언한다. 좌표 타입
 *       자체 외에는 아무것에도 의존하지 않는다.
 */

#ifndef COORD_H
#define COORD_H

/**
 * @brief A column and row on the level map.
 * @note Signed, since offsets and out-of-range sentinels are expressed in the
 *       same type as positions.
 */
/**
 * @brief 레벨 지도의 열과 행.
 * @note 부호가 있다. 오프셋과 범위 밖을 나타내는 특수값을 위치와 같은 타입으로
 *       표현하기 때문이다.
 */
typedef struct nhcoord {
    coordxy x, y;
} coord;

#endif /* COORD_H */
