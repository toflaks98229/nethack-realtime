/* NetHack 5.0	sp_lev.h	$NHDT-Date: 1781973088 2026/06/20 16:31:28 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.62 $ */
/* Copyright (c) 1989 by Jean-Christophe Collet                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file sp_lev.h
 * @brief The vocabulary a level description is written in.
 *
 * Named levels are not built by code but described in a file, and this header is the shape those
 * descriptions take once read. Every type here corresponds to something a level author can write --
 * a room, a monster, an object, a corridor between two rooms -- and holds it in the partly specified
 * state the description left it in.
 *
 * That partial specification is the whole point. A description rarely says "a dagger at column
 * twelve". It says "a weapon somewhere dry in this room", and the fields that are unset are the ones
 * the game is free to choose. So most structures here carry both an explicit value and the means to
 * say "any" -- which is why coordinates are stored packed, with a bit meaning "decide for me" and
 * flags saying what sort of square would do.
 *
 * @note A description can ask for something the level cannot provide -- a dry square on a level that
 *       is all water. Requests therefore have to be able to fail, and one of the humidity flags exists
 *       purely to say that failing quietly is acceptable.
 * @warning The map dimensions here are smaller than the map itself. A described map is inset, because
 *          the outermost row and column must remain available for the surrounding wall.
 */

/**
 * @file sp_lev.h
 * @brief 레벨 기술이 적혀 있는 어휘.
 *
 * 이름 붙은 레벨은 코드가 아니라 파일에 기술되며, 이 헤더는 그 기술이 읽힌 뒤 취하는 형태다. 여기의 모든 타입은 레벨 작성자가 적을 수 있는 것 --
 * 방, 몬스터, 물건, 두 방 사이의 통로 -- 에 대응하고, 그것을 기술이 남겨 둔 부분적으로만 확정된 상태로 담는다.
 *
 * 그 부분적 확정이 요점 전체다. 기술은 "12열에 단검"이라고 말하는 일이 드물다. "이 방 안 마른 어딘가에 무기 하나"라고 말하고, 정해지지 않은
 * 필드들이 게임이 마음대로 고를 수 있는 것들이다. 그래서 여기 대부분의 구조체는 명시적인 값과 "아무거나"라고 말할 수단을 함께 지닌다. 좌표가 "알아서
 * 정하라"를 뜻하는 비트와 어떤 종류의 칸이면 되는지를 말하는 플래그와 함께 압축되어 저장되는 이유다.
 *
 * @note 기술은 그 레벨이 제공할 수 없는 것을 요청할 수 있다. 전부 물인 레벨에서의 마른 칸. 그래서 요청은 실패할 수 있어야 하고, 습도 플래그 하나는
 *       조용히 실패해도 괜찮다고 말하기 위해서만 존재한다.
 * @warning 여기의 지도 크기는 지도 자체보다 작다. 기술된 지도는 안쪽으로 들어와 있다. 맨 바깥의 행과 열이 둘레의 벽을 위해 남아 있어야 하기 때문이다.
 */

#ifndef SP_LEV_H
#define SP_LEV_H

/**
 * @name Wall directions
 * @brief Which side of a room, for placing a door or joining a corridor.
 * @note Bits, so several sides can be offered at once and the game picks among them. @c W_ANY is all
 *       four together, which is how a description says "any wall will do".
 * @warning @c W_RANDOM is not a bit and must not be combined with the others -- it is negative, and
 *          masking it against the direction bits would match every side.
 * @{
 */
/**
 * @name 벽 방향
 * @brief 방의 어느 면인지. 문을 놓거나 통로를 잇기 위한 것.
 * @note 비트이므로 여러 면을 한꺼번에 제시하고 게임이 그 중에서 고를 수 있다. @c W_ANY 는 네 면을 합친 것이며, 기술이 "어느 벽이든 된다"고 말하는
 *       방식이다.
 * @warning @c W_RANDOM 은 비트가 아니며 다른 것들과 결합해서는 안 된다. 음수이고, 그것을 방향 비트와 마스킹하면 모든 면에 걸린다.
 * @{
 */
/* wall directions */
#define W_RANDOM -1
#define W_NORTH 1
#define W_SOUTH 2
#define W_EAST 4
#define W_WEST 8
#define W_ANY (W_NORTH | W_SOUTH | W_EAST | W_WEST)
/** @} */

