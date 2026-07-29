/* NetHack 5.0	nhaccess.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Added 2026-07 for the real-time fork (see MODIFICATIONS.md): opt-in
   accessors for the core aggregates.  Purely additive; existing direct field
   access remains valid and is untouched. */

/**
 * @file nhaccess.h
 * @brief Opt-in, intention-revealing accessors for @c struct obj and
 *        @c struct monst.
 *
 * NetHack exposes its core aggregates in full and reads their fields directly
 * in thousands of places, so hiding those structs outright is impractical.
 * This header takes the incremental path instead: it names the *concepts* that
 * the raw field reads keep re-expressing -- "is this object on the floor?",
 * "where does this monster stand?" -- so new and revised code can state intent
 * rather than repeat structure layout knowledge.
 *
 * Adoption is voluntary and incremental: nothing here changes behavior, and no
 * existing call site is required to migrate.  Prefer these in code you touch,
 * especially where the same predicate is repeated (the object @c where tests
 * below appear over a hundred times across @c src).
 *
 * @note These are macros, matching this codebase's convention (it does not use
 *       inline functions); they are deliberately thin so they compile to the
 *       exact same code as the direct field access they replace.
 * @warning Arguments are evaluated more than once in the positional helpers
 *          (@c obj_pos, @c mon_pos, @c mon_at_pos); pass simple pointer
 *          expressions, never calls or expressions with side effects.
 * @warning Requires the full definitions from @c obj.h / @c monst.h; include
 *          after @c hack.h.  The lightweight @c nhfwd.h is not sufficient.
 */

/**
 * @file nhaccess.h
 * @brief @c struct obj 및 @c struct monst 를 위한 선택적 의미 표현 접근자.
 *
 * NetHack 은 핵심 집합 구조체를 전면 공개하고 수천 곳에서 필드를 직접 읽으므로
 * 구조체를 완전히 은닉하는 것은 비현실적이다. 이 헤더는 대신 점진적 경로를
 * 택한다. 원시 필드 읽기가 반복해서 표현하던 *개념* -- "이 객체가 바닥에
 * 있는가?", "이 몬스터는 어디에 서 있는가?" -- 에 이름을 붙여, 신규·수정
 * 코드가 구조 배치 지식을 되풀이하는 대신 의도를 드러내도록 한다.
 *
 * 채택은 자발적이고 점진적이다. 여기의 어떤 것도 동작을 바꾸지 않으며 기존
 * 호출부의 이전을 강요하지 않는다. 손대는 코드에서, 특히 동일한 술어가 반복될
 * 때 사용하기를 권한다(아래의 객체 @c where 검사는 @c src 전반에 백 번 넘게
 * 나타난다).
 *
 * @note 이 코드베이스의 관례(인라인 함수를 쓰지 않음)에 맞추어 매크로로
 *       작성했다. 대체하는 직접 필드 접근과 완전히 동일한 코드로 컴파일되도록
 *       의도적으로 얇게 유지한다.
 * @warning 위치 헬퍼(@c obj_pos, @c mon_pos, @c mon_at_pos)는 인자를 여러 번
 *          평가한다. 부작용이 있는 호출·수식이 아니라 단순 포인터 식을 넘길 것.
 * @warning @c obj.h / @c monst.h 의 전체 정의가 필요하므로 @c hack.h 뒤에
 *          포함해야 한다. 경량 @c nhfwd.h 만으로는 충분하지 않다.
 */

#ifndef NHACCESS_H
#define NHACCESS_H

/* ---------------------------------------------------------------- objects */

/**
 * @brief Report which container/inventory/map slot an object lives in.
 * @param o Object to inspect.
 * @return The object's @c where field, one of the @c OBJ_* constants.
 */
/**
 * @brief 객체가 어느 컨테이너/소지품/맵 슬롯에 속해 있는지 보고한다.
 * @param o 검사할 객체.
 * @return 객체의 @c where 필드로, @c OBJ_* 상수 중 하나.
 */
#define obj_where(o) ((o)->where)

/**
 * @brief Test whether an object lies on the dungeon floor.
 * @param o Object to inspect.
 * @retval TRUE  The object is on the floor.
 * @retval FALSE It is elsewhere.
 */
