/* NetHack 5.0	dungeon.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.56 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file dungeon.h
 * @brief How the dungeon is put together, and what the hero remembers of it.
 *
 * The dungeon is not one stack of levels but several, joined at branches -- the mines leave
 * the main dungeon and end, the quest hangs off a portal, the endgame is elsewhere entirely.
 * So a level is never identified by a single number. It is identified by which dungeon it is
 * in and how deep within that dungeon, and the two must travel together.
 *
 * Three different numbers describe "how deep", and confusing them is the classic mistake
 * here. The level number is relative to its own dungeon and starts again at one for each.
 * Depth is how far below the surface, which is what the player is shown and what difficulty
 * scales with. The ledger number is a unique identifier used for naming save files, and means
 * nothing to the player. Each dungeon carries the two bases needed to convert its level
 * numbers into the other two.
 *
 * The second half of the file is a different thing entirely: what the player knows. The game
 * keeps a per-level summary -- "a fountain, a shop, a temple" -- so that the overview screen
 * can answer the questions a player would otherwise keep notes for. That record is
 * deliberately vague and deliberately fallible, because it stands for the hero's memory
 * rather than the map.
 *
 * @note The special levels are reached through named tests rather than by number, so a level's
 *       identity survives the dungeon being laid out differently.
 * @warning A level identifier of all zeroes is not level zero of dungeon zero -- it is an
 *          identifier that has not been assigned. That is why the tests check for assignment
 *          before comparing.
 */

/**
 * @file dungeon.h
 * @brief 던전이 어떻게 짜여 있는지, 그리고 영웅이 그것을 어떻게 기억하는지.
 *
 * 던전은 레벨 하나의 더미가 아니라 여러 개의 더미가 분기에서 이어진 것이다. 광산은 본 던전에서 갈라져 나가 끝나고, 퀘스트는
 * 포탈에 매달려 있고, 종반부는 아예 다른 곳에 있다. 그래서 레벨은 결코 하나의 숫자로 식별되지 않는다. 어느 던전에 있고 그
 * 던전 안에서 얼마나 깊은지로 식별되며, 그 둘은 함께 다녀야 한다.
 *
 * "얼마나 깊은지"를 기술하는 숫자가 세 가지이고, 그것들을 혼동하는 것이 여기서의 고전적인 잘못이다. 레벨 번호는 자기 던전에
 * 상대적이며 던전마다 다시 1에서 시작한다. 깊이는 지표 아래로 얼마나 내려왔는지이며, 플레이어에게 보여지는 것이고 난이도가
 * 그것에 따라 오른다. 원장 번호는 저장 파일 이름을 짓는 데 쓰이는 고유 식별자이고 플레이어에게는 아무 의미가 없다. 각 던전은
 * 자기 레벨 번호를 나머지 둘로 바꾸는 데 필요한 두 기준값을 지닌다.
 *
 * 이 파일의 후반부는 전혀 다른 것이다. 플레이어가 아는 것. 게임은 레벨별 요약 -- "분수 하나, 상점 하나, 신전 하나" -- 을
 * 보관하며, 그래서 개요 화면이 플레이어가 그러지 않으면 따로 적어 두어야 할 질문에 답할 수 있다. 그 기록은 의도적으로
 * 모호하고 의도적으로 틀릴 수 있다. 지도가 아니라 영웅의 기억을 나타내기 때문이다.
 *
 * @note 특수 레벨들은 번호가 아니라 이름 붙은 검사로 접근된다. 그래서 던전이 다르게 배치되어도 레벨의 정체가 살아남는다.
 * @warning 전부 0인 레벨 식별자는 0번 던전의 0번 레벨이 아니라 아직 배정되지 않은 식별자다. 그래서 검사들이 비교하기 전에
 *          배정 여부를 확인한다.
 */

#ifndef DUNGEON_H
#define DUNGEON_H

/**
 * @brief Which level, anywhere in the dungeon.
 *
 * Two numbers rather than one because the level number alone is ambiguous -- there is a level
 * five in the mines and a level five in the main dungeon. This pair is the only complete way
 * to say where a level is, which is why it is passed around whole rather than as an integer.
 *
 * @warning Both zero means unassigned, not a valid level.
 */
/**
 * @brief 던전 어디든의 어느 레벨인지.
 *
 * 하나가 아니라 두 숫자인 것은 레벨 번호만으로는 모호하기 때문이다. 광산에도 5층이 있고 본 던전에도 5층이 있다. 이 짝이
 * 레벨이 어디에 있는지 말하는 유일한 완전한 방법이며, 그래서 정수가 아니라 통째로 전달된다.
 *
 * @warning 둘 다 0이면 유효한 레벨이 아니라 배정되지 않았음을 뜻한다.
 */
typedef struct d_level { /* basic dungeon level element */
    xint16 dnum;          /* dungeon number */
    xint16 dlevel;        /* level number */
} d_level;

#if !defined(MAKEDEFS_C) && !defined(MDLIB_C)