/**
 * @name Described map limits
 * @brief How large a map a level description may draw.
 * @warning Smaller than the map itself on purpose: the outermost row and column are reserved for the
 *          wall that surrounds the level, so a description that filled the map exactly would leave
 *          nowhere for it.
 * @{
 */
/**
 * @name 기술된 지도의 한계
 * @brief 레벨 기술이 그릴 수 있는 지도의 크기.
 * @warning 의도적으로 지도 자체보다 작다. 맨 바깥의 행과 열이 레벨을 둘러싸는 벽을 위해 남겨져 있으므로, 지도를 꼭 맞게 채운 기술은 그 벽이 놓일
 *          자리를 남기지 않는다.
 * @{
 */
/* MAP limits */
#define MAP_X_LIM 76
#define MAP_Y_LIM 21
/** @} */

/**
 * @name Level rules
 * @brief Rules that hold for a whole level, set by its description.
 *
 * These are not properties of terrain but exceptions to the game's ordinary rules, declared once for
 * the level: teleportation does not work here, the floor cannot be dug, magic mapping fails, sight
 * does not reach far.
 *
 * @note @c PREMAPPED does two things at once -- the level starts known and Sokoban's rules apply -- so
 *       it cannot be used merely to reveal a map.
 * @note @c CHECK_INACCESSIBLES asks the generator to verify its own work and open a way out of any
 *       sealed area, which is needed because a described map plus random filling can produce a region
 *       nothing can reach.
 * @{
 */
/**
 * @name 레벨 규칙
 * @brief 레벨 전체에 적용되는 규칙. 그 기술이 정한다.
 *
 * 이들은 지형의 속성이 아니라 게임의 통상 규칙에 대한 예외이며, 레벨마다 한 번 선언된다. 여기서는 순간이동이 통하지 않는다, 바닥을 팔 수 없다, 마법
 * 지도가 실패한다, 시야가 멀리 닿지 않는다.
 *
 * @note @c PREMAPPED 는 두 가지를 동시에 한다. 레벨이 이미 알려진 상태로 시작하고 소코반의 규칙이 적용된다. 그래서 단지 지도를 드러내기 위해 쓸 수는
 *       없다.
 * @note @c CHECK_INACCESSIBLES 는 생성기에게 자기 작업을 검증하고 봉쇄된 영역에서 나갈 길을 내라고 요청한다. 기술된 지도에 무작위 채움이 겹치면 아무도
 *       닿을 수 없는 영역이 나올 수 있어 필요하다.
 * @{
 */
/* Per level flags */
#define NOTELEPORT 0x00000001L
#define HARDFLOOR 0x00000002L
#define NOMMAP 0x00000004L
#define SHORTSIGHTED 0x00000008L
#define ARBOREAL 0x00000010L
#define MAZELEVEL 0x00000020L
#define PREMAPPED 0x00000040L /* premapped level & sokoban rules */
#define SHROUD 0x00000080L
#define GRAVEYARD 0x00000100L
#define ICEDPOOLS 0x00000200L /* for ice locations: ICED_POOL vs ICED_MOAT \
                                 */
#define SOLIDIFY 0x00000400L  /* outer areas are nondiggable & nonpasswall */
#define CORRMAZE 0x00000800L  /* for maze levels only */
#define CHECK_INACCESSIBLES 0x00001000L /* check for inaccessible areas and
   generate ways to escape from them */
/** @} */

/**
 * @brief What the level starts out as, before the description modifies it.
 *
 * A description does not usually draw every square. It starts from one of these -- solid rock, a maze,
 * a swamp, the irregular caves of the mines -- and then places what it cares about on top.
 *
 * @note @c LVLINIT_NONE leaves the map as it was, for descriptions that draw the whole thing
 *       themselves.
 */
/**
 * @brief 기술이 손대기 전, 레벨이 처음에 무엇인지.
 *
 * 기술이 모든 칸을 그리는 일은 보통 없다. 이들 중 하나 -- 단단한 암반, 미로, 늪, 광산의 불규칙한 굴 -- 에서 시작해 그 위에 자기가 신경 쓰는 것을 놓는다.
 *
 * @note @c LVLINIT_NONE 은 지도를 그대로 둔다. 전체를 직접 그리는 기술을 위한 것이다.
 */