/**
 * @brief 객체가 던전 바닥에 놓여 있는지 검사한다.
 * @param o 검사할 객체.
 * @retval TRUE  객체가 바닥에 있음.
 * @retval FALSE 그 밖의 위치에 있음.
 */
#define obj_on_floor(o) ((o)->where == OBJ_FLOOR)

/**
 * @brief Test whether an object is carried by the hero.
 * @param o Object to inspect.
 * @retval TRUE  The object is in the hero's inventory.
 * @retval FALSE It is elsewhere.
 */
/**
 * @brief 객체를 영웅이 소지하고 있는지 검사한다.
 * @param o 검사할 객체.
 * @retval TRUE  영웅의 소지품에 있음.
 * @retval FALSE 그 밖의 위치에 있음.
 */
#define obj_in_invent(o) ((o)->where == OBJ_INVENT)

/**
 * @brief Test whether an object is carried by a monster.
 * @param o Object to inspect.
 * @retval TRUE  The object is in a monster's inventory.
 * @retval FALSE It is elsewhere.
 */
/**
 * @brief 객체를 몬스터가 소지하고 있는지 검사한다.
 * @param o 검사할 객체.
 * @retval TRUE  몬스터 소지품에 있음.
 * @retval FALSE 그 밖의 위치에 있음.
 */
#define obj_in_minvent(o) ((o)->where == OBJ_MINVENT)

/**
 * @brief Test whether an object sits inside a container.
 * @param o Object to inspect.
 * @retval TRUE  The object is contained.
 * @retval FALSE It is elsewhere.
 */
/**
 * @brief 객체가 용기 안에 들어 있는지 검사한다.
 * @param o 검사할 객체.
 * @retval TRUE  용기 안에 있음.
 * @retval FALSE 그 밖의 위치에 있음.
 */
#define obj_contained(o) ((o)->where == OBJ_CONTAINED)

/**
 * @brief Test whether an object is detached from every list.
 * @param o Object to inspect.
 * @retval TRUE  The object is free-floating.
 * @retval FALSE It is attached somewhere.
 * @note A free object is owned by whoever detached it and must be re-attached
 *       or deallocated; it is not reachable through any level list.
 */
/**
 * @brief 객체가 모든 목록에서 분리되어 있는지 검사한다.
 * @param o 검사할 객체.
 * @retval TRUE  어디에도 매여 있지 않음.
 * @retval FALSE 어딘가에 매여 있음.
 * @note 분리된 객체는 분리한 주체가 소유하며 재부착하거나 해제해야 한다. 어떤
 *       레벨 목록으로도 도달할 수 없다.
 */
#define obj_is_free(o) ((o)->where == OBJ_FREE)

/**
 * @brief Test whether an object rests at a given map square.
 * @param o Object to inspect.
 * @param x Column to compare against.
 * @param y Row to compare against.
 * @retval TRUE  The object's coordinates match.
 * @retval FALSE They differ.
 * @warning Meaningful only for floor-resident objects; check @c obj_on_floor()
 *          first, as @c ox / @c oy are stale for carried objects.
 */
/**
 * @brief 객체가 지정한 맵 칸에 놓여 있는지 검사한다.
 * @param o 검사할 객체.
 * @param x 비교할 열.
 * @param y 비교할 행.
 * @retval TRUE  객체 좌표가 일치함.
 * @retval FALSE 좌표가 다름.
 * @warning 바닥에 있는 객체에만 의미가 있다. 소지 중인 객체의 @c ox / @c oy 는
 *          유효하지 않으므로 먼저 @c obj_on_floor() 로 확인할 것.
 */
#define obj_pos(o, x, y) ((o)->ox == (x) && (o)->oy == (y))

/* --------------------------------------------------------------- monsters */

/**
 * @brief Test whether a monster occupies a given map square.
 * @param m Monster to inspect.
 * @param x Column to compare against.
 * @param y Row to compare against.
 * @retval TRUE  The monster stands there.
 * @retval FALSE It stands elsewhere.
 */
