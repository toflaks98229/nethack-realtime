/* NetHack 5.0	hack.h	$NHDT-Date: 1781973080 2026/06/20 16:31:20 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.299 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file hack.h
 * @brief The one header the whole game includes.
 *
 * Nearly every source file in NetHack includes this and nothing else. That is unusual, and it is deliberate rather than accidental: it means no file
 * has to work out which headers it needs, and it means the whole game is compiled against exactly the same set of declarations.
 *
 * The file has two halves. The first is a list of includes -- the subsystem headers, each defining one part of the game. Read in the order they appear:
 * the order is not alphabetical but dependency, and a header here may rely on one above it having already been read.
 *
 * The second half is what remains after those extractions: the definitions that belong to no single subsystem, or that are shared between several. They
 * are mostly the vocabulary of the interfaces between parts of the game -- the flags one routine passes another to say how a name should be formed, how
 * a movement should be attempted, what a search should include.
 *
 * @note This file is used as the precompiled header, which is why the include list is worth keeping deliberate. Every file pays for everything in it,
 *       and pays once rather than per file.
 * @note Many of the definitions here are sets of bit flags passed as one argument. Their comments give the precedence between them where it matters --
 *       a flag that overrides another is not visible from the values.
 * @note This is a modified copy of NetHack. The real-time fork's own header is included at the end of the list, where it can rely on everything above.
 */

/**
 * @file hack.h
 * @brief 게임 전체가 포함하는 단 하나의 헤더.
 *
 * NetHack 의 거의 모든 소스 파일이 이것을 포함하고 그 밖의 것은 포함하지 않는다. 그것은 흔치 않으며, 우연이 아니라 의도적이다. 어떤 파일도 자신이 어떤 헤더를 필요로 하는지 알아낼 필요가 없다는 뜻이고, 게임 전체가 정확히 같은
 * 선언 묶음에 맞춰 컴파일된다는 뜻이다.
 *
 * 이 파일에는 두 절반이 있다. 첫째는 포함 목록이다. 게임의 한 부분씩을 정의하는 하위 체계 헤더들. 나타나는 순서대로 읽을 것. 그 순서는 알파벳순이 아니라 의존 순서이며, 여기의 어떤 헤더는 그 위의 것이 이미 읽혔음에 의존할 수 있다.
 *
 * 둘째 절반은 그 추출 뒤에 남은 것이다. 어느 한 하위 체계에도 속하지 않거나 여럿이 공유하는 정의들. 대부분은 게임의 부분들 사이 인터페이스의 어휘다. 이름이 어떻게 만들어져야 하는지, 이동이 어떻게 시도되어야 하는지, 찾기가 무엇을
 * 포함해야 하는지를 말하기 위해 한 루틴이 다른 루틴에 넘기는 플래그들.
 *
 * @note 이 파일은 미리 컴파일된 헤더로 쓰인다. 그래서 포함 목록을 의도적으로 유지할 가치가 있다. 모든 파일이 그 안의 모든 것에 대한 비용을 치르며, 파일마다가 아니라 한 번만 치른다.
 * @note 여기 정의된 것 중 많은 것이 하나의 인자로 전달되는 비트 플래그 묶음이다. 중요한 곳에서는 그 주석이 서로의 우선순위를 밝힌다. 어떤 플래그가 다른 것을 무시한다는 것은 값에서 보이지 않는다.
 * @note 이것은 NetHack 의 수정된 사본이다. 실시간 포크의 자체 헤더가 목록 끝에 포함되며, 그곳에서 위의 모든 것에 의존할 수 있다.
 */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif
#include "lint.h"

#include "align.h"
#include "weight.h"
#include "dungeon.h"
#include "stairs.h"
#include "objclass.h"
#include "wintype.h"
#include "flag.h"
#include "rect.h"
#include "sym.h"
#include "trap.h"
#include "youprop.h"
#include "display.h"

#include "botl.h"
#include "context.h"
#include "engrave.h"
#include "mkroom.h"
#include "obj.h"
#include "quest.h"
#include "region.h"
#include "rm.h"
#include "selvar.h"
#include "sndprocs.h"
#include "spell.h"
#include "sys.h"
#include "timeout.h"
#include "winprocs.h"
#include "vision.h"
#include "you.h"
/* continuous positions used to animate grid-square motion (real-time fork) */
#include "nh_rtvector.h"

/**
 * @name Whether to say anything
 * @brief Passed to routines that may act silently or with a message.
 * @note Named rather than passed as a bare true or false, because at a call site "TELL" says what the argument means and a bare true does not.
 * @{
 */
/**
 * @name 무언가를 말할지 여부
 * @brief 조용히 또는 메시지와 함께 동작할 수 있는 루틴에 전달된다.
 * @note 맨 참이나 거짓으로 넘기는 대신 이름이 붙어 있다. 호출 지점에서 "TELL"은 그 인자가 무엇을 뜻하는지 말하고 맨 참은 그렇지 않기 때문이다.
 * @{
 */
#define TELL 1
#define NOTELL 0
/** @} */

/**
 * @name On or off
 * @brief For routines that switch something rather than testing it.
 * @{
 */
/**
 * @name 켜기 또는 끄기
 * @brief 무언가를 검사하는 것이 아니라 전환하는 루틴을 위한 것.
 * @{
 */
#define ON 1
#define OFF 0
/** @} */

/**
 * @def BOLT_LIM
 * @brief How far a ranged attack reaches.
 * @note One value for the whole game rather than per weapon or spell, so a player learns one distance and every ranged thing respects it.
 * @warning Frequently used squared, since distances are commonly compared without a square root. A comparison against this value unsquared is a
 *          different and much shorter range.
 */
/**
 * @def BOLT_LIM
 * @brief 원거리 공격이 얼마나 멀리 닿는지.
 * @note 무기나 주문마다가 아니라 게임 전체에 하나의 값이다. 그래서 플레이어는 하나의 거리를 익히고 모든 원거리 수단이 그것을 지킨다.
 * @warning 자주 제곱된 형태로 쓰인다. 거리가 흔히 제곱근 없이 비교되기 때문이다. 제곱하지 않은 이 값과의 비교는 다른, 훨씬 짧은 범위다.
 */
#define BOLT_LIM 8        /* from this distance ranged attacks will be made */
/**
 * @def DUMMY
 * @brief An initialiser for the first element of an array, leaving the rest to default.
 * @note Exists so a table can be written without listing every entry. The existing comment records the intent: give element zero and let the compiler
 *       zero the rest.
 */
/**
 * @def DUMMY
 * @brief 배열의 첫 원소를 위한 초기화자. 나머지는 기본값에 맡긴다.
 * @note 표를 모든 항목을 나열하지 않고 쓸 수 있도록 존재한다. 기존 주석이 그 의도를 기록한다. 0번 원소를 주고 나머지는 컴파일러가 0으로 채우게 한다.
 */
#define DUMMY { 0 }       /* array initializer, letting [1..N-1] default */
/**
 * @def DEF_NOTHING
 * @brief The character used for both an empty square and an unseen one, by default.
 * @note One character for two different meanings because both should look like nothing. The player can give them separate symbols, and the two glyphs are
 *       distinct even when they draw alike.
 */
/**
 * @def DEF_NOTHING
 * @brief 기본으로 빈 칸과 보지 못한 칸 둘 다에 쓰이는 문자.
 * @note 서로 다른 두 뜻에 하나의 문자인 것은, 둘 다 아무것도 아닌 것처럼 보여야 하기 때문이다. 플레이어는 그것들에 별개의 심볼을 줄 수 있고, 두 글리프는 같게 그려질 때에도 서로 다르다.
 */
#define DEF_NOTHING ' '   /* default symbol for NOTHING and UNEXPLORED  */

/**
 * @name How a rumor reached the hero
 * @brief Where a piece of dungeon lore came from, which decides how it is worded.
 * @note The same rumor is phrased differently depending on the source -- the Oracle speaks, a cookie is read -- so the source travels with the request
 *       rather than the caller composing the sentence.
 * @warning The values are not consecutive: the last is far from the others, so they cannot be iterated over.
 * @{
 */
/**
 * @name 소문이 영웅에게 어떻게 닿았는지
 * @brief 던전의 이야기 한 조각이 어디서 왔는지. 그것이 표현 방식을 정한다.
 * @note 같은 소문이 출처에 따라 다르게 표현된다. 오라클은 말하고 과자는 읽힌다. 그래서 호출자가 문장을 짓는 대신 출처가 요청과 함께 전달된다.
 * @warning 값들이 연속하지 않는다. 마지막 것이 나머지에서 멀리 떨어져 있으므로 순회할 수 없다.
 * @{
 */
/* Macros for how a rumor was delivered in outrumor() */
#define BY_ORACLE 0
#define BY_COOKIE 1
#define BY_PAPER 2
#define BY_OTHER 9
/** @} */

/**
 * @name Naming a corpse
 * @brief How to word a corpse's name, which is more variable than it sounds.
 *
 * "The corpse of a soldier ant", "soldier ant corpse", "soldier ant" -- the same object needs all of these depending on the sentence it goes into. So the
 * caller says what it needs and the naming routine composes accordingly.
 *
 * @warning There is a precedence between three of these that the values do not show, and the existing comment states it: the definite-article flag beats
 *          the indefinite-article flag, and the no-prefix flag beats both. Combining them is meaningful rather than an error, so a caller must know which
 *          wins.
 * @note The suppress-corpse flag exists because the word is sometimes carried by the surrounding sentence instead.
 * @{
 */
/**
 * @name 시체의 이름 짓기
 * @brief 시체의 이름을 어떻게 표현할지. 들리는 것보다 다양하다.
 *
 * "병정개미의 시체", "병정개미 시체", "병정개미". 같은 물건이 들어갈 문장에 따라 이 전부를 필요로 한다. 그래서 호출자가 자신이 필요한 것을 말하고 이름 짓기 루틴이 그에 맞춰 짓는다.
 *
 * @warning 이 중 셋 사이에 값으로는 보이지 않는 우선순위가 있고, 기존 주석이 그것을 밝힌다. 정관사 플래그가 부정관사 플래그를 이기고, 접두어 없음 플래그가 그 둘을 이긴다. 그것들을 결합하는 것은 오류가 아니라 의미가 있으므로, 호출자가
 *          무엇이 이기는지 알아야 한다.
 * @note 시체 낱말 억제 플래그가 있는 것은, 그 낱말이 때때로 둘러싼 문장이 대신 지니기 때문이다.
 * @{
 */
/* bitmask flags for corpse_xname();
   PFX_THE takes precedence over ARTICLE, NO_PFX takes precedence over both */
#define CXN_NORMAL 0    /* no special handling */
#define CXN_SINGULAR 1  /* override quantity if greater than 1 */
#define CXN_NO_PFX 2    /* suppress "the" from "the Unique Monst */
#define CXN_PFX_THE 4   /* prefix with "the " (unless pname) */
#define CXN_ARTICLE 8   /* include a/an/the prefix */
#define CXN_NOCORPSE 16 /* suppress " corpse" suffix */
#define CXN_ADDGNDR 32  /* include a gender */
/** @} */

/**
 * @def VAULT_GUARD_TIME
 * @brief How long the hero may stand in a vault before a guard arrives.
 * @note A delay rather than an immediate arrival, so a hero who enters and leaves promptly is not caught. That grace period is the whole of the vault
 *       puzzle.
 */
/**
 * @def VAULT_GUARD_TIME
 * @brief 경비가 오기까지 영웅이 금고에 서 있을 수 있는 시간.
 * @note 즉시 도착이 아니라 지연이므로, 들어갔다 곧 나오는 영웅은 들키지 않는다. 그 여유 시간이 금고 퍼즐의 전부다.
 */
/* number of turns it takes for vault guard to show up */
#define VAULT_GUARD_TIME 30

/* shopkeeper billing: selling states, damage prices, devaluation, repossession
   [gathered from four places in this file; see nh_shop.h] */
#include "nh_shop.h"

/**
 * @name Describing what is underfoot
 * @brief How to word the report of what is on the hero's square.
 * @note The two flags exist because the sentence changes with the situation. Having just picked something up means the rest should be phrased as what
 *       remains, and a square whose terrain has already been mentioned should not have it mentioned again.
 * @{
 */
/**
 * @name 발밑에 무엇이 있는지 알리기
 * @brief 영웅의 칸에 있는 것에 대한 보고를 어떻게 표현할지.
 * @note 두 플래그가 있는 것은 상황에 따라 문장이 달라지기 때문이다. 방금 무언가를 집었다는 것은 나머지를 남은 것으로 표현해야 한다는 뜻이고, 지형이 이미 언급된 칸은 그것이 다시 언급되어서는 안 된다.
 * @{
 */
/* flags for look_here() */
#define LOOKHERE_NOFLAGS       0U
#define LOOKHERE_PICKED_SOME   1U
#define LOOKHERE_SKIP_DFEATURE 2U
/** @} */

/**
 * @def WINTYPELEN
 * @brief The longest a display's name may be.
 */
/**
 * @def WINTYPELEN
 * @brief 표시부 이름이 가질 수 있는 최대 길이.
 */
/* max size of a windowtype option */
#define WINTYPELEN 16

/**
 * @brief Either a string or its length, in one field.
 *
 * Used while a level description is being read. The same field holds the text of a name at one stage and, once the text has been stored elsewhere, the
 * length it occupied. Two stages of one process share a field rather than the structure carrying both.
 *
 * @warning Which reading applies depends on how far the process has got. Nothing here records it.
 */