/* different level layout initializers */
enum lvlinit_types {
    LVLINIT_NONE = 0,
    LVLINIT_SOLIDFILL,
    LVLINIT_MAZEGRID,
    LVLINIT_MAZE,
    LVLINIT_MINES,
    LVLINIT_ROGUE,
    LVLINIT_SWAMP
};

/**
 * @def MAX_NESTED_ROOMS
 * @brief How deeply a room may contain another room.
 * @note A limit rather than an arbitrary bound: rooms within rooms are placed recursively, and the
 *       nesting stack in the coder below is sized from this.
 */
/**
 * @def MAX_NESTED_ROOMS
 * @brief 방이 다른 방을 얼마나 깊이 품을 수 있는지.
 * @note 임의의 상한이 아니라 한계다. 방 안의 방은 재귀적으로 배치되며, 아래 코더의 중첩 스택 크기가 이 값에서 나온다.
 */
/* max. nested depth of subrooms */
#define MAX_NESTED_ROOMS 5

/**
 * @name An object's place in a container
 * @brief Whether a described object is a container, something inside one, or both.
 * @note Both at once is meaningful -- a box inside a chest -- which is why these are bits rather than a
 *       choice.
 * @{
 */
/**
 * @name 물건이 용기 안에서 갖는 위치
 * @brief 기술된 물건이 용기인지, 용기 안의 것인지, 둘 다인지.
 * @note 둘 다인 것이 의미가 있다. 상자 안의 함. 그래서 이들은 택일이 아니라 비트다.
 * @{
 */
/* When creating objects, we need to know whether
 * it's a container and/or contents.
 */
#define SP_OBJ_CONTENT 0x1
#define SP_OBJ_CONTAINER 0x2
/** @} */

/**
 * @name Narrowing a set of squares
 * @brief How a description thins out a group of squares it has selected.
 *
 * A description works with sets of squares, and often wants only some of them -- a scattering rather
 * than the whole area. These say on what basis to discard the rest: at random by proportion, by
 * intersecting with another set, or by what terrain each square holds.
 * @{
 */
/**
 * @name 칸의 묶음을 좁히기
 * @brief 기술이 골라 둔 칸의 묶음을 어떻게 솎아 내는지.
 *
 * 기술은 칸의 묶음을 다루며, 종종 그 중 일부만 원한다. 영역 전체가 아니라 흩뿌려진 것. 이들은 나머지를 무엇을 근거로 버릴지 말한다. 비율에 따라
 * 무작위로, 다른 묶음과 교차시켜서, 또는 각 칸이 어떤 지형인지에 따라.
 * @{
 */
/* SPO_FILTER types */
#define SPOFILTER_PERCENT 0
#define SPOFILTER_SELECTION 1
#define SPOFILTER_MAPCHAR 2
/** @} */

/**
 * @name Gradient shapes
 * @brief How a selection's density falls off with distance from a point.
 * @note Used to scatter something thickly at a centre and sparsely away from it, which is what makes a
 *       described area look grown rather than drawn.
 * @{
 */
/**
 * @name 기울기 모양
 * @brief 선택의 밀도가 어떤 점에서 멀어질수록 어떻게 줄어드는지.
 * @note 중심에 빽빽하고 멀어질수록 드물게 흩뿌리는 데 쓴다. 기술된 영역이 그려진 것이 아니라 자라난 것처럼 보이게 만드는 것이다.
 * @{
 */
/* gradient filter types */
#define SEL_GRADIENT_RADIAL 0
#define SEL_GRADIENT_SQUARE 1
/** @} */

/**
 * @def SP_COORD_IS_RANDOM
 * @brief The mark that a packed coordinate is a request rather than a position.
 * @note Set high, above the two packed bytes, so a coordinate and a request can share one field and be
 *       told apart. When it is set, the low bits hold the humidity flags instead of a position.
 */
/**
 * @def SP_COORD_IS_RANDOM
 * @brief 압축된 좌표가 위치가 아니라 요청임을 나타내는 표시.
 * @note 압축된 두 바이트보다 위쪽에 놓여 있어, 좌표와 요청이 하나의 필드를 공유하면서 구별될 수 있다. 이것이 켜져 있으면 아래쪽 비트는 위치가 아니라
 *       습도 플래그를 담는다.
 */
