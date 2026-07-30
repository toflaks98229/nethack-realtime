/* NetHack 5.0	display.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.118 $ */
/* Copyright (c) Dean Luick, with acknowledgements to Kevin Darcy */
/* and Dave Cohrs, 1990.                                          */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file display.h
 * @brief Deciding what the hero perceives, and turning that into one number.
 *
 * Two subjects share this file, and they meet in the middle of it.
 *
 * The first is perception. Whether the hero knows a monster is there is not one question but several, and they are genuinely
 * different: seeing it, sensing it telepathically, detecting it by magic, being warned of it, feeling its heat. Each has its own
 * conditions, each fails differently, and the file asks them separately so that a rule can use the one it means. Getting the wrong
 * one is how a monster becomes visible to a blind hero, or invisible to a telepathic one.
 *
 * The second is the glyph. Once the game has decided what the hero perceives at a square, that answer has to reach the display as a
 * single value -- there is no room in the interface for a description. So every possible answer is given a number: every monster in
 * every state it can be perceived in, every object, every piece of terrain on every kind of level, every beam, every explosion.
 * They are laid out in contiguous ranges, one range per kind, and the second half of this file is the arithmetic that goes into and
 * out of that layout.
 *
 * That arithmetic is why the ranges exist. Asking "is this glyph a pet" is a comparison against the ends of a range rather than a
 * lookup, and building a glyph from a monster is an addition. So the ordering of the ranges is not presentation but interface.
 *
 * @note Many of the perception tests appear twice: a macro whose name begins with an underscore and a function of the same name
 *       without it. The existing comment explains that the functions exist to keep the compiled code small, and that the macro forms
 *       should be preferred only where a call is measurably too slow.
 * @note The hero is drawn from a glyph like everything else, which is what lets the same code path handle the hero being displaced,
 *       ridden, or mimicking something.
 * @warning Several tests are documented as unreliable for concealed mimics, and the existing comments explain why fixing that has
 *          not been attempted: a mimic disguised as an object does not carry the flag that means "hidden", and every caller would
 *          have to be reviewed.
 */

/**
 * @file display.h
 * @brief 영웅이 무엇을 지각하는지 정하고, 그것을 하나의 숫자로 바꾸기.
 *
 * 이 파일은 두 주제를 공유하며, 그 둘이 파일 중간에서 만난다.
 *
 * 첫째는 지각이다. 영웅이 몬스터가 거기 있다는 것을 아는지는 하나의 질문이 아니라 여러 질문이며, 그것들은 정말로 서로 다르다. 그것을 보는 것, 텔레파시로 감지하는 것, 마법으로 탐지하는 것, 그것에 대해 경고받는
 * 것, 그 열을 느끼는 것. 각각에 자기 조건이 있고, 각각이 다르게 실패하며, 이 파일은 그것들을 따로 묻는다. 그래서 규칙이 자신이 뜻하는 것을 쓸 수 있다. 잘못된 것을 고르는 것이 눈먼 영웅에게 몬스터가 보이게 되거나
 * 텔레파시를 가진 영웅에게 보이지 않게 되는 경로다.
 *
 * 둘째는 글리프다. 게임이 어떤 칸에서 영웅이 무엇을 지각하는지 정한 뒤에는, 그 답이 하나의 값으로 표시부에 닿아야 한다. 인터페이스에 기술이 들어갈 자리가 없기 때문이다. 그래서 가능한 모든 답에 숫자가 하나씩 주어진다.
 * 지각될 수 있는 모든 상태의 모든 몬스터, 모든 물건, 모든 종류의 레벨의 모든 지형, 모든 광선, 모든 폭발. 그것들은 종류마다 하나씩 연속된 구간으로 놓이며, 이 파일의 후반부는 그 배치로 들어가고 나오는 산술이다.
 *
 * 그 산술이 구간이 존재하는 이유다. "이 글리프가 애완동물인가"를 묻는 것은 조회가 아니라 구간의 양 끝과의 비교이고, 몬스터에서 글리프를 만드는 것은 덧셈이다. 그래서 구간의 순서는 표현이 아니라 인터페이스다.
 *
 * @note 많은 지각 검사가 두 번 나온다. 밑줄로 시작하는 이름의 매크로와 밑줄이 없는 같은 이름의 함수. 기존 주석은 컴파일된 코드를 작게 유지하기 위해 함수가 있으며, 호출이 측정 가능하게 느린 곳에서만 매크로 형태를
 *       택해야 한다고 설명한다.
 * @note 영웅도 다른 모든 것처럼 글리프로 그려진다. 그것이 영웅이 위치가 어긋나 보이거나, 탈것에 타고 있거나, 무언가를 모방하는 경우를 같은 코드 경로가 다룰 수 있게 하는 것이다.
 * @warning 몇몇 검사는 위장한 모방자에 대해 신뢰할 수 없다고 기록되어 있고, 기존 주석들이 그것을 고치려 시도하지 않은 이유를 설명한다. 물건으로 위장한 모방자는 "숨어 있음"을 뜻하는 플래그를 지니지 않으며, 모든
 *          호출자를 다시 살펴야 한다.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef VISION_H
#include "vision.h"
#endif
#ifndef MONDATA_H
#include "mondata.h" /* for mindless() */
#endif

/*
 * vobj_at()
 *
 * Returns the head of the list of objects that the player can see
 * at location (x,y).  [Vestige of unimplemented invisible objects.]
 */
/**
 * @def vobj_at
 * @brief The objects the hero can see at a square, topmost first.
 * @note Despite the name, this is simply the objects there. The existing comment records that the distinction it was named for --
 *       visible versus invisible objects -- was never implemented, so no filtering happens.
 */
/**
 * @def vobj_at
 * @brief 영웅이 어떤 칸에서 볼 수 있는 물건들. 맨 위의 것부터.
 * @note 이름과 달리 그것은 그저 거기 있는 물건들이다. 기존 주석은 이 이름이 가리키던 구별 -- 보이는 물건과 보이지 않는 물건 -- 이 결코 구현되지 않았다고 기록하므로, 걸러내는 일은 일어나지 않는다.
 */
#define vobj_at(x, y) (svl.level.objects[x][y])

/*
 * sensemon()
 *
 * Returns true if the hero can sense the given monster.  This includes
 * monsters that are hiding or mimicking other monsters.
 *
 * [5.0] Note: the map doesn't display any monsters when hero is swallowed
 * (or display non-adjacent, non-submerged ones when hero is underwater),
 * so treat those situations as blocking telepathy, detection, and warning
 * even though conceptually they shouldn't do so.
 *
 * [5.0 also] The macros whose name begins with an underscore have been
 * converted to functions in order to have compilers generate smaller code.
 * The retained underscore versions are still used in display.c but should
 * only be used in other situations if the function calls actually produce
 * noticeably slower processing.
 */
/**
 * @def _tp_sensemon
 * @brief Whether telepathy reaches this monster.
 *
 * Telepathy has two forms and they behave differently. The earned kind works only while the hero cannot see, so a blind telepathic
 * hero senses minds anywhere on the level. The kind granted by equipment works with the eyes open but only within a range.
 *
 * @note Requires the monster to have a mind. A mindless creature is invisible to telepathy however strong it is, which is the one
 *       thing telepathy cannot compensate for.
 */
/**
 * @def _tp_sensemon
 * @brief 텔레파시가 이 몬스터에 닿는지.
 *
 * 텔레파시에는 두 형태가 있고 서로 다르게 동작한다. 얻은 종류는 영웅이 볼 수 없는 동안에만 작동하므로, 눈먼 텔레파시 영웅은 레벨 어디에 있는 마음이든 감지한다. 장비가 주는 종류는 눈을 뜬 채로도 작동하지만 일정
 * 범위 안에서만이다.
 *
 * @note 몬스터에게 마음이 있어야 한다. 마음 없는 생물은 텔레파시가 얼마나 강하든 그것에 보이지 않으며, 그것이 텔레파시가 메울 수 없는 유일한 것이다.
 */
#define _tp_sensemon(mon) \
    (/* The hero can always sense a monster IF:        */  \
     /* 1. the monster has a brain to sense            */  \
     (!mindless(mon->data))                                \
     /* AND     2a. hero is blind and telepathic       */  \
      && ((Blind && Blind_telepat)                         \
          /* OR 2b. hero is using a telepathy inducing */  \
          /*        object and in range                */  \
          || (Unblind_telepat                              \
              && (mdistu(mon) <= u.unblind_telepat_range))))

/**
 * @def _sensemon
 * @brief Whether the hero perceives this monster by any means other than sight.
 *
 * Gathers the three non-visual ways -- magical detection, telepathy, and warning of a particular kind -- behind the two situations
 * that override all of them.
 *
 * @note Being swallowed and being underwater are treated as blocking every non-visual sense. The existing comment concedes that
 *       conceptually they should not, and gives the practical reason: the map shows nothing while swallowed, so a sensed monster
 *       would have nowhere to appear.
 * @note The underwater case tests for water rather than water or lava on purpose, as the existing comment explains: adjacent lava is
 *       visible from underwater, but anything standing in it is on top of it and so not in the hero's medium.
 * @note Ordered so the cheap tests come first, which matters because this is asked of every monster on the level whenever the map is
 *       redrawn.
 */
/**
 * @def _sensemon
 * @brief 영웅이 시각 외의 어떤 방법으로든 이 몬스터를 지각하는지.
 *
 * 시각이 아닌 세 방법 -- 마법 탐지, 텔레파시, 특정 종류에 대한 경고 -- 을 그 전부를 무시하는 두 상황 뒤에 모은다.
 *
 * @note 삼켜진 것과 물속에 있는 것은 모든 비시각 감각을 막는 것으로 취급된다. 기존 주석은 개념적으로 그러지 않아야 한다고 인정하면서 실질적인 이유를 밝힌다. 삼켜진 동안 지도는 아무것도 보이지 않으므로, 감지된
 *       몬스터가 나타날 자리가 없다.
 * @note 물속 경우는 의도적으로 용암 포함이 아니라 물만을 검사한다. 기존 주석이 설명하듯 물속에서 인접한 용암은 보이지만, 그 안에 서 있는 것은 그 위에 있는 것이므로 영웅과 같은 매체 안에 있지 않다.
 * @note 값싼 검사가 먼저 오도록 정렬되어 있다. 지도가 다시 그려질 때마다 레벨의 모든 몬스터에 대해 이것이 물어지므로 그것이 중요하다.
 */
/* organized to perform cheaper tests first;
   is_pool() vs is_pool_or_lava(): hero who is underwater can see adjacent
   lava, but presumably any monster there is on top so not sensed */
#define _sensemon(mon) \
    (   (!u.uswallow || (mon) == u.ustuck)                                   \
     && (!Underwater || (mdistu(mon) <= 2 && is_pool((mon)->mx, (mon)->my))) \
     && (Detect_monsters || tp_sensemon(mon) || MATCH_WARN_OF_MON(mon))   )

/*
 * mon_warning() is used to warn of any dangerous monsters in your
 * vicinity, and a glyph representing the warning level is displayed.
 */
/**
 * @def _mon_warning
 * @brief Whether this monster is dangerous enough and near enough to trigger a warning.
 *
 * Warning does not identify what is coming. It shows a level of danger, so the test is against how tough the monster is relative to
 * the threshold the player set -- a strong monster warns and a weak one does not, even at the same distance.
 *
 * @note Peaceful monsters never warn, since the danger warning represents is the danger of being attacked.
 */
/**
 * @def _mon_warning
 * @brief 이 몬스터가 경고를 일으킬 만큼 위험하고 가까운지.
 *
 * 경고는 무엇이 오고 있는지 밝히지 않는다. 위험의 정도를 보이므로, 검사는 플레이어가 정한 문턱에 비해 그 몬스터가 얼마나 강한지에 대한 것이다. 강한 몬스터는 경고하고 약한 몬스터는 같은 거리에서도 경고하지 않는다.
 *
 * @note 평화로운 몬스터는 결코 경고하지 않는다. 경고가 나타내는 위험이 공격받을 위험이기 때문이다.
 */
#define _mon_warning(mon) \
    (Warning && !(mon)->mpeaceful && (mdistu(mon) < 100)     \
     && (((int) ((mon)->m_lev / 4)) >= svc.context.warnlevel))

/*
 * mon_visible()
 *
 * Returns true if the hero can see the monster.  It is assumed that the
 * hero can physically see the location of the monster.  The function
 * vobj_at() returns a pointer to an object that the hero can see there.
 * Infravision is not taken into account.
 *
 * Note:  not reliable for concealed mimics.  They don't have
 * 'mon->mundetected' set even when mimicking objects or furniture.
 * [Fixing this with a pair of mon->m_ap_type checks here (via either
 * 'typ!=object && typ!=furniture' or 'typ==nothing || typ==monster')
 * will require reviewing every instance of mon_visible(), canseemon(),
 * canspotmon(), is_safemon() and perhaps others.  Fixing it by setting
 * mon->mundetected when concealed would be better but also require
 * reviewing all those instances and also existing mundetected instances.]
 */
/**
 * @def _mon_visible
 * @brief Whether the monster itself is visible, given that its square is.
 *
 * A narrow question on purpose: it asks only about the monster, assuming the caller has already established that the hero can see
 * where it is standing. That division is why this and @c canseemon are separate rather than one test.
 *
 * @note Infravision is not considered here. Seeing by heat is a different way of seeing and is asked separately.
 * @warning Unreliable for a concealed mimic. The existing comment explains: a mimic disguised as an object does not have the hidden
 *          flag set, and the note records that both possible fixes would mean reviewing every caller of this and of three related
 *          tests.
 * @note The disabled version above also excluded a buried hero or monster. It is kept visible rather than deleted so the reasoning
 *       is not lost.
 */
/**
 * @def _mon_visible
 * @brief 그 몬스터의 칸이 보인다고 할 때, 그 몬스터 자체가 보이는지.
 *
 * 의도적으로 좁은 질문이다. 호출자가 영웅이 그것이 서 있는 곳을 볼 수 있다는 것을 이미 확인했다고 가정하고, 그 몬스터에 대해서만 묻는다. 그 분업이 이것과 @c canseemon 이 하나의 검사가 아니라 따로 있는 이유다.
 *
 * @note 여기서 적외선 시야는 고려되지 않는다. 열로 보는 것은 다른 방식의 보기이며 따로 물어진다.
 * @warning 위장한 모방자에 대해 신뢰할 수 없다. 기존 주석이 설명한다. 물건으로 위장한 모방자는 숨음 플래그가 켜지지 않으며, 그 주석은 가능한 두 해결책 모두 이 검사와 관련된 세 검사의 모든 호출자를 다시 살펴야
 *          한다고 기록한다.
 * @note 위의 비활성화된 판본은 묻힌 영웅이나 몬스터도 제외했다. 그 논리가 사라지지 않도록 지워지지 않고 보이는 채로 남겨져 있다.
 */