/**
 * @brief What kind of place a dungeon or a level is.
 *
 * One set of flags serves both, because most of the questions apply at either scale: a whole
 * dungeon can be mazelike, or a single level can be. A dungeon's flags are the default for its
 * levels and a level may differ.
 *
 * @note @c town applies only to levels, as the existing comment notes -- there is no dungeon
 *       that is entirely a town.
 * @note @c rogue_like changes only presentation, not generation. It is what makes one level
 *       imitate the game NetHack descends from.
 * @note @c align is three bits rather than one because it holds an alignment, and alignment has
 *       a "none" as well as three values.
 */
/**
 * @brief 던전이나 레벨이 어떤 종류의 곳인지.
 *
 * 하나의 플래그 묶음이 둘 다를 맡는다. 대부분의 질문이 어느 규모에서든 성립하기 때문이다. 던전 전체가 미로 같을 수도, 레벨
 * 하나가 그럴 수도 있다. 던전의 플래그는 그 레벨들의 기본값이고, 레벨은 그것과 다를 수 있다.
 *
 * @note @c town 은 기존 주석이 밝히듯 레벨에만 적용된다. 전체가 마을인 던전은 없다.
 * @note @c rogue_like 는 생성이 아니라 표현만 바꾼다. 어떤 레벨이 NetHack 이 물려받은 그 게임을 모방하게 만드는 것이다.
 * @note @c align 이 1비트가 아니라 3비트인 것은 진영을 담기 때문이다. 진영에는 세 값 말고 "없음"도 있다.
 */
typedef struct d_flags {     /* dungeon/level type flags */
    Bitfield(town, 1);       /* is this a town? (levels only) */
    Bitfield(hellish, 1);    /* is this part of hell? */
    Bitfield(maze_like, 1);  /* is this a maze? */
    Bitfield(rogue_like, 1); /* is this an old-fashioned presentation? */
    Bitfield(align, 3);      /* dungeon alignment. */
    Bitfield(unconnected, 1); /* dungeon not connected to any branch */
} d_flags;

/**
 * @brief A level that is built from a description rather than generated.
 *
 * Named levels -- the Oracle, the Castle, Medusa's island -- are laid out by a file rather than
 * randomly, and the game has to know at what depth to place each one. That placement is not
 * fixed: several similar descriptions may exist and one is chosen, so where a special level
 * lands varies between games while the fact that it exists does not.
 *
 * @note The bones identifier is a single character because bones files are named by it, and it
 *       must stay stable so that a bones file from an older game is still recognised as
 *       belonging to this level.
 */
/**
 * @brief 무작위 생성이 아니라 기술로부터 지어지는 레벨.
 *
 * 이름 붙은 레벨들 -- 오라클, 성, 메두사의 섬 -- 은 무작위가 아니라 파일로 배치되며, 게임은 각각을 어느 깊이에 놓을지 알아야
 * 한다. 그 배치는 고정되어 있지 않다. 비슷한 기술이 여럿 있고 그 중 하나가 골라지므로, 특수 레벨이 어디에 놓이는지는 게임마다
 * 달라지고 그것이 존재한다는 사실은 달라지지 않는다.
 *
 * @note 유골 식별자가 문자 하나인 것은 유골 파일 이름이 그것으로 지어지기 때문이다. 그리고 예전 게임의 유골 파일이 여전히 이
 *       레벨에 속하는 것으로 인식되도록 그 값이 안정되어야 한다.
 */
typedef struct s_level { /* special dungeon level element */
    struct s_level *next;
    d_level dlevel; /* dungeon & level numbers */
    char proto[15]; /* name of prototype file (eg. "tower") */
    char boneid;    /* character to id level in bones files */
    uchar rndlevs;  /* no. of randomly available similar levels */
    d_flags flags;  /* type flags */
} s_level;

/**
 * @brief What an area of a level is set aside for.
 *
 * A level description marks regions by purpose -- this is where the down stairs go, this is
 * where arriving by teleport puts you, this is where monsters appear from. Naming the purpose
 * rather than the coordinates lets one description place a feature wherever the layout has
 * room.
 *
 * @note Arriving and arriving from a particular direction are separate: a hero who fell and a
 *       hero who teleported may belong in different places.
 */
/**
 * @brief 레벨의 어떤 영역이 무엇을 위해 마련되었는지.
 *
 * 레벨 기술은 영역을 용도로 표시한다. 여기가 내려가는 계단이 놓일 곳, 여기가 순간이동으로 도착하면 놓이는 곳, 여기가 몬스터가
 * 나타나는 곳. 좌표가 아니라 용도로 이름 붙이면, 하나의 기술이 배치에 자리가 있는 곳 아무 데나 그 요소를 놓을 수 있다.
 *
 * @note 도착과 특정 방향에서의 도착이 따로 있다. 떨어진 영웅과 순간이동한 영웅은 서로 다른 곳에 놓여야 할 수 있다.
 */
/* level region types */
enum level_region_types {
    LR_DOWNSTAIR = 0,
    LR_UPSTAIR,
    LR_PORTAL,
    LR_BRANCH,
    LR_TELE,
    LR_UPTELE,
    LR_DOWNTELE,
    LR_MONGEN,
};