#define SP_COORD_IS_RANDOM 0x01000000L
/**
 * @name Humidity flags
 * @brief What sort of square would satisfy a request for a random position.
 *
 * A description asking for "somewhere" almost always means somewhere of a kind: dry ground for a
 * monster, water for an eel, solid rock for a vault. These are how that kind is stated.
 *
 * @note @c SPACELOC is @c DRY widened to accept furniture, for things that may stand on an altar or a
 *       fountain rather than needing bare floor.
 * @warning @c ANY_LOC accepts positions outside the level, so a caller using it must not assume the
 *          result is on the map.
 * @warning A request can fail. Without @c NO_LOC_WARN failure is reported; with it the position comes
 *          back as -1,-1 and nothing is said -- so a caller that passes it must check.
 * @{
 */
/**
 * @name 습도 플래그
 * @brief 무작위 위치 요청을 만족시킬 칸이 어떤 종류인지.
 *
 * "어딘가"를 요청하는 기술은 거의 언제나 어떤 종류의 어딘가를 뜻한다. 몬스터를 위한 마른 땅, 뱀장어를 위한 물, 금고를 위한 단단한 암반. 이들이 그 종류를
 * 말하는 방식이다.
 *
 * @note @c SPACELOC 은 @c DRY 를 설비까지 받아들이도록 넓힌 것이다. 맨 바닥이 필요한 것이 아니라 제단이나 분수 위에 서도 되는 것들을 위한 것이다.
 * @warning @c ANY_LOC 은 레벨 밖의 위치도 받아들인다. 그래서 그것을 쓰는 호출자는 결과가 지도 위에 있다고 가정해서는 안 된다.
 * @warning 요청은 실패할 수 있다. @c NO_LOC_WARN 없이는 실패가 보고되고, 그것이 있으면 위치가 -1,-1 로 돌아오며 아무 말도 없다. 그래서 그것을 넘기는
 *          호출자는 반드시 확인해야 한다.
 * @{
 */
/* Humidity flags for get_location() and friends, used with
 * SP_COORD_PACK_RANDOM() */
#define DRY         0x01
#define WET         0x02
#define HOT         0x04
#define SOLID       0x08
#define ANY_LOC     0x10 /* even outside the level */
#define NO_LOC_WARN 0x20 /* no complaints and set x & y to -1, if no loc */
#define SPACELOC    0x40 /* like DRY, but accepts furniture too */
/** @} */

/**
 * @name Where a described monster's possessions come from
 * @brief Whether the description supplies them, the game does, or neither.
 * @note Three cases rather than two, because "nothing" and "whatever this kind normally carries" are
 *       both wanted -- a soldier described without equipment should still have its own, but a monster
 *       meant to be empty-handed must stay that way.
 * @{
 */
/**
 * @name 기술된 몬스터의 소지품이 어디서 오는지
 * @brief 기술이 주는지, 게임이 주는지, 아니면 아무도 주지 않는지.
 * @note 둘이 아니라 세 경우다. "아무것도 없음"과 "이 종류가 보통 지니는 것"이 둘 다 필요하기 때문이다. 장비 없이 기술된 병사는 여전히 자기 장비를
 *       가져야 하지만, 빈손이도록 의도된 몬스터는 그대로 있어야 한다.
 * @{
 */
/* has_invent flags */
#define NO_INVENT 0         /* monster doesn't get any invent */
#define CUSTOM_INVENT 0x01  /* monster gets items specified in lua */
#define DEFAULT_INVENT 0x02 /* monster gets items from makemon() */
/** @} */

/**
 * @name Packed coordinates
 * @brief Putting a position, or a request for one, into a single value.
 *
 * A described position is either an actual square or a description of what square would do, and the two
 * must travel in one field so that every place expecting a position can take either. So the two
 * coordinate bytes are packed with a gap between them, and the request marker sits above both.
 *
 * @warning @c SP_COORD_X and @c SP_COORD_Y do not parenthesise their argument, so an expression passed
 *          to them may be parsed unexpectedly. Pass a variable.
 * @warning Unpacking a value that carries the random marker yields the humidity flags read as
 *          coordinates -- small, plausible, and wrong. The marker must be tested first.
 * @{
 */
