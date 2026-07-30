/* NetHack 5.0	artifact.h	$NHDT-Date: 1781973076 2026/06/20 16:31:16 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.23 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file artifact.h
 * @brief What makes an artifact more than the object it is based on.
 *
 * An artifact is a particular object rather than a kind, so its description is
 * not a species entry but a single record: which base object it inhabits, whose
 * side it is on, what it grants while carried or worn, what it does when invoked,
 * and whom it hits harder.
 *
 * Alignment is part of that description, which is what makes wielding a question
 * with an answer -- an artifact may object to who holds it.
 *
 * @note Includes @c permonst.h and @c prop.h because an artifact's bonuses are
 *       expressed in terms of species and of properties it confers, so the
 *       description cannot be read without those vocabularies.
 */

/**
 * @file artifact.h
 * @brief 아티팩트를 그 기반 물건 이상으로 만드는 것.
 *
 * 아티팩트는 종류가 아니라 특정한 하나이므로, 그 기술은 종 항목이 아니라 단일 기록이다.
 * 어떤 기반 물건에 깃들어 있는지, 누구 편인지, 지니거나 착용한 동안 무엇을 주는지, 발동하면
 * 무엇을 하는지, 누구를 더 세게 때리는지다.
 *
 * 성향도 그 기술의 일부다. 그래서 휘두르는 일이 답이 있는 질문이 된다. 아티팩트는 자신을
 * 든 자를 못마땅해할 수 있다.
 *
 * @note @c permonst.h 와 @c prop.h 를 포함한다. 아티팩트의 보너스가 종과 그것이 부여하는
 *       성질의 용어로 표현되므로, 그 어휘 없이는 이 기술을 읽을 수 없기 때문이다.
 */

#ifndef ARTIFACT_H
#define ARTIFACT_H

#include "permonst.h"
#include "prop.h"

/**
 * @brief What an artifact does beyond being a good weapon.
 *
 * Most bits name a property the artifact confers or an ability it has. A few
 * describe how it behaves toward its owner rather than what it grants:
 * @c SPFX_NOGEN means it cannot be created by ordinary means because it is a
 * divine gift, @c SPFX_RESTR that it refuses to be named, @c SPFX_INTEL that it
 * has a will of its own and may object to whoever holds it.
 *
 * The attack-bonus bits are a family: @c SPFX_DMONS, @c SPFX_DCLAS,
 * @c SPFX_DFLAG1, @c SPFX_DFLAG2 and @c SPFX_DALIGN each say *how* the intended
 * victim is identified -- by species, by class symbol, by a species flag, or by
 * alignment -- and @c SPFX_DBONUS is the mask covering all of them.
 *
 * @warning @c SPFX_DBONUS is a mask, not a flag. Testing it asks "does this have
 *          any targeted bonus at all", which is not the same as any single bit.
 * @note @c SPFX_SPEAK is declared but not implemented; the comment says so, and it
 *       should not be relied on.
 */
/**
 * @brief 아티팩트가 좋은 무기라는 것 이상으로 하는 일.
 *
 * 대부분의 비트는 아티팩트가 부여하는 성질이나 지닌 능력을 지칭한다. 몇몇은 무엇을 주는지가
 * 아니라 소유자에게 어떻게 행동하는지를 기술한다. @c SPFX_NOGEN 은 신의 선물이므로 평범한
 * 방법으로 만들어질 수 없음을, @c SPFX_RESTR 은 이름 붙이기를 거부함을, @c SPFX_INTEL 은
 * 자기 의지가 있어 자신을 든 자를 못마땅해할 수 있음을 뜻한다.
 *
 * 공격 보너스 비트들은 한 무리다. @c SPFX_DMONS, @c SPFX_DCLAS, @c SPFX_DFLAG1,
 * @c SPFX_DFLAG2, @c SPFX_DALIGN 은 각각 겨냥하는 대상을 *어떻게* 식별하는지를 말한다.
 * 종으로, 분류 기호로, 종 플래그로, 성향으로다. 그리고 @c SPFX_DBONUS 가 그 전부를 덮는
 * 마스크다.
 *
 * @warning @c SPFX_DBONUS 는 플래그가 아니라 마스크다. 이것을 검사하는 것은 "겨냥된 보너스가
 *          하나라도 있는가"를 묻는 것이며, 개별 비트 하나와는 다른 질문이다.
 * @note @c SPFX_SPEAK 는 선언되어 있으나 구현되지 않았다. 주석이 그렇게 밝히고 있으며,
 *       이에 의존해서는 안 된다.
 */
/* clang-format off */