/**
 * @brief Where to put someone who arrives without using stairs.
 *
 * A rectangle, plus a second rectangle inside it that is not allowed. Two are needed because the
 * usable area is often not a rectangle: arriving anywhere in this room except in the shop, or
 * anywhere on the level except inside the vault. Carving the exception out of the region is
 * simpler than describing the remainder.
 *
 * @note The excluded rectangle may be empty, in which case the whole region is usable.
 */
/**
 * @brief 계단을 쓰지 않고 도착한 이를 어디에 놓을지.
 *
 * 사각형 하나와, 그 안에서 허용되지 않는 두 번째 사각형. 둘이 필요한 것은 쓸 수 있는 영역이 종종 사각형이 아니기 때문이다. 이
 * 방 안 어디든 단 상점 안은 아니고, 이 레벨 어디든 단 금고 안은 아니고. 남는 부분을 기술하기보다 영역에서 예외를 파내는 것이
 * 더 간단하다.
 *
 * @note 제외 사각형은 비어 있을 수 있고, 그 경우 영역 전체를 쓸 수 있다.
 */
typedef struct dest_area { /* non-stairway level change identifier */
    coordxy lx, ly;          /* "lower" left corner (near [0,0]) */
    coordxy hx, hy;          /* "upper" right corner (near [COLNO,ROWNO]) */
    coordxy nlx, nly;        /* outline of invalid area */
    coordxy nhx, nhy;        /* opposite corner of invalid area */
} dest_area;

/**
 * @brief An area that teleportation may not land in.
 *
 * The complement of an arrival region: rather than saying where someone may appear, this says
 * where they may not. A level may have any number of these, so they form a list.
 *
 * @note The zone records which kind of arrival it excludes, so a place can forbid teleporting in
 *       while still being reachable by stairs.
 */
/**
 * @brief 순간이동이 도착해서는 안 되는 영역.
 *
 * 도착 영역의 여집합이다. 누군가 나타날 수 있는 곳을 말하는 대신, 나타나서는 안 되는 곳을 말한다. 레벨은 이것을 몇 개든 가질
 * 수 있으므로 목록을 이룬다.
 *
 * @note 각 구역은 어떤 종류의 도착을 배제하는지 기록한다. 그래서 어떤 곳이 순간이동으로 들어오는 것은 금지하면서도 계단으로는
 *       닿을 수 있다.
 */
/* teleportation exclusion zones in the level */
typedef struct exclusion_zone {
    xint16 zonetype; /* level_region_types */
    coordxy lx, ly;
    coordxy hx, hy;
    struct exclusion_zone *next;
} exclusion_zone;

/**
 * @brief One dungeon: a run of levels with its own numbering.
 *
 * A dungeon holds the two bases that make its level numbers meaningful elsewhere. Its levels are
 * numbered from one, and adding the depth base gives what the player is told while adding the
 * ledger base gives the unique number used to name its save file.
 *
 * The prototype names say how its levels are built: one file for the dungeon's own layout, one for
 * filling the levels that have no description of their own, one for the themed rooms that may
 * appear in them.
 *
 * @note How deep the hero has been is recorded per dungeon rather than overall, because "deepest
 *       reached" is asked of a particular dungeon -- and going down the mines does not count as
 *       progress in the main dungeon.
 */
/**
 * @brief 던전 하나. 자기 번호 체계를 가진 레벨의 연속.
 *
 * 던전은 자기 레벨 번호가 다른 곳에서 의미를 갖게 하는 두 기준값을 지닌다. 그 레벨들은 1부터 번호가 붙고, 깊이 기준값을 더하면
 * 플레이어에게 알려지는 값이 되고, 원장 기준값을 더하면 저장 파일 이름에 쓰이는 고유 번호가 된다.
 *
 * 원형 파일 이름들은 그 레벨들이 어떻게 지어지는지를 말한다. 던전 자체의 배치를 위한 파일 하나, 자기 기술이 없는 레벨을 채우기
 * 위한 파일 하나, 그 안에 나타날 수 있는 주제 방을 위한 파일 하나.
 *
 * @note 영웅이 얼마나 깊이 들어갔는지가 전체가 아니라 던전마다 기록된다. "가장 깊이 도달한 곳"은 특정 던전에 대해 묻는 것이고,
 *       광산을 내려가는 것이 본 던전에서의 진척으로 셈해지지는 않기 때문이다.
 */
typedef struct dungeon {   /* basic dungeon identifier */
    char dname[24];        /* name of the dungeon (eg. "Hell") */
    char proto[15];        /* name of prototype file (eg. "tower") */
    char fill_lvl[15];     /* name of "fill" level protype file */
    char themerms[15];     /* lua file name containing themed rooms */
    char boneid;           /* character to id dungeon in bones files */
    d_flags flags;         /* dungeon flags */
    xint16 entry_lev;       /* entry level */
    xint16 num_dunlevs;     /* number of levels in this dungeon */
    xint16 dunlev_ureached; /* how deep you have been in this dungeon */
    int ledger_start,      /* the starting depth in "real" terms */
        depth_start;       /* the starting depth in "logical" terms */
} dungeon;