#if 0
#define _mon_visible(mon) \
    (/* The hero can see the monster IF the monster                     */ \
     (!mon->minvis || See_invisible)  /*     1. is not invisible        */ \
     && !mon->mundetected             /* AND 2. not an undetected hider */ \
     && !(mon->mburied || u.uburied)) /* AND 3. neither you nor it buried */
#else   /* without 'mburied' and 'uburied' */
#define _mon_visible(mon) \
    (/* The hero can see the monster IF the monster                     */ \
     (!mon->minvis || See_invisible)  /*     1. is not invisible        */ \
     && !mon->mundetected)            /* AND 2. not an undetected hider */
#endif

/*
 * see_with_infrared()
 *
 * This function is true if the player can see a monster using infravision.
 * The caller must check for invisibility (invisible monsters are also
 * invisible to infravision), because this is usually called from within
 * canseemon() or canspotmon() which already check that.
 */
/**
 * @def _see_with_infrared
 * @brief Whether the hero perceives this monster by its heat.
 * @note Requires the monster to be one that gives off heat and the square to be within line of sight -- heat does not travel through
 *       walls, so this is limited the same way ordinary vision is.
 * @warning Does not check invisibility, deliberately. Its callers already have, as the existing comment records, and an invisible
 *          monster is invisible to infravision too.
 */
/**
 * @def _see_with_infrared
 * @brief 영웅이 이 몬스터를 그 열로 지각하는지.
 * @note 그 몬스터가 열을 내는 종류여야 하고 그 칸이 시선 안에 있어야 한다. 열은 벽을 통과하지 않으므로, 이것은 평범한 시야와 같은 방식으로 제한된다.
 * @warning 의도적으로 투명 여부를 검사하지 않는다. 기존 주석이 기록하듯 그것을 호출하는 쪽이 이미 검사했으며, 투명한 몬스터는 적외선 시야에도 보이지 않는다.
 */
#define _see_with_infrared(mon) \
    (!Blind && Infravision && infravisible(mon->data) \
     && couldsee(mon->mx, mon->my))

/*
 * canseemon()
 *
 * This is the globally used canseemon().  It is not called within the display
 * routines.  Like mon_visible(), but it checks to see if the hero sees the
 * location instead of assuming it.  (And also considers worms.)
 */
/**
 * @def _canseemon
 * @brief Whether the hero can actually see this monster.
 *
 * The complete visual question, and what code outside the display routines should use. It establishes that the hero can see the
 * square, rather than assuming it, and it accepts heat as a way of seeing.
 *
 * @note Long worms are handled separately because a worm occupies many squares. Whether a worm is seen is a question about the whole
 *       worm, not about the square its head happens to be on.
 */
/**
 * @def _canseemon
 * @brief 영웅이 이 몬스터를 실제로 볼 수 있는지.
 *
 * 완전한 시각 질문이며, 표시 루틴 밖의 코드가 써야 하는 것이다. 영웅이 그 칸을 볼 수 있다는 것을 가정하지 않고 확인하며, 열로 보는 것도 받아들인다.
 *
 * @note 긴 벌레는 따로 처리된다. 벌레가 여러 칸을 차지하기 때문이다. 벌레가 보이는지는 그 머리가 마침 놓인 칸에 대한 질문이 아니라 벌레 전체에 대한 질문이다.
 */
#define _canseemon(mon) \
    ((mon->wormno ? worm_known(mon)                                       \
                  : (cansee(mon->mx, mon->my) || see_with_infrared(mon))) \
     && mon_visible(mon))

/*
 * canspotmon(mon)
 *
 * This function checks whether you can either see a monster or sense it by
 * telepathy, and is what you usually call for monsters about which nothing is
 * known.
 */
/**
 * @def canspotmon
 * @brief Whether the hero is aware of this monster at all, by any means.
 * @note The one to reach for by default. Sight and every non-visual sense together, which is what a rule wants when it asks whether
 *       the hero knows something is there rather than how they know.
 */
/**
 * @def canspotmon
 * @brief 영웅이 어떤 방법으로든 이 몬스터를 알고 있는지.
 * @note 기본으로 손을 뻗을 것. 시각과 모든 비시각 감각을 함께 묻는다. 규칙이 영웅이 어떻게 아는지가 아니라 무언가가 거기 있다는 것을 아는지를 물을 때 원하는 것이다.
 */
#define canspotmon(mon) (canseemon(mon) || sensemon(mon))

/* knowninvisible(mon)
 * This one checks to see if you know a monster is both there and invisible.
 * 1) If you can see the monster and have see invisible, it is assumed the
 * monster is transparent, but visible in some manner.  (Earlier versions of
 * Nethack were really inconsistent on this.)
 * 2) If you can't see the monster, but can see its location and you have
 * telepathy that works when you can see, you can tell that there is a
 * creature in an apparently empty spot.
 * Infravision is not relevant; we assume that invisible monsters are also
 * invisible to infravision.
 * [5.0: the macro definition erroneously started with 'mtmp->minvis' for
 * over 20 years.  The one place it's used called it as knowninvisible(mtmp)
 * so worked by coincidence when there was no argument expansion involved
 * on the first line.]
 */
/**
 * @def _knowninvisible
 * @brief Whether the hero knows both that a monster is there and that it is invisible.
 *
 * A narrower thing than it sounds. There are two ways to know it: seeing the monster while able to see the invisible, in which case it
 * is perceived as transparent rather than absent; or not seeing it but sensing a creature in what looks like an empty square, which
 * requires telepathy that works with the eyes open.
 *
 * @note Infravision is irrelevant, on the assumption that an invisible monster gives off no perceptible heat either.
 * @warning For over twenty years the definition began with a hard-coded variable name rather than the argument, as the existing
 *          comment records. It worked only because its single caller happened to pass a variable of that name.
 */
/**
 * @def _knowninvisible
 * @brief 영웅이 몬스터가 거기 있다는 것과 그것이 투명하다는 것을 함께 아는지.
 *
 * 들리는 것보다 좁은 것이다. 그것을 아는 방법은 둘이다. 투명한 것을 볼 수 있는 상태에서 그 몬스터를 보는 것 -- 그 경우 그것은 없는 것이 아니라 투명한 것으로 지각된다 -- 또는 그것을 보지 못하면서 빈 것처럼 보이는
 * 칸에서 생물을 감지하는 것. 뒤의 것은 눈을 뜬 채로 작동하는 텔레파시를 요구한다.
 *
 * @note 적외선 시야는 무관하다. 투명한 몬스터는 지각할 수 있는 열도 내지 않는다는 전제에서다.
 * @warning 기존 주석이 기록하듯 이십 년 넘게 이 정의는 인자가 아니라 하드코딩된 변수 이름으로 시작했다. 그 하나뿐인 호출자가 마침 그 이름의 변수를 넘겼기 때문에만 작동했다.
 */
#define _knowninvisible(mon) \
    ((mon)->minvis                                                      \
     && ((cansee((mon)->mx, (mon)->my)                                  \
          && (See_invisible || Detect_monsters))                        \
         || (!Blind && (HTelepat & ~INTRINSIC)                          \
             && mdistu(mon) <= (BOLT_LIM * BOLT_LIM))))

/*
 * is_safemon(mon)
 *
 * A special case check used in attack() and domove().  Placing the
 * definition here is convenient.  No longer limited to pets.
 */
/**
 * @def _is_safemon
 * @brief Whether moving into this monster should be refused rather than becoming an attack.
 *
 * The protection a player asks for when they do not want to kill their own pet by pressing a direction key. It requires the player to
 * have asked for it, the monster to be peaceful, and the hero to be able to perceive it -- there is nothing to protect against a
 * mistake the hero could not have known they were making.
 *
 * @note Also requires the hero to be in their right mind. A confused, hallucinating or stunned hero may attack a pet, since the
 *       protection represents the hero's care rather than the game's.
 * @note No longer limited to pets, as the existing comment notes, despite the historical name.
 */
/**
 * @def _is_safemon
 * @brief 이 몬스터 쪽으로 움직이는 것이 공격이 되는 대신 거절되어야 하는지.
 *
 * 플레이어가 방향키를 눌러 자기 애완동물을 죽이고 싶지 않을 때 요청하는 보호다. 플레이어가 그것을 요청했어야 하고, 그 몬스터가 평화로워야 하고, 영웅이 그것을 지각할 수 있어야 한다. 영웅이 자신이 실수하고 있는지
 * 알 수 없었을 실수에 대해서는 보호할 것이 없다.
 *
 * @note 영웅이 제정신이기를 함께 요구한다. 혼란하거나 환각 중이거나 기절한 영웅은 애완동물을 공격할 수 있다. 이 보호가 게임의 배려가 아니라 영웅의 주의를 나타내기 때문이다.
 * @note 기존 주석이 밝히듯 역사적인 이름과 달리 더는 애완동물에 한정되지 않는다.
 */
#define _is_safemon(mon) \
    (flags.safe_dog && (mon)->mpeaceful && canspotmon(mon)              \
     && !Confusion && !Hallucination && !Stunned)

/*
 * canseeself()
 * senseself()
 * canspotself()
 *
 * This returns true if the hero can see her/himself.
 *
 * Sensing yourself by touch is treated as seeing yourself, even if
 * unable to see.  So when blind, being invisible won't affect your
 * self-perception, and when swallowed, the enclosing monster touches.
 */
/**
 * @def canseeself
 * @brief Whether the hero perceives their own body.
 * @note Blindness makes this true rather than false, which looks backwards until the reasoning is stated: touch counts as seeing
 *       yourself. A blind hero knows where their own limbs are, so being invisible as well changes nothing. Being swallowed works the
 *       same way -- the enclosing monster is in contact.
 */
/**
 * @def canseeself
 * @brief 영웅이 자기 몸을 지각하는지.
 * @note 실명이 이것을 거짓이 아니라 참으로 만든다. 그 논리를 밝히기 전까지는 거꾸로 보인다. 촉각이 자신을 보는 것으로 셈해진다. 눈먼 영웅은 자기 팔다리가 어디 있는지 알고 있으므로, 거기에 투명함이 더해져도 달라지는
 *       것이 없다. 삼켜진 경우도 같은 방식이다. 감싸고 있는 몬스터가 닿아 있다.
 */
#define canseeself() (Blind || u.uswallow || (!Invisible && !u.uundetected))
/**
 * @def senseself
 * @brief Whether the hero perceives themselves by a non-visual sense.
 */
/**
 * @def senseself
 * @brief 영웅이 비시각 감각으로 자신을 지각하는지.
 */
#define senseself() (Unblind_telepat || Detect_monsters)
/**
 * @def canspotself
 * @brief Whether the hero perceives themselves at all.
 * @note The counterpart of @c canspotmon for the hero, so the same question can be asked of the hero and of a monster in the same
 *       shape.
 */
/**
 * @def canspotself
 * @brief 영웅이 자신을 조금이라도 지각하는지.
 * @note 영웅에 대한 @c canspotmon 의 짝이다. 그래서 영웅과 몬스터에게 같은 질문을 같은 모양으로 할 수 있다.
 */
#define canspotself() (canseeself() || senseself())

/*
 * random_monster()
 * random_object()
 *
 * Respectively return a random monster or object.
 * random_object() won't return STRANGE_OBJECT or the generic objects.
 * -/+ FIRST_OBJECT is used to skip it and them.
 */
/**
 * @def random_monster
 * @brief Any kind of monster, chosen at random.
 * @param rng which random number generator to draw from
 */
/**
 * @def random_monster
 * @brief 아무 종류의 몬스터. 무작위로 골라진다.
 * @param rng 어느 난수 생성기에서 뽑을지
 */
#define random_monster(rng) ((*rng)(NUMMONS))
/**
 * @def random_object
 * @brief Any real kind of object, chosen at random.
 * @param rng which random number generator to draw from
 * @note The offset skips the placeholder and generic entries at the start of the object table, as the existing comment records -- a
 *       hallucinating hero should see a real thing, not "strange object".
 */
/**
 * @def random_object
 * @brief 실제 존재하는 아무 종류의 물건. 무작위로 골라진다.
 * @param rng 어느 난수 생성기에서 뽑을지
 * @note 그 오프셋은 기존 주석이 기록하듯 물건 표 앞쪽의 자리 표시 항목과 일반 항목을 건너뛴다. 환각 중인 영웅은 "이상한 물건"이 아니라 실제 있는 것을 보아야 한다.
 */
#define random_object(rng) ((*rng)(NUM_OBJECTS - FIRST_OBJECT) + FIRST_OBJECT)

/*
 * what_obj()
 * what_mon()
 *
 * If hallucinating, choose a random object/monster, otherwise, use the one
 * given. Use the given rng to handle hallucination.
 */
/**
 * @name What the hero thinks they are looking at
 * @brief The truth, unless the hero is hallucinating, in which case anything.
 *
 * Every place that names something to the player passes it through one of these first. That is the whole implementation of
 * hallucination: the game's state is untouched and only the naming is substituted, so nothing has to be undone when it wears off.
 *
 * @warning The generator is an argument on purpose. Which one is used decides whether the substitution consumes the game's randomness,
 *          and using the wrong one makes a game's outcome depend on how often the display was redrawn.
 * @{
 */
/**
 * @name 영웅이 무엇을 보고 있다고 여기는지
 * @brief 진실. 다만 영웅이 환각 중이면 무엇이든.
 *
 * 플레이어에게 무언가의 이름을 말하는 모든 곳이 먼저 이 중 하나를 거친다. 그것이 환각의 구현 전체다. 게임의 상태는 손대지 않고 이름 붙이기만 갈아치우므로, 그것이 풀릴 때 되돌릴 것이 없다.
 *
 * @warning 생성기가 인자인 것은 의도적이다. 어느 것을 쓰는지가 그 갈아치우기가 게임의 무작위성을 소비하는지를 정하며, 잘못된 것을 쓰면 게임의 결과가 표시부가 몇 번 다시 그려졌는지에 달리게 된다.
 * @{
 */
#define what_obj(obj, rng) (Hallucination ? random_object(rng) : obj)
#define what_mon(mon, rng) (Hallucination ? random_monster(rng) : mon)
/** @} */

/*
 * newsym_rn2
 *
 * An appropriate random number generator for use with newsym(), when
 * randomness is needed there. This is currently hardcoded as
 * rn2_on_display_rng, but is futureproofed for cases where we might
 * want to prevent display-random objects entering the character's
 * memory (this isn't important at present but may be if we need
 * reproducible gameplay for some reason).
 */
/**
 * @def newsym_rn2
 * @brief The generator to use when redrawing a square needs a random choice.
 *
 * Deliberately a separate generator from the game's. Redrawing happens as often as the display demands, not as often as the game
 * advances, so drawing from the game's randomness would make the outcome of a game depend on how the player scrolled the map.
 *
 * @note Named rather than used directly so that this decision has one place to change. The existing comment records why that
 *       futureproofing exists: display randomness entering the hero's memory would be a problem if reproducible play were ever needed.
 */