/**
 * @name 압축된 좌표
 * @brief 위치, 또는 위치에 대한 요청을 하나의 값에 담기.
 *
 * 기술된 위치는 실제 칸이거나 어떤 칸이면 되는지에 대한 기술이며, 둘이 하나의 필드로 다녀야 한다. 그래야 위치를 기대하는 모든 곳이 어느 쪽이든 받을 수
 * 있다. 그래서 두 좌표 바이트가 사이에 틈을 두고 압축되고, 요청 표시가 둘보다 위에 놓인다.
 *
 * @warning @c SP_COORD_X 와 @c SP_COORD_Y 는 인자를 괄호로 감싸지 않는다. 그래서 이들에게 넘긴 수식이 예상과 다르게 해석될 수 있다. 변수를 넘길 것.
 * @warning 무작위 표시를 담은 값을 풀면 습도 플래그가 좌표로 읽힌다. 작고, 그럴듯하고, 틀리다. 표시를 먼저 검사해야 한다.
 * @{
 */
#define SP_COORD_X(l) (l & 0xff)
#define SP_COORD_Y(l) ((l >> 16) & 0xff)
#define SP_COORD_PACK(x, y) (((x) & 0xff) + (((y) & 0xff) << 16))
#define SP_COORD_PACK_RANDOM(f) (SP_COORD_IS_RANDOM | (f))
/** @} */

/**
 * @brief The state carried while a level description is being carried out.
 *
 * A description is executed rather than read: it places a room, then places things inside that room, so
 * "the current room" has to be known and rooms nest. Hence the stack.
 *
 * @note Failed rooms are remembered per nesting level, so that things described inside a room that could
 *       not be placed are skipped rather than placed loose on the level.
 * @note Whether the level may be mirrored is held here because flipping has to be decided once for the
 *       whole level, after everything is placed -- a level flipped halfway through would contradict
 *       itself.
 */
/**
 * @brief 레벨 기술이 수행되는 동안 지니는 상태.
 *
 * 기술은 읽히는 것이 아니라 실행된다. 방을 놓고, 그다음 그 방 안에 물건을 놓는다. 그래서 "현재 방"이 알려져 있어야 하고 방은 중첩된다. 스택이 있는
 * 이유다.
 *
 * @note 실패한 방이 중첩 단계마다 기억된다. 그래서 놓일 수 없었던 방 안에 기술된 것들이 레벨에 낱개로 놓이는 대신 건너뛰어진다.
 * @note 레벨을 거울처럼 뒤집어도 되는지가 여기에 보관되는 것은, 뒤집기가 모든 것이 놓인 뒤 레벨 전체에 대해 한 번 결정되어야 하기 때문이다. 도중에
 *       뒤집힌 레벨은 스스로 모순된다.
 */
struct sp_coder {
    int premapped;
    boolean solidify;
    struct mkroom *croom;
    int room_stack;
    struct mkroom *tmproomlist[MAX_NESTED_ROOMS + 1];
    boolean failed_room[MAX_NESTED_ROOMS + 1];
    int n_subroom;
    boolean lvl_is_joined;
    boolean check_inaccessibles;
    int allow_flips;
};

/*
 * Structures manipulated by the special levels loader & compiler
 */

/**
 * @def packed_coord
 * @brief A position or a request for one, in a single value.
 * @note A macro rather than a typedef, so it can be a plain integer everywhere without the compiler
 *       distinguishing it -- which is what lets it be written into descriptions and read back
 *       arithmetically.
 */
/**
 * @def packed_coord
 * @brief 위치, 또는 위치에 대한 요청. 하나의 값으로.
 * @note typedef 가 아니라 매크로다. 그래서 어디서나 평범한 정수일 수 있고 컴파일러가 그것을 구별하지 않는다. 기술에 적히고 산술적으로 되읽힐 수 있게
 *       만드는 것이 그것이다.
 */
#define packed_coord long
typedef uint32_t getloc_flags_t;
/**
 * @brief A packed coordinate taken apart, so its two readings are separate fields.
 * @note Exists so that code which has already tested the random marker does not have to keep testing it.
 *       If @c is_random is set the coordinates mean nothing and the flags do; otherwise the reverse.
 */
/**
 * @brief 압축된 좌표를 풀어 놓은 것. 두 해석이 각각의 필드가 된다.
 * @note 무작위 표시를 이미 검사한 코드가 그것을 계속 검사하지 않아도 되도록 존재한다. @c is_random 이 켜져 있으면 좌표는 아무 의미가 없고 플래그가
 *       의미를 가지며, 아니면 그 반대다.
 */
