/* NetHack 5.0	mkroom.h	$NHDT-Date: 1781973082 2026/06/20 16:31:22 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.35 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MKROOM_H
#define MKROOM_H

/* mkroom.h - types and structures for room and shop initialization */

/**
 * @file mkroom.h
 * @brief A room on the level, and what kind of shop a room may become.
 *
 * A room is a rectangle plus what has been decided about it: whether it is lit,
 * what special purpose it serves, how many doors it has, and which creature runs
 * it. The special purpose is kept twice, since @c rtype is cleared once the room's
 * contents have been dealt with while @c orig_rtype records what it was.
 *
 * Shops are described separately as classes -- what a shop of that kind sells, in
 * what proportion, and the names its keeper may have -- so a shop is chosen before
 * it is stocked.
 *
 * @note An irregular room is still given a bounding rectangle, so its coordinates
 *       describe an extent rather than its actual shape.
 * @warning @c hx may be -1 to mark an unused entry, so the coordinates cannot be
 *          assumed valid merely because the slot exists.
 */

/**
 * @file mkroom.h
 * @brief 레벨의 방과, 방이 어떤 종류의 상점이 될 수 있는지.
 *
 * 방은 직사각형에 그에 대해 결정된 것들이 더해진 것이다. 밝은지, 어떤 특수한 용도를
 * 맡는지, 문이 몇 개인지, 어떤 생물이 그곳을 운영하는지다. 특수 용도는 두 번 보관된다.
 * 방의 내용물 처리가 끝나면 @c rtype 은 지워지지만 @c orig_rtype 이 원래 무엇이었는지를
 * 기록하기 때문이다.
 *
 * 상점은 분류로 따로 기술된다. 그 종류의 상점이 무엇을 어떤 비율로 파는지, 주인이 가질 수
 * 있는 이름은 무엇인지다. 그래서 상점은 물건을 채우기 전에 종류가 정해진다.
 *
 * @note 불규칙한 방에도 경계 직사각형이 주어진다. 그래서 좌표는 실제 모양이 아니라 범위를
 *       기술한다.
 * @warning @c hx 는 쓰이지 않는 항목을 표시하기 위해 -1 일 수 있다. 자리가 존재한다는 것만
 *          으로 좌표가 유효하다고 가정할 수 없다.
 */

/**
 * @brief One room, with what has been decided about it.
 * @note @c resident is the priest, shopkeeper or guard the room belongs to -- the
 *       creature that enforces what the room is, rather than merely occupying it.
 * @warning Subrooms are pointers into the same pool of rooms, so a subroom is a
 *          room in its own right and appears in both places.
 */
/**
 * @brief 방 하나와, 그에 대해 결정된 것들.
 * @note @c resident 는 그 방에 속한 사제나 상점 주인, 경비병이다. 단지 그곳에 있는 것이
 *       아니라 그 방이 무엇인지를 집행하는 생물이다.
 * @warning 서브룸은 같은 방 풀 안을 가리키는 포인터다. 그래서 서브룸도 그 자체로 하나의
 *          방이며 양쪽 모두에 나타난다.
 */
struct mkroom {
    coordxy lx, hx, ly, hy; /* usually coordxy, but hx may be -1 */
    schar rtype;          /* type of room (zoo, throne, etc...) */
    schar orig_rtype;     /* same as rtype, but not zeroed later */
    schar rlit;           /* is the room lit ? */
    schar needfill;       /* sp_lev: does the room need filling? */
    boolean needjoining;  /* sp_lev: should the room connect to others? */
    schar doorct;         /* door count */
    int   fdoor;          /* index for the first door of the room */
    schar nsubrooms;      /* number of subrooms */
    boolean irregular;    /* true if room is non-rectangular */
    schar roomnoidx;
    struct mkroom *sbrooms[MAX_SUBROOMS]; /* Subrooms pointers */
    struct monst *resident; /* priest/shopkeeper/guard for this room */
};

/**
 * @brief A kind of shop: what it sells, how likely it is, and who keeps it.
 * @note @c iprobs is a distribution rather than a list: each entry is a
 *       probability paired with either an object class or one specific object,
 *       which is how a lighting shop ends up mostly lamps but not only lamps.
 * @note @c shdist decides arrangement rather than contents -- a temple lays its
 *       goods out differently from a general store.
 * @warning A negative @c itype means one particular object; a non-negative one
 *          means a whole class. The sign is the distinction.
 */
/**
 * @brief 상점의 종류. 무엇을 팔고, 얼마나 흔하며, 누가 지키는지.
 * @note @c iprobs 는 목록이 아니라 분포다. 각 항목은 확률과, 물건 분류 하나 또는 특정 물건
 *       하나가 짝지어진 것이다. 조명 가게가 대부분 등불이면서도 등불만은 아닌 이유다.
 * @note @c shdist 는 내용물이 아니라 배치를 정한다. 신전은 잡화점과 다르게 물건을 늘어놓는다.
 * @warning @c itype 이 음수면 특정 물건 하나를, 음수가 아니면 분류 전체를 뜻한다. 부호가
 *          그 구분이다.
 */