/**
 * @def newsym_rn2
 * @brief 칸을 다시 그리는 데 무작위 선택이 필요할 때 쓸 생성기.
 *
 * 의도적으로 게임의 것과 별개인 생성기다. 다시 그리기는 게임이 진행되는 만큼이 아니라 표시부가 요구하는 만큼 일어나므로, 게임의 무작위성에서 뽑으면 게임의 결과가 플레이어가 지도를 어떻게 움직였는지에 달리게 된다.
 *
 * @note 직접 쓰지 않고 이름을 붙인 것은 이 결정을 바꿀 곳이 한 곳이 되게 하기 위함이다. 기존 주석은 그 대비가 존재하는 이유를 기록한다. 재현 가능한 플레이가 필요해진다면 표시부의 무작위성이 영웅의 기억에 들어가는
 *       것이 문제가 된다.
 */
#define newsym_rn2 rn2_on_display_rng

/*
 * covers_objects()
 * covers_traps()
 *
 * These routines are true if what is really at the given location will
 * "cover" any objects or traps that might be there.
 */
/**
 * @def covers_objects
 * @brief Whether what is at this square hides anything lying on it.
 * @note Water and lava conceal what they contain. Water only from outside, though -- a hero who is underwater can see what is in it,
 *       which is why the test excludes that case.
 */
/**
 * @def covers_objects
 * @brief 이 칸에 있는 것이 그 위에 놓인 것을 가리는지.
 * @note 물과 용암은 자신이 담은 것을 감춘다. 다만 물은 밖에서 볼 때만이다. 물속에 있는 영웅은 그 안에 있는 것을 볼 수 있으며, 그래서 검사가 그 경우를 제외한다.
 */
#define covers_objects(xx, yy) \
    ((is_pool(xx, yy) && !Underwater) || (levl[xx][yy].typ == LAVAPOOL) \
      || (levl[xx][yy].typ == LAVAWALL))

/**
 * @def covers_traps
 * @brief Whether what is at this square hides a trap on it.
 * @note Identical to the object test, and named separately so the two questions can diverge later without every caller changing.
 */
/**
 * @def covers_traps
 * @brief 이 칸에 있는 것이 그 위의 함정을 가리는지.
 * @note 물건 검사와 동일하며, 따로 이름 붙여 두어 나중에 두 질문이 갈라질 때 모든 호출자가 바뀌지 않아도 되게 한다.
 */
#define covers_traps(xx, yy) covers_objects(xx, yy)

/**
 * @name Temporary display control
 * @brief Instructions for drawing something transient across the map -- a beam, a thrown object, a flash.
 *
 * Passed where a position would go, which is why they are negative: one call draws at a position or gives an instruction, and the sign
 * distinguishes them.
 *
 * The instructions differ in two ways, and the pairs make that visible. A beam leaves every square it has touched showing until the
 * end; a flash cleans up each square before the next. And for each, one form draws only where the hero can see and the other draws
 * regardless -- which is what lets an effect the hero is not watching still be shown when it should be.
 *
 * @note @c DISP_TETHER exists because a tethered weapon shows a line behind it that is drawn differently from its head.
 * @warning @c DISP_FREEMEM is not part of drawing. It releases the machinery's memory and is for shutdown only.
 * @{
 */
/**
 * @name 일시적 표시 제어
 * @brief 지도를 가로지르는 일시적인 것 -- 광선, 던진 물건, 섬광 -- 을 그리기 위한 지시.
 *
 * 위치가 들어갈 자리에 전달되며, 그래서 음수다. 하나의 호출이 어떤 위치에 그리거나 지시를 주고, 부호가 그 둘을 구별한다.
 *
 * 지시들은 두 가지 방식으로 다르며, 그 짝들이 그것을 드러낸다. 광선은 자신이 닿은 모든 칸을 끝까지 보이게 남기고, 섬광은 다음 칸으로 가기 전에 각 칸을 정리한다. 그리고 각각에 대해, 한 형태는 영웅이 볼 수 있는
 * 곳에만 그리고 다른 형태는 상관없이 그린다. 그것이 영웅이 보고 있지 않은 효과도 보여야 할 때 보이게 하는 것이다.
 *
 * @note @c DISP_TETHER 가 있는 것은, 줄이 달린 무기가 자기 뒤로 머리와 다르게 그려지는 선을 보이기 때문이다.
 * @warning @c DISP_FREEMEM 은 그리기의 일부가 아니다. 그 기제의 메모리를 놓아주며 종료 시에만 쓰인다.
 * @{
 */
/*
 * tmp_at() control calls.
 */
#define DISP_BEAM    (-1) /* Keep all glyphs showing & clean up at end. */
#define DISP_ALL     (-2) /* Like beam, but still displayed if not visible. */
#define DISP_TETHER  (-3) /* Like beam, but tether glyph differs from final */
#define DISP_FLASH   (-4) /* Clean up each glyph before displaying new one. */
#define DISP_ALWAYS  (-5) /* Like flash, but still displayed if not visible */
#define DISP_CHANGE  (-6) /* Change glyph. */
#define DISP_END     (-7) /* Clean up. */
#define DISP_FREEMEM (-8) /* Free all memory during exit only. */
/** @} */

/**
 * @def SHIELD_COUNT
 * @brief How many frames the shield effect animation has.
 * @note Derived from the length of the frame table rather than chosen, so adding a frame does not need this updated by hand.
 */
/**
 * @def SHIELD_COUNT
 * @brief 방어막 효과 애니메이션이 몇 프레임인지.
 * @note 고른 값이 아니라 프레임 표의 길이에서 나온다. 그래서 프레임을 더할 때 이것을 손으로 갱신할 필요가 없다.
 */
/* Total number of cmap indices in the shield_static[] array. */
#define SHIELD_COUNT 21
/**
 * @def BACKTRACK
 * @brief Ask the cleanup step to redraw each square the effect passed through, in reverse.
 * @note Passed where a position would go, like the control instructions above. It exists for effects that should be seen retreating
 *       rather than simply vanishing.
 */
/**
 * @def BACKTRACK
 * @brief 정리 단계에게 그 효과가 지나온 각 칸을 역순으로 다시 그리라고 요청한다.
 * @note 위의 제어 지시들처럼 위치가 들어갈 자리에 전달된다. 그냥 사라지는 것이 아니라 물러나는 것으로 보여야 하는 효과를 위해 존재한다.
 */
#define BACKTRACK    (-1) /* for DISP_END to display each prior location */

/*
 * display_self()
 *
 * Display the hero.  It is assumed that all checks necessary to determine
 * _if_ the hero can be seen have already been done.
 */
/**
 * @def maybe_display_usteed
 * @brief The mount if the hero is riding a visible one, otherwise whatever was given.
 * @param otherwise_self the glyph to use when there is no visible mount
 * @note A rider and their mount share a square, and it is the mount that is seen. So this sits in front of every way of drawing the
 *       hero rather than being one more case inside it.
 */
/**
 * @def maybe_display_usteed
 * @brief 영웅이 보이는 탈것을 타고 있으면 그 탈것, 아니면 주어진 것.
 * @param otherwise_self 보이는 탈것이 없을 때 쓸 글리프
 * @note 탄 자와 그 탈것은 한 칸을 공유하고, 보이는 것은 탈것이다. 그래서 이것은 영웅을 그리는 방식 안의 또 하나의 경우가 아니라 그 모든 방식의 앞에 놓인다.
 */
#define maybe_display_usteed(otherwise_self)                 \
    ((u.usteed && mon_visible(u.usteed))                     \
         ? ridden_mon_to_glyph(u.usteed, rn2_on_display_rng) \
         : (otherwise_self))

/**
 * @def display_self
 * @brief Draw the hero, as whatever they currently appear to be.
 *
 * The hero is not always drawn as the hero. A polymorphed hero may be mimicking a piece of furniture, an object, or another monster,
 * and each of those is drawn from a different range of glyphs -- so the appearance kind selects which conversion to use.
 *
 * @warning Assumes the decision has already been made that the hero should be shown. The existing comment is explicit that every check
 *          of whether the hero can be seen belongs to the caller.
 */
/**
 * @def display_self
 * @brief 영웅을 그린다. 지금 무엇으로 보이는 상태이든 그대로.
 *
 * 영웅이 항상 영웅으로 그려지는 것은 아니다. 변신한 영웅은 설비나 물건이나 다른 몬스터를 모방하고 있을 수 있고, 그것들 각각이 서로 다른 글리프 구간에서 그려진다. 그래서 외형의 종류가 어느 변환을 쓸지 고른다.
 *
 * @warning 영웅이 보여져야 한다는 판단이 이미 내려졌다고 가정한다. 기존 주석은 영웅이 보일 수 있는지에 대한 모든 검사가 호출자의 몫이라고 명시한다.
 */
#define display_self() \
    show_glyph(u.ux, u.uy, maybe_display_usteed(                        \
        ((int) U_AP_TYPE == M_AP_NOTHING)                               \
        ? hero_glyph                                                    \
        : ((int) U_AP_TYPE == M_AP_FURNITURE)                           \
          ? cmap_to_glyph((int) gy.youmonst.mappearance)                \
          : ((int) U_AP_TYPE == M_AP_OBJECT)                            \
            ? objnum_to_glyph((int) gy.youmonst.mappearance)            \
            /* else U_AP_TYPE == M_AP_MONSTER */                        \
            : monnum_to_glyph((int) gy.youmonst.mappearance, Ugender)))

/*
 * NetHack glyphs
 *
 * A glyph is an abstraction that represents a _unique_ monster, object,
 * dungeon part, or effect.  The uniqueness is important.  For example,
 * It is not enough to have four (one for each "direction") zap beam glyphs,
 * we need a set of four for each beam type.  Why go to so much trouble?
 * Because it is possible that any given window dependent display driver
 * [print_glyph()] can produce something different for each type of glyph.
 * That is, a beam of cold and a beam of fire would not only be different
 * colors, but would also be represented by different symbols.
 */


#include "color.h"
/* 3.6.3: poison gas zap used to be yellow and acid zap was green,
   which conflicted with the corresponding dragon colors */
/**
 * @brief The colour each kind of magical beam is drawn in.
 * @note Named per beam rather than derived from the damage type so that two beams which resist the same way can still look different.
 * @note The existing comment records that two of these were changed because their old colours clashed with the dragons of the
 *       corresponding kinds -- a player should not confuse a beam with the creature that breathes it.
 */
/**
 * @brief 각 종류의 마법 광선이 그려지는 색.
 * @note 피해 종류에서 유도되지 않고 광선마다 이름 붙어 있다. 그래서 같은 방식으로 저항되는 두 광선이 여전히 다르게 보일 수 있다.
 * @note 기존 주석은 이 중 둘이 예전 색이 해당 종류의 용과 충돌했기 때문에 바뀌었다고 기록한다. 플레이어가 광선과 그것을 내뿜는 생물을 혼동해서는 안 된다.
 */
enum zap_colors {
    zap_color_missile    = HI_ZAP,
    zap_color_fire       = CLR_ORANGE,
    zap_color_frost      = CLR_WHITE,
    zap_color_sleep      = HI_ZAP,
    zap_color_death      = CLR_BLACK,
    zap_color_lightning  = CLR_WHITE,
    zap_color_poison_gas = CLR_GREEN,
    zap_color_acid       = CLR_YELLOW
};

/**
 * @brief The colour an altar is drawn in, according to its alignment.
 *
 * By default every aligned altar is the same colour, and only the build option makes them differ. That is not an oversight: the
 * alignment colours are meaningful -- they match the unicorns -- and a terminal that renders them wrongly would be actively
 * misleading rather than merely plain.
 *
 * @warning The existing comment records exactly how those colours are misrendered on one common terminal, and concludes that there is
 *          not enough information available to correct for it per platform. Hence the distinction being opt-in.
 */
/**
 * @brief 제단이 그 진영에 따라 그려지는 색.
 *
 * 기본으로는 진영이 있는 모든 제단이 같은 색이며, 빌드 옵션만이 그것들을 다르게 만든다. 그것은 빠뜨림이 아니다. 진영 색은 의미를 지닌다. 유니콘과 대응한다. 그리고 그것을 잘못 그리는 터미널은 단지 단조로운 것이
 * 아니라 적극적으로 오해를 낳게 된다.
 *
 * @warning 기존 주석은 흔한 어느 터미널에서 그 색들이 정확히 어떻게 잘못 그려지는지 기록하고, 플랫폼별로 그것을 보정할 만한 정보가 충분하지 않다고 결론짓는다. 그 구별이 선택 사항인 이유가 그것이다.
 */
enum altar_colors {
    altar_color_unaligned = CLR_RED,
#if defined(USE_GENERAL_ALTAR_COLORS)
        /* On OSX with TERM=xterm-color256 these render as
         *  white -> tty: gray, curses: ok
         *  gray  -> both tty and curses: black
         *  black -> both tty and curses: blue
         *  red   -> both tty and curses: ok.
         * Since the colors have specific associations (with the
         * unicorns matched with each alignment), we shouldn't use
         * scrambled colors and we don't have sufficient information
         * to handle platform-specific color variations.
         */
    altar_color_chaotic = CLR_BLACK,
    altar_color_neutral = CLR_GRAY,
    altar_color_lawful  = CLR_WHITE,
#else
    altar_color_chaotic = CLR_GRAY,
    altar_color_neutral = CLR_GRAY,
    altar_color_lawful  = CLR_GRAY,
#endif
    altar_color_other = CLR_BRIGHT_MAGENTA,
};

/* types of explosions */
enum explosion_types {
    EXPL_DARK = 0,
    EXPL_NOXIOUS = 1,
    EXPL_MUDDY = 2,
    EXPL_WET = 3,
    EXPL_MAGICAL = 4,
    EXPL_FIERY = 5,
    EXPL_FROSTY = 6,
    EXPL_MAX = 7
};

/* above plus this redundant? */
/**
 * @brief The kinds of explosion, as glyph range offsets.
 * @warning Nearly the same list as @c explosion_types above, and the comment there asks whether one is redundant. They are not
 *          interchangeable: those are the values the rules use, these index the glyph ranges. Substituting one for the other draws the
 *          wrong explosion.
 */
/**
 * @brief 폭발의 종류들. 글리프 구간 오프셋으로.
 * @warning 위의 @c explosion_types 와 거의 같은 목록이며, 그곳의 주석은 하나가 잉여인지 묻고 있다. 그것들은 서로 바꿔 쓸 수 없다. 그쪽은 규칙이 쓰는 값이고, 이쪽은 글리프 구간을 색인한다. 하나를 다른 하나로
 *          갈아 넣으면 잘못된 폭발이 그려진다.
 */
enum expl_types {
    expl_dark,
    expl_noxious,
    expl_muddy,
    expl_wet,
    expl_magical,
    expl_fiery,
    expl_frosty,
};

/**
 * @brief The colour each kind of explosion is drawn in.
 */
/**
 * @brief 각 종류의 폭발이 그려지는 색.
 */
enum explode_colors {
    explode_color_dark = CLR_BLACK,
    explode_color_noxious = CLR_GREEN,
    explode_color_muddy = CLR_BROWN,
    explode_color_wet = CLR_BLUE,
    explode_color_magical = CLR_MAGENTA,
    explode_color_fiery = CLR_ORANGE,
    explode_color_frosty = CLR_WHITE
};
/**
 * @brief The kinds of altar, as glyph range offsets.
 * @note @c altar_other is not a fifth alignment but an altar whose alignment does not correspond to any of the hero's -- so it exists
 *       because the glyph range must cover altars the player has no word for.
 */