typedef struct {
    xint16 is_random;
    getloc_flags_t getloc_flags;
    int x, y;
} unpacked_coord;

/**
 * @brief How a level begins: its starting layout and the rules that hold on it.
 *
 * Gathers everything a description says before placing anything -- what to fill the map with, how wide
 * corridors are, how thick walls are, whether the place is lit, whether dead ends are removed.
 *
 * @note @c init_present distinguishes a description that said nothing about initialisation from one that
 *       asked for nothing, which are different: the first should inherit a default.
 */
/**
 * @brief 레벨이 어떻게 시작하는지. 그 초기 배치와 그곳에 적용되는 규칙.
 *
 * 기술이 무엇을 놓기 전에 말하는 모든 것을 모은다. 지도를 무엇으로 채울지, 통로가 얼마나 넓은지, 벽이 얼마나 두꺼운지, 그곳에 불이 켜져 있는지, 막다른
 * 길을 없앨지.
 *
 * @note @c init_present 는 초기화에 대해 아무 말도 하지 않은 기술과 아무것도 요청하지 않은 기술을 구별한다. 둘은 다르다. 앞의 것은 기본값을 물려받아야
 *       한다.
 */
typedef struct {
    xint16 init_style; /* one of LVLINIT_foo */
    long flags;
    schar filling;
    boolean init_present, padding;
    char fg, bg;
    boolean smoothed, joined;
    xint16 lit, walled;
    boolean icedpools;
    int corrwid, wallthick;
    boolean rm_deadends;
} lev_init;

/**
 * @brief A door in a room's wall, described by which wall rather than by position.
 * @note The position is an offset along the chosen wall, so the same description works whatever size the
 *       room turns out to be.
 */
/**
 * @brief 방 벽의 문. 위치가 아니라 어느 벽인지로 기술된다.
 * @note 위치는 고른 벽을 따라가는 오프셋이다. 그래서 방이 어떤 크기로 정해지든 같은 기술이 통한다.
 */
typedef struct {
    xint16 wall, pos, secret, mask;
} room_door;

/**
 * @brief A trap the description asks for.
 * @note @c novictim exists because some traps are placed already sprung, or placed where the description
 *       does not want the usual accompanying corpse -- so the trap can be present without the story of
 *       someone having triggered it.
 * @note A web may be described as already having its spider, since a web without one reads as abandoned.
 */
/**
 * @brief 기술이 요청하는 함정.
 * @note @c novictim 이 있는 것은, 어떤 함정은 이미 발동된 상태로 놓이거나 기술이 딸려 오는 시체를 원하지 않는 곳에 놓이기 때문이다. 그래서 누군가
 *       걸렸다는 이야기 없이 함정만 존재할 수 있다.
 * @note 거미줄은 이미 거미가 있는 상태로 기술될 수 있다. 거미 없는 거미줄은 버려진 것으로 읽히기 때문이다.
 */
typedef struct {
    packed_coord coord;
    coordxy x, y;
    xint16 type;
    boolean spider_on_web;
    boolean seen;
    boolean novictim;
} spltrap;

/**
 * @brief A monster the description asks for, in whatever detail it chose to give.
 *
 * Nearly every field is optional, and the many small state fields exist because a described monster is
 * often placed mid-situation: asleep, already fleeing, blinded, waiting. A level's story is told partly
 * by the state its inhabitants start in.
 *
 * @note @c appear_as lets a monster be placed already disguised, which is how a mimic can be part of a
 *       described scene rather than a surprise the game invented.
 * @note @c m_lev_adj is an adjustment rather than a level, so a description can ask for a tougher example
 *       of a kind without restating what that kind normally is.
 * @note @c seentraps records which traps this monster already knows about, so a guardian is not caught by
 *       the trap it stands beside.
 */