/**
 * @brief 문자열 또는 그 길이. 하나의 필드에.
 *
 * 레벨 기술이 읽히는 동안 쓰인다. 같은 필드가 어느 단계에서는 이름의 글을 담고, 그 글이 다른 곳에 저장된 뒤에는 그것이 차지했던 길이를 담는다. 구조체가 둘 다를 지니는 대신 한 과정의 두 단계가 하나의 필드를 공유한다.
 *
 * @warning 어느 해석이 적용되는지는 그 과정이 어디까지 왔는지에 달려 있다. 여기에는 그것을 기록하는 것이 없다.
 */
/* str_or_len from sp_lev.h */
typedef union str_or_len {
    char *str;
    int len;
} Str_or_Len;

/**
 * @brief The artifacts, generated from the artifact list.
 * @note Read from the same file that defines the artifacts themselves, so a new artifact yields both its entry and its name without either being written
 *       twice.
 */
/**
 * @brief 아티팩트들. 아티팩트 목록에서 생성된다.
 * @note 아티팩트 자체를 정의하는 것과 같은 파일에서 읽어 오므로, 새 아티팩트가 그 항목과 그 이름을 함께 만들어 내고 어느 쪽도 두 번 적히지 않는다.
 */
enum artifacts_nums {
#define ARTI_ENUM
#include "artilist.h"
#undef ARTI_ENUM
    AFTER_LAST_ARTIFACT
};

/**
 * @brief How many artifacts there are.
 * @note One fewer than the enumeration's end marker, because the numbering starts at one -- artifact zero means "not an artifact".
 */
/**
 * @brief 아티팩트가 몇 개인지.
 * @note 열거의 끝 표시보다 하나 적다. 번호가 1에서 시작하기 때문이다. 0번 아티팩트는 "아티팩트가 아님"을 뜻한다.
 */
enum misc_arti_nums {
    NROFARTIFACTS = (AFTER_LAST_ARTIFACT - 1)
};

/**
 * @brief The value that lifts a restriction rather than naming one.
 * @note Negative so it cannot be mistaken for a real restriction, since the same argument carries both.
 */
/**
 * @brief 제약을 지칭하는 대신 해제하는 값.
 * @note 같은 인자가 둘 다를 지니므로, 실제 제약으로 오인되지 않도록 음수다.
 */
/* related to breadcrumb struct */
enum bcargs {override_restriction = -1};

/**
 * @brief Where in the source something was set, kept so a later failure can say who did it.
 *
 * A record of the routine and line that established some condition. It exists for diagnosing state that turns out to be wrong: knowing what the state is
 * rarely explains it, and knowing where it was set usually does.
 *
 * @note @c in_effect distinguishes a record that is current from one left behind, which is what makes a stale trail detectable rather than misleading.
 */
/**
 * @brief 무언가가 소스의 어디에서 설정되었는지. 나중의 실패가 누가 그렇게 했는지 말할 수 있도록 보관된다.
 *
 * 어떤 조건을 세운 루틴과 줄의 기록. 잘못된 것으로 드러난 상태를 진단하기 위해 존재한다. 상태가 무엇인지 아는 것은 그것을 설명하는 일이 드물고, 그것이 어디서 설정되었는지 아는 것은 보통 설명한다.
 *
 * @note @c in_effect 는 현재인 기록과 남겨진 기록을 구별한다. 그것이 낡은 흔적을 오해를 낳는 것이 아니라 감지할 수 있는 것으로 만든다.
 */
struct breadcrumbs {
    const char *funcnm;
    int linenum;
    boolean in_effect;
};

/**
 * @brief What is travelling along a line, when something is.
 *
 * One routine walks a path square by square and applies whatever the travelling thing does, and this says what that thing is. The cases are distinct
 * because they differ in what stops them, what they do on arrival, and whether the hero gets it back.
 *
 * @note A tethered weapon is separate from an ordinary thrown one because it draws a line behind it and returns along it.
 * @note An invisible beam is included so that an effect with no appearance can still use the same path-walking, rather than needing its own.
 */
/**
 * @brief 무언가가 선을 따라 이동하고 있을 때, 그것이 무엇인지.
 *
 * 하나의 루틴이 경로를 칸 단위로 걸으며 이동하는 것이 하는 일을 적용하고, 이 값이 그것이 무엇인지 말한다. 각 경우가 구별되는 것은 무엇이 그것을 멈추는지, 도착해서 무엇을 하는지, 영웅이 그것을 되찾는지가 다르기 때문이다.
 *
 * @note 줄이 달린 무기가 평범한 던진 무기와 따로 있는 것은, 그것이 자기 뒤로 선을 그리고 그 선을 따라 돌아오기 때문이다.
 * @note 보이지 않는 광선이 포함되어 있어, 외형이 없는 효과도 자기 것을 따로 만들지 않고 같은 경로 걷기를 쓸 수 있다.
 */
/* types of calls to bhit() */
enum bhit_call_types {
    ZAPPED_WAND   = 0,
    THROWN_WEAPON = 1,
    THROWN_TETHERED_WEAPON = 2,
    KICKED_WEAPON = 3,
    FLASHED_LIGHT = 4,
    INVIS_BEAM    = 5
};

/**
 * @brief A part of the body, for composing messages about it.
 *
 * The hero may be polymorphed into something with no hands, no head, or no limbs at all, and a message that says "your hand" regardless would be wrong.
 * So a message names the part it means and the wording is worked out from the current form -- a snake's "hand" becomes something else, or the sentence
 * changes.
 *
 * @note Some entries are not parts at all but states a part-naming routine can be asked for, such as light-headedness. They are here because the same
 *       substitution handles them.
 * @note @c NO_PART is negative because it is an answer rather than a part: the current form has nothing that corresponds.
 */
/**
 * @brief 몸의 한 부위. 그것에 관한 메시지를 짓기 위한 것.
 *
 * 영웅은 손이 없거나 머리가 없거나 팔다리가 전혀 없는 것으로 변신할 수 있고, 상관없이 "당신의 손"이라고 말하는 메시지는 틀리다. 그래서 메시지는 자신이 뜻하는 부위를 지칭하고 표현은 현재 형태에서 계산된다. 뱀의 "손"은 다른 것이 되거나
 * 문장이 달라진다.
 *
 * @note 일부 항목은 부위가 전혀 아니라 부위 이름 짓기 루틴에 요청할 수 있는 상태다. 어지러움 같은 것. 같은 대체가 그것들을 함께 다루기 때문에 여기에 있다.
 * @note @c NO_PART 가 음수인 것은 그것이 부위가 아니라 답이기 때문이다. 현재 형태에 대응하는 것이 없다는 답.
 */
/* Macros for messages referring to hands, eyes, feet, etc... */
enum bodypart_types {
    NO_PART   = -1,
    ARM       =  0,
    EYE       =  1,
    FACE      =  2,
    FINGER    =  3,
    FINGERTIP =  4,
    FOOT      =  5,
    HAND      =  6,
    HANDED    =  7,
    HEAD      =  8,
    LEG       =  9,
    LIGHT_HEADED = 10,
    NECK      = 11,
    SPINE     = 12,
    TOE       = 13,
    HAIR      = 14,
    BLOOD     = 15,
    LUNG      = 16,
    NOSE      = 17,
    STOMACH   = 18
};

/* drifting bubbles and their contents on the water/air levels
   [struct container gathered here too; see nh_bubble.h] */
#include "nh_bubble.h"

/* command queue, special keys, and the command dispatch table */
#include "nh_cmd.h"

struct c_color_names {
    const char *const c_black, *const c_amber, *const c_golden,
        *const c_light_blue, *const c_red, *const c_green, *const c_silver,
        *const c_blue, *const c_purple, *const c_white, *const c_orange;
};

struct c_common_strings {
    const char *const c_nothing_happens, *const c_nothing_seems_to_happen,
        *const c_thats_enough_tries, *const c_silly_thing_to,
        *const c_shudder_for_moment, *const c_something, *const c_Something,
        *const c_You_can_move_again, *const c_Never_mind,
        *const c_vision_clears, *const c_the_your[2], *const c_fakename[2];
};

/* [struct container moved to nh_bubble.h] */

/* [cost_alteration_types and unpaid_cost_flags moved to nh_shop.h] */

/* read.c, create_particular() & create_particular_parse() */
struct _create_particular_data {
    int quan;
    int which;
    int fem;        /* -1, MALE, FEMALE, NEUTRAL */
    int genderconf;    /* conflicting gender */
    char monclass;
    boolean randmonst;
    boolean maketame, makepeaceful, makehostile;
    boolean sleeping, saddled, invisible, hidden;
};

/* dig_check() results */

enum digcheck_result {
    DIGCHECK_PASSED                 = 1,
    DIGCHECK_PASSED_DESTROY_TRAP    = 2,
    DIGCHECK_PASSED_PITONLY         = 3,
    DIGCHECK_FAILED                 = 4,
    DIGCHECK_FAIL_ONSTAIRS          = DIGCHECK_FAILED,
    DIGCHECK_FAIL_ONLADDER,
    DIGCHECK_FAIL_THRONE,
    DIGCHECK_FAIL_ALTAR,
    DIGCHECK_FAIL_AIRLEVEL,
    DIGCHECK_FAIL_WATERLEVEL,
    DIGCHECK_FAIL_TOOHARD,
    DIGCHECK_FAIL_UNDESTROYABLETRAP,
    DIGCHECK_FAIL_CANTDIG,
    DIGCHECK_FAIL_BOULDER,
    DIGCHECK_FAIL_OBJ_POOL_OR_TRAP
};


/* Dismount: causes for why you are no longer riding */
enum dismount_types {
    DISMOUNT_GENERIC  = 0,
    DISMOUNT_FELL     = 1,
    DISMOUNT_THROWN   = 2,
    DISMOUNT_KNOCKED  = 3, /* hero hit for knockback effect */
    DISMOUNT_POLY     = 4,
    DISMOUNT_ENGULFED = 5,
    DISMOUNT_BONES    = 6,
    DISMOUNT_BYCHOICE = 7
};

/* special-level locations and the short names used to reach them */
#include "nh_dgntopo.h"

enum lua_theme_group {
    all_themes = 1,  /* for end of game */
    most_themes = 2, /* for entering endgame */
    tut_themes = 3,  /* for leaving tutorial */
};

enum earlyarg {
    ARG_DEBUG, ARG_VERSION, ARG_SHOWPATHS
#ifndef NODUMPENUMS
    , ARG_DUMPENUMS
#endif
    , ARG_DUMPGLYPHIDS
    , ARG_DUMPMONGEN
    , ARG_DUMPWEIGHTS
#ifdef WIN32
    , ARG_WINDOWS
#endif
#if defined(CRASHREPORT)
    , ARG_BIDSHOW
#endif
};

struct early_opt {
    enum earlyarg e;
    const char *name;
    int minlength;
    boolean valallowed;
};

/* symbolic names for capacity levels */
enum encumbrance_types {
    UNENCUMBERED = 0,
    SLT_ENCUMBER = 1, /* Burdened */
    MOD_ENCUMBER = 2, /* Stressed */
    HVY_ENCUMBER = 3, /* Strained */
    EXT_ENCUMBER = 4, /* Overtaxed */
    OVERLOADED   = 5  /* Overloaded */
};

struct entity {
    struct monst *emon;     /* youmonst for the player */
    struct permonst *edata; /* must be non-zero for record to be valid */
    int ex, ey;
};

struct enum_dump {
    int val;
    const char *nm;
};

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.  Some parts of the
 * code assume that PANICKED separates the deaths from the non-deaths.
 */
enum game_end_types {
    DIED         =  0,
    CHOKING      =  1,
    POISONING    =  2,
    STARVING     =  3,
    DROWNING     =  4,
    BURNING      =  5,
    DISSOLVED    =  6,
    CRUSHING     =  7,
    STONING      =  8,
    TURNED_SLIME =  9,
    GENOCIDED    = 10,
    PANICKED     = 11,
    TRICKED      = 12,
    QUIT         = 13,
    ESCAPED      = 14,
    ASCENDED     = 15
};

/* game events log */
struct gamelog_line {
    long turn; /* turn when this happened */
    long flags; /* LL_foo flags */
    char *text;
    struct gamelog_line *next;
};


/* values returned from getobj() callback functions */
enum getobj_callback_returns {
    /* generally invalid - can't be used for this purpose. will give a "silly
     * thing" message if the player tries to pick it, unless a more specific
     * failure message is in getobj itself - e.g. "You cannot foo gold". */
    GETOBJ_EXCLUDE = -3,
    /* invalid because it is not in inventory; used when the hands/self
     * possibility is queried and the player passed up something on the
     * floor before getobj. */
    GETOBJ_EXCLUDE_NONINVENT = -2,
    /* invalid because it is an inaccessible or unwanted piece of gear, but
     * pseudo-valid for the purposes of allowing the player to select it and
     * getobj to return it if there is a prompt instead of getting "silly
     * thing", in order for the getobj caller to present a specific failure
     * message. Other than that, the only thing this does differently from
     * GETOBJ_EXCLUDE is that it inserts an "else" in "You don't have anything
     * else to foo". */
    GETOBJ_EXCLUDE_INACCESS = -1,
    /* invalid for purposes of not showing a prompt if nothing is valid but
     * pseudo-valid for selecting - identical to GETOBJ_EXCLUDE_INACCESS but
     * without the "else" in "You don't have anything else to foo". */
    GETOBJ_EXCLUDE_SELECTABLE = 0,
    /* valid - invlet not presented in the summary or the ? menu as a
     * recommendation, but is selectable if the player enters it anyway.
     * Used for objects that are actually valid but unimportantly so, such
     * as shirts for reading. */
    GETOBJ_DOWNPLAY = 1,
    /* valid - will be shown in summary and ? menu */
    GETOBJ_SUGGEST  = 2,
};