/**
 * @brief 제단의 종류들. 글리프 구간 오프셋으로.
 * @note @c altar_other 는 다섯 번째 진영이 아니라 영웅의 어느 진영과도 대응하지 않는 제단이다. 글리프 구간이 플레이어에게 이름이 없는 제단까지 덮어야 하므로 존재한다.
 */
enum altar_types {
    altar_unaligned,
    altar_chaotic,
    altar_neutral,
    altar_lawful,
    altar_other
};
/**
 * @brief The kinds of wall, which differ by where in the dungeon they are.
 * @note The same wall is drawn differently in the mines, in Gehennom and in Sokoban. Each gets its own glyph range, so a level's walls
 *       can look like its own place without the terrain type having to differ.
 */
/**
 * @brief 벽의 종류들. 던전의 어디에 있는지에 따라 다르다.
 * @note 같은 벽이 광산에서, 게헨놈에서, 소코반에서 다르게 그려진다. 각각이 자기 글리프 구간을 가지므로, 지형 타입이 달라지지 않고도 레벨의 벽이 그곳답게 보일 수 있다.
 */
enum level_walls  { main_walls, mines_walls, gehennom_walls,
                    knox_walls, sokoban_walls };
/**
 * @brief The three parts of a glyph's drawing that a customisation may replace.
 * @note Used as indices into a per-glyph table, so a player can change one glyph's colour without disturbing its character.
 */
/**
 * @brief 사용자 지정이 교체할 수 있는 글리프 그리기의 세 부분.
 * @note 글리프별 표의 색인으로 쓰인다. 그래서 플레이어가 어떤 글리프의 문자를 건드리지 않고 그 색만 바꿀 수 있다.
 */
enum { GM_FLAGS, GM_TTYCHAR, GM_COLOR, NUM_GLYPHMOD }; /* glyphmod entries */
/**
 * @brief Why the glyph table needs rebuilding.
 *
 * The mapping from glyphs to their appearance is worked out once and cached, so something has to say when it has gone stale. The reasons
 * are distinct because they differ in how much has to be redone -- a new game rebuilds everything, a change of level only what depends
 * on where the hero is.
 */
/**
 * @brief 글리프 표를 왜 다시 만들어야 하는지.
 *
 * 글리프에서 그 외형으로의 대응은 한 번 계산되어 캐시되므로, 그것이 낡았음을 말해 주는 것이 있어야 한다. 그 이유들이 구별되는 것은 다시 해야 하는 양이 다르기 때문이다. 새 게임은 전부를 다시 만들고, 레벨 변경은 영웅이
 * 어디 있는지에 달린 것만을 다시 만든다.
 */
enum glyphmap_change_triggers { gm_nochange, gm_newgame, gm_levelchange,
                                gm_optionchange, gm_symchange,
                                gm_accessibility_change };
/**
 * @def NUM_ZAP
 * @brief How many kinds of magical beam there are.
 * @note Each kind needs its own set of four directional glyphs, so this multiplies into the size of the beam glyph range.
 */
/**
 * @def NUM_ZAP
 * @brief 마법 광선의 종류가 몇 가지인지.
 * @note 각 종류가 네 방향의 글리프 묶음을 따로 필요로 하므로, 이 값이 광선 글리프 구간의 크기에 곱해진다.
 */
#define NUM_ZAP 8 /* number of zap beam types */

/*
 * Glyphs are grouped for easy accessibility:
 *
 * male monsters    Represents all the wild (not tame) male monsters.
 *                  Count: NUMMONS.
 *
 * female monsters  Represents all the wild (not tame) female monsters.
 *                  Count: NUMMONS.
 *
 * male pets        Represents all of the male tame monsters.
 *                  Count: NUMMONS.
 *
 * female pets      Represents all of the female tame monsters.
 *                  Count: NUMMONS.
 *
 * invisible        Invisible monster placeholder.
 *                  Count: 1.
 *
 * detect (male)    Represents all detected male monsters.
 *                  Count: NUMMONS.
 *
 * detect (female)  Represents all detected female monsters.
 *                  Count: NUMMONS.
 *
 * corpse           One for each monster (male/female not differentiated).
 *                  Count: NUMMONS.
 *
 * ridden (male)    Represents all male monsters being ridden.
 *                  Count: NUMMONS
 *
 * ridden (female)  Represents all female monsters being ridden.
 *                  Count: NUMMONS
 *
 * object           One for each type of object.  The first entry is
 *                  'strange object', the next 1..MAXOCLASSES-1 entries
 *                  are generic (one for each class, including one for
 *                  strange object's 'illobj class'), the rest are
 *                  regular objects.  Some members of the generic
 *                  subset are used to prevent color of potions, gems,
 *                  and spellbooks from being revealed when obj->dknown
 *                  hasn't been set, avoiding a bug which had been
 *                  present since day one of color support.
 *                  Count: NUM_OBJECTS
 *
 * Stone            Stone
 *                  Count: 1
 *
 * main walls       level walls (main)
 *                  Count: (S_trwall - S_vwall) + 1 = 11
 *
 * mines walls      level walls (mines)
 *                  Count: (S_trwall - S_vwall) + 1 = 11
 *
 * gehennom walls   level walls (gehennom)
 *                  Count: (S_trwall - S_vwall) + 1 = 11
 *
 * knox walls       level walls (knox)
 *                  Count: (S_trwall - S_vwall) + 1 = 11
 *
 * sokoban walls    level walls (sokoban)
 *                  Count: (S_trwall - S_vwall) + 1 = 11
 *
 * cmap A           S_ndoor through S_brdnladder
 *                  Count: (S_brdnladder - S_ndoor) + 1 = 19
 *
 * Altars           Altar (unaligned, chaotic, neutral, lawful, other)
 *                  Count: 5
 *
 * cmap B           S_grave through S_arrow_trap + TRAPNUM - 1
 *                  Count: (S_arrow_trap + (TRAPNUM - 1) - S_grave) = 39
 *
 * zap beams        set of four (there are four directions) HI_ZAP.
 *                  Count: 4 * NUM_ZAP
 *
 * cmap C           S_digbeam through S_goodpos
 *                  Count: (S_goodpos - S_digbeam) + 1 = 10
 *
 * swallow          A set of eight for each monster.  The eight positions
 *                  represent those surrounding the hero.  The monster
 *                  number is shifted over 3 positions and the swallow
 *                  position is stored in the lower three bits.
 *                  Count: NUMMONS << 3
 *
 * dark explosions        A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * noxious explosions     A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * muddy explosions       A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * wet explosions         A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * magical explosions     A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * fiery explosions       A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * frosty explosions      A set of nine.
 *                        Count: MAXEXPCHAR
 *
 * warning                A set of six representing the different warning
 *                        levels.
 *                        Count: 6
 *
 * statues (male)         One for each male monster.
 *                        Count: NUMMONS
 *
 * statues (female)       One for each female mo nster.
 *                        Count: NUMMONS
 *
 * objects piletop        Represents the top of a pile as well as
 *                        the object.
 *                        Count: NUM_OBJECTS
 *
 * bodies piletop         Represents the top of a pile as well as
 *                        the object, corpse in this case.
 *                        Count: NUMMONS
 *
 * male statues piletop   Represents the top of a pile as well as
 *                        the statue of a male monster.
 *                        Count: NUMMONS
 *
 * female statues piletop Represents the top of a pile as well as
 *                        the statue of a female monster.
 *                        Count: NUMMONS
 *
 * unexplored             One for unexplored areas of the map
 * nothing                Nothing but background
 *
 * The following are offsets used to convert to and from a glyph.
 */

/**
 * @brief Where each range of glyphs begins.
 *
 * The whole glyph numbering, laid out as a sequence of contiguous ranges. Each entry is the start of a range and is computed from the
 * previous entry plus that range's size, so the layout is described once and the numbers follow.
 *
 * This arithmetic is the point of the whole scheme. Turning a monster into a glyph is adding its index to the appropriate range start;
 * asking whether a glyph is a pet is comparing against that range's ends. Neither needs a table.
 *
 * The ranges are exhaustive in a way that repays reading the long comment above: there is a separate range for every state a monster can
 * be perceived in -- wild, tame, ridden, detected, as a statue, as a corpse -- and each of those is doubled for the two genders. That is
 * why the numbering is large, and why nothing needs to be combined at draw time.
 *
 * @warning Every entry depends on the one before it. Inserting a range, or changing a count that feeds one, shifts every glyph after it
 *          -- which is harmless within a run and breaks anything that stored a glyph, including a saved level.
 */
/**
 * @brief 각 글리프 구간이 어디서 시작하는지.
 *
 * 글리프 번호 체계 전체를 연속된 구간의 나열로 놓은 것. 각 항목은 한 구간의 시작이며 이전 항목에 그 구간의 크기를 더해 계산된다. 그래서 배치는 한 번만 기술되고 숫자는 따라 나온다.
 *
 * 이 산술이 이 방식 전체의 요점이다. 몬스터를 글리프로 바꾸는 것은 그 색인을 해당 구간의 시작에 더하는 일이고, 글리프가 애완동물인지 묻는 것은 그 구간의 양 끝과 비교하는 일이다. 어느 쪽도 표를 필요로 하지 않는다.
 *
 * 구간들은 위의 긴 주석을 읽어 볼 만한 방식으로 빠짐없다. 몬스터가 지각될 수 있는 모든 상태 -- 야생, 길들여짐, 타고 있음, 탐지됨, 조각상으로, 시체로 -- 마다 별개의 구간이 있고, 그 각각이 두 성별에 대해 두 배가 된다.
 * 번호 체계가 큰 이유이고, 그릴 때 무엇도 조합할 필요가 없는 이유다.
 *
 * @warning 모든 항목이 그 앞의 항목에 의존한다. 구간을 끼워 넣거나 어떤 구간에 들어가는 개수를 바꾸면 그 뒤의 모든 글리프가 밀린다. 한 번의 실행 안에서는 해롭지 않지만, 저장된 레벨을 포함해 글리프를 저장한 모든 것을
 *          깨뜨린다.
 */
enum glyph_offsets {
    GLYPH_MON_OFF = 0,
    GLYPH_MON_MALE_OFF = (GLYPH_MON_OFF),
    GLYPH_MON_FEM_OFF = (NUMMONS + GLYPH_MON_MALE_OFF),
    GLYPH_PET_OFF = (NUMMONS + GLYPH_MON_FEM_OFF),
    GLYPH_PET_MALE_OFF = (GLYPH_PET_OFF),
    GLYPH_PET_FEM_OFF = (NUMMONS + GLYPH_PET_MALE_OFF),
    GLYPH_INVIS_OFF = (NUMMONS + GLYPH_PET_FEM_OFF),
    GLYPH_DETECT_OFF = (1 + GLYPH_INVIS_OFF),
    GLYPH_DETECT_MALE_OFF = (GLYPH_DETECT_OFF),
    GLYPH_DETECT_FEM_OFF = (NUMMONS + GLYPH_DETECT_MALE_OFF),
    GLYPH_BODY_OFF = (NUMMONS + GLYPH_DETECT_FEM_OFF),
    GLYPH_RIDDEN_OFF = (NUMMONS + GLYPH_BODY_OFF),
    GLYPH_RIDDEN_MALE_OFF = (GLYPH_RIDDEN_OFF),
    GLYPH_RIDDEN_FEM_OFF = (NUMMONS + GLYPH_RIDDEN_MALE_OFF),
    GLYPH_OBJ_OFF = (NUMMONS + GLYPH_RIDDEN_FEM_OFF),
    GLYPH_CMAP_OFF = (NUM_OBJECTS + GLYPH_OBJ_OFF),
    GLYPH_CMAP_STONE_OFF = (GLYPH_CMAP_OFF),
    GLYPH_CMAP_MAIN_OFF = (1 + GLYPH_CMAP_STONE_OFF),
    GLYPH_CMAP_MINES_OFF = (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_MAIN_OFF),
    GLYPH_CMAP_GEH_OFF = (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_MINES_OFF),
    GLYPH_CMAP_KNOX_OFF = (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_GEH_OFF),
    GLYPH_CMAP_SOKO_OFF = (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_KNOX_OFF),
    GLYPH_CMAP_A_OFF = (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_SOKO_OFF),
    GLYPH_ALTAR_OFF = (((S_brdnladder - S_ndoor) + 1) + GLYPH_CMAP_A_OFF),
    GLYPH_CMAP_B_OFF = (5 + GLYPH_ALTAR_OFF),
    GLYPH_ZAP_OFF = ((S_arrow_trap + MAXTCHARS - S_grave) + GLYPH_CMAP_B_OFF),
    GLYPH_CMAP_C_OFF = ((NUM_ZAP << 2) + GLYPH_ZAP_OFF),
    GLYPH_SWALLOW_OFF = (((S_goodpos - S_digbeam) + 1) + GLYPH_CMAP_C_OFF),
    GLYPH_EXPLODE_OFF = ((NUMMONS << 3) + GLYPH_SWALLOW_OFF),
    GLYPH_EXPLODE_DARK_OFF = (GLYPH_EXPLODE_OFF),
    GLYPH_EXPLODE_NOXIOUS_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_DARK_OFF),
    GLYPH_EXPLODE_MUDDY_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_NOXIOUS_OFF),
    GLYPH_EXPLODE_WET_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_MUDDY_OFF),
    GLYPH_EXPLODE_MAGICAL_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_WET_OFF),
    GLYPH_EXPLODE_FIERY_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_MAGICAL_OFF),
    GLYPH_EXPLODE_FROSTY_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_FIERY_OFF),
    GLYPH_WARNING_OFF = (MAXEXPCHARS + GLYPH_EXPLODE_FROSTY_OFF),
    GLYPH_STATUE_OFF = (WARNCOUNT + GLYPH_WARNING_OFF),
    GLYPH_STATUE_MALE_OFF = (GLYPH_STATUE_OFF),
    GLYPH_STATUE_FEM_OFF = (NUMMONS + GLYPH_STATUE_MALE_OFF),
    GLYPH_PILETOP_OFF = (NUMMONS + GLYPH_STATUE_FEM_OFF),
    GLYPH_OBJ_PILETOP_OFF = (GLYPH_PILETOP_OFF),
    GLYPH_BODY_PILETOP_OFF = (NUM_OBJECTS + GLYPH_OBJ_PILETOP_OFF),
    GLYPH_STATUE_MALE_PILETOP_OFF = (NUMMONS + GLYPH_BODY_PILETOP_OFF),
    GLYPH_STATUE_FEM_PILETOP_OFF = (NUMMONS + GLYPH_STATUE_MALE_PILETOP_OFF),
    GLYPH_UNEXPLORED_OFF = (NUMMONS + GLYPH_STATUE_FEM_PILETOP_OFF),
    GLYPH_NOTHING_OFF = (GLYPH_UNEXPLORED_OFF + 1),
    MAX_GLYPH
};