/**
 * @brief A join between two dungeons.
 *
 * A branch is what makes the dungeon a tree rather than a stack. It names two levels and says
 * whether the first leads up or down to the second, so one record serves both directions -- a
 * staircase is the same staircase from either end.
 *
 * Which end is "first" is arbitrary; it is fixed only so that branches can be kept in a definite
 * order and compared. The type is expressed relative to that arbitrary choice, which is why a
 * type meaning "no stair from the first end" is not the same as one meaning "no stair from the
 * second".
 *
 * @note A portal branch has no staircase at either end, so the connection exists without anything
 *       on the map marking it until the portal is found.
 */
/**
 * @brief 두 던전 사이의 이음.
 *
 * 분기는 던전을 더미가 아니라 나무로 만드는 것이다. 두 레벨을 지목하고 첫 번째가 두 번째로 올라가는지 내려가는지를 말하므로, 하나의
 * 기록이 양방향을 맡는다. 계단은 어느 쪽 끝에서 보아도 같은 계단이다.
 *
 * 어느 쪽이 "첫 번째"인지는 임의적이다. 분기들을 확정된 순서로 보관하고 비교할 수 있게 하기 위해서만 정해져 있다. 종류가 그 임의의
 * 선택에 상대적으로 표현되며, 그래서 "첫 끝에서의 계단 없음"을 뜻하는 종류가 "두 번째 끝에서의 계단 없음"과 같지 않다.
 *
 * @note 포탈 분기는 어느 끝에도 계단이 없다. 그래서 포탈이 발견되기 전까지는 지도 위에 그것을 표시하는 것 없이 연결이 존재한다.
 */
typedef struct branch {
    struct branch *next; /* next in the branch chain */
    int id;              /* branch identifier */
    int type;            /* type of branch */
    d_level end1;        /* "primary" end point */
    d_level end2;        /* other end point */
    boolean end1_up;     /* does end1 go up? */
} branch;

/**
 * @name Branch types
 * @brief What the connection physically consists of.
 *
 * A one-way connection is described by which end lacks its staircase rather than by a direction,
 * because the branch's two ends are already fixed in an arbitrary order. So the same asymmetry has
 * two names depending on which end happens to be first.
 *
 * @warning @c BR_NO_END1 and @c BR_NO_END2 are not interchangeable and cannot be worked out from
 *          the geometry -- they refer to the branch's own ordering of its ends.
 * @{
 */
/**
 * @name 분기 종류
 * @brief 그 연결이 물리적으로 무엇으로 이루어져 있는지.
 *
 * 한 방향뿐인 연결은 방향이 아니라 어느 끝에 계단이 없는지로 기술된다. 분기의 두 끝이 이미 임의의 순서로 정해져 있기 때문이다. 그래서
 * 같은 비대칭이 어느 끝이 첫 번째냐에 따라 두 이름을 갖는다.
 *
 * @warning @c BR_NO_END1 과 @c BR_NO_END2 는 서로 바꿔 쓸 수 없고 기하로부터 알아낼 수도 없다. 분기 자신이 자기 끝들에 부여한 순서를
 *          가리킨다.
 * @{
 */
/* branch types */
#define BR_STAIR 0   /* "Regular" connection, 2 staircases. */
#define BR_NO_END1 1 /* "Regular" connection.  However, no stair from
                        end1 to end2.  There is a stair from end2 to end1. */
#define BR_NO_END2 2 /* "Regular" connection.  However, no stair from
                        end2 to end1.  There is a stair from end1 to end2. */
#define BR_PORTAL 3  /* Connection by magic portals (traps) */
/** @} */

/* A particular dungeon contains num_dunlevs d_levels with dlevel 1..
 * num_dunlevs.  Ledger_start and depth_start are bases that are added
 * to the dlevel of a particular d_level to get the effective ledger_no
 * and depth for that d_level.
 *
 * Ledger_no is a bookkeeping number that gives a unique identifier for a
 * particular d_level (for level.?? files, e.g.).
 *
 * Depth corresponds to the number of floors below the surface.
 */

/**
 * @def Lassigned
 * @brief Whether a level identifier has been filled in at all.
 * @note An unassigned identifier is all zeroes, so "either number is non-zero" is the test. This
 *       matters because the special level identifiers are empty until the dungeon has been laid
 *       out or a save restored.
 */
/**
 * @def Lassigned
 * @brief 레벨 식별자가 채워지기라도 했는지.
 * @note 배정되지 않은 식별자는 전부 0이므로 "두 숫자 중 하나가 0이 아님"이 그 검사다. 던전이 배치되거나 저장이 복원되기 전까지 특수 레벨
 *       식별자들이 비어 있기 때문에 이것이 중요하다.
 */