/**
 * @brief 기술이 요청하는 몬스터. 기술이 주기로 한 만큼의 세부와 함께.
 *
 * 거의 모든 필드가 선택적이며, 작은 상태 필드가 많은 것은 기술된 몬스터가 종종 상황 중간에 놓이기 때문이다. 잠들어 있고, 이미 달아나고 있고, 눈이 멀었고,
 * 기다리고 있다. 레벨의 이야기는 부분적으로 그 거주자들이 어떤 상태로 시작하는지로 이야기된다.
 *
 * @note @c appear_as 는 몬스터를 이미 위장한 상태로 놓게 해 준다. 모방자가 게임이 만들어 낸 놀라움이 아니라 기술된 장면의 일부가 될 수 있는 방식이다.
 * @note @c m_lev_adj 는 레벨이 아니라 조정값이다. 그래서 기술이 그 종류가 보통 어떤지 다시 말하지 않고도 더 강한 개체를 요청할 수 있다.
 * @note @c seentraps 는 이 몬스터가 이미 아는 함정을 기록한다. 그래서 수호자가 자기 옆에 서 있는 함정에 걸리지 않는다.
 */
typedef struct {
    Str_or_Len name, appear_as;
    short id;
    unsigned int sp_amask; /* splev amask */
    packed_coord coord;
    coordxy x, y;
    xint16 class, appear;
    schar peaceful, asleep;
    short female, invis, cancelled, revived, avenge, fleeing, blinded,
        paralyzed, stunned, confused, waiting;
    short m_lev_adj;
    long seentraps;
    short has_invent;
    mmflags_nht mm_flags; /* makemon flags */
} monster;

/**
 * @brief An object the description asks for.
 * @note Like a described monster, mostly optional. The condition fields -- eroded, locked, trapped,
 *       greased, broken -- let an object carry a history, which is how a described room can look lived in.
 * @note @c achievement marks an object whose being taken is itself an accomplishment, so the placement and
 *       the accomplishment are described together and cannot drift apart.
 * @warning @c class and @c id overlap in purpose: a description may name an exact kind or only a class and
 *          leave the kind to be chosen.
 */
/**
 * @brief 기술이 요청하는 물건.
 * @note 기술된 몬스터처럼 대부분 선택적이다. 상태 필드들 -- 삭음, 잠김, 함정, 기름칠, 부서짐 -- 은 물건이 이력을 지니게 해 준다. 기술된 방이 사람이 살던
 *       곳처럼 보일 수 있는 방식이다.
 * @note @c achievement 는 그것을 얻는 것 자체가 업적인 물건을 표시한다. 그래서 배치와 업적이 함께 기술되고 서로 어긋날 수 없다.
 * @warning @c class 와 @c id 는 용도가 겹친다. 기술은 정확한 종류를 지목할 수도, 계열만 말하고 종류는 골라지도록 남겨 둘 수도 있다.
 */
typedef struct {
    Str_or_Len name;
    int corpsenm;
    short id, spe;
    packed_coord coord;
    coordxy x, y;
    xint16 class, containment;
    schar curse_state;
    int quan;
    short buried;
    short lit;
    short eroded, locked, trapped, tknown, recharged, invis, greased, broken,
          achievement;
} object;

/**
 * @brief An altar, and whether it is merely an altar or the centre of a temple.
 * @note The alignment is a mask rather than a value so a description can ask for "any" or for one matching
 *       something else, instead of naming a god the level should not presume.
 */
/**
 * @brief 제단. 그리고 그것이 단지 제단인지 신전의 중심인지.
 * @note 진영이 값이 아니라 마스크다. 그래서 기술이 레벨이 단정해서는 안 되는 신을 지목하는 대신 "아무거나"나 다른 것과 일치하는 것을 요청할 수 있다.
 */
typedef struct {
    packed_coord coord;
    coordxy x, y;
    unsigned int sp_amask; /* splev amask */
    xint16 shrine;
} altar;

/**
 * @brief An area of the level given a purpose: a shop, a temple, a zoo, a barracks.
 * @note A region is not a room. It is a rectangle declared to be of a kind, which is how a described map --
 *       one with no rooms in the generator's sense -- can still contain a shop.
 * @note @c rirreg marks a region that is not really rectangular, in which case the rectangle is only its
 *       bounds and the actual shape comes from the map.
 */
/**
 * @brief 용도가 주어진 레벨의 영역. 상점, 신전, 동물원, 병영.
 * @note 영역은 방이 아니다. 어떤 종류라고 선언된 사각형이며, 그래서 기술된 지도 -- 생성기가 말하는 의미의 방이 없는 지도 -- 도 상점을 담을 수 있다.
 * @note @c rirreg 는 실제로 사각형이 아닌 영역을 표시한다. 그 경우 사각형은 그 경계일 뿐이고 실제 모양은 지도에서 온다.
 */