/* getpos() return values */
enum getpos_retval {
    LOOK_TRADITIONAL = 0, /* '.' -- ask about "more info?" */
    LOOK_QUICK       = 1, /* ',' -- skip "more info?" */
    LOOK_ONCE        = 2, /* ';' -- skip and stop looping */
    LOOK_VERBOSE     = 3  /* ':' -- show more info w/o asking */
};

struct h2o_ctx {
    int dkn_boom, unk_boom; /* track dknown, !dknown separately */
    boolean ctx_valid;
};

/* attack mode for hmon() */
enum hmon_atkmode_types {
    HMON_MELEE   = 0, /* hand-to-hand */
    HMON_THROWN  = 1, /* normal ranged (or spitting while poly'd) */
    HMON_KICKED  = 2, /* alternate ranged */
    HMON_APPLIED = 3, /* polearm, treated as ranged */
    HMON_DRAGGED = 4  /* attached iron ball, pulled into mon */
};

/**
 * @brief How hungry the hero is, worst last.
 * @note Ordered as a scale rather than a set, so a rule can ask whether the hero is at least as hungry as some point. That is what makes "weak from hunger
 *       or worse" one comparison.
 * @note The last two are past being merely hungry: one is unconsciousness and one is death. They are on the same scale because the progression is
 *       continuous.
 */
/**
 * @brief 영웅이 얼마나 배고픈지. 나쁜 것이 뒤로.
 * @note 묶음이 아니라 척도로 정렬되어 있어, 규칙이 영웅이 어떤 지점만큼은 배고픈지 물을 수 있다. 그것이 "굶주려 쇠약하거나 그보다 나쁨"을 한 번의 비교로 만드는 것이다.
 * @note 마지막 둘은 단지 배고픈 것을 넘어선다. 하나는 의식 상실이고 하나는 죽음이다. 그 진행이 이어지는 것이므로 같은 척도에 있다.
 */
/* hunger states - see hu_stat in eat.c */
enum hunger_state_types {
    SATIATED   = 0,
    NOT_HUNGRY = 1,
    HUNGRY     = 2,
    WEAK       = 3,
    FAINTING   = 4,
    FAINTED    = 5,
    STARVED    = 6
};

/**
 * @name Inventory letters that are not letters
 * @brief Stand-ins for slots that no letter can name.
 *
 * There are fifty-two letters and a hero can carry more kinds of thing than that. So the overflow slot shares one character, and two more characters name
 * things that are in the inventory list without being in the pack.
 *
 * @note Deliberately not letters, so they cannot collide with a real slot.
 * @note @c HANDS_SYM stands for whatever the current form uses instead of hands, so what it names depends on the hero's shape.
 * @{
 */
/**
 * @name 글자가 아닌 소지품 글자
 * @brief 어떤 글자도 지칭할 수 없는 칸을 위한 대역.
 *
 * 글자는 쉰두 개이고 영웅은 그보다 많은 종류를 지닐 수 있다. 그래서 넘침 칸이 하나의 문자를 공유하고, 또 두 문자가 가방에 있지 않으면서 소지품 목록에 있는 것을 지칭한다.
 *
 * @note 의도적으로 글자가 아니다. 그래서 실제 칸과 충돌할 수 없다.
 * @note @c HANDS_SYM 은 현재 형태가 손 대신 쓰는 것을 나타내므로, 그것이 무엇을 지칭하는지는 영웅의 모습에 달려 있다.
 * @{
 */
/* fake inventory letters, not 'a'..'z' or 'A'..'Z' */
#define NOINVSYM '#'      /* overflow because all 52 letters are in use */
#define CONTAINED_SYM '>' /* designator for inside a container */
#define HANDS_SYM '-'     /* hands|fingers|self depending on context */
/** @} */

/* inventory counts (slots in tty parlance)
 * a...zA..Z    invlet_basic (52)
 * $a...zA..Z#  2 special additions
 */
enum inventory_counts {
    invlet_basic = 52,
    invlet_gold = 1,
    invlet_overflow = 1,
    invlet_max = invlet_basic + invlet_gold + invlet_overflow,
    /* 2023/11/30 invlet_max is not yet used anywhere */
};

#ifndef IDLECHECKPOINT_WAIT_TIME
#define IDLECHECKPOINT_WAIT_TIME 10  /* seconds to wait before executing a checkpoint;
                                      * always #define'd but only has meaning if
                                      * IDLECHECKPOINT is defined.
                                      */
#endif

/**
 * @brief What will kill the hero later, recorded now.
 *
 * Several afflictions kill after a delay -- poison working through, stoning completing, strangulation. What kills the hero is the affliction and not
 * whatever is happening at the moment it finishes, so the killer has to be recorded when the affliction begins.
 *
 * They form a chain because more than one delayed death can be pending, and each is keyed by the property that caused it so that curing that property can
 * remove the right one.
 *
 * @note The format decides the wording rather than the text -- "killed by a soldier ant" and "killed by Ashikaga Takauji" differ in the article, and the
 *       third form suppresses the prefix entirely for causes that read as a phrase.
 */
/**
 * @brief 나중에 영웅을 죽일 것을, 지금 기록해 둔 것.
 *
 * 몇 가지 고통은 지연된 뒤 죽인다. 퍼져 나가는 독, 완성되는 석화, 목 조임. 영웅을 죽이는 것은 그 고통이며 그것이 끝나는 순간에 무슨 일이 벌어지고 있든 그것이 아니다. 그래서 살해자는 그 고통이 시작될 때 기록되어야 한다.
 *
 * 지연된 죽음이 둘 넘게 예정되어 있을 수 있으므로 사슬을 이루며, 각각이 그것을 일으킨 속성으로 색인된다. 그래서 그 속성을 치유하면 알맞은 것을 없앨 수 있다.
 *
 * @note 형식이 글이 아니라 표현을 정한다. "병정개미에게 죽었다"와 "아시카가 다카우지에게 죽었다"는 관사가 다르고, 세 번째 형태는 구절로 읽히는 원인을 위해 접두어를 아예 없앤다.
 */
struct kinfo {
    struct kinfo *next; /* chain of delayed killers */
    int id;             /* uprop keys to ID a delayed killer */
    int format;         /* one of the killer formats */
#define KILLED_BY_AN 0
#define KILLED_BY 1
#define NO_KILLER_PREFIX 2
    char name[BUFSZ]; /* actual killer name */
};

/**
 * @brief Where a launched object came from.
 * @note Kept because a rolling boulder or fired arrow must be able to be traced back -- what launched it may be gone by the time it lands.
 */
/**
 * @brief 발사된 물건이 어디서 왔는지.
 * @note 굴러오는 바위나 발사된 화살이 거슬러 추적될 수 있어야 하므로 보관된다. 그것을 발사한 것은 그것이 도착할 때쯤 사라졌을 수 있다.
 */
struct launchplace {
    struct obj *obj;
    coordxy x, y;
};

/**
 * @brief Something that lights the dungeon around it.
 *
 * A list rather than a property of squares, because a light source moves: a lamp is carried, a monster that glows walks. So what is lit is recomputed from
 * the sources rather than stored on the map.
 *
 * @note The owner is held as a generic identifier rather than a pointer, so the same list can hold lights belonging to objects and to monsters, and can be
 *       saved.
 */
/**
 * @brief 자기 둘레의 던전을 밝히는 것.
 *
 * 칸의 속성이 아니라 목록인 것은, 광원이 움직이기 때문이다. 등불은 들려 다니고, 빛나는 몬스터는 걸어 다닌다. 그래서 무엇이 밝혀지는지는 지도에 저장되는 대신 광원들에서 다시 계산된다.
 *
 * @note 소유자가 포인터가 아니라 범용 식별자로 보관된다. 그래서 하나의 목록이 물건에 속한 빛과 몬스터에 속한 빛을 함께 담을 수 있고, 저장될 수 있다.
 */
/* light source */
typedef struct ls_t {
    struct ls_t *next;
    coordxy x, y;  /* source's position */
    short range; /* source's current range */
    short flags;
    short type;  /* type of light source */
    anything id; /* source's identifier */
} light_source;

/**
 * @brief One of the player's rules for colouring menu lines.
 *
 * A rule is a pattern and an appearance: any line matching the pattern is shown that way. That is how a player makes cursed items stand out without the game
 * knowing which lines are about cursed items.
 *
 * @note The original text of the pattern is kept alongside the compiled form so the rule can be shown back to the player as they wrote it.
 * @note A list, and the order is the precedence -- so a later rule can be made to win by where it is placed.
 */
/**
 * @brief 메뉴 줄에 색을 입히는 플레이어의 규칙 하나.
 *
 * 규칙은 패턴과 외형이다. 그 패턴에 걸리는 줄은 그렇게 보여진다. 그것이 게임이 어느 줄이 저주받은 물건에 관한 것인지 모른 채로 플레이어가 저주받은 물건을 두드러지게 만드는 방식이다.
 *
 * @note 패턴의 원래 글이 컴파일된 형태와 함께 보관된다. 그래서 그 규칙을 플레이어가 쓴 그대로 되보여 줄 수 있다.
 * @note 목록이며, 그 순서가 우선순위다. 그래서 나중의 규칙을 놓는 위치로 이기게 만들 수 있다.
 */
struct menucoloring {
    struct nhregex *match;
    char *origstr;
    int color, attr;
    struct menucoloring *next;
};

/* directions, movement styles, and move-attempt outcomes
   [test_move and m_move values gathered here too; see nh_move.h] */
#include "nh_move.h"

/**
 * @brief A volley of several missiles fired as one action.
 * @note The count and the index within it are both kept because the message differs: "you shoot 3 arrows" is said once, and each arrow then flies
 *       separately.
 */
/**
 * @brief 하나의 행동으로 발사되는 여러 발의 투사체.
 * @note 총 개수와 그 안의 몇 번째인지가 둘 다 보관되는 것은 메시지가 다르기 때문이다. "화살 3발을 쏜다"는 한 번 말해지고, 그다음 각 화살이 따로 날아간다.
 */
struct multishot {
    int n, i;
    short o;
    boolean s;
};

/**
 * @brief What a monster has decided to use this turn.
 *
 * A monster considers its options in three categories -- attack, defence, and everything else -- and the decision is held here rather than acted on
 * immediately, so that the categories can be weighed against each other.
 *
 * @warning The capability numbers and the object pointers are not interchangeable. The existing comment sets out the rule: a non-zero capability may or may
 *          not have an object with it, and the pointer is only set when the capability is an object rather than an innate ability.
 */
/**
 * @brief 몬스터가 이번 턴에 무엇을 쓰기로 정했는지.
 *
 * 몬스터는 자기 선택지를 세 범주 -- 공격, 방어, 그 밖의 전부 -- 로 검토하고, 그 결정이 즉시 실행되는 대신 여기에 보관된다. 그래서 범주들이 서로 견주어질 수 있다.
 *
 * @warning 능력 번호와 물건 포인터는 서로 바꿔 쓸 수 없다. 기존 주석이 그 규칙을 밝힌다. 0이 아닌 능력에 물건이 딸릴 수도 딸리지 않을 수도 있으며, 포인터는 그 능력이 타고난 능력이 아니라 물건일 때만 설정된다.
 */
struct musable {
    struct obj *offensive;
    struct obj *defensive;
    struct obj *misc;
    int has_offense, has_defense, has_misc;
    /* =0, no capability; otherwise, different numbers.
     * If it's an object, the object is also set (it's 0 otherwise).
     */
};

/**
 * @brief The running record of one species this game.
 *
 * Counts of how many have been created and how many killed, plus the flags recording whether the species has been met, wiped out or exhausted. That is what
 * makes population limits and genocide work, and what lets the end-of-game account say what the hero actually did.
 *
 * @warning The counts are single bytes. A species the hero kills a great many of will saturate, so these are a record rather than an exact tally.
 */
/**
 * @brief 이번 게임에서 어떤 종족의 진행 기록.
 *
 * 몇 마리가 만들어졌고 몇 마리가 죽었는지의 개수, 그리고 그 종족을 만났는지 절멸했는지 소진되었는지를 기록하는 플래그. 그것이 개체 수 제한과 절멸을 작동하게 하는 것이고, 게임 종료 시의 보고가 영웅이 실제로 무엇을 했는지 말할 수 있게 하는
 * 것이다.
 *
 * @warning 개수가 단일 바이트다. 영웅이 아주 많이 죽인 종족은 포화하므로, 이들은 정확한 집계가 아니라 기록이다.
 */
struct mvitals {
    uchar born;
    uchar died;
    uchar mvflags;
    Bitfield(seen_close, 1);
    Bitfield(photographed, 1);
};


/**
 * @brief Points at which the game calls out to a script.
 * @note These are moments in the game's life rather than events in play -- starting, restoring, ending, entering the tutorial. A script hooks one to run
 *       something at that point.
 */
/**
 * @brief 게임이 스크립트를 불러내는 지점들.
 * @note 이들은 플레이 중의 사건이 아니라 게임의 생애의 순간들이다. 시작, 복원, 종료, 튜토리얼 진입. 스크립트가 그 중 하나에 걸려 그 지점에서 무언가를 실행한다.
 */