/* These both can't be zero, or dungeon_topology isn't init'd / restored */
#define Lassigned(y) ((y)->dlevel || (y)->dnum)
/**
 * @def Lcheck
 * @brief Whether one level is a particular named level, safely.
 * @param x the level being asked about
 * @param z the named level to compare with
 * @note Checks that the named level has been assigned before comparing. Without that, every
 *       unassigned special level would match any unassigned position -- so early in setup a level
 *       would appear to be all of them at once.
 */
/**
 * @def Lcheck
 * @brief 어떤 레벨이 특정한 이름 붙은 레벨인지를 안전하게 묻는다.
 * @param x 묻는 대상 레벨
 * @param z 비교할 이름 붙은 레벨
 * @note 비교하기 전에 그 이름 붙은 레벨이 배정되었는지 확인한다. 그러지 않으면 배정되지 않은 모든 특수 레벨이 배정되지 않은 아무 위치와
 *       일치하게 된다. 그래서 설정 초기에 한 레벨이 그 전부인 것처럼 보이게 된다.
 */
#define Lcheck(x,z) (Lassigned(z) && on_level(x, z))

/**
 * @name Named levels
 * @brief Is this that particular level.
 *
 * The special levels are identified by comparing against a recorded identifier rather than by
 * depth, because where each lands varies between games. So the rules can speak of "the Oracle
 * level" without knowing how deep it is this time.
 *
 * @note Each is safe on an unassigned identifier and answers false, so these may be used before the
 *       dungeon is fully laid out.
 * @warning The argument is a level identifier and is evaluated more than once.
 * @{
 */
/**
 * @name 이름 붙은 레벨
 * @brief 이것이 바로 그 레벨인지.
 *
 * 특수 레벨들은 깊이가 아니라 기록된 식별자와의 비교로 식별된다. 각각이 어디에 놓이는지가 게임마다 달라지기 때문이다. 그래서 규칙이 이번에는
 * 그것이 몇 층인지 모른 채 "오라클 레벨"을 말할 수 있다.
 *
 * @note 각각은 배정되지 않은 식별자에 대해 안전하며 거짓으로 답한다. 그래서 던전이 완전히 배치되기 전에도 쓸 수 있다.
 * @warning 인자는 레벨 식별자이며 한 번 넘게 평가된다.
 * @{
 */
#define Is_astralevel(x)    (Lcheck(x, &astral_level))
#define Is_earthlevel(x)    (Lcheck(x, &earth_level))
#define Is_waterlevel(x)    (Lcheck(x, &water_level))
#define Is_firelevel(x)     (Lcheck(x, &fire_level))
#define Is_airlevel(x)      (Lcheck(x, &air_level))
#define Is_medusa_level(x)  (Lcheck(x, &medusa_level))
#define Is_oracle_level(x)  (Lcheck(x, &oracle_level))
#define Is_valley(x)        (Lcheck(x, &valley_level))
#define Is_juiblex_level(x) (Lcheck(x, &juiblex_level))
#define Is_asmo_level(x)    (Lcheck(x, &asmodeus_level))
#define Is_baal_level(x)    (Lcheck(x, &baalzebub_level))
#define Is_wiz1_level(x)    (Lcheck(x, &wiz1_level))
#define Is_wiz2_level(x)    (Lcheck(x, &wiz2_level))
#define Is_wiz3_level(x)    (Lcheck(x, &wiz3_level))
#define Is_sanctum(x)       (Lcheck(x, &sanctum_level))
#define Is_portal_level(x)  (Lcheck(x, &portal_level))
#define Is_rogue_level(x)   (Lcheck(x, &rogue_level))
#define Is_stronghold(x)    (Lcheck(x, &stronghold_level))
#define Is_bigroom(x)       (Lcheck(x, &bigroom_level))
#define Is_qstart(x)        (Lcheck(x, &qstart_level))
#define Is_qlocate(x)       (Lcheck(x, &qlocate_level))
#define Is_nemesis(x)       (Lcheck(x, &nemesis_level))
#define Is_knox(x)          (Lcheck(x, &knox_level))
#define Is_mineend_level(x) (Lcheck(x, &mineend_level))
#define Is_sokoend_level(x) (Lcheck(x, &sokoend_level))
/** @} */

/**
 * @name Named dungeons
 * @brief Is this level anywhere within that dungeon.
 *
 * Compares only the dungeon number, so these are true of every level in the dungeon. That is the
 * point: Sokoban's rules apply throughout Sokoban, not on one level of it.
 *
 * @note @c Inhell takes no argument -- it asks about the hero's current level rather than a given
 *       one, so it cannot be used to ask about somewhere else.
 * @{
 */
/**
 * @name 이름 붙은 던전
 * @brief 이 레벨이 그 던전 안 어디든에 있는지.
 *
 * 던전 번호만 비교하므로, 그 던전의 모든 레벨에 대해 참이다. 그것이 요점이다. 소코반의 규칙은 그 한 레벨이 아니라 소코반 전체에 적용된다.
 *
 * @note @c Inhell 은 인자를 받지 않는다. 주어진 레벨이 아니라 영웅의 현재 레벨에 대해 묻는다. 그래서 다른 곳에 대해 묻는 데는 쓸 수 없다.
 * @{
 */