/**
 * @def NO_GLYPH
 * @brief The value meaning "nothing to draw", one past the last real glyph.
 * @warning Not a glyph. Using it where one is expected reads past the end of every per-glyph table.
 */
/**
 * @def NO_GLYPH
 * @brief "그릴 것 없음"을 뜻하는 값. 마지막 실제 글리프보다 하나 뒤다.
 * @warning 글리프가 아니다. 글리프를 기대하는 곳에 쓰면 모든 글리프별 표의 끝을 넘어 읽는다.
 */
#define NO_GLYPH          MAX_GLYPH
/**
 * @name Glyphs that stand for a state of knowledge
 * @brief The three single glyphs that are not a thing.
 * @note Each has a range of exactly one, so the range start is the glyph. They are named without the offset suffix because they are used
 *       as values rather than as bases to add to.
 * @note An invisible monster, a square never seen, and a square known to hold nothing are three different situations and each needs its
 *       own appearance.
 * @{
 */
/**
 * @name 앎의 상태를 나타내는 글리프
 * @brief 사물이 아닌 세 개의 단일 글리프.
 * @note 각각의 구간이 정확히 하나이므로 구간의 시작이 곧 그 글리프다. 더할 기준이 아니라 값으로 쓰이므로 오프셋 접미사 없이 이름 붙어 있다.
 * @note 투명한 몬스터, 한 번도 본 적 없는 칸, 아무것도 없다고 알려진 칸은 세 가지 다른 상황이며 각각 자기 외형을 필요로 한다.
 * @{
 */
#define GLYPH_INVISIBLE   GLYPH_INVIS_OFF
#define GLYPH_UNEXPLORED  GLYPH_UNEXPLORED_OFF
#define GLYPH_NOTHING     GLYPH_NOTHING_OFF
/** @} */

/**
 * @name Building a glyph
 * @brief Turning something the hero perceives into the number that draws it.
 *
 * Each of these is an addition: the thing's own index plus the start of the range for the state it is perceived in. That is the whole
 * conversion, which is why the ranges have to be contiguous and why their order is interface rather than presentation.
 *
 * The monster forms come in one per perceived state -- seen, detected, ridden, tame -- and each selects between the two gender ranges.
 * A monster's gender is part of its appearance, so it is part of the glyph rather than something applied afterwards.
 *
 * @note Every monster form passes through the hallucination substitution, which is why each takes a generator. Drawing is the only place
 *       hallucination is applied, so it has to be applied here.
 * @{
 */
/**
 * @name 글리프 만들기
 * @brief 영웅이 지각한 것을 그것을 그리는 숫자로 바꾸기.
 *
 * 이들 각각은 덧셈이다. 그 사물 자신의 색인에 그것이 지각된 상태에 해당하는 구간의 시작을 더한 것. 그것이 변환 전체이며, 그래서 구간이 연속해야 하고 그 순서가 표현이 아니라 인터페이스인 것이다.
 *
 * 몬스터 형태는 지각된 상태마다 하나씩 있다. 보임, 탐지됨, 타고 있음, 길들여짐. 그리고 각각이 두 성별 구간 중에서 고른다. 몬스터의 성별은 그 외형의 일부이므로, 나중에 적용되는 것이 아니라 글리프의 일부다.
 *
 * @note 모든 몬스터 형태가 환각 갈아치우기를 거치며, 그래서 각각이 생성기를 받는다. 환각이 적용되는 곳은 그리기뿐이므로 여기서 적용되어야 한다.
 * @{
 */
/**
 * @def warning_to_glyph
 * @brief The glyph for a warning of a given severity.
 * @param mwarnlev the warning level, not a monster
 */
/**
 * @def warning_to_glyph
 * @brief 주어진 심각도의 경고에 해당하는 글리프.
 * @param mwarnlev 경고 단계. 몬스터가 아니다
 */
#define warning_to_glyph(mwarnlev) ((mwarnlev) + GLYPH_WARNING_OFF)
#define mon_to_glyph(mon, rng) \
    ((int) what_mon(monsndx((mon)->data), rng)                          \
     + (((mon)->female == 0) ?  GLYPH_MON_MALE_OFF : GLYPH_MON_FEM_OFF))
#define detected_mon_to_glyph(mon, rng) \
    ((int) what_mon(monsndx((mon)->data), rng)                          \
     + (((mon)->female == 0) ?  GLYPH_DETECT_MALE_OFF : GLYPH_DETECT_FEM_OFF))
#define ridden_mon_to_glyph(mon, rng) \
    ((int) what_mon(monsndx((mon)->data), rng)                          \
     + (((mon)->female == 0) ?  GLYPH_RIDDEN_MALE_OFF : GLYPH_RIDDEN_FEM_OFF))
#define pet_to_glyph(mon, rng) \
    ((int) what_mon(monsndx((mon)->data), rng)                          \
     + (((mon)->female == 0) ? GLYPH_PET_MALE_OFF : GLYPH_PET_FEM_OFF))
/** @} */

/**
 * @def altar_to_glyph
 * @brief The glyph for an altar of a given alignment.
 * @param amsk the altar's alignment mask, not an alignment value
 * @note The high altar is checked first because it is a stronger claim than any alignment -- a sanctum altar is drawn as its own thing
 *       even though it also has an alignment.
 * @note Unaligned is left as the final case rather than tested for, as the existing comment records. The field holds three bits with four
 *       defined values, so four combinations are unused and would otherwise have no answer.
 */
/**
 * @def altar_to_glyph
 * @brief 주어진 진영의 제단에 해당하는 글리프.
 * @param amsk 제단의 진영 마스크. 진영 값이 아니다
 * @note 대제단이 먼저 검사되는 것은, 그것이 어떤 진영보다 강한 주장이기 때문이다. 성소의 제단은 진영도 함께 가지지만 그 자체로 그려진다.
 * @note 기존 주석이 기록하듯 무진영은 검사되지 않고 마지막 경우로 남겨진다. 그 필드는 3비트에 정의된 값이 넷이므로 네 조합이 쓰이지 않으며, 그렇지 않으면 그것들에는 답이 없게 된다.
 */
#define altar_to_glyph(amsk) \
    ((((amsk) & AM_SANCTUM) == AM_SANCTUM)                \
      ? (GLYPH_ALTAR_OFF + altar_other)                   \
      : (((amsk) & AM_MASK) == AM_LAWFUL)                 \
         ? (GLYPH_ALTAR_OFF + altar_lawful)               \
         : (((amsk) & AM_MASK) == AM_NEUTRAL)             \
            ? (GLYPH_ALTAR_OFF + altar_neutral)           \
            : (((amsk) & AM_MASK) == AM_CHAOTIC)          \
               ? (GLYPH_ALTAR_OFF + altar_chaotic)        \
               /* (((amsk) & AM_MASK) == AM_UNALIGNED) */ \
               : (GLYPH_ALTAR_OFF + altar_unaligned))

/* not used, nor is it correct
#define zap_to_glyph(zaptype, cmap_idx) \
    ((((cmap_idx) - S_vbeam) + 1) + GLYPH_ZAP_OFF)
*/

/**
 * @def explosion_to_glyph
 * @brief The glyph for one of the nine parts of an explosion.
 * @param expltyp which kind of explosion
 * @param idx which of the nine positions -- corner, edge, centre
 * @note An explosion covers three squares by three and each position is drawn differently, so this converts a position within that shape
 *       rather than a single symbol.
 * @note Fiery is the final case rather than one of the tested ones, as the existing comment records, so an unrecognised kind is drawn as
 *       fire rather than not at all.
 */
/**
 * @def explosion_to_glyph
 * @brief 폭발의 아홉 부분 중 하나에 해당하는 글리프.
 * @param expltyp 어떤 종류의 폭발인지
 * @param idx 아홉 위치 중 어디인지. 모서리, 변, 중앙
 * @note 폭발은 3×3을 덮고 각 위치가 다르게 그려진다. 그래서 이것은 심볼 하나가 아니라 그 모양 안의 위치를 변환한다.
 * @note 기존 주석이 기록하듯 불꽃이 검사되는 경우 중 하나가 아니라 마지막 경우다. 그래서 알 수 없는 종류는 아예 그려지지 않는 대신 불로 그려진다.
 */
/* EXPL_FIERY is the default explosion type */
#define explosion_to_glyph(expltyp, idx) \
    ((idx) - S_expl_tl                                                  \
     + (((expltyp) == EXPL_FROSTY) ? GLYPH_EXPLODE_FROSTY_OFF           \
        : ((expltyp) == EXPL_MAGICAL) ? GLYPH_EXPLODE_MAGICAL_OFF       \
          : ((expltyp) == EXPL_WET) ? GLYPH_EXPLODE_WET_OFF             \
           : ((expltyp) == EXPL_MUDDY) ? GLYPH_EXPLODE_MUDDY_OFF        \
             : ((expltyp) == EXPL_NOXIOUS) ? GLYPH_EXPLODE_NOXIOUS_OFF  \
               : GLYPH_EXPLODE_FIERY_OFF))

/**
 * @def cmap_walls_to_glyph
 * @brief The glyph for a wall, as it looks where the hero currently is.
 * @param cmap_idx the wall symbol
 * @warning Reads the hero's own level, so this is wrong for a wall anywhere else. Describing a level the hero is not on needs the
 *          main-dungeon form instead.
 */
/**
 * @def cmap_walls_to_glyph
 * @brief 벽에 해당하는 글리프. 영웅이 지금 있는 곳에서 보이는 모습으로.
 * @param cmap_idx 벽 심볼
 * @warning 영웅 자신의 레벨을 읽으므로, 다른 곳의 벽에 대해서는 틀리다. 영웅이 있지 않은 레벨을 기술할 때는 대신 본 던전용 형태가 필요하다.
 */
/* cmap_walls_to_glyph(): return the glyph number for specified wall
   symbol; result varies by dungeon branch */
#define cmap_walls_to_glyph(cmap_idx) \
    ((cmap_idx) - S_vwall                               \
     + (In_mines(&u.uz) ? GLYPH_CMAP_MINES_OFF          \
        : In_hell(&u.uz) ? GLYPH_CMAP_GEH_OFF           \
          : Is_knox(&u.uz) ? GLYPH_CMAP_KNOX_OFF        \
            : In_sokoban(&u.uz) ? GLYPH_CMAP_SOKO_OFF   \
              : GLYPH_CMAP_MAIN_OFF))

/**
 * @def cmap_D0walls_to_glyph
 * @brief The glyph for a wall as it would look in the main dungeon, wherever the hero is.
 * @param cmap_idx the wall symbol
 * @note The location-independent counterpart, for describing a wall without reference to the hero's position.
 */
/**
 * @def cmap_D0walls_to_glyph
 * @brief 영웅이 어디 있든 본 던전에서 보일 모습의 벽 글리프.
 * @param cmap_idx 벽 심볼
 * @note 위치에 의존하지 않는 짝이다. 영웅의 위치를 참조하지 않고 벽을 기술하기 위한 것.
 */
/* cmap_D0walls_to_glyph(): simpler version of cmap_walls_to_glyph()
   which returns the glyph that would be used in the main dungeon,
   regardless of hero's current location */
#define cmap_D0walls_to_glyph(cmap_idx) \
    ((cmap_idx) - S_vwall + GLYPH_CMAP_MAIN_OFF)

/**
 * @name Terrain glyph groups
 * @brief The terrain symbols split into groups, each converted against its own range.
 *
 * The terrain symbols do not form one range, because walls are drawn differently in different parts of the dungeon and altars have their
 * own set. So the rest is divided into three groups around those, and each group has its own base to add to.
 *
 * @note Callers should almost always use the combined form below, which decides which group a symbol falls in. These exist for code that
 *       already knows.
 * @warning Each subtracts a different starting symbol. Passing a symbol from the wrong group produces a number in the wrong range with
 *          no indication.
 * @{
 */
/**
 * @name 지형 글리프 묶음
 * @brief 지형 심볼을 묶음으로 나눈 것. 각 묶음이 자기 구간에 대해 변환된다.
 *
 * 지형 심볼은 하나의 구간을 이루지 않는다. 벽이 던전의 부분마다 다르게 그려지고 제단은 자기 묶음을 갖기 때문이다. 그래서 나머지가 그것들을 둘러싸고 셋으로 나뉘며, 각 묶음이 더할 자기 기준을 가진다.
 *
 * @note 호출자는 거의 항상 아래의 통합 형태를 써야 한다. 그것이 어떤 심볼이 어느 묶음에 드는지 정한다. 이들은 이미 아는 코드를 위해 존재한다.
 * @warning 각각이 서로 다른 시작 심볼을 뺀다. 잘못된 묶음의 심볼을 넘기면 아무 표시 없이 잘못된 구간의 숫자가 나온다.
 * @{
 */
#define cmap_a_to_glyph(cmap_idx) \
    (((cmap_idx) - S_ndoor) + GLYPH_CMAP_A_OFF)

#define cmap_b_to_glyph(cmap_idx) \
    (((cmap_idx) - S_grave) + GLYPH_CMAP_B_OFF)

#define cmap_c_to_glyph(cmap_idx) \
    (((cmap_idx) - S_digbeam) + GLYPH_CMAP_C_OFF)
/** @} */

/**
 * @def cmap_to_glyph
 * @brief The glyph for any terrain symbol, whichever group it belongs to.
 * @param cmap_idx the terrain symbol
 * @note The form to use. It works out which group the symbol falls in from the ordering of the terrain enumerators, which is why that
 *       ordering is interface.
 * @note An altar is converted as neutral here, since a bare symbol carries no alignment; code that knows the alignment must use the altar
 *       form directly.
 * @return the glyph, or the no-glyph value for a symbol outside every group
 */
/**
 * @def cmap_to_glyph
 * @brief 어느 묶음에 속하든 아무 지형 심볼에 대한 글리프.
 * @param cmap_idx 지형 심볼
 * @note 써야 할 형태다. 지형 열거자의 순서에서 그 심볼이 어느 묶음에 드는지 알아내며, 그 순서가 인터페이스인 이유가 그것이다.
 * @note 여기서 제단은 중립으로 변환된다. 맨 심볼은 진영을 지니지 않기 때문이다. 진영을 아는 코드는 제단 형태를 직접 써야 한다.
 * @return 글리프. 어느 묶음에도 들지 않는 심볼에 대해서는 글리프 없음 값
 */
#define cmap_to_glyph(cmap_idx) \
    ( ((cmap_idx) == S_stone)   ? GLYPH_CMAP_STONE_OFF                      \
    : ((cmap_idx) <= S_trwall)  ? cmap_walls_to_glyph(cmap_idx)             \
    : ((cmap_idx) <  S_altar)   ? cmap_a_to_glyph(cmap_idx)                 \
    : ((cmap_idx) == S_altar)   ? altar_to_glyph(AM_NEUTRAL)                \
    : ((cmap_idx) <  S_arrow_trap + MAXTCHARS) ? cmap_b_to_glyph(cmap_idx)  \
    : ((cmap_idx) <= S_goodpos) ? cmap_c_to_glyph(cmap_idx)                 \
      : NO_GLYPH )