typedef struct {
    coordxy x1, y1, x2, y2;
    xint16 rtype, rlit, rirreg;
} region;

/**
 * @brief A kind of ground to write onto squares, and whether those squares are lit.
 * @note Lighting travels with the terrain because they are written together; a described pool that is dark
 *       is one operation, not two.
 */
/**
 * @brief 칸에 써 넣을 땅의 종류와, 그 칸에 불이 켜져 있는지.
 * @note 조명이 지형과 함께 다니는 것은 둘이 같이 써지기 때문이다. 어두운 물웅덩이를 기술하는 것은 두 번의 연산이 아니라 한 번이다.
 */
typedef struct {
    xint16 ter, tlit;
} terrain;

/**
 * @brief A corridor to dig between two rooms, named by room and wall rather than by route.
 * @note Only the ends are described; the path between them is found. That is why a description can join two
 *       rooms without knowing where the generator will put either.
 */
/**
 * @brief 두 방 사이에 파낼 통로. 경로가 아니라 방과 벽으로 지목된다.
 * @note 양 끝만 기술되고 그 사이의 길은 찾아진다. 그래서 기술이 생성기가 두 방을 어디에 놓을지 모르고도 그것들을 이을 수 있다.
 */
typedef struct {
    struct {
        xint16 room;
        xint16 wall;
        xint16 door;
    } src, dest;
} corridor;

/**
 * @brief A room the description asks for, positioned by where it belongs rather than by coordinates.
 *
 * A described room usually gives an alignment -- toward a side, or centred -- and a size, and lets the
 * generator find room for it. That is what allows one description to produce a different-looking level each
 * game while still containing what it said it would.
 *
 * @note The parent is named rather than pointed at, since the description is text and a room may be written
 *       before the room that contains it.
 * @note @c chance lets a room be optional, so a description can offer a room that is sometimes simply not
 *       there.
 * @note @c needfill separates the room's existence from its contents: a room may be described and then
 *       populated by the ordinary generator rather than by hand.
 */
/**
 * @brief 기술이 요청하는 방. 좌표가 아니라 어디에 속하는지로 위치가 정해진다.
 *
 * 기술된 방은 보통 정렬 -- 어느 쪽으로, 또는 가운데 -- 과 크기를 주고, 생성기가 그것이 들어갈 자리를 찾게 한다. 그것이 하나의 기술이 게임마다 다르게 보이는
 * 레벨을 만들면서도 자기가 말한 것을 여전히 담게 하는 것이다.
 *
 * @note 부모가 포인터가 아니라 이름으로 지목된다. 기술이 텍스트이고, 어떤 방이 그것을 품는 방보다 먼저 적힐 수 있기 때문이다.
 * @note @c chance 는 방을 선택적으로 만든다. 그래서 기술이 때로는 그냥 없는 방을 제시할 수 있다.
 * @note @c needfill 은 방의 존재와 그 내용물을 분리한다. 방이 기술된 뒤 손으로가 아니라 평범한 생성기에 의해 채워질 수 있다.
 */
typedef struct _room {
    Str_or_Len name;
    Str_or_Len parent;
    coordxy x, y;
    xint16 w, h;
    xint16 xalign, yalign;
    xint16 rtype, chance, rlit, needfill;
    boolean joined;
} room;

/**
 * @brief A rectangle of map drawn as characters, to be stamped onto a level.
 * @note Stored as characters rather than terrain so a fragment stays readable in the description it came
 *       from; the translation to terrain happens when it is placed.
 * @warning The dimensions are carried separately because the data has no row terminators -- it is one run
 *          of characters, and the width is the only thing that says where a row ends.
 */
/**
 * @brief 문자로 그려진 지도의 사각형. 레벨 위에 찍어 넣기 위한 것.
 * @note 지형이 아니라 문자로 저장된다. 그래서 조각이 그것이 온 기술 안에서 읽을 수 있는 상태로 남는다. 지형으로의 번역은 그것이 놓일 때 일어난다.
 * @warning 크기가 따로 담기는 것은 데이터에 행 종결자가 없기 때문이다. 하나의 문자열이며, 행이 어디서 끝나는지 말해 주는 것은 너비뿐이다.
 */
struct mapfragment {
    int wid, hei;
    char *data;
};

#endif /* SP_LEV_H */
