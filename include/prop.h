/* NetHack 5.0	prop.h	$NHDT-Date: 1781973086 2026/06/20 16:31:26 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.28 $ */
/* Copyright (c) 1989 Mike Threepoint                             */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file prop.h
 * @brief Every property a creature can have, and where each one comes from.
 *
 * A property is not a boolean. The same effect can arrive intrinsically, from a
 * worn item, from something merely carried, or from a temporary source with a
 * timer -- and the game frequently needs to know which, because removing armour
 * takes away what the armour gave without touching what was innate.
 *
 * The list covers resistances, afflictions and abilities together, since all
 * three are held the same way.
 *
 * @note The first eight are laid out to match the monster resistance bits, so one
 *       can be converted to the other by @c res_to_mr() -- which is why their
 *       order is not free.
 * @warning Property 0 is unused, so the list is one-based; @c propertynames[] in
 *          timeout.c must stay in step with these values because it supplies
 *          their names.
 */

/**
 * @file prop.h
 * @brief 생물이 가질 수 있는 모든 성질과, 각각이 어디서 오는지.
 *
 * 성질은 참/거짓이 아니다. 같은 효과가 고유하게 올 수도, 착용한 물건에서 올 수도, 단지
 * 지니고 있는 것에서 올 수도, 타이머가 붙은 일시적 원천에서 올 수도 있다. 게임은 그중
 * 무엇인지를 자주 알아야 한다. 갑옷을 벗으면 갑옷이 준 것은 사라지지만 타고난 것은 그대로
 * 남아야 하기 때문이다.
 *
 * 목록은 저항과 이상 상태, 능력을 함께 담는다. 세 가지 모두 같은 방식으로 보관되기 때문이다.
 *
 * @note 앞의 여덟 개는 몬스터 저항 비트와 맞도록 배치되어 있다. 그래서 @c res_to_mr() 로
 *       한쪽을 다른 쪽으로 바꿀 수 있으며, 그 순서가 자유롭지 않은 이유가 그것이다.
 * @warning 0번 성질은 쓰이지 않으므로 목록은 1부터 시작한다. timeout.c 의
 *          @c propertynames[] 가 이 값들의 이름을 제공하므로 항상 맞춰야 한다.
 */

#ifndef PROP_H
#define PROP_H

/*** What the properties are ***
 *
 * note:  propertynames[] array in timeout.c has string values for these.
 *        Property #0 is not used.
 */
/* Resistances to troubles */
enum prop_types {
    FIRE_RES          =  1,
    COLD_RES          =  2,
    SLEEP_RES         =  3,
    DISINT_RES        =  4,
    SHOCK_RES         =  5,
    POISON_RES        =  6,
    ACID_RES          =  7,
    STONE_RES         =  8,
    /* note: the first eight properties above are equivalent to MR_xxx bits
     * MR_FIRE through MR_STONE, and can be directly converted to them: */
/**
 * @brief Convert a resistance property to the monster resistance bit for it.
 * @param r A property number.
 * @return The corresponding @c MR_ bit, or zero if @p r is not one of the eight
 *         resistances that have one.
 * @note Works only because the first eight properties were ordered to match those
 *       bits; it is arithmetic on that correspondence, not a lookup.
 * @warning Argument is evaluated more than once, so it must be a simple
 *          expression.
 */
/**
 * @brief 저항 성질을 그에 해당하는 몬스터 저항 비트로 변환한다.
 * @param r 성질 번호.
 * @return 대응하는 @c MR_ 비트. @p r 이 그것을 가진 여덟 저항 중 하나가 아니면 0.
 * @note 앞의 여덟 성질이 그 비트들과 맞도록 정렬되어 있기 때문에만 성립한다. 조회가 아니라
 *       그 대응 관계에 대한 산술이다.
 * @warning 인자가 여러 번 평가되므로 단순한 식이어야 한다.
 */
#define res_to_mr(r) \
    ((FIRE_RES <= (r) && (r) <= STONE_RES) ? (uchar) (1 << ((r) - 1)) : 0x00)
    DRAIN_RES         =  9,
    SICK_RES          = 10,
    INVULNERABLE      = 11,
    ANTIMAGIC         = 12,
    /* Troubles */
    STUNNED           = 13,
    CONFUSION         = 14,
    BLINDED           = 15,
    DEAF              = 16,
    SICK              = 17,
    STONED            = 18,
    STRANGLED         = 19,
    VOMITING          = 20,
    GLIB              = 21,
    SLIMED            = 22,
    HALLUC            = 23,
    HALLUC_RES        = 24,
    FUMBLING          = 25,
    WOUNDED_LEGS      = 26,
    SLEEPY            = 27,
    HUNGER            = 28,
    /* Vision and senses */
    SEE_INVIS         = 29,
    TELEPAT           = 30,
    WARNING           = 31,
    WARN_OF_MON       = 32,
    WARN_UNDEAD       = 33,
    SEARCHING         = 34,
    CLAIRVOYANT       = 35,
    INFRAVISION       = 36,
    DETECT_MONSTERS   = 37,
    BLND_RES          = 38,
    /* Appearance and behavior */
    ADORNED           = 39,
    INVIS             = 40,
    DISPLACED         = 41,
    STEALTH           = 42,
    AGGRAVATE_MONSTER = 43,
    CONFLICT          = 44,
    /* Transportation */
    JUMPING           = 45,
    TELEPORT          = 46,
    TELEPORT_CONTROL  = 47,
    LEVITATION        = 48,
    FLYING            = 49,
    WWALKING          = 50,
    SWIMMING          = 51,
    MAGICAL_BREATHING = 52,
    PASSES_WALLS      = 53,
    /* Physical attributes */
    SLOW_DIGESTION    = 54,
    HALF_SPDAM        = 55,
    HALF_PHDAM        = 56,
    REGENERATION      = 57,
    ENERGY_REGENERATION = 58,
    PROTECTION        = 59,
    PROT_FROM_SHAPE_CHANGERS = 60,
    POLYMORPH         = 61,
    POLYMORPH_CONTROL = 62,
    UNCHANGING        = 63,
    FAST              = 64,
    REFLECTING        = 65,
    FREE_ACTION       = 66,
    FIXED_ABIL        = 67,
    LIFESAVED         = 68,
    LAST_PROP = LIFESAVED
};