struct shclass {
    const char *name; /* name of the shop type */
    const char *annotation; /* simpler name for #overview; Null if same */
    char symb;        /* this identifies the shop type */
    int prob;         /* the shop type probability in % */
    schar shdist;     /* object placement type */
#define D_SCATTER 0   /* normal placement */
#define D_SHOP 1      /* shop-like placement */
#define D_TEMPLE 2    /* temple-like placement */
    struct itp {
        int iprob;    /* probability of an item type */
        int itype;    /* item type: if >=0 a class, if < 0 a specific item */
    } iprobs[9];
    const char *const *shknms; /* list of shopkeeper names for this type */
};

/* the normal rooms on the current level are described in svr.rooms[0..n] for
 * some n<MAXNROFROOMS
 * the vault, if any, is described by svr.rooms[n+1]
 * the next svr.rooms entry has hx -1 as a flag
 * there is at most one non-vault special room on a level
 */

/* values for rtype in the room definition structure */
enum roomtype_types {
    OROOM      =  0, /* ordinary room */
    THEMEROOM  =  1, /* like OROOM, but never converted to special room */
    COURT      =  2, /* contains a throne */
    SWAMP      =  3, /* contains pools */
    VAULT      =  4, /* detached room usually reached via teleport trap */
    BEEHIVE    =  5, /* contains killer bees and royal jelly */
    MORGUE     =  6, /* contains corpses, undead and graves */
    BARRACKS   =  7, /* contains soldiers and their gear */
    ZOO        =  8, /* floor covered with treasure and monsters */
    DELPHI     =  9, /* contains Oracle and peripherals */
    TEMPLE     = 10, /* contains a shrine (altar attended by priest[ess]) */
    LEPREHALL  = 11, /* leprechaun hall (Tom Proudfoot) */
    COCKNEST   = 12, /* cockatrice nest (Tom Proudfoot) */
    ANTHOLE    = 13, /* ants (Tom Proudfoot) */
    SHOPBASE   = 14, /* everything above this is a shop */
    ARMORSHOP  = 15, /* specific shop defines for level compiler */
    SCROLLSHOP = 16,
    POTIONSHOP = 17,
    WEAPONSHOP = 18,
    FOODSHOP   = 19,
    RINGSHOP   = 20,
    WANDSHOP   = 21,
    TOOLSHOP   = 22,
    BOOKSHOP   = 23,
    FODDERSHOP = 24, /* health food store */
    CANDLESHOP = 25
};

#define MAXRTYPE (CANDLESHOP) /* maximum valid room type */
#define UNIQUESHOP (CANDLESHOP) /* shops here & above not randomly gen'd. */

/* Special type for search_special() */
#define ANY_TYPE (-1)
#define ANY_SHOP (-2)

#define NO_ROOM     0 /* indicates lack of room-occupancy */
#define SHARED      1 /* indicates normal shared boundary */
#define SHARED_PLUS 2 /* indicates shared boundary - extra adjacent-square
                       * searching required */
#define ROOMOFFSET  3 /* (levl[x][y].roomno - ROOMOFFSET) gives svr.rooms[]
                       * index, for inside-squares and non-shared boundaries */

/* Values for needfill */
#define FILL_NONE    0 /* do not fill this room with anything */
#define FILL_NORMAL  1 /* fill the room normally (OROOM or THEMEROOM gets
                        * fill_ordinary_room; any other room type gets stocked
                        * with its usual monsters/objects/terrain) */
#define FILL_LVFLAGS 2 /* special rooms only; set the room's rtype and level
                        * flags as appropriate, but do not put anything in
                        * it */

#define IS_ROOM_PTR(x) \
    ((x) >= svr.rooms && (x) < svr.rooms + MAXNROFROOMS)
#define IS_ROOM_INDEX(x)    ((x) >= 0 && (x) < MAXNROFROOMS)
#define IS_SUBROOM_PTR(x) \
    ((x) >= gs.subrooms && (x) < gs.subrooms + MAXNROFROOMS)
#define IS_SUBROOM_INDEX(x) ((x) > MAXNROFROOMS && (x) <= (MAXNROFROOMS * 2))
#define ROOM_INDEX(x)       ((x) - svr.rooms)
#define SUBROOM_INDEX(x)    ((x) - gs.subrooms)
#define IS_LAST_ROOM_PTR(x) (ROOM_INDEX(x) == svn.nroom)
#define IS_LAST_SUBROOM_PTR(x) \
    (!gn.nsubroom || SUBROOM_INDEX(x) == gn.nsubroom)

#endif /* MKROOM_H */
