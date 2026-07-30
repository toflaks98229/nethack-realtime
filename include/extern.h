/* NetHack 5.0	extern.h	$NHDT-Date: 1778886716 2026/05/15 15:11:56 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1558 $ */
/* Copyright (c) Steve Creps, 1988.                               */
/* NetHack may be freely redistributed.  See license for details. */
/* MODIFIED 2026-07 (real-time fork): declared nt_ticks() and
   rt_world_tick_ready() for the real-time build; see MODIFICATIONS.md.
   This file differs from the upstream NetHack distribution. */

/**
 * @file extern.h
 * @brief Every function the game shares between its files.
 *
 * Almost three thousand prototypes, and the largest header in the game. It exists because NetHack has no per-module headers: a file declares nothing for its callers, and everything
 * that is not private to one file is declared here instead.
 *
 * It is organised by the file each function lives in, marked by the section comments. That is the only organisation, and it is a useful one -- a name found here tells you where the
 * code is, which in a codebase of this size is most of what you wanted to know.
 *
 * Two things about the file are worth knowing before reading it.
 *
 * The first is the nullability annotations. Every prototype may carry a note saying which of its pointer arguments must not be null, and the long comment below sets out both what each
 * annotation means and -- more usefully -- the rule by which it was decided. Read that rule: an argument is marked non-null if the function dereferences it without checking, and left
 * unmarked if the function tests it. So an annotation is a statement about what the function actually does, and the absence of one on an analysed function is equally informative.
 *
 * The second is the conditional covering nearly the whole file. Several small tools are built from the game's sources and include this header but link against almost none of the game,
 * so the bulk of the declarations are hidden from them and the few they need are picked out individually.
 *
 * @note One annotation exists purely to record that a function was examined and found to need none. That distinction -- unexamined versus examined and clean -- is why it is not simply
 *       omitted.
 * @note Several of the annotations exist for one function each, and their comments say which and why. Those are the cases where the non-null arguments are not adjacent, and one of the
 *       comments observes that reordering that function's parameters would let its annotation be retired.
 * @note This is a modified copy of NetHack. The real-time fork's two shared functions are declared here alongside the rest; the file header above records that.
 * @warning An annotation is an assertion the compiler is allowed to act on, not a check. Marking an argument non-null when a caller may pass null turns a defensive test into
 *          undefined behaviour, and the comment below notes that such a test inside the function will itself draw a warning.
 */

/**
 * @file extern.h
 * @brief 게임이 자기 파일들 사이에서 공유하는 모든 함수.
 *
 * 프로토타입이 거의 삼천 개이며, 게임에서 가장 큰 헤더다. NetHack 에 모듈별 헤더가 없기 때문에 존재한다. 파일은 자기 호출자를 위해 아무것도 선언하지 않고, 한 파일에만 사적이지 않은 모든 것이 대신 여기에 선언된다.
 *
 * 각 함수가 사는 파일별로 조직되어 있고, 절 주석이 그것을 표시한다. 그것이 유일한 조직이며 쓸모 있는 것이다. 여기서 찾은 이름이 그 코드가 어디 있는지 알려 주고, 이만한 크기의 코드베이스에서 그것이 알고 싶었던 것의 대부분이다.
 *
 * 이 파일에 대해 읽기 전에 알아 둘 만한 두 가지가 있다.
 *
 * 첫째는 널 가능성 표시다. 모든 프로토타입이 자기 포인터 인자 중 어느 것이 널이어서는 안 되는지 말하는 메모를 지닐 수 있고, 아래의 긴 주석이 각 표시가 무엇을 뜻하는지와 -- 더 쓸모 있게 -- 그것이 어떤 규칙으로 정해졌는지를 밝힌다. 그 규칙을 읽을 것. 함수가 확인 없이
 * 역참조하는 인자는 널 아님으로 표시되고, 함수가 검사하는 인자는 표시되지 않는다. 그래서 표시는 함수가 실제로 무엇을 하는지에 관한 선언이며, 분석된 함수에 그것이 없다는 것도 똑같이 정보를 준다.
 *
 * 둘째는 파일 거의 전체를 덮는 조건문이다. 게임의 소스에서 몇 개의 작은 도구가 빌드되어 이 헤더를 포함하면서 게임의 거의 어느 것에도 링크하지 않으므로, 선언의 대부분이 그것들에게 감춰지고 그것들이 필요로 하는 몇 개만 개별적으로 골라진다.
 *
 * @note 표시 하나는 순전히 어떤 함수가 검토되었고 아무 표시도 필요하지 않다고 판단되었음을 기록하기 위해 존재한다. 검토되지 않은 것과 검토되었고 깨끗한 것의 그 구별이, 그것이 그냥 생략되지 않는 이유다.
 * @note 몇몇 표시는 각각 함수 하나를 위해 존재하고, 그 주석들이 어느 함수이고 왜인지 밝힌다. 그것들은 널 아닌 인자들이 인접하지 않은 경우이며, 그 주석 중 하나는 그 함수의 매개변수 순서를 바꾸면 그 표시를 없앨 수 있으리라고 지적한다.
 * @note 이것은 NetHack 의 수정된 사본이다. 실시간 포크의 공유 함수 둘이 나머지와 나란히 여기에 선언되어 있으며, 위의 파일 머리말이 그것을 기록한다.
 * @warning 표시는 검사가 아니라 컴파일러가 그것에 따라 행동해도 되는 주장이다. 호출자가 널을 넘길 수 있는 인자를 널 아님으로 표시하는 것은 방어적 검사를 정의되지 않은 동작으로 바꾸며, 아래의 주석은 함수 안의 그런 검사가 그 자체로 경고를 낼 것이라고 밝힌다.
 */

#ifndef EXTERN_H
#define EXTERN_H

/*
 * The placements of the NONNULLARG* and NONNULLPTRS macros were done
 * using the following rules:
 * These were the rules that were followed when determining which function
 * parameters should be nonnull, and which are nullable:
 *
 *   1. If the first use of, or reference to, the pointer parameter in the
 *      function is a dereference, then the parameter will be considered
 *      nonnull.
 *
 *   2. If there is code in the function that tests for the pointer parameter
 *      being null, and adjusts the code-path accordingly so that no segfault
 *      will occur, then the parameter will not be considered nonnull (it can
 *      be null).
 *
 * Note that if an arg is declared nonnull, any tests inside the function
 * for the variable being null, will likely trigger a compiler warning
 * diagnostic about the unnecessary test.
 *
 * Description of the NONNULL macros:
 *
 *  NONNULL         The function return value is never NULL.
 *  NONNULLPTRS     Every pointer argument is declared nonnull.
 *  NONNULLARG1     The 1st argument is declared nonnull.
 *  NONNULLARG2     The 2nd argument is declared nonnull.
 *  NONNULLARG3     The 3rd argument is declared nonnull.
 *  NONNULLARG4     The 4th argument is declared nonnull (not used).
 *  NONNULLARG5     The 5th argument is declared nonnull.
 *  NONNULLARG6     The 6th argument is declared nonnull.
 *  NONNULLARG7     The 7th argument is declared nonnull (bhit).
 *  NONNULLARG12    The 1st and 2nd arguments are declared nonnull.
 *  NONNULLARG23    The 2nd and 3rd arguments are declared nonnull.
 *  NONNULLARG13    The 1st and 3rd arguments are declared nonnull.
 *  NONNULLARG123   The 1st, 2nd and 3rd arguments are declared nonnull.
 *  NONNULLARG14    The 1st and 4th arguments are declared nonnull.
 *  NONNULLARG134   The 1st, 3rd and 4th arguments are declared nonnull.
 *  NONNULLARG17    The 1st and 7th arguments are declared nonnull (this
 *                  was a special-case added for askchain(), where the
 *                  arguments are spread out that way. This macro
 *                  could be removed if the askchain arguments in the
 *                  prototype and callers were changed to make the
 *                  nonnull arguments side-by-side).
 *  NONNULLARG145   The 1st, 4th and 5th arguments are declared nonnull
 *                  (this was a special-case added for find_roll_to_hit(),
 *                  in uhitm.c, where the arguments are spread out that way.
 *                  We can't just use NONNULLPTRS there because the 3rd
 *                  argument 'weapon' can be NULL).
 *  NONNULLARG24    The 2nd and 4th arguments are declared nonnull (this
 *                  was a special-case added for query_objlist() in invent.c).
 *  NONNULLARG45    The 4th and 5th arguments are declared nonnull (this
 *                  was a special-case added for do_screen_description(),
 *                  in pager.c, where the arguments are spread out that way.
 *                  We can't just use NONNULLPTRS there because the 6th
 *                  argument can be NULL).
 *  NO_NNARGS       This macro expands to nothing. It is just used to
 *                  mark that analysis has been done on the function,
 *                  and concluded that none of the arguments could be
 *                  marked nonnull.That distinguishes a function that has
 *                  not been analyzed (yet), from one that has.
 *
 */

#ifndef ARTIFACT_H
#include "artifact.h"
#endif

#ifndef MFNDPOS_H
#include "mfndpos.h"
#endif

/* ### alloc.c ### */

#if 0
/* routines in alloc.c depend on MONITOR_HEAP and are declared in global.h */
extern long *alloc(unsigned int) NONNULL;
#endif
extern char *fmt_ptr(const void *) NONNULL;
/* moved from hacklib.c to alloc.c so that utility programs have access */
#define FITSint(x) FITSint_(x, __func__, __LINE__)
extern int FITSint_(long long, const char *, int) NONNULLARG2;
#define FITSuint(x) FITSuint_(x, __func__, __LINE__)
extern unsigned FITSuint_(unsigned long long, const char *, int) NONNULLARG2;
/* for Strlen() which returns unsigned instead of size_t and panics for
   strings of length INT_MAX (32K - 1) or longer */

#include "hacklib.h"

/**
 * @note Everything from here to near the end of the file is hidden from the small tools built out of the game's sources. They include this header for a handful of declarations and link
 *       against almost none of the game, so declaring the rest to them would leave them unable to link. The conditional is reopened here and there to let one function through, and each
 *       of those interruptions is a tool needing that one function.
 */
/**
 * @note 여기서부터 파일 끝 부근까지의 모든 것이 게임 소스에서 빌드되는 작은 도구들에게 감춰진다. 그것들은 몇 개의 선언을 위해 이 헤더를 포함하면서 게임의 거의 어느 것에도 링크하지 않으므로, 나머지를 그것들에게 선언하면 링크할 수 없게 된다. 이 조건문은 함수 하나를 통과시키기
 *       위해 여기저기서 다시 열리며, 그 중단들 각각은 그 하나의 함수를 필요로 하는 도구다.
 */
/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(MDLIB_C) && !defined(CPPREGEX_C)

/* ### allmain.c ### */

/**
 * @brief Set up the things that must exist before anything else can run.
 * @note Takes the command line because some of what it decides comes from there -- so this runs before options are read, not after.
 */
/**
 * @brief 다른 무엇도 돌아가기 전에 존재해야 하는 것들을 세운다.
 * @note 명령줄을 받는다. 그것이 정하는 것 중 일부가 거기서 오기 때문이다. 그래서 이것은 옵션이 읽힌 뒤가 아니라 그 전에 돌아간다.
 */
extern void early_init(int, char *[]);
/**
 * @brief One pass of the main loop: let the hero act, then let everything else.
 * @note Separated from the loop itself so that a port which cannot give up control of its own event loop can drive the game one pass at a time.
 */
/**
 * @brief 주 루프의 한 번의 통과. 영웅이 행동하게 하고, 그다음 나머지 전부가 행동하게 한다.
 * @note 루프 자체와 분리되어 있어, 자기 사건 루프의 통제를 내줄 수 없는 포트가 게임을 한 번에 한 통과씩 몰 수 있다.
 */
extern void moveloop_core(void);
/**
 * @brief The main loop, which does not return until the game ends.
 * @note Its argument distinguishes a new game from a restored one, because the first pass differs -- a restored game must not repeat what already happened.
 */
/**
 * @brief 주 루프. 게임이 끝나기 전까지 반환하지 않는다.
 * @note 그 인자가 새 게임과 복원된 게임을 구별한다. 첫 통과가 다르기 때문이다. 복원된 게임은 이미 일어난 일을 되풀이해서는 안 된다.
 */
extern void moveloop(boolean);
#ifdef REALTIME_PROTO
/**
 * @brief Whether enough real time has passed for the world to advance a turn.
 *
 * This fork's clock. It is asked rather than waited on, so a caller can do something else while the answer is no -- which is the whole difference between this and the turn-based loop,
 * where the world advanced when the player acted.
 *
 * @return true if a turn is due, and consumes it -- so two calls in the same interval do not both succeed
 * @note Catches up rather than drifting: the next turn is due a fixed interval after the last one was, not after this call. A long pause therefore does not slow the game down
 *       permanently, though it is capped so a very long one does not produce a flood of turns.
 * @warning Not part of upstream NetHack. It is declared here alongside the loop it belongs to.
 */
/**
 * @brief 세계가 한 턴 진행하기에 충분한 실제 시간이 지났는지.
 *
 * 이 포크의 시계. 기다리는 것이 아니라 묻는 것이므로, 답이 아니오인 동안 호출자가 다른 일을 할 수 있다. 그것이 이것과 턴제 루프의 차이 전부다. 그쪽에서는 플레이어가 행동할 때 세계가 진행했다.
 *
 * @return 턴이 도래했으면 참. 그리고 그것을 소비한다. 그래서 같은 간격 안의 두 호출이 둘 다 성공하지는 않는다
 * @note 밀리는 대신 따라잡는다. 다음 턴은 이 호출로부터가 아니라 지난 턴이 도래했던 때로부터 고정된 간격 뒤에 도래한다. 그래서 긴 멈춤이 게임을 영구히 느리게 만들지는 않는다. 다만 아주 긴 멈춤이 턴의 홍수를 내지 않도록 상한이 있다.
 * @warning 상류 NetHack 의 일부가 아니다. 자신이 속한 루프와 나란히 여기에 선언되어 있다.
 */
extern boolean rt_world_tick_ready(void);
#endif
/**
 * @brief Abandon whatever multi-turn action the hero was performing.
 * @note What is abandoned rather than paused. An occupation that could be resumed is left alone by this; it is for the cases where continuing no longer makes sense.
 */
/**
 * @brief 영웅이 수행하던 여러 턴짜리 행동을 포기한다.
 * @note 멈추는 것이 아니라 포기하는 것이다. 이어 갈 수 있는 작업은 이것이 건드리지 않는다. 이어 가는 것이 더는 뜻이 통하지 않는 경우를 위한 것이다.
 */
extern void stop_occupation(void);
extern void init_sound_disp_gamewindows(void);
extern void newgame(void);
extern void welcome(boolean);
/**
 * @name Working with wall-clock time
 * @brief Turning the system's notion of time into plain numbers the game can store.
 * @note The system's time type is not a number the game may assume anything about, so it is converted here rather than used directly. That is what lets an elapsed time be saved and
 *       compared across platforms.
 * @{
 */
/**
 * @name 실제 시계 시간 다루기
 * @brief 시스템의 시간 개념을 게임이 저장할 수 있는 평범한 숫자로 바꾸기.
 * @note 시스템의 시간 타입은 게임이 무엇을 가정해도 되는 숫자가 아니므로, 직접 쓰이는 대신 여기서 변환된다. 그것이 경과 시간이 저장되고 플랫폼을 건너 비교될 수 있게 하는 것이다.
 * @{
 */
extern long timet_to_seconds(time_t);
extern long timet_delta(time_t, time_t);
/** @} */

/* ### apply.c ### */

extern void do_blinding_ray(struct obj *) NONNULLPTRS;
extern int doapply(void);
extern int dorub(void);
extern int dojump(void);
extern int jump(int);
extern int number_leashed(void);
extern void o_unleash(struct obj *) NONNULLPTRS;
extern void m_unleash(struct monst *, boolean) NONNULLPTRS;
extern void unleash_all(void);
/**
 * @brief Whether this monster could be put on a leash.
 * @note A question about the monster's suitability, not about whether one is available or whether it is already leashed. So a true answer is not permission to leash it.
 */
/**
 * @brief 이 몬스터를 목줄에 묶을 수 있을지.
 * @note 목줄이 있는지나 이미 묶여 있는지가 아니라 그 몬스터가 적합한지에 대한 질문이다. 그래서 참이라는 답이 그것을 묶어도 된다는 허락은 아니다.
 */
extern boolean leashable(struct monst *) NONNULLARG1;
/**
 * @brief Whether every leashed pet is still within reach of the hero.
 * @note Asks about all of them at once rather than one, since the interesting moment is when any leash has been stretched too far.
 */
/**
 * @brief 목줄에 묶인 모든 애완동물이 여전히 영웅의 손이 닿는 곳에 있는지.
 * @note 하나가 아니라 그 전부에 대해 한꺼번에 묻는다. 흥미로운 순간은 어느 목줄이든 너무 멀리 당겨졌을 때이기 때문이다.
 */
extern boolean next_to_u(void);
/**
 * @brief The leash this monster is on, if it is on one.
 * @return the leash object, or null if the monster is not leashed
 * @note Both a test and a lookup: the null answer is how code asks whether a monster is leashed at all, so there is no separate predicate.
 */
/**
 * @brief 이 몬스터가 묶여 있는 목줄. 묶여 있다면.
 * @return 그 목줄 물건. 몬스터가 묶여 있지 않으면 널
 * @note 검사이면서 조회다. 널이라는 답이 코드가 몬스터가 아예 묶여 있는지 묻는 방식이므로, 따로 판정 함수가 없다.
 */
extern struct obj *get_mleash(struct monst *) NONNULLARG1;
/**
 * @brief A word for how something looks, chosen to suit the hero's own senses.
 * @note Exists because "beautiful" is wrong for a hero who cannot see. The word returned depends on the hero's condition, so a message using it reads correctly either way.
 */
/**
 * @brief 무언가가 어떻게 보이는지에 대한 낱말. 영웅 자신의 감각에 맞게 골라진다.
 * @note 볼 수 없는 영웅에게 "아름답다"가 틀리기 때문에 존재한다. 반환되는 낱말이 영웅의 상태에 달려 있으므로, 그것을 쓰는 메시지가 어느 쪽이든 올바르게 읽힌다.
 */
extern const char *beautiful(void);
/**
 * @brief Deal with every leash after the hero has moved to the given square.
 * @note Called after the move rather than before, so it reacts to a leash that is now too long instead of preventing the move. A leash may snap here.
 */
/**
 * @brief 영웅이 주어진 칸으로 움직인 뒤 모든 목줄을 처리한다.
 * @note 움직이기 전이 아니라 뒤에 호출되므로, 그 움직임을 막는 것이 아니라 이제 너무 길어진 목줄에 반응한다. 여기서 목줄이 끊어질 수 있다.
 */
extern void check_leash(coordxy, coordxy);
/**
 * @brief Whether a square is farther from the hero than a given distance.
 * @note The sense is "beyond", not "within" -- so a true answer means too far. The distance is compared in the game's usual squared form.
 */
/**
 * @brief 어떤 칸이 영웅에게서 주어진 거리보다 먼지.
 * @note 그 뜻은 "안"이 아니라 "밖"이다. 그래서 참이라는 답은 너무 멀다는 뜻이다. 거리는 게임의 통상적인 제곱된 형태로 비교된다.
 */
extern boolean um_dist(coordxy, coordxy, xint16);
extern boolean snuff_candle(struct obj *) NONNULLPTRS;
extern boolean snuff_lit(struct obj *) NONNULLPTRS;
extern boolean splash_lit(struct obj *) NONNULLPTRS;
extern boolean catch_lit(struct obj *) NONNULLPTRS;
extern void use_unicorn_horn(struct obj **);
extern boolean tinnable(struct obj *) NONNULLPTRS;
extern void reset_trapset(void);
extern int use_whip(struct obj *) NONNULLPTRS;
extern boolean could_pole_mon(void);
extern int use_pole(struct obj *, boolean) NONNULLPTRS;
extern void maybe_dunk_boulders(coordxy, coordxy);
extern void fig_transform(union any *, long) NONNULLARG1;
extern int unfixable_trouble_count(boolean);

/* ### artifact.c ### */

extern void init_artifacts(void);
extern void save_artifacts(NHFILE *);
extern void restore_artifacts(NHFILE *);
extern const char *artiname(int);
extern struct obj *mk_artifact(struct obj *, aligntyp, uchar, boolean);
extern const char *artifact_name(const char *, short *, boolean) NONNULLARG1;
extern boolean exist_artifact(int, const char *) NONNULLPTRS;
extern void artifact_exists(struct obj *, const char *, boolean, unsigned) ;
extern void found_artifact(int);
extern void find_artifact(struct obj *) NONNULLPTRS;
extern int nartifact_exist(void);
extern void artifact_origin(struct obj *, unsigned) NONNULLPTRS;
extern boolean arti_immune(struct obj *, int);
extern boolean spec_ability(struct obj *, unsigned long);
extern boolean confers_luck(struct obj *) NONNULLPTRS;
extern boolean arti_reflects(struct obj *);
extern boolean shade_glare(struct obj *) NONNULLPTRS;
extern boolean restrict_name(struct obj *, const char *) NONNULLPTRS;
/**
 * @name What an artifact does about a kind of harm
 * @brief Whether an artifact attacks with, or defends against, a given damage type.
 * @note Three questions rather than two, and the third is the one worth knowing about: an artifact may defend only while wielded, or also while merely carried, and the two are separate
 *       because a player can benefit from one without using the weapon.
 * @note Each accepts a null object, so a caller need not check before asking -- which is why the wielded weapon can be passed directly even when there is none.
 * @{
 */
/**
 * @name 아티팩트가 어떤 종류의 피해에 대해 무엇을 하는지
 * @brief 아티팩트가 주어진 피해 종류로 공격하는지, 또는 그것을 막는지.
 * @note 둘이 아니라 세 질문이며, 세 번째가 알아 둘 만한 것이다. 아티팩트는 들고 있을 때만 막을 수도, 그저 지니고 있을 때도 막을 수도 있으며, 그 둘이 따로 있는 것은 플레이어가 그 무기를 쓰지 않고도 하나의 이득을 볼 수 있기 때문이다.
 * @note 각각이 널 물건을 받아들이므로 호출자가 묻기 전에 확인할 필요가 없다. 그래서 든 무기가 없을 때도 그것을 직접 넘길 수 있다.
 * @{
 */
extern boolean attacks(int, struct obj *);
extern boolean defends(int, struct obj *);
extern boolean defends_when_carried(int, struct obj *);
/** @} */
/**
 * @brief Whether an artifact confers protection, optionally counting one carried rather than worn.
 * @note The boolean widens the question rather than narrowing it, which is the opposite of what such an argument usually does here.
 */
/**
 * @brief 아티팩트가 방호를 주는지. 선택적으로 착용한 것이 아니라 지닌 것도 셈하여.
 * @note 그 논리값은 질문을 좁히는 것이 아니라 넓힌다. 여기서 그런 인자가 보통 하는 것과 반대다.
 */
extern boolean protects(struct obj *, boolean);
extern void set_artifact_intrinsic(struct obj *, boolean, long);
/**
 * @brief Attempt to handle an artifact, which may refuse and may hurt.
 * @return whether the handling is permitted; a refusal has already been reported
 * @warning Not a test. It has effects -- it may blast the toucher and may destroy what they were holding -- so it must not be called to find out what would happen.
 */
/**
 * @brief 아티팩트를 다루려 시도한다. 거부될 수 있고 다칠 수 있다.
 * @return 그 다룸이 허용되는지. 거부는 이미 알려져 있다
 * @warning 검사가 아니다. 효과가 있다. 만지는 자를 후려칠 수 있고 그가 들고 있던 것을 파괴할 수 있다. 그래서 무슨 일이 일어날지 알아보려고 호출해서는 안 된다.
 */
extern int touch_artifact(struct obj *, struct monst *) NONNULLARG2;
/**
 * @name An artifact's bonus against a particular target
 * @brief How much an artifact adds to accuracy, or to damage, against this monster.
 * @note Against this monster specifically -- an artifact's bonus is usually conditional on what it is fighting, which is why the target is required rather than optional.
 * @note The object may be null, since an unarmed attack still has to be asked.
 * @{
 */
/**
 * @name 특정 대상에 대한 아티팩트의 보너스
 * @brief 아티팩트가 이 몬스터에 대해 명중이나 피해에 얼마를 더하는지.
 * @note 특히 이 몬스터에 대해서다. 아티팩트의 보너스는 보통 무엇과 싸우는지에 조건적이며, 그래서 대상이 선택이 아니라 필수다.
 * @note 물건은 널일 수 있다. 맨손 공격도 여전히 물어야 하기 때문이다.
 * @{
 */
extern int spec_abon(struct obj *, struct monst *) NONNULLARG2;
extern int spec_dbon(struct obj *, struct monst *, int) NONNULLARG2;
/** @} */
extern void discover_artifact(xint16);
extern boolean undiscovered_artifact(xint16);
extern int disp_artifact_discoveries(winid);
extern void dump_artifact_info(winid);
extern boolean artifact_hit(struct monst *, struct monst *, struct obj *,
                            int *, int) NONNULLARG2;
extern int doinvoke(void);
extern boolean finesse_ahriman(struct obj *);
extern int arti_speak(struct obj *);
extern boolean artifact_light(struct obj *);
extern long spec_m2(struct obj *);
extern boolean artifact_has_invprop(struct obj *, uchar);
extern long arti_cost(struct obj *) NONNULLARG1;
/**
 * @brief Which piece of equipment is granting a property.
 * @param  a pointer to the property's own source word, not the property's number
 * @return the object responsible, or null if none is
 * @note Answers "why do I have this" for the player. The argument is unusual: it is the address of the field recording the property's sources, because the answer is found by matching
 *       what is worn against that field rather than by looking the property up.
 */
/**
 * @brief 어떤 장비가 속성을 주고 있는지.
 * @param  그 속성의 번호가 아니라 그 속성의 출처 워드에 대한 포인터
 * @return 책임이 있는 물건. 없으면 널
 * @note 플레이어를 위해 "내가 왜 이것을 지니는가"에 답한다. 그 인자가 특이하다. 속성의 출처를 기록하는 필드의 주소인데, 그 답이 속성을 찾아보는 것이 아니라 착용한 것을 그 필드와 맞춰 보아 나오기 때문이다.
 */
extern struct obj *what_gives(long *) NONNULLARG1;
extern const char *glow_color(int);
extern const char *glow_verb(int, boolean);
extern void Sting_effects(int);
/**
 * @brief Reconsider whether the hero may still be holding an object, now that something has changed.
 * @warning Takes a pointer to the pointer because the object may be dropped or destroyed -- the caller's own reference is updated, and may be left null.
 * @note Called after a change of form or alignment, when something that was safe to hold no longer is. So it is not about touching a new object but about re-examining one already held.
 */
/**
 * @brief 무언가가 바뀐 지금, 영웅이 여전히 그 물건을 들고 있어도 되는지 다시 판단한다.
 * @warning 물건이 떨어지거나 파괴될 수 있으므로 포인터에 대한 포인터를 받는다. 호출자 자신의 참조가 갱신되며, 널로 남을 수 있다.
 * @note 형태나 진영이 바뀐 뒤, 들고 있어도 안전했던 것이 더는 그렇지 않을 때 호출된다. 그래서 새 물건을 만지는 것에 관한 것이 아니라 이미 든 것을 다시 살피는 것에 관한 것이다.
 */
extern int retouch_object(struct obj **, boolean) NONNULLARG1;
/**
 * @brief The same, for everything the hero is wearing or wielding at once.
 * @note A separate routine rather than a loop over the other, because equipment has to come off in a valid order and dropping one piece may affect another.
 */
/**
 * @brief 같은 일을 영웅이 착용하거나 들고 있는 모든 것에 대해 한꺼번에 한다.
 * @note 앞의 것을 순회하는 대신 별도의 루틴인 것은, 장비가 유효한 순서로 벗겨져야 하고 한 벌을 떨어뜨리는 것이 다른 벌에 영향을 줄 수 있기 때문이다.
 */
extern void retouch_equipment(int);
/**
 * @name Recognising a magic key
 * @brief Whether a monster has one, and which object it is.
 * @note Two forms of one question, and the second is the useful one -- it both answers whether and hands back what, so the caller does not ask twice.
 * @note A magic key never fails to open a lock, so this decides an outcome rather than a chance.
 * @{
 */
/**
 * @name 마법 열쇠 알아보기
 * @brief 몬스터가 그것을 가지고 있는지, 그리고 어느 물건인지.
 * @note 하나의 질문의 두 형태이며, 두 번째가 쓸모 있는 것이다. 그것은 여부에 답하면서 무엇인지를 되돌려주므로, 호출자가 두 번 묻지 않는다.
 * @note 마법 열쇠는 자물쇠를 여는 데 결코 실패하지 않으므로, 이것은 확률이 아니라 결과를 정한다.
 * @{
 */
extern boolean is_magic_key(struct monst *, struct obj *);
extern struct obj *has_magic_key(struct monst *);
/** @} */
extern boolean is_art(struct obj *, int);
extern boolean permapoisoned(struct obj *);

/* ### attrib.c ### */

/**
 * @brief Change one of the hero's attributes, reporting whether it actually moved.
 * @return false if the attribute was already at its limit, so nothing changed
 * @note The return value is what a caller needs in order to decide what to say: an attribute that could not rise is a different message from one that did.
 */
/**
 * @brief 영웅의 능력치 하나를 바꾸고, 실제로 움직였는지 알린다.
 * @return 그 능력치가 이미 한계에 있어서 아무것도 바뀌지 않았으면 거짓
 * @note 그 반환값이 호출자가 무엇을 말할지 정하기 위해 필요한 것이다. 오를 수 없었던 능력치는 오른 능력치와 다른 메시지다.
 */
extern boolean adjattrib(int, int, int);
/**
 * @name Changing strength
 * @brief Gain or lose strength, with the loss carrying who or what caused it.
 * @note The loss takes a killer and its format because losing strength can be fatal, and the cause has to be recorded at the moment it happens rather than when the hero dies of it.
 * @note The gain takes the object responsible instead, because gaining is never fatal and what matters is the message.
 * @{
 */
/**
 * @name 힘 바꾸기
 * @brief 힘을 얻거나 잃는다. 잃는 쪽은 누가 또는 무엇이 그렇게 했는지를 지닌다.
 * @note 잃는 쪽이 살해자와 그 형식을 받는 것은 힘을 잃는 것이 치명적일 수 있기 때문이다. 그 원인은 영웅이 그것으로 죽을 때가 아니라 그 일이 일어나는 순간에 기록되어야 한다.
 * @note 얻는 쪽은 대신 책임이 있는 물건을 받는다. 얻는 것은 결코 치명적이지 않고 중요한 것은 메시지이기 때문이다.
 * @{
 */
extern void gainstr(struct obj *, int, boolean);
extern void losestr(int, const char *, schar);
/** @} */
extern void poison_strdmg(int, int, const char *, schar);
extern void poisontell(int, boolean);
extern void poisoned(const char *, int, const char *, int, boolean) NONNULLARG1;
extern void change_luck(schar);
/**
 * @brief The luck contributed by carried luckstones.
 * @note Separate from the hero's earned luck, which is what makes the total exceedable and the two worth keeping apart. Its argument asks whether to count a stone whose blessing is
 *       unknown.
 */
/**
 * @brief 지니고 있는 행운석이 기여하는 운.
 * @note 영웅이 쌓은 운과 별개이며, 그것이 총합을 한계 넘게 만들 수 있게 하고 그 둘을 떼어 놓을 가치가 있게 하는 것이다. 그 인자는 축복 여부를 모르는 돌을 셈할지를 묻는다.
 */
extern int stone_luck(boolean);
/**
 * @brief Recompute the carried luck bonus after the pack has changed.
 * @note A cache refresh. Nothing recomputes it as luck is read, so failing to call this after picking up or dropping a luckstone leaves the bonus wrong until something else does.
 */
/**
 * @brief 가방이 바뀐 뒤 지닌 운 보너스를 다시 계산한다.
 * @note 캐시 갱신이다. 운을 읽을 때 그것을 다시 계산하는 것이 없으므로, 행운석을 집거나 버린 뒤 이것을 호출하지 않으면 다른 무엇이 그렇게 하기까지 그 보너스가 틀린 채로 남는다.
 */
extern void set_moreluck(void);
/**
 * @brief Move attributes back toward their true values after temporary changes expire.
 * @note Called once per turn. It is what makes a temporary attribute change temporary, so an effect that alters an attribute does not need to schedule its own reversal.
 */
/**
 * @brief 일시적 변화가 끝난 뒤 능력치를 참된 값 쪽으로 되돌린다.
 * @note 턴마다 한 번 호출된다. 일시적인 능력치 변화를 일시적으로 만드는 것이며, 그래서 능력치를 바꾸는 효과가 자기 되돌림을 따로 예약할 필요가 없다.
 */
extern void restore_attrib(void);
/**
 * @name Exercising an attribute
 * @brief Note that the hero did something that should train an attribute, and later act on it.
 *
 * Exercise is not an immediate change. Doing something strenuous records a lean toward higher strength, and the accumulated lean is applied occasionally -- which is why these are two
 * routines and why the first has no visible effect.
 *
 * @note That is the point of the design: an attribute rises because of how the hero has been played rather than because of one action, and no single action can be identified as the one
 *       that raised it.
 * @{
 */
/**
 * @name 능력치 단련
 * @brief 영웅이 능력치를 단련할 만한 일을 했음을 기록하고, 나중에 그것에 따라 행동한다.
 *
 * 단련은 즉각적인 변화가 아니다. 힘든 일을 하는 것은 더 높은 힘 쪽으로의 기울기를 기록하고, 쌓인 기울기가 이따금 적용된다. 그것이 이들이 두 루틴인 이유이고 첫 번째가 눈에 보이는 효과가 없는 이유다.
 *
 * @note 그것이 이 설계의 요점이다. 능력치는 하나의 행동 때문이 아니라 영웅이 어떻게 플레이되어 왔는지 때문에 오르며, 어느 한 행동도 그것을 올린 것으로 지목될 수 없다.
 * @{
 */
extern void exercise(int, boolean);
extern void exerchk(void);
/** @} */
extern void init_attr(int);
extern void redist_attr(void);
extern void vary_init_attr(void);
extern void adjabil(int, int);
extern int newhp(void);
extern int minuhpmax(int);
extern void setuhpmax(int, boolean);
extern int adjuhploss(int, int);
/**
 * @brief The value of an attribute as the rules see it, after every influence.
 * @note The one to use. An attribute is not stored as a single number but assembled from a base, a maximum, a bonus and a temporary change -- this is what performs that assembly, so
 *       reading any of those fields directly gives a partial answer.
 */
/**
 * @brief 모든 영향이 반영된 뒤, 규칙이 보는 능력치의 값.
 * @note 써야 할 것이다. 능력치는 하나의 숫자로 저장되지 않고 기준값, 최대값, 보너스, 일시적 변화로 조립된다. 이것이 그 조립을 수행하는 것이므로, 그 필드 중 어느 것을 직접 읽으면 부분적인 답이 나온다.
 */
extern schar acurr(int);
/**
 * @brief The same for strength, which needs its own routine.
 * @warning Strength does not use a plain numeric scale in the range where it is exceptional, so it cannot be read through the general routine. Doing so yields a number that is on the
 *          wrong scale rather than merely wrong.
 */
/**
 * @brief 힘에 대한 같은 것. 힘은 자기 루틴을 필요로 한다.
 * @warning 힘은 뛰어난 범위에서 평범한 숫자 척도를 쓰지 않으므로 일반 루틴으로 읽을 수 없다. 그렇게 하면 단지 틀린 것이 아니라 잘못된 척도의 숫자가 나온다.
 */
extern schar acurrstr(void);
/**
 * @brief Whether an attribute is at either of its limits.
 * @note Either end, not just the top -- so it answers "cannot move further" in whichever direction, which is what a caller deciding whether to bother usually wants.
 */
/**
 * @brief 능력치가 그 두 한계 중 어느 쪽에 있는지.
 * @note 위쪽만이 아니라 양쪽이다. 그래서 어느 방향이든 "더 움직일 수 없음"에 답하며, 그것이 애쓸 만한지 정하려는 호출자가 보통 원하는 것이다.
 */
extern boolean extremeattr(int);
/**
 * @brief Move the hero's standing with their god.
 * @note Not the same as changing alignment: the hero's alignment is what they are, and this is how well they are living up to it. A single act cannot change the former.
 */
/**
 * @brief 영웅이 자기 신 앞에서 서 있는 자리를 움직인다.
 * @note 진영을 바꾸는 것과 같지 않다. 영웅의 진영은 그가 무엇인지이고, 이것은 그가 그것에 얼마나 부합하게 살고 있는지다. 하나의 행위가 앞의 것을 바꿀 수는 없다.
 */
extern void adjalign(int);
/**
 * @brief Whether a property is the hero's own rather than granted by something.
 * @return which kind of source it is, not a plain yes or no
 * @note More than a test: the several ways a property can be the hero's own -- from their role, their race, their experience -- are distinguished, because the answer is shown to the
 *       player and each reads differently.
 */
/**
 * @brief 어떤 속성이 무엇에 의해 주어진 것이 아니라 영웅 자신의 것인지.
 * @return 평범한 예/아니오가 아니라 어떤 종류의 출처인지
 * @note 검사 이상이다. 속성이 영웅 자신의 것일 수 있는 여러 방식 -- 직업에서, 종족에서, 경험에서 -- 이 구별된다. 그 답이 플레이어에게 보여지고 각각이 다르게 읽히기 때문이다.
 */
extern int is_innate(int);
/**
 * @brief A phrase naming where a property comes from, for showing to the player.
 * @return the phrase, which may be empty when the source is not worth naming
 * @warning Returns a pointer into a shared buffer. A second call replaces the first answer, so two sources cannot be described in one sentence without copying.
 */
/**
 * @brief 속성이 어디서 오는지 지칭하는 구절. 플레이어에게 보이기 위한 것.
 * @return 그 구절. 출처가 지칭할 가치가 없을 때는 빈 것일 수 있다
 * @warning 공유 버퍼를 가리키는 포인터를 반환한다. 두 번째 호출이 첫 답을 대체하므로, 복사하지 않고 두 출처를 한 문장에 기술할 수 없다.
 */
extern char *from_what(int);
extern void uchangealign(int, int);

/* ### ball.c ### */

extern void ballrelease(boolean);
extern void ballfall(void);
#ifndef BREADCRUMBS
extern void placebc(void);
extern void unplacebc(void);
extern int unplacebc_and_covet_placebc(void);
extern void lift_covet_and_placebc(int);
#else
#define placebc() Placebc(__FUNCTION__, __LINE__)
#define unplacebc() Unplacebc(__FUNCTION__, __LINE__)
#define unplacebc_and_covet_placebc() \
            Unplacebc_and_covet_placebc(__FUNCTION__, __LINE__)
#define lift_covet_and_placebc(x) \
            Lift_covet_and_placebc(x, __FUNCTION__, __LINE__)
#endif
extern void set_bc(int);
extern void move_bc(int, int, coordxy, coordxy, coordxy, coordxy);
extern boolean drag_ball(coordxy, coordxy, int *, coordxy *, coordxy *,
                         coordxy *, coordxy *, boolean *, boolean) NONNULLPTRS;
extern void drop_ball(coordxy, coordxy);
extern void drag_down(void);
extern void bc_sanity_check(void);

/* ### bones.c ### */

/**
 * @brief Make a name safe to write into a shared file.
 * @warning Alters the string in place. It exists because a bones file may be read by another player's game, so a name that could confuse the file's format has to be neutralised before
 *          it is written -- this is a safety measure, not a formatting one.
 */
/**
 * @brief 이름을 공유 파일에 기록해도 안전하게 만든다.
 * @warning 문자열을 제자리에서 바꾼다. 유골 파일이 다른 플레이어의 게임에 읽힐 수 있으므로, 그 파일의 형식을 혼란시킬 수 있는 이름은 기록되기 전에 무해하게 되어야 한다. 이것은 서식이 아니라 안전 조치다.
 */
extern void sanitize_name(char *) NONNULLARG1;
/**
 * @brief Whether this level may be saved as bones at all.
 * @note Several things forbid it -- the level being special, the death being of a kind that leaves nothing, the option being off -- and asking first avoids doing the work of preparing
 *       a bones file that will be discarded.
 */
/**
 * @brief 이 레벨이 아예 유골로 저장되어도 되는지.
 * @note 여러 가지가 그것을 금한다. 그 레벨이 특수하다는 것, 죽음이 아무것도 남기지 않는 종류라는 것, 그 선택지가 꺼져 있다는 것. 먼저 물으면 버려질 유골 파일을 준비하는 일을 피할 수 있다.
 */
extern boolean can_make_bones(void);
/**
 * @brief Whether a bones file's name matches one the current game should refuse.
 * @note Used to keep a player from meeting their own remains, which would let a game feed itself equipment. So this is a fairness check rather than a validity one.
 */
/**
 * @brief 유골 파일의 이름이 현재 게임이 거부해야 할 것과 일치하는지.
 * @note 플레이어가 자기 자신의 잔해를 만나지 않게 하기 위해 쓰인다. 그러면 게임이 스스로에게 장비를 먹일 수 있게 된다. 그래서 이것은 유효성 검사가 아니라 공평성 검사다.
 */
extern boolean bones_include_name(const char *) NONNULLARG1;
/**
 * @brief Repair an object that has come from another game's bones file.
 * @note An object from a bones file may refer to things that game had and this one does not -- a fruit name, a shuffled appearance. This reconciles it with the current game, which is
 *       why an object arriving from bones cannot simply be used as it was read.
 */
/**
 * @brief 다른 게임의 유골 파일에서 온 물건을 고친다.
 * @note 유골 파일에서 온 물건은 그 게임이 가졌고 이 게임은 갖지 않은 것들을 가리킬 수 있다. 과일 이름, 섞인 외형. 이것이 그것을 현재 게임과 조화시키며, 그래서 유골에서 도착한 물건을 읽은 그대로 쓸 수는 없다.
 */
extern void fix_ghostly_obj(struct obj *) NONNULLARG1;
extern void newebones(struct monst *) NONNULLARG1;
extern void free_ebones(struct monst *) NONNULLARG1;

/* ### botl.c ### */

extern char *get_strength_str(void);
extern char *do_statusline1(void);
extern void check_gold_symbol(void);
extern char *do_statusline2(void);
extern void bot(void);
extern void timebot(void);
/**
 * @name Experience level and rank title
 * @brief Converting between how experienced the hero is and what they are called.
 * @note Not a one-to-one mapping: a rank covers a span of levels, so converting a level to a rank and back does not return the original level. It returns the lowest level of that rank.
 * @{
 */
/**
 * @name 경험 레벨과 계급 칭호
 * @brief 영웅이 얼마나 숙련되었는지와 그가 무엇이라 불리는지 사이의 변환.
 * @note 일대일 대응이 아니다. 계급이 레벨의 구간을 덮으므로, 레벨을 계급으로 바꾸고 다시 되돌려도 원래 레벨이 나오지 않는다. 그 계급의 가장 낮은 레벨이 나온다.
 * @{
 */
extern int xlev_to_rank(int);
extern int rank_to_xlev(int);
/** @} */
/**
 * @brief The title for a rank, in the right role and gender.
 * @note Takes a role rather than assuming the hero's, so a title can be produced for a player-monster or for another hero's remains.
 */
/**
 * @brief 어떤 계급의 칭호. 알맞은 직업과 성별로.
 * @note 영웅의 것을 가정하는 대신 직업을 받는다. 그래서 플레이어 몬스터나 다른 영웅의 잔해에 대한 칭호를 만들 수 있다.
 */
extern const char *rank_of(int, short, boolean);
/**
 * @brief Work out which monster a rank title belongs to, and at what level.
 * @return the monster, with the level and gender written back through the pointers
 * @note The reverse of naming a rank, and it exists so that a title written in a file -- a bones file, a log -- can be turned back into who that was.
 */
/**
 * @brief 계급 칭호가 어느 몬스터에게 속하는지, 그리고 어느 레벨인지 알아낸다.
 * @return 그 몬스터. 레벨과 성별은 포인터를 통해 되기록된다
 * @note 계급에 이름을 붙이는 것의 역이며, 파일 -- 유골 파일, 로그 -- 에 적힌 칭호를 그것이 누구였는지로 되돌릴 수 있도록 존재한다.
 */
extern int title_to_mon(const char *, int *, int *);
/**
 * @brief Work out how wide the widest rank title is.
 * @note Called once, and what it produces is a layout figure: the status line reserves that much room so it does not shift as the hero is promoted.
 */
/**
 * @brief 가장 긴 계급 칭호가 얼마나 넓은지 알아낸다.
 * @note 한 번 호출되며, 그것이 내는 것은 배치용 수치다. 상태줄이 그만큼의 자리를 예약하므로 영웅이 승급할 때 그것이 밀리지 않는다.
 */
extern void max_rank_sz(void);
#ifdef SCORE_ON_BOTL
extern long botl_score(void);
#endif
/**
 * @brief Write a description of where the hero is into a caller's buffer.
 * @return how the description was formed, since a level may be named, numbered, or both
 * @note The integer argument selects how much detail, because the same description serves the status line, where space is scarce, and the overview, where it is not.
 */
/**
 * @brief 영웅이 어디 있는지에 대한 기술을 호출자의 버퍼에 쓴다.
 * @return 그 기술이 어떻게 만들어졌는지. 레벨은 이름으로도, 번호로도, 둘 다로도 지칭될 수 있다
 * @note 정수 인자가 얼마나 자세할지를 고른다. 같은 기술이 자리가 부족한 상태줄과 그렇지 않은 개요를 함께 맡기 때문이다.
 */
extern int describe_level(char *, int);
/**
 * @name Equipment summaries for the status line
 * @brief Describe what the hero is wielding or wearing, briefly.
 * @note Each writes into the caller's buffer and returns it, which is what lets one be used directly inside a larger piece of formatting.
 * @warning The buffer must be large enough; neither is told its size. That is why both mark their argument as required rather than optional.
 * @{
 */
/**
 * @name 상태줄을 위한 장비 요약
 * @brief 영웅이 무엇을 들고 있거나 입고 있는지 간략히 기술한다.
 * @note 각각이 호출자의 버퍼에 쓰고 그것을 반환한다. 그것이 하나를 더 큰 서식 조각 안에서 직접 쓸 수 있게 하는 것이다.
 * @warning 버퍼가 충분히 커야 한다. 어느 쪽도 자기 크기를 듣지 않는다. 둘 다 자기 인자를 선택이 아니라 필수로 표시하는 이유가 그것이다.
 * @{
 */
extern char *weapon_status(char *) NONNULL NONNULLARG1;
extern char *armor_status(char *) NONNULL NONNULLARG1;
/** @} */
extern void status_initialize(boolean);
extern void status_finish(void);
/**
 * @brief Whether the experience percentage shown would differ from last time.
 * @note Exists so the status line can be left alone when nothing visible changed. The underlying number changes almost every turn while the displayed percentage does not, and
 *       redrawing on the former would make the status line flicker constantly.
 */
/**
 * @brief 보여지는 경험치 백분율이 지난번과 다를지.
 * @note 눈에 보이는 것이 바뀌지 않았을 때 상태줄을 건드리지 않기 위해 존재한다. 바탕 숫자는 거의 매 턴 바뀌지만 표시되는 백분율은 그렇지 않으며, 앞의 것에 따라 다시 그리면 상태줄이 끊임없이 깜박이게 된다.
 */
extern boolean exp_percent_changing(void);
/**
 * @name Which condition to show
 * @brief Turn a state into the index of the word describing it.
 * @note An index rather than the word, because the status line needs to compare what it is showing against what it showed -- and comparing indices is how it decides whether to redraw
 *       without formatting the text first.
 * @{
 */
/**
 * @name 어떤 상태를 보일지
 * @brief 어떤 상태를 그것을 기술하는 낱말의 색인으로 바꾼다.
 * @note 낱말이 아니라 색인인 것은, 상태줄이 자신이 보이고 있는 것과 보였던 것을 비교해야 하기 때문이다. 색인을 비교하는 것이 글을 먼저 서식화하지 않고 다시 그릴지 정하는 방식이다.
 * @{
 */
extern int stat_cap_indx(void);
extern int stat_hunger_indx(void);
/** @} */
/**
 * @brief The name of a status field, for showing to the player.
 * @note Keyed by the field's index, so the same numbering serves the display, the player's highlight rules and the option that enables a field.
 */
/**
 * @brief 상태 필드의 이름. 플레이어에게 보이기 위한 것.
 * @note 그 필드의 색인으로 접근된다. 그래서 같은 번호 체계가 표시부, 플레이어의 강조 규칙, 그리고 필드를 켜는 선택지를 함께 맡는다.
 */
extern const char *bl_idx_to_fldname(int);
/**
 * @brief Fill the unused part of a status field with dashes.
 * @note So a field that has shrunk does not leave the previous, longer value's tail behind it. A terminal display does not clear what it does not write over, which is why the padding
 *       has to be written rather than the field simply being shorter.
 */
/**
 * @brief 상태 필드의 쓰이지 않는 부분을 붙임표로 채운다.
 * @note 그래서 줄어든 필드가 이전의 더 긴 값의 꼬리를 뒤에 남기지 않는다. 터미널 표시부는 자신이 덮어 쓰지 않은 것을 지우지 않으며, 그래서 필드가 그냥 짧아지는 대신 채움이 기록되어야 한다.
 */
extern void repad_with_dashes(char *);
extern void condopt(int, boolean *, boolean);
extern int parse_cond_option(boolean, char *);
extern boolean cond_menu(void);
extern boolean opt_next_cond(int, char *);
#ifdef STATUS_HILITES
extern void status_eval_next_unhilite(void);
extern void reset_status_hilites(void);
extern boolean parse_status_hl1(char *op, boolean);
extern void status_notify_windowport(boolean);
extern void clear_status_hilites(void);
extern int count_status_hilites(void);
extern void all_options_statushilites(strbuf_t *);
extern boolean status_hilite_menu(void);
#endif /* STATUS_HILITES */

/* ### calendar.c ### */

extern time_t getnow(void);
extern int getyear(void);
#if 0
extern char *yymmdd(time_t) NONNULL;
#endif
extern long yyyymmdd(time_t);
extern long hhmmss(time_t);
extern char *yyyymmddhhmmss(time_t) NONNULL;
extern time_t time_from_yyyymmddhhmmss(char *);
/**
 * @name What day and hour it is in the real world
 * @brief The outside world's calendar, which the game consults for a few of its rules.
 *
 * A full moon changes the hero's luck and how some monsters behave; an unlucky date changes it the other way; night and midnight matter to a few effects. So these are game rules that
 * happen to be decided by the player's own clock.
 *
 * @note The phase is recorded at the start of a game rather than asked each turn, so a session spanning midnight does not change the hero's luck underneath them. These are what that
 *       recording reads.
 * @warning Reading the real clock during play would be a different thing from what the game does. That the values exist here does not mean they are consulted continuously.
 * @{
 */
/**
 * @name 실제 세계에서 어느 날 몇 시인지
 * @brief 바깥 세계의 달력. 게임이 몇몇 규칙에 대해 그것을 참조한다.
 *
 * 보름달은 영웅의 운과 몇몇 몬스터의 행동을 바꾸고, 불운한 날짜는 그것을 반대로 바꾸며, 밤과 자정이 몇 가지 효과에 중요하다. 그래서 이들은 마침 플레이어 자신의 시계로 결정되는 게임 규칙이다.
 *
 * @note 달의 위상은 매 턴 물어지는 것이 아니라 게임 시작에 기록된다. 그래서 자정을 넘기는 세션이 영웅 아래에서 그의 운을 바꾸지 않는다. 이들은 그 기록이 읽는 것이다.
 * @warning 플레이 중에 실제 시계를 읽는 것은 게임이 하는 것과 다른 일이다. 이 값들이 여기 존재한다는 것이 그것들이 끊임없이 참조된다는 뜻은 아니다.
 * @{
 */
extern int phase_of_the_moon(void);
extern boolean friday_13th(void);
extern int night(void);
extern int midnight(void);
/** @} */

/* ### cfgfiles.c ### */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
extern int l_get_config_errors(lua_State *) NONNULLARG1;
#endif
extern int do_write_config_file(void);
extern boolean parse_config_line(char *) NONNULLARG1;
#ifdef USER_SOUNDS
extern boolean can_read_file(const char *) NONNULLARG1;
#endif
extern void config_error_init(boolean, const char *, boolean);
extern void config_erradd(const char *);
extern int config_error_done(void);
/* arg1 of read_config_file can be NULL to pass through
 * to fopen_config_file() to mean 'use the default config file name' */
extern boolean read_config_file(const char *, int);
extern boolean parse_conf_str(const char *str, boolean (*proc)(char *));
extern boolean parse_conf_file(FILE *fp, boolean (*proc)(char *arg));
extern void set_configfile_name(const char *);
extern char *get_configfile(void);
extern const char *get_default_configfile(void);
extern void rcfile(void);
extern void rcfile_interface_options(void);
extern void rcfile_only_this_option(enum opt);
extern void heed_all_config_statements(void);
extern void disregard_all_config_statements(void);
extern void heed_this_config_statement(int);
extern void disregard_this_config_statement(int);
extern boolean config_unmatched_ignored(void);
extern void clear_ignore_errors_on_unmatched(void);
extern void set_ignore_errors_on_unmatched(void);
extern void rcfile_only_this_statement(int);
#ifdef WIN32
extern boolean check_for_portable_config(void);
#endif
#ifdef MSWIN_GRAPHICS
extern void disregard_some_mswin_options(void);
extern void rcfile_only_some_mswin_options(void);
#endif

/* ### coloratt.c ### */

extern char *color_attr_to_str(color_attr *);
extern boolean color_attr_parse_str(color_attr *, char *);
extern int32 colortable_to_int32(const struct nethack_color *);
extern int query_color(const char *, int) NO_NNARGS;
extern int query_attr(const char *, int) NO_NNARGS;
extern boolean query_color_attr(color_attr *, const char *) NONNULLARG1;
extern const char *attr2attrname(int);
extern void basic_menu_colors(boolean);
extern boolean add_menu_coloring_parsed(const char *, int, int);
extern const char *clr2colorname(int);
extern int match_str2clr(char *, boolean) NONNULLARG1;
extern int match_str2attr(const char *, boolean) NONNULLARG1;
extern boolean add_menu_coloring(char *) NONNULLARG1;
extern void free_one_menu_coloring(int);
extern void free_menu_coloring(void);
extern int count_menucolors(void);
extern int32 check_enhanced_colors(char *) NONNULLARG1;
extern const char *wc_color_name(int32) NONNULL;
extern int32_t rgbstr_to_int32(const char *rgbstr);
extern boolean closest_color(uint32_t lcolor, uint32_t *closecolor, uint16 *clridx);
extern int color_distance(uint32_t, uint32_t);
extern boolean onlyhexdigits(const char *buf);
extern uint32 get_nhcolor_from_256_index(int idx);
#ifdef CHANGE_COLOR
extern int count_alt_palette(void);
extern int alternative_palette(char *);
extern void change_palette(void);
#endif

/* ### cmd.c ### */

extern void cmdbind_freeall(void);
extern int dotoggleoption(void);
extern void set_move_cmd(int, int);
extern int do_move_west(void);
extern int do_move_northwest(void);
extern int do_move_north(void);
extern int do_move_northeast(void);
extern int do_move_east(void);
extern int do_move_southeast(void);
extern int do_move_south(void);
extern int do_move_southwest(void);
extern int do_rush_west(void);
extern int do_rush_northwest(void);
extern int do_rush_north(void);
extern int do_rush_northeast(void);
extern int do_rush_east(void);
extern int do_rush_southeast(void);
extern int do_rush_south(void);
extern int do_rush_southwest(void);
extern int do_run_west(void);
extern int do_run_northwest(void);
extern int do_run_north(void);
extern int do_run_northeast(void);
extern int do_run_east(void);
extern int do_run_southeast(void);
extern int do_run_south(void);
extern int do_run_southwest(void);
extern int do_reqmenu(void);
extern int do_rush(void);
extern int do_run(void);
extern int do_fight(void);
extern int do_repeat(void);
extern char randomkey(void);
extern void random_response(char *, int);
extern int rnd_extcmd_idx(void);
extern int domonability(void);
extern const struct ext_func_tab *ext_func_tab_from_func(int(*)(void));
extern char cmd_from_func(int(*)(void));
extern char cmd_from_dir(int, int);
extern char *cmd_from_ecname(const char *);
extern const char *cmdname_from_func(int(*)(void), char *, boolean);
extern boolean redraw_cmd(char);
extern const char *levltyp_to_name(int);
extern int dolookaround(void);
extern void reset_occupations(void);
extern void set_occupation(int(*)(void), const char *, cmdcount_nht);
extern void cmdq_add_ec(int, int(*)(void));
extern void cmdq_add_key(int, char);
extern void cmdq_add_dir(int, schar, schar, schar);
extern void cmdq_add_userinput(int);
extern void cmdq_add_int(int, int);
extern void cmdq_shift(int);
extern struct _cmd_queue *cmdq_reverse(struct _cmd_queue *);
extern struct _cmd_queue *cmdq_copy(int);
extern struct _cmd_queue *cmdq_pop(void);
extern struct _cmd_queue *cmdq_peek(int);
extern void cmdq_clear(int);
extern char pgetchar(void);
extern char extcmd_initiator(void);
extern int doextcmd(void);
extern struct ext_func_tab *extcmds_getentry(int);
extern int count_bind_keys(void);
extern int count_autocompletions(void);
extern void get_changed_key_binds(strbuf_t *);
extern void handler_rebind_keys(void);
extern void handler_change_autocompletions(void);
extern int extcmds_match(const char *, int, int **);
extern const char *key2extcmddesc(uchar);
extern boolean bind_specialkey(uchar, const char *);
extern void parseautocomplete(char *, boolean);
extern void all_options_autocomplete(strbuf_t *);
extern void lock_mouse_buttons(boolean);
extern void reset_commands(boolean);
extern void update_rest_on_space(void);
extern void rhack(int);
extern int doextlist(void);
extern int extcmd_via_menu(void);
extern int enter_explore_mode(void);
extern boolean bind_mousebtn(int, const char *);
extern boolean bind_key(uchar, const char *, boolean);
extern void dokeylist(void);
extern int xytodir(int, int);
extern void dirtocoord(coord *, int);
extern int movecmd(char, int);
extern int dxdy_moveok(void);
extern int getdir(const char *);
extern void confdir(boolean);
extern const char *directionname(int);
extern int isok(coordxy, coordxy);
extern int get_adjacent_loc(const char *, const char *, coordxy, coordxy,
                            coord *);
extern void click_to_cmd(coordxy, coordxy, int);
extern char get_count(const char *, char, long, cmdcount_nht *, unsigned);
#ifdef HANGUPHANDLING
extern void hangup(int);
extern void end_of_input(void);
#endif
extern char readchar(void);
extern char readchar_poskey(coordxy *, coordxy *, int *);
extern char* key2txt(uchar, char *);
extern char yn_function(const char *, const char *, char, boolean);
extern char paranoid_ynq(boolean, const char *, boolean);
extern boolean paranoid_query(boolean, const char *);
extern void makemap_prepost(boolean, boolean);
extern const char *ecname_from_fn(int (*)(void));

/* ### date.c ### */

extern void populate_nomakedefs(struct version_info *) NONNULLARG1;
extern void free_nomakedefs(void);

/* ### dbridge.c ### */

extern boolean is_waterwall(coordxy, coordxy);
extern boolean is_pool(coordxy, coordxy);
extern boolean is_lava(coordxy, coordxy);
extern boolean is_pool_or_lava(coordxy, coordxy);
extern boolean is_ice(coordxy, coordxy);
extern boolean is_moat(coordxy, coordxy);
extern schar db_under_typ(int);
extern int is_drawbridge_wall(coordxy, coordxy);
extern boolean is_db_wall(coordxy, coordxy);
extern boolean find_drawbridge(coordxy *, coordxy *) NONNULLPTRS;
extern boolean create_drawbridge(coordxy, coordxy, int, boolean);
extern void open_drawbridge(coordxy, coordxy);
extern void close_drawbridge(coordxy, coordxy);
extern void destroy_drawbridge(coordxy, coordxy);

/* ### decl.c ### */

extern void program_state_init(void);
extern void level_status_init(void);
extern void decl_globals_init(void);
extern void sa_victual(volatile struct victual_info *);

/* ### detect.c ### */

extern boolean trapped_chest_at(int, coordxy, coordxy);
extern boolean trapped_door_at(int, coordxy, coordxy);
extern struct obj *o_in(struct obj *, char) NONNULLARG1;
extern struct obj *o_material(struct obj *, unsigned) NONNULLARG1;
extern int gold_detect(struct obj *) NONNULLARG1;
extern int food_detect(struct obj *);
extern int object_detect(struct obj *, int);
extern int monster_detect(struct obj *, int);
extern int trap_detect(struct obj *);
extern const char *level_distance(d_level *) NONNULL NONNULLARG1;
extern void use_crystal_ball(struct obj **) NONNULLARG1;
extern void show_map_spot(coordxy, coordxy, boolean);
extern void do_mapping(void);
extern void do_vicinity_map(struct obj *);
extern void cvt_sdoor_to_door(struct rm *) NONNULLARG1;
extern int findit(void);
extern int openit(void);
extern boolean detecting(void(*)(coordxy, coordxy, void *));
extern void find_trap(struct trap *) NONNULLARG1;
extern void warnreveal(void);
extern int dosearch0(int);
extern int dosearch(void);
extern void premap_detect(void);
#ifdef DUMPLOG
extern void dump_map(void);
#endif
extern void reveal_terrain(unsigned);
extern int wiz_mgender(void);

/* ### dig.c ### */

extern int dig_typ(struct obj *, coordxy, coordxy);
extern boolean is_digging(void);
extern int holetime(void);
extern enum digcheck_result dig_check(struct monst *, coordxy, coordxy);
extern void digcheck_fail_message(enum digcheck_result, struct monst *,
                                  coordxy, coordxy);
extern void digactualhole(coordxy, coordxy, struct monst *, int);
extern boolean dighole(boolean, boolean, coord *);
extern int use_pick_axe(struct obj *) NONNULLARG1;
extern int use_pick_axe2(struct obj *) NONNULLARG1;
extern boolean mdig_tunnel(struct monst *) NONNULLARG1;
extern void draft_message(boolean);
extern void watch_dig(struct monst *, coordxy, coordxy, boolean);
extern void zap_dig(void);
extern struct obj *bury_an_obj(struct obj *, boolean *) NONNULLARG1;
extern void bury_objs(int, int);
extern void unearth_objs(int, int);
extern void rot_organic(union any *, long) NONNULLARG1;
extern void rot_corpse(union any *, long) NONNULLARG1;
extern struct obj *buried_ball(coord *) NONNULLARG1;
extern void buried_ball_to_punishment(void);
extern void buried_ball_to_freedom(void);
extern schar fillholetyp(coordxy, coordxy, boolean);
extern void liquid_flow(coordxy, coordxy, schar, struct trap *, const char *);
extern boolean conjoined_pits(struct trap *, struct trap *, boolean);
#if 0
extern void bury_monst(struct monst *) NONNULLARG1;
extern void bury_you(void);
extern void unearth_you(void);
extern void escape_tomb(void);
extern void bury_obj(struct obj *) NONNULLARG1;
#endif
#ifdef DEBUG
extern int wiz_debug_cmd_bury(void);
#endif

/* ### display.c ### */

/**
 * @name The perception tests, as functions
 * @brief The same questions display.h asks as macros, provided as functions.
 * @note Both forms exist deliberately, and display.h explains the reason: the functions keep the compiled code small, and the macro forms are preferred only where a call is measurably
 *       too slow. So these are what almost all code should use.
 * @note Read display.h for what each actually asks. They differ in ways their names do not reveal -- which of them assumes the square is visible, which accepts heat as sight, which is
 *       unreliable for a concealed mimic.
 * @{
 */
/**
 * @name 지각 검사들. 함수로서.
 * @brief display.h 가 매크로로 묻는 것과 같은 질문들. 함수로 제공된다.
 * @note 두 형태가 의도적으로 존재하며, display.h 가 그 이유를 설명한다. 함수는 컴파일된 코드를 작게 유지하고, 매크로 형태는 호출이 측정 가능하게 느린 곳에서만 택해진다. 그래서 이들이 거의 모든 코드가 써야 하는 것이다.
 * @note 각각이 실제로 무엇을 묻는지는 display.h 를 볼 것. 그것들은 이름이 드러내지 않는 방식으로 서로 다르다. 어느 것이 그 칸이 보인다고 가정하는지, 어느 것이 열을 시각으로 받아들이는지, 어느 것이 위장한 모방자에 대해 신뢰할 수 없는지.
 * @{
 */
extern int tp_sensemon(struct monst *) NONNULLARG1;
extern int sensemon(struct monst *) NONNULLARG1;
extern int mon_warning(struct monst *) NONNULLARG1;
extern int mon_visible(struct monst *) NONNULLARG1;
extern int see_with_infrared(struct monst *) NONNULLARG1;
extern int canseemon(struct monst *) NONNULLARG1;
extern int knowninvisible(struct monst *) NONNULLARG1;
extern int is_safemon(struct monst *) NONNULLARG1;
/** @} */
/**
 * @brief Record what magic mapping revealed at a square, without the hero having seen it.
 * @note Distinct from ordinary background mapping because what magic reveals differs from what looking reveals: a magically mapped square is known but not seen, and some things are
 *       omitted.
 */
/**
 * @brief 마법 지도가 어떤 칸에서 드러낸 것을 기록한다. 영웅이 그것을 본 것 없이.
 * @note 평범한 배경 기록과 구별되는 것은, 마법이 드러내는 것이 보는 것이 드러내는 것과 다르기 때문이다. 마법으로 기록된 칸은 알려졌으나 보이지는 않았으며, 몇 가지는 빠진다.
 */
extern void magic_map_background(coordxy, coordxy, int);
/**
 * @brief Record what the hero now knows the terrain at a square to be.
 * @note Writes the hero's memory rather than the map. The map is what is there; this is what the hero believes -- and the two are allowed to differ, which is what makes a remembered
 *       map possible.
 */
/**
 * @brief 영웅이 이제 어떤 칸의 지형이 무엇이라고 아는지 기록한다.
 * @note 지도가 아니라 영웅의 기억에 쓴다. 지도는 거기 있는 것이고, 이것은 영웅이 믿는 것이다. 그리고 그 둘은 달라도 되며, 그것이 기억된 지도를 가능하게 하는 것이다.
 */
extern void map_background(coordxy, coordxy, int);
extern void map_trap(struct trap *, int) NONNULLARG1;
extern void map_object(struct obj *, int) NONNULLARG1;
/**
 * @name Remembering an unseen monster
 * @brief Mark a square as holding something the hero knows is there but cannot see, and clear that mark.
 * @note The mark is the hero's belief, so it persists after the monster has gone -- which is the point: the player should have to check rather than being told the invisible thing left.
 * @note Clearing it reports whether there was anything to clear, so a caller can tell whether the hero's belief was corrected.
 * @{
 */
/**
 * @name 보이지 않는 몬스터를 기억하기
 * @brief 어떤 칸을, 영웅이 거기 있다고 알지만 볼 수 없는 무언가를 담은 것으로 표시하고, 그 표시를 지운다.
 * @note 그 표시는 영웅의 믿음이므로 몬스터가 떠난 뒤에도 남는다. 그것이 요점이다. 플레이어는 그 투명한 것이 떠났다고 알려지는 대신 확인해야 한다.
 * @note 지우는 쪽은 지울 것이 있었는지 알린다. 그래서 호출자가 영웅의 믿음이 바로잡혔는지 알 수 있다.
 * @{
 */
extern void map_invisible(coordxy, coordxy);
extern boolean unmap_invisible(coordxy, coordxy);
/** @} */
extern void map_engraving(struct engr *, int);
extern void unmap_object(coordxy, coordxy);
extern void map_location(coordxy, coordxy, int);
/**
 * @brief Whether map drawing should be withheld at the moment.
 * @note True during level generation and a few other times when the map is not in a consistent state. Drawing then would show a half-built level, so this is asked rather than each such
 *       situation remembering to suppress drawing itself.
 */
/**
 * @brief 지금 지도 그리기를 보류해야 하는지.
 * @note 레벨 생성 중과 지도가 일관된 상태가 아닌 몇몇 다른 때에 참이다. 그때 그리면 반쯤 지어진 레벨이 보이므로, 그런 상황마다 스스로 그리기를 억제하기를 기억하는 대신 이것을 묻는다.
 */
extern boolean suppress_map_output(void);
/**
 * @name Learning a square by touch
 * @brief Record what the hero found out about a square by feeling it rather than seeing it.
 * @note What touch reveals is not what sight reveals -- the terrain is learned but not what is lying on it -- so this is a different operation and not a blind hero's version of the same
 *       one.
 * @{
 */
/**
 * @name 만져서 칸을 알기
 * @brief 영웅이 보는 것이 아니라 만져서 어떤 칸에 대해 알아낸 것을 기록한다.
 * @note 촉각이 드러내는 것은 시각이 드러내는 것과 다르다. 지형은 알게 되지만 그 위에 놓인 것은 그렇지 않다. 그래서 이것은 같은 연산의 눈먼 영웅용 판본이 아니라 다른 연산이다.
 * @{
 */
extern void feel_newsym(coordxy, coordxy);
extern void feel_location(coordxy, coordxy);
/** @} */
/**
 * @brief Work out afresh what should be shown at a square, and show it.
 * @note The workhorse of the display. Nearly everything that changes the world calls this for the affected squares, which is why it must be cheap and why it decides for itself whether
 *       anything actually needs redrawing.
 */
/**
 * @brief 어떤 칸에 무엇이 보여야 하는지 새로 계산하고 그것을 보인다.
 * @note 표시부의 일꾼이다. 세계를 바꾸는 거의 모든 것이 영향받은 칸에 대해 이것을 호출한다. 그래서 값이 싸야 하고, 실제로 다시 그릴 것이 있는지를 스스로 정하는 이유가 그것이다.
 */
extern void newsym(coordxy, coordxy);
/**
 * @brief The same, but draw whether or not anything appears to have changed.
 * @note For the cases where the appearance changed without the game's own state doing so -- a symbol setting altered, the display reinitialised. The ordinary form would decide there was
 *       nothing to do.
 */
/**
 * @brief 같은 일을 하되, 무엇이 바뀐 것처럼 보이는지와 무관하게 그린다.
 * @note 게임 자신의 상태가 바뀌지 않은 채로 외형이 바뀐 경우를 위한 것이다. 심볼 설정이 바뀌었거나, 표시부가 다시 초기화되었거나. 평범한 형태는 할 일이 없다고 판단할 것이다.
 */
extern void newsym_force(coordxy, coordxy);
extern void shieldeff(coordxy, coordxy);
extern void tmp_at(coordxy, coordxy);
extern void flash_glyph_at(coordxy, coordxy, int, int);
extern void swallowed(int);
extern void under_ground(int);
extern void under_water(int);
extern void see_monsters(void);
extern void set_mimic_blocking(void);
extern void see_objects(void);
extern void see_nearby_objects(void);
extern void see_traps(void);
extern void curs_on_u(void);
extern int doredraw(void);
extern void docrt(void);
extern void docrt_flags(int);
extern void redraw_map(boolean);
/**
 * @brief Put a glyph at a square in the game's copy of the screen.
 * @note Does not draw. It records what should be there, and the drawing happens when the screen is flushed -- which is what lets many changes in one turn produce one redraw.
 */
/**
 * @brief 게임의 화면 사본에서 어떤 칸에 글리프를 놓는다.
 * @note 그리지 않는다. 무엇이 거기 있어야 하는지를 기록하고, 그리기는 화면이 비워질 때 일어난다. 그것이 한 턴 안의 많은 변경이 한 번의 다시 그리기를 내게 하는 것이다.
 */
extern void show_glyph(coordxy, coordxy, int);
extern void clear_glyph_buffer(void);
extern void row_refresh(coordxy, coordxy, coordxy);
extern void cls(void);
/**
 * @brief Send everything that has changed to the display.
 * @note The counterpart of recording glyphs. Its argument says how urgently -- some callers need the player to see the result before the next thing happens, and others are content to let
 *       it wait.
 */
/**
 * @brief 바뀐 모든 것을 표시부로 보낸다.
 * @note 글리프를 기록하는 것의 짝이다. 그 인자가 얼마나 급한지를 말한다. 어떤 호출자는 다음 일이 일어나기 전에 플레이어가 그 결과를 보아야 하고, 어떤 호출자는 그것이 기다려도 괜찮다.
 */
extern void flush_screen(int);
/**
 * @brief The glyph for the terrain at a square, ignoring everything standing on it.
 * @note Answers "what is the floor here" rather than "what is here". That distinction is what lets something moving across a square be erased by restoring what was underneath, rather
 *       than by asking the game what is there -- which would include the thing being erased.
 */
/**
 * @brief 어떤 칸의 지형에 해당하는 글리프. 그 위에 서 있는 모든 것을 무시하고.
 * @note "여기 무엇이 있는가"가 아니라 "여기 바닥이 무엇인가"에 답한다. 그 구별이, 칸을 지나가는 것이 그 아래에 있던 것을 되돌려서 지워질 수 있게 하는 것이다. 게임에게 거기 무엇이 있는지 묻는 방식이 아니라. 그렇게 물으면 지워지는 대상이 포함된다.
 */
extern int back_to_glyph(coordxy, coordxy);
extern int zapdir_to_glyph(int, int, int);
/**
 * @brief The glyph currently shown at a square.
 * @warning What is displayed, not what is there. It reads the game's copy of the screen, so it reflects the hero's knowledge and whatever transient effect is being drawn -- not the
 *          world.
 */
/**
 * @brief 어떤 칸에 지금 보여지고 있는 글리프.
 * @warning 거기 있는 것이 아니라 표시되고 있는 것이다. 게임의 화면 사본을 읽으므로, 영웅의 앎과 지금 그려지고 있는 일시적인 효과를 반영한다. 세계가 아니다.
 */
extern int glyph_at(coordxy, coordxy);
/**
 * @brief Redraw every unlit room square after the setting that governs them changed.
 * @note A whole-map operation for a single option, because whether an unlit room shows as dark or as floor affects every such square at once and nothing else would notice.
 */
/**
 * @brief 그것들을 지배하는 설정이 바뀐 뒤, 불 없는 모든 방 칸을 다시 그린다.
 * @note 하나의 선택지를 위한 지도 전체 연산이다. 불 없는 방이 어둡게 보이는지 바닥으로 보이는지가 그런 모든 칸에 한꺼번에 영향을 주고, 다른 어느 것도 그것을 알아채지 못하기 때문이다.
 */
extern void reglyph_darkroom(void);
/**
 * @name How a wall joins its neighbours
 * @brief Work out which of the wall shapes each wall square should be drawn as.
 * @note A wall's appearance depends on which of its neighbours are also walls -- a corner, a tee, a crossing. That is derived rather than stored, so it has to be recomputed whenever the
 *       map changes around a wall.
 * @note One form does a single square and one does the whole level; the single-square form exists because digging changes one wall and its neighbours rather than the map.
 * @{
 */
/**
 * @name 벽이 이웃과 어떻게 이어지는지
 * @brief 각 벽 칸이 어떤 벽 모양으로 그려져야 하는지 알아낸다.
 * @note 벽의 외형은 그 이웃 중 어느 것이 또한 벽인지에 달려 있다. 모서리, T자, 십자. 그것은 저장되지 않고 유도되므로, 벽 둘레의 지도가 바뀔 때마다 다시 계산되어야 한다.
 * @note 한 형태는 칸 하나를 하고 한 형태는 레벨 전체를 한다. 칸 하나 형태가 있는 것은 굴착이 지도가 아니라 벽 하나와 그 이웃을 바꾸기 때문이다.
 * @{
 */
extern void xy_set_wall_state(coordxy, coordxy);
extern void set_wall_state(void);
/** @} */
/**
 * @brief Forget that a square was seen from certain directions.
 * @note Which directions a square has been seen from is remembered, because a wall looks different depending on which side it was viewed from. This unremembers some of that -- for
 *       instance when the wall is dug through and the old view no longer applies.
 */
/**
 * @brief 어떤 칸이 특정 방향에서 보였다는 것을 잊는다.
 * @note 칸이 어느 방향에서 보였는지가 기억된다. 벽이 어느 쪽에서 보였는지에 따라 다르게 보이기 때문이다. 이것은 그 중 일부를 잊는다. 예컨대 그 벽이 파여 지나가지고 예전의 시점이 더는 적용되지 않을 때.
 */
extern void unset_seenv(struct rm *, coordxy, coordxy, coordxy, coordxy);
/**
 * @brief Which degree of warning a monster deserves.
 * @note Not whether -- that is a separate question. This is the level, and it is what decides which of the warning symbols is shown, so the player learns roughly how dangerous
 *       something is without learning what it is.
 */
/**
 * @brief 몬스터가 어느 정도의 경고를 받을 만한지.
 * @note 여부가 아니다. 그것은 별개의 질문이다. 이것은 단계이며, 경고 심볼 중 어느 것이 보여질지를 정하는 것이다. 그래서 플레이어는 그것이 무엇인지 알지 못한 채로 얼마나 위험한지를 대략 알게 된다.
 */
extern int warning_of(struct monst *) NONNULLARG1;
/**
 * @brief Resolve a glyph into everything a display needs in order to draw it.
 * @note The bridge between the game's numbering and the display's drawing. It fills in every representation at once, because the core does not know which one this display will read.
 * @note The square is passed as well as the glyph because some of the resolution depends on where it is -- what a wall looks like, whether an accessibility substitution applies.
 */
/**
 * @brief 글리프를 표시부가 그것을 그리기 위해 필요한 모든 것으로 해석한다.
 * @note 게임의 번호 체계와 표시부의 그리기 사이의 다리다. 모든 표현을 한꺼번에 채우는 것은, 코어가 이 표시부가 어느 것을 읽을지 모르기 때문이다.
 * @note 글리프와 함께 칸도 전달되는 것은, 그 해석의 일부가 그것이 어디인지에 달려 있기 때문이다. 벽이 어떻게 보이는지, 접근성 대체가 적용되는지.
 */
extern void map_glyphinfo(coordxy, coordxy, int, unsigned, glyph_info *) NONNULLPTRS;
/**
 * @brief Rebuild the cached mapping from glyphs to appearances.
 * @note Takes the reason rather than a flag, because how much has to be rebuilt depends on why -- a new game rebuilds everything, a change of level only what depends on where the hero
 *       is.
 */
/**
 * @brief 글리프에서 외형으로의 캐시된 대응을 다시 만든다.
 * @note 플래그가 아니라 이유를 받는다. 얼마만큼을 다시 만들어야 하는지가 왜인지에 달려 있기 때문이다. 새 게임은 전부를 다시 만들고, 레벨 변경은 영웅이 어디 있는지에 달린 것만을 다시 만든다.
 */
extern void reset_glyphmap(enum glyphmap_change_triggers trigger);
extern int fn_cmap_to_glyph(int);

/* ### do.c ### */

extern int dodrop(void);
extern boolean boulder_hits_pool(struct obj *, coordxy, coordxy, boolean);
extern boolean flooreffects(struct obj *, coordxy, coordxy,
                            const char *) NONNULLPTRS;
extern void doaltarobj(struct obj *) NONNULLARG1;
extern void polymorph_sink(void);
extern void trycall(struct obj *) NONNULLARG1;
extern boolean canletgo(struct obj *, const char *) NONNULLPTRS;
extern void dropx(struct obj *) NONNULLARG1;
extern void dropy(struct obj *) NONNULLARG1;
extern void dropz(struct obj *, boolean) NONNULLARG1;
extern void obj_no_longer_held(struct obj *);
extern int doddrop(void);
extern int dodown(void);
extern int doup(void);
#ifdef INSURANCE
extern void save_currentstate(void);
#endif
extern void u_collide_m(struct monst *);
extern void goto_level(d_level *, boolean, boolean, boolean) NONNULLARG1;
extern void hellish_smoke_mesg(void);
extern void maybe_lvltport_feedback(void);
extern void schedule_goto(d_level *, int, const char *, const char *) NONNULLARG1;
extern void deferred_goto(void);
extern boolean revive_corpse(struct obj *) NONNULLARG1;
extern void revive_mon(union any *, long) NONNULLARG1;
extern void zombify_mon(union any *, long) NONNULLARG1;
extern boolean cmd_safety_prevention(const char *, const char *,
                                     const char *, int *) NONNULLPTRS;
extern int donull(void);
extern int dowipe(void);
extern void legs_in_no_shape(const char *, boolean) NONNULLARG1;
extern void set_wounded_legs(long, int);
extern void heal_legs(int);

/* ### do_name.c ### */

extern void new_mgivenname(struct monst *, int) NONNULLARG1;
extern void free_mgivenname(struct monst *) NONNULLARG1;
extern void new_oname(struct obj *, int) NONNULLARG1;
extern void free_oname(struct obj *) NONNULLARG1;
extern const char *safe_oname(struct obj *) NONNULLARG1;
extern struct monst *christen_monst(struct monst *, const char *) NONNULL
                                                                   NONNULLARG1;
extern struct obj *oname(struct obj *, const char *, unsigned) NONNULLPTRS;
extern boolean objtyp_is_callable(int);
extern int name_ok(struct obj *);
extern int call_ok(struct obj *);
extern int docallcmd(void);
extern void docall(struct obj *) NONNULLARG1;
extern const char *rndghostname(void);
extern char *x_monnam(struct monst *, int, const char *, int, boolean) NONNULLARG1;
extern char *l_monnam(struct monst *) NONNULLARG1;
extern char *mon_nam(struct monst *) NONNULLARG1;
extern char *noit_mon_nam(struct monst *) NONNULLARG1;
extern char *some_mon_nam(struct monst *) NONNULLARG1;
extern char *Monnam(struct monst *) NONNULLARG1;
extern char *noit_Monnam(struct monst *) NONNULLARG1;
extern char *Some_Monnam(struct monst *) NONNULLARG1;
extern char *noname_monnam(struct monst *, int) NONNULLARG1;
extern char *m_monnam(struct monst *) NONNULLARG1;
extern char *y_monnam(struct monst *) NONNULLARG1;
extern char *YMonnam(struct monst *) NONNULLARG1;
extern char *Adjmonnam(struct monst *, const char *) NONNULLARG1;
extern char *Amonnam(struct monst *) NONNULLARG1;
extern char *a_monnam(struct monst *) NONNULLARG1;
extern char *distant_monnam(struct monst *, int, char *) NONNULLARG1;
extern char *mon_nam_too(struct monst *, struct monst *) NONNULLPTRS;
extern char *monverbself(struct monst *, char *,
                         const char *, const char *) NONNULLARG123;
extern char *minimal_monnam(struct monst *, boolean);
extern char *bogusmon(char *, char *) NONNULLARG1;
extern char *rndmonnam(char *);
extern const char *hcolor(const char *);
extern const char *rndcolor(void);
extern const char *hliquid(const char *);
extern const char *roguename(void);
/*
extern struct obj *realloc_obj(struct obj *, int, genericptr_t, int,
                               const char *);
*/
extern char *coyotename(struct monst *, char *);
extern char *rndorcname(char *);
extern struct monst *christen_orc(struct monst *, const char *,
                                  const char *) NONNULLARG1;
extern const char *noveltitle(int *);
extern const char *lookup_novel(const char *, int *) NONNULLARG1;
#ifndef PMNAME_MACROS
extern int Mgender(struct monst *) NONNULLARG1;
extern const char *pmname(struct permonst *, int) NONNULLARG1;
#endif
extern const char *mon_pmname(struct monst *) NONNULLARG1;
extern const char *obj_pmname(struct obj *) NONNULLARG1;

/* ### do_wear.c ### */

extern const char *fingers_or_gloves(boolean);
extern void off_msg(struct obj *) NONNULLARG1;
extern void toggle_displacement(struct obj *, long, boolean);
extern void set_wear(struct obj *);
extern boolean donning(struct obj *) NONNULLARG1;
extern boolean doffing(struct obj *) NONNULLARG1;
extern void cancel_doff(struct obj *, long) NONNULLARG1;
extern void cancel_don(void);
extern int stop_donning(struct obj *); /* doseduce() calls with NULL */
extern int Armor_off(void);
extern int Armor_gone(void);
extern int Helmet_off(void);
extern boolean hard_helmet(struct obj *);
extern void wielding_corpse(struct obj *, struct obj *, boolean);
extern int Gloves_off(void);
extern int Boots_on(void);
extern int Boots_off(void);
extern int Cloak_off(void);
extern int Shield_off(void);
extern int Shirt_off(void);
extern void Amulet_off(void);
extern void Ring_on(struct obj *) NONNULLARG1;
extern void Ring_off(struct obj *) NONNULLARG1;
extern void Ring_gone(struct obj *) NONNULLARG1;
extern void Blindf_on(struct obj *) NONNULLARG1;
extern void Blindf_off(struct obj *);
extern int dotakeoff(void);
extern int ia_dotakeoff(void);
extern int doremring(void);
extern int cursed(struct obj *);
extern int armoroff(struct obj *);
extern int canwearobj(struct obj *, long *, boolean) NONNULLPTRS;
extern int dowear(void);
extern int doputon(void);
extern void find_ac(void);
extern void glibr(void);
extern struct obj *some_armor(struct monst *) NONNULLARG1;
extern struct obj *stuck_ring(struct obj *, int);
extern struct obj *unchanger(void);
extern void reset_remarm(void);
extern int doddoremarm(void);
extern int remarm_swapwep(void);
extern int disintegrate_arm(struct obj *);
extern int destroy_arm(void);
extern void adj_abon(struct obj *, schar) NONNULLARG1;
extern boolean inaccessible_equipment(struct obj *, const char *, boolean);
extern int any_worn_armor_ok(struct obj *);
extern int count_worn_armor(void);

/* ### dog.c ### */

extern void newedog(struct monst *) NONNULLARG1;
extern void free_edog(struct monst *) NONNULLARG1;
extern void initedog(struct monst *, boolean) NONNULLARG1;
extern struct monst *make_familiar(struct obj *, coordxy, coordxy, boolean);
extern struct monst *makedog(void);
/**
 * @brief Note on every monster the turn at which it was last considered.
 * @note Needed because a level the hero is not on does not run. When the hero returns, each monster there has to be brought forward by however long it was unattended, and this is what
 *       records the point to catch up from.
 */
/**
 * @brief 모든 몬스터에, 그것이 마지막으로 고려된 턴을 기록한다.
 * @note 영웅이 있지 않은 레벨은 돌아가지 않기 때문에 필요하다. 영웅이 돌아오면 그곳의 각 몬스터가 방치된 만큼 앞으로 당겨져야 하며, 이것이 따라잡을 기준점을 기록하는 것이다.
 */
extern void update_mlstmv(void);
/**
 * @brief Bring in the monsters that were following the hero to this level.
 * @warning The name is historical and much narrower than what it does: it places every monster that was in transit, pets and otherwise, and is what makes following the hero between
 *          levels work at all.
 */
/**
 * @brief 영웅을 따라 이 레벨로 오던 몬스터들을 들여온다.
 * @warning 그 이름은 역사적인 것이고 그것이 하는 일보다 훨씬 좁다. 이동 중이던 모든 몬스터를 놓으며, 애완동물이든 아니든이다. 그리고 그것이 영웅을 따라 레벨을 오가는 것을 아예 작동하게 하는 것이다.
 */
extern void losedogs(void);
extern void mon_arrive(struct monst *, int) NONNULLARG1;
/**
 * @brief Advance a monster by however long the level it was on stood still.
 * @note This is the other half of recording the last-considered turn. It applies healing, hunger and timers for the whole absence at once rather than stepping through it -- which is why
 *       a monster left alone for a thousand turns does not take a thousand turns to catch up.
 * @warning What is applied is a summary rather than a replay. A monster does not do anything during the absence; it only ends up as though time had passed.
 */
/**
 * @brief 몬스터가 있던 레벨이 멈춰 있던 만큼 그 몬스터를 진행시킨다.
 * @note 이것이 마지막 고려 턴을 기록하는 것의 나머지 절반이다. 부재 전체에 대한 치유, 배고픔, 타이머를 하나하나 밟아 나가는 대신 한꺼번에 적용한다. 그것이 천 턴 동안 방치된 몬스터가 따라잡는 데 천 턴이 걸리지 않는 이유다.
 * @warning 적용되는 것은 재생이 아니라 요약이다. 몬스터는 그 부재 동안 아무것도 하지 않는다. 시간이 지난 것처럼 되기만 한다.
 */
extern void mon_catchup_elapsed_time(struct monst *, long) NONNULLARG1;
/**
 * @brief Take with the hero whichever monsters should follow them off this level.
 * @note Called before the level is left, so it decides who comes rather than who arrives. Adjacency and leashes both matter here, which is why a pet across the room is left behind.
 */
/**
 * @brief 이 레벨을 떠나는 영웅을 따라야 할 몬스터들을 함께 데려간다.
 * @note 레벨을 떠나기 전에 호출되므로, 누가 도착하는지가 아니라 누가 오는지를 정한다. 여기서 인접함과 목줄이 둘 다 중요하며, 그래서 방 건너의 애완동물은 남겨진다.
 */
extern void keepdogs(boolean);
/**
 * @brief Set a monster aside to be placed on another level when that level is next entered.
 * @note A monster cannot be placed on a level that is not loaded, so it is held with a note saying where it should go. That note is why the placement argument may be a request rather
 *       than coordinates.
 */
/**
 * @brief 몬스터를 다른 레벨에 다음에 들어갈 때 놓이도록 따로 둔다.
 * @note 적재되지 않은 레벨에 몬스터를 놓을 수는 없으므로, 어디로 가야 하는지 적은 쪽지와 함께 보관된다. 그 쪽지가 배치 인자가 좌표가 아니라 요청일 수 있는 이유다.
 */
extern void migrate_to_level(struct monst *, xint16, xint16, coord *) NONNULLARG1;
extern void discard_migrations(void);
/**
 * @brief How a pet regards a piece of food.
 * @return one of the food rankings, best first -- so a lower answer is a better food
 * @warning The scale runs the opposite way from intuition. A pet chooses by preferring the smaller value, so comparing these as though larger were better inverts every pet's taste.
 */
/**
 * @brief 애완동물이 음식을 어떻게 보는지.
 * @return 음식 순위 중 하나. 좋은 것부터. 그래서 낮은 답이 더 좋은 음식이다
 * @warning 그 척도는 직관과 반대 방향으로 간다. 애완동물은 더 작은 값을 선호해서 고르므로, 큰 것이 낫다는 듯이 이것을 비교하면 모든 애완동물의 취향이 뒤집힌다.
 */
extern int dogfood(struct monst *, struct obj *) NONNULLPTRS;
/**
 * @brief Attempt to tame a monster, optionally by offering it something.
 * @return whether it became tame
 * @note The object may be null, for taming by means other than food. What will work depends on the monster, and the balance decisions behind that are documented with the test in
 *       mondata.h rather than here.
 */
/**
 * @brief 몬스터를 길들이려 시도한다. 선택적으로 무언가를 내주어서.
 * @return 그것이 길들여졌는지
 * @note 물건은 널일 수 있다. 음식 말고 다른 수단으로 길들이는 경우를 위해서다. 무엇이 통할지는 몬스터에 달려 있고, 그 뒤의 균형에 관한 결정은 여기가 아니라 mondata.h 의 검사와 함께 기록되어 있다.
 */
extern boolean tamedog(struct monst *, struct obj *, boolean) NONNULLARG1;
/**
 * @brief Record that the hero mistreated a pet.
 * @note Counted rather than acted on immediately. A pet's loyalty erodes with accumulated mistreatment, so one abuse rarely does anything visible and the record is what makes the
 *       eventual desertion follow from the history.
 */
/**
 * @brief 영웅이 애완동물을 학대했음을 기록한다.
 * @note 즉시 행동으로 옮겨지는 것이 아니라 세어진다. 애완동물의 충성은 쌓인 학대와 함께 무너지므로, 한 번의 학대가 눈에 보이는 일을 하는 경우는 드물고, 그 기록이 결국의 이탈이 그 이력에서 따라 나오게 하는 것이다.
 */
extern void abuse_dog(struct monst *) NONNULLARG1;
/**
 * @brief Make a pet distrustful, which is a step short of no longer being a pet.
 * @note A wary pet is still tame but keeps its distance and is harder to command. That intermediate state exists so that losing a pet's trust is visible before the pet is lost.
 */
/**
 * @brief 애완동물을 불신하게 만든다. 더는 애완동물이 아니게 되는 것의 한 단계 앞이다.
 * @note 경계하는 애완동물은 여전히 길들여져 있으나 거리를 두고 명령하기 어렵다. 그 중간 상태가 있는 것은, 애완동물을 잃기 전에 그 신뢰를 잃는 것이 눈에 보이도록 하기 위함이다.
 */
extern void wary_dog(struct monst *, boolean) NONNULLARG1;

/* ### dogmove.c ### */

/**
 * @brief Whether a square holds anything cursed.
 * @note Asked of pets, who avoid cursed things they can detect. So this is the game answering on the pet's behalf using knowledge the hero does not have -- which is why a pet refusing a
 *       square is information to the player.
 */
/**
 * @brief 어떤 칸이 저주받은 것을 담고 있는지.
 * @note 애완동물에게 물어진다. 그들은 감지할 수 있는 저주받은 것을 피한다. 그래서 이것은 게임이 영웅이 갖지 않은 지식으로 애완동물을 대신해 답하는 것이며, 애완동물이 어떤 칸을 거부하는 것이 플레이어에게 정보인 이유다.
 */
extern boolean cursed_object_at(coordxy, coordxy);
/**
 * @brief The things a monster is carrying that it would be willing to put down.
 * @return the first such object, or null if there are none
 * @note Not everything it carries. A monster keeps what it is using and what it values, so this is what a pet might fetch or a thief might discard.
 */
/**
 * @brief 몬스터가 지니고 있는 것 중 내려놓을 만한 것들.
 * @return 그런 물건 중 첫 번째. 없으면 널
 * @note 그것이 지닌 전부가 아니다. 몬스터는 쓰고 있는 것과 값지게 여기는 것을 지키므로, 이것은 애완동물이 물어 올 만한 것이거나 도둑이 버릴 만한 것이다.
 */
extern struct obj *droppables(struct monst *) NONNULLARG1;
extern int dog_nutrition(struct monst *, struct obj *) NONNULLPTRS;
extern int dog_eat(struct monst *, struct obj *,
                   coordxy, coordxy, boolean) NONNULLPTRS;
extern int pet_ranged_attk(struct monst *, boolean) NONNULLARG1;
extern int dog_move(struct monst *, int) NONNULLARG1;
/**
 * @brief Whether a monster could get at something on a given square.
 * @note About the medium rather than the distance: a land pet cannot reach into water, and a swimmer cannot reach onto land. So a nearby object may be unreachable and this is what says
 *       so.
 */
/**
 * @brief 몬스터가 주어진 칸의 무언가에 닿을 수 있을지.
 * @note 거리가 아니라 매체에 관한 것이다. 땅의 애완동물은 물속으로 손을 뻗을 수 없고, 수영하는 것은 땅으로 뻗을 수 없다. 그래서 가까운 물건이 닿을 수 없을 수 있고, 이것이 그것을 말한다.
 */
extern boolean could_reach_item(struct monst *, coordxy, coordxy) NONNULLARG1;
/**
 * @brief End a monster's meal, whether it finished or was interrupted.
 * @note A monster eating occupies several turns, so the meal is state that has to be cleared -- and it has to be cleared on interruption as well as completion, which is why this is one
 *       routine for both.
 */
/**
 * @brief 몬스터의 식사를 끝낸다. 그것을 마쳤든 방해받았든.
 * @note 몬스터가 먹는 것은 여러 턴을 차지하므로, 그 식사는 정리되어야 하는 상태다. 그리고 완료뿐 아니라 방해에서도 정리되어야 하며, 그래서 이것이 둘 모두를 위한 하나의 루틴이다.
 */
extern void finish_meating(struct monst *) NONNULLARG1;
/**
 * @brief Make a monster take on a disguise immediately.
 * @note Immediately rather than at the monster's own choosing, for the cases where the disguise is imposed -- a mimic created already hidden, or one made to hide by something else.
 */
/**
 * @brief 몬스터가 즉시 위장을 취하게 한다.
 * @note 그 몬스터 자신의 선택이 아니라 즉시다. 위장이 부과되는 경우를 위한 것이다. 이미 숨은 상태로 만들어진 모방자, 또는 다른 무엇에 의해 숨게 된 것.
 */
extern void quickmimic(struct monst *) NONNULLARG1;

/* ### dokick.c ### */

extern boolean ghitm(struct monst *, struct obj *) NONNULLPTRS;
extern void container_impact_dmg(struct obj *, coordxy, coordxy) NONNULLARG1;
extern int dokick(void);
/**
 * @brief Send an object down a hole, a trapdoor or a chute to another level.
 * @return whether it was sent, since the square may have nothing to send it down
 * @note The object leaves this level and arrives on another, so it goes through the same setting-aside as a migrating monster. That is why it does not simply appear where it was aimed.
 */
/**
 * @brief 물건을 구멍이나 뚜껑문이나 활송로로 다른 레벨로 보낸다.
 * @return 보내졌는지. 그 칸에 그것을 내려보낼 것이 없을 수 있다
 * @note 물건이 이 레벨을 떠나 다른 레벨에 도착하므로, 이동하는 몬스터와 같은 따로 두기를 거친다. 그것이 겨냥된 곳에 그냥 나타나지 않는 이유다.
 */
extern boolean ship_object(struct obj *, coordxy, coordxy, boolean);
/**
 * @brief Place the objects that were set aside for this level.
 * @note The counterpart of sending them. Called on arriving at a level, and its argument distinguishes objects that should land where they were aimed from those that may scatter.
 */
/**
 * @brief 이 레벨을 위해 따로 두어졌던 물건들을 놓는다.
 * @note 그것들을 보낸 것의 짝이다. 레벨에 도착할 때 호출되며, 그 인자가 겨냥된 곳에 놓여야 하는 물건과 흩어져도 되는 물건을 구별한다.
 */
extern void obj_delivery(boolean);
extern void deliver_obj_to_mon(struct monst *mtmp, int, unsigned long) NONNULLARG1;
/**
 * @brief What kind of way down, if any, a square has.
 * @return the kind, or the no-destination value when there is none
 * @warning The failure value is not zero and is documented with the migration codes. Treating a zero answer as "nothing here" is wrong -- zero is a valid kind.
 */
/**
 * @brief 어떤 칸에 내려가는 길이 있다면 어떤 종류인지.
 * @return 그 종류. 없으면 목적지 없음 값
 * @warning 실패 값은 0이 아니며 이동 코드와 함께 기록되어 있다. 0인 답을 "여기 아무것도 없음"으로 취급하는 것은 틀리다. 0은 유효한 종류다.
 */
extern schar down_gate(coordxy, coordxy);
/**
 * @brief Shake loose whatever a heavy impact would dislodge from above.
 * @note What falls depends on where the impact was -- a ceiling above, a level above -- so this is about the impact's surroundings and not about the object that caused it.
 */
/**
 * @brief 무거운 충격이 위에서 흔들어 떨어뜨릴 것을 떨어뜨린다.
 * @note 무엇이 떨어지는지는 그 충격이 어디였는지에 달려 있다. 위의 천장, 위의 레벨. 그래서 이것은 그것을 일으킨 물건에 관한 것이 아니라 그 충격의 둘레에 관한 것이다.
 */
extern void impact_drop(struct obj *, coordxy, coordxy, xint16);

/* ### dothrow.c ### */

extern int multishot_class_bonus(int, struct obj *, struct obj *) NONNULLARG2;
extern int dothrow(void);
extern int dofire(void);
extern void endmultishot(boolean);
extern void hitfloor(struct obj *, boolean) NONNULLARG1;
/**
 * @name Being flung across the map
 * @brief Moving someone along a line against their will, a square at a time.
 *
 * Being knocked back is not a move the hero or a monster chose, so it does not go through the ordinary movement code. It is walked square by square instead, stopping when something is
 * in the way, and each step may have consequences of its own.
 *
 * @note The step routines take an untyped pointer because the same path-walking serves the hero and a monster, and the two are not the same type. What it points at is settled by which
 *       routine is being used.
 * @note A jump is separate from a step because a jump passes over the squares between rather than entering them, so what stops it is different.
 * @{
 */
/**
 * @name 지도를 가로질러 날려지기
 * @brief 누군가를 그 뜻과 무관하게 선을 따라 한 칸씩 옮기기.
 *
 * 뒤로 밀려나는 것은 영웅이나 몬스터가 고른 이동이 아니므로, 평범한 이동 코드를 거치지 않는다. 대신 칸 단위로 걸어지며 무언가가 가로막을 때 멈추고, 각 걸음이 자기 결과를 가질 수 있다.
 *
 * @note 걸음 루틴들이 타입 없는 포인터를 받는 것은, 같은 경로 걷기가 영웅과 몬스터를 함께 맡고 그 둘이 같은 타입이 아니기 때문이다. 그것이 무엇을 가리키는지는 어느 루틴이 쓰이고 있는지가 정한다.
 * @note 도약이 걸음과 따로 있는 것은, 도약이 사이의 칸에 들어가는 것이 아니라 그 위를 지나기 때문이다. 그래서 그것을 멈추는 것이 다르다.
 * @{
 */
extern boolean hurtle_jump(genericptr_t, coordxy, coordxy) NONNULLARG1;
extern boolean hurtle_step(genericptr_t, coordxy, coordxy) NONNULLARG1;
/**
 * @brief Whether a monster would actually be moved by a knock-back.
 * @note Asked before applying one, because a monster that is too heavy, anchored or otherwise immovable should produce a different message rather than a knock-back that goes nowhere.
 */
/**
 * @brief 몬스터가 밀려남에 의해 실제로 움직여질지.
 * @note 그것을 적용하기 전에 물어진다. 너무 무겁거나 고정되어 있거나 그 밖의 이유로 움직일 수 없는 몬스터는 아무 데도 가지 않는 밀려남 대신 다른 메시지를 내야 하기 때문이다.
 */
extern boolean will_hurtle(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void hurtle(int, int, int, boolean);
extern void mhurtle(struct monst *, int, int, int) NONNULLARG1;
/** @} */
/**
 * @brief Whether a thrown object would do no damage at all.
 * @note Exists so that throwing something harmless can be described differently rather than reported as a miss. A cream pie striking a monster is an event, not a failed attack.
 */
/**
 * @brief 던진 물건이 아무 피해도 주지 않을지.
 * @note 무해한 것을 던지는 일이 빗맞음으로 보고되는 대신 다르게 기술될 수 있도록 존재한다. 크림 파이가 몬스터를 맞히는 것은 실패한 공격이 아니라 하나의 사건이다.
 */
extern boolean harmless_missile(struct obj *) NONNULLARG1;
/**
 * @brief Whether an object is a weapon meant to be thrown.
 * @note Not whether it can be thrown -- anything can. This is whether throwing it is its intended use, which decides whether the hero's skill with it applies.
 */
/**
 * @brief 물건이 던지도록 만들어진 무기인지.
 * @note 던질 수 있는지가 아니다. 무엇이든 던질 수 있다. 이것은 던지는 것이 그 의도된 용도인지이며, 그것이 영웅의 그 기술이 적용되는지를 정한다.
 */
extern boolean throwing_weapon(struct obj *) NONNULLARG1;
extern boolean throwit_mon_hit(struct obj *, struct monst *) NONNULLARG1;
extern void throwit(struct obj *, long, boolean, struct obj *) NONNULLARG1;
extern int omon_adj(struct monst *, struct obj *, boolean) NONNULLPTRS;
/**
 * @brief Whether a fired missile should be destroyed rather than recovered.
 * @note Missiles are consumed at a rate rather than always or never, so that firing is not free and not prohibitively expensive. This is where that rate is applied, which is why an
 *       arrow sometimes survives and sometimes does not.
 */
/**
 * @brief 발사된 투사체가 회수되는 대신 파괴되어야 하는지.
 * @note 투사체는 항상도 결코도 아니라 어떤 비율로 소모된다. 그래서 발사가 공짜도 아니고 감당 못 할 만큼 비싸지도 않다. 그 비율이 적용되는 곳이 여기이며, 그래서 화살이 때로는 살아남고 때로는 그렇지 않다.
 */
extern boolean should_mulch_missile(struct obj *);
extern int thitmonst(struct monst *, struct obj *) NONNULLPTRS;
/**
 * @name Breaking an object
 * @brief Break something, with one form that attributes the breakage to the hero.
 * @note Two forms because the consequences differ, not the breaking: something the hero broke may be paid for, may anger a shopkeeper, may count against a conduct. The general form is
 *       for breakage with no culprit.
 * @{
 */
/**
 * @name 물건 부수기
 * @brief 무언가를 부순다. 한 형태는 그 부숨을 영웅에게 귀속시킨다.
 * @note 두 형태인 것은 부숨이 아니라 그 결과가 다르기 때문이다. 영웅이 부순 것은 값을 물어야 할 수도, 상점 주인을 화나게 할 수도, 계율에 셈해질 수도 있다. 일반 형태는 범인이 없는 부숨을 위한 것이다.
 * @{
 */
extern int hero_breaks(struct obj *, coordxy, coordxy, unsigned);
extern int breaks(struct obj *, coordxy, coordxy) NONNULLARG1;
/** @} */
extern void release_camera_demon(struct obj *, coordxy, coordxy) NONNULLARG1;
extern int breakobj(struct obj *, coordxy, coordxy, boolean, boolean) NONNULLARG1;
extern boolean breaktest(struct obj *) NONNULLARG1;
extern boolean walk_path(coord *, coord *,
                         boolean(*)(void *, coordxy, coordxy), genericptr_t) NONNULLARG12;

/* ### drawing.c ### */

extern int def_char_to_objclass(char);
extern int def_char_to_monclass(char);
extern int def_char_is_furniture(char);

/* ### dungeon.c ### */

extern void save_dungeon(NHFILE *, boolean, boolean) NONNULLARG1;
extern void restore_dungeon(NHFILE *) NONNULLARG1;
extern void insert_branch(branch *, boolean) NONNULLARG1;
extern void init_dungeons(void);
extern s_level *find_level(const char *) NONNULLARG1;
extern s_level *Is_special(d_level *) NONNULLARG1;
extern branch *Is_branchlev(d_level *) NONNULLARG1;
extern boolean builds_up(d_level *) NONNULLARG1;
extern xint16 ledger_no(d_level *) NONNULLARG1;
extern xint16 maxledgerno(void);
extern schar depth(d_level *) NONNULLARG1;
extern xint16 dunlev(d_level *) NONNULLARG1;
extern xint16 dunlevs_in_dungeon(d_level *) NONNULLARG1;
extern xint16 ledger_to_dnum(xint16);
extern xint16 ledger_to_dlev(xint16);
extern xint16 deepest_lev_reached(boolean);
extern boolean on_level(d_level *, d_level *) NONNULLARG12;
extern void next_level(boolean);
extern void prev_level(boolean);
extern void u_on_newpos(coordxy, coordxy);
extern void u_on_rndspot(int);
extern void get_level(d_level *, int) NONNULLARG1;
extern boolean Is_botlevel(d_level *) NONNULLARG1;
extern boolean Can_fall_thru(d_level *) NONNULLARG1;
extern boolean Can_dig_down(d_level *) NONNULLARG1;
extern boolean Can_rise_up(coordxy, coordxy, d_level *) NONNULLARG3;
extern boolean has_ceiling(d_level *) NONNULLARG1;
extern boolean avoid_ceiling(d_level *) NONNULLARG1;
extern const char *surface(coordxy, coordxy);
extern const char *ceiling(coordxy, coordxy);
extern boolean In_quest(d_level *) NONNULLARG1;
extern boolean In_mines(d_level *) NONNULLARG1;
extern branch *dungeon_branch(const char *) NONNULL NONNULLARG1;
extern boolean at_dgn_entrance(const char *) NONNULLARG1;
extern boolean In_hell(d_level *) NONNULLARG1;
extern boolean In_V_tower(d_level *) NONNULLARG1;
extern boolean On_W_tower_level(d_level *) NONNULLARG1;
extern boolean In_W_tower(coordxy, coordxy, d_level *) NONNULLARG3;
extern void find_hell(d_level *) NONNULLARG1;
extern void goto_hell(boolean, boolean);
extern boolean single_level_branch(d_level *) NONNULLARG1;
extern void assign_level(d_level *, d_level *) NONNULLPTRS;
extern void assign_rnd_level(d_level *, d_level *, int) NONNULLARG12;
extern unsigned int induced_align(int);
extern boolean Invocation_lev(d_level *) NONNULLARG1;
extern xint16 level_difficulty(void);
extern schar lev_by_name(const char *);
extern schar print_dungeon(boolean, schar *, xint16 *);
extern void print_level_annotation(void);
extern int donamelevel(void);
extern void free_exclusions(void);
extern void save_exclusions(NHFILE *) NONNULLARG1;
extern void load_exclusions(NHFILE *) NONNULLARG1;
extern int dooverview(void);
extern void show_overview(int, int);
extern void rm_mapseen(int);
extern void init_mapseen(d_level *) NONNULLARG1;
extern void update_lastseentyp(coordxy, coordxy);
extern int update_mapseen_for(coordxy, coordxy);
extern void recalc_mapseen(void);
extern void mapseen_temple(struct monst *);
extern void room_discovered(int);
extern void recbranch_mapseen(d_level *, d_level *) NONNULLPTRS;
extern void overview_stats(winid, const char *, long *, long *) NONNULLPTRS;
extern void remdun_mapseen(int);
extern const char *endgamelevelname(char *, int);

/* ### earlyarg.c ### */

extern int argcheck(int, char **, enum earlyarg);
extern void early_options(int *argc_p, char ***argv_p, char **hackdir_p);
#ifdef WIN32
int windows_early_options(const char *);
#endif
extern void genl_prag(int, char **); /* profession, race, align, gender */

/* ### eat.c ### */

extern void eatmupdate(void);
extern boolean is_edible(struct obj *) NONNULLARG1;
extern void init_uhunger(void);
extern int Hear_again(void);
extern boolean eating_glob(struct obj *);
extern void reset_eat(void);
extern unsigned obj_nutrition(struct obj *) NONNULLARG1;
extern int doeat(void);
extern int use_tin_opener(struct obj *) NONNULLARG1;
extern void gethungry(void);
extern void morehungry(int);
extern void lesshungry(int);
extern boolean is_fainted(void);
extern void reset_faint(void);
extern int corpse_intrinsic(struct permonst *) NONNULLARG1;
extern void violated_vegetarian(void);
extern void newuhs(boolean);
extern struct obj *floorfood(const char *, int) NONNULLARG1;
extern void vomit(void);
extern int eaten_stat(int, struct obj *) NONNULLARG2;
extern void food_disappears(struct obj *) NONNULLARG1;
extern void food_substitution(struct obj *, struct obj *) NONNULLPTRS;
extern long temp_resist(int);
extern boolean eating_dangerous_corpse(int);
extern void eating_conducts(struct permonst *) NONNULLARG1;
extern int eat_brains(struct monst *, struct monst *, boolean,
                                                          int *) NONNULLARG12;
extern void fix_petrification(void);
extern int intrinsic_possible(int, struct permonst *) NONNULLARG2;
extern boolean should_givit(int, struct permonst *) NONNULLARG2;
extern void consume_oeaten(struct obj *, int) NONNULLARG1;
extern boolean maybe_finished_meal(boolean);
extern void cant_finish_meal(struct obj *) NONNULLARG1;
extern void set_tin_variety(struct obj *, int) NONNULLARG1;
extern int tin_variety_txt(char *, int *);
extern void tin_details(struct obj *, int, char *);
extern boolean Popeye(int);
extern int Finish_digestion(void);

/* ### end.c ### */

extern void done1(int);
extern int done2(void);
extern void done_in_by(struct monst *, int) NONNULLARG1;
extern void done_object_cleanup(void);
extern void NH_abort(const char *);
#endif /* !MAKEDEFS_C && MDLIB_C && !CPPREGEX_C */
#if !defined(CPPREGEX_C)
ATTRNORETURN extern void panic(const char *, ...) PRINTF_F(1, 2) NORETURN;
#endif
#if !defined(MAKEDEFS_C) && !defined(MDLIB_C) && !defined(CPPREGEX_C)
extern void done(int);
extern void container_contents(struct obj *, boolean, boolean, boolean);
ATTRNORETURN extern void nh_terminate(int) NORETURN;
extern void delayed_killer(int, int, const char *);
extern struct kinfo *find_delayed_killer(int);
extern void dealloc_killer(struct kinfo *);
extern void save_killers(NHFILE *) NONNULLARG1;
extern void restore_killers(NHFILE *) NONNULLARG1;
extern char *build_english_list(char *) NONNULLARG1;

/* ### engrave.c ### */

extern char *random_engraving(char *, char *) NONNULLARG12;
extern void wipeout_text(char *, int, unsigned) NONNULLARG1;
extern boolean can_reach_floor(boolean);
extern void cant_reach_floor(coordxy, coordxy, boolean, boolean, boolean);
extern struct engr *engr_at(coordxy, coordxy);
extern struct engr *sengr_at(const char *, coordxy, coordxy, boolean) NONNULLARG1;
extern void u_wipe_engr(int);
extern void wipe_engr_at(coordxy, coordxy, xint16, boolean);
extern void read_engr_at(coordxy, coordxy);
extern void make_engr_at(coordxy, coordxy, const char *, const char *, long, int) NONNULLARG3;
extern void del_engr_at(coordxy, coordxy);
extern int freehand(void);
extern int doengrave(void);
extern void sanitize_engravings(void);
extern void forget_engravings(void);
extern void engraving_sanity_check(void);
extern void save_engravings(NHFILE *) NONNULLARG1;
extern void rest_engravings(NHFILE *) NONNULLARG1;
extern void engr_stats(const char *, char *, long *, long *) NONNULLPTRS;
extern void del_engr(struct engr *) NONNULLARG1;
extern void rloc_engr(struct engr *) NONNULLARG1;
extern void make_grave(coordxy, coordxy, const char *);
extern void disturb_grave(coordxy, coordxy);
extern void see_engraving(struct engr *) NONNULLARG1;
extern void feel_engraving(struct engr *) NONNULLARG1;
extern boolean engr_can_be_felt(struct engr *) NONNULLARG1;

/* ### exper.c ### */

extern long newuexp(int);
extern int newpw(void);
extern int experience(struct monst *, int) NONNULLARG1;
extern void more_experienced(int, int);
extern void losexp(const char *);
extern void newexplevel(void);
extern void pluslvl(boolean);
extern long rndexp(boolean);

/* ### explode.c ### */

extern void explode(coordxy, coordxy, int, int, char, int);
extern long scatter(coordxy, coordxy, int, unsigned int, struct obj *);
extern void splatter_burning_oil(coordxy, coordxy, boolean);
extern void explode_oil(struct obj *, coordxy, coordxy) NONNULLARG1;
extern int adtyp_to_expltype(const int);
extern void mon_explodes(struct monst *, struct attack *) NONNULLPTRS;

/* ### extralev.c ### */

extern void makeroguerooms(void);
extern void corr(coordxy, coordxy);
extern void makerogueghost(void);

/* ### files.c ### */

extern const char *nh_basename(const char *, boolean) NONNULLARG1;
extern char *fname_encode(const char *, char,
                          char *, char *, int) NONNULLPTRS;
extern char *fname_decode(char, char *, char *, int) NONNULLPTRS;
extern const char *fqname(const char *, int, int);
extern FILE *fopen_datafile(const char *, const char *, int) NONNULLPTRS;
extern void init_nhfile(NHFILE *) NONNULLARG1;
extern void close_nhfile(NHFILE *) NONNULLARG1;
extern void rewind_nhfile(NHFILE *) NONNULLARG1;
extern void set_levelfile_name(char *, int) NONNULLARG1;
extern NHFILE *create_levelfile(int, char *);
extern NHFILE *open_levelfile(int, char *);
extern void delete_levelfile(int);
extern void clearlocks(void);
extern NHFILE *create_bonesfile(d_level *, char **, char *) NONNULLARG12;
extern void commit_bonesfile(d_level *) NONNULLARG1;
extern NHFILE *open_bonesfile(d_level *, char **) NONNULLPTRS;
extern int delete_bonesfile(d_level *) NONNULLARG1;
extern void compress_bonesfile(void);
extern void set_savefile_name(boolean);
#ifdef INSURANCE
extern void save_savefile_name(NHFILE *) NONNULLARG1;
#endif
#ifndef MICRO
extern void set_error_savefile(void);
#endif
extern NHFILE *create_savefile(void);
extern NHFILE *open_savefile(void);
extern int delete_savefile(void);
extern NHFILE *get_freeing_nhfile(void);
extern NHFILE *restore_saved_game(void);
extern int check_panic_save(void);
#ifdef SELECTSAVED
extern char *plname_from_file(const char *, boolean, int) NONNULLARG1;
#endif
extern char **get_saved_games(void);
extern void free_saved_games(char **);
extern void nh_compress(const char *);
extern void nh_uncompress(const char *);
extern void nh_sfconvert(const char *);
extern void nh_sfunconvert(const char *);
extern int delete_convertedfile(const char *);
extern void free_convert_filenames(void);
extern boolean lock_file(const char *, int, int) NONNULLARG1;
extern void unlock_file(const char *) NONNULLARG1;
extern void check_recordfile(const char *);
extern void read_wizkit(void);
extern int read_sym_file(int);
extern void paniclog(const char *, const char *) NONNULLPTRS;
extern void testinglog(const char *, const char *, const char *);
extern int validate_prefix_locations(char *);
#ifdef SELF_RECOVER
extern boolean recover_savefile(void);
extern void assure_syscf_file(void);
#endif
#ifdef SYSCF_FILE
extern void assure_syscf_file(void);
#endif
extern int nhclose(int);
#ifdef DEBUG
extern boolean debugcore(const char *, boolean);
#endif
extern void reveal_paths(int);
extern boolean read_tribute(const char *, const char *, int, char *, int,
                            unsigned);
extern boolean Death_quote(char *, int) NONNULLARG1;
extern void livelog_add(long ll_type, const char *) NONNULLARG2;
ATTRNORETURN extern void do_deferred_showpaths(int) NORETURN;
extern boolean contains_directory(const char *);
extern void get_nhuuid(void);
extern void free_nhuuid(void);

/* ### fountain.c ### */

extern void floating_above(const char *) NONNULLARG1;
extern void dogushforth(int);
extern void dryup(coordxy, coordxy, boolean);
extern void drinkfountain(void);
extern void dipfountain(struct obj *) NONNULLARG1;
extern int wash_hands(void);
extern void breaksink(coordxy, coordxy);
extern void drinksink(void);
extern void dipsink(struct obj *) NONNULLARG1;
extern void sink_backs_up(coordxy, coordxy);

/* ### getpos.c ### */

extern char *dxdy_to_dist_descr(coordxy, coordxy, boolean);
extern char *coord_desc(coordxy, coordxy, char *, char) NONNULLARG3;
extern void auto_describe(coordxy, coordxy);
extern boolean getpos_menu(coord *, int) NONNULLARG1;
extern int getpos(coord *, boolean, const char *) NONNULLARG1;
extern void getpos_sethilite(void(*f)(boolean), boolean(*d)(coordxy,coordxy));
extern boolean mapxy_valid(coordxy, coordxy);
extern boolean gather_locs_interesting(coordxy, coordxy, int);

/* ### glyphs.c ### */

extern int glyphrep_to_custom_map_entries(const char *op,
                                          int *glyph) NONNULLPTRS;
extern int add_custom_urep_entry(const char *symset_name, int glyphidx,
                                 uint32 utf32ch, const uint8 *utf8str,
                                 enum graphics_sets which_set) NONNULLARG1;
extern int add_custom_nhcolor_entry(const char *customization_name,
                                    int glyphidx, uint32 nhcolor,
                                    enum graphics_sets which_set) NONNULLARG1;
struct customization_detail *find_matching_customization(
                                             const char *customization_name,
                                             enum customization_types custtype,
                                             enum graphics_sets which_set);
int set_map_customcolor(glyph_map *gm, uint32 nhcolor) NONNULLARG1;
extern int unicode_val(const char *);
extern int glyphrep(const char *) NONNULLARG1;
extern int match_glyph(char *) NONNULLARG1;
extern void dump_all_glyphnames(FILE *fp) NONNULLARG1;
extern void wizcustom_glyphnames(winid win);
extern void populate_glyphname_hash_indices(void);
extern void empty_glyphname_hash_indices(void);
extern boolean glyphname_hash_indices_loaded(void);
extern void apply_customizations(enum graphics_sets which_set,
                                 enum do_customizations docustomize);
extern void purge_custom_entries(enum graphics_sets which_set);
extern void purge_all_custom_entries(void);
extern void dump_glyphnames(void);
extern void clear_all_glyphmap_colors(void);
extern void reset_customcolors(void);
extern int glyph_to_cmap(int);
extern void maybe_shuffle_customizations(void);

/* ### hack.c ### */

/**
 * @brief Whether the hero could travel to a square.
 * @note About reachability rather than the square itself: a valid destination is one a route exists to, so an ordinary floor square behind a wall is not one.
 */
/**
 * @brief 영웅이 어떤 칸으로 여행할 수 있을지.
 * @note 그 칸 자체가 아니라 도달 가능성에 관한 것이다. 유효한 목적지는 경로가 존재하는 곳이므로, 벽 뒤의 평범한 바닥 칸은 그것이 아니다.
 */
extern boolean is_valid_travelpt(coordxy, coordxy);
/**
 * @name Wrapping a value as a generic one
 * @brief Put a value into the generic type the display interface takes.
 * @warning Each returns a pointer to a shared static value, not a new one. A second call overwrites the first, so two of these cannot be live at once -- which matters because a menu is
 *          built one entry at a time and each entry's value must be copied before the next is made.
 * @{
 */
/**
 * @name 값을 범용 값으로 감싸기
 * @brief 값을 표시부 인터페이스가 받는 범용 타입에 넣는다.
 * @warning 각각이 새 값이 아니라 공유된 정적 값을 가리키는 포인터를 반환한다. 두 번째 호출이 첫 것을 덮어쓰므로, 이들 중 둘이 동시에 살아 있을 수 없다. 메뉴가 항목 하나씩 만들어지고 각 항목의 값이 다음 것이 만들어지기 전에 복사되어야 하므로 그것이 중요하다.
 * @{
 */
extern anything *uint_to_any(unsigned);
extern anything *long_to_any(long);
extern anything *monst_to_any(struct monst *) NONNULLARG1;
extern anything *obj_to_any(struct obj *) NONNULLARG1;
/** @} */
extern boolean revive_nasty(coordxy, coordxy, const char *);
/**
 * @brief Continue gnawing through rock, and say whether it is still going.
 * @note The multi-turn form of digging as a monster does it. Its name is the state it reports: the hero is still chewing, so the command has not finished and should not be replaced.
 */
/**
 * @brief 암반을 계속 갉고, 그것이 아직 진행 중인지 알린다.
 * @note 몬스터가 하는 방식의 굴착의 여러 턴짜리 형태다. 그 이름이 그것이 알리는 상태다. 영웅이 아직 갉고 있으므로, 그 명령은 끝나지 않았고 대체되어서는 안 된다.
 */
extern int still_chewing(coordxy, coordxy);
extern void movobj(struct obj *, coordxy, coordxy);
/**
 * @name What the terrain permits
 * @brief Whether a square may be dug through, or passed through as though it were not there.
 * @note About the square rather than the digger: a wall may be undiggable by anything, which is how a level protects part of itself. Whether the hero has the means is a separate
 *       question.
 * @{
 */
/**
 * @name 지형이 무엇을 허용하는지
 * @brief 어떤 칸을 파고 지나갈 수 있는지, 또는 그것이 없는 것처럼 통과할 수 있는지.
 * @note 파는 자가 아니라 그 칸에 관한 것이다. 벽은 무엇으로도 팔 수 없을 수 있고, 그것이 레벨이 자기 일부를 보호하는 방식이다. 영웅에게 그 수단이 있는지는 별개의 질문이다.
 * @{
 */
extern boolean may_dig(coordxy, coordxy);
extern boolean may_passwall(coordxy, coordxy);
/** @} */
/**
 * @brief Whether a square is rock this kind of monster cannot handle.
 * @note Takes the kind rather than the monster, so it can be asked about a form the hero might take. A monster that tunnels or passes walls is not stopped by rock, so this is not simply
 *       "is it rock".
 */
/**
 * @brief 어떤 칸이 이 종류의 몬스터가 다룰 수 없는 암반인지.
 * @note 몬스터가 아니라 종류를 받으므로, 영웅이 취할 수 있는 형태에 대해 물을 수 있다. 굴을 파거나 벽을 통과하는 몬스터는 암반에 막히지 않으므로, 이것은 단순히 "그것이 암반인가"가 아니다.
 */
extern boolean bad_rock(struct permonst *, coordxy, coordxy) NONNULLARG1;
/**
 * @brief Whether a monster is too encumbered or too large to squeeze through a tight gap.
 * @return how much is in the way, not merely whether -- so a caller can say what to put down
 */
/**
 * @brief 몬스터가 좁은 틈을 비집고 지나가기에 너무 짐이 많거나 너무 큰지.
 * @return 여부만이 아니라 무엇이 얼마나 가로막는지. 그래서 호출자가 무엇을 내려놓아야 할지 말할 수 있다
 */
extern int cant_squeeze_thru(struct monst *) NONNULLARG1;
/**
 * @brief Whether a square is the one where the invocation must be performed.
 * @note One specific square on one specific level, so this is effectively a comparison against a recorded position rather than a property of the terrain.
 */
/**
 * @brief 어떤 칸이 발동 의식을 수행해야 하는 그 칸인지.
 * @note 특정 레벨의 특정 칸 하나이므로, 이것은 사실상 지형의 속성이 아니라 기록된 위치와의 비교다.
 */
extern boolean invocation_pos(coordxy, coordxy);
/**
 * @brief Whether the hero may move between two squares, and set up what happens if they do.
 *
 * The gate every hero move passes through. It considers the terrain, what is in the way, whether a door must be opened, whether the hero must squeeze -- and its flag argument says
 * whether it is being asked speculatively or is about to be acted on.
 *
 * @warning Not a pure test in every mode. Asked as part of an actual move it may open a door and may print a message, so it cannot be used freely to probe the map. The flags are what
 *          separate the two uses, and their meanings are documented with them rather than here.
 * @note This is the routine the real-time work in this fork has to reckon with: it decides whether a step is allowed, and the continuous position has to accept its answer rather than
 *       overriding it.
 */
/**
 * @brief 영웅이 두 칸 사이를 움직여도 되는지, 그리고 움직인다면 무슨 일이 일어날지 준비한다.
 *
 * 영웅의 모든 이동이 지나는 관문이다. 지형, 무엇이 가로막는지, 문을 열어야 하는지, 영웅이 비집고 지나가야 하는지를 고려하며, 그 플래그 인자가 추측으로 물어지는 것인지 곧 실행될 것인지를 말한다.
 *
 * @warning 모든 모드에서 순수한 검사는 아니다. 실제 이동의 일부로 물어지면 문을 열 수 있고 메시지를 인쇄할 수 있다. 그래서 지도를 탐색하는 데 자유롭게 쓸 수 없다. 그 플래그가 두 용도를 나누는 것이며, 그 뜻은 여기가 아니라 그것들과 함께 기록되어 있다.
 * @note 이 포크의 실시간 작업이 셈해야 하는 루틴이 이것이다. 걸음이 허용되는지를 정하며, 연속 위치는 그것을 무시하는 대신 그 답을 받아들여야 한다.
 */
extern boolean test_move(coordxy, coordxy, coordxy, coordxy, int);
#ifdef DEBUG
extern int wiz_debug_cmd_traveldisplay(void);
#endif
/**
 * @brief Whether the hero is stuck in place and cannot move at all.
 * @note Distinct from being held: a rooted hero is anchored by their own form or the ground rather than by something gripping them, so escaping is a different matter and the message is
 *       different.
 */
/**
 * @brief 영웅이 제자리에 박혀 전혀 움직일 수 없는지.
 * @note 붙잡힌 것과 구별된다. 박힌 영웅은 무언가가 쥐고 있어서가 아니라 자기 형태나 땅에 의해 고정되어 있으므로, 벗어나는 것이 다른 문제이고 메시지도 다르다.
 */
extern boolean u_rooted(void);
/**
 * @name Announcing a monster to a screen reader
 * @brief Say aloud that a monster has been noticed, and catch up on any that were missed.
 * @note Only does anything when the accessibility setting asks for it. A player watching the map sees a monster appear; a player listening has to be told, and these are what tell them.
 * @note The catch-up form exists because announcing can be suspended while the game has something else to say. Suspending does not lose the notices, so they have to be delivered
 *       afterwards.
 * @{
 */
/**
 * @name 화면 읽기 프로그램에 몬스터를 알리기
 * @brief 몬스터가 알아채졌음을 소리로 말하고, 놓친 것들을 따라잡는다.
 * @note 접근성 설정이 요청할 때만 무언가를 한다. 지도를 보는 플레이어는 몬스터가 나타나는 것을 본다. 듣는 플레이어는 들어야 하며, 이들이 그것을 알려 주는 것이다.
 * @note 따라잡기 형태가 있는 것은, 게임이 다른 할 말이 있는 동안 알리기를 멈춰 둘 수 있기 때문이다. 멈추는 것이 알림을 잃는 것은 아니므로, 그것들이 나중에 전달되어야 한다.
 * @{
 */
extern void notice_mon(struct monst *) NONNULLARG1;
extern void notice_all_mons(boolean);
/** @} */
extern void impact_disturbs_zombies(struct obj *, boolean) NONNULLARG1;
extern void disturb_buried_zombies(coordxy, coordxy);
/**
 * @brief Whether the hero might be misperceiving things.
 * @note "Might" is deliberate: it covers confusion, hallucination, stunning and blindness together, because most callers only need to know whether to trust what the hero thinks rather
 *       than which impairment applies.
 */
/**
 * @brief 영웅이 무언가를 잘못 지각하고 있을 수 있는지.
 * @note "있을 수 있는지"는 의도적이다. 혼란, 환각, 기절, 실명을 함께 덮는다. 대부분의 호출자가 어떤 손상인지가 아니라 영웅이 여기는 것을 믿어도 되는지만 알아야 하기 때문이다.
 */
extern boolean u_maybe_impaired(void);
/**
 * @brief A verb for how the hero is getting about, since it is not always walking.
 * @param  the verb to fall back on when nothing more specific applies
 * @note Exists so a message can say "you float" or "you slither" without every message having to consider the hero's form. The caller supplies the ordinary word and gets whichever one
 *       is true.
 */
/**
 * @brief 영웅이 어떻게 돌아다니고 있는지에 대한 동사. 항상 걷는 것은 아니기 때문이다.
 * @param  더 구체적인 것이 적용되지 않을 때 돌아갈 동사
 * @note 메시지가 모든 메시지마다 영웅의 형태를 고려하지 않고도 "당신은 떠간다"나 "당신은 기어간다"라고 말할 수 있도록 존재한다. 호출자가 평범한 낱말을 제공하고 참인 것을 받는다.
 */
extern const char *u_locomotion(const char *) NONNULLARG1;
/**
 * @brief Offer a hint if it has not been offered before, and record that it has.
 * @return whether anything was said
 * @note The recording is the point. A hint that appeared every time the situation arose would be noise, so each is shown once and the fact is kept with the character.
 */
/**
 * @brief 아직 제시되지 않았다면 조언을 제시하고, 제시되었음을 기록한다.
 * @return 무언가가 말해졌는지
 * @note 그 기록이 요점이다. 그 상황이 생길 때마다 나오는 조언은 소음이 되므로, 각각은 한 번 보여지고 그 사실이 캐릭터와 함께 보관된다.
 */
extern boolean handle_tip(int);
/**
 * @brief Carry out the hero's move for this turn.
 * @note The main entry to hero movement, and it does far more than change a position: it resolves what the move means -- an attack, an attempt to open something, a swap with a pet -- and
 *       applies everything that follows from arriving somewhere.
 */
/**
 * @brief 이번 턴의 영웅의 이동을 수행한다.
 * @note 영웅 이동의 주 입구이며, 위치를 바꾸는 것보다 훨씬 많은 일을 한다. 그 이동이 무엇을 뜻하는지 -- 공격, 무언가를 열려는 시도, 애완동물과의 자리 바꿈 -- 를 해석하고, 어딘가에 도착하는 것에서 따라 나오는 모든 것을 적용한다.
 */
extern void domove(void);
/**
 * @brief Pause between steps of a run, according to what the player asked for.
 * @note The pause is a display setting rather than a game one -- how visible running should be -- so this is where that preference is honoured, and it is why one run mode is slower than
 *       drawing every step.
 */
/**
 * @brief 달리기의 걸음 사이에 멈춘다. 플레이어가 요청한 것에 따라.
 * @note 그 멈춤은 게임 설정이 아니라 표시 설정이다. 달리기가 얼마나 보여야 하는지. 그래서 이곳이 그 선호가 받아들여지는 곳이며, 어느 달리기 모드가 매 걸음을 그리는 것보다 느린 이유다.
 */
extern void runmode_delay_output(void);
/**
 * @name Overexertion
 * @brief Whether the hero has strained themselves, and the harm if so.
 * @note Two routines because the question and the consequence are asked separately: a caller may need to know whether the hero can afford an action before performing it.
 * @{
 */
/**
 * @name 과로
 * @brief 영웅이 자신을 무리하게 했는지, 그리고 그렇다면 그 피해.
 * @note 두 루틴인 것은 질문과 결과가 따로 물어지기 때문이다. 호출자는 어떤 행동을 수행하기 전에 영웅이 그것을 감당할 수 있는지 알아야 할 수 있다.
 * @{
 */
extern void overexert_hp(void);
extern boolean overexertion(void);
/** @} */
extern void invocation_message(void);
extern void classify_terrain(void);
extern void switch_terrain(void);
extern void set_uinwater(int);
extern boolean pooleffects(boolean);
extern void spoteffects(boolean);
/**
 * @brief Which rooms a square belongs to.
 * @return a string of room identifiers, since a square may be in more than one
 * @warning Returns a pointer into a shared buffer, and it is a string rather than a single room -- because rooms may contain rooms, and a square in a shop inside a temple is in both.
 */
/**
 * @brief 어떤 칸이 어느 방들에 속하는지.
 * @return 방 식별자의 문자열. 칸이 둘 넘는 방에 있을 수 있다
 * @warning 공유 버퍼를 가리키는 포인터를 반환하며, 하나의 방이 아니라 문자열이다. 방이 방을 품을 수 있고, 신전 안의 상점에 있는 칸은 둘 다에 있기 때문이다.
 */
extern char *in_rooms(coordxy, coordxy, int);
extern boolean in_town(coordxy, coordxy);
/**
 * @brief React to the hero having entered or left a room with a purpose.
 * @note Called after the move rather than before, so it reacts to arrival: a shopkeeper greets, a temple's atmosphere is described, a zoo wakes up. That is why entering a shop and being
 *       inside one are handled in different places.
 */
/**
 * @brief 영웅이 용도가 있는 방에 들어서거나 떠난 것에 반응한다.
 * @note 움직이기 전이 아니라 뒤에 호출되므로 도착에 반응한다. 상점 주인이 인사하고, 신전의 기운이 기술되고, 동물원이 깨어난다. 그것이 상점에 들어서는 것과 그 안에 있는 것이 다른 곳에서 다뤄지는 이유다.
 */
extern void check_special_room(boolean);
extern int dopickup(void);
/**
 * @brief Decide whether a run should stop here, and which way it should turn.
 * @note What makes running feel deliberate. It stops at anything worth stopping at -- a doorway, a fork, something in view -- and follows a corridor around corners, so a run is not a
 *       straight line but a route.
 */
/**
 * @brief 달리기가 여기서 멈춰야 하는지, 그리고 어느 쪽으로 돌아야 하는지 정한다.
 * @note 달리기를 의도적인 것처럼 느껴지게 만드는 것이다. 멈출 만한 것 -- 문간, 갈림길, 시야에 든 무엇 -- 에서 멈추고, 통로를 따라 모서리를 돌아간다. 그래서 달리기는 직선이 아니라 경로다.
 */
extern void lookaround(void);
/**
 * @brief Whether a square is a doorway with no door in it.
 * @note A distinction that matters because a doorway restricts movement -- no diagonal passage -- whether or not a door is present. So an empty doorway is not simply floor.
 */
/**
 * @brief 어떤 칸이 문이 없는 문간인지.
 * @note 문이 있든 없든 문간이 이동을 제한하므로 -- 대각선 통과 불가 -- 중요한 구별이다. 그래서 빈 문간은 그냥 바닥이 아니다.
 */
extern boolean doorless_door(coordxy, coordxy);
/**
 * @brief Whether a square would do as somewhere to crawl to after being killed and saved.
 * @note For the moment after life-saving: the hero must end up somewhere survivable, which may not be where they fell. So this is a search criterion rather than a movement test.
 */
/**
 * @brief 죽었다가 구조된 뒤 기어갈 곳으로 어떤 칸이 쓸모 있을지.
 * @note 목숨을 구한 직후의 순간을 위한 것이다. 영웅은 살아남을 수 있는 곳에 있게 되어야 하고, 그곳이 쓰러진 자리가 아닐 수 있다. 그래서 이것은 이동 검사가 아니라 탐색 기준이다.
 */
extern boolean crawl_destination(coordxy, coordxy);
/**
 * @brief Whether there is a monster close enough to interrupt what the hero is doing.
 * @note The reason a multi-turn action stops when something approaches. What counts as near enough and as worth noticing are both decided here, so a peaceful monster wandering past does
 *       not interrupt a meal.
 */
/**
 * @brief 영웅이 하는 일을 중단시킬 만큼 가까운 몬스터가 있는지.
 * @note 무언가가 다가올 때 여러 턴짜리 행동이 멈추는 이유다. 무엇이 충분히 가까운지와 무엇이 알아챌 만한지가 둘 다 여기서 정해지므로, 지나가는 평화로운 몬스터가 식사를 중단시키지는 않는다.
 */
extern int monster_nearby(void);
extern void end_running(boolean);
/**
 * @name Being unable to act for a while
 * @brief Put the hero out of action for a number of turns, and bring them back.
 * @note The count is what stops the hero acting, and it is negative while it lasts -- which is why so much code tests it for being less than zero rather than non-zero.
 * @note Ending it takes a message because coming round is worth reporting, and the message differs by what caused the interruption rather than by what ends it.
 * @{
 */
/**
 * @name 한동안 행동할 수 없기
 * @brief 영웅을 몇 턴 동안 행동 불가로 두고, 되돌린다.
 * @note 그 계수가 영웅이 행동하지 못하게 하는 것이며, 지속되는 동안 음수다. 그래서 아주 많은 코드가 그것이 0이 아닌지가 아니라 0보다 작은지를 검사한다.
 * @note 끝내는 쪽이 메시지를 받는 것은 정신을 차리는 것이 알릴 가치가 있기 때문이다. 그 메시지는 무엇이 그것을 끝내는지가 아니라 무엇이 그 중단을 일으켰는지에 따라 다르다.
 * @{
 */
extern void nomul(int);
extern void unmul(const char *);
/** @} */
extern void showdamage(int);
extern void losehp(int, const char *, schar) ;
/**
 * @name How much the hero can carry
 * @brief The carrying capacity, what is being carried, and how burdened that makes them.
 *
 * Three different numbers and it is worth knowing which is which. The capacity is what the hero could carry unburdened. The weight is what they are actually carrying. The burden level is
 * the consequence, and it is a step on a scale rather than a ratio -- which is why it is computed rather than derived by division at each use.
 *
 * @warning The weight of the pack may be negative in one circumstance: a container that reduces what it holds can make its contents weigh less than nothing in the arithmetic. Code that
 *          assumes a non-negative weight will be surprised.
 * @note One form asks what the burden would be with a given amount added, which is how "you would be overloaded" is answered before picking something up.
 * @{
 */
/**
 * @name 영웅이 얼마나 나를 수 있는지
 * @brief 나를 수 있는 양, 나르고 있는 것, 그리고 그것이 얼마나 짐이 되는지.
 *
 * 세 개의 다른 숫자이며 어느 것이 어느 것인지 알아 둘 가치가 있다. 용량은 영웅이 짐 없이 나를 수 있는 것이다. 무게는 실제로 나르고 있는 것이다. 부담 단계는 그 결과이며, 비율이 아니라 척도 위의 한 단계다. 그래서 쓰일 때마다 나눗셈으로 유도되는 대신 계산된다.
 *
 * @warning 가방의 무게는 한 상황에서 음수일 수 있다. 담은 것을 줄이는 용기가 산술상 그 내용물이 아무것도보다 덜 나가게 만들 수 있다. 무게가 음이 아니라고 가정하는 코드는 놀라게 된다.
 * @note 한 형태는 주어진 양을 더했을 때의 부담이 얼마일지를 묻는다. 그것이 무언가를 집기 전에 "당신은 짐에 짓눌릴 것이다"에 답하는 방식이다.
 * @{
 */
extern int weight_cap(void);
extern int inv_weight(void);
extern int near_capacity(void);
extern int calc_capacity(int);
extern int max_capacity(void);
extern boolean check_capacity(const char *);
/** @} */
extern void dump_weights(void);
extern int inv_cnt(boolean);
/**
 * @brief How much money a chain of objects amounts to.
 * @warning Accepts null on purpose, as the accompanying comment records: it is often called on the hero's pack, which may be empty. So a null argument is a valid question with the answer
 *          zero, and the annotation records that this was examined rather than overlooked.
 */
/**
 * @brief 물건 사슬이 얼마의 돈에 해당하는지.
 * @warning 딸린 주석이 기록하듯 의도적으로 널을 받아들인다. 영웅의 가방에 대해 자주 호출되고, 그것이 비어 있을 수 있다. 그래서 널 인자는 답이 0인 유효한 질문이며, 그 표시는 이것이 간과된 것이 아니라 검토되었음을 기록한다.
 */
/* sometimes money_cnt(gi.invent) which can be null */
extern long money_cnt(struct obj *) NO_NNARGS;
/**
 * @brief Verify and repair what is recorded about a square.
 * @note A consistency pass rather than an operation on the square. It is called where the game has done something that could have left the square's record disagreeing with what is
 *       actually there -- so it exists to catch the game's own mistakes.
 */
/**
 * @brief 어떤 칸에 대해 기록된 것을 검증하고 고친다.
 * @note 그 칸에 대한 연산이 아니라 일관성 점검이다. 게임이 그 칸의 기록을 실제로 거기 있는 것과 어긋나게 남겼을 수 있는 일을 한 곳에서 호출된다. 그래서 게임 자신의 잘못을 잡기 위해 존재한다.
 */
extern void spot_checks(coordxy, coordxy, schar);
/**
 * @brief Divide, rounding to nearest rather than toward zero.
 * @note Named because the game's rules mean it: a value halved should not systematically shrink, which plain division would cause over many applications.
 */
/**
 * @brief 나눈다. 0 쪽으로가 아니라 가장 가까운 쪽으로 반올림하여.
 * @note 게임의 규칙이 그것을 뜻하기 때문에 이름이 붙었다. 절반이 되는 값이 체계적으로 줄어들어서는 안 되는데, 평범한 나눗셈은 여러 번 적용되면 그렇게 만든다.
 */
extern int rounddiv(long, int);

/* ### strutil.c ### */

extern void strbuf_init(strbuf_t *) NONNULLARG1;
extern void strbuf_append(strbuf_t *, const char *) NONNULLPTRS;
extern void strbuf_reserve(strbuf_t *, int) NONNULLARG1;
extern void strbuf_empty(strbuf_t *) NONNULLARG1;
extern void strbuf_nl_to_crlf(strbuf_t *) NONNULLARG1;
extern unsigned Strlen_(const char *, const char *, int) NONNULLPTRS;
extern boolean pmatch(const char *, const char *) NONNULLPTRS;
extern boolean pmatchi(const char *, const char *) NONNULLPTRS;
/*
extern boolean pmatchz(const char *, const char *) NONNULLPTRS;
*/

/* ### iactions.c ### */

extern int itemactions(struct obj *otmp) NONNULLARG1;

/* ### insight.c ### */

extern int doattributes(void);
extern void enlightenment(int, int);
extern void youhiding(boolean, int);
extern char *trap_predicament(char *, int, boolean) NONNULLARG1;
extern int doconduct(void);
extern void show_conduct(int);
extern void record_achievement(schar);
extern boolean remove_achievement(schar);
extern int count_achievements(void);
extern schar achieve_rank(int);
extern boolean sokoban_in_play(void);
extern int do_gamelog(void);
extern void show_gamelog(int);
extern int set_vanq_order(boolean);
extern int dovanquished(void);
extern int doborn(void);
extern void list_vanquished(char, boolean);
extern int num_genocides(void);
extern void list_genocided(char, boolean);
extern int dogenocided(void);
extern const char *align_str(aligntyp);
extern char *piousness(boolean, const char *);
extern void mstatusline(struct monst *) NONNULLARG1;
extern void ustatusline(void);

/* ### invent.c ### */

extern void loot_classify(Loot *, struct obj *) NONNULLPTRS;
/**
 * @name Sorting a list of objects for display
 * @brief Produce a sorted view of a chain of objects, and release it afterwards.
 * @warning The result is a separate array that must be released, and the pair is not optional -- the sorted view does not own the objects but does own itself. The release form takes a pointer to
 *          the pointer so it can be nulled, which is how a caller cannot release it twice.
 * @note It sorts a view rather than the chain. The objects' own order is untouched, which matters because the pack's order is the hero's and the player may have arranged it.
 * @{
 */
/**
 * @name 표시를 위해 물건 목록을 정렬하기
 * @brief 물건 사슬의 정렬된 뷰를 만들고, 나중에 그것을 놓아준다.
 * @warning 결과는 놓아주어야 하는 별개의 배열이며, 그 짝은 선택이 아니다. 정렬된 뷰는 그 물건들을 소유하지 않지만 자기 자신은 소유한다. 놓아주는 형태는 널로 만들 수 있도록 포인터에 대한 포인터를 받으며, 그것이 호출자가 그것을 두 번 놓아줄 수 없게 하는 방식이다.
 * @note 사슬이 아니라 뷰를 정렬한다. 물건들 자신의 순서는 건드려지지 않으며, 가방의 순서가 영웅의 것이고 플레이어가 그것을 정돈했을 수 있으므로 그것이 중요하다.
 * @{
 */
extern Loot *sortloot(struct obj **, unsigned, boolean,
                      boolean(*)(struct obj *)) NONNULLARG1;
extern void unsortloot(Loot **) NONNULLARG1;
/** @} */
/**
 * @brief Give an object a letter in the hero's pack.
 * @note The letters are a scarce resource -- fifty-two of them -- and this is where the overflow character is handed out when they run out. So an object is not guaranteed a letter of its own.
 */
/**
 * @brief 물건에 영웅의 가방에서의 글자를 부여한다.
 * @note 그 글자들은 희소한 자원이다. 쉰두 개. 그리고 그것들이 다했을 때 넘침 문자가 주어지는 곳이 여기다. 그래서 물건이 자기 글자를 보장받지는 않는다.
 */
extern void assigninvlet(struct obj *) NONNULLARG1;
/**
 * @name Combining identical objects
 * @brief Find what an object could merge with, and merge it.
 *
 * Two routines because the question and the act are needed separately: a caller may need to know where an object would go before deciding to put it there.
 *
 * @warning The merging form destroys one of the two objects and takes pointers to the pointers so both callers' references can be corrected. After it succeeds, one of the two pointers refers to
 *          freed memory unless it was updated -- which is precisely why the arguments have that shape.
 * @note Merging is not only about saving space. Two objects that merge become indistinguishable, so a merge can lose information the player had -- which is why the rules about what may merge are
 *       strict and are documented with the object's own fields.
 * @{
 */
/**
 * @name 똑같은 물건 합치기
 * @brief 물건이 무엇과 합쳐질 수 있는지 찾고, 그것을 합친다.
 *
 * 두 루틴인 것은 질문과 행위가 따로 필요하기 때문이다. 호출자는 물건을 어디에 둘지 정하기 전에 그것이 어디로 갈지 알아야 할 수 있다.
 *
 * @warning 합치는 형태는 두 물건 중 하나를 파괴하며, 두 호출자의 참조가 모두 바로잡힐 수 있도록 포인터에 대한 포인터를 받는다. 그것이 성공한 뒤에는, 갱신되지 않았다면 두 포인터 중 하나가 해제된 메모리를 가리킨다. 그것이 바로 인자가 그런 모양인 이유다.
 * @note 합치기는 공간을 아끼는 것에 관한 것만이 아니다. 합쳐진 두 물건은 구별할 수 없게 되므로, 합치기가 플레이어가 가졌던 정보를 잃을 수 있다. 그래서 무엇이 합쳐져도 되는지에 관한 규칙이 엄격하고 물건 자신의 필드와 함께 기록되어 있다.
 * @{
 */
extern struct obj *merge_choice(struct obj *, struct obj *) NONNULLARG2;
extern int merged(struct obj **, struct obj **) NONNULLPTRS;
/** @} */
/**
 * @name Putting an object into the pack
 * @brief The several forms of adding to inventory, and the two halves each of them performs.
 *
 * The plain form is what almost everything should use. It may merge the object with one already held, which means the object handed in may cease to exist -- so it returns what is now in the pack,
 * and using the original pointer afterwards is the mistake this shape is meant to prevent.
 *
 * The other forms exist for the cases where merging or ordering must be controlled: one adds at a given place in the pack, one refuses to merge at all.
 *
 * The two numbered halves are the internals. They are separate because adding has bookkeeping that must happen before the object is linked in and bookkeeping that must happen after, and a few
 * callers need to do something in between.
 *
 * @warning Every form may free the object passed in. The return value is the surviving object and is not optional to use.
 * @{
 */
/**
 * @name 물건을 가방에 넣기
 * @brief 소지품에 더하는 여러 형태와, 각각이 수행하는 두 절반.
 *
 * 평범한 형태가 거의 모든 것이 써야 하는 것이다. 그것은 그 물건을 이미 지닌 것과 합칠 수 있으며, 그것은 건네진 물건이 존재하기를 그만둘 수 있다는 뜻이다. 그래서 지금 가방에 있는 것을 반환하며, 그 뒤로 원래 포인터를 쓰는 것이 이 모양이 막으려는 잘못이다.
 *
 * 다른 형태들은 합치기나 순서가 통제되어야 하는 경우를 위해 존재한다. 하나는 가방의 주어진 자리에 더하고, 하나는 합치기를 아예 거부한다.
 *
 * 번호가 붙은 두 절반은 내부다. 따로 있는 것은, 더하기에 물건이 이어 붙여지기 전에 일어나야 하는 기록과 그 뒤에 일어나야 하는 기록이 있고, 몇몇 호출자가 그 사이에 무언가를 해야 하기 때문이다.
 *
 * @warning 모든 형태가 건네진 물건을 해제할 수 있다. 반환값이 살아남은 물건이며 그것을 쓰는 것은 선택이 아니다.
 * @{
 */
extern void addinv_core1(struct obj *) NONNULLARG1;
extern void addinv_core2(struct obj *) NONNULLARG1;
extern struct obj *addinv(struct obj *) NONNULLARG1;
extern struct obj *addinv_before(struct obj *, struct obj *) NONNULLARG1;
extern struct obj *addinv_nomerge(struct obj *) NONNULLARG1;
/** @} */
/**
 * @brief Add an object to the pack and tell the player, dealing with not being able to carry it.
 * @note The form to use when the hero is acquiring something during play. It handles the whole situation: the object may be too heavy, may not fit, may go into a container, may need a different
 *       message -- and the three text arguments are those messages, supplied by the caller because only the caller knows how the object was acquired.
 */
/**
 * @brief 물건을 가방에 더하고 플레이어에게 알리며, 그것을 나를 수 없는 경우를 처리한다.
 * @note 영웅이 플레이 중에 무언가를 얻을 때 써야 할 형태다. 그 상황 전체를 다룬다. 물건이 너무 무거울 수도, 들어가지 않을 수도, 용기 안으로 갈 수도, 다른 메시지가 필요할 수도 있다. 그리고 세 개의 글 인자가 그 메시지들이며, 호출자만이 그 물건이 어떻게 얻어졌는지 알기
 *       때문에 호출자가 제공한다.
 */
extern struct obj *hold_another_object(struct obj *, const char *,
                                       const char *, const char *) NONNULLARG1;
/* nhlua.c calls useupall(gi.invent), but checks gi.invent against NULL
 * before doing so. useupall() won't handle NULL*/
extern void useupall(struct obj *) NONNULLARG1;
/**
 * @brief Consume one of a stack, or the whole object if only one remains.
 * @warning May free the object. Its distinction from consuming the whole stack is exactly that it usually does not, so a caller cannot know from the call site whether its pointer is still good.
 */
/**
 * @brief 묶음에서 하나를 소비하거나, 하나만 남았으면 그 물건 전체를 소비한다.
 * @warning 그 물건을 해제할 수 있다. 묶음 전체를 소비하는 것과의 구별이 바로 그것이 보통 그러지 않는다는 것이므로, 호출자는 호출 지점에서 자기 포인터가 여전히 유효한지 알 수 없다.
 */
extern void useup(struct obj *) NONNULLARG1;
/**
 * @brief Spend one of a charged object's charges.
 * @note Separate from using up the object because the two are different resources: a wand may be out of charges and still exist, which is a state the player can discover and act on.
 */
/**
 * @brief 충전된 물건의 충전 하나를 쓴다.
 * @note 물건을 소비하는 것과 따로 있는 것은, 그 둘이 다른 자원이기 때문이다. 지팡이는 충전이 다했으면서도 존재할 수 있고, 그것은 플레이어가 발견하고 그에 따라 행동할 수 있는 상태다.
 */
extern void consume_obj_charge(struct obj *, boolean) NONNULLARG1;
/**
 * @name Taking an object out of the pack
 * @brief Remove an object from inventory without destroying it, and the internal half of doing so.
 * @note The distinction from deleting is the point: an object taken out this way still exists and has to go somewhere -- onto the floor, into a container, into a monster's possession. Failing to
 *       place it afterwards leaks it.
 * @{
 */
/**
 * @name 물건을 가방에서 꺼내기
 * @brief 물건을 파괴하지 않고 소지품에서 꺼내고, 그렇게 하는 것의 내부 절반.
 * @note 삭제와의 구별이 요점이다. 이 방식으로 꺼내진 물건은 여전히 존재하고 어딘가로 가야 한다. 바닥으로, 용기 안으로, 몬스터의 소유로. 그 뒤에 그것을 놓지 못하면 누수된다.
 * @{
 */
extern void freeinv_core(struct obj *) NONNULLARG1;
extern void freeinv(struct obj *) NONNULLARG1;
/** @} */
extern void delallobj(coordxy, coordxy);
/**
 * @name Destroying an object
 * @brief Remove an object from the game entirely.
 * @warning Frees the object and unlinks it from wherever it was. Anything holding a pointer to it -- an in-progress action, a shop bill, a container's contents -- has to have been dealt with
 *          first, and nothing here checks that.
 * @note The core form's boolean controls whether the object's contents go with it, which is why destroying a container is not simply destroying an object.
 * @{
 */
/**
 * @name 물건을 파괴하기
 * @brief 물건을 게임에서 완전히 없애기.
 * @warning 그 물건을 해제하고 그것이 있던 곳에서 떼어 낸다. 그것을 가리키는 포인터를 쥔 무엇이든 -- 진행 중인 행동, 상점 계산서, 용기의 내용물 -- 먼저 처리되어 있어야 하며, 여기의 어느 것도 그것을 검사하지 않는다.
 * @note 핵심 형태의 논리값은 그 물건의 내용물이 함께 갈지를 통제한다. 그래서 용기를 파괴하는 것이 단순히 물건 하나를 파괴하는 것이 아니다.
 * @{
 */
extern void delobj(struct obj *) NONNULLARG1;
extern void delobj_core(struct obj *, boolean) NONNULLARG1;
/** @} */
/**
 * @name Finding an object of a kind
 * @brief Search a square, or the hero's pack, for an object of a given kind.
 * @note The square form returns the first such object and the continuation form finds the next, which is how a caller walks all of them without holding the list itself -- necessary because
 *       examining an object may remove it.
 * @note Neither searches inside containers. That is deliberate: a boulder inside a bag is not a boulder in the way, and a caller wanting the contents has to ask for them.
 * @{
 */
/**
 * @name 어떤 종류의 물건 찾기
 * @brief 어떤 칸이나 영웅의 가방에서 주어진 종류의 물건을 찾는다.
 * @note 칸 형태는 그런 물건 중 첫 번째를 반환하고 이어가기 형태가 다음 것을 찾는다. 그것이 호출자가 목록 자체를 쥐지 않고 그 전부를 도는 방식이다. 물건을 살피는 것이 그것을 없앨 수 있으므로 필요하다.
 * @note 어느 쪽도 용기 안을 찾지 않는다. 의도적이다. 가방 안의 바위는 길을 막는 바위가 아니며, 내용물을 원하는 호출자는 그것을 따로 요청해야 한다.
 * @{
 */
extern struct obj *sobj_at(int, coordxy, coordxy);
extern struct obj *nxtobj(struct obj *, int, boolean) NONNULLARG1;
extern struct obj *carrying(int);
/** @} */
extern struct obj *u_carried_gloves(void);
extern struct obj *u_have_novel(void);
extern struct obj *o_on(unsigned int, struct obj *);
extern boolean obj_here(struct obj *, coordxy, coordxy) NONNULLARG1;
extern boolean wearing_armor(void);
/**
 * @name Whether an object is being used
 * @brief Whether it is worn, and the wider question of whether it is in use at all.
 * @note The wider one includes a wielded weapon, a lit lamp, an attached leash -- things that are not worn but must not simply be dropped. So "in use" is the question most callers actually want,
 *       and "worn" is the narrower one about armour and accessories.
 * @{
 */
/**
 * @name 물건이 쓰이고 있는지
 * @brief 착용되어 있는지, 그리고 아예 쓰이고 있는지에 대한 더 넓은 질문.
 * @note 더 넓은 것은 들고 있는 무기, 켜진 등불, 매인 목줄을 포함한다. 착용된 것은 아니지만 그냥 버려져서는 안 되는 것들. 그래서 "쓰이는 중"이 대부분의 호출자가 실제로 원하는 질문이고, "착용됨"은 갑옷과 장신구에 관한 더 좁은 것이다.
 * @{
 */
extern boolean is_worn(struct obj *) NONNULLARG1;
extern boolean is_inuse(struct obj *) NONNULLARG1;
/** @} */
extern struct obj *g_at(coordxy, coordxy);
extern boolean splittable(struct obj *) NONNULLARG1;
extern int any_obj_ok(struct obj *);
/**
 * @name Asking the player to choose an object
 * @brief The three ways the game asks which object a command should act on.
 *
 * They differ in how many and from where. The single form asks for one object and takes a routine deciding which are acceptable, so a command's own notion of a valid target is expressed as a
 * function rather than as a class list. The counted form asks for several by class. The chain form walks a list asking about each.
 *
 * @note That the acceptability test is a function is what allows a command to accept, say, any weapon the hero could actually throw -- a condition no class list could express.
 * @warning The chain form's annotation is one of the special cases the file's own header explains: its two required arguments are not adjacent, and the header observes that reordering its
 *          parameters would let that annotation be retired.
 * @{
 */
/**
 * @name 플레이어에게 물건을 고르게 하기
 * @brief 게임이 어떤 명령이 어느 물건에 대해 작동해야 하는지 묻는 세 방식.
 *
 * 몇 개를 어디서 묻는지에서 다르다. 단일 형태는 하나의 물건을 요청하고 어느 것이 받아들여지는지 정하는 루틴을 받는다. 그래서 명령 자신의 유효한 대상 개념이 계열 목록이 아니라 함수로 표현된다. 개수 형태는 계열별로 여럿을 요청한다. 사슬 형태는 목록을 돌며 각각에 대해 묻는다.
 *
 * @note 받아들임 검사가 함수라는 것이, 명령이 예컨대 영웅이 실제로 던질 수 있는 아무 무기든 받아들일 수 있게 하는 것이다. 어떤 계열 목록도 표현할 수 없는 조건이다.
 * @warning 사슬 형태의 표시는 이 파일 자신의 머리말이 설명하는 특별한 경우 중 하나다. 그 두 필수 인자가 인접하지 않으며, 그 머리말은 그 매개변수 순서를 바꾸면 그 표시를 없앨 수 있으리라고 지적한다.
 * @{
 */
extern struct obj *getobj(const char *, int(*)(struct obj *), unsigned int);
extern int ggetobj(const char *, int(*)(struct obj *), int, boolean,
                   unsigned *) NONNULLARG1;
extern int askchain(struct obj **, const char *, int, int(*)(struct obj *),
                    int(*)(struct obj *), int, const char *) NONNULLARG17;
/** @} */
/**
 * @brief Record that the hero now knows whether a container is locked and what is in it.
 * @note Two separate pieces of knowledge, set together because they are learned together -- opening a container reveals both. They are held apart because a container can be known to be locked
 *       without its contents being known.
 */
/**
 * @brief 영웅이 이제 용기가 잠겨 있는지와 그 안에 무엇이 있는지 안다고 기록한다.
 * @note 두 개의 별개 앎이며, 함께 배워지기 때문에 함께 설정된다. 용기를 여는 것이 둘 다를 드러낸다. 따로 보관되는 것은, 용기가 잠겨 있다고 알려지면서 그 내용물은 알려지지 않을 수 있기 때문이다.
 */
extern void set_cknown_lknown(struct obj *) NONNULLARG1;
/**
 * @name Identifying an object
 * @brief Make the hero know what an object is, in the full sense or by asking.
 * @note The full form makes everything about the object known at once, including its enchantment and its blessing. The asking form is the player-facing one: it puts up the prompt, so it belongs to
 *       a command rather than to an effect.
 * @note Identification is per kind and not per object, so identifying one potion of healing identifies them all. That is why these are so consequential relative to how little they appear to do.
 * @{
 */
/**
 * @name 물건을 감별하기
 * @brief 영웅이 물건이 무엇인지 알게 만든다. 온전한 의미로, 또는 물어서.
 * @note 온전한 형태는 그 물건에 관한 모든 것을 한꺼번에 알려진 것으로 만든다. 강화와 축복까지 포함해서. 묻는 형태는 플레이어를 향한 것이다. 프롬프트를 띄우므로 효과가 아니라 명령에 속한다.
 * @note 감별은 물건별이 아니라 종류별이므로, 치유의 물약 하나를 감별하면 그 전부가 감별된다. 그것이 이들이 하는 일이 적어 보이는 데 비해 그토록 결과가 큰 이유다.
 * @{
 */
extern void fully_identify_obj(struct obj *) NONNULLARG1;
extern int identify(struct obj *) NONNULLARG1;
/** @} */
/**
 * @brief How many objects in a chain the hero does not fully know.
 * @warning Accepts null, and the annotation records that this was examined -- the pack may be empty, and an empty pack has zero unidentified objects rather than being an error.
 */
/**
 * @brief 사슬 안의 물건 중 영웅이 온전히 알지 못하는 것이 몇 개인지.
 * @warning 널을 받아들이며, 그 표시가 이것이 검토되었음을 기록한다. 가방이 비어 있을 수 있고, 빈 가방은 오류가 아니라 미확인 물건이 0개인 것이다.
 */
extern int count_unidentified(struct obj *) NO_NNARGS;
extern void identify_pack(int, boolean);
extern void learn_unseen_invent(void);
extern void update_inventory(void);
extern int doperminv(void);
extern void prinv(const char *, struct obj *, long) NONNULLARG2;
extern char *xprname(struct obj *, const char *, char, boolean, long, long);
extern int ddoinv(void);
extern char display_inventory(const char *, boolean);
extern int display_binventory(coordxy, coordxy, boolean);
extern struct obj *display_cinventory(struct obj *) NONNULLARG1;
extern struct obj *display_minventory(struct monst *, int, char *) NONNULLARG1;
extern int dotypeinv(void);
extern const char *dfeature_at(coordxy, coordxy, char *) NONNULLARG3;
extern int look_here(int, unsigned);
extern int dolook(void);
/**
 * @name Touching something petrifying
 * @brief Whether handling an object would turn the hero to stone, and doing so.
 * @note The asking form exists so a command can refuse before it happens. That matters here more than usual: petrification is not damage but death, so a command that discovered it afterwards would
 *       be a command that killed the hero for looking.
 * @{
 */
/**
 * @name 석화시키는 것을 만지기
 * @brief 물건을 다루는 것이 영웅을 돌로 만들지, 그리고 그렇게 하기.
 * @note 묻는 형태가 있는 것은 명령이 그 일이 일어나기 전에 거부할 수 있도록 하기 위함이다. 여기서는 그것이 평소보다 더 중요하다. 석화는 피해가 아니라 죽음이므로, 나중에 그것을 발견하는 명령은 영웅을 살펴본 대가로 죽이는 명령이 된다.
 * @{
 */
extern boolean will_feel_cockatrice(struct obj *, boolean) NONNULLARG1;
extern void feel_cockatrice(struct obj *, boolean) NONNULLARG1;
/** @} */
/**
 * @brief Merge an object with whatever it belongs with on the floor.
 * @warning May free the object, and unlike the pack's merging it returns nothing -- so a caller must not use its pointer afterwards at all, rather than reading back a survivor.
 */
/**
 * @brief 물건을 바닥에서 그것이 속한 것과 합친다.
 * @warning 그 물건을 해제할 수 있으며, 가방의 합치기와 달리 아무것도 반환하지 않는다. 그래서 호출자는 살아남은 것을 되읽는 대신 그 뒤로 그 포인터를 전혀 쓰지 않아야 한다.
 */
extern void stackobj(struct obj *) NONNULLARG1;
/**
 * @brief Whether two objects are alike enough to become one.
 * @note Strict on purpose. Two objects that merge become indistinguishable, so anything the player might know about one and not the other has to prevent it -- which is why identical-looking
 *       objects sometimes refuse to stack.
 */
/**
 * @brief 두 물건이 하나가 될 만큼 비슷한지.
 * @note 일부러 엄격하다. 합쳐진 두 물건은 구별할 수 없게 되므로, 플레이어가 한쪽에 대해 알고 다른 쪽에 대해 알지 못할 만한 것이 있으면 그것을 막아야 한다. 그것이 똑같아 보이는 물건이 때때로 쌓이기를 거부하는 이유다.
 */
extern boolean mergable(struct obj *, struct obj *) NONNULLPTRS;
extern int doprgold(void);
extern int doprwep(void);
extern int doprarm(void);
extern int doprring(void);
extern int dopramulet(void);
extern int doprtool(void);
extern int doprinuse(void);
extern void useupf(struct obj *, long) NONNULLARG1;
extern char *let_to_name(char, boolean, boolean);
extern void free_invbuf(void);
extern void reassign(void);
extern boolean check_invent_gold(const char *) NONNULLARG1;
extern int doorganize(void);
extern int adjust_split(void);
extern void free_pickinv_cache(void);
/* sometimes count_unpaid(gi.invent) which can be null */
extern int count_unpaid(struct obj *) NO_NNARGS;
extern int count_buc(struct obj *, int, boolean(*)(struct obj *));
extern void tally_BUCX(struct obj *, boolean, int *, int *, int *, int *,
                       int *, int *);
extern long count_contents(struct obj *, boolean,
                           boolean, boolean, boolean) NONNULLARG1;
extern void carry_obj_effects(struct obj *) NONNULLARG1;
extern const char *currency(long);
extern void silly_thing(const char *, struct obj *) NONNULLARG1;
extern void sync_perminvent(void);
extern void perm_invent_toggled(boolean negated);
extern void prepare_perminvent(winid window);
extern struct obj *carrying_stoning_corpse(void);
extern void repopulate_perminvent(void);
extern int check_for_puzzling_nonmerge(struct obj *);

/* ### ioctl.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void getwindowsz(void);
extern void getioctls(void);
extern void setioctls(void);
#ifdef SUSPEND
extern int dosuspend(void);
#endif /* SUSPEND */
#endif /* UNIX || __BEOS__ */

/* ### light.c ### */

extern void new_light_source(coordxy, coordxy,
                             int, int, union any *) NONNULLPTRS;
extern void del_light_source(int, union any *) NONNULLARG2;
extern void do_light_sources(seenV **) NONNULLARG1;
extern void show_transient_light(struct obj *, coordxy, coordxy);
extern void transient_light_cleanup(void);
extern struct monst *find_mid(unsigned, unsigned);
extern void save_light_sources(NHFILE *, int);
extern void restore_light_sources(NHFILE *) NONNULLARG1;
extern void light_stats(const char *, char *, long *, long *) NONNULLPTRS;
extern void relink_light_sources(boolean);
extern void light_sources_sanity_check(void);
extern void obj_move_light_source(struct obj *, struct obj *) NONNULLARG12;
extern boolean any_light_source(void);
extern void snuff_light_source(coordxy, coordxy);
extern boolean obj_sheds_light(struct obj *) NONNULLARG1;
extern boolean obj_is_burning(struct obj *) NONNULLARG1;
extern void obj_split_light_source(struct obj *, struct obj *) NONNULLARG12;
extern void obj_merge_light_sources(struct obj *, struct obj *) NONNULLARG12;
extern void obj_adjust_light_radius(struct obj *, int) NONNULLARG1;
extern int candle_light_range(struct obj *) NONNULLARG1;
extern int arti_light_radius(struct obj *) NONNULLARG1;
extern const char *arti_light_description(struct obj *) NONNULLARG1;
extern int wiz_light_sources(void);

/* ### lock.c ### */

extern boolean picking_lock(coordxy *, coordxy *);
extern boolean picking_at(coordxy, coordxy);
extern void breakchestlock(struct obj *, boolean) NONNULLARG1;
extern void reset_pick(void);
extern void maybe_reset_pick(struct obj *);
extern struct obj *autokey(boolean);
extern int pick_lock(struct obj *, coordxy, coordxy, struct obj *);
extern boolean u_have_forceable_weapon(void);
extern int doforce(void);
extern boolean boxlock(struct obj *, struct obj *) NONNULLARG12;
extern boolean doorlock(struct obj *, coordxy, coordxy) NONNULLARG1;
extern int doopen(void);
extern boolean stumble_on_door_mimic(coordxy, coordxy);
extern int doopen_indir(coordxy, coordxy);
extern int doclose(void);

#ifdef MAC68K
/* outdated functions removed */
/* ### macfile.c ### */
/* ### macmain.c ### */
/* ### macunix.c ### */
/* ### macwin.c ### */
/* ### mttymain.c ### */
#endif

/* ### mail.c ### */

#ifdef MAIL
#ifdef UNIX
extern void free_maildata(void);
extern void getmailstatus(void);
extern void ck_server_admin_msg(void);
#endif
extern void ckmailstatus(void);
extern void readmail(struct obj *);
#endif /* MAIL */

/* ### makemon.c ### */

extern boolean is_home_elemental(struct permonst *) NONNULLARG1;
extern struct monst *clone_mon(struct monst *, coordxy, coordxy) NONNULLARG1;
extern int monhp_per_lvl(struct monst *) NONNULLARG1;
extern void newmonhp(struct monst *, int) NONNULLARG1;
extern struct mextra *newmextra(void) NONNULL;
extern struct monst *makemon(struct permonst *, coordxy, coordxy, mmflags_nht);
extern struct monst *unmakemon(struct monst *, mmflags_nht) NONNULLARG1;
extern boolean create_critters(int, struct permonst *, boolean);
extern struct permonst *rndmonst_adj(int, int);
extern struct permonst *rndmonst(void);
extern struct permonst *mkclass(char, int);
extern struct permonst *mkclass_aligned(char, int, aligntyp);
extern int mkclass_poly(int);
extern int adj_lev(struct permonst *) NONNULLARG1;
extern struct permonst *grow_up(struct monst *, struct monst *) NONNULLARG1;
extern struct obj* mongets(struct monst *, int) NONNULLARG1;
extern int golemhp(int);
extern boolean peace_minded(struct permonst *) NONNULLARG1;
extern void set_malign(struct monst *) NONNULLARG1;
extern void newmcorpsenm(struct monst *) NONNULLARG1;
extern void freemcorpsenm(struct monst *) NONNULLARG1;
extern void set_mimic_sym(struct monst *) NO_NNARGS; /* tests for NULL mtmp */
extern int mbirth_limit(int);
extern void mkmonmoney(struct monst *, long) NONNULLARG1;
extern int bagotricks(struct obj *, boolean, int *);
extern boolean propagate(int, boolean, boolean);
extern void summon_furies(int);
extern void dump_mongen(void);

/* ### mcastu.c ### */

extern int castmu(struct monst *, struct attack *,
                  boolean, boolean) NONNULLARG12;
extern void touch_of_death(struct monst *) NONNULLARG1;
extern char *death_inflicted_by(char *, const char *,
                                struct monst *) NONNULLARG12;
extern int buzzmu(struct monst *, struct attack *) NONNULLARG12;

/* ### mdlib.c ### */

extern void runtime_info_init(void);
extern const char *do_runtime_info(int *) NO_NNARGS;
extern void release_runtime_info(void);
extern char *mdlib_version_string(char *, const char *) NONNULL NONNULLPTRS;

/* ### mhitm.c ### */

extern int fightm(struct monst *) NONNULLARG1;
extern int mdisplacem(struct monst *, struct monst *, boolean);
extern int mattackm(struct monst *, struct monst *);
extern boolean failed_grab(struct monst *, struct monst *,
                           struct attack *) NONNULLPTRS;
extern boolean engulf_target(struct monst *, struct monst *) NONNULLARG12;
extern int mon_poly(struct monst *, struct monst *, int) NONNULLARG12;
extern void paralyze_monst(struct monst *, int) NONNULLARG1;
extern int sleep_monst(struct monst *, int, int) NONNULLARG1;
extern void slept_monst(struct monst *) NONNULLARG1;
extern void xdrainenergym(struct monst *, boolean) NONNULLARG1;
extern long attk_protection(int);
extern void rustm(struct monst *, struct obj *);

/* ### mhitu.c ### */

extern void hitmsg(struct monst *, struct attack *) NONNULLARG12;
extern const char *mswings_verb(struct obj *, boolean) NONNULLARG1;
extern const char *mpoisons_subj(struct monst *, struct attack *) NONNULLARG12;
extern void u_slow_down(void);
extern struct monst *cloneu(void);
extern void expels(struct monst *, struct permonst *, boolean) NONNULLARG12;
extern struct attack *getmattk(struct monst *, struct monst *, int, int *,
                               struct attack *) NONNULLARG12;
extern boolean mtrapped_in_pit(struct monst *) NONNULLARG1;
extern int mattacku(struct monst *) NONNULLARG1;
boolean diseasemu(struct permonst *) NONNULLARG1;
boolean u_slip_free(struct monst *, struct attack *) NONNULLARG12;
extern int magic_negation(struct monst *) NONNULLARG1;
extern boolean gulp_blnd_check(void);
extern int gazemu(struct monst *, struct attack *) NONNULLARG12;
extern void mdamageu(struct monst *, int) NONNULLARG1;
extern int could_seduce(struct monst *, struct monst *, struct attack *) NONNULLARG12;
extern int doseduce(struct monst *) NONNULLARG1;
extern boolean mon_avoiding_this_attack(struct monst *, int) NONNULLARG1;
/* extern boolean ranged_attk_assessed(struct monst *mtmp,
                             boolean (*assessfunct)(struct monst *, int)) NONNULLARG1;
*/
extern boolean ranged_attk_available(struct monst *mtmp) NONNULLARG1;

/* ### minion.c ### */

extern void newemin(struct monst *) NONNULLARG1;
extern void free_emin(struct monst *) NONNULLARG1;
extern int monster_census(boolean);
extern int msummon(struct monst *);
extern void summon_minion(aligntyp, boolean);
extern int demon_talk(struct monst *) NONNULLARG1;
extern long bribe(struct monst *, const char *) NONNULLARG12;
extern int dprince(aligntyp);
extern int dlord(aligntyp);
extern int llord(void);
extern int ndemon(aligntyp);
extern int lminion(void);
extern void lose_guardian_angel(struct monst *);
extern void gain_guardian_angel(void);

/* ### mklev.c ### */

extern void sort_rooms(void);
extern void add_room(coordxy, coordxy, coordxy, coordxy,
                     boolean, schar, boolean);
extern void add_subroom(struct mkroom *,
                        coordxy, coordxy, coordxy, coordxy,
                        boolean, schar, boolean) NONNULLARG1;
extern void free_luathemes(enum lua_theme_group);
extern void makecorridors(void);
extern void add_door(coordxy, coordxy, struct mkroom *) NONNULLARG3;
extern void count_level_features(void);
extern void clear_level_structures(void);
extern void level_finalize_topology(void);
extern void mklev(void);
#ifdef SPECIALIZATION
extern void topologize(struct mkroom *, boolean) NONNULLARG1;
#else
extern void topologize(struct mkroom *) NONNULLARG1;
#endif
/* place_branch() has tests for NULL branch arg, preventing NONNULLARG1 */
extern void place_branch(branch *, coordxy, coordxy) NO_NNARGS;
extern boolean occupied(coordxy, coordxy);
extern int okdoor(coordxy, coordxy);
extern boolean maybe_sdoor(int);
extern void dodoor(coordxy, coordxy, struct mkroom *) NONNULLARG3;
extern void mktrap(int, unsigned, struct mkroom *, coord *) NO_NNARGS;
extern void mkstairs(coordxy, coordxy, char, struct mkroom *, boolean);
extern void mkinvokearea(void);
extern void mineralize(int, int, int, int, boolean);

/* ### mkmap.c ### */

extern void flood_fill_rm(coordxy, coordxy, int, boolean, boolean);
extern void remove_rooms(coordxy, coordxy, coordxy, coordxy);
extern boolean litstate_rnd(int);

/* ### mkmaze.c ### */

extern boolean set_levltyp(coordxy, coordxy, schar);
extern boolean set_levltyp_lit(coordxy, coordxy, schar, schar);
extern void create_maze(int, int, boolean);
extern void wallification(coordxy, coordxy, coordxy, coordxy);
extern void fix_wall_spines(coordxy, coordxy, coordxy, coordxy);
extern void walkfrom(coordxy, coordxy, schar);
extern void pick_vibrasquare_location(void);
extern void makemaz(const char *) NONNULLARG1;
extern void mazexy(coord *) NONNULLARG1;
extern void get_level_extends(coordxy *, coordxy *, coordxy *, coordxy *) NONNULLPTRS;
extern void bound_digging(void);
extern void mkportal(coordxy, coordxy, xint16, xint16);
extern boolean bad_location(coordxy, coordxy, coordxy, coordxy, coordxy,
                            coordxy);
extern boolean is_exclusion_zone(xint16, coordxy, coordxy);
/* dungeon.c u_on_rndspot() passes NULL final arg to place_lregion() */
extern void place_lregion(coordxy, coordxy, coordxy, coordxy, coordxy,
                          coordxy, coordxy, coordxy, xint16, d_level *) NO_NNARGS;
extern void fixup_special(void);
extern void fumaroles(void);
extern void movebubbles(void);
extern void water_friction(void);
extern void save_waterlevel(NHFILE *) NONNULLARG1;
extern void restore_waterlevel(NHFILE *) NONNULLARG1;
extern void maybe_adjust_hero_bubble(void);

/* ### mkobj.c ### */

extern struct oextra *newoextra(void) NONNULL;
extern void copy_oextra(struct obj *, struct obj *);
extern void dealloc_oextra(struct obj *) NONNULLARG1;
extern void newomonst(struct obj *) NONNULLARG1;
extern void free_omonst(struct obj *) NONNULLARG1;
extern void newomid(struct obj *) NONNULLARG1;
extern void free_omid(struct obj *) NONNULLARG1;
/*
extern void newolong(struct obj *);
extern void free_olong(struct obj *);
*/
extern void new_omailcmd(struct obj *, const char *) NONNULLPTRS;
extern void free_omailcmd(struct obj *) NONNULLARG1;
extern struct obj *mkobj_at(char, coordxy, coordxy, boolean);
extern struct obj *mksobj_at(int, coordxy, coordxy, boolean, boolean);
extern struct obj *mksobj_migr_to_species(int, unsigned, boolean, boolean);
extern struct obj *mkobj(int, boolean) NONNULL;
extern int rndmonnum_adj(int, int);
extern int rndmonnum(void);
extern boolean bogon_is_pname(char);
extern struct obj *splitobj(struct obj *, long) NONNULLARG1;
extern unsigned next_ident(void);
extern struct obj *unsplitobj(struct obj *) NONNULLARG1;
extern void clear_splitobjs(void);
extern void replace_object(struct obj *, struct obj *) NONNULLARG12;
extern struct obj *unknwn_contnr_contents(struct obj *) NONNULLARG1;
extern void bill_dummy_object(struct obj *) NONNULLARG1;
extern void costly_alteration(struct obj *, int) NONNULLARG1;
extern void clear_dknown(struct obj *);
extern void unknow_object(struct obj *);
extern struct obj *mksobj(int, boolean, boolean) NONNULL;
extern boolean stone_object_type(unsigned);
extern boolean stone_furniture_type(unsigned);
extern int bcsign(struct obj *) NONNULLARG1;
extern int weight(struct obj *) NONNULLARG1;
extern struct obj *mkgold(long, coordxy, coordxy);
extern void fixup_oil(struct obj *, struct obj *) NONNULLARG1;
extern struct obj *mkcorpstat(int, struct monst *, struct permonst *,
                              coordxy, coordxy, unsigned) NONNULL;
extern int corpse_revive_type(struct obj *) NONNULLARG1;
extern struct obj *obj_attach_mid(struct obj *, unsigned);
extern struct monst *get_mtraits(struct obj *, boolean) NONNULLARG1;
extern struct obj *mk_tt_object(int, coordxy, coordxy);
extern struct obj *mk_named_object(int, struct permonst *,
                                   coordxy, coordxy,
                                   const char *) ;
extern struct obj *rnd_treefruit_at(coordxy, coordxy);
extern boolean is_treefruit(struct obj *) NONNULLARG1;
extern void set_corpsenm(struct obj *, int) NONNULLARG1;
extern long rider_revival_time(struct obj *, boolean) NONNULLARG1;
extern void start_corpse_timeout(struct obj *) NONNULLARG1;
extern void start_glob_timeout(struct obj *, long) NONNULLARG1;
extern void shrink_glob(anything *, long) NONNULLARG1;
extern void maybe_adjust_light(struct obj *, int) NONNULLARG1;
extern void bless(struct obj *) NONNULLARG1;
extern void unbless(struct obj *) NONNULLARG1;
extern void curse(struct obj *) NONNULLARG1;
extern void uncurse(struct obj *) NONNULLARG1;
extern void blessorcurse(struct obj *, int) NONNULLARG1;
extern void set_bknown(struct obj *, unsigned) NONNULLARG1;
extern boolean is_flammable(struct obj *) NONNULLARG1;
extern boolean is_rottable(struct obj *) NONNULLARG1;
extern void place_object(struct obj *, coordxy, coordxy) NONNULLARG1;
extern void recreate_pile_at(coordxy, coordxy);
extern void remove_object(struct obj *) NONNULLARG1;
extern void discard_minvent(struct monst *, boolean) NONNULLARG1;
extern void obj_extract_self(struct obj *) NONNULLARG1;
extern void extract_nobj(struct obj *, struct obj **) NONNULLARG12;
extern void extract_nexthere(struct obj *, struct obj **) NONNULLARG12;
extern int add_to_minv(struct monst *, struct obj *) NONNULLARG12;
extern struct obj *add_to_container(struct obj *, struct obj *) NONNULLARG12;
extern void add_to_migration(struct obj *) NONNULLARG1;
extern void add_to_buried(struct obj *) NONNULLARG1;
extern void container_weight(struct obj *) NONNULLARG1;
extern void dealloc_obj(struct obj *) NONNULLARG1;
extern void obj_ice_effects(coordxy, coordxy, boolean);
extern long peek_at_iced_corpse_age(struct obj *) NONNULLARG1;
extern void dobjsfree(void);
extern int hornoplenty(struct obj *, boolean, struct obj *);
extern void obj_sanity_check(void);
extern struct obj *obj_nexto(struct obj *);
extern struct obj *obj_nexto_xy(struct obj *, coordxy, coordxy, boolean) NONNULLARG1;
extern struct obj *obj_absorb(struct obj **, struct obj **);
extern struct obj *obj_meld(struct obj **, struct obj **);
extern void pudding_merge_message(struct obj *, struct obj *) NONNULLARG12;
extern struct obj *init_dummyobj(struct obj *, short, long);

/* ### mkroom.c ### */

extern void do_mkroom(int);
extern void fill_zoo(struct mkroom *) NONNULLARG1;
extern struct permonst *antholemon(void);
extern boolean nexttodoor(int, int);
extern boolean has_dnstairs(struct mkroom *) NONNULLARG1;
extern boolean has_upstairs(struct mkroom *) NONNULLARG1;
extern int somex(struct mkroom *) NONNULLARG1;
extern int somey(struct mkroom *) NONNULLARG1;
extern boolean inside_room(struct mkroom *, coordxy, coordxy) NONNULLARG1;
extern boolean somexy(struct mkroom *, coord *) NONNULLARG12;
extern boolean somexyspace(struct mkroom *, coord *) NONNULLARG12;
extern void mkundead(coord *, boolean, int) NONNULLARG1;
extern struct permonst *courtmon(void);
extern void save_rooms(NHFILE *) NONNULLARG1;
extern void rest_rooms(NHFILE *) NONNULLARG1;
extern struct mkroom *search_special(schar);
extern int cmap_to_type(int);

/* ### mon.c ### */

extern void dealloc_monst(struct monst *) NONNULLARG1;
extern void copy_mextra(struct monst *, struct monst *);
extern void dealloc_mextra(struct monst *) NONNULLARG1;
extern void mon_sanity_check(void);
extern boolean zombie_maker(struct monst *) NONNULLARG1;
extern int zombie_form(struct permonst *) NONNULLARG1;
extern int m_poisongas_ok(struct monst *) NONNULLARG1;
extern int undead_to_corpse(int);
extern int genus(int, int);
extern int pm_to_cham(int);
/**
 * @brief Deal with a monster standing in water or lava.
 * @return whether the monster is gone -- it may have drowned, burned or fled
 * @warning The monster may not exist when this returns. A caller holding a pointer to it must check the result before using the pointer again.
 */
/**
 * @brief 물이나 용암에 서 있는 몬스터를 처리한다.
 * @return 그 몬스터가 사라졌는지. 익사했거나 타 버렸거나 달아났을 수 있다
 * @warning 이것이 반환할 때 그 몬스터가 존재하지 않을 수 있다. 그것을 가리키는 포인터를 쥔 호출자는 그 포인터를 다시 쓰기 전에 결과를 확인해야 한다.
 */
extern int minliquid(struct monst *) NONNULLARG1;
/**
 * @name Letting the monsters act
 * @brief Give every monster its turn, or give one monster its turn.
 *
 * The single-monster form exists because a monster's turn is not a fixed thing: it may act more than once or not at all depending on its movement allowance, and the loop over all monsters
 * needs to hand each one its share separately.
 *
 * @note This is the machinery the real-time work in this fork changes the timing of rather than the substance. Which monster acts and what it does is decided here as before; what differs
 *       is when this is reached.
 * @warning A monster may die during its own turn, and the single-monster form reports that. Iterating over monsters while calling it therefore cannot hold a plain pointer to the next one.
 * @{
 */
/**
 * @name 몬스터들이 행동하게 하기
 * @brief 모든 몬스터에게 그 차례를 주거나, 한 몬스터에게 그 차례를 준다.
 *
 * 몬스터 하나 형태가 있는 것은, 몬스터의 차례가 고정된 것이 아니기 때문이다. 자기 이동 허용량에 따라 한 번 넘게 행동할 수도, 전혀 하지 않을 수도 있으며, 모든 몬스터를 도는 반복문이 각각에게 그 몫을 따로 건네주어야 한다.
 *
 * @note 이 포크의 실시간 작업이 내용이 아니라 시점을 바꾸는 기제가 이것이다. 어느 몬스터가 행동하고 무엇을 하는지는 전과 같이 여기서 정해진다. 다른 것은 이곳에 언제 이르는지다.
 * @warning 몬스터는 자기 차례 동안 죽을 수 있고, 몬스터 하나 형태가 그것을 알린다. 그래서 그것을 호출하며 몬스터를 순회하는 것은 다음 것에 대한 맨 포인터를 쥘 수 없다.
 * @{
 */
extern boolean movemon_singlemon(struct monst *) NONNULLARG1;
extern int movemon(void);
/** @} */
extern void meatbox(struct monst *, struct obj *) NONNULLPTRS;
extern void m_consume_obj(struct monst *, struct obj *) NONNULLPTRS;
extern int meatmetal(struct monst *) NONNULLARG1;
extern int meatobj(struct monst *) NONNULLARG1;
extern int meatcorpse(struct monst *) NONNULLARG1;
extern void mon_give_prop(struct monst *, int) NONNULLARG1;
extern void mon_givit(struct monst *, struct permonst *) NONNULLARG12;
extern void mpickgold(struct monst *) NONNULLARG1;
extern boolean mpickstuff(struct monst *) NONNULLARG1;
extern int curr_mon_load(struct monst *) NONNULLARG1;
extern int max_mon_load(struct monst *) NONNULLARG1;
extern boolean can_touch_safely(struct monst *, struct obj *) NONNULLARG12;
extern int can_carry(struct monst *, struct obj *) NONNULLARG12;
/**
 * @brief What kinds of square this monster is permitted to enter.
 * @note Derived from the monster rather than stored, and handed to the position search as one value -- so a monster's abilities are turned into movement permissions in one place instead of
 *       being reconsidered at each candidate square.
 */
/**
 * @brief 이 몬스터가 어떤 종류의 칸에 들어가도 되는지.
 * @note 저장되지 않고 몬스터에서 유도되며, 위치 탐색에 하나의 값으로 건네진다. 그래서 몬스터의 능력이 후보 칸마다 다시 검토되는 대신 한곳에서 이동 허가로 바뀐다.
 */
extern long mon_allowflags(struct monst *) NONNULLARG1;
extern boolean m_in_air(struct monst *) NONNULLARG1;
/**
 * @brief Find the squares a monster could move to from where it is.
 *
 * The heart of monster movement. It fills in the candidate squares, given what the monster is allowed to enter, and the caller then chooses among them -- so deciding where a monster may go
 * and deciding where it wants to go are deliberately separate.
 *
 * @note That separation is why every kind of monster movement, from a pet fetching an object to a covetous monster pursuing an artifact, shares one notion of what is reachable.
 * @warning The permitted squares depend on the flags passed, not only on the monster. Two calls for the same monster with different flags legitimately give different answers.
 */
/**
 * @brief 몬스터가 지금 있는 곳에서 움직일 수 있는 칸들을 찾는다.
 *
 * 몬스터 이동의 심장이다. 그 몬스터가 무엇에 들어가도 되는지를 받아 후보 칸들을 채우고, 그다음 호출자가 그 중에서 고른다. 그래서 몬스터가 어디로 갈 수 있는지 정하는 일과 어디로 가고 싶은지 정하는 일이 의도적으로 분리되어 있다.
 *
 * @note 그 분리가, 물건을 물어 오는 애완동물부터 아티팩트를 쫓는 탐욕스러운 몬스터까지 모든 종류의 몬스터 이동이 무엇에 닿을 수 있는지에 대한 하나의 개념을 공유하는 이유다.
 * @warning 허용되는 칸은 몬스터만이 아니라 넘겨진 플래그에 달려 있다. 같은 몬스터에 대해 다른 플래그로 두 번 호출하면 정당하게 다른 답이 나온다.
 */
extern int mfndpos(struct monst *, struct mfndposdata *, long) NONNULLPTRS;
/**
 * @brief Whether a monster is close enough to a square to act on it.
 * @note Adjacency as the rules mean it, which is not simply a distance: a long worm is near anywhere its body reaches, and a monster that cannot cross a diagonal gap is not near what is
 *       past it.
 */
/**
 * @brief 몬스터가 어떤 칸에 행동할 만큼 가까이 있는지.
 * @note 규칙이 뜻하는 의미의 인접함이며, 그것은 단순히 거리가 아니다. 긴 벌레는 자기 몸이 닿는 어디에든 가깝고, 대각선 틈을 건널 수 없는 몬스터는 그 너머의 것에 가깝지 않다.
 */
extern boolean monnear(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void dmonsfree(void);
extern void elemental_clog(struct monst *) NONNULLARG1;
/**
 * @brief How much movement allowance a monster gets this turn.
 *
 * The core of the speed system. A monster is not given a number of steps but an allowance, which it spends; a fast monster is given more than one step's worth and a slow one sometimes less
 * than one, so the same mechanism produces both "acts twice" and "acts every other turn".
 *
 * @note Its boolean argument distinguishes actually granting the allowance from asking what it would be, because the speed is randomised and asking twice would give two answers.
 * @note This is where the real-time work in this fork touches the speed system: the allowance is unchanged, and what changed is that turns arrive on a clock rather than on the hero's
 *       input. The rounding that makes a monster's leftover allowance discard is guarded out in the real-time build, which is what lets a monster be outrun.
 */
/**
 * @brief 몬스터가 이번 턴에 받는 이동 허용량이 얼마인지.
 *
 * 속도 체계의 핵심이다. 몬스터에게 걸음의 개수가 주어지는 것이 아니라 허용량이 주어지고, 그것을 쓴다. 빠른 몬스터는 한 걸음 분량보다 많이 받고 느린 몬스터는 때로 한 걸음보다 적게 받는다. 그래서 같은 기제가 "두 번 행동함"과 "한 턴 걸러 행동함"을 함께 만들어 낸다.
 *
 * @note 그 논리값 인자는 허용량을 실제로 부여하는 것과 그것이 얼마일지 묻는 것을 구별한다. 그 속도가 무작위화되어 있어 두 번 물으면 두 답이 나오기 때문이다.
 * @note 이 포크의 실시간 작업이 속도 체계에 닿는 곳이 여기다. 허용량은 바뀌지 않았고, 바뀐 것은 턴이 영웅의 입력이 아니라 시계로 도래한다는 점이다. 몬스터의 남은 허용량을 버리게 만드는 반올림이 실시간 빌드에서 조건으로 제외되며, 그것이 몬스터를 앞질러 달아날 수 있게 하는 것이다.
 */
extern int mcalcmove(struct monst *, boolean) NONNULLARG1;
/**
 * @brief Advance the timers and afflictions of every monster on the level.
 * @note Once per turn, and separate from letting them act -- so a monster's poison works and its wounds heal whether or not it had movement to spend.
 */
/**
 * @brief 레벨의 모든 몬스터의 타이머와 고통을 진행시킨다.
 * @note 턴마다 한 번이며, 그것들이 행동하게 하는 것과 별개다. 그래서 몬스터의 독이 퍼지고 상처가 낫는 일은 쓸 이동량이 있었는지와 무관하다.
 */
extern void mcalcdistress(void);
/**
 * @brief Put one monster in another's place in every list and on the map.
 * @note Used where a monster becomes a different monster -- a shapeshift that replaces rather than alters. It exists because a monster is referred to from several places, and all of them
 *       have to be updated together.
 */
/**
 * @brief 모든 목록과 지도에서 한 몬스터를 다른 몬스터의 자리에 놓는다.
 * @note 몬스터가 다른 몬스터가 되는 곳에서 쓰인다. 바꾸는 것이 아니라 대체하는 모습 변화. 몬스터가 여러 곳에서 참조되고 그 전부가 함께 갱신되어야 하기 때문에 존재한다.
 */
extern void replmon(struct monst *, struct monst *) NONNULLARG12;
/**
 * @brief Take a monster out of the level's list without destroying it.
 * @note The distinction from killing is the point: a monster removed this way still exists and may be placed elsewhere. That is how a monster follows the hero between levels.
 * @warning The second argument is where to put it -- a chain to move it to. Passing null there means it is simply removed, so a caller must be sure something else holds it or it is
 *          leaked.
 */
/**
 * @brief 몬스터를 파괴하지 않고 레벨의 목록에서 꺼낸다.
 * @note 죽이는 것과의 구별이 요점이다. 이 방식으로 꺼내진 몬스터는 여전히 존재하고 다른 곳에 놓일 수 있다. 그것이 몬스터가 영웅을 따라 레벨을 오가는 방식이다.
 * @warning 두 번째 인자가 그것을 어디에 둘지다. 옮겨 갈 사슬. 거기에 널을 넘기면 그냥 꺼내지기만 하므로, 호출자는 다른 무엇이 그것을 쥐고 있음을 확실히 해야 한다. 그러지 않으면 누수된다.
 */
extern void relmon(struct monst *, struct monst **) NONNULLARG1;
extern struct obj *mlifesaver(struct monst *) NONNULLARG1;
extern boolean corpse_chance(struct monst *, struct monst *, boolean) NONNULLARG1;
/**
 * @name The several ways a monster can stop existing
 * @brief Removing a monster, in the various forms the game distinguishes.
 *
 * These are not alternatives to choose freely among; each means something different and choosing the wrong one produces a monster that leaves the wrong thing behind, or gives the wrong
 * credit, or fails to be resurrectable.
 *
 * Roughly: one is the bare death, one is death with the remains that follow from it, one is vanishing without dying at all, one is being turned to stone, and two more are death credited to
 * the hero -- with the second taking flags saying what to say and what to leave.
 *
 * @note Vanishing exists because some monsters must be removed without a corpse, experience or a death message -- a summoned creature departing, or a monster the level generator withdraws.
 *       Killing such a monster instead would give the hero credit for it.
 * @warning Every one of these may free the monster. A caller must not use its pointer afterwards, and where a caller must know whether the monster survived, that is what the return values
 *          of the routines that have them are for.
 * @{
 */
/**
 * @name 몬스터가 존재하기를 그만두는 여러 방식
 * @brief 몬스터를 없애기. 게임이 구별하는 여러 형태로.
 *
 * 이들은 마음대로 골라도 되는 대안이 아니다. 각각이 다른 것을 뜻하고, 잘못된 것을 고르면 잘못된 것을 남기는 몬스터가 되거나, 잘못된 공을 돌리거나, 되살릴 수 없게 된다.
 *
 * 대략, 하나는 맨 죽음이고, 하나는 그것에서 따라 나오는 잔해와 함께의 죽음이고, 하나는 아예 죽지 않고 사라지는 것이고, 하나는 돌로 변하는 것이고, 나머지 둘은 영웅에게 공이 돌아가는 죽음이다. 두 번째 것은 무엇을 말하고 무엇을 남길지 말하는 플래그를 받는다.
 *
 * @note 사라짐이 있는 것은, 어떤 몬스터는 시체도 경험치도 사망 메시지도 없이 없애져야 하기 때문이다. 떠나는 소환된 생물, 또는 레벨 생성기가 철회하는 몬스터. 그런 몬스터를 대신 죽이면 영웅에게 그 공이 돌아간다.
 * @warning 이들 하나하나가 그 몬스터를 해제할 수 있다. 호출자는 그 뒤로 그 포인터를 써서는 안 되며, 호출자가 그 몬스터가 살아남았는지 알아야 하는 곳에서는 반환값이 있는 루틴의 그 반환값이 그것을 위한 것이다.
 * @{
 */
extern void mondead(struct monst *) NONNULLARG1;
extern void mondied(struct monst *) NONNULLARG1;
extern void mongone(struct monst *) NONNULLARG1;
extern void monstone(struct monst *) NONNULLARG1;
extern void monkilled(struct monst *, const char *, int) NONNULLARG1;
extern void killed(struct monst *) NONNULLARG1;
extern void xkilled(struct monst *, int) NONNULLARG1;
/** @} */
/**
 * @name What has hold of the hero
 * @brief Record that a monster is holding the hero, or that it no longer is.
 * @note Setting accepts null, which is how the hold is cleared without a monster to name -- so the two are not simply a pair.
 * @warning The hold is recorded on both sides: the hero remembers what has them and the monster remembers it has them. Setting one without the other leaves a hold that only one party
 *          believes in, which is why these exist rather than the fields being written directly.
 * @{
 */
/**
 * @name 무엇이 영웅을 붙잡고 있는지
 * @brief 몬스터가 영웅을 붙잡고 있음을, 또는 더는 그렇지 않음을 기록한다.
 * @note 설정하는 쪽은 널을 받아들인다. 그것이 지칭할 몬스터 없이 그 붙잡음을 지우는 방식이다. 그래서 그 둘은 단순한 짝이 아니다.
 * @warning 그 붙잡음은 양쪽에 기록된다. 영웅이 자신을 붙잡은 것을 기억하고 몬스터가 자신이 붙잡았음을 기억한다. 한쪽만 설정하면 한 당사자만 믿는 붙잡음이 남으며, 그래서 필드를 직접 쓰는 대신 이들이 존재한다.
 * @{
 */
extern void set_ustuck(struct monst *);
extern void unstuck(struct monst *) NONNULLARG1;
/** @} */
extern void mon_to_stone(struct monst *) NONNULLARG1;
/**
 * @brief Put a monster nowhere -- neither on this level nor on another.
 * @note For a monster that must be removed from play without being destroyed and without a destination. It is the fallback when there is nowhere to put something that must not simply be
 *       deleted.
 */
/**
 * @brief 몬스터를 아무 데도 두지 않는다. 이 레벨에도 다른 레벨에도.
 * @note 파괴되지 않고, 목적지도 없이 플레이에서 빠져야 하는 몬스터를 위한 것이다. 그냥 삭제되어서는 안 되는 것을 둘 곳이 없을 때의 대비책이다.
 */
extern void m_into_limbo(struct monst *) NONNULLARG1;
extern void migrate_mon(struct monst *, xint16, xint16) NONNULLARG1;
/**
 * @name Placing a monster near somewhere
 * @brief Put a monster beside the hero, or near a given square, finding a spot that will do.
 *
 * A monster cannot simply be placed: the square may be occupied, may be unsuitable for that monster, may not exist. So these search outward for somewhere acceptable, and the search is what
 * they are for rather than the placement.
 *
 * @note One form is conditional -- it places the monster only if it is not already somewhere reasonable, which is what stops a monster being shuffled every turn for no reason.
 * @warning If no acceptable square is found the monster has to go somewhere, and the overcrowding routine is what decides what happens then. A monster is not left unplaced.
 * @{
 */
/**
 * @name 몬스터를 어딘가 근처에 놓기
 * @brief 몬스터를 영웅 옆이나 주어진 칸 근처에 놓는다. 쓸모 있는 자리를 찾아서.
 *
 * 몬스터를 그냥 놓을 수는 없다. 그 칸이 차 있을 수도, 그 몬스터에게 적합하지 않을 수도, 존재하지 않을 수도 있다. 그래서 이들은 받아들일 만한 곳을 바깥으로 찾아 나가며, 그 탐색이 놓기가 아니라 이들의 목적이다.
 *
 * @note 한 형태는 조건적이다. 몬스터가 이미 합당한 곳에 있지 않을 때만 놓으며, 그것이 몬스터가 이유 없이 매 턴 옮겨지는 것을 막는다.
 * @warning 받아들일 만한 칸이 발견되지 않으면 그 몬스터는 어딘가로 가야 하며, 과밀 처리 루틴이 그때 무슨 일이 일어날지 정하는 것이다. 몬스터가 놓이지 않은 채로 남지는 않는다.
 * @{
 */
extern void mnexto(struct monst *, unsigned) NONNULLARG1;
extern void deal_with_overcrowding(struct monst *) NONNULLARG1;
extern void maybe_mnexto(struct monst *) NONNULLARG1;
extern int mnearto(struct monst *, coordxy, coordxy, boolean, unsigned) NONNULLARG1;
/** @} */
extern void m_respond(struct monst *) NONNULLARG1;
/**
 * @brief Make a peaceful monster hostile, with everything that follows from it.
 * @note Far more than setting a flag. Angering one monster may anger its fellows, may cost the hero alignment, may summon guards -- so the consequences of provoking something are gathered
 *       here rather than at each place that provokes.
 */
/**
 * @brief 평화로운 몬스터를 적대적으로 만든다. 그것에서 따라 나오는 모든 것과 함께.
 * @note 플래그를 설정하는 것보다 훨씬 많다. 한 몬스터를 화나게 하는 것이 그 동료들을 화나게 할 수도, 영웅의 진영에 값을 물릴 수도, 경비를 부를 수도 있다. 그래서 무언가를 자극하는 것의 결과가 자극하는 곳마다가 아니라 여기에 모여 있다.
 */
extern void setmangry(struct monst *, boolean) NONNULLARG1;
/**
 * @name Waking a monster
 * @brief Wake something, and say so if the hero would notice.
 *
 * Three forms, and they differ in scope rather than in effect: one monster, everything near the hero, or everything near a given square. The last exists because a noise happens somewhere and
 * not necessarily where the hero is -- a boulder falling elsewhere wakes what is near it.
 *
 * @note Saying so is separate from doing it, because a monster waking is only worth a message if the hero could tell. That is why one routine exists for the message alone.
 * @{
 */
/**
 * @name 몬스터를 깨우기
 * @brief 무언가를 깨우고, 영웅이 알아챌 만하면 그렇게 말한다.
 *
 * 세 형태이며, 효과가 아니라 범위에서 다르다. 몬스터 하나, 영웅 근처의 전부, 또는 주어진 칸 근처의 전부. 마지막 것이 있는 것은, 소리가 어딘가에서 나고 그것이 반드시 영웅이 있는 곳은 아니기 때문이다. 다른 곳에서 떨어지는 바위는 그 근처의 것을 깨운다.
 *
 * @note 그렇게 말하는 것이 그렇게 하는 것과 따로 있는 것은, 몬스터가 깨어나는 것이 영웅이 알 수 있을 때만 메시지를 낼 가치가 있기 때문이다. 그래서 메시지만을 위한 루틴이 하나 존재한다.
 * @{
 */
extern void wake_msg(struct monst *, boolean) NONNULLARG1;
extern void wakeup(struct monst *, boolean) NONNULLARG1;
extern void wake_nearby(boolean);
extern void wake_nearto(coordxy, coordxy, int);
/** @} */
extern void seemimic(struct monst *) NONNULLARG1;
extern void normal_shape(struct monst *) NONNULLARG1;
/**
 * @name Visiting every monster
 * @brief Walk the level's monsters, calling something for each, or find the first that satisfies something.
 *
 * The reason these exist rather than each caller walking the list is that the list may change while it is being walked -- a monster may die, be replaced, or be moved. Doing that safely is
 * fiddly, and doing it wrongly gives a use-after-free that only shows up rarely.
 *
 * The safe form is for exactly that case: it copies the monsters into an array first, so the callback may do anything at all to the list. The plain form is faster and requires the callback
 * not to disturb it.
 *
 * @warning Choosing the plain form for a callback that can kill a monster is the mistake these are here to prevent, and nothing detects it.
 * @note The array the safe form needs is allocated separately so that a repeated walk does not reallocate it each time.
 * @{
 */
/**
 * @name 모든 몬스터를 방문하기
 * @brief 레벨의 몬스터들을 돌며 각각에 대해 무언가를 호출하거나, 무언가를 만족시키는 첫 번째를 찾는다.
 *
 * 호출자마다 목록을 도는 대신 이들이 존재하는 이유는, 목록이 돌아지는 동안 바뀔 수 있기 때문이다. 몬스터가 죽거나 대체되거나 옮겨질 수 있다. 그것을 안전하게 하는 일은 까다롭고, 잘못하면 드물게만 드러나는 해제 후 사용이 된다.
 *
 * 안전한 형태는 바로 그 경우를 위한 것이다. 몬스터들을 먼저 배열로 복사하므로, 콜백이 그 목록에 무엇이든 해도 된다. 평범한 형태는 더 빠르고 콜백이 그것을 흐트러뜨리지 않기를 요구한다.
 *
 * @warning 몬스터를 죽일 수 있는 콜백에 평범한 형태를 고르는 것이 이들이 막기 위해 여기 있는 잘못이며, 그것을 감지하는 것은 없다.
 * @note 안전한 형태가 필요로 하는 배열은 따로 할당된다. 그래서 되풀이되는 순회가 매번 그것을 다시 할당하지 않는다.
 * @{
 */
extern void alloc_itermonarr(unsigned);
extern void iter_mons_safe(boolean (*)(struct monst *));
extern void iter_mons(void (*)(struct monst *));
extern struct monst *get_iter_mons(boolean (*)(struct monst *));
extern struct monst *get_iter_mons_xy(boolean (*)(struct monst *,
                                                  coordxy, coordxy),
                                      coordxy, coordxy);
/** @} */
extern int healmon(struct monst *, int, int) NONNULLARG1;
extern void rescham(void);
extern void restartcham(void);
extern void restore_cham(struct monst *) NONNULLARG1;
/**
 * @brief Reveal whatever was hiding at a square, if the reason it could hide has gone.
 * @note Called after the square changed -- the object it was under picked up, the ceiling dug through. A hider is not re-examined every turn, so something has to notice when its cover
 *       disappeared.
 */
/**
 * @brief 어떤 칸에 숨어 있던 것을, 숨을 수 있던 이유가 사라졌다면 드러낸다.
 * @note 그 칸이 바뀐 뒤에 호출된다. 아래에 있던 물건이 집혔거나, 천장이 파여 지나갔거나. 숨은 것이 매 턴 다시 살펴지지는 않으므로, 그 은신처가 사라졌을 때 무언가가 알아채야 한다.
 */
extern void maybe_unhide_at(coordxy, coordxy);
/**
 * @brief Have a monster hide under whatever is here, and say whether it managed to.
 * @return whether it is now hidden -- there may have been nothing to hide under
 * @note Attempts rather than tests, which is why it reports. Whether a monster can hide is a property of its kind; whether it can hide here depends on the square.
 */
/**
 * @brief 몬스터가 여기 있는 것 아래에 숨게 하고, 성공했는지 알린다.
 * @return 지금 숨어 있는지. 아래로 들어갈 것이 없었을 수 있다
 * @note 검사가 아니라 시도이며, 그래서 알린다. 몬스터가 숨을 수 있는지는 그 종류의 속성이고, 여기서 숨을 수 있는지는 그 칸에 달려 있다.
 */
extern boolean hideunder(struct monst *) NONNULLARG1;
extern void hide_monst(struct monst *) NONNULLARG1;
extern void mon_animal_list(boolean);
extern boolean valid_vampshiftform(int, int);
extern boolean validvamp(struct monst *, int *, int) NONNULLARG12;
/**
 * @brief Choose what a shapeshifter should turn into.
 * @note Separate from performing the change, because what a shapeshifter may become depends on what it is -- a vampire's options differ from a chameleon's -- and the choice may fail while the
 *       change cannot.
 */
/**
 * @brief 모습을 바꾸는 것이 무엇으로 변해야 할지 고른다.
 * @note 변화를 수행하는 것과 따로 있다. 모습을 바꾸는 것이 무엇이 될 수 있는지가 그것이 무엇인지에 달려 있고 -- 뱀파이어의 선택지는 카멜레온의 것과 다르다 -- 그 선택은 실패할 수 있는데 변화는 그럴 수 없기 때문이다.
 */
extern int select_newcham_form(struct monst *) NONNULLARG1;
extern void mgender_from_permonst(struct monst *, struct permonst *) NONNULLARG12;
/**
 * @brief Change a monster into a different kind.
 * @return whether the change happened, since a monster may resist or the form may be refused
 * @warning Alters the monster in place rather than replacing it, so a pointer to it stays valid -- but everything derived from what it was does not. Its statistics, its inventory's fit, and
 *          what it is allowed to do all change.
 * @note A null form means "choose one", which is why the second argument is not required.
 */
/**
 * @brief 몬스터를 다른 종류로 바꾼다.
 * @return 그 변화가 일어났는지. 몬스터가 저항할 수도 그 형태가 거부될 수도 있다
 * @warning 몬스터를 대체하는 것이 아니라 제자리에서 바꾸므로 그것을 가리키는 포인터는 유효한 채로 남는다. 그러나 그것이 무엇이었는지에서 유도된 모든 것은 그렇지 않다. 능력치, 소지품이 맞는지, 무엇을 해도 되는지가 모두 바뀐다.
 * @note 널 형태는 "하나를 골라라"를 뜻하며, 그래서 두 번째 인자가 필수가 아니다.
 */
extern int newcham(struct monst *, struct permonst *, unsigned) NONNULLARG1;
extern int can_be_hatched(int);
extern int egg_type_from_parent(int, boolean);
/**
 * @brief Whether a species can no longer be created.
 * @note Covers both genocide and exhaustion, since the generator does not care which -- and its boolean argument widens the question to include the species' whole class, which is what a
 *       request for "any monster of this kind" needs.
 */
/**
 * @brief 어떤 종족을 더는 만들 수 없는지.
 * @note 절멸과 소진을 함께 덮는다. 생성기는 어느 쪽인지 신경 쓰지 않는다. 그리고 그 논리값 인자가 질문을 그 종족의 계열 전체까지 넓히며, 그것이 "이 종류의 아무 몬스터"라는 요청이 필요로 하는 것이다.
 */
extern boolean dead_species(int, boolean);
/**
 * @brief Remove from the level any monster whose species has since been wiped out.
 * @note Needed because genocide does not reach monsters that already exist elsewhere. When such a level is next loaded, its inhabitants have to be reconciled with what has happened since --
 *       and a bones file's inhabitants likewise.
 */
/**
 * @brief 그 종족이 그 뒤로 절멸된 몬스터를 레벨에서 없앤다.
 * @note 절멸이 이미 다른 곳에 존재하는 몬스터에는 닿지 않기 때문에 필요하다. 그런 레벨이 다음에 적재될 때, 그 거주자들이 그 뒤로 일어난 일과 조화되어야 한다. 유골 파일의 거주자들도 마찬가지다.
 */
extern void kill_genocided_monsters(void);
extern void golemeffects(struct monst *, int, int);
extern boolean angry_guards(boolean);
extern void pacify_guards(void);
extern void decide_to_shapeshift(struct monst *) NONNULLARG1;
extern boolean vamp_stone(struct monst *) NONNULLARG1;
extern void check_gear_next_turn(struct monst *) NONNULLARG1;
extern void copy_mextra(struct monst *, struct monst *);
extern void dealloc_mextra(struct monst *);
extern boolean usmellmon(struct permonst *);
extern void mimic_hit_msg(struct monst *, short);
extern void adj_erinys(unsigned);
extern void see_monster_closeup(struct monst *, boolean) NONNULLARG1;
extern void see_nearby_monsters(void);
extern void shieldeff_mon(struct monst *) NONNULLARG1;
extern void flash_mon(struct monst *) NONNULLARG1;

/* ### mondata.c ### */

/**
 * @brief Point a monster at a different species entry.
 * @warning The lowest-level part of changing what a monster is, and it does nothing else -- no adjustment of health, inventory or position follows. Calling it directly leaves a monster whose
 *          kind and whose state disagree, which is why the shapeshifting routines exist above it.
 */
/**
 * @brief 몬스터를 다른 종족 항목으로 향하게 한다.
 * @warning 몬스터가 무엇인지를 바꾸는 가장 낮은 층의 부분이며, 그 밖에 아무것도 하지 않는다. 체력, 소지품, 위치의 조정이 따라오지 않는다. 이것을 직접 호출하면 종류와 상태가 어긋나는 몬스터가 남으며, 그래서 그 위에 모습 변화 루틴들이 존재한다.
 */
extern void set_mon_data(struct monst *, struct permonst *) NONNULLARG12;
/**
 * @name Finding an attack in a species' list
 * @brief Whether a kind of monster has a given sort of attack, and which one it is.
 * @note Two forms because both questions arise: some code needs to know whether an attack exists and some needs its damage figures. The finding form returns the attack itself, so the caller
 *       does not search again.
 * @warning A species has six attack slots and they are not ordered by importance. Neither of these assumes a position, which is why looking at the first slot directly is a mistake the
 *          could-two-weapon test in mondata.h documents having made once.
 * @{
 */
/**
 * @name 종족의 목록에서 공격 찾기
 * @brief 어떤 종류의 몬스터가 주어진 종류의 공격을 가졌는지, 그리고 그것이 어느 것인지.
 * @note 두 형태인 것은 두 질문이 다 생기기 때문이다. 어떤 코드는 공격이 존재하는지 알아야 하고 어떤 코드는 그 피해 수치를 알아야 한다. 찾는 형태는 그 공격 자체를 반환하므로 호출자가 다시 찾지 않는다.
 * @warning 종족은 여섯 개의 공격 칸을 가지며 그것들은 중요도로 정렬되어 있지 않다. 이들 중 어느 것도 위치를 가정하지 않으며, 그래서 첫 칸을 직접 보는 것이 잘못이다. mondata.h 의 두 무기 가능 검사가 그 잘못을 한 번 했음을 기록하고 있다.
 * @{
 */
extern struct attack *attacktype_fordmg(struct permonst *, int, int) NONNULLARG1;
extern boolean attacktype(struct permonst *, int) NONNULLARG1;
/** @} */
extern boolean noattacks(struct permonst *) NONNULLARG1;
extern boolean poly_when_stoned(struct permonst *) NONNULLARG1;
extern boolean defended(struct monst *, int) NONNULLARG1;
extern boolean Resists_Elem(struct monst *, int) NONNULLARG1;
extern boolean resists_drli(struct monst *) NONNULLARG1;
extern boolean resists_magm(struct monst *) NONNULLARG1;
extern boolean resists_blnd(struct monst *) NONNULLARG1;
extern boolean resists_blnd_by_arti(struct monst *) NONNULLARG1;
extern boolean can_blnd(struct monst *, struct monst *,
                        uchar, struct obj *) NONNULLARG2;
extern boolean ranged_attk(struct permonst *) NONNULLARG1;
/**
 * @name What a monster cannot bear
 * @brief Whether silver burns it, whether blessing harms it, whether light hurts it.
 *
 * Each comes in two forms, and the pairing is the thing to notice. One takes a species and one takes a monster, and they are not the same question: a species answers what such a creature is
 * normally like, and a monster answers what this one is -- which may differ because it is a lycanthrope in human form, or has been affected by something.
 *
 * @warning Choosing the species form where the monster form was meant gives an answer that is right about the kind and wrong about the individual. That is the standard mistake with these,
 *          and the names are the only warning.
 * @{
 */
/**
 * @name 몬스터가 견딜 수 없는 것
 * @brief 은이 그것을 태우는지, 축복이 해를 입히는지, 빛이 아프게 하는지.
 *
 * 각각이 두 형태로 오며, 그 짝지음이 알아챌 것이다. 하나는 종족을 받고 하나는 몬스터를 받으며, 그것들은 같은 질문이 아니다. 종족은 그런 생물이 보통 어떤지에 답하고, 몬스터는 이 개체가 어떤지에 답한다. 그것이 인간 형태의 늑인간이라거나 무언가에 영향을 받았기 때문에 다를 수
 * 있다.
 *
 * @warning 몬스터 형태가 뜻이었던 곳에서 종족 형태를 고르면, 종류에 대해서는 맞고 개체에 대해서는 틀린 답이 나온다. 이들과 관련된 전형적인 잘못이며, 그 이름들만이 경고다.
 * @{
 */
extern boolean mon_hates_silver(struct monst *) NONNULLARG1;
extern boolean hates_silver(struct permonst *) NONNULLARG1;
extern boolean mon_hates_blessings(struct monst *) NONNULLARG1;
extern boolean hates_blessings(struct permonst *) NONNULLARG1;
extern boolean mon_hates_light(struct monst *) NONNULLARG1;
/** @} */
extern boolean passes_bars(struct permonst *) NONNULLARG1;
extern boolean can_blow(struct monst *) NONNULLARG1;
extern boolean can_chant(struct monst *) NONNULLARG1;
extern boolean can_be_strangled(struct monst *) NONNULLARG1;
extern boolean can_track(struct permonst *) NONNULLARG1;
/**
 * @name Why armour will not stay on
 * @brief The two ways a body can be unsuited to worn armour.
 * @note Distinct because the consequence differs: one bursts the armour and destroys it, the other lets it fall off intact. So a hero polymorphing into one kind loses their suit and into the
 *       other does not, and the pair is what makes that difference expressible.
 * @{
 */
/**
 * @name 갑옷이 왜 붙어 있지 못하는지
 * @brief 몸이 착용한 갑옷에 맞지 않을 수 있는 두 방식.
 * @note 결과가 다르므로 구별된다. 하나는 갑옷을 터뜨려 파괴하고, 다른 하나는 온전한 채로 흘러내리게 한다. 그래서 한 종류로 변신하는 영웅은 갑옷을 잃고 다른 종류로 변신하면 잃지 않는다. 그 짝이 그 차이를 표현할 수 있게 하는 것이다.
 * @{
 */
extern boolean breakarm(struct permonst *) NONNULLARG1;
extern boolean sliparm(struct permonst *) NONNULLARG1;
/** @} */
/**
 * @brief Whether this kind of monster holds on rather than letting go.
 * @warning The name suggests adhesion; what it means is that a monster of this kind, once it has grabbed the hero, cannot be escaped by ordinary means. So it is about the grip and not about
 *          the surface.
 */
/**
 * @brief 이 종류의 몬스터가 놓아주는 대신 붙잡고 있는지.
 * @warning 그 이름은 들러붙음을 시사한다. 그것이 뜻하는 것은, 이 종류의 몬스터가 영웅을 붙잡은 뒤에는 평범한 수단으로 벗어날 수 없다는 것이다. 그래서 표면이 아니라 그 쥠에 관한 것이다.
 */
extern boolean sticks(struct permonst *) NONNULLARG1;
extern boolean cantvomit(struct permonst *) NONNULLARG1;
extern int num_horns(struct permonst *) NONNULLARG1;
extern struct attack *dmgtype_fromattack(struct permonst *, int, int) NONNULLARG1;
extern boolean dmgtype(struct permonst *, int) NONNULLARG1;
extern int max_passive_dmg(struct monst *, struct monst *) NONNULLARG12;
extern boolean same_race(struct permonst *, struct permonst *) NONNULLARG12;
/**
 * @name Recognising a monster by name
 * @brief Turn text the player typed into a monster, or into a monster class.
 *
 * Three forms, and the differences matter. The plain one wants the whole string to be a monster's name. The extended one accepts a name with something after it and hands back where the name
 * ended, which is what lets "gnome lord corpse" be parsed without the caller guessing where to split. The class form accepts a class name instead, for a request that means any of a kind.
 *
 * @note Each writes back extra information through a pointer -- how the match was made, or where it ended -- because a caller usually needs to know more than which monster it was.
 * @warning A name may match a class and a monster both. Which of these is asked decides the answer, so trying one and then the other is not the same as trying them in the other order.
 * @{
 */
/**
 * @name 이름으로 몬스터를 알아보기
 * @brief 플레이어가 입력한 글을 몬스터로, 또는 몬스터 계열로 바꾼다.
 *
 * 세 형태이며 그 차이가 중요하다. 평범한 것은 문자열 전체가 몬스터의 이름이기를 원한다. 확장된 것은 뒤에 무언가가 붙은 이름을 받아들이고 그 이름이 어디서 끝났는지를 되돌려준다. 그것이 호출자가 어디서 쪼갤지 짐작하지 않고 "노움 영주의 시체"를 파싱할 수 있게 하는 것이다. 계열
 * 형태는 대신 계열 이름을 받아들인다. 어떤 종류의 아무것이나를 뜻하는 요청을 위해서다.
 *
 * @note 각각이 포인터를 통해 추가 정보를 되기록한다. 어떻게 일치했는지, 또는 어디서 끝났는지. 호출자가 보통 어느 몬스터였는지 이상을 알아야 하기 때문이다.
 * @warning 이름이 계열과 몬스터에 둘 다 일치할 수 있다. 이들 중 어느 것을 묻는지가 답을 정하므로, 하나를 시도한 뒤 다른 것을 시도하는 것은 반대 순서로 시도하는 것과 같지 않다.
 * @{
 */
extern int name_to_mon(const char *, int *) NONNULLARG1;
extern int name_to_monplus(const char *, const char **, int *) NONNULLARG1;
extern int name_to_monclass(const char *, int *);
/** @} */
/**
 * @brief What a monster's sex actually is.
 * @note The truth, regardless of what the hero can tell. Use it for rules; use the pronoun form for anything the player will read.
 */
/**
 * @brief 몬스터의 성별이 실제로 무엇인지.
 * @note 영웅이 알 수 있는 바와 무관한 진실이다. 규칙에는 이것을 쓰고, 플레이어가 읽을 것에는 대명사 형태를 쓸 것.
 */
extern int gender(struct monst *) NONNULLARG1;
/**
 * @brief Which sex to speak of a monster as, given what the hero can tell.
 * @warning Not the monster's sex. An unseen monster is spoken of as neuter whatever it is, and a hallucinating hero is told about something else entirely -- so using this in a rule makes the
 *          rule depend on the hero's perception.
 */
/**
 * @brief 영웅이 알 수 있는 바에 따라, 몬스터를 어느 성별로 말할지.
 * @warning 몬스터의 성별이 아니다. 보이지 않는 몬스터는 그것이 무엇이든 중성으로 말해지고, 환각 중인 영웅에게는 전혀 다른 것이 알려진다. 그래서 이것을 규칙에 쓰면 그 규칙이 영웅의 지각에 의존하게 된다.
 */
extern int pronoun_gender(struct monst *, unsigned) NONNULLARG1;
/**
 * @brief Whether a monster is of a kind that would follow the hero to another level.
 * @note About the kind rather than the circumstances -- whether it is close enough or leashed is decided elsewhere. So a true answer is a precondition for following and not a prediction of it.
 */
/**
 * @brief 몬스터가 영웅을 다른 레벨로 따라올 종류인지.
 * @note 정황이 아니라 종류에 관한 것이다. 그것이 충분히 가까운지나 목줄에 묶였는지는 다른 곳에서 정해진다. 그래서 참이라는 답은 따라오기의 전제 조건이며 그것에 대한 예측이 아니다.
 */
extern boolean levl_follower(struct monst *) NONNULLARG1;
/**
 * @name Growing up and shrinking down
 * @brief Convert between a young monster's kind and its adult form.
 * @note Not every monster has both, so a conversion may return the same kind unchanged rather than failing -- which is why the match test exists: to ask whether two kinds are the two ages of
 *       one creature without performing a conversion.
 * @{
 */
/**
 * @name 자라기와 줄어들기
 * @brief 어린 몬스터의 종류와 그 성체 형태 사이를 변환한다.
 * @note 모든 몬스터가 둘 다를 갖지는 않으므로, 변환이 실패하는 대신 같은 종류를 그대로 반환할 수 있다. 그것이 일치 검사가 존재하는 이유다. 변환을 수행하지 않고 두 종류가 한 생물의 두 나이인지 묻기 위해서.
 * @{
 */
extern int little_to_big(int);
extern int big_to_little(int);
extern boolean big_little_match(int, int);
/** @} */
extern const char *locomotion(const struct permonst *, const char *) NONNULLARG12;
extern const char *stagger(const struct permonst *, const char *) NONNULLARG12;
extern const char *on_fire(struct permonst *, struct attack *) NONNULLARG12;
extern const char *msummon_environ(struct permonst *, const char **) NONNULLARG12;
extern const struct permonst *raceptr(struct monst *) NONNULLARG1;
extern boolean olfaction(struct permonst *) NONNULLARG1;
/**
 * @name What the monsters have seen the hero resist
 * @brief Record and withdraw the monsters' knowledge of the hero's resistances, and convert into its numbering.
 *
 * The monsters learn. If the hero shrugs off fire in front of something, that something knows better than to try fire again -- so what has been demonstrated is recorded, and it is recorded for
 * the monsters collectively rather than per monster.
 *
 * The two conversions exist because the same fact arrives in two vocabularies: as a damage type when an attack fails, and as a property when the hero's own state is examined. Both have to be
 * expressed in the one numbering this record uses.
 *
 * @note Withdrawing matters as much as recording. A resistance the hero loses has to be unlearned, or the monsters go on avoiding an attack that would now work.
 * @{
 */
/**
 * @name 몬스터들이 영웅이 무엇을 저항하는 것을 보았는지
 * @brief 영웅의 저항에 대한 몬스터들의 앎을 기록하고 철회하며, 그 번호 체계로 변환한다.
 *
 * 몬스터들은 배운다. 영웅이 무언가 앞에서 불을 떨쳐내면, 그 무언가는 다시 불을 시도하지 않을 만큼 안다. 그래서 무엇이 입증되었는지가 기록되며, 몬스터마다가 아니라 몬스터들 전체에 대해 기록된다.
 *
 * 두 변환이 있는 것은, 같은 사실이 두 어휘로 도착하기 때문이다. 공격이 실패할 때는 피해 종류로, 영웅 자신의 상태가 살펴질 때는 속성으로. 둘 다 이 기록이 쓰는 하나의 번호 체계로 표현되어야 한다.
 *
 * @note 철회가 기록만큼 중요하다. 영웅이 잃은 저항은 잊혀져야 한다. 그러지 않으면 몬스터들이 이제 통할 공격을 계속 피한다.
 * @{
 */
unsigned long cvt_adtyp_to_mseenres(uchar);
unsigned long cvt_prop_to_mseenres(uchar);
extern void monstseesu(unsigned long);
extern void monstunseesu(unsigned long);
/** @} */
/**
 * @brief Give a monster the resistances the hero has.
 * @note For a monster made to be a copy of the hero, or a form the hero has taken being applied to something else. It exists because those resistances are held differently on the two sides,
 *       so they cannot simply be assigned across.
 */
/**
 * @brief 몬스터에게 영웅이 가진 저항을 준다.
 * @note 영웅의 사본으로 만들어진 몬스터를 위한 것이거나, 영웅이 취한 형태가 다른 것에 적용되는 경우를 위한 것이다. 그 저항이 양쪽에서 다르게 보관되므로 그냥 대입해 넘길 수 없기 때문에 존재한다.
 */
extern void give_u_to_m_resistances(struct monst *) NONNULLARG1;
extern boolean resist_conflict(struct monst *) NONNULLARG1;
/**
 * @name What a monster knows about traps
 * @brief Ask, record, and broadcast knowledge of traps.
 *
 * A monster that has seen a trap avoids it, which is why a trap the hero set is not a reliable weapon twice. The knowledge is per monster, so one learning does not teach the rest -- except
 * through the broadcast form, which is for a trap being sprung visibly enough that everything watching learns.
 *
 * @note All three accept the value meaning every kind of trap, which is how a monster that has learned caution in general is expressed without listing the kinds.
 * @{
 */
/**
 * @name 몬스터가 함정에 대해 무엇을 아는지
 * @brief 함정에 대한 앎을 묻고, 기록하고, 널리 알린다.
 *
 * 함정을 본 몬스터는 그것을 피한다. 그것이 영웅이 놓은 함정이 두 번은 믿을 만한 무기가 아닌 이유다. 그 앎은 몬스터마다이므로 하나가 배운 것이 나머지를 가르치지 않는다. 다만 널리 알리는 형태를 통해서는 그렇다. 그것은 보고 있는 모든 것이 배울 만큼 눈에 띄게 발동된 함정을 위한
 * 것이다.
 *
 * @note 셋 모두가 모든 종류의 함정을 뜻하는 값을 받아들인다. 그것이 종류를 나열하지 않고 전반적인 조심을 배운 몬스터를 표현하는 방식이다.
 * @{
 */
extern boolean mon_knows_traps(struct monst *, int) NONNULLARG1;
extern void mon_learns_traps(struct monst *, int) NONNULLARG1;
extern void mons_see_trap(struct trap *) NONNULLARG1;
/** @} */
extern int get_atkdam_type(int);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED) || defined(DEBUG)
extern int mstrength(struct permonst *) NONNULLARG1;
#endif

/* ### monmove.c ### */

extern boolean mon_would_take_item(struct monst *, struct obj *) NONNULLARG12;
extern boolean mon_would_consume_item(struct monst *, struct obj *) NONNULLARG12;
extern boolean itsstuck(struct monst *) NONNULLARG1;
extern boolean mb_trapped(struct monst *, boolean) NONNULLARG1;
extern void mon_track_add(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void mon_track_clear(struct monst *) NONNULLARG1;
extern boolean monhaskey(struct monst *, boolean) NONNULLARG1;
extern void mon_regen(struct monst *, boolean) NONNULLARG1;
extern void m_everyturn_effect(struct monst *) NONNULLARG1;
extern void m_postmove_effect(struct monst *) NONNULLARG1;
extern int dochugw(struct monst *, boolean) NONNULLARG1;
extern boolean onscary(coordxy, coordxy, struct monst *) NONNULLARG3;
extern struct monst *find_pmmonst(int);
extern int bee_eat_jelly(struct monst *, struct obj *) NONNULLARG12;
extern void monflee(struct monst *, int, boolean, boolean) NONNULLARG1;
extern void mon_yells(struct monst *, const char *) NONNULLARG12;
extern boolean m_can_break_boulder(struct monst *) NONNULLARG1;
extern void m_break_boulder(struct monst *, coordxy, coordxy) NONNULLARG1;
extern int dochug(struct monst *) NONNULLARG1;
extern boolean m_digweapon_check(struct monst *, coordxy, coordxy) NONNULLARG1;
extern boolean m_avoid_kicked_loc(struct monst *, coordxy, coordxy) NONNULLARG1;
extern boolean m_avoid_soko_push_loc(struct monst *, coordxy, coordxy) NONNULLARG1;
extern int m_move(struct monst *, int) NONNULLARG1;
extern int m_move_aggress(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void dissolve_bars(coordxy, coordxy);
extern boolean closed_door(coordxy, coordxy);
extern boolean accessible(coordxy, coordxy);
extern void set_apparxy(struct monst *) NONNULLARG1;
extern boolean can_ooze(struct monst *) NONNULLARG1;
extern boolean can_fog(struct monst *) NONNULLARG1;
extern boolean should_displace(struct monst *, const struct mfndposdata *,
                               coordxy, coordxy) NONNULLPTRS;
extern boolean undesirable_disp(struct monst *, coordxy, coordxy) NONNULLARG1;
extern boolean can_hide_under_obj(struct obj *);

/* ### monst.c ### */

extern void monst_globals_init(void);

/* ### mplayer.c ### */

extern struct monst *mk_mplayer(struct permonst *,
                                coordxy, coordxy, boolean) NONNULLARG1;
extern void create_mplayers(int, boolean);
extern void mplayer_talk(struct monst *) NONNULLARG1;

#if defined(MICRO) || defined(WIN32)

/* ### msdos.c,os2.c,tos.c,windsys.c ### */

#ifndef WIN32
extern int tgetch(void);
#endif
#ifndef TOS
extern char switchar(void);
#endif
#ifndef __GO32__
extern long freediskspace(char *);
#ifdef MSDOS
extern int findfirst_file(char *);
extern int findnext_file(void);
extern long filesize_nh(char *);
#else
extern int findfirst(char *);
extern int findnext(void);
extern long filesize(char *);
#endif /* MSDOS */
extern char *foundfile_buffer(void);
#endif /* __GO32__ */
extern void chdrive(const char *);
#ifndef TOS
extern void disable_ctrlP(void);
extern void enable_ctrlP(void);
#endif
#if defined(MICRO) && !defined(WIN32)
extern void get_scr_size(void);
#ifndef TOS
extern void gotoxy(int, int);
#endif
#endif
#ifdef TOS
extern int _copyfile(char *, char *);
extern int kbhit(void);
extern void set_colors(void);
extern void restore_colors(void);
#ifdef SUSPEND
extern int dosuspend(void);
#endif
#endif /* TOS */
#ifdef WIN32
extern void nt_regularize(char *);
extern int(*nt_kbhit)(void);
extern void Delay(int);
#ifdef REALTIME_PROTO
extern unsigned long nt_ticks(void);
#endif
boolean get_user_home_folder(char *, size_t);
# ifdef CRASHREPORT
struct CRctxt;
extern struct CRctxt *ctxp;
extern int win32_cr_helper(char, struct CRctxt *, void *, int);
extern int win32_cr_gettrace(int, char *, int);
extern int *win32_cr_shellexecute(const char *);
# endif
#endif /* WIN32 */

#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

extern const char *rnd_hallublast(void);
extern boolean m_has_launcher_and_ammo(struct monst *) NONNULLARG1;
extern int thitu(int, int, struct obj **, const char *) NO_NNARGS;
extern boolean ohitmon(struct monst *, struct obj *,
                       int, boolean) NONNULLARG12;
extern void thrwmu(struct monst *) NONNULLARG1;
extern int spitmu(struct monst *, struct attack *) NONNULLPTRS;
extern int breamu(struct monst *, struct attack *) NONNULLPTRS;
extern boolean linedup_callback(coordxy, coordxy, coordxy, coordxy,
                                boolean(*)(coordxy, coordxy));
extern boolean linedup(coordxy, coordxy, coordxy, coordxy, int);
extern boolean lined_up(struct monst *) NONNULLARG1;
extern struct obj *m_carrying(struct monst *, int) NONNULLARG1;
extern int thrwmm(struct monst *, struct monst *) NONNULLARG12;
extern int spitmm(struct monst *, struct attack *, struct monst *) NONNULLPTRS;
extern int breamm(struct monst *, struct attack *, struct monst *) NONNULLPTRS;
extern void m_useupall(struct monst *, struct obj *) NONNULLARG12;
extern void m_useup(struct monst *, struct obj *) NONNULLARG12;
extern void m_throw(struct monst *, coordxy, coordxy, coordxy, coordxy,
                    int, struct obj *) NONNULLPTRS;
extern void hit_bars(struct obj **, coordxy, coordxy, coordxy, coordxy,
                     unsigned) NONNULLARG1;
extern boolean hits_bars(struct obj **, coordxy, coordxy, coordxy, coordxy,
                         int, int) NONNULLARG1;

/* ### muse.c ### */

extern boolean find_defensive(struct monst *, boolean) NONNULLARG1;
extern int use_defensive(struct monst *) NONNULLARG1;
extern int rnd_defensive_item(struct monst *) NONNULLARG1;
extern boolean find_offensive(struct monst *) NONNULLARG1;
extern int use_offensive(struct monst *) NONNULLARG1;
extern int rnd_offensive_item(struct monst *) NONNULLARG1;
extern boolean find_misc(struct monst *) NONNULLARG1;
extern int use_misc(struct monst *) NONNULLARG1;
extern int rnd_misc_item(struct monst *) NONNULLARG1;
extern boolean searches_for_item(struct monst *, struct obj *) NONNULLARG12;
extern boolean mon_reflects(struct monst *, const char *) NONNULLARG1;
extern boolean ureflects(const char *, const char *) NO_NNARGS;
extern void mcureblindness(struct monst *, boolean) NONNULLARG1;
extern boolean munstone(struct monst *, boolean) NONNULLARG1;
extern boolean munslime(struct monst *, boolean) NONNULLARG1;

/* ### music.c ### */

extern void awaken_soldiers(struct monst *) NONNULLARG1;
extern int do_play_instrument(struct obj *) NONNULLARG1;
enum instruments obj_to_instr(struct obj *) NONNULLARG1;

/* ### nhlsel.c ### */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
extern struct selectionvar *l_selection_check(lua_State *, int) NONNULLARG1;
extern int l_selection_register(lua_State *) NONNULLARG1;
extern void l_selection_push_copy(lua_State *, struct selectionvar *) NONNULLARG12;
extern int l_obj_register(lua_State *) NONNULLARG1;
#endif

/* ### nhlobj.c ### */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
extern void nhl_push_obj(lua_State *, struct obj *) NONNULLARG12;
extern int nhl_obj_u_giveobj(lua_State *) NONNULLARG1;
extern int l_obj_register(lua_State *) NONNULLARG1;
#endif

/* ### nhlua.c ### */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
extern void l_nhcore_init(void);
extern void l_nhcore_done(void);
extern void l_nhcore_call(int);
extern lua_State * nhl_init(nhl_sandbox_info *) NONNULLARG1;
/* nhl_done contains a test for NULL arg1, preventing NONNULLARG1 */
extern void nhl_done(lua_State *) NO_NNARGS;
extern boolean nhl_loadlua(lua_State *, const char *) NONNULLARG12;
extern char *get_nh_lua_variables(void);
extern void save_luadata(NHFILE *) NONNULLARG1;
extern void restore_luadata(NHFILE *) NONNULLARG1;
extern int nhl_pcall(lua_State *, int, int, const char *) NONNULLARG1;
extern int nhl_pcall_handle(lua_State *, int, int, const char *,
                            NHL_pcall_action) NONNULLARG1;
extern boolean load_lua(const char *, nhl_sandbox_info *) NONNULLARG12;
ATTRNORETURN extern void nhl_error(lua_State *, const char *)
                                                        NORETURN NONNULLARG12;
extern void lcheck_param_table(lua_State *) NONNULLARG1;
extern schar get_table_mapchr(lua_State *, const char *) NONNULLARG12;
extern schar get_table_mapchr_opt(lua_State *, const char *, schar)
                                                                 NONNULLARG12;
extern short nhl_get_timertype(lua_State *, int) NONNULLARG1;
extern boolean nhl_get_xy_params(lua_State *, lua_Integer *, lua_Integer *)
                                                                NONNULLARG123;
extern void nhl_add_table_entry_int(lua_State *, const char *, lua_Integer)
                                                                 NONNULLARG12;
extern void nhl_add_table_entry_char(lua_State *, const char *, char)
                                                                 NONNULLARG12;
extern void nhl_add_table_entry_str(lua_State *, const char *, const char *)
                                                                NONNULLARG123;
extern void nhl_add_table_entry_bool(lua_State *, const char *, boolean)
                                                                 NONNULLARG12;
extern void nhl_add_table_entry_region(lua_State *, const char *,
                                       coordxy, coordxy, coordxy, coordxy)
                                                                 NONNULLARG12;
extern schar splev_chr2typ(char);
extern schar check_mapchr(const char *) NO_NNARGS;
extern int get_table_int(lua_State *, const char *) NONNULLARG12;
extern int get_table_int_opt(lua_State *, const char *, int) NONNULLARG12;
extern char *get_table_str(lua_State *, const char *) NONNULLARG12;
/* dungeon.c init_dungeon_levels() passes NULL to get_table_str_opt arg3 */
extern char *get_table_str_opt(lua_State *, const char *, char *) NONNULLARG12;
extern int get_table_boolean(lua_State *, const char *) NONNULLARG12;
extern int get_table_boolean_opt(lua_State *, const char *, int) NONNULLARG12;
/* lspo_feature calls get_table_option(L, "type", NULL, features),
   so arg3 can be NULL.  NONNULLARG124 is not currently defined */
extern int get_table_option(lua_State *, const char *, const char *,
                            const char *const *) NO_NNARGS;
/* extern int str_lines_max_width(const char *); */
extern const char *get_lua_version(void);
extern void nhl_pushhooked_open_table(lua_State *L) NONNULLARG1;
extern void free_tutorial(void);
extern void tutorial(boolean);
#endif /* !CROSSCOMPILE || CROSSCOMPILE_TARGET */

#endif /* MAKEDEFS_C MDLIB_C CPPREGEX_C */

/* ### {cpp,pmatch,posix}regex.c ### */
#include "nhregex.h"

#if !defined(MAKEDEFS_C) && !defined(MDLIB_C) && !defined(CPPREGEX_C)

/* ### consoletty.c  ### */

#ifdef WIN32
extern void get_scr_size(void);
extern int consoletty_kbhit(void);
extern void consoletty_open(int);
extern void consoletty_rubout(void);
extern int tgetch(void);
extern int console_poskey(coordxy *, coordxy *, int *);
void console_g_putch(int in_ch);
extern void set_output_mode(int);
extern void synch_cursor(void);
extern void nethack_enter_consoletty(void);
/* body in consoletty.c and mhmain.c */
extern int get_approx_display_cols(void);
extern int get_approx_display_rows(void);
extern void console_exit(void);
extern int set_keyhandling_via_option(void);
#ifdef ENHANCED_SYMBOLS
extern void tty_utf8graphics_fixup(void);
extern void tty_ibmgraphics_fixup(void);
#endif /* ENHANCED_SYMBOLS */
#endif /* WIN32 */

/* ### o_init.c ### */

extern void init_objects(void);
extern void init_oclass_probs(void);
extern void obj_shuffle_range(int, int *, int *) NONNULLPTRS;
/* objdescr_is() contains a test for NULL arg1, so can't be NONNULLARG12 */
extern boolean objdescr_is(struct obj *, const char *) NONNULLARG2;
extern void oinit(void);
extern void savenames(NHFILE *) NONNULLARG1;
extern void restnames(NHFILE *) NONNULLARG1;
extern void observe_object(struct obj *) NONNULLARG1;
extern void discover_object(int, boolean, boolean, boolean);
extern void undiscover_object(int);
extern boolean interesting_to_discover(int);
extern int choose_disco_sort(int);
extern int dodiscovered(void);
extern int doclassdisco(void);
extern void rename_disco(void);
extern void get_sortdisco(char *opts, boolean cnf) NONNULLARG1;

/* ### objects.c ### */

extern void objects_globals_init(void);

/* ### objnam.c ### */

/**
 * @note Almost every routine in this section returns a pointer into a shared pool of buffers rather than into memory of its own. That single fact governs how the whole section must be used, and it
 *       is why the naming routines look interchangeable and are not safe to nest freely.
 *
 *       The pool is a rotation: each call takes the next buffer, so several answers can be live at once but not indefinitely many. Building a sentence from two names works; building one from a
 *       dozen does not, and the failure is that an early name has been quietly overwritten by the time the sentence is assembled.
 *
 *       Where a caller must hold a name for longer than that, it has to copy it. Where a caller has finished with one early, releasing it back explicitly is what the release routine is for.
 */
/**
 * @note 이 절의 거의 모든 루틴이 자기 메모리가 아니라 공유된 버퍼 풀을 가리키는 포인터를 반환한다. 그 하나의 사실이 이 절 전체를 어떻게 써야 하는지를 지배하며, 그것이 이름 짓기 루틴들이 서로 바꿔 쓸 수 있어 보이면서 자유롭게 중첩해도 안전하지는 않은 이유다.
 *
 *       그 풀은 순환이다. 각 호출이 다음 버퍼를 가져가므로, 여러 답이 동시에 살아 있을 수 있으나 무한히 많이는 아니다. 두 이름으로 문장을 짓는 것은 통한다. 열두 개로 짓는 것은 통하지 않으며, 그 실패는 문장이 조립될 때쯤 이른 이름이 조용히 덮어써져 있었다는 것이다.
 *
 *       호출자가 이름을 그보다 오래 쥐어야 하는 곳에서는 그것을 복사해야 한다. 호출자가 하나를 일찍 다 쓴 곳에서는 그것을 명시적으로 되돌려주는 것이 해제 루틴의 목적이다.
 */
/**
 * @brief Give a name buffer back to the pool early.
 * @note Optional, and worth using where a routine takes several names in a loop -- returning each as it finishes keeps the rotation from being exhausted.
 * @warning The pointer must be one the pool handed out. Passing anything else is undefined, and there is nothing that checks.
 */
/**
 * @brief 이름 버퍼를 풀에 일찍 되돌려준다.
 * @note 선택적이며, 어떤 루틴이 반복문에서 여러 이름을 가져가는 곳에서 쓸 가치가 있다. 각각을 다 쓸 때마다 되돌려주면 그 순환이 소진되지 않는다.
 * @warning 그 포인터는 풀이 내준 것이어야 한다. 다른 것을 넘기는 것은 정의되지 않았고, 검사하는 것이 없다.
 */
extern void maybereleaseobuf(char *) NONNULLARG1;
/**
 * @name Naming a kind of object
 * @brief The name of an object kind, in three degrees of caution.
 *
 * The plain form gives the name as the hero knows it, which may be its appearance rather than its identity. The simple form omits the decoration -- no charges, no called-name. The safe form is
 * the one to reach for when it is not certain the kind is valid.
 *
 * @note The safe form exists because a name is sometimes wanted for a value that came from outside the game -- a configuration file, a saved log -- where an invalid kind is possible. The others
 *       would fail on such a value.
 * @{
 */
/**
 * @name 물건 종류의 이름 짓기
 * @brief 물건 종류의 이름. 세 단계의 조심스러움으로.
 *
 * 평범한 형태는 영웅이 아는 대로의 이름을 준다. 그것은 정체가 아니라 외형일 수 있다. 단순 형태는 장식을 뺀다. 충전도, 붙여진 이름도 없이. 안전한 형태는 그 종류가 유효한지 확실하지 않을 때 손을 뻗을 것이다.
 *
 * @note 안전한 형태가 있는 것은, 이름이 때때로 게임 밖에서 온 값 -- 설정 파일, 저장된 로그 -- 에 대해 필요하고 그곳에서는 유효하지 않은 종류가 가능하기 때문이다. 나머지는 그런 값에서 실패한다.
 * @{
 */
extern char *obj_typename(int);
extern char *simple_typename(int);
extern char *safe_typename(int);
/** @} */
/**
 * @brief Whether an object's name is a proper name and so takes no article.
 * @note Asked before adding "a" or "the". An artifact is called Excalibur and not the Excalibur, so this is what keeps the naming routines from producing that.
 */
/**
 * @brief 물건의 이름이 고유명사여서 관사를 받지 않는지.
 * @note "a"나 "the"를 붙이기 전에 물어진다. 아티팩트는 the Excalibur 가 아니라 Excalibur 라고 불리므로, 이것이 이름 짓기 루틴이 그것을 만들어 내지 않게 하는 것이다.
 */
extern boolean obj_is_pname(struct obj *) NONNULLARG1;
/**
 * @brief Name an object as it would be described from a distance.
 * @param  the object
 * @param  the naming routine to use for the parts that are visible from afar
 * @note Takes the naming routine as an argument because "from a distance" is a modification of naming rather than a kind of it: the caller says how it would name the object up close, and this
 *       withholds what could not be seen from where the hero is.
 */
/**
 * @brief 물건을 멀리서 기술될 대로 이름 짓는다.
 * @param  그 물건
 * @param  멀리서 보이는 부분에 대해 쓸 이름 짓기 루틴
 * @note 이름 짓기 루틴을 인자로 받는 것은, "멀리서"가 이름 짓기의 한 종류가 아니라 그것에 대한 수정이기 때문이다. 호출자가 가까이서 그 물건을 어떻게 이름 짓겠는지 말하고, 이것이 영웅이 있는 곳에서 볼 수 없었을 것을 보류한다.
 */
extern char *distant_name(struct obj *, char *(*)(struct obj *)) NONNULLPTRS;
extern char *fruitname(boolean);
extern struct fruit *fruit_from_indx(int);
extern struct fruit *fruit_from_name(const char *, boolean, int *) NONNULLARG1;
extern void reorder_fruit(boolean);
/**
 * @brief The bare name of an object, without article, quantity or condition.
 * @note The foundation the rest of the naming routines are built on. It gives what the hero knows the object to be and nothing more, so a caller adds whatever the sentence needs.
 * @warning There is a variant of this taking flags, and one of the flags exists because this is sometimes called indirectly and the caller cannot reach the variant. That is recorded with the flag
 *          in flag.h rather than here.
 */
/**
 * @brief 물건의 맨 이름. 관사도 수량도 상태도 없이.
 * @note 나머지 이름 짓기 루틴들이 그 위에 세워지는 토대다. 영웅이 그 물건을 무엇이라고 아는지를 주고 그 이상은 주지 않으므로, 호출자가 문장이 필요로 하는 것을 더한다.
 * @warning 플래그를 받는 이것의 변종이 있고, 그 플래그 중 하나가 존재하는 것은 이것이 때때로 간접적으로 호출되어 호출자가 그 변종에 닿을 수 없기 때문이다. 그것은 여기가 아니라 flag.h 의 그 플래그와 함께 기록되어 있다.
 */
extern char *xname(struct obj *) NONNULLARG1;
/**
 * @brief The name of an object as part of a multi-shot volley.
 * @note Volleys are described once for several missiles -- "you shoot 3 arrows" -- so the name has to agree with a count the caller is about to print rather than with the object's own quantity.
 */
/**
 * @brief 여러 발 사격의 일부로서의 물건 이름.
 * @note 여러 투사체에 대해 한 번 기술되므로 -- "화살 3발을 쏜다" -- 그 이름이 물건 자신의 수량이 아니라 호출자가 곧 인쇄할 개수와 맞아야 한다.
 */
extern char *mshot_xname(struct obj *) NONNULLARG1;
extern boolean the_unique_obj(struct obj *) NONNULLARG1;
extern boolean the_unique_pm(struct permonst *) NONNULLARG1;
/**
 * @brief Whether an object's erosion is worth mentioning.
 * @note Not whether it is eroded. Erosion is only worth reporting on something whose condition affects its use, so an eroded object may still be described without it -- and that judgement is made
 *       here rather than in each naming routine.
 */
/**
 * @brief 물건의 삭음이 언급할 가치가 있는지.
 * @note 그것이 삭았는지가 아니다. 삭음은 상태가 그 용도에 영향을 주는 것에 대해서만 보고할 가치가 있으므로, 삭은 물건도 그것 없이 기술될 수 있다. 그리고 그 판단이 이름 짓기 루틴마다가 아니라 여기서 내려진다.
 */
extern boolean erosion_matters(struct obj *) NONNULLARG1;
/**
 * @name The full name of an object
 * @brief An object named as the inventory would show it, in variants for particular situations.
 *
 * The plain form is the one to use: quantity, condition, enchantment where known, what it is being used for. The variants each add or alter one thing -- a shop price, a statue's or corpse's sex, a
 * quantity given vaguely rather than exactly.
 *
 * @note They exist as separate routines rather than as flags because each is wanted in one specific place: a shop listing, a description of a statue, a message about a pile too large to count.
 *       Reading the list is therefore a reasonable way to find which situations the game distinguishes.
 * @{
 */
/**
 * @name 물건의 온전한 이름
 * @brief 소지품이 보이는 대로 이름 붙은 물건. 특정 상황을 위한 변종들로.
 *
 * 평범한 형태가 써야 할 것이다. 수량, 상태, 알려진 경우의 강화, 무엇에 쓰이고 있는지. 변종들은 각각 한 가지를 더하거나 바꾼다. 상점 가격, 조각상이나 시체의 성별, 정확하게가 아니라 어림으로 주어진 수량.
 *
 * @note 플래그가 아니라 별개의 루틴으로 존재하는 것은, 각각이 특정한 한 곳에서 필요하기 때문이다. 상점 목록, 조각상의 기술, 셀 수 없이 큰 무더기에 관한 메시지. 그래서 이 목록을 읽는 것은 게임이 어떤 상황을 구별하는지 알아내는 합당한 방법이다.
 * @{
 */
extern char *doname(struct obj *) NONNULLARG1;
extern char *doname_with_price(struct obj *) NONNULLARG1;
extern char *doname_with_cgender(struct obj *) NONNULLARG1;
extern char *doname_with_price_and_cgender(struct obj *) NONNULLARG1;
extern char *doname_vague_quan(struct obj *) NONNULLARG1;
/** @} */
extern boolean not_fully_identified(struct obj *) NONNULLARG1;
extern char *corpse_xname(struct obj *, const char *, unsigned) NONNULLARG1;
extern char *cxname(struct obj *) NONNULLARG1;
extern char *cxname_singular(struct obj *) NONNULLARG1;
/**
 * @brief The name of an object as it should appear in a death message.
 * @note Different from ordinary naming in one important way: it names the object as it truly is rather than as the hero knew it, because a death is recorded permanently and "killed by an unlabeled
 *       scroll" would be a worse record than the truth.
 */
/**
 * @brief 사망 메시지에 나타나야 하는 대로의 물건 이름.
 * @note 평범한 이름 짓기와 한 가지 중요한 점에서 다르다. 영웅이 알던 대로가 아니라 실제 그대로 그 물건을 이름 짓는다. 죽음은 영구히 기록되고, "이름 없는 두루마리에게 죽었다"는 진실보다 나쁜 기록이 되기 때문이다.
 */
extern char *killer_xname(struct obj *) NONNULLARG1;
/**
 * @brief Name an object, falling back to a shorter naming if the first result is too long.
 * @param  the object
 * @param  the preferred naming routine
 * @param  the naming routine to fall back to
 * @param  the length to stay within
 * @note Takes two naming routines because there is no general way to shorten a name -- so the caller supplies both the full and the abbreviated way of saying it, and this chooses. That is how a
 *       message fits on a line without the caller measuring anything.
 */
/**
 * @brief 물건을 이름 짓되, 첫 결과가 너무 길면 더 짧은 이름 짓기로 돌아간다.
 * @param  그 물건
 * @param  선호하는 이름 짓기 루틴
 * @param  돌아갈 이름 짓기 루틴
 * @param  머물러야 할 길이
 * @note 두 이름 짓기 루틴을 받는 것은, 이름을 줄이는 일반적인 방법이 없기 때문이다. 그래서 호출자가 온전한 방식과 줄인 방식 둘 다를 제공하고 이것이 고른다. 그것이 호출자가 아무것도 재지 않고 메시지가 한 줄에 들어가는 방식이다.
 */
extern char *short_oname(struct obj *, char *(*)(struct obj *),
                         char *(*)(struct obj *), unsigned) NONNULLARG12;
/**
 * @brief Name an object as though there were one of it.
 * @note Takes the naming routine as an argument for the same reason as the distant form: it is a modification of naming rather than a kind of it, so it works with whichever naming the caller
 *       intended.
 */
/**
 * @brief 물건을 하나만 있는 것처럼 이름 짓는다.
 * @note 멀리서 보는 형태와 같은 이유로 이름 짓기 루틴을 인자로 받는다. 이름 짓기의 한 종류가 아니라 그것에 대한 수정이므로, 호출자가 의도한 어느 이름 짓기와도 함께 작동한다.
 */
extern const char *singular(struct obj *, char *(*)(struct obj *)) NONNULLPTRS;
/**
 * @brief Put the right indefinite article in front of a name, writing into the caller's buffer.
 * @note The only one of the article routines that does not use the shared pool. That is why it exists: a caller that needs the result to outlive the pool's rotation uses this and supplies the
 *       memory.
 */
/**
 * @brief 이름 앞에 알맞은 부정관사를 붙이며, 호출자의 버퍼에 쓴다.
 * @note 관사 루틴 중 공유 풀을 쓰지 않는 유일한 것이다. 그것이 존재하는 이유다. 결과가 풀의 순환보다 오래 살아야 하는 호출자가 이것을 쓰고 메모리를 제공한다.
 */
extern char *just_an(char *, const char *) NONNULL NONNULLARG12;
/**
 * @name Articles
 * @brief Put an article in front of a name, choosing it from the name itself.
 *
 * Four routines because English needs both articles in both cases: the indefinite and the definite, each with and without a capital. The capitalisation is a separate routine rather than a flag
 * because it is the first letter of the article and not of the name, so it cannot be applied afterwards.
 *
 * @note Choosing the indefinite article is not a matter of looking at the first letter: a name may begin with a vowel and take "a", or be a proper name and take nothing at all. That judgement is
 *       what these are for.
 * @warning All four accept null, and the accompanying comment records that this is why they are not annotated as requiring an argument. A null name is handled rather than rejected.
 * @{
 */
/**
 * @name 관사
 * @brief 이름 앞에 관사를 붙인다. 그 이름 자체에서 관사를 골라서.
 *
 * 네 루틴인 것은 영어가 두 관사를 두 형태로 필요로 하기 때문이다. 부정관사와 정관사, 각각 대문자로와 그렇지 않게. 대문자화가 플래그가 아니라 별개의 루틴인 것은, 그것이 이름의 첫 글자가 아니라 관사의 첫 글자이기 때문이다. 그래서 나중에 적용할 수 없다.
 *
 * @note 부정관사를 고르는 것은 첫 글자를 보는 문제가 아니다. 이름이 모음으로 시작하면서 "a"를 받을 수도, 고유명사여서 아무것도 받지 않을 수도 있다. 그 판단이 이들의 목적이다.
 * @warning 넷 모두 널을 받아들이며, 딸린 주석이 그것이 이들이 인자를 요구하는 것으로 표시되지 않은 이유라고 기록한다. 널 이름은 거부되는 것이 아니라 처리된다.
 * @{
 */
/* an(), the() contain tests for NULL arg, preventing NONNULLARG1 */
extern char *an(const char *) NONNULL NO_NNARGS;
extern char *An(const char *) NONNULL NO_NNARGS;
extern char *The(const char *) NONNULL NO_NNARGS;
extern char *the(const char *) NONNULL NO_NNARGS;
/** @} */
extern char *aobjnam(struct obj *, const char *) NONNULL NONNULLARG1;
extern char *yobjnam(struct obj *, const char *) NONNULL NONNULLARG1;
extern char *Yobjnam2(struct obj *, const char *) NONNULL NONNULLARG1;
extern char *Tobjnam(struct obj *, const char *) NONNULL NONNULLARG1;
/**
 * @name Making a verb agree
 * @brief Put a verb into the form that agrees with its subject.
 *
 * English inflects a verb by number, so a message about one object and a message about several need different verbs. Rather than writing both sentences, the game writes one and asks for the verb in
 * the right form.
 *
 * @note The object form takes the object as the subject; the general form takes any subject as text. The second exists because the subject is not always an object -- it may be a monster, the hero,
 *       or a phrase.
 * @warning The general form's subject may be null, and that is not an error: a null subject means the caller has already established agreement and wants only the default inflection. Its annotation
 *          reflects that only the verb is required.
 * @{
 */
/**
 * @name 동사를 일치시키기
 * @brief 동사를 그 주어와 일치하는 형태로 만든다.
 *
 * 영어는 동사를 수에 따라 굴절시키므로, 물건 하나에 관한 메시지와 여럿에 관한 메시지는 다른 동사를 필요로 한다. 두 문장을 쓰는 대신 게임은 하나를 쓰고 알맞은 형태의 동사를 요청한다.
 *
 * @note 물건 형태는 그 물건을 주어로 받는다. 일반 형태는 아무 주어든 글로 받는다. 두 번째가 있는 것은 주어가 항상 물건은 아니기 때문이다. 몬스터일 수도, 영웅일 수도, 구절일 수도 있다.
 * @warning 일반 형태의 주어는 널일 수 있고 그것은 오류가 아니다. 널 주어는 호출자가 이미 일치를 확정했고 기본 굴절만을 원한다는 뜻이다. 그 표시가 동사만이 필수임을 반영한다.
 * @{
 */
extern char *otense(struct obj *, const char *) NONNULL NONNULLARG12;
extern char *vtense(const char *, const char *) NONNULL NONNULLARG2;
/** @} */
extern char *Doname2(struct obj *) NONNULL NONNULLARG1;
extern char *paydoname(struct obj *) NONNULL NONNULLARG1;
extern char *yname(struct obj *) NONNULL NONNULLARG1;
extern char *Yname2(struct obj *) NONNULL NONNULLARG1;
extern char *ysimple_name(struct obj *) NONNULL NONNULLARG1;
extern char *Ysimple_name2(struct obj *) NONNULL NONNULLARG1;
extern char *simpleonames(struct obj *) NONNULL NONNULLARG1;
extern char *ansimpleoname(struct obj *) NONNULL NONNULLARG1;
extern char *thesimpleoname(struct obj *) NONNULL NONNULLARG1;
extern char *actualoname(struct obj *) NONNULL NONNULLARG1;
extern char *bare_artifactname(struct obj *) NONNULL NONNULLARG1;
/**
 * @name Changing a name's number
 * @brief Turn a name into its plural or its singular.
 *
 * English pluralisation is irregular, and the game's vocabulary contains a good deal of it -- "staves", "dwarves", "mice", names ending in "s" that are already singular. These handle the cases the
 * game actually needs rather than implementing a general rule.
 *
 * @warning Not inverses. Making a name plural and then singular again does not reliably return the original, because some plurals are ambiguous about what they came from.
 * @note Both accept null and both report an internal error rather than failing on input they cannot handle, which the accompanying comment records as the reason they are not annotated as requiring an
 *       argument.
 * @{
 */
/**
 * @name 이름의 수를 바꾸기
 * @brief 이름을 복수형이나 단수형으로 바꾼다.
 *
 * 영어의 복수화는 불규칙하고, 게임의 어휘에는 그것이 상당히 많다. "staves", "dwarves", "mice", 이미 단수인데 "s"로 끝나는 이름들. 이들은 일반 규칙을 구현하는 대신 게임이 실제로 필요로 하는 경우들을 다룬다.
 *
 * @warning 서로의 역이 아니다. 이름을 복수로 만든 뒤 다시 단수로 만드는 것이 원래 것을 믿을 만하게 되돌려주지는 않는다. 어떤 복수형은 그것이 무엇에서 왔는지에 대해 모호하기 때문이다.
 * @note 둘 다 널을 받아들이고, 둘 다 다룰 수 없는 입력에서 실패하는 대신 내부 오류를 알린다. 딸린 주석이 그것이 이들이 인자를 요구하는 것으로 표시되지 않은 이유라고 기록한다.
 * @{
 */
/* makeplural() and makesingular() never return NULL but have tests for NULL
   arg1, and code path that leads to impossible(), preventing NONNULLARG1 */
extern char *makeplural(const char *) NONNULL NO_NNARGS;
extern char *makesingular(const char *) NONNULL NO_NNARGS;
/** @} */
/**
 * @brief Build an object from a name the player typed.
 * @return the object, or null if the name could not be made into one
 * @note The inverse of all the naming above, and far harder: it accepts what a player might reasonably write, including quantities, conditions, enchantments and partial names. That is what makes
 *       wishing work.
 * @note A null name is not an error but a request for a random object, as the accompanying comment records -- which is how a wish for nothing in particular is expressed.
 * @warning It may modify the string it is given. A caller must not pass a literal or a buffer it needs afterwards.
 */
/**
 * @brief 플레이어가 입력한 이름에서 물건을 만든다.
 * @return 그 물건. 그 이름으로 하나를 만들 수 없었으면 널
 * @note 위의 모든 이름 짓기의 역이며 훨씬 어렵다. 플레이어가 합당하게 적을 만한 것을 받아들인다. 수량, 상태, 강화, 부분적인 이름까지. 그것이 소원을 작동하게 하는 것이다.
 * @note 널 이름은 오류가 아니라 무작위 물건에 대한 요청이며, 딸린 주석이 그것을 기록한다. 그것이 특정한 것 없는 소원이 표현되는 방식이다.
 * @warning 건네진 문자열을 바꿀 수 있다. 호출자는 리터럴이나 그 뒤에 필요한 버퍼를 넘겨서는 안 된다.
 */
/* readobjnam() can return NULL and  allows a NULL to trigger code path for
   random object */
extern struct obj *readobjnam(char *, struct obj *) NO_NNARGS;
extern int rnd_class(int, int);
/* discover_object() passes NULL arg2 to Japanese_item_name(),
 * preventing NONNULLARG2 */
extern const char *Japanese_item_name(int, const char *) NO_NNARGS;
extern const char *armor_simple_name(struct obj *) NONNULL NONNULLARG1;
/* suit_simple_name has its code in a NULL arg test
   conditional block, preventing NONNULLARG1 */
extern const char *suit_simple_name(struct obj *) NONNULL NO_NNARGS;
/* cloak_simple_name has its code in a NULL arg test
   conditional block, preventing NONNULLARG1 */
extern const char *cloak_simple_name(struct obj *) NONNULL NO_NNARGS;
/* helm_simple_name always just returns hardcoded literals */
extern const char *helm_simple_name(struct obj *) NONNULL NO_NNARGS;
/* gloves_simple_name has its code in a NULL arg test
   conditional block, preventing NONNULLARG1 */
extern const char *gloves_simple_name(struct obj *) NONNULL NO_NNARGS;
/* boots_simple_name has its code in a NULL arg test
   conditional block, preventing NONNULLARG1 */
extern const char *boots_simple_name(struct obj *) NONNULL NO_NNARGS;
/* shield_simple_name has its code in a NULL arg test
   conditional block, preventing NONNULLARG1 */
extern const char *shield_simple_name(struct obj *) NONNULL NO_NNARGS;
/* shirt_simple_name always just returns hardcoded "shirt" */
extern const char *shirt_simple_name(struct obj *) NONNULL NO_NNARGS;
extern const char *mimic_obj_name(struct monst *) NONNULL NONNULLARG1;
/* safe_qbuf() contains tests for NULL arg2 and arg3, qprefix and qsuffix,
   preventing use of NONNULLPTRS. */
extern char *safe_qbuf(char *, const char *, const char *, struct obj *,
                       char * (*)(struct obj *), char * (*)(struct obj *),
                       const char *) NONNULL NONNULLARG14;
extern int shiny_obj(char);

/* ### options.c ### */

extern boolean ask_do_tutorial(void);
extern boolean match_optname(const char *, const char *, int, boolean) NONNULLARG12;
extern uchar txt2key(char *) NONNULLARG1;
extern void initoptions(void);
extern void initoptions_init(void);
extern void initoptions_finish(void);
extern boolean parseoptions(char *, boolean, boolean) NONNULLARG1;
extern void freeroleoptvals(void);
extern char *get_option_value(const char *, boolean) NONNULLARG1;
extern int doset_simple(void);
extern int doset(void);
extern int dotogglepickup(void);
extern int toggle_bool_option(const char *);
extern void option_help(void);
extern void all_options_strbuf(strbuf_t *) NONNULLARG1;
extern void next_opt(winid, const char *) NONNULLARG2;
extern int fruitadd(char *, struct fruit *) NONNULLARG1;
extern boolean parsebindings(char *) NONNULLARG1;
extern void oc_to_str(char *, char *) NONNULLARG12;
extern void add_menu_cmd_alias(char, char);
extern char get_menu_cmd_key(char);
extern char map_menu_cmd(char);
extern char *collect_menu_keys(char *, unsigned, boolean) NONNULLARG1;
extern void show_menu_controls(winid, boolean);
extern void assign_warnings(uchar *) NONNULLARG1;
extern char *nh_getenv(const char *) NONNULLARG1;
extern void reset_duplicate_opt_detection(void);
extern void set_wc_option_mod_status(unsigned long, int);
extern void set_wc2_option_mod_status(unsigned long, int);
extern void set_option_mod_status(const char *, int) NONNULLARG1;
extern int add_autopickup_exception(const char *) NONNULLARG1;
extern void free_autopickup_exceptions(void);
extern void set_playmode(void);
extern int sym_val(const char *) NONNULLARG1;
extern boolean msgtype_parse_add(char *) NONNULLARG1;
extern int msgtype_type(const char *, boolean) NONNULLARG1;
extern void hide_unhide_msgtypes(boolean, int);
extern void msgtype_free(void);
extern void options_free_window_colors(void);
extern void heed_all_options(void);
extern void disregard_all_options(void);
extern void heed_this_option(enum opt);
extern void disregard_this_option(enum opt);
extern void clear_ignore_errors_on_unmatched(void);
#ifdef TTY_PERM_INVENT
extern void check_perm_invent_again(void);
#endif

/* ### pager.c ### */

extern char *self_lookat(char *) NONNULL NONNULLARG1;
extern char *monhealthdescr(struct monst *mon, boolean,
                            char *) NONNULL NONNULLARG3;
extern void mhidden_description(struct monst *, unsigned, char *) NONNULLPTRS;
extern boolean object_from_map(int, coordxy, coordxy,
                               struct obj **) NONNULLPTRS;
extern const char *waterbody_name(coordxy, coordxy) NONNULL;
extern char *ice_descr(coordxy, coordxy, char *) NONNULL NONNULLARG3;
extern boolean ia_checkfile(struct obj *) NONNULLARG1;
extern int do_screen_description(coord, boolean, int, char *, const char **,
                                 struct permonst **) NONNULLARG45;
extern int do_look(int, coord *);
extern int dowhatis(void);
extern int doquickwhatis(void);
extern int doidtrap(void);
extern int dowhatdoes(void);
extern char *dowhatdoes_core(char, char *) NONNULLARG2; /*might return NULL*/
extern int dohelp(void);
extern int dohistory(void);
void allopt_array_init(void);

/* ### xxmain.c ### */

#if defined(UNIX) || defined(MICRO) || defined(WIN32)
#ifdef CHDIR
extern void chdirx(const char *, boolean);
#endif /* CHDIR */
extern boolean authorize_wizard_mode(void);
extern boolean authorize_explore_mode(void);
#endif
#if defined(WIN32)
extern int getlock(void);
extern const char *get_portable_device(void);
#endif

/* ### pcsys.c, windsys.c ### */
#if defined(MICRO) || defined(WIN32)
ATTRNORETURN extern void nethack_exit(int) NORETURN;
#else
#define nethack_exit exit
#endif

/* ### pcsys.c  ### */

#if defined(MICRO) || defined(WIN32)
extern void flushout(void);
extern int dosh(void);
extern void append_slash(char *);
extern void getreturn(const char *);
#ifndef AMIGA
extern void msmsg(const char *, ...) PRINTF_F(1, 2);
#endif
/* E FILE *fopenp(const char *, const char *); */
#endif /* MICRO || WIN2 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
extern void gettty(void);
extern void settty(const char *);
extern void setftty(void);
ATTRNORETURN extern void error(const char *, ...) PRINTF_F(1, 2) NORETURN;
#if defined(TIMED_DELAY) && defined(_MSC_VER)
extern void msleep(unsigned);
#endif
#endif /* MICRO || WIN32 */

/* ### pcunix.c ### */
#if defined(MICRO)
extern void regularize(char *);
#if defined(PC_LOCKING)
extern void getlock(void);
#endif
#endif /* MICRO */

/* ### pickup.c ### */

extern int collect_obj_classes(char *, struct obj *, boolean,
                               boolean(*)(struct obj *), int *) NONNULLARG5;
extern boolean rider_corpse_revival(struct obj *, boolean) NO_NNARGS;
extern void force_decor(boolean);
extern void deferred_decor(boolean);
extern boolean menu_class_present(int);
extern void add_valid_menu_class(int);
extern boolean allow_all(struct obj *) NO_NNARGS;
extern boolean allow_category(struct obj *) NONNULLARG1;
extern boolean is_worn_by_type(struct obj *) NONNULLARG1;
extern int ck_bag(struct obj *) NONNULLARG1;
extern void removed_from_icebox(struct obj *) NONNULLARG1;
/* reset_justpicked() is sometimes passed gi.invent
 * which can be null */
extern void reset_justpicked(struct obj *) NO_NNARGS;
/* sometimes count_justpicked(gi.invent) which can be null */
extern int count_justpicked(struct obj *) NO_NNARGS;
/* sometimes find_justpicked(gi.invent) which can be null */
extern struct obj *find_justpicked(struct obj *) NO_NNARGS;
extern int pickup(int);
extern int pickup_object(struct obj *, long, boolean) NONNULLARG1;
extern int query_category(const char *, struct obj *, int, menu_item **, int) NONNULLARG14;
/* dotypeinv() call query_objlist with NULL arg1 */
extern int query_objlist(const char *, struct obj **, int, menu_item **, int,
                         boolean(*)(struct obj *)) NONNULLARG24;
extern boolean reroll_menu(void);
extern struct obj *pick_obj(struct obj *) NONNULLARG1;
extern void encumber_msg(void);
extern int container_at(coordxy, coordxy, boolean);
extern int doloot(void);
extern void observe_quantum_cat(struct obj *, boolean, boolean) NONNULLARG1;
extern boolean container_gone(int(*)(struct obj *)) NONNULLARG1;
extern boolean u_handsy(void);
extern int use_container(struct obj **, boolean, boolean) NONNULLARG1;
extern int loot_mon(struct monst *, int *, boolean *) NO_NNARGS;
extern int dotip(void);
extern struct autopickup_exception *check_autopickup_exceptions(struct obj *) NONNULLARG1;
extern boolean autopick_testobj(struct obj *, boolean) NONNULLARG1;
extern boolean u_safe_from_fatal_corpse(struct obj *obj, int) NONNULLARG1;

/* ### pline.c ### */

#ifdef DUMPLOG_CORE
extern void dumplogmsg(const char *);
extern void dumplogfreemessages(void);
#endif
extern void pline(const char *, ...) PRINTF_F(1, 2);
extern void pline_dir(int, const char *, ...) PRINTF_F(2, 3);
extern void pline_xy(coordxy, coordxy, const char *, ...) PRINTF_F(3, 4);
extern void pline_mon(struct monst *, const char *, ...) PRINTF_F(2, 3) NONNULLARG1;
extern void set_msg_dir(int);
extern void set_msg_xy(coordxy, coordxy);
extern void custompline(unsigned, const char *, ...) PRINTF_F(2, 3);
extern void urgent_pline(const char *, ...) PRINTF_F(1, 2);
extern void Norep(const char *, ...) PRINTF_F(1, 2);
extern void free_youbuf(void);
extern void You(const char *, ...) PRINTF_F(1, 2);
extern void Your(const char *, ...) PRINTF_F(1, 2);
extern void You_feel(const char *, ...) PRINTF_F(1, 2);
extern void You_cant(const char *, ...) PRINTF_F(1, 2);
extern void You_hear(const char *, ...) PRINTF_F(1, 2);
extern void You_see(const char *, ...) PRINTF_F(1, 2);
extern void pline_The(const char *, ...) PRINTF_F(1, 2);
extern void There(const char *, ...) PRINTF_F(1, 2);
extern void verbalize(const char *, ...) PRINTF_F(1, 2);
extern void gamelog_add(long, long, const char *);
extern void livelog_printf(long, const char *, ...) PRINTF_F(2, 3);
extern void raw_printf(const char *, ...) PRINTF_F(1, 2);
extern void impossible(const char *, ...) PRINTF_F(1, 2);
extern void config_error_add(const char *, ...) PRINTF_F(1, 2);
extern void nhassert_failed(const char *, const char *, int);

/* ### polyself.c ### */

extern void set_uasmon(void);
extern void float_vs_flight(void);
extern void steed_vs_stealth(void);
extern void change_sex(void);
extern void livelog_newform(boolean, int, int);
extern void polyself(int);
extern int polymon(int);
extern schar uasmon_maxStr(void);
extern void rehumanize(void);
extern int dobreathe(void);
extern int dospit(void);
extern int doremove(void);
extern int dospinweb(void);
extern int dosummon(void);
extern int dogaze(void);
extern int dohide(void);
extern int dopoly(void);
extern int domindblast(void);
extern void uunstick(void);
extern void skinback(boolean);
extern const char *mbodypart(struct monst *, int) NONNULLARG1;
extern const char *body_part(int);
extern int poly_gender(void);
extern void ugolemeffects(int, int);
extern boolean ugenocided(void);
extern const char *udeadinside(void);

/* ### potion.c ### */

extern void set_itimeout(long *, long) NONNULLARG1;
extern void incr_itimeout(long *, int) NONNULLARG1;
extern void make_confused(long, boolean);
extern void make_stunned(long, boolean);
extern void make_sick(long, const char *, boolean, int) NO_NNARGS;
extern void make_slimed(long, const char *) NO_NNARGS;
extern void make_stoned(long, const char *, int, const char *) NO_NNARGS;
extern void make_vomiting(long, boolean);
extern void make_blinded(long, boolean);
extern void toggle_blindness(void);
extern boolean make_hallucinated(long, boolean, long);
extern void make_deaf(long, boolean);
extern void make_glib(int);
extern void self_invis_message(void);
extern int dodrink(void);
extern int dopotion(struct obj *) NONNULLARG1;
extern int peffects(struct obj *) NONNULLARG1;
extern void healup(int, int, boolean, boolean);
extern void strange_feeling(struct obj *, const char *) NO_NNARGS;
extern void impact_arti_light(struct obj *, boolean, boolean) NONNULLARG1;
extern void potionhit(struct monst *, struct obj *, int) NONNULLARG12;
extern void potionbreathe(struct obj *) NONNULLARG1;
extern int dodip(void);
extern int dip_into(void); /* altdip */
extern void mongrantswish(struct monst **) NONNULLARG1;
extern void djinni_from_bottle(struct obj *) NONNULLARG1;
extern struct monst *split_mon(struct monst *, struct monst *) NONNULLARG1;
extern const char *bottlename(void);
extern void speed_up(long);

/* ### pray.c ### */

extern boolean critically_low_hp(boolean);
extern boolean stuck_in_wall(void);
extern void desecrate_altar(boolean, aligntyp);
extern int dosacrifice(void);
extern boolean can_pray(boolean);
extern int dopray(void);
extern const char *u_gname(void);
extern int doturn(void);
extern int altarmask_at(coordxy, coordxy);
extern const char *a_gname(void);
extern const char *a_gname_at(coordxy x, coordxy y);
extern const char *align_gname(aligntyp);
extern const char *halu_gname(aligntyp);
extern const char *align_gtitle(aligntyp);
extern void altar_wrath(coordxy, coordxy);

/* ### priest.c ### */

extern int move_special(struct monst *, boolean, schar, boolean, boolean,
                        coordxy, coordxy, coordxy, coordxy) NONNULLARG1;
extern char temple_occupied(char *) NONNULLARG1;
extern boolean inhistemple(struct monst *) NO_NNARGS;
extern int pri_move(struct monst *) NONNULLARG1;
extern void priestini(d_level *, struct mkroom *, int, int, boolean) NONNULLARG12;
extern aligntyp mon_aligntyp(struct monst *) NONNULLARG1;
extern char *priestname(struct monst *, int, boolean, char *) NONNULLARG1;
extern boolean p_coaligned(struct monst *) NONNULLARG1;
extern struct monst *findpriest(char);
extern void intemple(int);
extern void forget_temple_entry(struct monst *) NONNULLARG1;
extern void priest_talk(struct monst *) NONNULLARG1;
extern struct monst *mk_roamer(struct permonst *, aligntyp, coordxy, coordxy,
                               boolean) NO_NNARGS;
extern void reset_hostility(struct monst *) NONNULLARG1;
extern boolean in_your_sanctuary(struct monst *, coordxy, coordxy) NO_NNARGS;
extern void ghod_hitsu(struct monst *) NONNULLARG1;
extern void angry_priest(void);
extern void clearpriests(void);
extern void restpriest(struct monst *, boolean) NONNULLARG1;
extern void newepri(struct monst *) NONNULLARG1;
extern void free_epri(struct monst *) NONNULLARG1;

/* ### quest.c ### */

extern void onquest(void);
extern void nemdead(void);
extern void leaddead(void);
extern void artitouch(struct obj *) NONNULLARG1;
extern boolean ok_to_quest(void);
extern void leader_speaks(struct monst *) NONNULLARG1;
extern void nemesis_speaks(void);
extern void nemesis_stinks(coordxy, coordxy);
extern void quest_chat(struct monst *) NONNULLARG1;
extern void quest_talk(struct monst *) NONNULLARG1;
extern void quest_stat_check(struct monst *) NONNULLARG1;
extern void finish_quest(struct obj *) NO_NNARGS;

/* ### questpgr.c ### */

extern void load_qtlist(void);
extern void unload_qtlist(void);
extern short quest_info(int);
extern const char *ldrname(void);
extern boolean is_quest_artifact(struct obj *) NONNULLARG1;
extern struct obj *find_quest_artifact(unsigned);
extern int stinky_nemesis(struct monst *);
extern void com_pager(const char *);
extern void qt_pager(const char *);
extern struct permonst *qt_montype(void);
extern void deliver_splev_message(void);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
#ifndef CROSS_TO_AMIGA
extern void srandom(unsigned);
extern char *initstate(unsigned, char *, int);
extern char *setstate(char *);
extern long random(void);
#endif /* CROSS_TO_AMIGA */
#endif /* RANDOM */

/* ### read.c ### */

extern void learnscroll(struct obj *) NONNULLARG1;
extern char *tshirt_text(struct obj *, char *) NONNULLARG12;
extern char *hawaiian_motif(struct obj *, char *) NONNULLARG12;
extern char *apron_text(struct obj *, char *) NONNULLARG12;
extern const char *candy_wrapper_text(struct obj *) NONNULLARG1;
extern void assign_candy_wrapper(struct obj *) NONNULLARG1;
extern int doread(void);
extern int charge_ok(struct obj *) NO_NNARGS;
extern void recharge(struct obj *, int) NONNULLARG1;
extern boolean valid_cloud_pos(coordxy, coordxy);
extern int seffects(struct obj *) NONNULLARG1;
extern void drop_boulder_on_player(boolean, boolean, boolean, boolean);
extern boolean drop_boulder_on_monster(coordxy, coordxy, boolean, boolean);
extern void wand_explode(struct obj *, int) NONNULLARG1;
extern void litroom(boolean, struct obj *) NO_NNARGS;
extern void do_genocide(int);
extern void punish(struct obj *) NO_NNARGS;
extern void unpunish(void);
extern boolean cant_revive(int *, boolean, struct obj *) NO_NNARGS;
extern boolean create_particular(void);

/* ### rect.c ### */

extern void init_rect(void);
extern void free_rect(void);
extern NhRect *get_rect(NhRect *) NONNULLARG1;
extern NhRect *rnd_rect(void);
extern void rect_bounds(NhRect, NhRect, NhRect *) NONNULLARG3;
extern void remove_rect(NhRect *) NONNULLARG1;
extern void add_rect(NhRect *) NONNULLARG1;
extern void split_rects(NhRect *, NhRect *) NONNULLARG12;

/* ## region.c ### */

extern boolean inside_region(NhRegion *, int, int) NO_NNARGS;
extern void clear_regions(void);
extern void run_regions(void);
extern boolean in_out_region(coordxy, coordxy);
extern boolean m_in_out_region(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void update_player_regions(void);
extern void update_monster_region(struct monst *) NONNULLARG1;
extern int reg_damg(NhRegion *) NONNULLARG1;
extern boolean any_visible_region(void);
extern void visible_region_summary(winid);
extern NhRegion *visible_region_at(coordxy, coordxy);
extern void show_region(NhRegion *, coordxy, coordxy) NONNULLARG1;
extern void save_regions(NHFILE *) NONNULLARG1;
extern void rest_regions(NHFILE *) NONNULLARG1;
extern void region_stats(const char *, char *, long *, long *) NONNULLPTRS;
extern NhRegion *create_gas_cloud(coordxy, coordxy, int, int);
extern NhRegion *create_gas_cloud_selection(struct selectionvar *, int);
extern boolean region_danger(void);
extern void region_safety(void);

/* ### report.c ### */

#ifdef CRASHREPORT
extern boolean submit_web_report(int, const char *, const char *);
extern boolean submit_web_report(int, const char *, const char *);
extern void crashreport_init(int, char *[]);
extern void crashreport_bidshow(void);
extern boolean swr_add_uricoded(const char *, char **, int *, char *);
extern int dobugreport(void);
#endif /* CRASHREPORT */
# ifndef NO_SIGNAL
extern void panictrace_handler(int);
# endif
#ifdef PANICTRACE
extern const char *get_saved_pline(int);
extern boolean NH_panictrace_libc(void);
extern boolean NH_panictrace_gdb(void);
#if defined(PANICTRACE) && !defined(NO_SIGNAL)
extern void panictrace_setsignals(boolean);
#endif
#endif /* PANICTRACE */

/* ### restore.c ### */

extern void inven_inuse(boolean);
extern int dorecover(NHFILE *) NONNULLARG1;
extern void restcemetery(NHFILE *, struct cemetery **) NONNULLARG12;
extern void trickery(char *) NO_NNARGS;
extern void getlev(NHFILE *, int, xint8) NONNULLARG1;
extern void get_plname_from_file(NHFILE *, char *, boolean) NONNULLARG12;
#ifdef SELECTSAVED
extern int restore_menu(winid);
#endif
extern boolean lookup_id_mapping(unsigned, unsigned *) NONNULLARG2;
/* extern void reset_restpref(void); */
/* extern void set_restpref(const char *); */
/* extern void set_savepref(const char *); */
#ifdef SFCTOOL
void rest_bubbles(NHFILE *);
void restore_gamelog(NHFILE *);
boolean restgamestate(NHFILE *);
void restore_msghistory(NHFILE *);
#endif
extern void rest_adjust_levelflags(long);
extern void moves_to_relative_time(long *);
extern void relative_time_to_moves(long *);
extern boolean revision_increment(int, int, uchar *);

/* ### revision.c ### */

extern boolean revision_increment(int, int, uchar *);
#ifdef DEMO_UPLIFTS
void uplift_mystruct_rev0_to_mystruct(struct mystruct_rev0 *rev0,
                                      struct mystruct *rev1);
#endif /* DEMO_UPLIFTS */

/* ### rip.c ### */

extern void genl_outrip(winid, int, time_t);

/* ### rnd.c ### */

#ifdef USE_ISAAC64
extern void init_isaac64(unsigned long, int(*fn)(int));
extern long nhrand(void);
#endif
extern int rn2(int);
extern int rn2_on_display_rng(int);
extern int rnd_on_display_rng(int);
extern int rnl(int);
extern int rnd(int);
extern int d(int, int);
extern int rne(int);
extern int rnz(int);
extern void init_random(int(*fn)(int));
extern void reseed_random(int(*fn)(int));
extern void shuffle_int_array(int *, int) NONNULLARG1;

/* ### role.c ### */

extern boolean validrole(int);
extern boolean validrace(int, int);
extern boolean validgend(int, int, int);
extern boolean validalign(int, int, int);
extern int randrole(boolean);
extern int randrace(int);
extern int randgend(int, int);
extern int randalign(int, int);
extern int str2role(const char *) NO_NNARGS;
extern int str2race(const char *) NO_NNARGS;
extern int str2gend(const char *) NO_NNARGS;
extern int str2align(const char *) NO_NNARGS;
extern boolean ok_role(int, int, int, int);
extern int pick_role(int, int, int, int);
extern boolean ok_race(int, int, int, int);
extern int pick_race(int, int, int, int);
extern boolean ok_gend(int, int, int, int);
extern int pick_gend(int, int, int, int);
extern boolean ok_align(int, int, int, int);
extern int pick_align(int, int, int, int);
extern void rigid_role_checks(void);
extern boolean setrolefilter(const char *) NONNULLARG1;
extern boolean gotrolefilter(void);
extern char *rolefilterstring(char *, int) NONNULLARG1;
extern void clearrolefilter(int);
extern char *root_plselection_prompt(char *, int, int, int, int, int) NO_NNARGS;
extern char *build_plselection_prompt(char *, int, int, int, int, int) NONNULLARG1;
extern void plnamesuffix(void);
extern void role_selection_prolog(int, winid);
extern void role_menu_extra(int, winid, boolean);
extern void role_init(void);
extern const char *Hello(struct monst *) NO_NNARGS;
extern const char *Goodbye(void);
extern const struct Race *character_race(short);
extern void genl_player_selection(void);
extern int genl_player_setup(int);

/* ### rumors.c ### */

extern char *getrumor(int, char *, boolean) NONNULLARG2;
extern char *get_rnd_text(const char *, char *, int(*)(int),
                          unsigned) NONNULLPTRS;
extern void outrumor(int, int);
extern void outoracle(boolean, boolean);
extern void save_oracles(NHFILE *) NONNULLARG1;
extern void restore_oracles(NHFILE *) NONNULLARG1;
extern int doconsult(struct monst *) NO_NNARGS;
extern void rumor_check(void);
extern boolean CapitalMon(const char *) NO_NNARGS;
extern void free_CapMons(void);

/* ### save.c ### */

extern int dosave(void);
extern int dosave0(void);
extern boolean tricked_fileremoved(NHFILE *, char *) NONNULLARG2;
#ifdef INSURANCE
extern void savestateinlock(void);
#endif
extern void savelev(NHFILE *, xint8) NONNULLARG1;
/* extern genericptr_t mon_to_buffer(struct monst *, int *); */
extern void savecemetery(NHFILE *, struct cemetery **) NONNULLARG12;
extern void savefruitchn(NHFILE *) NONNULLARG1;
extern void store_plname_in_file(NHFILE *) NONNULLARG1;
extern void free_dungeons(void);
extern void freedynamicdata(void);
extern void store_savefileinfo(NHFILE *) NONNULLARG1;
extern void store_savefileinfo(NHFILE *) NONNULLARG1;
extern int nhdatatypes_size(void);
#if 0
extern void assignlog(char *, char*, int);
extern FILE *getlog(NHFILE *);
extern void closelog(NHFILE *);
#endif

/* ### selvar.c ### */

extern struct selectionvar *selection_new(void);
extern void selection_free(struct selectionvar *, boolean) NO_NNARGS;
extern void selection_clear(struct selectionvar *, int) NONNULLARG1;
extern struct selectionvar *selection_clone(struct selectionvar *) NONNULLARG1;
extern void selection_getbounds(struct selectionvar *, NhRect *) NO_NNARGS;
extern void selection_recalc_bounds(struct selectionvar *) NONNULLARG1;
extern coordxy selection_getpoint(coordxy, coordxy, struct selectionvar *) NO_NNARGS;
extern void selection_setpoint(coordxy, coordxy, struct selectionvar *, int);
extern struct selectionvar * selection_not(struct selectionvar *);
extern struct selectionvar *selection_filter_percent(struct selectionvar *,
                                                     int);
extern struct selectionvar *selection_filter_mapchar(struct selectionvar *,
                                                     xint16, int);
extern int selection_rndcoord(struct selectionvar *, coordxy *, coordxy *,
                              boolean);
extern void selection_do_grow(struct selectionvar *, int);
extern void set_selection_floodfillchk(int(*)(coordxy, coordxy));
extern void selection_floodfill(struct selectionvar *, coordxy, coordxy,
                                boolean);
extern void selection_do_ellipse(struct selectionvar *, int, int, int, int,
                                 int);
extern void selection_do_gradient(struct selectionvar *, long, long, long,
                                  long, long, long, long);
extern void selection_do_line(coordxy, coordxy, coordxy, coordxy,
                              struct selectionvar *);
extern void selection_do_randline(coordxy, coordxy, coordxy, coordxy,
                                  schar, schar, struct selectionvar *);
extern void selection_iterate(struct selectionvar *, select_iter_func,
                              genericptr_t);
extern boolean selection_is_irregular(struct selectionvar *);
extern char *selection_size_description(struct selectionvar *, char *);
extern struct selectionvar *selection_from_mkroom(struct mkroom *) NO_NNARGS;
extern void selection_force_newsyms(struct selectionvar *) NONNULLARG1;

/* ### sfstruct.c ### */

extern boolean close_check(int);
/* extern void newread(NHFILE *, int, int, genericptr_t, unsigned); */
extern void bufon(int);
extern void bufoff(int);
extern void bflush(int);
extern void bwrite(int, const genericptr_t, unsigned) NONNULLARG2;
extern void mread(int, genericptr_t, unsigned) NONNULLARG2;
extern void bclose(int);

/* ### shk.c ### */

/* setpaid() has a conditional code block near the end of the
   function, where arg1 is tested for NULL, preventing NONNULLARG1 */
extern void setpaid(struct monst *) NO_NNARGS;
extern void record_price_quote(int, unsigned long, boolean);
extern void append_price_quote(char *, char **, int) NONNULLARG12;
extern long money2mon(struct monst *, long) NONNULLARG1;
extern void money2u(struct monst *, long) NONNULLARG1;
extern void shkgone(struct monst *) NONNULLARG1;
extern void set_residency(struct monst *, boolean) NONNULLARG1;
extern void replshk(struct monst *, struct monst *) NONNULLARG12;
extern void restshk(struct monst *, boolean) NONNULLARG1;
extern char inside_shop(coordxy, coordxy);
extern void u_left_shop(char *, boolean) NONNULLARG1;
extern void remote_burglary(coordxy, coordxy);
extern void u_entered_shop(char *);
extern void pick_pick(struct obj *) NONNULLARG1;
extern boolean same_price(struct obj *, struct obj *) NONNULLARG12;
extern void shopper_financial_report(void);
extern int inhishop(struct monst *) NONNULLARG1;
extern struct monst *shop_keeper(char);
extern struct monst *find_objowner(struct obj *,
                                   coordxy x, coordxy y) NONNULLARG1;
extern boolean tended_shop(struct mkroom *) NONNULLARG1;
extern boolean onshopbill(struct obj *, struct monst *, boolean) NONNULLARG1;
extern boolean is_unpaid(struct obj *) NONNULLARG1;
extern void delete_contents(struct obj *) NONNULLARG1;
extern void obfree(struct obj *, struct obj *) NONNULLARG1;
extern void make_happy_shk(struct monst *, boolean) NONNULLARG1;
extern void make_happy_shoppers(boolean);
extern void hot_pursuit(struct monst *) NONNULLARG1;
extern void make_angry_shk(struct monst *, coordxy, coordxy) NONNULLARG1;
extern int dopay(void);
extern boolean paybill(int, boolean);
extern void finish_paybill(void);
extern struct obj *find_oid(unsigned);
extern long contained_cost(struct obj *, struct monst *, long, boolean,
                           boolean) NONNULLARG12;
extern long contained_gold(struct obj *, boolean) NONNULLARG1;
extern void picked_container(struct obj *) NONNULLARG1;
extern void gem_learned(int);
extern void alter_cost(struct obj *, long) NONNULLARG1;
extern long unpaid_cost(struct obj *, uchar) NONNULLARG1;
extern boolean billable(struct monst **, struct obj *, char,
                        boolean) NONNULLARG12;
extern void addtobill(struct obj *, boolean, boolean, boolean) NONNULLARG1;
extern void splitbill(struct obj *, struct obj *) NONNULLARG12;
extern void subfrombill(struct obj *, struct monst *) NONNULLARG12;
extern long stolen_value(struct obj *, coordxy, coordxy,
                         boolean, boolean) NONNULLARG1;
extern void donate_gold(long, struct monst *, boolean) NONNULLARG2;
extern void sellobj_state(int);
extern void sellobj(struct obj *, coordxy, coordxy) NONNULLARG1;
extern int doinvbill(int);
extern struct monst *shkcatch(struct obj *, coordxy, coordxy) NONNULLARG1;
extern void add_damage(coordxy, coordxy, long);
extern void fix_shop_damage(void);
extern int shk_move(struct monst *) NONNULLARG1;
extern void after_shk_move(struct monst *) NONNULLARG1;
extern boolean is_fshk(struct monst *) NONNULLARG1;
extern void shopdig(int);
extern void pay_for_damage(const char *, boolean);
extern boolean costly_spot(coordxy, coordxy);
/* costly_adjacent() has checks for null 1st arg, and an early return,
   so it cannot be NONNULLARG1 */
extern boolean costly_adjacent(struct monst *, coordxy, coordxy) NO_NNARGS;
extern struct obj *shop_object(coordxy, coordxy);
extern void price_quote(struct obj *) NONNULLARG1;
extern void shk_chat(struct monst *) NONNULLARG1;
extern void check_unpaid_usage(struct obj *, boolean) NONNULLARG1;
extern void check_unpaid(struct obj *) NONNULLARG1;
extern void costly_gold(coordxy, coordxy, long, boolean);
extern long get_cost_of_shop_item(struct obj *, int *) NONNULLARG1;
extern int oid_price_adjustment(struct obj *, unsigned) NONNULLARG1;
extern boolean block_door(coordxy, coordxy);
extern boolean block_entry(coordxy, coordxy);
extern char *shk_your(char *, struct obj *) NONNULLPTRS;
extern char *Shk_Your(char *, struct obj *) NONNULLPTRS;
extern void globby_bill_fixup(struct obj *, struct obj *) NONNULLARG12;
/*extern void globby_donation(struct obj *, struct obj *); */
extern void credit_report(struct monst *shkp, int idx,
                          boolean silent) NONNULLARG1;
extern void use_unpaid_trapobj(struct obj *, coordxy, coordxy) NONNULLARG1;
extern void noisy_shop(struct mkroom *);
extern const char *says(void);


/* ### shknam.c ### */

extern void neweshk(struct monst *) NONNULLARG1;
extern void free_eshk(struct monst *) NONNULLARG1;
extern void stock_room(int, struct mkroom *) NONNULLARG2;
extern boolean saleable(struct monst *, struct obj *) NONNULLARG12;
extern int get_shop_item(int);
extern char *Shknam(struct monst *) NONNULLARG1;
extern char *shkname(struct monst *) NONNULLARG1;
extern boolean shkname_is_pname(struct monst *) NONNULLARG1;
extern boolean is_izchak(struct monst *, boolean) NONNULLARG1;

/* ### sit.c ### */

extern void take_gold(void);
extern int dosit(void);
extern void rndcurse(void);
extern int attrcurse(void);

/* ### sounds.c ### */

extern void dosounds(void);
extern const char *growl_sound(struct monst *) NONNULLARG1;
extern void growl(struct monst *) NONNULLARG1;
extern void yelp(struct monst *) NONNULLARG1;
extern void whimper(struct monst *) NONNULLARG1;
extern void beg(struct monst *) NONNULLARG1;
extern const char *maybe_gasp(struct monst *) NONNULLARG1;
extern const char *cry_sound(struct monst *) NONNULLARG1;
extern int domonnoise(struct monst *) NONNULLARG1;
extern int dotalk(void);
extern int tiphat(void);
#ifdef USER_SOUNDS
extern int add_sound_mapping(const char *) NONNULLARG1;
extern void play_sound_for_message(const char *) NONNULLARG1;
extern void maybe_play_sound(const char *) NONNULLARG1;
extern void release_sound_mappings(void);
#if defined(WIN32) || defined(QT_GRAPHICS)
extern void play_usersound(const char *, int);
#endif
#if defined(TTY_SOUND_ESCCODES)
extern void play_usersound_via_idx(int, int);
#endif
#endif /* USER SOUNDS */
extern void assign_soundlib(int);
extern void activate_chosen_soundlib(void);
extern void get_soundlib_name(char *dest, int maxlen) NONNULLARG1;
#ifdef SND_SOUNDEFFECTS_AUTOMAP
extern char *get_sound_effect_filename(int32_t seidint,
                                       char *buf, size_t bufsz, int32_t);
#endif
extern char *base_soundname_to_filename(char *, char *, size_t, int32_t) NONNULLARG1;
extern void set_voice(struct monst *, int32_t, int32_t, int32_t) NO_NNARGS;
extern void sound_speak(const char *) NO_NNARGS;
extern enum soundlib_ids soundlib_id_from_opt(char *);

/* ### sp_lev.c ### */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
extern boolean match_maptyps(xint16, xint16);
extern void create_des_coder(void);
extern void reset_xystart_size(void);
extern struct mapfragment *mapfrag_fromstr(char *) NONNULLARG1;
extern void mapfrag_free(struct mapfragment **) NO_NNARGS;
extern schar mapfrag_get(struct mapfragment *, int, int) NONNULLARG1;
extern boolean mapfrag_canmatch(struct mapfragment *) NONNULLARG1;
extern const char * mapfrag_error(struct mapfragment *) NO_NNARGS;
extern boolean mapfrag_match(struct mapfragment *, int, int) NONNULLARG1;
extern void flip_level(int, boolean);
extern void flip_level_rnd(int, boolean);
extern boolean check_room(coordxy *, coordxy *, coordxy *, coordxy *, boolean) NONNULLPTRS;
extern boolean create_room(coordxy, coordxy, coordxy, coordxy,
                           coordxy, coordxy, xint16, xint16);
extern boolean dig_corridor(coord *, coord *, int *, boolean, schar, schar) NONNULLARG12;
extern void fill_special_room(struct mkroom *) NO_NNARGS;
extern void wallify_map(coordxy, coordxy, coordxy, coordxy);
extern boolean load_special(const char *) NONNULLARG1;
extern coordxy random_wdir(void);
extern boolean pm_good_location(coordxy, coordxy, struct permonst *) NONNULLARG3;
extern void get_location_coord(coordxy *, coordxy *, int, struct mkroom *,
                               long) NONNULLARG12;
extern void set_floodfillchk_match_under(coordxy);
extern int lspo_reset_level(lua_State *) NO_NNARGS; /* wiz_load_splua NULL */
/* lspo_finalize_level() has tests for whether arg1 L is null, and chooses
   code paths to follow based on that. Also preventing NONNULLARG1 is it
   being called from wiz_load_splua() with a NULL arg.
   Side note: The parameter is also marked as UNUSED, but apparently it is */
extern int lspo_finalize_level(lua_State *) NO_NNARGS;
extern boolean get_coord(lua_State *, int, lua_Integer *, lua_Integer *) NONNULLPTRS;
extern void cvt_to_abscoord(coordxy *, coordxy *) NONNULLPTRS;
extern void cvt_to_relcoord(coordxy *, coordxy *) NONNULLPTRS;
extern int nhl_abs_coord(lua_State *) NONNULLARG1;
extern void update_croom(void);
extern const char *get_trapname_bytype(int);
extern void l_register_des(lua_State *) NONNULLARG1;
extern int get_table_objclass(lua_State *) NONNULLARG1;
extern int get_table_objtype(lua_State *) NONNULLARG1;
#endif /* !CROSSCOMPILE || CROSSCOMPILE_TARGET */

/* ### spell.c ### */

extern void book_cursed(struct obj *) NONNULLARG1;
extern int study_book(struct obj *) NONNULLARG1;
extern void book_disappears(struct obj *) NONNULLARG1;
extern void book_substitution(struct obj *, struct obj *) NONNULLARG12;
extern void age_spells(void);
extern int dowizcast(void);
extern int docast(void);
extern int spell_skilltype(int);
extern int spelleffects(int, boolean, boolean);
extern int tport_spell(int);
extern void losespells(void);
extern int dovspell(void);
extern void show_spells(void);
extern void initialspell(struct obj *) NONNULLARG1;
extern int known_spell(short);
extern int spell_idx(short);
extern char force_learn_spell(short);
extern int num_spells(void);
extern void skill_based_spellbook_id(void);

/* ### stairs.c ### */

/**
 * @name The level's ways up and down
 * @brief Record a staircase, and find one by any of the several things one might know about it.
 *
 * A level's stairs are held as a list rather than on the map, because a staircase is more than a square: it knows which level it leads to and whether it is a ladder or a branch. So the map says
 * something is here and this says what.
 *
 * The several finding forms are the several questions that arise: which stairs are at this square, which lead to that level, which go up, which of a particular kind go up. Each exists because some
 * piece of code has exactly that much information and no more.
 *
 * @note The branch and special forms are separate because a branch's stairs are not interchangeable with the ordinary ones -- descending the mines is not descending the dungeon, even though both are
 *       downward.
 * @{
 */
/**
 * @name 레벨의 오르내리는 길
 * @brief 계단을 기록하고, 그것에 대해 알 만한 여러 가지 중 무엇으로든 하나를 찾는다.
 *
 * 레벨의 계단은 지도가 아니라 목록으로 보관된다. 계단이 칸 이상이기 때문이다. 그것은 자신이 어느 레벨로 이어지는지와 사다리인지 분기인지를 안다. 그래서 지도는 여기 무언가가 있다고 말하고 이것이 그것이 무엇인지 말한다.
 *
 * 여러 찾기 형태는 생기는 여러 질문이다. 이 칸에 어느 계단이 있는지, 어느 것이 그 레벨로 이어지는지, 어느 것이 올라가는지, 특정 종류 중 어느 것이 올라가는지. 각각이 존재하는 것은 어떤 코드가 정확히 그만큼의 정보를 가지고 그 이상은 갖지 않기 때문이다.
 *
 * @note 분기 형태와 특수 형태가 따로 있는 것은, 분기의 계단이 평범한 것과 바꿔 쓸 수 없기 때문이다. 광산을 내려가는 것은 던전을 내려가는 것이 아니다. 둘 다 아래쪽이더라도.
 * @{
 */
extern void stairway_add(coordxy, coordxy,
                         boolean, boolean, d_level *) NONNULLPTRS;
extern void stairway_free_all(void);
extern stairway *stairway_at(coordxy, coordxy);
extern stairway *stairway_find(d_level *) NONNULLARG1;
extern stairway *stairway_find_from(d_level *, boolean) NONNULLARG1;
extern stairway *stairway_find_dir(boolean);
extern stairway *stairway_find_type_dir(boolean, boolean);
extern stairway *stairway_find_special_dir(boolean);
/** @} */
extern void u_on_sstairs(int);
extern void u_on_upstairs(void);
extern void u_on_dnstairs(void);
extern boolean On_stairs(coordxy, coordxy);
extern boolean On_ladder(coordxy, coordxy);
extern boolean On_stairs_up(coordxy, coordxy);
extern boolean On_stairs_dn(coordxy, coordxy);
/**
 * @brief Whether the hero knows that a staircase leads out of this dungeon.
 * @note Knowing that stairs are there and knowing where they go are separate. A branch's stairs look like any others until they have been taken, so the overview cannot mark them until then.
 */
/**
 * @brief 영웅이 어떤 계단이 이 던전 밖으로 이어진다는 것을 아는지.
 * @note 계단이 있다는 것을 아는 것과 그것이 어디로 가는지 아는 것은 별개다. 분기의 계단은 지나가 보기 전까지 다른 것과 똑같아 보이므로, 개요가 그때까지는 그것을 표시할 수 없다.
 */
extern boolean known_branch_stairs(stairway *);
/**
 * @brief Describe a staircase, writing into the caller's buffer.
 * @note Its boolean asks whether to include where the stairs lead, which the hero may not know -- so the same routine serves both a description of what is here and a fuller one for the overview.
 */
/**
 * @brief 계단을 기술하며, 호출자의 버퍼에 쓴다.
 * @note 그 논리값은 그 계단이 어디로 이어지는지를 포함할지 묻는다. 영웅이 그것을 모를 수 있다. 그래서 같은 루틴이 여기 무엇이 있는지에 대한 기술과 개요를 위한 더 온전한 기술을 함께 맡는다.
 */
extern char *stairs_description(stairway *, char *, boolean) NONNULLARG1;

/* ### steal.c ### */

extern long somegold(long);
extern void stealgold(struct monst *) NONNULLARG1;
extern void thiefdead(void);
/**
 * @brief Whether the hero is in no state to resist what is being done to them.
 * @note Gathers the several conditions that amount to helplessness -- sleep, paralysis, being fainted -- because a thief, a nurse and a few other things all need the same question and none of them
 *       cares which condition it is.
 */
/**
 * @brief 영웅이 자신에게 가해지는 것에 저항할 수 없는 상태인지.
 * @note 무력함에 해당하는 여러 상태 -- 잠, 마비, 기절 -- 를 모은다. 도둑, 간호사, 그 밖의 몇 가지가 모두 같은 질문을 필요로 하고 그 중 어느 것도 그것이 어느 상태인지 신경 쓰지 않기 때문이다.
 */
extern boolean unresponsive(void);
/**
 * @brief Take a worn item off, whether or not the hero agreed to it.
 * @note For theft and for forced removal. It does the bookkeeping of unwearing without the checks and the delay of the hero choosing to -- so it must not be used for an ordinary removal, which the
 *       player is allowed to be told about and to spend time on.
 */
/**
 * @brief 착용한 물건을 벗긴다. 영웅이 동의했는지와 무관하게.
 * @note 도둑질과 강제 탈거를 위한 것이다. 영웅이 스스로 택할 때의 검사와 지연 없이 벗기의 기록만을 한다. 그래서 평범한 탈거에 써서는 안 된다. 그것은 플레이어가 알려질 수 있고 시간을 들일 수 있는 것이다.
 */
extern void remove_worn_item(struct obj *, boolean) NONNULLARG1;
/**
 * @brief Have a monster steal something from the hero.
 * @return whether the thief should now flee -- a successful theft is usually followed by escape
 * @note The second argument receives the name of what was taken, because the caller composes the message and the theft has to happen before the name can be given. So the name comes back rather than
 *       being printed here.
 */
/**
 * @brief 몬스터가 영웅에게서 무언가를 훔치게 한다.
 * @return 그 도둑이 이제 달아나야 하는지. 성공한 도둑질에는 보통 도주가 따른다
 * @note 두 번째 인자가 무엇이 빼앗겼는지의 이름을 받는다. 호출자가 메시지를 짓고, 그 이름이 주어질 수 있기 전에 도둑질이 일어나야 하기 때문이다. 그래서 그 이름이 여기서 인쇄되는 대신 되돌려진다.
 */
extern int steal(struct monst *, char *) NONNULLARG1;
/* mpickobj() contains a test for NULL arg2 obj and a code path
   that leads to impossible(). Prevents NONNULLARG12. */
extern int mpickobj(struct monst *, struct obj *) NONNULLARG1;
extern void stealamulet(struct monst *) NONNULLARG1;
extern void maybe_absorb_item(struct monst *, struct obj *, int, int) NONNULLARG12;
extern void mdrop_obj(struct monst *, struct obj *, boolean) NONNULLARG12;
extern void mdrop_special_objs(struct monst *) NONNULLARG1;
/**
 * @brief Have a monster put down everything it is carrying.
 * @note Used when a monster dies or is removed, and its middle argument says how far the objects may scatter -- because a monster that exploded should not leave a neat pile.
 */
/**
 * @brief 몬스터가 지니고 있는 모든 것을 내려놓게 한다.
 * @note 몬스터가 죽거나 없애질 때 쓰이며, 가운데 인자가 그 물건들이 얼마나 멀리 흩어져도 되는지를 말한다. 폭발한 몬스터가 깔끔한 무더기를 남겨서는 안 되기 때문이다.
 */
extern void relobj(struct monst *, int, boolean) NONNULLARG1;
/**
 * @brief The gold in a chain of objects, if any.
 * @warning Accepts null, and the annotation records that this was examined: it is often asked of a monster's possessions, which may be empty. An empty chain has no gold rather than being an error.
 */
/**
 * @brief 물건 사슬 안의 금화. 있다면.
 * @warning 널을 받아들이며, 그 표시가 이것이 검토되었음을 기록한다. 몬스터의 소지품에 대해 자주 물어지고 그것이 비어 있을 수 있다. 빈 사슬은 오류가 아니라 금화가 없는 것이다.
 */
extern struct obj *findgold(struct obj *) NO_NNARGS;

/* ### steed.c ### */

extern void rider_cant_reach(void);
extern boolean can_saddle(struct monst *) NONNULLARG1;
extern int use_saddle(struct obj *) NONNULLARG1;
extern void put_saddle_on_mon(struct obj *, struct monst *) NONNULLARG2;
extern boolean can_ride(struct monst *) NONNULLARG1;
extern int doride(void);
extern boolean mount_steed(struct monst *, boolean) NO_NNARGS;
extern void exercise_steed(void);
extern void kick_steed(void);
extern void dismount_steed(int);
/**
 * @brief Put a monster on a square, updating the grid that maps squares to monsters.
 *
 * The lowest layer of monster placement. The map holds a one-to-one association from square to monster, and this is what maintains it -- so a monster's coordinates and the grid entry that finds it
 * are set together and cannot drift apart.
 *
 * @warning It writes the grid, not the map's contents. Removing a monster from its old square is a separate operation and must have happened first, or the old entry is left pointing at a monster that
 *          has moved.
 * @note This is the function this fork's continuous-position work hooks, because it is the single point through which a monster's square changes -- which is what lets the vector position be kept in
 *       step without touching the movement rules.
 */
/**
 * @brief 몬스터를 어떤 칸에 놓으며, 칸에서 몬스터로의 대응 격자를 갱신한다.
 *
 * 몬스터 배치의 가장 낮은 층이다. 지도는 칸에서 몬스터로의 일대일 연관을 지니고, 이것이 그것을 유지하는 것이다. 그래서 몬스터의 좌표와 그것을 찾아 주는 격자 항목이 함께 설정되고 서로 어긋날 수 없다.
 *
 * @warning 지도의 내용이 아니라 그 격자에 쓴다. 몬스터를 예전 칸에서 없애는 것은 별개의 연산이며 먼저 일어나 있어야 한다. 그러지 않으면 예전 항목이 옮겨 간 몬스터를 계속 가리킨다.
 * @note 이 포크의 연속 위치 작업이 이 함수에 걸려 있다. 몬스터의 칸이 바뀌는 유일한 지점이기 때문이며, 그것이 이동 규칙을 건드리지 않고 벡터 위치를 보조 맞추게 하는 것이다.
 */
extern void place_monster(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void poly_steed(struct monst *, struct permonst *) NONNULLARG12;
/**
 * @brief Whether the hero's mount is stuck and so the hero cannot move either.
 * @note A rider's movement is the mount's, so anything holding the mount holds the rider. This exists because the hero's own held state does not record that -- the mount is what is held, and the
 *       hero is merely on it.
 */
/**
 * @brief 영웅의 탈것이 붙잡혀 있어서 영웅도 움직일 수 없는지.
 * @note 탄 자의 이동은 탈것의 것이므로, 탈것을 붙잡는 무엇이든 탄 자를 붙잡는다. 이것이 존재하는 것은 영웅 자신의 붙잡힘 상태가 그것을 기록하지 않기 때문이다. 붙잡힌 것은 탈것이고, 영웅은 그저 그 위에 있다.
 */
extern boolean stucksteed(boolean);

/* ### symbols.c ### */

extern void switch_symbols(int);
extern void assign_graphics(int);
extern void init_symbols(void);
extern void init_showsyms(void);
extern void init_primary_symbols(void);
extern void init_rogue_symbols(void);
extern void init_ov_primary_symbols(void);
extern void init_ov_rogue_symbols(void);
extern void clear_symsetentry(int, boolean);
extern void update_primary_symset(const struct symparse *, int) NONNULLARG1;
extern void update_rogue_symset(const struct symparse *, int) NONNULLARG1;
extern void update_ov_primary_symset(const struct symparse *, int) NONNULLARG1;
extern void update_ov_rogue_symset(const struct symparse *, int) NONNULLARG1;
extern int parse_sym_line(char *, int) NONNULLARG1;
extern nhsym get_othersym(int, int);
extern boolean symset_is_compatible(enum symset_handling_types, unsigned long);
extern void set_symhandling(char *handling, int which_set) NONNULLARG1;
extern boolean proc_symset_line(char *) NONNULLARG1;
extern int do_symset(boolean);
extern int load_symset(const char *, int) NONNULLARG1;
extern void free_symsets(void);
extern const struct symparse *match_sym(char *) NONNULLARG1;
extern void savedsym_free(void);
extern void savedsym_strbuf(strbuf_t *) NONNULLARG1;
extern boolean parsesymbols(char *, int) NONNULLARG1;

/* ### sys.c ### */

extern void sys_early_init(void);
extern void sysopt_release(void);
extern void sysopt_seduce_set(int);

/* ### teleport.c ### */

extern boolean noteleport_level(struct monst *) NONNULLARG1;
/* rloc_engr() passes NULL monst arg to goodpos()*/
extern boolean goodpos(coordxy, coordxy, struct monst *,
                       mmflags_nht) NO_NNARGS;
extern boolean enexto(coord *, coordxy, coordxy,
                      struct permonst *) NONNULLARG1;
extern boolean enexto_gpflags(coord *, coordxy, coordxy, struct permonst *,
                           mmflags_nht) NONNULLARG1;
extern boolean enexto_core(coord *, coordxy, coordxy, struct permonst *,
                           mmflags_nht) NONNULLARG1;
extern void teleds(coordxy, coordxy, int);
extern int collect_coords(coord *, coordxy, coordxy, int, unsigned,
                          boolean (*)(coordxy, coordxy)) NONNULLARG1;
extern boolean safe_teleds(int);
extern boolean teleport_pet(struct monst *, boolean) NONNULLARG1;
extern void tele_to_rnd_pet(void);
extern void tele(void);
extern void scrolltele(struct obj *) NO_NNARGS;
extern int dotelecmd(void);
extern int dotele(boolean);
extern void level_tele(void);
extern void domagicportal(struct trap *) NONNULLARG1;
extern void tele_trap(struct trap *) NONNULLARG1;
extern void level_tele_trap(struct trap *, unsigned) NONNULLARG1;
extern void rloc_to(struct monst *, coordxy, coordxy) NONNULLARG1;
extern void rloc_to_flag(struct monst *, coordxy, coordxy,
                         unsigned) NONNULLARG1;
extern boolean rloc(struct monst *, unsigned) NONNULLARG1;
extern boolean control_mon_tele(struct monst *, coord *cc, unsigned,
                                boolean) NONNULLARG1;
extern boolean tele_restrict(struct monst *) NONNULLARG1;
extern void mtele_trap(struct monst *, struct trap *, int) NONNULLARG12;
extern int mlevel_tele_trap(struct monst *, struct trap *,
                            boolean, int) NONNULLARG1;
extern boolean rloco(struct obj *) NONNULLARG1;
extern int random_teleport_level(void);
extern boolean u_teleport_mon(struct monst *, boolean) NONNULLARG1;

/* ### timeout.c ### */

extern const char *property_by_index(int, int *) NO_NNARGS;
extern void burn_away_slime(void);
extern void nh_timeout(void);
extern void fall_asleep(int, boolean);
extern void attach_egg_hatch_timeout(struct obj *, long) NONNULLARG1;
extern void attach_fig_transform_timeout(struct obj *) NONNULLARG1;
extern void kill_egg(struct obj *) NONNULLARG1;
extern void hatch_egg(union any *, long) NONNULLARG1;
extern void learn_egg_type(int);
extern void burn_object(union any *, long) NONNULLARG1;
extern void begin_burn(struct obj *, boolean) NONNULLARG1;
extern void end_burn(struct obj *, boolean) NONNULLARG1;
extern void do_storms(void);
/**
 * @name Timers
 * @brief Arrange for something to happen later, and cancel or inspect what is arranged.
 *
 * The game's mechanism for anything that happens at a time rather than in response to something: an egg hatching, a candle burning down, a figurine coming to life. A timer names what to do, when, and
 * what to do it to -- and the last is the generic value, so a timer may belong to an object, a monster or a square.
 *
 * Timers are held in one queue ordered by time, so running them is walking the front of the queue and no individual timer is polled. That is why an arbitrary number of them costs nothing per turn.
 *
 * @note The inspecting form exists because a timer's remaining time is sometimes information the player has -- how long a candle has left -- and the caller must be able to ask without disturbing it.
 * @warning A timer holds the generic value it was given, not a copy of what that value refers to. If the thing is destroyed without its timers being stopped, the timer fires on freed memory -- which
 *          is why the object routines below exist rather than each caller remembering.
 * @{
 */
/**
 * @name 타이머
 * @brief 무언가가 나중에 일어나도록 마련하고, 마련된 것을 취소하거나 살펴본다.
 *
 * 무언가에 반응해서가 아니라 어떤 시점에 일어나는 모든 것을 위한 게임의 기제다. 알이 부화하는 것, 양초가 다 타는 것, 인형이 살아나는 것. 타이머는 무엇을 할지, 언제, 무엇에 할지를 지칭하며, 마지막 것이 범용 값이다. 그래서 타이머가 물건이나 몬스터나 칸에 속할 수 있다.
 *
 * 타이머는 시간으로 정렬된 하나의 대기열에 보관되므로, 그것들을 돌리는 일은 대기열의 앞을 훑는 일이고 개별 타이머가 폴링되지 않는다. 그래서 그것이 몇 개든 턴마다 비용이 들지 않는다.
 *
 * @note 살펴보는 형태가 있는 것은, 타이머의 남은 시간이 때때로 플레이어가 가진 정보이기 때문이다. 양초가 얼마 남았는지. 그리고 호출자가 그것을 흐트러뜨리지 않고 물을 수 있어야 한다.
 * @warning 타이머는 자신이 받은 범용 값을 지니며, 그 값이 가리키는 것의 사본을 지니지 않는다. 그 대상이 자기 타이머들이 멈춰지지 않은 채로 파괴되면, 그 타이머가 해제된 메모리에 대해 발동한다. 그것이 호출자마다 기억하는 대신 아래의 물건 루틴들이 존재하는 이유다.
 * @{
 */
extern boolean start_timer(long, short, short, union any *) NONNULLARG4;
extern long stop_timer(short, union any *) NONNULLARG2;
extern long peek_timer(short, union any *) NONNULLARG2;
extern void run_timers(void);
/** @} */
/**
 * @name Keeping an object's timers with it
 * @brief Move, split or stop the timers belonging to an object, and ask whether it has one.
 *
 * These exist because an object's timers are not part of the object. They are entries in the shared queue holding a reference to it, so anything that changes the object's identity or existence has to
 * bring them along -- and forgetting to is a timer firing on something that is gone.
 *
 * The splitting form is the subtle one. When a stack is divided, a timer belonging to the stack has to be apportioned, and what that means depends on the timer: some are duplicated, some go with one
 * half.
 *
 * @{
 */
/**
 * @name 물건의 타이머를 그것과 함께 유지하기
 * @brief 물건에 속한 타이머를 옮기거나 나누거나 멈추고, 그것이 하나를 가졌는지 묻는다.
 *
 * 이들이 존재하는 것은 물건의 타이머가 그 물건의 일부가 아니기 때문이다. 그것들은 그것에 대한 참조를 지닌 공유 대기열의 항목이므로, 그 물건의 정체나 존재를 바꾸는 무엇이든 그것들을 함께 데려가야 한다. 그것을 잊는 것은 사라진 것에 대해 발동하는 타이머다.
 *
 * 나누는 형태가 미묘한 것이다. 묶음이 나뉘면 그 묶음에 속한 타이머가 배분되어야 하고, 그것이 무엇을 뜻하는지는 그 타이머에 달려 있다. 어떤 것은 복제되고 어떤 것은 한쪽과 함께 간다.
 *
 * @{
 */
extern void obj_move_timers(struct obj *, struct obj *) NONNULLARG12;
extern void obj_split_timers(struct obj *, struct obj *) NONNULLARG12;
extern void obj_stop_timers(struct obj *) NONNULLARG1;
extern boolean obj_has_timer(struct obj *, short) NONNULLARG1;
/** @} */
/**
 * @name Timers belonging to a square
 * @brief Stop a square's timers, and ask when one will fire or how long it has left.
 * @note A square's timers cannot be moved, only stopped -- which is why there is no moving form here as there is for objects. A square does not go anywhere.
 * @note The two asking forms differ in what they return: an absolute time and a remaining duration. Both are wanted because one is compared against the clock and the other is shown to the player.
 * @{
 */
/**
 * @name 칸에 속한 타이머
 * @brief 칸의 타이머를 멈추고, 그것이 언제 발동할지 또는 얼마 남았는지 묻는다.
 * @note 칸의 타이머는 옮겨질 수 없고 멈춰질 수만 있다. 그것이 물건에 대해서와 달리 여기에 옮기는 형태가 없는 이유다. 칸은 아무 데도 가지 않는다.
 * @note 두 묻는 형태는 반환하는 것이 다르다. 절대 시각과 남은 기간. 둘 다 필요한 것은 하나가 시계와 비교되고 다른 하나가 플레이어에게 보여지기 때문이다.
 * @{
 */
extern void spot_stop_timers(coordxy, coordxy, short);
extern long spot_time_expires(coordxy, coordxy, short);
extern long spot_time_left(coordxy, coordxy, short);
/** @} */
/**
 * @brief Whether an object belongs to this level rather than travelling with the hero.
 * @note Decides which timers are saved with the level and which with the game. An object in the hero's pack goes with them and its timers must too, so getting this wrong loses a timer or duplicates it
 *       across levels.
 */
/**
 * @brief 물건이 영웅과 함께 다니는 것이 아니라 이 레벨에 속하는지.
 * @note 어느 타이머가 레벨과 함께 저장되고 어느 것이 게임과 함께 저장되는지를 정한다. 영웅의 가방에 있는 물건은 그와 함께 가고 그 타이머도 그래야 하므로, 이것을 틀리면 타이머를 잃거나 레벨 사이에 복제하게 된다.
 */
extern boolean obj_is_local(struct obj *) NONNULLARG1;
/**
 * @name Saving and restoring timers
 * @brief Write out the timers and read them back, in the level's part or the game's.
 * @warning A timer refers to its target by pointer while running and cannot be saved that way. So restoring a timer leaves it referring to nothing until the relinking pass runs -- which is why that
 *          pass exists and why it is separate.
 * @{
 */
/**
 * @name 타이머를 저장하고 복원하기
 * @brief 타이머를 써 내고 되읽는다. 레벨의 부분에서든 게임의 부분에서든.
 * @warning 타이머는 돌아가는 동안 자기 대상을 포인터로 가리키며 그 방식으로 저장될 수 없다. 그래서 타이머를 복원하는 것은 다시 잇기 통과가 돌아가기 전까지 그것이 아무것도 가리키지 않는 상태로 남긴다. 그것이 그 통과가 존재하는 이유이고 따로 있는 이유다.
 * @{
 */
extern void save_timers(NHFILE *, int) NONNULLARG1;
extern void restore_timers(NHFILE *, int, long) NONNULLARG1;
/** @} */
extern void timer_stats(const char *, char *, long *, long *) NONNULLPTRS;
/**
 * @brief Turn restored timers' saved identifiers back into pointers.
 * @note The pass the restoring routines depend on. A timer saved with an identifier is useless until the thing it names has itself been restored, so the two cannot happen in one step and this is the
 *       second.
 */
/**
 * @brief 복원된 타이머의 저장된 식별자를 다시 포인터로 바꾼다.
 * @note 복원 루틴들이 의존하는 통과다. 식별자와 함께 저장된 타이머는 그것이 지칭하는 대상 자체가 복원되기 전까지 쓸모가 없으므로, 그 둘이 한 단계에서 일어날 수 없고 이것이 두 번째다.
 */
extern void relink_timers(boolean);
extern int wiz_timeout_queue(void);
/**
 * @brief Check that the timer queue is consistent and report anything wrong.
 * @note Exists because a timer's target is a reference the queue cannot validate on its own -- a timer for a freed object looks exactly like one for a live object. So the check is run deliberately
 *       rather than continuously, and it is what finds the class of bug the object routines above are meant to prevent.
 */
/**
 * @brief 타이머 대기열이 일관되는지 검사하고 잘못된 것을 알린다.
 * @note 타이머의 대상이 그 대기열이 스스로 검증할 수 없는 참조이기 때문에 존재한다. 해제된 물건에 대한 타이머는 살아 있는 물건에 대한 것과 정확히 똑같아 보인다. 그래서 이 검사는 끊임없이가 아니라 의도적으로 돌려지며, 위의 물건 루틴들이 막으려는 부류의 버그를 찾는 것이 이것이다.
 */
extern void timer_sanity_check(void);

/* ### topten.c ### */

extern void formatkiller(char *, unsigned, int, boolean) NONNULLARG1;
extern int observable_depth(d_level *) NONNULLARG1;
extern void topten(int, time_t);
extern void prscore(int, char **);
extern struct toptenentry *get_rnd_toptenentry(void);
extern struct obj *tt_oname(struct obj *) NO_NNARGS;
extern int tt_doppel(struct monst *) NONNULLARG1;

/* ### track.c ### */

extern void initrack(void);
extern void settrack(void);
extern coord *gettrack(coordxy, coordxy);
extern boolean hastrack(coordxy, coordxy);
extern void save_track(NHFILE *) NONNULLARG1;
extern void rest_track(NHFILE *) NONNULLARG1;

/* ### trap.c ### */

extern boolean burnarmor(struct monst *) NO_NNARGS;
extern int erode_obj(struct obj *, const char *, int, int) NO_NNARGS;
extern boolean grease_protect(struct obj *, const char *,
                              struct monst *) NONNULLARG1;
extern struct trap *maketrap(coordxy, coordxy, int);
extern d_level *clamp_hole_destination(d_level *) NONNULLARG1;
extern void fall_through(boolean, unsigned);
extern struct monst *animate_statue(struct obj *, coordxy, coordxy,
                                    int, int *) NONNULLARG1;
extern struct monst *activate_statue_trap(struct trap *, coordxy, coordxy,
                                          boolean) NONNULLARG1;
extern int immune_to_trap(struct monst *, unsigned) NO_NNARGS; /* revisit */
extern void set_utrap(unsigned, unsigned);
extern void reset_utrap(boolean);
extern boolean wearing_iron_shoes(struct monst *);
extern boolean m_harmless_trap(struct monst *, struct trap *) NONNULLPTRS;
extern void dotrap(struct trap *, unsigned) NONNULLARG1;
extern void seetrap(struct trap *) NONNULLARG1;
extern void feeltrap(struct trap *) NONNULLARG1;
extern int mintrap(struct monst *, unsigned) NONNULLARG1;
extern void instapetrify(const char *) NO_NNARGS;
extern void minstapetrify(struct monst *, boolean) NONNULLARG1;
extern void selftouch(const char *) NONNULLARG1;
extern void mselftouch(struct monst *, const char *, boolean) NONNULLARG1;
extern void float_up(void);
extern void fill_pit(coordxy, coordxy);
extern int float_down(long, long);
extern void climb_pit(void);
extern boolean fire_damage(struct obj *, boolean,
                           coordxy, coordxy) NONNULLARG1;
extern int fire_damage_chain(struct obj *, boolean, boolean,
                            coordxy, coordxy) NO_NNARGS;
extern boolean lava_damage(struct obj *, coordxy, coordxy) NONNULLARG1;
/* acid_damage() has a test for NULL arg and early return if so,
   preventing NONNULLARG1 */
extern void acid_damage(struct obj *) NO_NNARGS;
extern int water_damage(struct obj *, const char *, boolean) NO_NNARGS;
extern void water_damage_chain(struct obj *, boolean) NO_NNARGS;
extern boolean rnd_nextto_goodpos(coordxy *, coordxy *,
                                  struct monst *) NONNULLPTRS;
extern void back_on_ground(boolean);
extern void rescued_from_terrain(int);
extern boolean drown(void);
extern void drain_en(int, boolean);
extern int dountrap(void);
extern int could_untrap(boolean, boolean);
extern void cnv_trap_obj(int, int, struct trap *, boolean) NONNULLARG3;
extern boolean into_vs_onto(int);
extern int untrap(boolean, coordxy, coordxy, struct obj *) NO_NNARGS;
extern boolean openholdingtrap(struct monst *, boolean *) NO_NNARGS;
extern boolean closeholdingtrap(struct monst *, boolean *) NO_NNARGS;
extern boolean openfallingtrap(struct monst *, boolean, boolean *) NONNULLARG3;
extern boolean chest_trap(struct obj *, int, boolean) NONNULLARG1;
extern void deltrap(struct trap *) NONNULLARG1;
extern boolean delfloortrap(struct trap *) NO_NNARGS;
extern struct trap *t_at(coordxy, coordxy);
extern int count_traps(int);
extern void b_trapped(const char *, int) NONNULLARG1;
extern boolean unconscious(void);
extern void blow_up_landmine(struct trap *) NONNULLARG1;
extern int launch_obj(short, coordxy, coordxy, coordxy, coordxy, int);
extern boolean launch_in_progress(void);
extern void force_launch_placement(void);
extern boolean uteetering_at_seen_pit(struct trap *) NO_NNARGS;
extern boolean uescaped_shaft(struct trap *) NO_NNARGS;
extern boolean lava_effects(void);
extern void sink_into_lava(void);
extern void sokoban_guilt(void);
extern const char * trapname(int, boolean);
extern void ignite_items(struct obj *) NO_NNARGS;
extern void trap_ice_effects(coordxy x, coordxy y, boolean ice_is_melting);
extern void trap_sanity_check(void);

/* ### u_init.c ### */

extern void u_init_misc(void);
extern void u_init_inventory_attrs(void);
extern void u_init_skills_discoveries(void);

/* ### uhitm.c ### */

extern void dynamic_multi_reason(struct monst *, const char *, boolean) NONNULLARG12;
extern void erode_armor(struct monst *, int) NONNULLARG1;
extern boolean attack_checks(struct monst *, struct obj *) NONNULLARG1;
extern void check_caitiff(struct monst *) NONNULLARG1;
extern void mon_maybe_unparalyze(struct monst *) NONNULLARG1;
extern int find_roll_to_hit(struct monst *, uchar, struct obj *,
                            int *, int *) NONNULLARG145;
extern boolean force_attack(struct monst *, boolean) NONNULLARG1;
extern boolean do_attack(struct monst *) NONNULLARG1;
extern boolean hmon(struct monst *, struct obj *, int, int) NONNULLARG1;
extern boolean shade_miss(struct monst *, struct monst *, struct obj *,
                          boolean, boolean) NONNULLARG12;
extern void mhitm_ad_rust(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_corr(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_dcay(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_dren(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_drli(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_fire(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_cold(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_elec(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_acid(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_sgld(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_tlpt(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
/* gazemm() calls mhitm_ad_blnd with a NULL 4th arg */
extern void mhitm_ad_blnd(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLARG123;
extern void mhitm_ad_curs(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_drst(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_drin(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_stck(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_wrap(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_plys(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_slee(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_slim(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_ench(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_slow(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_conf(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_poly(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_famn(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_pest(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_deth(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_halu(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_phys(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_ston(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_were(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_heal(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_stun(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_legs(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_dgst(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_samu(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_dise(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_sedu(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_ad_ssex(struct monst *, struct attack *, struct monst *,
                          struct mhitm_data *) NONNULLPTRS;
extern void mhitm_adtyping(struct monst *, struct attack *, struct monst *,
                           struct mhitm_data *) NONNULLPTRS;
extern boolean do_stone_u(struct monst *) NONNULLARG1;
extern void do_stone_mon(struct monst *, struct attack *, struct monst *,
                         struct mhitm_data *) NONNULLARG134;
extern int damageum(struct monst *, struct attack *, int) NONNULLARG12;
/* domove_fight_empty passes NULL to explum arg1 */
extern int explum(struct monst *, struct attack *) NONNULLARG2;
extern void missum(struct monst *, struct attack *, boolean) NONNULLARG12;
extern boolean m_is_steadfast(struct monst *) NONNULLARG1;
extern boolean mhitm_knockback(struct monst *, struct monst *,struct attack *,
                               int *, boolean) NONNULLPTRS;
extern int passive(struct monst *, struct obj *, boolean, boolean, uchar,
                   boolean) NONNULLARG1;
extern void passive_obj(struct monst *, struct obj *, struct attack *) NONNULLARG1;
extern void that_is_a_mimic(struct monst *, unsigned) NONNULLARG1;
extern void stumble_onto_mimic(struct monst *) NONNULLARG1;
extern boolean disguised_as_non_mon(struct monst *) NONNULLARG1;
extern boolean disguised_as_mon(struct monst *) NONNULLARG1;
extern int flash_hits_mon(struct monst *, struct obj *) NONNULLARG12;
extern void light_hits_gremlin(struct monst *, int) NONNULLARG1;


/* ### unixmain.c ### */
#ifdef UNIX
#ifdef PORT_HELP
extern void port_help(void);
#endif
extern void sethanguphandler(void(*)(int));
extern boolean authorize_wizard_mode(void);
extern boolean authorize_explore_mode(void);
extern void append_slash(char *) NONNULLARG1;
extern boolean check_user_string(const char *) NONNULLARG1;
extern char *get_login_name(void);
extern unsigned long sys_random_seed(void);
#endif /* UNIX */
ATTRNORETURN extern void after_opt_showpaths(const char *) NORETURN;

/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
extern void gettty(void);
extern void settty(const char *) NO_NNARGS;
extern void setftty(void);
extern void intron(void);
extern void introff(void);
ATTRNORETURN extern void error(const char *, ...) PRINTF_F(1, 2) NORETURN;
#ifdef ENHANCED_SYMBOLS
extern void tty_utf8graphics_fixup(void);
#endif
#endif /* UNIX || __BEOS__ */

/* ### unixunix.c ### */

#ifdef UNIX
extern void getlock(void);
extern void ask_about_panic_save(void);
extern void regularize(char *) NONNULLARG1;
#if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
extern void msleep(unsigned);
#endif
#ifdef SHELL
extern int dosh(void);
#endif /* SHELL */
#if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
extern int child(int);
#endif
#ifdef PANICTRACE
extern boolean file_exists(const char *) NONNULLARG1;
#endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
#ifdef GNOME_GRAPHICS
extern int hide_privileges(boolean);
#endif
#endif /* UNIX */

/* ### utf8map.c ### */

#ifdef ENHANCED_SYMBOLS
extern char *mixed_to_utf8(char *buf, size_t bufsz, const char *str,
                           int *) NONNULLARG1;
void free_all_glyphmap_u(void);
int set_map_u(glyph_map *gm, uint32 utf32ch, const uint8 *utf8str) NONNULLPTRS;
#endif /* ENHANCED_SYMBOLS */
extern void reset_customsymbols(void);

/* ### vault.c ### */

extern void newegd(struct monst *) NONNULLARG1;
extern void free_egd(struct monst *) NONNULLARG1;
extern boolean grddead(struct monst *) NONNULLARG1;
extern struct monst *findgd(void);
extern void vault_summon_gd(void);
extern char vault_occupied(char *) NONNULLARG1;
extern void uleftvault(struct monst *); /* NULL leads to impossible() */
extern void invault(void);
extern int gd_move(struct monst *) NONNULLARG1;
extern void paygd(boolean);
extern long hidden_gold(boolean);
extern boolean gd_sound(void);
extern void vault_gd_watching(unsigned int);

/* ### version.c ### */

extern char *version_string(char *, size_t bufsz) NONNULL NONNULLARG1;
extern char *getversionstring(char *, size_t bufsz) NONNULL NONNULLARG1;
extern char *status_version(char *, size_t, boolean) NONNULL NONNULLARG1;
extern int doversion(void);
extern int doextversion(void);
#ifdef MICRO
extern boolean comp_times(long);
#endif
extern boolean check_version(struct version_info *, const char *, boolean,
                             unsigned long) NONNULLARG1;
extern int uptodate(NHFILE *, const char *, unsigned long) NONNULLARG1;
extern void store_version(NHFILE *) NONNULLARG1;
extern unsigned long get_feature_notice_ver(char *) NO_NNARGS;
extern unsigned long get_current_feature_ver(void);
extern const char *copyright_banner_line(int) NONNULL;
extern void early_version_info(boolean);
extern void dump_version_info(void);
extern void store_critical_bytes(NHFILE *) NONNULLARG1;
extern int compare_critical_bytes(NHFILE *, int *, unsigned long) NONNULLARG1;
extern int get_critical_size_count(void);
extern int validate(NHFILE *, const char *, boolean, int) NONNULLARG1;

/* ### video.c ### */

#ifdef MSDOS
extern int assign_video(char *) NONNULLARG1;
#ifdef NO_TERMS
extern void gr_init(void);
extern void gr_finish(void);
#endif
extern void tileview(boolean);
#endif
#ifdef VIDEOSHADES
extern int assign_videoshades(char *) NONNULLARG1;
extern int assign_videocolors(char *) NONNULLARG1;
#endif

/* ### vision.c ### */

extern boolean get_viz_clear(int, int);
extern void vision_init(void);
extern int does_block(int, int, struct rm *) NONNULLARG3;
extern void vision_reset(void);
extern void vision_recalc(int);
extern void block_point(int, int);
extern void unblock_point(int, int);
extern void recalc_block_point(coordxy, coordxy);
extern boolean clear_path(int, int, int, int);
extern void do_clear_area(coordxy, coordxy, int,
                          void(*)(coordxy, coordxy, void *), genericptr_t);
extern unsigned howmonseen(struct monst *) NONNULLARG1;

#ifdef VMS

/* ### vmsfiles.c ### */

extern int vms_link(const char *, const char *);
extern int vms_unlink(const char *);
extern int vms_creat(const char *, unsigned int);
extern int vms_open(const char *, int, unsigned int);
extern boolean same_dir(const char *, const char *);
extern int c__translate(int);
extern char *vms_basename(const char *, boolean);

/* ### vmsmail.c ### */

extern unsigned long init_broadcast_trapping(void);
extern unsigned long enable_broadcast_trapping(void);
extern unsigned long disable_broadcast_trapping(void);
#if 0
extern struct mail_info *parse_next_broadcast(void);
#endif /*0*/

/* ### vmsmain.c ### */

extern int main(int, char **);
#ifdef CHDIR
extern void chdirx(const char *, boolean);
#endif /* CHDIR */
extern void sethanguphandler(void(*)(int));
extern boolean authorize_wizard_mode(void);
extern boolean authorize_explore_mode(void);

/* ### vmsmisc.c ### */

ATTRNORETURN extern void vms_abort(void) NORETURN;
ATTRNORETURN extern void vms_exit(int) NORETURN;
#ifdef PANICTRACE
extern void vms_traceback(int);
#endif

/* ### vmstty.c ### */

extern int vms_getchar(void);
extern void gettty(void);
extern void settty(const char *);
extern void shuttty(const char *);
extern void setftty(void);
extern void intron(void);
extern void introff(void);
ATTRNORETURN extern void error (const char *, ...) PRINTF_F(1, 2) NORETURN;
#ifdef TIMED_DELAY
extern void msleep(unsigned);
#endif
#ifdef SIGWINCH
extern void getwindowsz(void);
#endif
#ifdef ENHANCED_SYMBOLS
extern void tty_utf8graphics_fixup(void);
#endif

/* ### vmsunix.c ### */

extern void getlock(void);
extern void regularize(char *);
extern int vms_getuid(void);
extern boolean file_is_stmlf(int);
extern int vms_define(const char *, const char *, int);
extern int vms_putenv(const char *);
extern char *verify_termcap(void);
#if defined(CHDIR) || defined(SHELL) || defined(SECURE)
extern void privoff(void);
extern void privon(void);
#endif
#ifdef SYSCF
extern boolean check_user_string(const char *);
#endif
#ifdef SHELL
extern int dosh(void);
#endif
#if defined(SHELL) || defined(MAIL)
extern int vms_doshell(const char *, boolean);
#endif
#ifdef SUSPEND
extern int dosuspend(void);
#endif
#ifdef SELECTSAVED
extern int vms_get_saved_games(const char *, char ***);
#endif

#endif /* VMS */

/* ### weapon.c ### */

extern const char *weapon_descr(struct obj *) NONNULLARG1;
extern int hitval(struct obj *, struct monst *) NONNULLARG12;
extern int dmgval(struct obj *, struct monst *) NONNULLARG12;
extern int special_dmgval(struct monst *, struct monst *, long, long *) NONNULLARG12;
extern void silver_sears(struct monst *, struct monst *, long) NONNULLARG2;
extern struct obj *select_rwep(struct monst *) NONNULLARG1;
extern boolean monmightthrowwep(struct obj *) NONNULLARG1;
extern struct obj *select_hwep(struct monst *) NONNULLARG1;
extern void possibly_unwield(struct monst *, boolean) NONNULLARG1;
extern int mon_wield_item(struct monst *) NONNULLARG1;
extern void mwepgone(struct monst *) NONNULLARG1;
extern int abon(void);
extern int dbon(void);
extern void wet_a_towel(struct obj *, int, boolean) NONNULLARG1;
extern void dry_a_towel(struct obj *, int, boolean) NONNULLARG1;
extern char *skill_level_name(int, char *) NONNULLARG2;
extern const char *skill_name(int);
extern boolean can_advance(int, boolean);
extern void show_skills(void);
extern int enhance_weapon_skill(void);
extern void unrestrict_weapon_skill(int);
extern void use_skill(int, int);
extern void add_weapon_skill(int);
extern void lose_weapon_skill(int);
extern void drain_weapon_skill(int);
extern int weapon_type(struct obj *) NO_NNARGS;
extern int uwep_skill_type(void);
/* find_roll_to_hit() calls weapon_hit_bonus() with a NULL argument,
   preventing NONNULLARG1 */
extern int weapon_hit_bonus(struct obj *) NO_NNARGS;
extern int weapon_dam_bonus(struct obj *) NO_NNARGS;
extern void skill_init(const struct def_skill *) NONNULLARG1;
extern void setmnotwielded(struct monst *, struct obj *) NONNULLARG1;
extern const struct throw_and_return_weapon *autoreturn_weapon(struct obj *)
    NONNULLARG1;

/* ### were.c ### */

extern void were_change(struct monst *) NONNULLARG1;
extern int counter_were(int);
extern int were_beastie(int);
extern void new_were(struct monst *) NONNULLARG1;
extern int were_summon(struct permonst *, boolean, int *, char *) NONNULLARG13;
extern void you_were(void);
extern void you_unwere(boolean);
extern void set_ulycn(int);

/* ### wield.c ### */

extern void setuwep(struct obj *) NO_NNARGS; /* NULL:ball.c, do.c */
extern const char *empty_handed(void);
extern void setuqwep(struct obj *) NO_NNARGS;  /* NULL:ball.c, do.c */
extern void setuswapwep(struct obj *) NO_NNARGS; /* NULL: ball.c, do.c */
extern int dowield(void);
extern int doswapweapon(void);
extern int dowieldquiver(void);
extern int doquiver_core(const char *) NONNULLARG1;
extern boolean wield_tool(struct obj *, const char *) NONNULLARG1;
extern int can_twoweapon(void);
extern void drop_uswapwep(void);
extern int dotwoweapon(void);
extern void uwepgone(void);
extern void uswapwepgone(void);
extern void uqwepgone(void);
extern void set_twoweap(boolean);
extern void untwoweapon(void);
extern int chwepon(struct obj *, int) NO_NNARGS;
extern int welded(struct obj *) NO_NNARGS;
extern void weldmsg(struct obj *) NONNULLARG1;
extern boolean mwelded(struct obj *) NO_NNARGS;

/* ### windows.c ### */

extern void choose_windows(const char *) NONNULLARG1;
#ifdef WINCHAIN
void addto_windowchain(const char *s) NONNULLARG1;
void commit_windowchain(void);
#endif
#ifdef TTY_GRAPHICS
extern boolean check_tty_wincap(unsigned long);
extern boolean check_tty_wincap2(unsigned long);
#endif
extern boolean genl_can_suspend_no(void);
extern boolean genl_can_suspend_yes(void);
extern char genl_message_menu(char, int, const char *) NONNULLARG3;
extern void genl_preference_update(const char *) NO_NNARGS;
extern char *genl_getmsghistory(boolean);
extern void genl_putmsghistory(const char *, boolean) NONNULLARG1;
#ifdef HANGUPHANDLING
extern void nhwindows_hangup(void);
#endif
extern void genl_status_init(void);
extern void genl_status_finish(void);
extern void genl_status_enablefield(int, const char *, const char *,
                                    boolean) NONNULLPTRS;
extern void genl_status_update(int, genericptr_t, int, int, int,
                               unsigned long *) NONNULLARG2;
#ifdef DUMPLOG
extern char *dump_fmtstr(const char *, char *, boolean) NONNULLPTRS;
#endif
extern void dump_open_log(time_t);
extern void dump_close_log(void);
extern void dump_redirect(boolean);
extern void dump_forward_putstr(winid, int, const char*, int) NONNULLARG3;
extern int has_color(int);
extern int glyph2ttychar(int);
extern int glyph2symidx(int);
extern char *encglyph(int);
extern int decode_glyph(const char *str, int *glyph_ptr) NONNULLPTRS;
extern char *decode_mixed(char *, const char *) NONNULLARG1;
extern void genl_putmixed(winid, int, const char *) NONNULLARG3;
extern void genl_display_file(const char *, boolean) NONNULLARG1;
extern boolean menuitem_invert_test(int, unsigned, boolean);
extern const char *mixed_to_glyphinfo(const char *str,
                                      glyph_info *gip) NO_NNARGS;
extern void adjust_menu_promptstyle(winid, color_attr *) NONNULLARG2;
extern int choose_classes_menu(const char *, int, boolean,
                               char *, char *) NONNULLARG1;
extern void add_menu(winid, const glyph_info *, const ANY_P *,
                     char, char, int, int, const char *, unsigned int);
extern void add_menu_heading(winid, const char *) NONNULLARG2;
extern void add_menu_str(winid, const char *) NONNULLARG2;
extern int select_menu(winid, int, menu_item **) NONNULLARG3;
extern void getlin(const char *, char *) NONNULLARG2;

/* ### windsys.c ### */

#ifdef WIN32
extern void nethack_enter_windows(void);
#endif

/* ### wizard.c ### */

extern void amulet(void);
extern int mon_has_amulet(struct monst *) NONNULLARG1;
extern int mon_has_special(struct monst *) NONNULLARG1;
extern void choose_stairs(coordxy *, coordxy *, boolean) NONNULLARG12;
extern int tactics(struct monst *) NONNULLARG1;
extern boolean has_aggravatables(struct monst *) NONNULLARG1;
extern void aggravate(void);
extern void clonewiz(void);
extern int pick_nasty(int);
extern int nasty(struct monst *) NO_NNARGS;
extern void resurrect(void);
extern void intervene(void);
extern void wizdeadorgone(void);
extern void cuss(struct monst *) NONNULLARG1;

/* ### wizcmds.c ### */

extern int wiz_custom(void);
extern int wiz_detect(void);
extern int wiz_flip_level(void);
extern int wiz_fuzzer(void);
extern int wiz_genesis(void);
extern int wiz_identify(void);
extern int wiz_intrinsic(void);
extern int wiz_kill(void);
extern int wiz_level_change(void);
extern int wiz_level_tele(void);
extern int wiz_load_lua(void);
extern int wiz_load_splua(void);
extern int wiz_makemap(void);
extern int wiz_map(void);
extern int wiz_migrate_mons(void);
extern int wiz_panic(void);
extern int wiz_polyself(void);
extern int wiz_rumor_check(void);
extern int wiz_show_seenv(void);
extern int wiz_show_stats(void);
extern int wiz_show_vision(void);
extern int wiz_show_wmodes(void);
extern int wiz_smell(void);
extern int wiz_telekinesis(void);
extern int wiz_where(void);
extern int wiz_wish(void);
extern void makemap_remove_mons(void);
extern void wiz_levltyp_legend(void);
extern void wiz_map_levltyp(void);
extern void wizcustom_callback(winid win, int glyphnum, char *id);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED) || defined(DEBUG)
extern int wiz_display_macros(void);
extern int wiz_mon_diff(void);
extern int wiz_objprobs(void);
extern int wiz_show_nhuuid(void);
#endif
extern void sanity_check(void);

/* ### worm.c ### */

extern int get_wormno(void);
extern void initworm(struct monst *, int) NONNULLARG1;
extern void worm_move(struct monst *) NONNULLARG1;
extern void worm_nomove(struct monst *) NONNULLARG1;
extern void wormgone(struct monst *) NONNULLARG1;
extern int wormhitu(struct monst *) NONNULLARG1;
extern void cutworm(struct monst *, coordxy, coordxy, boolean) NONNULLARG1;
extern void see_wsegs(struct monst *) NONNULLARG1;
extern void detect_wsegs(struct monst *, boolean) NONNULLARG1;
extern void save_worm(NHFILE *) NONNULLARG1;
extern void rest_worm(NHFILE *) NONNULLARG1;
extern void place_wsegs(struct monst *, struct monst *) NONNULLARG1;
extern void sanity_check_worm(struct monst *);  /* NULL leads to impossible */
extern void wormno_sanity_check(void);
extern void remove_worm(struct monst *) NONNULLARG1;
extern void place_worm_tail_randomly(struct monst *, coordxy, coordxy) NONNULLARG1;
extern int size_wseg(struct monst *) NONNULLARG1;
extern int count_wsegs(struct monst *) NONNULLARG1;
extern boolean worm_known(struct monst *) NONNULLARG1;
extern boolean worm_cross(int, int, int, int);
extern int wseg_at(struct monst *, int, int) NO_NNARGS;
extern void flip_worm_segs_vertical(struct monst *, int, int) NONNULLARG1;
extern void flip_worm_segs_horizontal(struct monst *, int, int) NONNULLARG1;
extern void redraw_worm(struct monst *);

/* ### worn.c ### */

extern void recalc_telepat_range(void);
extern void setworn(struct obj *, long) NO_NNARGS; /* has tests for obj */
extern void setnotworn(struct obj *) NO_NNARGS; /* has tests for obj */
extern void allunworn(void);
extern struct obj *wearmask_to_obj(long);
extern int wornmask_to_armcat(long);
extern long armcat_to_wornmask(int);
extern long wearslot(struct obj *) NONNULLARG1;
extern void check_wornmask_slots(void);
extern void mon_set_minvis(struct monst *, boolean) NONNULLARG1;
extern void mon_adjust_speed(struct monst *, int, struct obj *) NONNULLARG1;
extern void update_mon_extrinsics(struct monst *, struct obj *, boolean,
                                  boolean) NONNULLARG12;
extern int find_mac(struct monst *) NONNULLARG1;
extern void m_dowear(struct monst *, boolean) NONNULLARG1;
extern struct obj *which_armor(struct monst *, long) NONNULLARG1;
extern void mon_break_armor(struct monst *, boolean) NONNULLARG1;
extern void bypass_obj(struct obj *) NONNULLARG1;
extern void clear_bypasses(void);
/* callers don't check gi.invent before passing to bypass_objlist */
extern void bypass_objlist(struct obj *, boolean) NO_NNARGS;
extern struct obj *nxt_unbypassed_obj(struct obj *) NO_NNARGS;
extern struct obj *nxt_unbypassed_loot(Loot *, struct obj *) NONNULLARG1;
extern int racial_exception(struct monst *, struct obj *) NONNULLARG12;
extern void extract_from_minvent(struct monst *, struct obj *, boolean,
                                 boolean) NONNULLARG12;

/* ### write.c ### */

extern int dowrite(struct obj *) NONNULLARG1;

/* ### zap.c ### */

extern void learnwand(struct obj *) NONNULLARG1;
extern int bhitm(struct monst *, struct obj *) NONNULLARG12;
extern void release_hold(void);
extern void probe_monster(struct monst *) NONNULLARG1;
extern boolean get_obj_location(struct obj *, coordxy *, coordxy *,
                                int) NONNULLPTRS;
extern boolean get_mon_location(struct monst *, coordxy *, coordxy *,
                                int) NONNULLPTRS;
extern struct monst *get_container_location(struct obj *,
                                            int *, int *) NONNULLARG2;
extern struct monst *montraits(struct obj *, coord *, boolean) NONNULLARG12;
extern struct monst *revive(struct obj *, boolean) NONNULLARG1;
extern int unturn_dead(struct monst *) NONNULLARG1;
extern void unturn_you(void);
extern void cancel_item(struct obj *) NONNULLARG1;
extern void blank_novel(struct obj *) NONNULLARG1;
extern boolean drain_item(struct obj *, boolean) NO_NNARGS; /* tests !obj */
extern boolean obj_unpolyable(struct obj *) NONNULLARG1;
extern struct obj *poly_obj(struct obj *, int) NONNULLARG1;
extern boolean obj_resists(struct obj *, int, int) NONNULLARG1;
extern boolean obj_shudders(struct obj *) NONNULLARG1;
extern void do_osshock(struct obj *) NONNULLARG1;
extern int bhito(struct obj *, struct obj *) NONNULLARG12;
extern int bhitpile(struct obj *, int(*)(struct obj *, struct obj *),
                    coordxy, coordxy, schar) NONNULLARG12;
extern int zappable(struct obj *) NONNULLARG1;
extern void do_enlightenment_effect(void);
extern void zapnodir(struct obj *) NONNULLARG1;
extern int dozap(void);
extern int zapyourself(struct obj *, boolean) NONNULLARG1;
extern void ubreatheu(struct attack *) NONNULLARG1;
extern int lightdamage(struct obj *, boolean, int) NONNULLARG1;
extern boolean flashburn(long, boolean);
extern boolean cancel_monst(struct monst *, struct obj *, boolean, boolean,
                            boolean) NONNULLARG12;
extern void zapsetup(void);
extern void zapwrapup(void);
extern void weffects(struct obj *) NONNULLARG1;
extern int spell_damage_bonus(int);
extern const char *exclam(int force) NONNULL;
extern void hit(const char *, struct monst *, const char *) NONNULLPTRS;
extern void miss(const char *, struct monst *) NONNULLPTRS;
extern struct monst *bhit(int, int, int, enum bhit_call_types,
                          int(*)(struct monst *, struct obj *),
                          int(*)(struct obj *, struct obj *),
                          struct obj **) NONNULLARG7;
extern struct monst *boomhit(struct obj *, int, int) NONNULLARG1;
extern int zhitm(struct monst *, int, int, struct obj **) NONNULLPTRS;
extern int burn_floor_objects(coordxy, coordxy, boolean, boolean);
extern void ubuzz(int, int);
extern void buzz(int, int, coordxy, coordxy, int, int);
extern void dobuzz(int, int, coordxy, coordxy, int, int,
                   boolean, boolean, boolean);
extern void melt_ice(coordxy, coordxy, const char *) NO_NNARGS;
extern void start_melt_ice_timeout(coordxy, coordxy, long);
extern void melt_ice_away(union any *, long) NONNULLARG1;
extern int zap_over_floor(coordxy, coordxy, int, boolean *,
                          boolean, short) NONNULLARG4;
extern void mon_spell_hits_spot(struct monst *, int, coordxy x, coordxy y);
extern void fracture_rock(struct obj *) NONNULLARG1;
extern boolean break_statue(struct obj *) NONNULLARG1;
extern int u_adtyp_resistance_obj(int);
extern boolean inventory_resistance_check(int);
extern char *item_what(int);
extern int destroy_items(struct monst *, int, int) NONNULLARG1;
extern int resist(struct monst *, char, int, int) NONNULLARG1;
extern void wish_history_add(char *);
extern void wish_history_flush(void);
extern void makewish(void);
extern const char *flash_str(int, boolean) NONNULL;

/* ### unixmain.c, windsys.c ### */

#ifdef RUNTIME_PORT_ID
extern char *get_port_id(char *);
#endif
#ifdef RUNTIME_PASTEBUF_SUPPORT
extern void port_insert_pastebuf(char *);
#endif
extern void get_nhuuid(void);
extern void free_nhuuid(void);

#endif /* !MAKEDEFS_C && !MDLIB_C */

#endif /* EXTERN_H */

/*extern.h*/