/*** Where the properties come from ***/
/* Definitions were moved here from obj.h and you.h */
struct prop {
    /*** Properties conveyed by objects ***/
    long extrinsic;
/* Armor */
#define W_ARM 0x00000001L  /* Body armor */
#define W_ARMC 0x00000002L /* Cloak */
#define W_ARMH 0x00000004L /* Helmet/hat */
#define W_ARMS 0x00000008L /* Shield */
#define W_ARMG 0x00000010L /* Gloves/gauntlets */
#define W_ARMF 0x00000020L /* Footwear */
#define W_ARMU 0x00000040L /* Undershirt */
#define W_ARMOR (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
/* Weapons and artifacts */
#define W_WEP 0x00000100L     /* Wielded weapon */
#define W_QUIVER 0x00000200L  /* Quiver for (f)iring ammo */
#define W_SWAPWEP 0x00000400L /* Secondary weapon */
#define W_WEAPONS (W_WEP | W_SWAPWEP | W_QUIVER)
#define W_ART 0x00001000L     /* Carrying artifact (not really worn) */
#define W_ARTI 0x00002000L    /* Invoked artifact  (not really worn) */
/* Amulets, rings, tools, and other items */
#define W_AMUL 0x00010000L    /* Amulet */
#define W_RINGL 0x00020000L   /* Left ring */
#define W_RINGR 0x00040000L   /* Right ring */
#define W_RING (W_RINGL | W_RINGR)
#define W_TOOL 0x00080000L   /* Eyewear */
#define W_ACCESSORY (W_RING | W_AMUL | W_TOOL)
    /* historical note: originally in slash'em, 'worn' saddle stayed in
       hero's inventory; in nethack, it's kept in the steed's inventory */
#define W_SADDLE 0x00100000L /* KMH -- For riding monsters */
#define W_BALL 0x00200000L   /* Punishment ball */
#define W_CHAIN 0x00400000L  /* Punishment chain */

    /*** Property is blocked by an object ***/
    long blocked; /* Same assignments as extrinsic */

    /*** Timeouts, permanent properties, and other flags ***/
    long intrinsic;
/* Timed properties */
#define TIMEOUT 0x00ffffffL     /* Up to 16 million turns */
/* Permanent properties */
#define FROMEXPER   0x01000000L /* Gain/lose with experience, for role */
#define FROMRACE    0x02000000L /* Gain/lose with experience, for race */
#define FROMOUTSIDE 0x04000000L /* By corpses, prayer, thrones, etc. */
#define INTRINSIC   (FROMOUTSIDE | FROMRACE | FROMEXPER)
/* Control flags */
#define FROMFORM    0x10000000L /* Polyd; conferred by monster form */
#define I_SPECIAL   0x20000000L /* Property is controllable */
};

/*** Definitions for backwards compatibility ***/
#define LEFT_RING W_RINGL
#define RIGHT_RING W_RINGR
#define LEFT_SIDE LEFT_RING
#define RIGHT_SIDE RIGHT_RING
#define BOTH_SIDES (LEFT_SIDE | RIGHT_SIDE)
#define WORN_ARMOR W_ARM
#define WORN_CLOAK W_ARMC
#define WORN_HELMET W_ARMH
#define WORN_SHIELD W_ARMS
#define WORN_GLOVES W_ARMG
#define WORN_BOOTS W_ARMF
#define WORN_AMUL W_AMUL
#define WORN_BLINDF W_TOOL
#define WORN_SHIRT W_ARMU

#endif /* PROP_H */