#define SPFX_NONE   0x00000000L /* no special effects, just a bonus */
#define SPFX_NOGEN  0x00000001L /* item is special, bequeathed by gods */
#define SPFX_RESTR  0x00000002L /* item is restricted - can't be named */
#define SPFX_INTEL  0x00000004L /* item is self-willed - intelligent */
#define SPFX_SPEAK  0x00000008L /* item can speak (not implemented) */
#define SPFX_SEEK   0x00000010L /* item helps you search for things */
#define SPFX_WARN   0x00000020L /* item warns you of danger */
#define SPFX_ATTK   0x00000040L /* item has a special attack (attk) */
#define SPFX_DEFN   0x00000080L /* item has a special defence (defn) */
#define SPFX_DRLI   0x00000100L /* drains a level from monsters */
#define SPFX_SEARCH 0x00000200L /* helps searching */
#define SPFX_BEHEAD 0x00000400L /* beheads monsters */
#define SPFX_HALRES 0x00000800L /* blocks hallucinations */
#define SPFX_ESP    0x00001000L /* ESP (like amulet of ESP) */
#define SPFX_STLTH  0x00002000L /* Stealth */
#define SPFX_REGEN  0x00004000L /* Regeneration */
#define SPFX_EREGEN 0x00008000L /* Energy Regeneration */
#define SPFX_HSPDAM 0x00010000L /* 1/2 spell damage (on player) in combat */
#define SPFX_HPHDAM 0x00020000L /* 1/2 physical damage (on player) in combat */
#define SPFX_TCTRL  0x00040000L /* Teleportation Control */
#define SPFX_LUCK   0x00080000L /* Increase Luck (like Luckstone) */
#define SPFX_DMONS  0x00100000L /* attack bonus on one monster type */
#define SPFX_DCLAS  0x00200000L /* attack bonus on monsters w/ symbol mtype */
#define SPFX_DFLAG1 0x00400000L /* attack bonus on monsters w/ mflags1 flag */
#define SPFX_DFLAG2 0x00800000L /* attack bonus on monsters w/ mflags2 flag */
#define SPFX_DALIGN 0x01000000L /* attack bonus on non-aligned monsters  */
#define SPFX_DBONUS 0x01F00000L /* attack bonus mask */
#define SPFX_XRAY   0x02000000L /* gives X-RAY vision to player */
#define SPFX_REFLECT 0x04000000L /* Reflection */
#define SPFX_PROTECT 0x08000000L /* Protection */

struct artifact {
    short otyp;
    const char *name;
    unsigned long spfx;  /* special effect from wielding/wearing */
    unsigned long cspfx; /* special effect just from carrying obj */
    unsigned long mtype; /* monster type, symbol, or flag */
    struct attack attk, defn, cary;
    uchar inv_prop;     /* property obtained by invoking artifact */
    aligntyp alignment; /* alignment of bequeathing gods */
    short role;         /* character role associated with */
    short race;         /* character race associated with */
    schar gen_spe;      /* bias to spe when gifted or randomly generated */
    uchar gift_value;   /* minimum sacrifice value to be gifted this */
    long cost;          /* price when sold to hero (default 100 x base cost) */
    char acolor;        /* color to use if artifact 'glows' */
};

/* invoked properties with special powers */
enum invoke_prop_types {
    TAMING = (LAST_PROP + 1),
    HEALING,
    ENERGY_BOOST,
    UNTRAP,
    CHARGE_OBJ,
    LEV_TELE,
    CREATE_PORTAL,
    ENLIGHTENING,
    CREATE_AMMO,
    BANISH,
    FLING_POISON,
    FIRESTORM,
    SNOWSTORM,
    BLINDING_RAY
};

/* artifact tracking; gift and wish imply found; it also gets set for items
   seen on the floor, in containers, and wielded or dropped by monsters */
struct arti_info {
    Bitfield(exists, 1); /* 1 if corresponding artifact has been created */
    Bitfield(found, 1);  /* 1 if artifact is known by hero to exist */
    Bitfield(gift, 1);   /* 1 iff artifact was created as a prayer reward */
    Bitfield(wish, 1);   /* 1 iff artifact was created via wish */
    Bitfield(named, 1);  /* 1 iff artifact was made by naming an item */
    Bitfield(viadip, 1); /* 1 iff dipped long sword became Excalibur */
    Bitfield(lvldef, 1); /* 1 iff created by special level definition */
    Bitfield(bones, 1);  /* 1 iff came from bones file */
    Bitfield(rndm, 1);   /* 1 iff randomly generated */
};

/* clang-format on */
#endif /* ARTIFACT_H */