#define In_sokoban(x) ((x)->dnum == sokoban_dnum)
#define Inhell In_hell(&u.uz) /* now gehennom */
#define In_endgame(x) ((x)->dnum == astral_level.dnum)
#define In_tutorial(x) ((x)->dnum == tutorial_dnum)
/** @} */

/**
 * @def within_bounded_area
 * @brief Whether a point lies inside a rectangle, ends included.
 * @param X,Y the point
 * @param LX,LY the corner nearer the origin
 * @param HX,HY the opposite corner
 * @warning Inclusive at both ends, so a rectangle whose corners are equal still contains one square.
 * @warning All six arguments are evaluated more than once.
 */
/**
 * @def within_bounded_area
 * @brief 어떤 점이 사각형 안에 있는지. 끝을 포함하여.
 * @param X,Y 그 점
 * @param LX,LY 원점에 가까운 쪽 모서리
 * @param HX,HY 반대쪽 모서리
 * @warning 양 끝을 포함하므로, 모서리가 같은 사각형도 여전히 칸 하나를 담는다.
 * @warning 여섯 인자 모두 한 번 넘게 평가된다.
 */
#define within_bounded_area(X, Y, LX, LY, HX, HY) \
    ((X) >= (LX) && (X) <= (HX) && (Y) >= (LY) && (Y) <= (HY))

/**
 * @name Where an arrival should be placed
 * @brief How to decide where a monster or object put on another level ends up.
 *
 * Something sent to a level that is not loaded cannot simply be placed -- the level does not exist
 * in memory yet. So it is set aside with a note saying how to place it when that level is next
 * entered, and these are the notes.
 *
 * Most are a manner of arriving: at the stairs, at the ladder, at the branch, at given coordinates,
 * anywhere. The values from @c MIGR_NOBREAK upward are different in kind -- they are bits combined
 * with one of the others, saying what must not happen on delivery rather than where.
 *
 * @warning The two groups share one argument. The placement values are small numbers and the
 *          modifiers are high bits, so a placement must be extracted rather than compared against
 *          the whole value once a modifier has been combined in.
 * @note @c MIGR_NOWHERE is not a placement at all but a report that no destination could be found.
 * @{
 */
/**
 * @name 도착한 것을 어디에 놓을지
 * @brief 다른 레벨로 보내진 몬스터나 물건이 어디에 놓일지 정하는 방법.
 *
 * 적재되지 않은 레벨로 보내진 것은 그냥 놓을 수 없다. 그 레벨이 아직 메모리에 존재하지 않는다. 그래서 그 레벨에 다음에 들어갈 때 어떻게 놓을지를
 * 적은 쪽지와 함께 따로 보관되며, 이들이 그 쪽지다.
 *
 * 대부분은 도착하는 방식이다. 계단에, 사다리에, 분기에, 주어진 좌표에, 아무 데나. @c MIGR_NOBREAK 부터의 값들은 종류가 다르다. 다른 것 하나와
 * 결합되는 비트이며, 어디인지가 아니라 배달 시 무엇이 일어나서는 안 되는지를 말한다.
 *
 * @warning 두 묶음이 하나의 인자를 공유한다. 배치 값은 작은 수이고 수정자는 높은 비트이므로, 수정자가 결합된 뒤에는 배치를 값 전체와 비교하는
 *          것이 아니라 뽑아내야 한다.
 * @note @c MIGR_NOWHERE 는 배치가 전혀 아니라 목적지를 찾을 수 없었다는 보고다.
 * @{
 */
/* monster and object migration codes */

#define MIGR_NOWHERE (-1) /* failure flag for down_gate() */
#define MIGR_RANDOM 0
#define MIGR_APPROX_XY 1 /* approximate coordinates */
#define MIGR_EXACT_XY 2  /* specific coordinates */
#define MIGR_STAIRS_UP 3
#define MIGR_STAIRS_DOWN 4
#define MIGR_LADDER_UP 5
#define MIGR_LADDER_DOWN 6
#define MIGR_SSTAIRS 7      /* dungeon branch */
#define MIGR_PORTAL 8       /* magic portal */
#define MIGR_WITH_HERO 9    /* mon: followers; obj: trap door */
#define MIGR_NOBREAK 1024   /* bitmask: don't break on delivery */
#define MIGR_NOSCATTER 2048 /* don't scatter on delivery */
#define MIGR_TO_SPECIES 4096 /* migrating to species as they are made */
#define MIGR_LEFTOVERS 8192  /* grab remaining MIGR_TO_SPECIES objects */
/** @} */

/**
 * @brief What the game knows about a level it is not currently holding.
 * @note Whether the hero has been there and whether a file exists for it are separate questions even
 *       though they nearly always agree, as the existing comment insists. A level can exist as a file
 *       without having been visited, and the distinction is what makes the difference meaningful.
 */
/**
 * @brief 게임이 지금 지니고 있지 않은 레벨에 대해 아는 것.
 * @note 영웅이 거기 가 보았는지와 그것에 대한 파일이 있는지는 거의 항상 일치하지만, 기존 주석이 강조하듯 서로 다른 질문이다. 방문되지 않은 채
 *       파일로 존재하는 레벨이 있을 수 있고, 그 구별이 차이를 의미 있게 만든다.
 */