/* Lua callback functions */
enum nhcore_calls {
    NHCORE_START_NEW_GAME = 0,
    NHCORE_RESTORE_OLD_GAME,
    NHCORE_MOVELOOP_TURN,
    NHCORE_GAME_EXIT,
    NHCORE_GETPOS_TIP,
    NHCORE_ENTER_TUTORIAL,
    NHCORE_LEAVE_TUTORIAL,

    NUM_NHCORE_CALLS
};

/**
 * @brief Points during play at which the game calls out to a script.
 * @note These recur, unlike the ones above: before a command, on entering or leaving a level, at the end of a turn.
 * @warning A second mechanism doing the same job as the first. The existing note records that they ought to be merged, so which enumeration a hook belongs to
 *          is historical rather than principled.
 */
/**
 * @brief 플레이 중에 게임이 스크립트를 불러내는 지점들.
 * @note 위의 것들과 달리 이들은 되풀이된다. 명령 전, 레벨에 들어가거나 떠날 때, 턴이 끝날 때.
 * @warning 첫 번째와 같은 일을 하는 두 번째 기제다. 기존 메모는 그것들이 합쳐져야 한다고 기록하고 있으므로, 어떤 훅이 어느 열거에 속하는지는 원칙이 아니라 역사적인 것이다.
 */
/* Lua callbacks. TODO: Merge with NHCORE */
enum nhcb_calls {
    NHCB_CMD_BEFORE = 0,
    NHCB_LVL_ENTER,
    NHCB_LVL_LEAVE,
    NHCB_END_TURN,

    NUM_NHCB
};

/**
 * @def NHUUIDSZ
 * @brief The size of the buffer holding a game's unique identifier as text.
 * @note Fixed by the textual form of the identifier plus a terminator, so it is not a choice.
 */
/**
 * @def NHUUIDSZ
 * @brief 게임의 고유 식별자를 글로 담는 버퍼의 크기.
 * @note 식별자의 글 형태와 종결자에 의해 정해지므로 선택이 아니다.
 */
#define NHUUIDSZ 37

/* message classification, single-argument wrappers, yes/no queries, and
   custompline() flags [gathered from four places; see nh_msg.h] */
#include "nh_msg.h"

/**
 * @brief How a polymorph of the hero is happening.
 * @note Whether the hero chose the form, whether they are becoming a monster rather than another of their own kind, and whether this is the polymorph ending
 *       rather than beginning -- each changes what is allowed and what is said.
 * @note @c POLY_LOW_CTRL is partial control: the hero may choose but the choice can fail, which is a different situation from having no say and from having a
 *       free hand.
 */
/**
 * @brief 영웅의 변신이 어떻게 일어나고 있는지.
 * @note 영웅이 그 형태를 골랐는지, 자기 부류의 다른 개체가 아니라 몬스터가 되는 것인지, 그리고 이것이 변신의 시작이 아니라 끝인지. 각각이 무엇이 허용되는지와 무엇이 말해지는지를 바꾼다.
 * @note @c POLY_LOW_CTRL 은 부분적인 통제다. 영웅이 고를 수는 있으나 그 선택이 실패할 수 있으며, 그것은 발언권이 없는 것과도 자유로운 것과도 다른 상황이다.
 */
/* polyself flags */
enum polyself_flags {
    POLY_NOFLAGS    = 0x00,
    POLY_CONTROLLED = 0x01,
    POLY_MONSTER    = 0x02,
    POLY_REVERT     = 0x04,
    POLY_LOW_CTRL   = 0x08
};

/* [struct repo moved to nh_shop.h] */

/**
 * @brief What is being restored and how strictly.
 * @note The name is for a failure message, so it can say what was being read when a save turned out to be damaged rather than only that it was.
 */
/**
 * @brief 무엇이 복원되고 있고 얼마나 엄격하게인지.
 * @note 이름은 실패 메시지를 위한 것이다. 그래서 저장이 손상된 것으로 드러났을 때 손상되었다는 사실만이 아니라 무엇을 읽던 중이었는지 말할 수 있다.
 */
struct restore_info {
    const char *name;
    int mread_flags;
};

/**
 * @brief How far restoring a saved game has got.
 * @note Three passes rather than one, and the current level is touched in two of them. It has to be: the game's state must be read before the levels, and the
 *       current level cannot be finished until the rest of the dungeon exists to connect it to.
 */
/**
 * @brief 저장 게임 복원이 어디까지 왔는지.
 * @note 한 번이 아니라 세 번의 통과이며, 현재 레벨이 그 중 둘에서 다뤄진다. 그래야 한다. 게임의 상태가 레벨들보다 먼저 읽혀야 하고, 현재 레벨은 그것을 이어 붙일 던전의 나머지가 존재하기 전까지 마무리될 수 없다.
 */
enum restore_stages {
    REST_GSTATE = 1, /* restoring game state + first pass of current level */
    REST_LEVELS = 2, /* restoring remainder of dungeon */
    REST_CURRENT_LEVEL = 3, /* final pass of restoring current level */
};

/**
 * @brief One cell of the Rogue level's grid, while it is being laid out.
 *
 * The Rogue level imitates the game NetHack descends from, which built its levels as a three by three grid of cells, each holding a room or not. So the
 * generator works in cells, and a cell that holds no room is still a cell -- a corridor may pass through it.
 *
 * @note The room number is meaningful only for a cell that actually holds a room, as the existing comment records.
 */
/**
 * @brief Rogue 레벨의 격자 칸 하나. 그것이 배치되는 동안.
 *
 * Rogue 레벨은 NetHack 이 물려받은 그 게임을 모방하며, 그 게임은 레벨을 3×3 칸의 격자로 지었고 각 칸이 방을 담거나 담지 않았다. 그래서 생성기가 칸 단위로 일하고, 방을 담지 않은 칸도 여전히 칸이다. 통로가 그것을 지날 수 있다.
 *
 * @note 기존 주석이 기록하듯 방 번호는 실제로 방을 담은 칸에 대해서만 의미가 있다.
 */
struct rogueroom {
    coordxy rlx, rly;
    coordxy dx, dy;
    boolean real;
    uchar doortable;
    int nroom; /* Only meaningful for "real" rooms */
};

/**
 * @def NUM_ROLES
 * @brief How many roles the game offers.
 * @warning A written number, not derived from the role table. Adding a role means changing this too, and nothing checks that the two agree.
 */
/**
 * @def NUM_ROLES
 * @brief 게임이 제공하는 직업의 수.
 * @warning 직업 표에서 유도된 것이 아니라 적어 놓은 숫자다. 직업을 더하는 것은 이것도 바꾸는 일이며, 둘이 일치하는지 검사하는 것은 없다.
 */
#define NUM_ROLES (13)
/**
 * @brief Which roles a listing should include.
 * @note Holds both a per-role list and a mask over the other choices, so one filter can express "any Valkyrie or Samurai who is also lawful".
 * @note One larger than the role count, because the array is indexed by role and role numbering does not start at zero.
 */
/**
 * @brief 목록이 어떤 직업을 포함해야 하는지.
 * @note 직업별 목록과 다른 선택들에 대한 마스크를 함께 담는다. 그래서 하나의 필터가 "발키리나 사무라이 중 법을 따르는 자"를 표현할 수 있다.
 * @note 직업 수보다 하나 크다. 배열이 직업으로 색인되고 직업 번호가 0에서 시작하지 않기 때문이다.
 */
struct role_filter {
    boolean roles[NUM_ROLES + 1];
    short mask;
};
/**
 * @def NUM_RACES
 * @brief How many races the game offers.
 * @warning Likewise a written number rather than a derived one.
 */
/**
 * @def NUM_RACES
 * @brief 게임이 제공하는 종족의 수.
 * @warning 이것도 유도된 것이 아니라 적어 놓은 숫자다.
 */
#define NUM_RACES (5)

/**
 * @brief A set of map squares, held as a bitmap the size of the map.
 *
 * Level descriptions work with sets of squares -- everywhere in this room, everywhere that is water, a scattering of those. A bitmap rather than a list because
 * the operations wanted are intersection, union and filtering, which are cheap on a bitmap and not on a list.
 *
 * @note The bounding rectangle is a cache, and the flag says whether it is still valid. That is why the existing comment directs callers to the accessor rather
 *       than the field: reading it directly may give bounds from before the last change.
 */
/**
 * @brief 지도 칸의 묶음. 지도 크기의 비트맵으로 보관된다.
 *
 * 레벨 기술은 칸의 묶음을 다룬다. 이 방 안 전부, 물인 곳 전부, 그 중 흩뿌려진 일부. 목록이 아니라 비트맵인 것은, 원하는 연산이 교집합, 합집합, 걸러내기이고 그것들이 비트맵에서는 값싸고 목록에서는 그렇지 않기 때문이다.
 *
 * @note 경계 사각형은 캐시이고, 그 플래그가 그것이 여전히 유효한지 말한다. 기존 주석이 호출자에게 필드가 아니라 접근자를 쓰라고 지시하는 이유가 그것이다. 직접 읽으면 마지막 변경 이전의 경계가 나올 수 있다.
 */
struct selectionvar {
    int wid, hei;
    boolean bounds_dirty;
    NhRect bounds; /* use selection_getbounds() */
    char *map;
};

/* program_state / level_status phases and input-state enum */
#include "nh_progstate.h"

/**
 * @brief One item in a list being sorted for display.
 *
 * Sorting a pile or an inventory is not sorting objects but sorting the way they will be shown, and the sort keys are worked out once and cached here rather
 * than recomputed for every comparison. That is what makes sorting a large pile affordable.
 *
 * @note The order class is not the object class, as the field's name insists. Objects are shown in an order the player finds useful rather than the order the
 *       object table happens to use.
 * @note The original index is kept as a tie-breaker, so that items which compare equal keep their previous relative order rather than shuffling between sorts.
 * @warning The keys must be signed, and the existing comment says why the index is wider than the rest: it has to hold a count of the largest possible pile,
 *          while the others fit in a byte.
 */
/**
 * @brief 표시를 위해 정렬되는 목록의 한 항목.
 *
 * 무더기나 소지품을 정렬하는 것은 물건을 정렬하는 것이 아니라 그것들이 보여질 방식을 정렬하는 것이며, 정렬 키는 비교마다 다시 계산되는 대신 한 번 계산되어 여기에 캐시된다. 그것이 큰 무더기의 정렬을 감당할 수 있게 만드는 것이다.
 *
 * @note 그 필드 이름이 강조하듯 순서 계열은 물건 계열이 아니다. 물건은 물건 표가 마침 쓰는 순서가 아니라 플레이어에게 쓸모 있는 순서로 보여진다.
 * @note 원래 색인이 동점 처리용으로 보관된다. 그래서 같게 비교되는 항목들이 정렬 사이에서 뒤섞이는 대신 이전의 상대 순서를 유지한다.
 * @warning 키들은 부호가 있어야 하며, 기존 주석은 색인이 나머지보다 넓은 이유를 밝힌다. 그것은 가능한 가장 큰 무더기의 개수를 담아야 하고, 나머지는 한 바이트에 들어간다.
 */
/* sortloot() return type; needed before extern.h */
struct sortloot_item {
    struct obj *obj;
    char *str; /* result of loot_xname(obj) in some cases, otherwise null */
    /* these need to be signed; 'indx' should be big enough to hold a count
       of the largest pile of items, the others fit within char */
    int indx;        /* index into original list (used as tie-breaker) */
    int8 orderclass; /* order rather than object class; 0 => not yet init'd */
    int8 subclass;   /* subclass for some classes */
    int8 disco;      /* discovery status */
    int8 inuse;      /* 0: not in-use or not sorting by inuse_only;
                      * 1: lit candle/lamp or attached leash; 2: worn armor;
                      * 3: wielded weapon (including uswapwep and uquiver);
                      * 4: worn accessory (amulet, rings, blindfold). */
};
typedef struct sortloot_item Loot;

/**
 * @brief A string that grows, with a small amount of room built in.
 *
 * Holds a pointer to the text and a fixed buffer alongside it. Short strings live in the buffer and need no allocation; a string that outgrows it moves to
 * allocated memory and the pointer follows. Most strings the game builds are short, so most cost nothing.
 *
 * @warning The pointer may or may not point into the built-in buffer. Freeing it unconditionally, or assuming it does not move as the string grows, are both
 *          mistakes.
 */
/**
 * @brief 자라나는 문자열. 약간의 자리를 내장하고 있다.
 *
 * 글에 대한 포인터와 그 곁의 고정 버퍼를 담는다. 짧은 문자열은 그 버퍼에 살고 할당이 필요하지 않다. 그것을 넘어서 자란 문자열은 할당된 메모리로 옮겨 가고 포인터가 그것을 따른다. 게임이 만드는 대부분의 문자열이 짧으므로 대부분이 아무 비용도
 * 들지 않는다.
 *
 * @warning 그 포인터는 내장 버퍼를 가리킬 수도 있고 아닐 수도 있다. 조건 없이 해제하는 것, 그리고 문자열이 자라도 그것이 옮겨지지 않는다고 가정하는 것은 둘 다 잘못이다.
 */
typedef struct strbuf {
    int    len;
    char  *str;
    char   buf[256];
} strbuf_t;

/**
 * @brief Which of the conditions for being turned to stone hold.
 *
 * Bits rather than a yes or no, because the answer the caller needs is not whether the hero will be petrified but which condition decides it -- so it can say
 * "you are wearing gloves" rather than "nothing happens", and can ask before acting.
 *
 * @note @c st_all is all of them together, for a caller that wants every condition tested rather than stopping at the first.
 */