/**
 * @brief 몬스터가 지정한 맵 칸을 차지하고 있는지 검사한다.
 * @param m 검사할 몬스터.
 * @param x 비교할 열.
 * @param y 비교할 행.
 * @retval TRUE  몬스터가 그곳에 있음.
 * @retval FALSE 다른 곳에 있음.
 */
#define mon_at_pos(m, x, y) ((m)->mx == (x) && (m)->my == (y))

/**
 * @brief Copy a monster's map position into a coordinate pair.
 * @param m       Monster to read.
 * @param[out] cx Receives the column.
 * @param[out] cy Receives the row.
 * @note Expands to a comma expression so it can be used where a statement is
 *       expected; it assigns and does not allocate.
 */
/**
 * @brief 몬스터의 맵 위치를 좌표 쌍으로 복사한다.
 * @param m       읽어올 몬스터.
 * @param[out] cx 열을 받는다.
 * @param[out] cy 행을 받는다.
 * @note 문장이 필요한 자리에서 쓸 수 있도록 쉼표 식으로 확장된다. 대입만 할 뿐
 *       할당하지 않는다.
 */
#define mon_pos(m, cx, cy) ((cx) = (m)->mx, (cy) = (m)->my)

/**
 * @brief Report a monster's species template.
 * @param m Monster to inspect.
 * @return Pointer to the monster's @c permonst entry.
 * @note Reflects the current form: for a shapeshifted monster this is the shape
 *       it currently wears, not its original species.
 */
/**
 * @brief 몬스터의 종 템플릿을 반환한다.
 * @param m 검사할 몬스터.
 * @return 몬스터의 @c permonst 항목 포인터.
 * @note 현재 형태를 반영한다. 변신한 몬스터의 경우 원래 종이 아니라 현재 취한
 *       형태다.
 */
#define mon_species(m) ((m)->data)

/**
 * @brief Report a monster's current hit points.
 * @param m Monster to inspect.
 * @return Current hit points.
 */
/**
 * @brief 몬스터의 현재 체력을 반환한다.
 * @param m 검사할 몬스터.
 * @return 현재 체력.
 */
#define mon_hp(m) ((m)->mhp)

/**
 * @brief Report a monster's maximum hit points.
 * @param m Monster to inspect.
 * @return Maximum hit points.
 */
/**
 * @brief 몬스터의 최대 체력을 반환한다.
 * @param m 검사할 몬스터.
 * @return 최대 체력.
 */
#define mon_hpmax(m) ((m)->mhpmax)

/**
 * @brief Test whether a monster is tame.
 * @param m Monster to inspect.
 * @retval TRUE  The monster is tame (and therefore peaceful).
 * @retval FALSE It is not tame.
 */
/**
 * @brief 몬스터가 길들여졌는지 검사한다.
 * @param m 검사할 몬스터.
 * @retval TRUE  몬스터가 길들여짐(따라서 우호적).
 * @retval FALSE 길들여지지 않음.
 */
#define mon_is_tame(m) ((m)->mtame != 0)

/**
 * @brief Test whether a monster is currently invisible.
 * @param m Monster to inspect.
 * @retval TRUE  The monster is invisible.
 * @retval FALSE It is visible.
 * @note Reports the monster's own state only; whether the hero perceives it
 *       additionally depends on see-invisible and lighting.
 */
/**
 * @brief 몬스터가 현재 투명 상태인지 검사한다.
 * @param m 검사할 몬스터.
 * @retval TRUE  투명 상태임.
 * @retval FALSE 보이는 상태임.
 * @note 몬스터 자신의 상태만 보고한다. 영웅이 실제로 인지하는지는 투명 감지와
 *       조명 상태에 따라 추가로 달라진다.
 */
#define mon_is_invis(m) ((m)->minvis != 0)

/**
 * @brief Test whether a monster can see.
 * @param m Monster to inspect.
 * @retval TRUE  The monster has sight.
 * @retval FALSE It is blind, permanently or temporarily.
 */
/**
 * @brief 몬스터가 볼 수 있는지 검사한다.
 * @param m 검사할 몬스터.
 * @retval TRUE  시야가 있음.
 * @retval FALSE 영구 또는 일시적으로 실명 상태임.
 */
#define mon_can_see(m) ((m)->mcansee != 0)

#endif /* NHACCESS_H */