/**
 * @def trap_to_glyph
 * @brief The glyph for a trap, by way of the terrain symbol that represents it.
 * @param trap the trap, not a trap type
 * @note Traps are drawn as terrain rather than having their own conversion, since on the map a trap is a feature of the square.
 */
/**
 * @def trap_to_glyph
 * @brief 함정에 해당하는 글리프. 그것을 나타내는 지형 심볼을 거쳐서.
 * @param trap 함정. 함정 종류가 아니다
 * @note 함정은 자기 변환을 갖는 대신 지형으로 그려진다. 지도 위에서 함정은 그 칸의 한 특징이기 때문이다.
 */
#define trap_to_glyph(trap)                                \
    cmap_to_glyph(trap_to_defsym(((int) (trap)->ttyp)))

/**
 * @def engraving_to_glyph
 * @brief The glyph for an engraving, likewise by way of a terrain symbol.
 * @param ep the engraving
 * @note Which symbol depends on what the engraving was made with, since a burned message and one written in dust are drawn differently.
 */
/**
 * @def engraving_to_glyph
 * @brief 새김에 해당하는 글리프. 마찬가지로 지형 심볼을 거쳐서.
 * @param ep 그 새김
 * @note 어떤 심볼인지는 그 새김이 무엇으로 만들어졌는지에 달려 있다. 태워 쓴 글과 먼지에 쓴 글이 다르게 그려지기 때문이다.
 */
#define engraving_to_glyph(ep)                             \
    cmap_to_glyph(engraving_to_defsym(ep))

/**
 * @name Building a glyph from a bare number
 * @brief The conversions that take an index rather than a thing.
 *
 * Used where there is no object or monster to hand -- describing a kind rather than an instance.
 *
 * @warning None of these passes through the hallucination substitution, unlike their counterparts that take a monster. That is deliberate,
 *          as the existing comment records, and it is what makes them right for naming a kind and wrong for drawing what the hero sees.
 * @note A body is drawn from the generic corpse object rather than per monster, and a statue likewise -- so the object conversion serves
 *       for both.
 * @{
 */
/**
 * @name 맨 숫자에서 글리프 만들기
 * @brief 사물이 아니라 색인을 받는 변환들.
 *
 * 손에 든 물건이나 몬스터가 없는 곳 -- 개체가 아니라 종류를 기술하는 곳 -- 에서 쓰인다.
 *
 * @warning 몬스터를 받는 짝들과 달리 이들 중 어느 것도 환각 갈아치우기를 거치지 않는다. 기존 주석이 기록하듯 의도적이며, 그것이 이들을 종류의 이름을 말하는 데 알맞게 하고 영웅이 보는 것을 그리는 데 알맞지 않게 하는
 *          것이다.
 * @note 시체는 몬스터별이 아니라 일반 시체 물건에서 그려지고, 조각상도 마찬가지다. 그래서 물건 변환이 둘 다를 맡는다.
 * @{
 */
/* Not affected by hallucination.  Gives a generic body for CORPSE */
/* MRKR: ...and the generic statue */
#define objnum_to_glyph(onum) ((int) (onum) + GLYPH_OBJ_OFF)
#define monnum_to_glyph(mnum,gnd) \
    ((int) (mnum) + (((gnd) == MALE) ? GLYPH_MON_MALE_OFF       \
                                     : GLYPH_MON_FEM_OFF))
#define detected_monnum_to_glyph(mnum,gnd) \
    ((int) (mnum) + (((gnd) == MALE) ? GLYPH_DETECT_MALE_OFF    \
                                     : GLYPH_DETECT_FEM_OFF))
#define ridden_monnum_to_glyph(mnum,gnd) \
    ((int) (mnum) + (((gnd) == MALE) ? GLYPH_RIDDEN_MALE_OFF    \
                                     : GLYPH_RIDDEN_FEM_OFF))
#define petnum_to_glyph(mnum,gnd) \
    ((int) (mnum) + (((gnd) == MALE) ? GLYPH_PET_MALE_OFF       \
                                     : GLYPH_PET_FEM_OFF))
/** @} */

/**
 * @def hero_glyph
 * @brief The hero, drawn as the monster they are.
 * @note Which monster depends on two things: the hero's current form if polymorphed, and otherwise whether the player asked to be shown by
 *       race rather than by role. So a human Valkyrie may be drawn as a Valkyrie or as a human at the player's choice.
 */
/**
 * @def hero_glyph
 * @brief 영웅. 자신이 지금 어떤 몬스터인지로 그려진다.
 * @note 어떤 몬스터인지는 두 가지에 달려 있다. 변신했다면 영웅의 현재 형태, 그렇지 않다면 플레이어가 직업이 아니라 종족으로 보여지기를 요청했는지. 그래서 인간 발키리가 플레이어의 선택에 따라 발키리로도 인간으로도
 *       그려질 수 있다.
 */
/* The hero's glyph when seen as a monster.
 */
#define hero_glyph \
    monnum_to_glyph((Upolyd || !flags.showrace) ? u.umonnum : gu.urace.mnum, \
                    (Ugender))

/*
 * Change the given glyph into its given type.  Note:
 *      1) Pets, detected, and ridden monsters are animals and are converted
 *         to the proper monster number.
 *      2) Bodies are all mapped into the generic CORPSE object
 *      3) If handed a glyph out of range for the type, these functions
 *         will return NO_GLYPH (see exception below)
 *      4) glyph_to_swallow() does not return a showsyms[] index, but an
 *         offset from the first swallow symbol.  If handed something
 *         out of range, it will return zero (for lack of anything better
 *         to return).
 */

/**
 * @name Taking a glyph apart
 * @brief Going the other way: from a glyph back to what it represents.
 *
 * The inverse of the building conversions, and lossy in places. The long comment above lists the ways: a pet, a detected monster and a
 * ridden monster all reduce to the same monster number, because the state was in the range rather than in the number; and every body
 * reduces to the generic corpse.
 *
 * @warning Handed a glyph outside the range for what is being asked, these yield the no-glyph value. That is a report, not a glyph, so a
 *          caller that uses the result without checking will index past the end of a table.
 * @warning The swallow form is the exception the comment calls out: it returns an offset within the swallow shape rather than a symbol, and
 *          on being given something out of range it returns zero rather than the no-glyph value -- so its failure is indistinguishable
 *          from a valid answer.
 * @{
 */
/**
 * @name 글리프를 분해하기
 * @brief 반대 방향. 글리프에서 그것이 나타내는 것으로.
 *
 * 만들기 변환의 역이며, 곳에 따라 정보를 잃는다. 위의 긴 주석이 그 방식들을 나열한다. 애완동물, 탐지된 몬스터, 타고 있는 몬스터가 모두 같은 몬스터 번호로 줄어든다. 그 상태가 숫자가 아니라 구간에 있었기 때문이다. 그리고
 * 모든 시체가 일반 시체로 줄어든다.
 *
 * @warning 묻는 대상의 구간을 벗어난 글리프를 받으면 이들은 글리프 없음 값을 낸다. 그것은 글리프가 아니라 보고이므로, 확인하지 않고 결과를 쓰는 호출자는 표의 끝을 넘어 색인하게 된다.
 * @warning 삼킴 형태는 그 주석이 따로 지적하는 예외다. 심볼이 아니라 삼킴 모양 안의 오프셋을 반환하며, 범위를 벗어난 것을 받으면 글리프 없음 값이 아니라 0을 반환한다. 그래서 그 실패가 유효한 답과 구별되지 않는다.
 * @{
 */
#define glyph_to_trap(glyph) \
    (glyph_is_trap(glyph)                                                   \
        ? ((int) defsym_to_trap(((glyph) - GLYPH_TRAP_OFF) + S_arrow_trap)) \
        : NO_GLYPH)

#define glyph_is_cmap_main(glyph) \
    ((glyph) >= GLYPH_CMAP_MAIN_OFF                                     \
     && (glyph) < (((S_trwall - S_vwall) +1) + GLYPH_CMAP_MAIN_OFF))
#define glyph_is_cmap_mines(glyph) \
    ((glyph) >= GLYPH_CMAP_MINES_OFF                                    \
     && (glyph) < (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_MINES_OFF))
#define glyph_is_cmap_gehennom(glyph) \
    ((glyph) >= GLYPH_CMAP_GEH_OFF                                      \
     && (glyph) < (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_GEH_OFF))
#define glyph_is_cmap_knox(glyph) \
    ((glyph) >= GLYPH_CMAP_KNOX_OFF                                     \
     && (glyph) < (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_KNOX_OFF))
#define glyph_is_cmap_sokoban(glyph) \
    ((glyph) >= GLYPH_CMAP_SOKO_OFF                                     \
     && (glyph) < (((S_trwall - S_vwall) + 1) + GLYPH_CMAP_SOKO_OFF))
#define glyph_is_cmap_a(glyph) \
    ((glyph) >= GLYPH_CMAP_A_OFF                                        \
     && (glyph) < (((S_brdnladder - S_ndoor) + 1) + GLYPH_CMAP_A_OFF))
#define glyph_is_cmap_altar(glyph) \
    ((glyph) >= GLYPH_ALTAR_OFF && (glyph) < (5 + GLYPH_ALTAR_OFF))
#define glyph_is_cmap_b(glyph) \
    ((glyph) >= GLYPH_CMAP_B_OFF                                        \
     && ((glyph) < ((S_arrow_trap + MAXTCHARS - S_grave) + GLYPH_CMAP_B_OFF)))
#define glyph_is_cmap_zap(glyph) \
    ((glyph) >= GLYPH_ZAP_OFF && (glyph) < ((NUM_ZAP << 2) + GLYPH_ZAP_OFF))
#define glyph_is_cmap_c(glyph) \
    ((glyph) >= GLYPH_CMAP_C_OFF                                        \
     && (glyph) < (((S_goodpos - S_digbeam) + 1) + GLYPH_CMAP_C_OFF))
#define glyph_is_swallow(glyph) \
    ((glyph) >= GLYPH_SWALLOW_OFF                                       \
     && (glyph) < (((NUMMONS << 3) + GLYPH_SWALLOW_OFF)))
#define glyph_is_explosion(glyph) \
    ((glyph) >= GLYPH_EXPLODE_OFF                                       \
     && (glyph) < (MAXEXPCHARS + GLYPH_EXPLODE_FROSTY_OFF))
#if 0   /* this is more precise but expands to a lot of unnecessary code */
#define glyph_is_cmap(glyph) \
    (((glyph) == GLYPH_CMAP_STONE_OFF) \
     || glyph_is_cmap_main(glyph)      \
     || glyph_is_cmap_mines(glyph)     \
     || glyph_is_cmap_gehennom(glyph)  \
     || glyph_is_cmap_knox(glyph)      \
     || glyph_is_cmap_sokoban(glyph)   \
     || glyph_is_cmap_a(glyph)         \
     || glyph_is_cmap_altar(glyph)     \
     || glyph_is_cmap_b(glyph)         \
     || glyph_is_cmap_c(glyph))
#endif
#define glyph_is_cmap(glyph) \
    ((glyph) >= GLYPH_CMAP_STONE_OFF \
     && (glyph) < (GLYPH_CMAP_C_OFF + ((S_goodpos - S_digbeam) + 1)))
#define glyph_to_swallow(glyph) \
    (glyph_is_swallow(glyph) ? (((glyph) - GLYPH_SWALLOW_OFF) & 0x7) : 0)
#define glyph_to_explosion(glyph) \
    (glyph_is_explosion(glyph) ? (((glyph) - GLYPH_EXPLODE_OFF) % (S_expl_br - S_expl_tl + 1)) : 0)
#define glyph_to_warning(glyph) \
    (glyph_is_warning(glyph) ? ((glyph) - GLYPH_WARNING_OFF) : 0)
/** @} */

/**
 * @name Asking what kind a glyph is
 * @brief Range comparisons that classify a glyph without decoding it.
 *
 * Each is a pair of comparisons against the ends of a range. That is the reason the ranges exist and the reason their order is interface:
 * classifying a glyph is two comparisons rather than a lookup, and it happens for every square every time the map is drawn.
 *
 * The tests come in layers. The narrow ones ask about one exact range -- a wild male monster, a female pet -- and the broader ones are built
 * from those, so a caller can ask "is this a monster at all" or "is this specifically a ridden female one".
 *
 * @note A body counts as an object, as the existing comment records, not as a monster. It is a thing lying on the floor.
 * @warning The argument is evaluated more than once throughout, and none of these validates its input -- any number will be classified as
 *          something or as nothing.
 * @{
 */
/**
 * @name 글리프가 어떤 종류인지 묻기
 * @brief 글리프를 해독하지 않고 분류하는 범위 비교들.
 *
 * 각각은 한 구간의 양 끝과의 두 번의 비교다. 그것이 구간이 존재하는 이유이고 그 순서가 인터페이스인 이유다. 글리프를 분류하는 것은 조회가 아니라 두 번의 비교이며, 지도를 그릴 때마다 모든 칸에 대해 일어난다.
 *
 * 검사들은 층을 이룬다. 좁은 것들은 정확히 한 구간에 대해 묻고 -- 야생의 수컷 몬스터, 암컷 애완동물 -- 넓은 것들은 그것들로 만들어진다. 그래서 호출자가 "이것이 몬스터이기라도 한가"나 "이것이 특히 타고 있는 암컷인가"를
 * 물을 수 있다.
 *
 * @note 기존 주석이 기록하듯 시체는 몬스터가 아니라 물건으로 셈해진다. 바닥에 놓인 사물이다.
 * @warning 전반적으로 인자가 한 번 넘게 평가되며, 이들 중 어느 것도 입력을 검증하지 않는다. 어떤 숫자든 무언가로, 또는 아무것도 아닌 것으로 분류된다.
 * @{
 */
/*
 * Return true if the given glyph is what we want.  Note that bodies are
 * considered objects.
 */
#define glyph_is_normal_male_monster(glyph) \
    ((glyph) >= GLYPH_MON_MALE_OFF                      \
     && (glyph) < (GLYPH_MON_MALE_OFF + NUMMONS))
#define glyph_is_normal_female_monster(glyph) \
    ((glyph) >= GLYPH_MON_FEM_OFF && (glyph) < (GLYPH_MON_FEM_OFF + NUMMONS))
#define glyph_is_normal_monster(glyph) \
    (glyph_is_normal_male_monster(glyph)                \
     || glyph_is_normal_female_monster(glyph))
#define glyph_is_female_pet(glyph) \
    ((glyph) >= GLYPH_PET_FEM_OFF && (glyph) < (GLYPH_PET_FEM_OFF + NUMMONS))
#define glyph_is_male_pet(glyph) \
    ((glyph) >= GLYPH_PET_MALE_OFF                      \
     && (glyph) < (GLYPH_PET_MALE_OFF + NUMMONS))
#define glyph_is_pet(glyph) \
    (glyph_is_male_pet(glyph) || glyph_is_female_pet(glyph))