/**
 * @brief 돌로 변하는 조건 중 무엇이 성립하는지.
 *
 * 예/아니오가 아니라 비트인 것은, 호출자가 필요한 답이 영웅이 석화될지가 아니라 어떤 조건이 그것을 정하는지이기 때문이다. 그래서 "아무 일도 일어나지 않는다" 대신 "당신은 장갑을 끼고 있다"고 말할 수 있고, 행동하기 전에 물을 수 있다.
 *
 * @note @c st_all 은 그 전부를 합친 것이다. 첫 번째에서 멈추는 대신 모든 조건이 검사되기를 원하는 호출자를 위한 것.
 */
enum stoning_checks {
    st_gloves    = 0x1,  /* wearing gloves? */
    st_corpse    = 0x2,  /* is it a corpse obj? */
    st_petrifies = 0x4,  /* does the corpse petrify on touch? */
    st_resists   = 0x8,  /* do you have stoning resistance? */
    st_all = (st_gloves | st_corpse | st_petrifies | st_resists)
};

/**
 * @brief A weapon that comes back when thrown.
 * @note Whether it is tethered matters beyond flavour: a tethered weapon draws a visible line and returns along the path it took, while an untethered one
 *       simply reappears.
 */
/**
 * @brief 던지면 돌아오는 무기.
 * @note 줄이 달렸는지는 분위기 이상의 문제다. 줄 달린 무기는 보이는 선을 그리고 자기가 지나온 경로를 따라 돌아오며, 줄 없는 것은 그냥 다시 나타난다.
 */
struct throw_and_return_weapon {
    short otyp;
    int range;
    Bitfield(tethered, 1);
};

/**
 * @brief A trap the hero is in the middle of setting.
 * @note Setting a trap takes several turns, so this is another of the in-progress records -- the object, where, and how much longer.
 * @note @c force_bungle exists so that a botched attempt can be arranged rather than left to chance, which is what lets the outcome of an interrupted attempt be
 *       decided when the interruption happens rather than when it resumes.
 */
/**
 * @brief 영웅이 설치하는 중인 함정.
 * @note 함정 설치는 여러 턴에 걸리므로, 이것도 진행 중 기록 중 하나다. 그 물건, 어디에, 얼마나 더 걸리는지.
 * @note @c force_bungle 이 있는 것은, 실패한 시도를 우연에 맡기는 대신 정해 둘 수 있게 하기 위함이다. 그것이 중단된 시도의 결과를 이어 갈 때가 아니라 중단이 일어날 때 정할 수 있게 하는 것이다.
 */
struct trapinfo {
    struct obj *tobj;
    coordxy tx, ty;
    int time_needed;
    boolean force_bungle;
};

/**
 * @brief A named region a level description declares, with an area to remove afterwards.
 *
 * Two rectangles: the region itself and one to be cleared once it has served. That second area exists because a description may need to mark somewhere
 * temporarily -- to place something relative to it -- and then not leave the mark behind.
 *
 * @note Either rectangle may be given as a whole level rather than an area, which is what the two accompanying flags say.
 */
/**
 * @brief 레벨 기술이 선언하는 이름 붙은 영역. 그리고 나중에 없앨 구역.
 *
 * 두 사각형이다. 영역 자체와, 그것이 역할을 다한 뒤 지워질 것. 그 두 번째 구역이 있는 것은, 기술이 어딘가를 임시로 표시해 두고 -- 그것에 상대적으로 무언가를 놓기 위해 -- 그 표시를 남기지 않아야 할 수 있기 때문이다.
 *
 * @note 어느 사각형이든 구역이 아니라 레벨 전체로 주어질 수 있으며, 딸린 두 플래그가 그것을 밝힌다.
 */
/* values for rtype are defined in dungeon.h */
/* lev_region from sp_lev.h */
typedef struct {
    struct {
        coordxy x1, y1, x2, y2;
    } inarea;
    struct {
        coordxy x1, y1, x2, y2;
    } delarea;
    boolean in_islev, del_islev;
    coordxy rtype, padding;
    Str_or_Len rname;
} lev_region;

/* savefile compatibility flags, NHFILE handle, and serializer mode bits
   [uptodate/status flags gathered here with the handle; see nh_savefile.h] */
#include "nh_savefile.h"

/**
 * @def ENTITIES
 * @brief How many kinds of valuable the end-of-game accounting tallies separately.
 * @note Gems and amulets are counted apart because they are valued differently, so the accounting keeps two lists rather than one.
 */
/**
 * @def ENTITIES
 * @brief 게임 종료 시의 회계가 따로 집계하는 귀중품의 종류 수.
 * @note 보석과 부적이 따로 세어지는 것은 그 값이 다르게 매겨지기 때문이다. 그래서 회계가 하나가 아니라 두 목록을 보관한다.
 */
#define ENTITIES 2
/**
 * @brief How many of one kind of valuable the hero escaped with.
 * @note Counted by kind rather than listed as objects, since what matters at the end is the tally and not which particular gems they were.
 */
/**
 * @brief 영웅이 한 종류의 귀중품을 몇 개 가지고 탈출했는지.
 * @note 물건으로 나열되지 않고 종류별로 세어진다. 끝에 중요한 것은 그것이 어느 보석이었는지가 아니라 집계이기 때문이다.
 */
struct valuable_data {
    long count;
    int typ;
};

/**
 * @brief One of the tally lists, with its length.
 * @note The length travels with the list because the two lists are of different sizes -- there are more kinds of gem than of amulet.
 */
/**
 * @brief 집계 목록 중 하나와 그 길이.
 * @note 길이가 목록과 함께 다니는 것은 두 목록의 크기가 다르기 때문이다. 부적보다 보석의 종류가 많다.
 */
struct val_list {
    struct valuable_data *list;
    int size;
};

/**
 * @brief How to order the list of monsters the hero killed.
 *
 * Eight orderings, because the list is read for different purposes: to see what the toughest kill was, to find whether a particular monster was met, to see
 * which class the hero fought most. No single order serves all of those.
 *
 * @note The comments give the key the player types for each, so the enumeration and the interface are documented together.
 * @note Several come in pairs differing only in direction, since which end of a list is interesting depends on the question.
 */
/**
 * @brief 영웅이 죽인 몬스터 목록을 어떤 순서로 놓을지.
 *
 * 여덟 가지 순서인 것은, 그 목록이 서로 다른 목적으로 읽히기 때문이다. 가장 강한 사냥이 무엇이었는지 보기 위해, 특정 몬스터를 만났는지 찾기 위해, 영웅이 어느 계열과 가장 많이 싸웠는지 보기 위해. 어느 하나의 순서도 그 전부를 맡지 못한다.
 *
 * @note 주석들이 각각에 대해 플레이어가 입력하는 키를 밝힌다. 그래서 열거와 인터페이스가 함께 기록된다.
 * @note 여러 개가 방향만 다른 짝으로 온다. 목록의 어느 끝이 흥미로운지가 질문에 달려 있기 때문이다.
 */
enum vanq_order_modes {
    VANQ_MLVL_MNDX = 0, /* t - traditional: by monster level */
    VANQ_MSTR_MNDX,     /* d - by difficulty rating */
    VANQ_ALPHA_SEP,     /* a - alphabetical, first uniques, then ordinary */
    VANQ_ALPHA_MIX,     /* A - alpha with uniques and ordinary intermixed */
    VANQ_MCLS_HTOL,     /* C - by class, high to low within class */
    VANQ_MCLS_LTOH,     /* c - by class, low to high within class */
    VANQ_COUNT_H_L,     /* n - by count, high to low */
    VANQ_COUNT_L_H,     /* z - by count, low to high */

    NUM_VANQ_ORDER_MODES
};

/**
 * @brief One of the player's exceptions to automatic pickup.
 *
 * A pattern with a direction: pick this up despite the class rules, or leave it alone despite them. Exceptions rather than a second rule set, because a player
 * wants "all scrolls except blank paper" and expressing that as classes is not possible.
 *
 * @note The pattern text is kept alongside the compiled form so the rule can be shown back as written.
 * @note A list, and the order is the precedence.
 */
/**
 * @brief 자동 집기에 대한 플레이어의 예외 하나.
 *
 * 방향이 딸린 패턴이다. 계열 규칙에도 불구하고 이것을 집으라, 또는 그 규칙에도 불구하고 이것은 두라. 두 번째 규칙 묶음이 아니라 예외인 것은, 플레이어가 "백지를 뺀 모든 두루마리"를 원하고 그것을 계열로 표현하는 것이 불가능하기 때문이다.
 *
 * @note 패턴의 글이 컴파일된 형태와 함께 보관된다. 그래서 그 규칙을 쓴 그대로 되보여 줄 수 있다.
 * @note 목록이며, 그 순서가 우선순위다.
 */
struct autopickup_exception {
    struct nhregex *regex;
    char *pattern;
    boolean grab;
    struct autopickup_exception *next;
};

/**
 * @brief An attempt to open something locked, in progress.
 *
 * Picking a lock and forcing one open take several turns, so this is another in-progress record. One structure serves both a door and a box because the two
 * cannot be worked on at once.
 *
 * @warning At most one of the two targets is set at any time, as the existing comment insists. Both being set is a bug rather than a case to handle.
 * @note The tool field means different things for the two operations, as its comment records: which kind of unlocking tool for picking, and sharp against blunt
 *       for forcing. The operation decides the reading.
 * @note @c magic_key is kept because a magic key never fails, which changes both the outcome and what is said about the attempt.
 */
/**
 * @brief 잠긴 것을 여는 시도. 진행 중.
 *
 * 자물쇠를 따는 것과 억지로 여는 것은 여러 턴에 걸리므로, 이것도 진행 중 기록 중 하나다. 하나의 구조체가 문과 상자를 함께 맡는 것은, 그 둘을 동시에 다룰 수 없기 때문이다.
 *
 * @warning 기존 주석이 강조하듯 두 대상 중 최대 하나만이 어느 순간에든 설정된다. 둘 다 설정된 것은 다룰 경우가 아니라 버그다.
 * @note 도구 필드는 두 연산에 대해 다른 것을 뜻하며, 그 주석이 그것을 기록한다. 따기에는 어떤 종류의 여는 도구인지, 억지로 열기에는 날카로운지 뭉툭한지. 연산이 그 해석을 정한다.
 * @note @c magic_key 가 보관되는 것은 마법 열쇠는 결코 실패하지 않기 때문이다. 그것이 결과와 그 시도에 대해 말해지는 것을 함께 바꾼다.
 */
/* at most one of `door' and `box' should be non-null at any given time */
struct xlock_s {
    struct rm *door;
    struct obj *box;
    int picktyp, /* key|pick|card for unlock, sharp vs blunt for #force */
        chance, usedtime;
    boolean magic_key;
};

/**
 * @def MAX_BMASK
 * @brief How many blocking masks a vision calculation keeps at once.
 */
/**
 * @def MAX_BMASK
 * @brief 시야 계산이 한 번에 보관하는 차단 마스크의 개수.
 */
#define MAX_BMASK 4

/* [NHFILE handle and mode bits now included earlier, with the savefile
   compatibility flags; see nh_savefile.h] */

/* articles and suppress masks used when naming a monster */
#include "nh_monnam.h"

/* [single-argument pline wrappers moved to nh_msg.h] */

/* directory classes used to locate data and state files */
#include "nh_fileprefix.h"

/**
 * @def MAX_MENU_MAPPED_CMDS
 * @brief How many menu keys the player may remap.
 * @note An arbitrary limit, as the existing comment admits. It bounds a fixed array rather than expressing anything about menus.
 */
/**
 * @def MAX_MENU_MAPPED_CMDS
 * @brief 플레이어가 다시 대응시킬 수 있는 메뉴 키의 개수.
 * @note 기존 주석이 인정하듯 임의의 한계다. 메뉴에 관한 무엇을 표현하는 것이 아니라 고정 배열의 크기를 정한다.
 */
/* from options.c */
#define MAX_MENU_MAPPED_CMDS 32 /* some number */

/**
 * @name Character creation choices, as indices
 * @brief The four things a player chooses, numbered so they can index a table.
 * @note In alphabetical order rather than the order they are asked in. The numbering is for indexing and not for sequencing, so nothing should walk it as a
 *       progression.
 * @{
 */
/**
 * @name 캐릭터 생성 선택. 색인으로.
 * @brief 플레이어가 고르는 네 가지. 표를 색인할 수 있도록 번호가 붙어 있다.
 * @note 묻는 순서가 아니라 알파벳 순서다. 그 번호는 색인을 위한 것이고 순서를 위한 것이 아니므로, 어느 것도 그것을 진행으로 밟아 나가서는 안 된다.
 * @{
 */
/* player selection constants */
#define BP_ALIGN 0
#define BP_GEND 1
#define BP_RACE 2
#define BP_ROLE 3
#define NUM_BP 4
/** @} */

/**
 * @name Fixed buffer sizes
 * @brief Sizes for a few globals that hold text of bounded length.
 * @note Gathered here rather than beside their globals because the globals themselves live in the grouped structures, where a size would be out of place.
 * @{
 */
/**
 * @name 고정 버퍼 크기
 * @brief 길이가 정해진 글을 담는 몇몇 전역 변수의 크기.
 * @note 그 전역 변수 곁이 아니라 여기에 모여 있는 것은, 전역 변수 자체가 묶인 구조체 안에 살고 그곳에서는 크기가 어울리지 않기 때문이다.
 * @{
 */
