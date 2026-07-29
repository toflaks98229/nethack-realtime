/* NetHack 5.0	nhfwd.h	$NHDT-Date: 0 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Added 2026-07 for the real-time fork (see MODIFICATIONS.md): a canonical
   forward-declaration header supporting incremental decomposition of the
   hack.h "god header".  This file is additive and changes no behavior. */

/**
 * @file nhfwd.h
 * @brief Canonical forward declarations for NetHack's core aggregate structs.
 *
 * NetHack's convenience header @c hack.h pulls in the full definitions of ~35
 * headers, so every translation unit that includes it recompiles all of them.
 * Modules that only pass these aggregates *by pointer* do not need the full
 * definitions; they can include this lightweight header instead and let the
 * linker resolve the concrete types.  This is the foundation for reducing
 * transitive include cost without touching game logic.
 *
 * @note Forward declarations only; never add field access or full definitions
 *       here.  Including this before the full definition in @c hack.h is legal
 *       and harmless.
 * @warning A pointer to an incomplete type cannot be dereferenced; a file that
 *          needs to touch fields must include the owning header for the full
 *          definition.
 */

/**
 * @file nhfwd.h
 * @brief NetHack 핵심 집합 구조체들의 표준 전방 선언 모음.
 *
 * 편의 헤더 @c hack.h 는 약 35개 헤더의 전체 정의를 끌어오므로, 이를 포함하는
 * 모든 번역 단위가 그것들을 매번 다시 컴파일한다. 이 집합체들을 *포인터로만*
 * 주고받는 모듈은 전체 정의가 필요 없으며, 대신 이 가벼운 헤더를 포함해
 * 구체 타입은 링커가 해결하게 할 수 있다. 게임 로직을 건드리지 않고 전이적
 * include 비용을 줄이기 위한 토대다.
 *
 * @note 전방 선언만 둔다. 필드 접근이나 전체 정의를 여기 추가하지 말 것.
 *       @c hack.h 의 전체 정의보다 앞서 포함해도 합법이며 무해하다.
 * @warning 불완전 타입 포인터는 역참조할 수 없다. 필드를 만져야 하는 파일은
 *          전체 정의를 위해 해당 소유 헤더를 포함해야 한다.
 */

#ifndef NHFWD_H
#define NHFWD_H

/*
 * Objects and their extensions.
 *   obj       - an object instance
 *   oextra    - optional per-object extension block
 *   obj_split - bookkeeping for splitting object stacks
 */
/*
 * 객체와 그 확장.
 *   obj       - 객체 인스턴스
 *   oextra    - 객체별 선택적 확장 블록
 *   obj_split - 객체 스택 분할 기록
 */
struct obj;
struct oextra;
struct obj_split;

/*
 * Monsters.
 *   monst    - a monster instance
 *   permonst - a monster species/template
 *   attack   - a single monster attack
 */
/*
 * 몬스터.
 *   monst    - 몬스터 인스턴스
 *   permonst - 몬스터 종/템플릿
 *   attack   - 몬스터 공격 하나
 */
struct monst;
struct permonst;
struct attack;

/*
 * Level geometry.
 *   rm     - a single map cell (row/column location)
 *   mkroom - a room (or subroom) on the current level
 *   trap   - a trap on the map
 *   engr   - an engraving on the floor
 *   region - an active area effect region
 */
/*
 * 레벨 지형.
 *   rm     - 맵 한 칸(행/열 위치)
 *   mkroom - 현재 레벨의 방(또는 서브룸)
 *   trap   - 맵의 함정
 *   engr   - 바닥의 각인
 *   region - 활성 영역 효과
 */
struct rm;
struct mkroom;
struct trap;
struct engr;
struct region;

/*
 * Miscellaneous aggregates commonly passed by pointer.
 *   damage       - pending shop/structural damage record
 *   flag         - global gameplay flags
 *   context_info - turn/action context state
 *   you          - the hero
 */
/*
 * 포인터로 자주 전달되는 기타 집합체.
 *   damage       - 미처리 상점/구조물 손상 기록
 *   flag         - 전역 게임플레이 플래그
 *   context_info - 턴/행동 컨텍스트 상태
 *   you          - 영웅
 */
struct damage;
struct flag;
struct context_info;
struct you;

#endif /* NHFWD_H */