#define glyph_is_ridden_female_monster(glyph) \
    ((glyph) >= GLYPH_RIDDEN_FEM_OFF                    \
     && (glyph) < (GLYPH_RIDDEN_FEM_OFF + NUMMONS))
#define glyph_is_ridden_male_monster(glyph) \
    ((glyph) >= GLYPH_RIDDEN_MALE_OFF                   \
     && (glyph) < (GLYPH_RIDDEN_MALE_OFF + NUMMONS))
#define glyph_is_ridden_monster(glyph) \
    (glyph_is_ridden_male_monster(glyph)                \
     || glyph_is_ridden_female_monster(glyph))
#define glyph_is_detected_female_monster(glyph) \
    ((glyph) >= GLYPH_DETECT_FEM_OFF                    \
     && (glyph) < (GLYPH_DETECT_FEM_OFF + NUMMONS))
#define glyph_is_detected_male_monster(glyph) \
    ((glyph) >= GLYPH_DETECT_MALE_OFF                   \
     && (glyph) < (GLYPH_DETECT_MALE_OFF + NUMMONS))
#define glyph_is_detected_monster(glyph) \
    (glyph_is_detected_male_monster(glyph)              \
        || glyph_is_detected_female_monster(glyph))
#define glyph_is_monster(glyph) \
    (glyph_is_normal_monster(glyph) || glyph_is_pet(glyph)              \
     || glyph_is_ridden_monster(glyph) || glyph_is_detected_monster(glyph))
#define glyph_is_invisible(glyph) ((glyph) == GLYPH_INVISIBLE)

/* final NUMMONS is legal array index because of trailing fencepost entry */
#define glyph_to_mon(glyph) \
       (glyph_is_normal_female_monster(glyph)                  \
         ? ((glyph) - GLYPH_MON_FEM_OFF)                       \
         : glyph_is_normal_male_monster(glyph)                 \
           ? ((glyph) - GLYPH_MON_MALE_OFF)                    \
           : glyph_is_female_pet(glyph)                        \
             ? ((glyph) - GLYPH_PET_FEM_OFF)                   \
             : glyph_is_male_pet(glyph)                        \
               ? ((glyph) - GLYPH_PET_MALE_OFF)                \
               : glyph_is_detected_female_monster(glyph)       \
                 ? ((glyph) - GLYPH_DETECT_FEM_OFF)            \
                 : glyph_is_detected_male_monster(glyph)       \
                   ? ((glyph) - GLYPH_DETECT_MALE_OFF)         \
                   : glyph_is_ridden_female_monster(glyph)     \
                     ? ((glyph) - GLYPH_RIDDEN_FEM_OFF)        \
                     : glyph_is_ridden_male_monster(glyph)     \
                       ? ((glyph) - GLYPH_RIDDEN_MALE_OFF)     \
                       : NUMMONS)

/* boulder hides pile except when on top of another boulder;
   the otg_otmp assignment might occur multiple times in the same
   expression but there will always be sequence points in between */
#define obj_is_piletop(obj) \
    ((obj)->where == OBJ_FLOOR                                             \
     && ((go.otg_otmp = svl.level.objects[(obj)->ox][(obj)->oy]->nexthere) \
         != 0)                                                             \
     && ((obj)->otyp != BOULDER || go.otg_otmp->otyp == BOULDER))
/* used to hide info such as potion and gem color when not seen yet;
   stones and rock are excluded for gem class; LAST_SPELL includes blank
   spellbook but excludes novel and the Book of the Dead */
#define obj_is_generic(obj) \
    (!(obj)->dknown                                             \
     && ((obj)->oclass == POTION_CLASS                          \
         || ((obj)->otyp >= FIRST_REAL_GEM                      \
             && ((obj)->otyp <= LAST_GLASS_GEM))                \
         || ((obj)->otyp >= FIRST_SPELL                         \
             && ((obj)->otyp <= LAST_SPELL))))

#define glyph_is_body_piletop(glyph) \
    (((glyph) >= GLYPH_BODY_PILETOP_OFF)                        \
     && ((glyph) < (GLYPH_BODY_PILETOP_OFF + NUMMONS)))
#define glyph_is_body(glyph) \
    ((((glyph) >= GLYPH_BODY_OFF) && ((glyph) < (GLYPH_BODY_OFF + NUMMONS))) \
     || glyph_is_body_piletop(glyph))

#define glyph_is_fem_statue_piletop(glyph) \
    (((glyph) >= GLYPH_STATUE_FEM_PILETOP_OFF)                  \
      && ((glyph) < (GLYPH_STATUE_FEM_PILETOP_OFF + NUMMONS)))
#define glyph_is_male_statue_piletop(glyph) \
    (((glyph) >= GLYPH_STATUE_MALE_PILETOP_OFF)                 \
         && ((glyph) < (GLYPH_STATUE_MALE_PILETOP_OFF + NUMMONS)))
#define glyph_is_fem_statue(glyph) \
    ((((glyph) >= GLYPH_STATUE_FEM_OFF)                         \
      && ((glyph) < (GLYPH_STATUE_FEM_OFF + NUMMONS)))          \
     || glyph_is_fem_statue_piletop(glyph))
#define glyph_is_male_statue(glyph) \
    ((((glyph) >= GLYPH_STATUE_MALE_OFF)                        \
      && ((glyph) < (GLYPH_STATUE_MALE_OFF + NUMMONS)))         \
     || glyph_is_male_statue_piletop(glyph))
#define glyph_is_statue(glyph) \
    (glyph_is_male_statue(glyph) || glyph_is_fem_statue(glyph))
/**
 * @name Objects known only by class
 * @brief Glyphs for "some weapon" rather than a particular one.
 *
 * There is one of these per object class, sitting in the object range between the strange-object entry and the real objects, as the existing
 * comment records. They exist because the hero may know what class a thing is without knowing what it is -- an object seen from a distance,
 * or remembered from a magic mapping.
 *
 * @note Their position in the range is what makes them findable, so the boundary values in these tests come from where the real objects begin
 *       rather than from a count.
 * @{
 */
/**
 * @name 계열만 알려진 물건
 * @brief 특정한 것이 아니라 "어떤 무기"에 대한 글리프.
 *
 * 물건 계열마다 하나씩 있으며, 기존 주석이 기록하듯 물건 구간에서 이상한 물건 항목과 실제 물건들 사이에 놓인다. 영웅이 어떤 것이 무엇인지 모른 채로 그것이 어느 계열인지 알 수 있기 때문에 존재한다. 멀리서 본 물건, 또는
 * 마법 지도로 기억한 물건.
 *
 * @note 구간 안의 그 위치가 이것들을 찾을 수 있게 하는 것이다. 그래서 이 검사들의 경계값은 개수가 아니라 실제 물건이 시작하는 곳에서 나온다.
 * @{
 */
/* generic objects are after strange object (GLYPH_OBJ_OFF) and before
   other objects (GLYPH_OBJ_OFF + FIRST_OBJECT) */
#define glyph_is_normal_generic_obj(glyph) \
    ((glyph) > GLYPH_OBJ_OFF && (glyph) < GLYPH_OBJ_OFF + FIRST_OBJECT - 1)
#define glyph_is_piletop_generic_obj(glyph) \
    ((glyph) > GLYPH_OBJ_PILETOP_OFF                            \
     && (glyph) < GLYPH_OBJ_PILETOP_OFF + FIRST_OBJECT - 1)
#define glyph_is_generic_object(glyph) \
    (glyph_is_normal_generic_obj(glyph)                         \
     || glyph_is_piletop_generic_obj(glyph))
/** @} */

/**
 * @name Objects on top of a pile
 * @brief The duplicate ranges that say an object has others beneath it.
 *
 * Every object range is doubled: one for an object alone and one for the same object with a pile under it. The doubling exists because the
 * display may want to show that there is more here, and a glyph is the only thing it is told.
 *
 * @note This is why several of the tests above have a piletop counterpart, and why the broad tests are unions of the two.
 * @{
 */
/**
 * @name 무더기 맨 위의 물건
 * @brief 어떤 물건 아래에 다른 것들이 있음을 말하는 중복 구간들.
 *
 * 모든 물건 구간이 두 배로 되어 있다. 물건 하나만인 경우와 그 아래에 무더기가 있는 같은 물건인 경우. 그 이중화가 존재하는 것은 표시부가 여기 더 있다는 것을 보이고 싶어 할 수 있고, 그것이 듣는 것은 글리프뿐이기 때문이다.
 *
 * @note 위의 여러 검사에 무더기 맨 위 짝이 있는 이유이고, 넓은 검사들이 그 둘의 합집합인 이유다.
 * @{
 */
#define glyph_is_normal_piletop_obj(glyph) \
    ((glyph) == GLYPH_OBJ_PILETOP_OFF                           \
     || ((glyph) >= GLYPH_OBJ_PILETOP_OFF + FIRST_OBJECT - 1    \
         && (glyph) < (GLYPH_OBJ_PILETOP_OFF + NUM_OBJECTS)))
#define glyph_is_normal_object(glyph) \
    ((glyph) == GLYPH_OBJ_OFF                                   \
     || ((glyph) >= GLYPH_OBJ_OFF + FIRST_OBJECT - 1            \
         && (glyph) < (GLYPH_OBJ_OFF + NUM_OBJECTS))            \
     || glyph_is_normal_piletop_obj(glyph))
/** @} */

/**
 * @note The disabled version below enumerated every object-like range by hand. It is marked out of date and kept only so the earlier reasoning
 *       can be compared with the composed version that replaced it.
 */
/**
 * @note 아래의 비활성화된 판본은 물건 같은 모든 구간을 손으로 열거했다. 낡았다고 표시되어 있고, 그것을 대체한 조합 판본과 이전의 논리를 비교할 수 있도록만 남겨져 있다.
 */
#if 0   /* [note: out of date] */
#define glyph_is_object(glyph) \
  (   (((glyph) >= GLYPH_OBJ_OFF)                                       \
       && ((glyph) < (GLYPH_OBJ_OFF + NUM_OBJECTS)))                    \
   || (((glyph) >= GLYPH_OBJ_PILETOP_OFF)                               \
       && ((glyph) < (GLYPH_OBJ_PILETOP_OFF + NUM_OBJECTS)))            \
   || (((glyph) >= GLYPH_STATUE_MALE_OFF)                               \
       && ((glyph) < (GLYPH_STATUE_MALE_OFF + NUMMONS)))                \
   || (((glyph) >= GLYPH_STATUE_MALE_PILETOP_OFF)                       \
       && ((glyph) < (GLYPH_STATUE_MALE_PILETOP_OFF + NUMMONS)))        \
   || (((glyph) >= GLYPH_STATUE_FEM_OFF)                                \
       && ((glyph) < (GLYPH_STATUE_FEM_OFF + NUMMONS)))                 \
   || (((glyph) >= GLYPH_STATUE_FEM_PILETOP_OFF)                        \
       && ((glyph) < (GLYPH_STATUE_FEM_PILETOP_OFF + NUMMONS)))         \
   || (((glyph) >= GLYPH_BODY_OFF)                                      \
       && ((glyph) < (GLYPH_BODY_OFF + NUMMONS)))                       \
   || (((glyph) >= GLYPH_BODY_PILETOP_OFF)                              \
       && ((glyph) < (GLYPH_BODY_PILETOP_OFF + NUMMONS)))               \
  )
#endif
/**
 * @def glyph_is_object
 * @brief Whether this glyph is anything that lies on the floor.
 * @note Composed from the narrower tests rather than enumerating ranges, which is what keeps it correct as ranges are added -- the disabled
 *       version above is what happens otherwise.
 * @note Statues and bodies count. They are objects that happen to be shaped like monsters.
 */
/**
 * @def glyph_is_object
 * @brief 이 글리프가 바닥에 놓이는 것 중 하나인지.
 * @note 구간을 열거하는 대신 더 좁은 검사들로 조합된다. 그것이 구간이 더해져도 이것이 올바르게 유지되게 하는 것이다. 위의 비활성화된 판본이 그러지 않으면 어떻게 되는지를 보여 준다.
 * @note 조각상과 시체가 셈해진다. 그것들은 마침 몬스터 모양인 물건이다.
 */
#define glyph_is_object(glyph) \
    (glyph_is_normal_object(glyph) || glyph_is_generic_object(glyph)    \
     || glyph_is_statue(glyph) || glyph_is_body(glyph))

/**
 * @def obj_to_true_glyph
 * @brief An object's glyph as it really is, ignoring hallucination.
 * @warning Unused and marked out of date by the existing comment. It was written for an inventory display that showed map tiles for equipped
 *          objects; those now vary under hallucination like anything else, and the definition predates the class-only object glyphs. It is
 *          kept as a record rather than as something to call.
 */
/**
 * @def obj_to_true_glyph
 * @brief 물건의 실제 글리프. 환각을 무시하고.
 * @warning 쓰이지 않으며 기존 주석에 낡았다고 표시되어 있다. 착용한 물건에 대해 지도 타일을 보이는 소지품 표시부를 위해 작성되었는데, 그것들은 이제 다른 모든 것처럼 환각에 따라 달라지고, 이 정의는 계열만 아는 물건 글리프보다
 *          앞선다. 호출할 것이 아니라 기록으로 남겨져 있다.
 */
/* briefly used for Qt's "paper doll" inventory which shows map tiles for
   equipped objects; those vary like floor items during hallucination now
   so this isn't used anywhere [note: out of date since generic objs added] */
#define obj_to_true_glyph(obj)                                  \
    (((obj)->otyp == STATUE)                                    \
     ? ((int) (obj)->corpsenm                                   \
        + (((obj)->spe & CORPSTAT_GENDER) == CORPSTAT_FEMALE)   \
             ? (obj_is_piletop(obj)                             \
                ? (GLYPH_STATUE_FEM_PILETOP_OFF)                \
                : (GLYPH_STATUE_FEM_OFF))                       \
             : (obj_is_piletop(obj)                             \
                ? (GLYPH_STATUE_MALE_PILETOP_OFF)               \
                : (GLYPH_STATUE_MALE_OFF))                      \
     : (((obj)->otyp == CORPSE)                                 \
         ? ((int) (obj)->corpsenm                               \
            + (obj_is_piletop(obj)                              \
                ? (GLYPH_BODY_PILETOP_OFF)                      \
                ? (GLYPH_BODY_OFF)))                            \
         : ((int) (obj)->otyp + GLYPH_OBJ_OFF))))

/**
 * @def glyph_to_obj
 * @brief The kind of object a glyph stands for.
 * @note Bodies and statues collapse to the generic corpse and statue kinds, since their range recorded which monster and the object kind does
 *       not.
 * @note The failure value is the object count rather than the no-glyph value, and that is legal to use as an index: the existing comment
 *       records that the object table has a trailing entry for exactly this. So an unrecognised glyph yields a harmless lookup rather than a
 *       crash.
 */