/* some array sizes for 'g?' */
#define WIZKIT_MAX 128
#define CVT_BUF_SIZE 64

#define LUA_VER_BUFSIZ 20
#define LUA_COPYRIGHT_BUFSIZ 120
/** @} */

/**
 * @name Symbol table offsets
 * @brief Where each kind of symbol begins within one combined table.
 *
 * All the symbol kinds -- terrain, object classes, monster classes, warnings, the rest -- are held in a single table so that a player's symbol settings can be
 * one list. Each offset is computed from the previous one plus that kind's count, so the layout is described once.
 *
 * @warning The same arrangement as the glyph ranges, and the same hazard: every offset depends on the one before it, so changing a count shifts everything after
 *          it. A saved symbol set is stated in names rather than numbers, which is what keeps that from breaking configuration files.
 * @{
 */
/**
 * @name 심볼 표 오프셋
 * @brief 하나의 통합 표 안에서 각 종류의 심볼이 어디서 시작하는지.
 *
 * 모든 심볼 종류 -- 지형, 물건 계열, 몬스터 계열, 경고, 나머지 -- 가 하나의 표에 담긴다. 그래서 플레이어의 심볼 설정이 하나의 목록일 수 있다. 각 오프셋이 이전 것에 그 종류의 개수를 더해 계산되므로, 배치가 한 번만 기술된다.
 *
 * @warning 글리프 구간과 같은 배치이고 같은 위험이다. 모든 오프셋이 그 앞의 것에 의존하므로, 개수를 바꾸면 그 뒤의 모든 것이 밀린다. 저장된 심볼 세트가 숫자가 아니라 이름으로 적혀 있는 것이 그것이 설정 파일을 깨뜨리지 않게 하는 것이다.
 * @{
 */
/* Symbol offsets */
#define SYM_OFF_P (0)
#define SYM_OFF_O (SYM_OFF_P + MAXPCHARS)   /* MAXPCHARS from sym.h */
#define SYM_OFF_M (SYM_OFF_O + MAXOCLASSES) /* MAXOCLASSES from objclass.h */
#define SYM_OFF_W (SYM_OFF_M + MAXMCLASSES) /* MAXMCLASSES from sym.h*/
#define SYM_OFF_X (SYM_OFF_W + WARNCOUNT)
#define SYM_MAX (SYM_OFF_X + MAXOTHER)
/** @} */

/**
 * @name Initialisers that say nothing
 * @brief Values used to initialise a variable whose starting value does not matter.
 *
 * They exist to distinguish two situations that look alike in the source: a variable given zero because zero is the right starting value, and one given zero
 * because the compiler requires an initialiser and nothing depends on it. The second is what these say.
 *
 * @note Three forms because a scalar, a structure and a pointer need different spellings, and the existing comment states which is for which.
 * @warning Not interchangeable. Using the scalar form on a pointer, or the structure form on a scalar, may compile with a warning or may not compile at all
 *          depending on the compiler.
 * @{
 */
/**
 * @name 아무것도 말하지 않는 초기화자
 * @brief 시작값이 중요하지 않은 변수를 초기화하는 데 쓰이는 값.
 *
 * 소스에서 똑같아 보이는 두 상황을 구별하기 위해 존재한다. 0이 올바른 시작값이기 때문에 0을 받은 변수와, 컴파일러가 초기화자를 요구하고 그것에 의존하는 것이 없기 때문에 0을 받은 변수. 뒤의 것이 이들이 말하는 바다.
 *
 * @note 세 형태인 것은 스칼라, 구조체, 포인터가 서로 다른 표기를 필요로 하기 때문이며, 기존 주석이 어느 것이 어디에 쓰이는지 밝힌다.
 * @warning 서로 바꿔 쓸 수 없다. 포인터에 스칼라 형태를 쓰거나 스칼라에 구조체 형태를 쓰는 것은, 컴파일러에 따라 경고와 함께 컴파일되거나 아예 컴파일되지 않을 수 있다.
 * @{
 */
/* The UNDEFINED macros are used to initialize variables whose
   initialized value is not relied upon.
   UNDEFINED_VALUE: used to initialize any scalar type except pointers.
   UNDEFINED_VALUES: used to initialize any non scalar type without pointers.
   UNDEFINED_PTR: can be used only on pointer types. */
#define UNDEFINED_VALUE 0
#define UNDEFINED_VALUES { 0 }
#define UNDEFINED_PTR NULL
/** @} */

/**
 * @def UNDEFINED_ROLE
 * @brief An empty role entry, for a slot that holds no role.
 *
 * Written out in full rather than zeroed, because the fields are not all meaningfully zero: a monster index of zero is a real monster and an object index of zero
 * is a real object, so the absent-value constants have to be named explicitly.
 *
 * @warning The initialiser is positional, and its internal comments are the only thing naming which field each value belongs to. Adding a field to the role
 *          structure means adding a value here in the right place, and a mistake produces a valid-looking role rather than an error.
 */
/**
 * @def UNDEFINED_ROLE
 * @brief 빈 직업 항목. 직업을 담지 않은 자리를 위한 것.
 *
 * 0으로 채우는 대신 전부 적어 낸다. 필드들이 모두 0으로 의미 있게 되지는 않기 때문이다. 몬스터 색인 0은 실제 몬스터이고 물건 색인 0은 실제 물건이므로, 없음을 뜻하는 상수들이 명시적으로 지칭되어야 한다.
 *
 * @warning 이 초기화자는 위치 기반이며, 그 안의 주석들만이 각 값이 어느 필드에 속하는지 밝힌다. 직업 구조체에 필드를 더하는 것은 여기에 알맞은 자리에 값을 더하는 일이며, 실수는 오류가 아니라 유효해 보이는 직업을 만들어 낸다.
 */
/* The UNDEFINED_ROLE macro is used to initialize Role variables */
#define UNDEFINED_ROLE \
    {                                           \
      /* role name, set of rank names */        \
      { NULL, NULL }, { { NULL, NULL } },       \
      /* strings: pantheon deity names */       \
      NULL, NULL, NULL,                         \
      /* file code, quest home+goal names */    \
      NULL, NULL, NULL,                         \
      /* indices: base mon type, pet */         \
      NON_PM, NON_PM,                           \
      /* quest leader, guardians, nemesis */    \
      NON_PM, NON_PM, NON_PM,                   \
      /* quest enemy types (index, symbol) */   \
      NON_PM, NON_PM, '\0', '\0',               \
      /* quest artifact object index */         \
      STRANGE_OBJECT,                           \
      /* Bitmasks */                            \
      0,                                        \
      /* Attributes */                          \
      {0}, {0}, {0}, {0}, 0, 0,                 \
      /* spell statistics */                    \
      0, 0, 0, 0, 0, 0, 0 }

/**
 * @def UNDEFINED_RACE
 * @brief An empty race entry, for the same reason and with the same hazard.
 */
/**
 * @def UNDEFINED_RACE
 * @brief 빈 종족 항목. 같은 이유로, 같은 위험과 함께.
 */
/* The UNDEFINED_RACE macro is used to initialize Race variables */
#define UNDEFINED_RACE \
    {                                           \
      /* strings */                             \
      NULL, NULL, NULL, NULL, { NULL, NULL },   \
      /* Indices: base race, mummy, zombie */   \
      NON_PM, NON_PM, NON_PM,                   \
      /* Bitmasks */                            \
      0, 0, 0, 0,                               \
      /* Characteristic limits */               \
      {0}, {0},                                 \
      /* Level change HP and Pw adjustments */  \
      {0}, {0}                                  \
    }

/**
 * @def MATCH_WARN_OF_MON
 * @brief Whether the hero is being warned specifically about this monster.
 * @param mon the monster
 * @note Checks all three sources of a specific warning: the class an object warns of, the class the hero's current form warns of, and the exact species a form
 *       may warn of. All three are needed because they end at different times and any may be active alone.
 * @warning The argument is evaluated several times.
 */
/**
 * @def MATCH_WARN_OF_MON
 * @brief 영웅이 특히 이 몬스터에 대해 경고받고 있는지.
 * @param mon 그 몬스터
 * @note 특정 경고의 세 출처를 모두 검사한다. 어떤 물건이 경고하는 계열, 영웅의 현재 형태가 경고하는 계열, 그리고 형태가 경고할 수 있는 정확한 종족. 셋 다 필요한 것은 끝나는 시점이 다르고 어느 것이든 혼자 유효할 수 있기 때문이다.
 * @warning 인자가 여러 번 평가된다.
 */
#define MATCH_WARN_OF_MON(mon) \
    (Warn_of_mon                                                        \
     && ((svc.context.warntype.obj & (mon)->data->mflags2) != 0           \
         || (svc.context.warntype.polyd & (mon)->data->mflags2) != 0      \
         || (svc.context.warntype.species                                 \
             && (svc.context.warntype.species == (mon)->data))))

/* makemon()/goodpos() control flags, sharing one bit space */
#include "nh_makemon.h"

/**
 * @name Describing something hidden
 * @brief How much to say about a monster that is pretending to be something else.
 * @note The pieces are separate because the description goes into sentences of different shapes -- sometimes as a clause appended to a name, sometimes as a
 *       standalone report -- and the caller knows which.
 * @{
 */
/**
 * @name 숨은 것을 기술하기
 * @brief 다른 것인 척하고 있는 몬스터에 대해 얼마나 말할지.
 * @note 조각들이 따로 있는 것은, 그 기술이 서로 다른 모양의 문장에 들어가기 때문이다. 때로는 이름에 덧붙는 절로, 때로는 독립된 보고로. 그리고 호출자가 어느 쪽인지 안다.
 * @{
 */
/* flags for mhidden_description() (pager.c; used for mimics and hiders) */
#define MHID_PREFIX  1 /* include ", mimicking " prefix */
#define MHID_ARTICLE 2 /* include "a " or "an " after prefix */
#define MHID_ALTMON  4 /* if mimicking a monster, include that */
#define MHID_REGION  8 /* include region when mon is in one */
/** @} */

/**
 * @name Revealing a mimic
 * @brief Whether to actually reveal it, and how to word the surprise.
 * @note Revealing and describing are separate because the hero may work out that something is a mimic without the mimic dropping its disguise.
 * @note The second flag trims the exclamation from the front of the sentence, for a caller that has already established the surprise.
 * @{
 */
/**
 * @name 모방자를 드러내기
 * @brief 실제로 드러낼지, 그리고 그 놀라움을 어떻게 표현할지.
 * @note 드러내는 것과 기술하는 것이 따로 있는 것은, 모방자가 위장을 벗지 않은 채로도 영웅이 그것이 모방자임을 알아낼 수 있기 때문이다.
 * @note 두 번째 플래그는 문장 앞의 감탄을 잘라 낸다. 이미 그 놀라움을 세워 둔 호출자를 위한 것.
 * @{
 */
/* flags for that_is_a_mimic() */
#define MIM_REVEAL    1 /* seemimic() */
#define MIM_OMIT_WAIT 2 /* strip beginning from "Wait!  That is a <foo>" */
/** @} */

/* corpse/statue creation flags, low bits stored in obj->spe */
#include "nh_corpstat.h"

/**
 * @name Collecting nearby squares
 * @brief How to gather the squares around a point, and in what order.
 *
 * Squares are collected in rings by distance and each ring shuffled, so that something spreading outward picks randomly within a ring but never reaches a
 * farther square before a nearer one. The flags adjust that: include the centre, do not shuffle, treat pairs of rings as one, or skip squares that will not do.
 *
 * @warning Two of these contradict each other, and the existing comment states the resolution: asking for ring pairs and for no shuffling at the same time is
 *          meaningless, and no shuffling wins.
 * @note The two skip flags exclude for different reasons. One rejects squares with monsters on them; the other rejects rock and wall while still allowing pools,
 *       which is a narrower rejection than "not walkable" and is what its comment spells out.
 * @{
 */
/**
 * @name 근처 칸 모으기
 * @brief 어떤 점 둘레의 칸을 어떻게, 어떤 순서로 모을지.
 *
 * 칸은 거리에 따라 고리로 모이고 각 고리가 섞인다. 그래서 밖으로 퍼지는 것이 고리 안에서는 무작위로 고르면서도 가까운 칸보다 먼 칸에 먼저 닿는 일이 결코 없다. 플래그들이 그것을 조정한다. 중심을 포함하기, 섞지 않기, 고리 짝을 하나로
 * 다루기, 또는 쓸 수 없는 칸을 건너뛰기.
 *
 * @warning 이 중 둘이 서로 모순되며, 기존 주석이 그 해결을 밝힌다. 고리 짝과 섞지 않기를 동시에 요청하는 것은 무의미하고, 섞지 않기가 이긴다.
 * @note 두 건너뛰기 플래그는 서로 다른 이유로 제외한다. 하나는 몬스터가 있는 칸을 거부하고, 다른 하나는 물웅덩이는 여전히 허용하면서 암반과 벽을 거부한다. 그것은 "걸을 수 없음"보다 좁은 거부이며, 그 주석이 그것을 적어 두고 있다.
 * @{
 */
/* flag bits for collect_coords(); combining ring_pairs with unshuffled
   makes no sense--if both are specified unshuffled takes precedence */
#define CC_NO_FLAGS    0x00 /* skip center, collect in distinct rings and
                             * shuffle each ring, ignore monster occupants */
