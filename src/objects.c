/* NetHack 5.0	objects.c	$NHDT-Date: 1781973059 2026/06/20 16:30:59 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.76 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file objects.c
 * @brief 게임 내 모든 오브젝트(아이템)의 정적 정의 테이블을 구성한다.
 *
 * X-매크로 기법으로 @c objects.h 를 두 번 포함하여, 오브젝트 설명
 * (@c obj_descr_init)과 오브젝트 속성(@c obj_init) 초기 테이블을 만든다.
 * 게임 시작 시 이 초기 테이블을 런타임 전역 배열로 복사한다.
 *
 * @note @c #define / @c #include / @c #undef 순서가 테이블 생성에 필수적이므로
 *       선언 재배치를 적용하지 않는다.
 */

#include "config.h"
#include "weight.h"
#include "obj.h"

#include "prop.h"
#include "skills.h"
#include "color.h"
#include "objclass.h"

/** @brief 오브젝트 설명(외형/식별 텍스트) 초기 테이블. 런타임 복사의 원본. */
static struct objdescr obj_descr_init[NUM_OBJECTS + 1] = {
#define OBJECTS_DESCR_INIT
#include "objects.h"
#undef OBJECTS_DESCR_INIT
};

/** @brief 오브젝트 클래스(속성/능력치) 초기 테이블. 런타임 복사의 원본. */
static struct objclass obj_init[NUM_OBJECTS + 1] = {
#define OBJECTS_INIT
#include "objects.h"
#undef OBJECTS_INIT
};

void objects_globals_init(void); /* in hack.h but we're using config.h */

/** @brief 런타임에 사용되는 오브젝트 설명 전역 배열. */
struct objdescr obj_descr[SIZE(obj_descr_init)];
/** @brief 런타임에 사용되는 오브젝트 클래스 전역 배열. */
struct objclass objects[SIZE(obj_init)];

/**
 * @brief 오브젝트 전역 배열을 초기 테이블 값으로 채운다.
 *
 * @c obj_descr_init / @c obj_init 의 내용을 런타임 전역 배열
 * @c obj_descr / @c objects 로 복사한다. 게임 초기화 시 호출된다.
 */
void
objects_globals_init(void)
{
    memcpy(obj_descr, obj_descr_init, sizeof(obj_descr));
    memcpy(objects, obj_init, sizeof(objects));
}

/*objects.c*/