/**
 * @def glyph_to_obj
 * @brief 글리프가 나타내는 물건의 종류.
 * @note 시체와 조각상은 일반 시체와 일반 조각상 종류로 무너진다. 어느 몬스터인지를 기록한 것은 그 구간이고 물건 종류는 그렇지 않기 때문이다.
 * @note 실패 값이 글리프 없음 값이 아니라 물건 개수이며, 그것을 색인으로 쓰는 것은 적법하다. 기존 주석은 물건 표에 바로 이것을 위한 끝 항목이 있다고 기록한다. 그래서 알 수 없는 글리프는 충돌이 아니라 무해한 조회를 낸다.
 */
/* final NUM_OBJECTS is legal array idx because of trailing fencepost entry */
#define glyph_to_obj(glyph) \
    (glyph_is_body(glyph) ? CORPSE                              \
     : glyph_is_statue(glyph) ? STATUE                          \
       : glyph_is_generic_object(glyph)                         \
         ? ((glyph) - (glyph_is_piletop_generic_obj(glyph)      \
                       ? GLYPH_OBJ_PILETOP_OFF                  \
                       : GLYPH_OBJ_OFF))                        \
         : glyph_is_normal_object(glyph)                        \
           ? ((glyph) - (glyph_is_normal_piletop_obj(glyph)     \
                         ? GLYPH_OBJ_PILETOP_OFF                \
                         : GLYPH_OBJ_OFF))                      \
           : NUM_OBJECTS)

#define glyph_to_body_corpsenm(glyph) \
    (glyph_is_body_piletop(glyph)                       \
       ? ((glyph) - GLYPH_BODY_PILETOP_OFF)             \
       : ((glyph) - GLYPH_BODY_OFF))

#define glyph_to_statue_corpsenm(glyph) \
    (glyph_is_fem_statue_piletop(glyph)                 \
       ? ((glyph) - GLYPH_STATUE_FEM_PILETOP_OFF)       \
       : glyph_is_male_statue_piletop(glyph)            \
           ? ((glyph) - GLYPH_STATUE_MALE_PILETOP_OFF)  \
           : glyph_is_fem_statue(glyph)                 \
               ? ((glyph) - GLYPH_STATUE_FEM_OFF)       \
               : glyph_is_male_statue(glyph)            \
                   ? ((glyph) - GLYPH_STATUE_MALE_OFF)  \
                   : NO_GLYPH)

/* This has the unfortunate side effect of needing a global variable
   to store a result. 'otg_temp' is defined and declared in decl.{ch}.  */
#define random_obj_to_glyph(rng) \
    (((go.otg_temp = random_object(rng)) == CORPSE)     \
         ? (random_monster(rng) + GLYPH_BODY_OFF)       \
         : (go.otg_temp + GLYPH_OBJ_OFF))
#define corpse_to_glyph(obj) \
    ((int) ((obj)->corpsenm                             \
          + (obj_is_piletop(obj) ? GLYPH_BODY_PILETOP_OFF : GLYPH_BODY_OFF)))
#define generic_obj_to_glyph(obj) \
    ((int) ((obj)->oclass                               \
            + (obj_is_piletop(obj) ? GLYPH_OBJ_PILETOP_OFF : GLYPH_OBJ_OFF)))
#define normal_obj_to_glyph(obj) \
    ((int) ((obj)->otyp                                 \
            + (obj_is_piletop(obj) ? GLYPH_OBJ_PILETOP_OFF : GLYPH_OBJ_OFF)))

/* MRKR: Statues now have glyphs corresponding to the monster they    */
/*       represent and look like monsters when you are hallucinating. */

#define statue_to_glyph(obj, rng) \
    ((Hallucination)                                                \
     ? ((random_monster(rng))                                       \
        + ((!(rng)(2)) ? GLYPH_MON_MALE_OFF : GLYPH_MON_FEM_OFF))   \
     : ((int) (obj)->corpsenm                                       \
        + ((((obj)->spe & CORPSTAT_GENDER) == CORPSTAT_FEMALE)      \
           ? (obj_is_piletop(obj)                                   \
              ? GLYPH_STATUE_FEM_PILETOP_OFF                        \
              : GLYPH_STATUE_FEM_OFF)                               \
           : (obj_is_piletop(obj)                                   \
              ? GLYPH_STATUE_MALE_PILETOP_OFF                       \
              : GLYPH_STATUE_MALE_OFF))))

#define obj_to_glyph(obj, rng) \
    (((obj)->otyp == STATUE) ? statue_to_glyph(obj, rng)            \
     : (Hallucination) ? random_obj_to_glyph(rng)                   \
       : ((obj)->otyp == CORPSE) ? corpse_to_glyph(obj)             \
         : obj_is_generic(obj) ? generic_obj_to_glyph(obj)          \
           : normal_obj_to_glyph(obj))

#define GLYPH_TRAP_OFF \
    (GLYPH_CMAP_B_OFF + (S_arrow_trap - S_grave))
#define glyph_is_trap(glyph) \
    ((glyph) >= (GLYPH_TRAP_OFF)                        \
     && (glyph) < ((GLYPH_TRAP_OFF) + MAXTCHARS))
#define glyph_is_warning(glyph) \
    ((glyph) >= GLYPH_WARNING_OFF                       \
     && (glyph) < (GLYPH_WARNING_OFF + WARNCOUNT))
#define glyph_is_unexplored(glyph) ((glyph) == GLYPH_UNEXPLORED)
#define glyph_is_nothing(glyph) ((glyph) == GLYPH_NOTHING)

#if 0
#define glyph_is_piletop(glyph) \
    (glyph_is_body_piletop(glyph)           \
     || glyph_is_fem_statue_piletop(glyph)  \
     || glyph_is_male_statue_piletop(glyph) \
     || glyph_is_piletop_generic_obj(glyph) \
     || glyph_is_normal_piletop_obj(glyph))
#endif

/**
 * @name Glyph resolution options
 * @brief How to behave while working out a glyph's appearance.
 * @note The override the second one disregards is the accessibility substitution -- so this exists for code that needs the real appearance
 *       rather than the one the player has asked to see instead.
 * @{
 */
/**
 * @name 글리프 해석 선택지
 * @brief 글리프의 외형을 계산하는 동안 어떻게 동작할지.
 * @note 두 번째 것이 무시하는 덮어쓰기는 접근성 대체다. 그래서 이것은 플레이어가 대신 보기로 요청한 외형이 아니라 실제 외형을 필요로 하는 코드를 위해 존재한다.
 * @{
 */
/* mgflags for altering map_glyphinfo() internal behavior */
#define MG_FLAG_NORMAL     0x00
#define MG_FLAG_NOOVERRIDE 0x01 /* disregard accessibility override values */
/** @} */

/**
 * @name What a resolved glyph turned out to be
 * @brief Facts about a glyph, worked out once so a display does not have to classify it again.
 *
 * The range tests above answer these questions, but a display would have to ask several of them per square. So they are answered during
 * resolution and handed over as flags -- which is what lets a display highlight pets or mark object piles without knowing how glyphs are
 * numbered.
 *
 * @note The pair of gender flags exists because a display may want to draw a monster and a statue of one differently by sex, and neither can be
 *       recovered from the appearance.
 * @warning Three of these deliberately share one value: lava, ice and sinks each need highlighting for the same reason -- they cannot be told
 *          apart from something similar by colour alone -- and one flag serves all three. The existing comment records that this may one day
 *          need separating.
 * @{
 */
/**
 * @name 해석된 글리프가 무엇으로 드러났는지
 * @brief 글리프에 관한 사실들. 표시부가 다시 분류하지 않아도 되도록 한 번 계산된다.
 *
 * 위의 범위 검사들이 이 질문들에 답하지만, 표시부는 칸마다 그 중 여럿을 물어야 할 것이다. 그래서 해석 중에 답이 나오고 플래그로 넘겨진다. 그것이 표시부가 글리프 번호 체계를 모른 채로 애완동물을 강조하거나 물건 무더기를
 * 표시할 수 있게 하는 것이다.
 *
 * @note 성별 플래그 짝이 있는 것은, 표시부가 몬스터와 그 조각상을 성별에 따라 다르게 그리고 싶어 할 수 있고 어느 쪽도 외형에서 되찾을 수 없기 때문이다.
 * @warning 이들 중 셋이 의도적으로 한 값을 공유한다. 용암, 얼음, 싱크대가 각각 같은 이유로 강조를 필요로 한다. 색만으로는 비슷한 것과 구별되지 않는다. 그리고 하나의 플래그가 그 셋을 함께 맡는다. 기존 주석은 이것이
 *          언젠가 나뉘어야 할 수 있다고 기록한다.
 * @{
 */
/* Special mapped glyphflags encoded by reset_glyphmap() + map_glyphinfo() */
#define MG_HERO    0x00001  /* represents the hero */
#define MG_CORPSE  0x00002  /* represents a body */
#define MG_INVIS   0x00004  /* represents invisible monster */
#define MG_DETECT  0x00008  /* represents a detected monster */
#define MG_PET     0x00010  /* represents a pet */
#define MG_RIDDEN  0x00020  /* represents a ridden monster */
#define MG_STATUE  0x00040  /* represents a statue */
#define MG_OBJPILE 0x00080  /* more than one stack of objects */
#define MG_BW_LAVA 0x00100  /* 'black & white lava': highlight lava if it
                             * can't be distinguished from water by color */
#define MG_BW_ICE  0x00200  /* similar for ice vs floor */
#define MG_BW_SINK 0x00200  /* identical for sink vs fountain [note: someday
                             * this may become a distinct flag */
#define MG_BW_ENGR 0x00200  /* likewise for corridor engravings */
#define MG_NOTHING 0x00400  /* char represents GLYPH_NOTHING */
#define MG_UNEXPL  0x00800  /* char represents GLYPH_UNEXPLORED */
#define MG_MALE    0x01000  /* represents a male mon or statue of one */
#define MG_FEMALE  0x02000  /* represents a female mon or statue of one */
#define MG_BADXY   0x04000  /* bad coordinates were passed */
/** @} */

/**
 * @brief How thorough a redraw to perform.
 *
 * Redrawing has two quite different meanings and the distinction matters for speed: working out afresh what the map should show, or simply
 * painting again what the game already believes it shows. The first is needed after the world changed; the second after the screen was
 * disturbed.
 *
 * @note The remaining values are modifiers combined with one of those two -- draw only the map, or do not clear the screen first.
 * @warning So this is not a plain choice. A value here may be a base or a base with a modifier, and comparing it whole will miss the combined
 *          cases.
 */
/**
 * @brief 얼마나 철저하게 다시 그릴지.
 *
 * 다시 그리기에는 꽤 다른 두 뜻이 있고 그 구별이 속도에 중요하다. 지도가 무엇을 보여야 하는지 새로 계산하는 것, 또는 게임이 이미 보여지고 있다고 여기는 것을 그저 다시 칠하는 것. 앞의 것은 세계가 바뀐 뒤에 필요하고, 뒤의
 * 것은 화면이 흐트러진 뒤에 필요하다.
 *
 * @note 나머지 값들은 그 둘 중 하나와 결합되는 수정자다. 지도만 그리기, 또는 먼저 화면을 지우지 않기.
 * @warning 그래서 이것은 단순한 택일이 아니다. 여기의 값은 기준일 수도, 수정자가 붙은 기준일 수도 있다. 값 전체를 비교하면 결합된 경우를 놓친다.
 */
/* docrt(): re-draw whole screen; docrt_flags(): docrt() with more control */
enum docrt_flags_bits {
    docrtRecalc  = 0, /* full docrt(), recalculate what the map should show */
    docrtRefresh = 1, /* redraw_map(), draw what we think the map shows */
    docrtMapOnly = 2, /* ORed with Recalc or Refresh; draw the map but not
                       * status or perminv */
    docrtNocls = 4,
};

/**
 * @brief One square of the game's belief about what the display is showing.
 *
 * The game keeps its own copy of the screen so that redrawing can send only what changed. That is what the flag is for: it marks a square whose
 * appearance has been altered since the display last saw it.
 *
 * @note The existing comment wonders whether that flag would be better kept with the map square itself. It is here because it belongs to the
 *       display's state rather than the world's.
 */
/**
 * @brief 표시부가 무엇을 보이고 있는지에 대한 게임의 믿음, 한 칸 분량.
 *
 * 게임은 다시 그릴 때 바뀐 것만 보낼 수 있도록 화면의 사본을 스스로 보관한다. 그 플래그가 그것을 위한 것이다. 표시부가 마지막으로 본 뒤로 외형이 바뀐 칸을 표시한다.
 *
 * @note 기존 주석은 그 플래그가 지도 칸 자체와 함께 보관되는 것이 낫지 않은지 묻고 있다. 그것이 세계의 상태가 아니라 표시부의 상태에 속하기 때문에 여기에 있다.
 */
typedef struct {
    xint8 gnew; /* perhaps move this bit into the rm structure. */
    glyph_info glyphinfo;
} gbuf_entry;

/**
 * @name Colour tables
 * @brief The colour for each altar, beam, explosion and wall kind, as arrays.
 * @note The enumerations above give these values names; the arrays let them be indexed by kind. Both forms exist because some code knows which
 *       kind it means and some is iterating.
 * @warning The wall table is the one that is not constant -- wall colours can be customised, and the rest cannot.
 * @{
 */
/**
 * @name 색 표
 * @brief 각 제단, 광선, 폭발, 벽 종류의 색. 배열로.
 * @note 위의 열거형들이 이 값들에 이름을 붙이고, 배열은 그것들을 종류로 색인할 수 있게 한다. 어떤 코드는 자신이 뜻하는 종류를 알고 어떤 코드는 순회하기 때문에 두 형태가 다 존재한다.
 * @warning 벽 표가 상수가 아닌 것이다. 벽 색은 사용자 지정될 수 있고 나머지는 그렇지 않다.
 * @{
 */
extern const int altarcolors[];
extern const int zapcolors[];
extern const int explodecolors[];
extern int wallcolors[];
/** @} */

/**
 * @var nul_glyphinfo
 * @brief A resolved glyph meaning "nothing here", to copy from.
 * @note Exists so that clearing a square is a copy rather than knowing which fields to zero -- and the structure gains fields over time.
 * @warning Defined in two different places depending on the build, as the existing comment records: in the generated tile file when tiles are
 *          built in, so that it can name a tile, and in the display code otherwise.
 */
/**
 * @var nul_glyphinfo
 * @brief "여기 아무것도 없음"을 뜻하는 해석된 글리프. 복사해 오기 위한 것.
 * @note 칸을 비우는 일이 어느 필드를 0으로 만들지 아는 것이 아니라 복사가 되도록 존재한다. 그리고 이 구조체는 시간이 흐르며 필드가 늘어난다.
 * @warning 기존 주석이 기록하듯 빌드에 따라 서로 다른 두 곳에서 정의된다. 타일이 빌드에 포함될 때는 타일을 지칭할 수 있도록 생성된 타일 파일에서, 그렇지 않으면 표시 코드에서.
 */
/* If TILES_IN_GLYPHMAP is defined during build, this is defined
 * in the generated tile.c, complete with appropriate tile references in
 * the initialization.  Otherwise, it gets defined in display.c.
 */
extern const glyph_info nul_glyphinfo;

#endif /* DISPLAY_H */