#define CC_INCL_CENTER 0x01 /* include center point as ring #0 */
#define CC_UNSHUFFLED  0x02 /* don't shuffle the rings */
#define CC_RING_PAIRS  0x04 /* shuffle w/ odd and next even rings together */
#define CC_SKIP_MONS   0x08 /* skip locations occupied by monsters */
#define CC_SKIP_INACCS 0x10 /* skip !ZAP_POS: reject rock and wall locations
                             * but allow pools, unlike !ACCESSIBLE */
/** @} */

/**
 * @name Announcing a change of shape
 * @brief Whether a shapeshift should be reported, and unconditionally or only if seen.
 * @note Two levels because a shapeshift the hero did not witness usually should not be mentioned, but sometimes must be -- when the hero is the one shifting, for
 *       instance.
 * @{
 */
/**
 * @name 모습 변화 알리기
 * @brief 모습 변화를 알릴지, 그리고 조건 없이인지 보였을 때만인지.
 * @note 두 단계인 것은, 영웅이 목격하지 않은 모습 변화는 보통 언급되지 않아야 하지만 때로는 반드시 언급되어야 하기 때문이다. 예컨대 변하는 것이 영웅 자신일 때.
 * @{
 */
/* flags for decide_to_shift() */
#define SHIFT_SEENMSG 0x01 /* put out a message if in sight */
#define SHIFT_MSG 0x02     /* always put out a message */
/** @} */

/**
 * @name Tolerance of poison gas
 * @brief How a monster fares in poison gas, in three degrees.
 * @note Three rather than two because the middle case matters: a monster that is not harmed may still cough, which is worth a message and may still interrupt what
 *       it was doing. Collapsing that into "unaffected" would make a gas cloud look inert.
 * @{
 */
/**
 * @name 독가스에 대한 내성
 * @brief 몬스터가 독가스에서 어떻게 되는지. 세 단계로.
 * @note 둘이 아니라 셋인 것은 가운데 경우가 중요하기 때문이다. 해를 입지 않는 몬스터도 여전히 기침할 수 있고, 그것은 메시지를 낼 가치가 있으며 그것이 하던 일을 여전히 중단시킬 수 있다. 그것을 "영향 없음"으로 뭉개면 가스 구름이 무력해
 *       보이게 된다.
 * @{
 */
/* m_poisongas_ok() return values */
#define M_POISONGAS_BAD   0 /* poison gas is bad */
#define M_POISONGAS_MINOR 1 /* poison gas is ok, maybe causes coughing */
#define M_POISONGAS_OK    2 /* ignores poison gas completely */
/** @} */

/**
 * @name Handing objects to a monster
 * @brief How many of a set of objects to give, and which.
 * @note @c DF_ALL and @c DF_RANDOM are the two useful cases: everything, or a randomly chosen one. Without either, the default is a single specific object.
 * @warning The values are not consecutive -- the middle bit is unused -- so these are flags to be tested and not a range to be walked.
 * @{
 */
/**
 * @name 몬스터에게 물건 건네기
 * @brief 물건 묶음 중 몇 개를 줄지, 그리고 어느 것을.
 * @note @c DF_ALL 과 @c DF_RANDOM 이 쓸모 있는 두 경우다. 전부, 또는 무작위로 고른 하나. 둘 다 없으면 기본은 특정한 물건 하나다.
 * @warning 값들이 연속하지 않는다. 가운데 비트가 쓰이지 않는다. 그래서 이들은 밟아 나갈 범위가 아니라 검사할 플래그다.
 * @{
 */
/* flags for deliver_obj_to_mon */
#define DF_NONE     0x00
#define DF_RANDOM   0x01
#define DF_ALL      0x04
/** @} */

/**
 * @def DEFUNCT_MONSTER
 * @brief The health value that marks a monster from a bones file as one that may no longer exist.
 *
 * A bones file may contain a species the current game has wiped out or exhausted. Such a monster cannot simply be dropped when the file is read, because the level
 * has been built around it -- so it is loaded and marked, and removed afterwards.
 *
 * @warning A negative maximum health, which is otherwise impossible. That is what makes it a marker rather than a value, and code that treats health arithmetically
 *          must not encounter one.
 */
/**
 * @def DEFUNCT_MONSTER
 * @brief 유골 파일에서 온 몬스터를 더는 존재할 수 없는 것으로 표시하는 체력 값.
 *
 * 유골 파일은 현재 게임이 절멸시켰거나 소진한 종족을 담고 있을 수 있다. 그런 몬스터를 파일을 읽을 때 그냥 버릴 수는 없다. 레벨이 그것을 둘러싸고 지어져 있기 때문이다. 그래서 적재된 뒤 표시되고 나중에 제거된다.
 *
 * @warning 음수인 최대 체력이며, 그것은 그러지 않으면 불가능하다. 그것이 이것을 값이 아니라 표시로 만드는 것이고, 체력을 산술적으로 다루는 코드는 이것을 마주쳐서는 안 된다.
 */
/* special mhpmax value when loading bones monster to flag as extinct or
 * genocided */
#define DEFUNCT_MONSTER (-100)

/* macro form of adjustments of physical damage based on Half_physical_damage.
 * Can be used on-the-fly with the 1st parameter to losehp() if you don't
 * need to retain the dmg value beyond that call scope.
 * Take care to ensure it doesn't get used more than once in other instances.
 */
/**
 * @def Maybe_Half_Phys
 * @brief Halve a damage amount if the hero has that protection, rounding up.
 * @param dmg the damage before protection
 * @note Rounds up so that protection never reduces damage to nothing -- a blow that lands still hurts.
 * @warning The argument is evaluated twice. The existing comment gives the practical rule: use it directly as an argument where the value is not needed again, and
 *          take care never to use it more than once in one expression.
 */
/**
 * @def Maybe_Half_Phys
 * @brief 영웅이 그 보호를 지니고 있으면 피해량을 절반으로. 올림하여.
 * @param dmg 보호 전의 피해량
 * @note 올림하므로 보호가 피해를 결코 0으로 줄이지 않는다. 맞은 타격은 여전히 아프다.
 * @warning 인자가 두 번 평가된다. 기존 주석이 실질적인 규칙을 밝힌다. 값이 다시 필요하지 않은 곳에서 인자로 직접 쓰고, 하나의 수식 안에서 한 번 넘게 쓰지 않도록 주의할 것.
 */
#define Maybe_Half_Phys(dmg) \
    ((Half_physical_damage) ? (((dmg) + 1) / 2) : (dmg))

/* object-selection menus: what to offer, by category, and in what order
   [sortloot flags gathered here too; see nh_objsel.h] */
#include "nh_objsel.h"

/* where an artifact came from, recorded when it is named */
#include "nh_oname.h"

/**
 * @name Where to look for a monster
 * @brief Which lists a search by identity should cover.
 *
 * A monster may not be on the current level. It may be in transit to another level, following the hero between levels, or be the hero themselves. Those are separate
 * lists, and a caller has to say which of them its question covers -- looking only on the current level is often right and often wrong.
 *
 * @note @c FM_EVERYWHERE is all of them, for a search that must not miss.
 * @warning A search that omits a list will simply not find a monster that is on it, which reads as the monster not existing.
 * @{
 */
/**
 * @name 몬스터를 어디서 찾을지
 * @brief 정체로 찾는 검색이 어느 목록을 덮어야 하는지.
 *
 * 몬스터가 현재 레벨에 없을 수 있다. 다른 레벨로 옮겨 가는 중일 수도, 영웅을 따라 레벨 사이를 오는 중일 수도, 영웅 자신일 수도 있다. 그것들은 별개의 목록이며, 호출자가 자기 질문이 그 중 어느 것을 덮는지 말해야 한다. 현재 레벨만 보는 것은
 * 종종 옳고 종종 틀리다.
 *
 * @note @c FM_EVERYWHERE 는 그 전부다. 놓쳐서는 안 되는 검색을 위한 것.
 * @warning 어떤 목록을 빠뜨린 검색은 그 목록에 있는 몬스터를 그냥 찾지 못하며, 그것은 그 몬스터가 존재하지 않는 것으로 읽힌다.
 * @{
 */
/* Flags to control find_mid() and whereis_mon() */
#define FM_FMON 0x01    /* search the fmon chain */
#define FM_MIGRATE 0x02 /* search the migrating monster chain */
#define FM_MYDOGS 0x04  /* search gm.mydogs */
#define FM_YOU 0x08     /* check for gy.youmonst */
#define FM_EVERYWHERE (FM_YOU | FM_FMON | FM_MIGRATE | FM_MYDOGS)
/** @} */

/**
 * @name Choosing a character trait
 * @brief Whether an unspecified choice may be filled in at random or must be left alone.
 * @note The strict form exists for validating what the player asked for: a role that cannot be a dwarf must not have "dwarf" quietly replaced by something workable.
 * @{
 */
/**
 * @name 캐릭터 특성 고르기
 * @brief 정해지지 않은 선택을 무작위로 채워도 되는지, 아니면 그대로 두어야 하는지.
 * @note 엄격한 형태는 플레이어가 요청한 것을 검증하기 위해 존재한다. 드워프가 될 수 없는 직업은 "드워프"가 조용히 통하는 것으로 갈아치워져서는 안 된다.
 * @{
 */
/* Flags to control pick_[race,role,gend,align] routines in role.c */
#define PICK_RANDOM 0
#define PICK_RIGID 1
/** @} */

/* circumstances under which a trap is being triggered */
#include "nh_trapflags.h"

/* [test_move flags and m_move return values moved to nh_move.h] */

/* [yes/no query wrappers moved to nh_msg.h] */

/* Macros for scatter */
#define VIS_EFFECTS 0x01 /* display visual effects */
#define MAY_HITMON 0x02  /* objects may hit monsters */
#define MAY_HITYOU 0x04  /* objects may hit you */
#define MAY_HIT (MAY_HITMON | MAY_HITYOU)
#define MAY_DESTROY 0x08  /* objects may be destroyed at random */
#define MAY_FRACTURE 0x10 /* boulders & statues may fracture */

/* Macros for launching objects */
#define ROLL 0x01          /* the object is rolling */
#define FLING 0x02         /* the object is flying through the air */
#define LAUNCH_UNSEEN 0x40 /* hero neither caused nor saw it */
#define LAUNCH_KNOWN 0x80  /* the hero caused this by explicit action */

/* enlightenment control flags */
#define BASICENLIGHTENMENT 1 /* show mundane stuff */
#define MAGICENLIGHTENMENT 2 /* show intrinsics and such */
#define ENL_GAMEINPROGRESS 0
#define ENL_GAMEOVERALIVE  1 /* ascension, escape, quit, trickery */
#define ENL_GAMEOVERDEAD   2

/* [sortloot() control flags moved to nh_objsel.h] */

/* flags for xkilled() [note: meaning of first bit used to be reversed,
   1 to give message and 0 to suppress] */
#define XKILL_GIVEMSG   0
#define XKILL_NOMSG     1
#define XKILL_NOCORPSE  2
#define XKILL_NOCONDUCT 4

/* [custompline() pline_flags moved to nh_msg.h] */

/* get_count flags */
#define GC_NOFLAGS   0
#define GC_SAVEHIST  1 /* save "Count: 123" in message history */
#define GC_CONDHIST  2 /* save "Count: N" in message history unless the
                        * first digit is passed in and N matches it */
#define GC_ECHOFIRST 4 /* echo "Count: 1" even when there's only one digit */

/* rloc() flags */
#define RLOC_NONE    0x00
#define RLOC_ERR     0x01 /* allow impossible() if no rloc */
#define RLOC_MSG     0x02 /* show vanish/appear msg */
#define RLOC_NOMSG   0x04 /* prevent appear msg, even for STRAT_APPEARMSG */

/* indices for some special tin types */
#define ROTTEN_TIN 0
#define HOMEMADE_TIN 1
#define SPINACH_TIN (-1)
#define RANDOM_TIN (-2)
#define HEALTHY_TIN (-3)

/* Corpse aging */
#define TAINT_AGE (50L)        /* age when corpses go bad */
#define TROLL_REVIVE_CHANCE 37 /* 1/37 chance for 50 turns ~ 75% chance */
#define ROT_AGE (250L)         /* age when corpses rot away */

/* Some misc definitions */
#define POTION_OCCUPANT_CHANCE(n) (13 + 2 * (n))
#define WAND_BACKFIRE_CHANCE 100
#define WAND_WREST_CHANCE 121
#define BALL_IN_MON (u.uswallow && uball && uball->where == OBJ_FREE)
#define CHAIN_IN_MON (u.uswallow && uchain && uchain->where == OBJ_FREE)
#define NODIAG(monnum) ((monnum) == PM_GRID_BUG)

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_FULL 2
#define MENU_PARTIAL 3

/* flags to control teleds() */
#define TELEDS_NO_FLAGS   0
#define TELEDS_ALLOW_DRAG 1
#define TELEDS_TELEPORT   2

/* flags for mktrap() */
#define MKTRAP_NOFLAGS       0x0U
#define MKTRAP_SEEN          0x1U /* trap is seen */
#define MKTRAP_MAZEFLAG      0x2U /* choose random coords instead of room */
#define MKTRAP_NOSPIDERONWEB 0x4U /* web will not generate a spider */
#define MKTRAP_NOVICTIM      0x8U /* no victim corpse or items on it */

#define MON_POLE_DIST 5 /* How far monsters can use pole-weapons */
#define PET_MISSILE_RANGE2 36 /* Square of distance within which pets shoot */

/* flags passed to getobj() to control how it responds to player input */
#define GETOBJ_NOFLAGS  0x0
#define GETOBJ_ALLOWCNT 0x1 /* is a count allowed with this command? */
#define GETOBJ_PROMPT   0x2 /* should it force a prompt for input? (prevents
                             * it exiting early with "You don't have anything
                             * to foo" if nothing in inventory is valid) */