/* level information (saved via ledger number) */

struct linfo {
    unsigned char flags;
#define VISITED 0x01      /* hero has visited this level */
/* 0x02 was FORGOTTEN, when amnesia made you forget maps */
#define LFILE_EXISTS 0x04 /* a level file exists for this level */
        /* Note:  VISITED and LFILE_EXISTS are currently almost always
         * set at the same time.  However, they _mean_ different things.
         */
};

/* types and structures for dungeon map recording
 *
 * It is designed to eliminate the need for an external notes file for some
 * mundane dungeon elements.  "Where was the last altar I passed?" etc...
 * Presumably the character can remember this sort of thing even if, months
 * later in real time picking up an old save game, I can't.
 *
 * To be consistent, one can assume that this map is in the player's mind and
 * has no physical correspondence (eliminating illiteracy/blind/hands/hands
 * free concerns).  Therefore, this map is not exhaustive nor detailed ("some
 * fountains").  This makes it also subject to player conditions (amnesia).
 */

/**
 * @brief What the hero remembers being on a level, counted roughly.
 *
 * Each count is two bits, so it saturates at three. That is deliberate: the record stands for a
 * memory, and a memory says "some fountains", not "four fountains". Making the counts exact would
 * turn the overview screen into an inventory of the dungeon.
 *
 * @note Some of these are worked out from the map and some from the room list, which is why they are
 *       gathered rather than updated as things are seen.
 * @note The altar alignment collapses to "none" when the level's altars disagree, since one value
 *       cannot describe several -- so a level with two differently aligned altars reports neither.
 * @warning A count of three means three or more. Arithmetic on these values will understate.
 */
/**
 * @brief 영웅이 어떤 레벨에 무엇이 있었다고 기억하는지. 대략적으로 세어서.
 *
 * 각 개수는 2비트이므로 3에서 포화한다. 이것은 의도적이다. 이 기록은 기억을 나타내고, 기억은 "분수 네 개"가 아니라 "분수 몇 개"라고 말한다. 개수를
 * 정확하게 만들면 개요 화면이 던전의 재고 목록이 되어 버린다.
 *
 * @note 일부는 지도에서, 일부는 방 목록에서 계산된다. 그래서 무언가를 볼 때마다 갱신되는 것이 아니라 모아서 집계된다.
 * @note 레벨의 제단들이 서로 다르면 제단 진영은 "없음"으로 무너진다. 하나의 값이 여럿을 기술할 수 없기 때문이다. 그래서 진영이 다른 제단이 둘 있는
 *       레벨은 어느 쪽도 보고하지 않는다.
 * @warning 개수 3은 셋 이상을 뜻한다. 이 값들로 계산하면 실제보다 적게 나온다.
 */
/* what the player knows about a single dungeon level */
/* initialized in mklev() */
struct mapseen_feat {
    /* feature knowledge that must be calculated from levl array */
    Bitfield(nfount, 2);
    Bitfield(nsink, 2);
    Bitfield(naltar, 2);
    Bitfield(nthrone, 2);

    Bitfield(ngrave, 2);
    Bitfield(ntree, 2);
    Bitfield(water, 2);
    Bitfield(lava, 2);

    Bitfield(ice, 2);
    /* calculated from rooms array */
    Bitfield(nshop, 2);
    Bitfield(ntemple, 2);
    /* altar alignment; MSA_NONE if there is more than one and
       they aren't all the same */
    Bitfield(msalign, 2);

    Bitfield(shoptype, 5);
};
/**
 * @brief Facts about a level worth annotating on the overview.
 *
 * These are not features but things the player would want noted -- that this is the Valley, that
 * Sokoban was solved here, that the level cannot be returned to.
 *
 * @note @c forgot is why the whole record is fallible: amnesia clears what the hero knew, and this
 *       marks a level whose entry no longer reflects what is there.
 * @note The quest flags sit on two different levels and mean different things: one on the level with
 *       the entry portal, cleared once the quest is done, and one on the quest's own first level.
 * @note @c vibrating_square is cleared once the sanctum annotation supersedes it, so it marks a
 *       finding that has not yet been explained rather than a permanent fact.
 */
/**
 * @brief 개요에 적어 둘 만한 레벨에 관한 사실들.
 *
 * 이들은 요소가 아니라 플레이어가 기록해 두고 싶어 할 것들이다. 여기가 계곡이라는 것, 여기서 소코반을 풀었다는 것, 이 레벨로 돌아갈 수 없다는 것.
 *
 * @note @c forgot 이 이 기록 전체가 틀릴 수 있는 이유다. 기억상실은 영웅이 알던 것을 지우며, 이것은 그 항목이 더는 실제와 맞지 않는 레벨을 표시한다.
 * @note 퀘스트 플래그들은 서로 다른 두 레벨에 놓이고 다른 것을 뜻한다. 하나는 입구 포탈이 있는 레벨에 놓여 퀘스트가 끝나면 지워지고, 다른 하나는
 *       퀘스트 자체의 첫 레벨에 놓인다.
 * @note @c vibrating_square 는 성소 주석이 그것을 대체하면 지워진다. 그래서 영구한 사실이 아니라 아직 설명되지 않은 발견을 표시한다.
 */
struct mapseen_flags {
    Bitfield(notreachable, 1); /* can't get back to this level */
    Bitfield(forgot, 1);       /* player has forgotten about this level */
    Bitfield(knownbones, 1);   /* player aware of bones */
    Bitfield(oracle, 1);
    Bitfield(sokosolved, 1);
    Bitfield(bigroom, 1);
    Bitfield(castle, 1);
    Bitfield(castletune, 1); /* add tune hint to castle annotation */

    Bitfield(valley, 1);
    Bitfield(msanctum, 1);
    Bitfield(ludios, 1);
    Bitfield(roguelevel, 1);
    /* quest annotations: quest_summons is for main dungeon level
       with entry portal and is reset once quest has been finished;
       questing is for quest home (level 1) */
    Bitfield(quest_summons, 1); /* heard summons from leader */
    Bitfield(questing, 1);      /* quest leader has unlocked quest stairs */
    /* "gateway to sanctum" */
    Bitfield(vibrating_square, 1); /* found vibrating square 'trap';
                                    * flag cleared once the msanctum
                                    * annotation has been added (on
                                    * the next dungeon level; temple
                                    * entered or high altar mapped) */
    Bitfield(spare1, 1);           /* not used */
};

/**
 * @brief What the hero remembers of one room.
 * @note A shop with no shopkeeper is worth remembering separately from a shop, because it is free to
 *       loot -- so the overview must be able to say so.
 */
/**
 * @brief 영웅이 방 하나에 대해 기억하는 것.
 * @note 상점 주인이 없는 상점은 그냥 상점과 따로 기억할 가치가 있다. 마음대로 가져갈 수 있기 때문이다. 그래서 개요가 그것을 말할 수 있어야 한다.
 */
struct mapseen_rooms {
    Bitfield(seen, 1);
    Bitfield(untended, 1); /* flag for shop without shk */
};

/**
 * @brief The hero's memory of one level, as the overview screen presents it.
 *
 * Kept so the player does not have to keep notes -- where the last altar was, which level had the
 * shop. The hero could reasonably be expected to remember that much, and the game answering it is not
 * the same as the game playing for the player.
 *
 * Held as a list with one entry per level actually visited, rather than an array over every possible
 * level, because most games visit a fraction of the dungeon.
 *
 * @note Stands for what is in the hero's mind and not for anything written down. That is the reasoning
 *       behind its vagueness, and it is also why being blind, illiterate or empty-handed does not
 *       affect it, as the existing comment sets out.
 * @note The branch is recorded when it is taken rather than when it is seen, so the overview knows
 *       about a connection the hero actually used.
 * @note The player's own annotation is kept here too, alongside the game's, since both belong to the
 *       same recollection.
 */
/**
 * @brief 개요 화면이 제시하는, 영웅의 한 레벨에 대한 기억.
 *
 * 플레이어가 따로 적어 두지 않아도 되도록 보관된다. 마지막 제단이 어디였는지, 어느 레벨에 상점이 있었는지. 영웅이 그 정도는 기억할 것이라 봐도 무리가
 * 없고, 게임이 그것에 답하는 것은 게임이 플레이어 대신 플레이하는 것과 같지 않다.
 *
 * 가능한 모든 레벨에 대한 배열이 아니라 실제로 방문한 레벨마다 한 항목인 목록으로 보관된다. 대부분의 게임은 던전의 일부만 방문한다.
 *
 * @note 적어 둔 무엇이 아니라 영웅의 머릿속에 있는 것을 나타낸다. 그것이 이 기록이 모호한 이유이고, 기존 주석이 밝히듯 눈이 멀었거나 글을 모르거나 손이
 *       비어 있는 것이 이것에 영향을 주지 않는 이유이기도 하다.
 * @note 분기는 보였을 때가 아니라 지나갔을 때 기록된다. 그래서 개요는 영웅이 실제로 쓴 연결에 대해 안다.
 * @note 플레이어 자신의 주석도 게임의 것과 나란히 여기에 보관된다. 둘 다 같은 회상에 속하기 때문이다.
 */
typedef struct mapseen {
    struct mapseen *next; /* next map in the chain */
    branch *br;           /* knows about branch via taking it in goto_level */
    d_level lev;          /* corresponding dungeon level */
    struct mapseen_feat feat;
    struct mapseen_flags flags;
    /* custom naming */
    char *custom;
    unsigned custom_lth;
    struct mapseen_rooms msrooms[(MAXNROFROOMS + 1) * 2]; /* same size as svr.rooms[] */
    /* dead heroes; might not have graves or ghosts */
    struct cemetery *final_resting_place; /* same as level.bonesinfo */
} mapseen;

#endif /* !MAKEDEFS_C && !MDLIB_C */

#endif /* DUNGEON_H */