/* flags for hero_breaks() and hits_bars(); BRK_KNOWN* let callers who have
   already called breaktest() prevent it from being called again since it
   has a random factor which makes it be non-deterministic */
#define BRK_BY_HERO        0x01
#define BRK_FROM_INV       0x02
#define BRK_KNOWN2BREAK    0x04
#define BRK_KNOWN2NOTBREAK 0x08
#define BRK_KNOWN_OUTCOME  (BRK_KNOWN2BREAK | BRK_KNOWN2NOTBREAK)
#define BRK_MELEE          0x10

/* extended command return values */
#define ECMD_OK     0x00 /* cmd done successfully */
#define ECMD_TIME   0x01 /* cmd took time, uses up a turn */
#define ECMD_CANCEL 0x02 /* cmd canceled by user */
#define ECMD_FAIL   0x04 /* cmd failed to finish, maybe with a yafm */

/* flags for newcham() */
#define NO_NC_FLAGS          0U
#define NC_SHOW_MSG          0x01U
#define NC_VIA_WAND_OR_SPELL 0x02U

/* Constant passed to explode() for gas spores because gas spores are weird.
 * Specifically, this is an exception to whole "explode() uses dobuzz types"
 * system (the range -1 to -9 isn't used by it, for some reason), where this
 * is effectively an extra dobuzz type, and some zap.c code needs to be aware
 * of it. */
#define PHYS_EXPL_TYPE -1

/* macros for dobuzz() type */
#define BZ_VALID_ADTYP(adtyp) ((adtyp) >= AD_MAGM && (adtyp) <= AD_SPC2)

#define BZ_OFS_AD(adtyp) (abs((adtyp) - AD_MAGM) % 10)
#define BZ_OFS_WAN(otyp) (abs((otyp) - WAN_MAGIC_MISSILE) % 10)
#define BZ_OFS_SPE(otyp) (abs((otyp) - SPE_MAGIC_MISSILE) % 10)
/* hero shooting a wand */
#define BZ_U_WAND(bztyp) (0 + (bztyp))     /*  0..9  */
/* hero casting a spell */
#define BZ_U_SPELL(bztyp) (10 + (bztyp))   /* 10..19 */
/* hero breathing as a monster */
#define BZ_U_BREATH(bztyp) (20 + (bztyp))  /* 20..29 */
/* monster casting a spell */
#define BZ_M_SPELL(bztyp) (-10 - (bztyp))  /* -19..-10 */
/* monster breathing */
#define BZ_M_BREATH(bztyp) (-20 - (bztyp)) /* -29..-20 */
/* monster shooting a wand; note: not -9 to -0 because -0 is ambiguous  */
#define BZ_M_WAND(bztyp) (-30 - (bztyp))   /* -39..-30 */

/* pick a random entry from array */
#define ROLL_FROM(array) array[rn2(SIZE(array))]
/* array with terminator variation */
/* #define ROLL_FROMT(array) array[rn2(SIZE(array) - 1)] */

/* validate index of array */
#define IndexOk(idx, array) \
    ((idx) >= 0 && (idx) < SIZE(array))
/* array with terminator variation */
#define IndexOkT(idx, array) \
    ((idx) >= 0 && (idx) < (SIZE(array) - 1))

#define FEATURE_NOTICE_VER(major, minor, patch)                    \
    (((unsigned long) major << 24) | ((unsigned long) minor << 16) \
     | ((unsigned long) patch << 8) | ((unsigned long) 0))

#define FEATURE_NOTICE_VER_MAJ (flags.suppress_alert >> 24)
#define FEATURE_NOTICE_VER_MIN \
    (((unsigned long) (0x0000000000FF0000L & flags.suppress_alert)) >> 16)
#define FEATURE_NOTICE_VER_PATCH \
    (((unsigned long) (0x000000000000FF00L & flags.suppress_alert)) >> 8)

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
/**
 * @def plur
 * @brief The plural suffix for a count -- nothing for one, an s otherwise.
 * @param x the count
 * @note Zero takes the plural, which is correct English and is why the test is against one rather than against being greater than one.
 */
/**
 * @def plur
 * @brief 개수에 대한 복수 접미사. 하나면 없음, 아니면 s.
 * @param x 개수
 * @note 0은 복수를 취하며, 그것이 올바른 영어다. 검사가 1보다 큰지가 아니라 1인지에 대한 것인 이유가 그것이다.
 */
#define plur(x) (((x) == 1) ? "" : "s")

/**
 * @def LIMIT_TO_RANGE_INT
 * @brief Clamp a value into a range and yield it as an integer.
 * @param lo the lowest allowed value
 * @param hi the highest allowed value
 * @param var the value to clamp
 * @warning All three arguments are evaluated more than once.
 * @warning Clamps silently. A value far outside the range becomes the nearest bound with no indication that it was ever out of range.
 */
/**
 * @def LIMIT_TO_RANGE_INT
 * @brief 값을 범위 안으로 가두고 정수로 낸다.
 * @param lo 허용되는 최솟값
 * @param hi 허용되는 최댓값
 * @param var 가둘 값
 * @warning 세 인자 모두 한 번 넘게 평가된다.
 * @warning 조용히 가둔다. 범위에서 멀리 벗어난 값은 아무 표시 없이 가장 가까운 경계가 된다.
 */
/* Cast to int, but limit value to range. */
#define LIMIT_TO_RANGE_INT(lo, hi, var) \
    ((int) ((var) < (lo) ? (lo) : (var) > (hi) ? (hi) : (var)))

/**
 * @def ARM_BONUS
 * @brief How much protection a piece of armour actually gives.
 * @param obj the armour
 * @note Its base protection plus its enchantment, less what erosion has taken. The erosion loss is capped at the base protection, so a thoroughly rusted piece of
 *       armour is worth nothing rather than worse than nothing -- but its enchantment still counts.
 * @warning The argument is evaluated several times, and one of the evaluations is a function call.
 */
/**
 * @def ARM_BONUS
 * @brief 갑옷 한 벌이 실제로 주는 방호가 얼마인지.
 * @param obj 그 갑옷
 * @note 기본 방호에 강화를 더하고 삭음이 가져간 것을 뺀다. 삭음으로 인한 손실은 기본 방호까지로 묶여 있으므로, 완전히 녹슨 갑옷은 없는 것보다 나쁜 것이 아니라 그냥 없는 것과 같아진다. 다만 그 강화는 여전히 셈해진다.
 * @warning 인자가 여러 번 평가되며, 그 중 한 번은 함수 호출이다.
 */
#define ARM_BONUS(obj) \
    (objects[(obj)->otyp].a_ac + (obj)->spe                             \
     - min((int) greatest_erosion(obj), objects[(obj)->otyp].a_ac))

/**
 * @def makeknown
 * @brief Identify a kind of object outright.
 * @param x the object kind
 * @note A shorthand for the fullest form of discovery -- name known, seen, and announced. Most identification is partial, which is why the general routine has
 *       several arguments and this names the case where all of them are yes.
 */
/**
 * @def makeknown
 * @brief 어떤 종류의 물건을 완전히 감별한다.
 * @param x 그 물건 종류
 * @note 감별의 가장 온전한 형태 -- 이름을 알고, 보았고, 알려짐 -- 에 대한 줄임이다. 대부분의 감별은 부분적이며, 그래서 일반 루틴이 인자를 여럿 받고 이것이 그 전부가 참인 경우에 이름을 붙인다.
 */
#define makeknown(x) discover_object((x), TRUE, TRUE, TRUE)
/**
 * @name Distance from the hero
 * @brief How far something is from where the hero is standing.
 *
 * @warning The distance is squared, not actual. That is deliberate -- comparing squared distances avoids a square root -- but it means a threshold must be squared
 *          too. Comparing against an unsquared limit gives a much shorter range than intended, which is the standard mistake with these.
 * @note @c onlineu asks a different question: whether the square lies on a straight line from the hero, which is what a beam or a polearm needs.
 * @{
 */
/**
 * @name 영웅으로부터의 거리
 * @brief 무언가가 영웅이 서 있는 곳에서 얼마나 떨어져 있는지.
 *
 * @warning 그 거리는 실제 거리가 아니라 제곱된 거리다. 의도적이다. 제곱된 거리를 비교하면 제곱근을 피할 수 있다. 그런데 그것은 문턱값도 제곱되어야 한다는 뜻이다. 제곱하지 않은 한계와 비교하면 의도한 것보다 훨씬 짧은 범위가 되며, 그것이 이들과
 *          관련된 전형적인 잘못이다.
 * @note @c onlineu 는 다른 질문을 한다. 그 칸이 영웅으로부터의 직선 위에 있는지. 광선이나 장병기가 필요로 하는 것이다.
 * @{
 */
#define distu(xx, yy) dist2((coordxy) (xx), (coordxy) (yy), u.ux, u.uy)
#define mdistu(mon) distu((mon)->mx, (mon)->my)
#define onlineu(xx, yy) online2((coordxy)(xx), (coordxy)(yy), u.ux, u.uy)
/** @} */

/**
 * @def rn1
 * @brief A random number in a range, given as a span and a starting point.
 * @param x how many values are possible
 * @param y the lowest possible value
 * @return a value from @c y to @c y+x-1
 * @warning Not "between x and y". The first argument is a count and the second an offset, which is the opposite of what the name suggests to a reader who has not
 *          checked.
 */
/**
 * @def rn1
 * @brief 어떤 범위의 난수. 폭과 시작점으로 주어진다.
 * @param x 가능한 값의 개수
 * @param y 가능한 최솟값
 * @return @c y 부터 @c y+x-1 까지의 값
 * @warning "x와 y 사이"가 아니다. 첫 인자는 개수이고 둘째는 오프셋이며, 그것은 확인하지 않은 독자에게 그 이름이 시사하는 것과 반대다.
 */
#define rn1(x, y) (rn2(x) + (y))

/**
 * @def AC_VALUE
 * @brief The protection a negative armour class actually provides on a given blow.
 * @param AC the armour class
 * @note Negative armour class is weakened randomly rather than applied in full, and the existing comment gives the reason: applied in full it would make a
 *       sufficiently armoured hero unhittable. So very good armour is very good most of the time rather than always.
 * @warning Not a pure function -- it draws a random number. Calling it twice for one blow gives two different answers.
 */
/**
 * @def AC_VALUE
 * @brief 음수인 방어도가 어떤 타격에 대해 실제로 제공하는 방호.
 * @param AC 그 방어도
 * @note 음수 방어도는 그대로 적용되는 대신 무작위로 약화되며, 기존 주석이 그 이유를 밝힌다. 그대로 적용되면 충분히 무장한 영웅을 맞힐 수 없게 된다. 그래서 아주 좋은 갑옷은 항상이 아니라 대부분의 경우에 아주 좋다.
 * @warning 순수 함수가 아니다. 난수를 뽑는다. 하나의 타격에 대해 두 번 호출하면 서로 다른 두 답이 나온다.
 */
/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC) ((AC) >= 0 ? (AC) : -rnd(-(AC)))

#if defined(MICRO) && !defined(__DJGPP__)
#define getuid() 1
#define getlogin() ((char *) 0)
#endif /* MICRO */

/* These may have been defined to platform-specific values in *conf.h
 * or on the compiler command line from a hints file or Makefile */

#ifndef QSORTCALLBACK
#define QSORTCALLBACK
#endif

#ifndef SIG_RET_TYPE
#define SIG_RET_TYPE void (*)(int)
#endif

/**
 * @name Where to report a problem
 * @brief The upstream project's contact details, shown when the game fails.
 * @warning This is a modified copy of NetHack. A fault introduced here should not be reported to the addresses above; see the fork's own documentation.
 * @{
 */
/**
 * @name 문제를 알릴 곳
 * @brief 원 프로젝트의 연락처. 게임이 실패할 때 보여진다.
 * @warning 이것은 NetHack 의 수정된 사본이다. 여기서 생긴 결함을 위 주소로 알려서는 안 된다. 포크 자체의 문서를 볼 것.
 * @{
 */
#define DEVTEAM_EMAIL "devteam@nethack.org"
#define DEVTEAM_URL "https://www.nethack.org/"
/** @} */

#if !defined(CROSSCOMPILE) || defined(CROSSCOMPILE_TARGET)
#include "nhlua.h"
#endif

/**
 * @note The last three includes come at the end because they depend on everything above: the prototypes name the types declared earlier, the save routines name what
 *       they save, and the globals name their own types.
 * @warning They are omitted for the recovery utility, which links against the game's headers but not its code. Including prototypes it cannot satisfy would leave it
 *          unable to link.
 */
/**
 * @note 마지막 세 포함이 끝에 오는 것은 위의 모든 것에 의존하기 때문이다. 프로토타입들은 앞서 선언된 타입을 지칭하고, 저장 루틴들은 자신이 저장하는 것을 지칭하고, 전역 변수들은 자기 타입을 지칭한다.
 * @warning 복구 도구에서는 이들이 제외된다. 그 도구는 게임의 헤더에 대해 링크하지만 게임의 코드에 대해서는 링크하지 않는다. 만족시킬 수 없는 프로토타입을 포함하면 링크할 수 없게 된다.
 */
#if !defined(RECOVER_C)

#include "extern.h"
#include "savefile.h"
#include "decl.h"

#endif  /* RECOVER_C */

#endif /* HACK_H */

