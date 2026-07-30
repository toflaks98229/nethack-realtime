/* NetHack 5.0	objclass.h	$NHDT-Date: 1781973084 2026/06/20 16:31:24 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.43 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2018. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file objclass.h
 * @brief What a kind of object is, as opposed to what a particular object is.
 *
 * Every object in the game points at one row of a table, and that row holds everything
 * true of all objects of that kind: what it is made of, what it weighs, what it costs,
 * how hard it hits, what a shopkeeper will pay. An individual object then only needs to
 * record what makes it different from its kind -- how enchanted, how eroded, whether
 * the hero knows what it is.
 *
 * The table is unusual in two ways worth knowing before reading it.
 *
 * First, several fields are shared. A single field is the armour class of armour, the
 * spell level of a book, and the accuracy bonus of a weapon, under three different
 * names. This is why the object's class must be known before most of its row can be
 * read: the same bytes mean different things for different classes.
 *
 * Second, the identity of an object kind is split in two. There is the name, which the
 * hero may not know, and the description, which is what an unidentified object looks
 * like -- and the descriptions are shuffled at the start of each game. So the row does
 * not hold names but two indices into a separate table, and discovery is a matter of
 * connecting them.
 *
 * @note The list of object kinds is not written here. It comes from objects.h, read with
 *       a macro defined so that one description of an object yields both the enumerator
 *       and the table row, which cannot then disagree.
 * @warning Reading a shared field without knowing the class yields a plausible number
 *          that means something else entirely.
 */

/**
 * @file objclass.h
 * @brief 어떤 종류의 물건인지. 어떤 개별 물건인지가 아니라.
 *
 * 게임의 모든 물건은 표의 한 줄을 가리키고, 그 줄에는 그 종류의 모든 물건에 참인 것이 담긴다. 무엇으로
 * 만들어졌는지, 무게가 얼마인지, 값이 얼마인지, 얼마나 세게 때리는지, 상점 주인이 얼마를 낼지. 그러면 개별
 * 물건은 자기 종류와 다른 점만 기록하면 된다. 얼마나 마법이 걸렸는지, 얼마나 삭았는지, 영웅이 그것이 무엇인지
 * 아는지.
 *
 * 이 표는 읽기 전에 알아 둘 만한 두 가지 점에서 특이하다.
 *
 * 첫째, 여러 필드가 공유된다. 하나의 필드가 갑옷의 방어도이고, 책의 주문 등급이고, 무기의 명중 보정이며, 세
 * 가지 다른 이름으로 불린다. 그래서 그 줄의 대부분을 읽기 전에 물건의 계열을 알아야 한다. 같은 바이트가
 * 계열에 따라 다른 것을 뜻한다.
 *
 * 둘째, 물건 종류의 정체가 둘로 나뉘어 있다. 영웅이 모를 수도 있는 이름이 있고, 미확인 물건이 어떻게 보이는지인
 * 외형이 있다. 그리고 그 외형들은 매 게임 시작마다 섞인다. 그래서 그 줄은 이름을 담지 않고 별도 표에 대한 두
 * 색인을 담으며, 감별이란 그 둘을 이어 붙이는 일이다.
 *
 * @note 물건 종류의 목록은 여기에 적혀 있지 않다. objects.h 에서 매크로를 정의한 채 읽어 오며, 그래서 하나의
 *       물건 기술이 열거자와 표의 줄을 함께 만들어 내고 그 둘이 어긋날 수 없다.
 * @warning 계열을 모른 채 공유 필드를 읽으면 그럴듯하지만 전혀 다른 것을 뜻하는 숫자가 나온다.
 */

#ifndef OBJCLASS_H
#define OBJCLASS_H

/**
 * @brief What an object is chiefly made of.
 *
 * Material decides what harms an object -- fire, rust, rot, acid -- and how a monster
 * that eats a particular substance sees it. The ordering is not alphabetical but by
 * kind, so the tests below can ask "is this organic" or "is this metal" as a range
 * comparison.
 *
 * @warning Most objects are composites -- a potion is liquid in glass, an arrow is metal
 *          on wood -- and a row records one material on a best-fit basis, as the existing
 *          comment concedes. So the answer is the material that matters, not the whole
 *          truth.
 * @note @c DRAGON_HIDE is deliberately not leather; that is the point of it having its
 *       own value.
 */
/**
 * @brief 물건이 주로 무엇으로 만들어졌는지.
 *
 * 재질은 무엇이 물건을 손상시키는지 -- 불, 녹, 부패, 산 -- 를 정하고, 특정 물질을 먹는 몬스터가 그것을 어떻게
 * 보는지를 정한다. 순서는 알파벳순이 아니라 종류순이다. 그래서 아래의 검사들이 "이것이 유기물인가", "이것이
 * 금속인가"를 범위 비교로 물을 수 있다.
 *
 * @warning 대부분의 물건은 복합물이다. 물약은 유리에 담긴 액체이고 화살은 나무에 붙은 금속이다. 그리고 기존
 *          주석이 인정하듯 각 줄은 가장 잘 맞는 재질 하나만 기록한다. 그래서 그 답은 중요한 재질이며, 진실의
 *          전부가 아니다.
 * @note @c DRAGON_HIDE 는 의도적으로 가죽이 아니다. 자기 값을 따로 갖는 이유가 그것이다.
 */
/* [misnamed] definition of a type of object; many objects are composites
   (liquid potion inside glass bottle, metal arrowhead on wooden shaft)
   and object definitions only specify one type on a best-fit basis */
enum obj_material_types {
    NO_MATERIAL =  0,
    LIQUID      =  1, /* currently only for venom */
    WAX         =  2,
    VEGGY       =  3, /* foodstuffs */
    FLESH       =  4, /*   ditto    */
    PAPER       =  5,
    CLOTH       =  6,
    LEATHER     =  7,
    WOOD        =  8,
    BONE        =  9,
    DRAGON_HIDE = 10, /* not leather! */
    IRON        = 11, /* Fe - includes steel */
    METAL       = 12, /* Sn, &c. */
    COPPER      = 13, /* Cu - includes brass */
    SILVER      = 14, /* Ag */
    GOLD        = 15, /* Au */
    PLATINUM    = 16, /* Pt */
    MITHRIL     = 17,
    PLASTIC     = 18,
    GLASS       = 19,
    GEMSTONE    = 20,
    MINERAL     = 21
};

/**
 * @brief Where on the body a piece of armour goes.
 * @note Also the order in which armour must be removed: a suit cannot come off over a
 *       cloak, which is why the slot is stored rather than deduced from the object.
 * @note @c ARM_SHIELD is called out in the existing comment because wearing one needs
 *       its own handling -- it occupies a hand rather than covering a part.
 */
/**
 * @brief 갑옷이 몸의 어디에 놓이는지.
 * @note 갑옷을 벗어야 하는 순서이기도 하다. 겉옷 위로 갑옷 몸통을 벗을 수는 없다. 그래서 부위가 물건에서
 *       유도되지 않고 저장된다.
 * @note 기존 주석이 @c ARM_SHIELD 를 따로 언급하는 것은 그것을 착용하는 데 별도의 처리가 필요하기 때문이다.
 *       부위를 덮는 것이 아니라 손을 차지한다.
 */
enum obj_armor_types {
    ARM_SUIT   = 0,
    ARM_SHIELD = 1,        /* needed for special wear function */
    ARM_HELM   = 2,
    ARM_GLOVES = 3,
    ARM_BOOTS  = 4,
    ARM_CLOAK  = 5,
    ARM_SHIRT  = 6
};

/**
 * @brief One kind of object: everything true of every object of that kind.
 *
 * Split loosely into three parts. First, identity and what the hero knows of it -- the
 * name, the shuffled description, whether it has been discovered, what the player has
 * chosen to call it. Then the object's nature as flags. Then its numbers: weight, price,
 * damage, nutrition.
 *
 * The numbers are where the sharing happens. One field is armour class or spell level or
 * accuracy bonus depending on the class; the shared names are defined alongside the
 * fields so the intended reading is visible at the point of use.
 *
 * @note The knowledge fields belong here and not on the object because knowledge is about
 *       the kind: identifying one potion of healing identifies them all.
 * @note @c oc_uses_known distinguishes kinds whose full description depends on more than
 *       being seen. For the rest, the existing comment warns that the object's own known
 *       flag must be set anyway, or otherwise identical objects will refuse to merge.
 * @warning A shared field read under the wrong name is not detectable at run time.
 */
/**
 * @brief 물건 종류 하나. 그 종류의 모든 물건에 참인 것 전부.
 *
 * 대략 세 부분으로 나뉜다. 먼저 정체와 영웅이 그것에 대해 아는 것. 이름, 섞인 외형, 감별되었는지, 플레이어가
 * 무엇이라 부르기로 했는지. 다음으로 플래그로 표현된 물건의 성질. 다음으로 숫자들. 무게, 값, 피해, 영양.
 *
 * 공유가 일어나는 곳이 그 숫자들이다. 하나의 필드가 계열에 따라 갑옷의 방어도이거나 주문 등급이거나 명중
 * 보정이다. 공유되는 이름들은 필드 곁에 정의되어 있어 의도된 해석이 쓰이는 자리에서 보인다.
 *
 * @note 앎에 관한 필드가 물건이 아니라 여기에 있는 것은, 앎이 종류에 관한 것이기 때문이다. 치유의 물약 하나를
 *       감별하면 그 전부가 감별된다.
 * @note @c oc_uses_known 은 완전한 기술이 단지 보이는 것 이상에 달려 있는 종류를 구별한다. 나머지에 대해서는
 *       기존 주석이 물건 자신의 known 플래그를 어쨌든 켜 두어야 한다고 경고한다. 그러지 않으면 똑같은 물건들이
 *       합쳐지기를 거부한다.
 * @warning 잘못된 이름으로 공유 필드를 읽는 것은 실행 중에 감지되지 않는다.
 */
struct objclass {
    short oc_name_idx;              /* index of actual name */
    short oc_descr_idx;             /* description when name unknown */
    char *oc_uname;                 /* called by user */
    Bitfield(oc_name_known, 1);     /* discovered */
    Bitfield(oc_merge, 1);          /* merge otherwise equal objects */
    Bitfield(oc_uses_known, 1);     /* obj->known affects full description;
                                     * otherwise, obj->dknown and obj->bknown
                                     * tell all, and obj->known should always
                                     * be set for proper merging behavior. */
    Bitfield(oc_encountered, 1);    /* hero has observed such an item at least
                                       once (perhaps without naming it) */
    Bitfield(oc_magic, 1);          /* inherently magical object */
    Bitfield(oc_charged, 1);        /* may have +n or (n) charges */
    Bitfield(oc_unique, 1);         /* special one-of-a-kind object */
    Bitfield(oc_nowish, 1);         /* cannot wish for this object */

    Bitfield(oc_big, 1);
#define oc_bimanual oc_big /* for weapons & tools used as weapons */
#define oc_bulky oc_big    /* for armor */
    Bitfield(oc_tough, 1); /* hard gems/rings */

    Bitfield(oc_spare1, 6);         /* padding to align oc_dir + oc_material;
                                     * can be cannibalized for other use;
                                     * aka 6 free bits */

    /**
     * @brief How the object reaches its target -- meaning one of two unrelated things.
     *
     * For a wand or spell it is how the magic travels: at nothing in particular, in a
     * straight line, or as a beam that bounces off walls. For a weapon it is instead a
     * set of bits saying how the weapon strikes, which decides the wording of a hit and
     * what the blow can cut through.
     *
     * @warning The two readings share the field and their values overlap. A weapon may
     *          have several strike bits set at once, so the same stored number is a
     *          single choice for a wand and a combination for a weapon. Nothing here
     *          records which reading applies; the object's class does.
     */
    /**
     * @brief 물건이 대상에 어떻게 닿는지. 서로 무관한 두 가지 중 하나를 뜻한다.
     *
     * 지팡이나 주문에게는 마법이 어떻게 이동하는지다. 특정 대상 없이, 직선으로, 또는 벽에 튀는 광선으로.
     * 무기에게는 대신 무기가 어떻게 때리는지를 말하는 비트 묶음이며, 그것이 명중 문장의 표현과 그 타격이 무엇을
     * 베어 낼 수 있는지를 정한다.
     *
     * @warning 두 해석이 필드를 공유하고 값이 겹친다. 무기는 여러 타격 비트를 동시에 가질 수 있으므로, 저장된
     *          같은 숫자가 지팡이에게는 하나의 선택이고 무기에게는 조합이다. 어느 해석이 적용되는지를 기록하는
     *          것은 여기에 없다. 물건의 계열이 그것을 정한다.
     */
    Bitfield(oc_dir, 3);
    /* oc_dir: zap style for wands and spells */
#define NODIR     1 /* non-directional */
#define IMMEDIATE 2 /* directional beam that doesn't ricochet */
#define RAY       3 /* beam that does bounce off walls */
    /* overloaded oc_dir: strike mode bit mask for weapons and weptools */
#define PIERCE    1 /* pointed weapon punctures target */
#define SLASH     2 /* sharp weapon cuts target */
#define WHACK     4 /* blunt weapon bashes target */
    Bitfield(oc_material, 5); /* one of obj_material_types */

    /**
     * @brief A finer classification within the object's class.
     * @note For most classes this is the skill the object uses, which is why skills.h is
     *       written so that objects.c can include it without the hero. For armour it is
     *       the body slot instead.
     * @warning Read under the wrong name it is a valid value of the other kind: a skill
     *          number is also a plausible armour slot.
     */
    /**
     * @brief 물건의 계열 안에서의 더 세밀한 분류.
     * @note 대부분의 계열에서는 그 물건이 쓰는 기술이다. skills.h 가 objects.c 에서 영웅 없이 포함할 수 있도록
     *       작성된 이유가 그것이다. 갑옷에서는 대신 몸의 부위다.
     * @warning 잘못된 이름으로 읽으면 다른 종류의 유효한 값이 된다. 기술 번호는 그럴듯한 갑옷 부위이기도 하다.
     */
    schar oc_subtyp;
#define oc_skill oc_subtyp  /* Skills of weapons, spellbooks, tools, gems */
#define oc_armcat oc_subtyp /* for armor (enum obj_armor_types) */

    uchar oc_oprop; /* property (invis, &c.) conveyed */
    char  oc_class; /* object class (enum obj_class_types) */
    schar oc_delay; /* delay when using such an object */
    uchar oc_color; /* color of the object */

    short oc_prob;            /* probability, used in mkobj() */
    unsigned oc_weight;       /* encumbrance (1 cn = 0.1 lb.) */
    short oc_cost;            /* base cost in shops */
    /* Check the AD&D rules!  The FIRST is small monster damage. */
    /* for weapons, and tools, rocks, and gems useful as weapons */
    schar oc_wsdam, oc_wldam; /* max small/large monster damage */
    /**
     * @brief Two fields with no meaning of their own, named differently per class.
     *
     * Deliberately given colourless names, because there is no honest single name for
     * them: the first is a weapon's accuracy bonus or a piece of armour's protection, the
     * second is armour's bonus or a spellbook's level. The aliases below are the real
     * names, and code should use those.
     *
     * @warning Every reading is a small signed number, so a wrong alias produces a
     *          believable value. Nothing detects the mistake.
     */
    /**
     * @brief 자기 의미가 없는 두 필드. 계열마다 다르게 이름 붙여 쓴다.
     *
     * 일부러 특색 없는 이름을 주었다. 정직한 단일 이름이 없기 때문이다. 첫 번째는 무기의 명중 보정이거나 갑옷의
     * 방호이고, 두 번째는 갑옷의 보정이거나 주문서의 등급이다. 아래의 별칭들이 진짜 이름이며, 코드는 그것을 써야
     * 한다.
     *
     * @warning 모든 해석이 작은 부호 있는 수다. 그래서 잘못된 별칭이 그럴듯한 값을 낸다. 그 잘못을 감지하는
     *          것은 없다.
     */
    schar oc_oc1, oc_oc2;
#define oc_hitbon oc_oc1 /* weapons: "to hit" bonus */

#define a_ac oc_oc1     /* armor class, used in ARM_BONUS in do.c */
#define a_can oc_oc2    /* armor: used in mhitu.c */
#define oc_level oc_oc2 /* books: spell level */

    unsigned short oc_nutrition; /* food value */

    /**
     * @brief The range of prices the hero has actually been quoted for this kind.
     *
     * Shop prices vary with the hero's charisma and how the shopkeeper judges them, so one
     * quote does not reveal an object's base price. Remembering the extremes seen lets the
     * player narrow down an unidentified kind by shopping, which is a genuine
     * identification technique -- so the game keeps the evidence rather than making the
     * player write it down.
     *
     * @note Buying and selling are tracked separately because the two prices are derived
     *       differently.
     */
    /**
     * @brief 영웅이 이 종류에 대해 실제로 들은 가격의 범위.
     *
     * 상점 가격은 영웅의 매력과 상점 주인이 그를 어떻게 보는지에 따라 달라지므로, 한 번의 호가로 물건의 기준
     * 가격을 알 수는 없다. 본 값의 양 극단을 기억해 두면 플레이어가 장을 봐서 미확인 종류의 범위를 좁힐 수 있다.
     * 그것은 실제로 쓰이는 감별 기법이므로, 게임이 플레이어에게 적어 두게 하는 대신 근거를 보관한다.
     *
     * @note 사는 값과 파는 값은 유도되는 방식이 다르므로 따로 기록된다.
     */
    unsigned long oc_sell_minseen;
    unsigned long oc_sell_maxseen;
    unsigned long oc_buy_minseen;
    unsigned long oc_buy_maxseen;
};

/**
 * @brief One object class as the player meets it: its character, its name, and a line
 *        explaining it.
 * @note The explanation is what the player is shown when asking what a symbol means, so it
 *       is game text rather than a comment.
 */
/**
 * @brief 플레이어가 마주하는 물건 계열 하나. 그 문자, 그 이름, 그리고 설명 한 줄.
 * @note 설명문은 플레이어가 심볼이 무엇을 뜻하는지 물었을 때 보여지는 글이다. 그래서 주석이 아니라 게임 텍스트다.
 */
struct class_sym {
    char sym;
    const char *name;
    const char *explain;
};

/**
 * @brief The two identities of an object kind: what it is, and what it looks like.
 *
 * Held apart and reached through two separate indices because the descriptions are
 * shuffled at the start of each game. A red potion is a different potion in every game,
 * so the name cannot be stored next to the appearance.
 *
 * @note A kind with no secret identity has no description; the name serves for both.
 */
/**
 * @brief 물건 종류의 두 정체. 그것이 무엇인지, 그리고 어떻게 보이는지.
 *
 * 따로 보관되고 두 개의 별도 색인으로 접근된다. 외형이 매 게임 시작마다 섞이기 때문이다. 빨간 물약은 게임마다
 * 다른 물약이므로, 이름을 외형 곁에 저장할 수 없다.
 *
 * @note 감출 정체가 없는 종류에는 외형이 없다. 이름이 둘 다를 맡는다.
 */
struct objdescr {
    const char *oc_name;  /* actual name */
    const char *oc_descr; /* description when name unknown */
};

/*
 * All objects have a class. Make sure that all classes have a corresponding
 * symbol below.
 */

enum objclass_defchars {
#define OBJCLASS_DEFCHAR_ENUM
#include "defsym.h"
#undef OBJCLASS_DEFCHAR_ENUM
};

enum objclass_classes {
    RANDOM_CLASS =  0, /* used for generating random objects */
#define OBJCLASS_CLASS_ENUM
#include "defsym.h"
#undef OBJCLASS_CLASS_ENUM
    MAXOCLASSES
};

/* Default characters for object classes */
enum objclass_syms {
#define OBJCLASS_S_ENUM
#include "defsym.h"
#undef OBJCLASS_S_ENUM
};

/**
 * @def SPBOOK_no_NOVEL
 * @brief Ask for a random spellbook that is not a novel.
 * @note Expressed as the negation of the class so that one argument can carry both "any
 *       book" and "any book except novels" -- a novel is a book to the game but not a book
 *       a hero can learn from.
 * @warning For object creation only, and cast to a signed type on purpose: the enum may be
 *          unsigned, in which case negating it without the cast gives a large positive
 *          number rather than a negative one.
 */
/**
 * @def SPBOOK_no_NOVEL
 * @brief 소설이 아닌 무작위 주문서를 요청한다.
 * @note 계열의 음수로 표현되어 하나의 인자가 "아무 책"과 "소설을 뺀 아무 책"을 함께 담을 수 있다. 소설은 게임에게
 *       책이지만 영웅이 배울 수 있는 책은 아니다.
 * @warning 물건 생성에만 쓰이며, 부호 있는 타입으로의 형변환은 의도적이다. 열거형이 부호 없을 수 있고, 그 경우
 *          형변환 없이 음수화하면 음수가 아니라 큰 양수가 된다.
 */
/* for mkobj() use ONLY! odd '-SPBOOK_CLASS' is in case of unsigned enums */
#define SPBOOK_no_NOVEL (0 - (int) SPBOOK_CLASS)

/**
 * @name Explosion sources that are not object classes
 * @brief Things that can explode but have no class of their own.
 *
 * An explosion is described by what caused it, and the cause is usually an object class --
 * a potion, a wand. But burning oil, a bursting monster and a trap are also causes, so they
 * are given values past the end of the classes and passed through the same argument.
 *
 * @warning These are deliberately out of range of the class enumeration. Anything that
 *          indexes a per-class table with an explosion source will read past its end.
 * @{
 */
/**
 * @name 물건 계열이 아닌 폭발의 원인
 * @brief 폭발할 수 있으나 자기 계열이 없는 것들.
 *
 * 폭발은 무엇이 그것을 일으켰는지로 기술되고, 그 원인은 보통 물건 계열이다. 물약, 지팡이. 그런데 타는 기름,
 * 터지는 몬스터, 함정도 원인이므로, 계열들의 끝 너머의 값을 받아 같은 인자를 통해 전달된다.
 *
 * @warning 이들은 의도적으로 계열 열거의 범위를 벗어나 있다. 폭발 원인으로 계열별 표를 색인하는 것은 그 끝을
 *          넘어 읽게 된다.
 * @{
 */
#define BURNING_OIL (MAXOCLASSES + 1) /* Can be used as input to explode    */
#define MON_EXPLODE (MAXOCLASSES + 2) /* Exploding monster (e.g. gas spore) */
#define TRAP_EXPLODE (MAXOCLASSES + 3)
/** @} */

#if 0 /* moved to decl.h so that makedefs.c won't see them */
extern const struct class_sym
        def_oc_syms[MAXOCLASSES];       /* default class symbols */
extern uchar oc_syms[MAXOCLASSES];      /* current class symbols */
#endif

/**
 * @brief A fruit the player named.
 *
 * There is one object kind for "a fruit" and the player supplies its name at the start of
 * the game, but the name can change -- and fruits created earlier must keep the name they
 * had. So each name used is kept with an identifier, and a fruit object records the
 * identifier rather than the text.
 *
 * @note A list rather than a table because the number of names a game accumulates is not
 *       bounded in advance.
 */
/**
 * @brief 플레이어가 이름 붙인 과일.
 *
 * "과일"에 해당하는 물건 종류는 하나이고 플레이어가 게임 시작에 그 이름을 정한다. 그런데 그 이름은 바뀔 수 있고,
 * 먼저 만들어진 과일은 그때의 이름을 지녀야 한다. 그래서 쓰인 이름마다 식별자와 함께 보관되며, 과일 물건은 글이
 * 아니라 그 식별자를 기록한다.
 *
 * @note 표가 아니라 목록인 것은, 한 게임이 쌓는 이름의 수가 미리 정해져 있지 않기 때문이다.
 */
struct fruit {
    char fname[PL_FSIZ];
    int fid;
    struct fruit *nextf;
};
#define newfruit() (struct fruit *) alloc(sizeof(struct fruit))
#define dealloc_fruit(rind) free((genericptr_t)(rind))

enum objects_nums {
#define OBJECTS_ENUM
#include "objects.h"
#undef OBJECTS_ENUM
    NUM_OBJECTS
};

/**
 * @brief Sizes worked out from where groups of object kinds begin and end.
 * @note Computed from the enumerators rather than written as numbers, so inserting a gem
 *       into the run of gems updates the count without anyone remembering to.
 * @note @c MAXSPELL is one larger than the number of spells a hero can learn, because the
 *       last book kind is blank paper. The existing comment records that this guarantees a
 *       spare slot at the end of the spell list to act as a terminator.
 */
/**
 * @brief 물건 종류 묶음이 어디서 시작하고 끝나는지에서 계산한 크기들.
 * @note 숫자로 적지 않고 열거자에서 계산한다. 그래서 보석 구간에 보석 하나를 끼워 넣으면 아무도 기억하지 않아도
 *       개수가 갱신된다.
 * @note @c MAXSPELL 은 영웅이 배울 수 있는 주문의 수보다 하나 크다. 마지막 책 종류가 백지이기 때문이다. 기존 주석은
 *       이것이 주문 목록 끝에 종결자로 쓸 빈 칸 하나를 보장한다고 기록하고 있다.
 */
enum misc_object_nums {
    NUM_REAL_GEMS  = (LAST_REAL_GEM - FIRST_REAL_GEM + 1),
    NUM_GLASS_GEMS = (LAST_GLASS_GEM - FIRST_GLASS_GEM + 1),
    /* LAST_SPELL is SPE_BLANK_PAPER, guaranteeing that spl_book[] will
       have at least one unused slot at end to be used as a terminator */
    MAXSPELL       = (LAST_SPELL - FIRST_SPELL + 1),
};

/**
 * @var objects
 * @brief The table of object kinds, one row per kind.
 * @warning Not constant. Discovery, the player's own names, and the shuffling of
 *          descriptions all write to it, so it is part of the saved game rather than
 *          static data.
 */
/**
 * @var objects
 * @brief 물건 종류의 표. 종류마다 한 줄.
 * @warning 상수가 아니다. 감별, 플레이어 자신이 붙인 이름, 외형의 섞기가 모두 여기에 쓴다. 그래서 정적 데이터가
 *          아니라 저장 게임의 일부다.
 */
extern NEARDATA struct objclass objects[NUM_OBJECTS + 1];
/**
 * @var obj_descr
 * @brief The names and appearances, kept apart from the table that points into it.
 * @note Separate so that shuffling appearances is a matter of changing indices in the main
 *       table, leaving this text untouched.
 */
/**
 * @var obj_descr
 * @brief 이름과 외형. 이곳을 가리키는 표와 따로 보관된다.
 * @note 따로 두어, 외형을 섞는 일이 이 글은 건드리지 않고 본 표의 색인만 바꾸는 일이 되게 한다.
 */
extern NEARDATA struct objdescr obj_descr[NUM_OBJECTS + 1];

/**
 * @def OBJ_NAME
 * @brief What a kind of object actually is.
 * @param obj an object class row, not a pointer and not an object
 * @warning This is the true name whether or not the hero has discovered it, so showing it
 *          to the player unconditionally gives away an identification.
 */
/**
 * @def OBJ_NAME
 * @brief 어떤 종류의 물건이 실제로 무엇인지.
 * @param obj 물건 계열의 줄. 포인터도 물건도 아니다
 * @warning 영웅이 감별했는지와 무관하게 참된 이름이다. 그래서 조건 없이 플레이어에게 보여 주면 감별을 누설한다.
 */
#define OBJ_NAME(obj) (obj_descr[(obj).oc_name_idx].oc_name)
/**
 * @def OBJ_DESCR
 * @brief What a kind of object looks like to someone who does not know it.
 * @param obj an object class row, not a pointer and not an object
 * @note Yields nothing for kinds that have no secret identity, so a caller must be prepared
 *       for an absent description rather than assuming one exists.
 */
/**
 * @def OBJ_DESCR
 * @brief 그것을 모르는 이에게 어떤 종류의 물건이 어떻게 보이는지.
 * @param obj 물건 계열의 줄. 포인터도 물건도 아니다
 * @note 감출 정체가 없는 종류에는 아무것도 내지 않는다. 그래서 호출자는 외형이 있다고 가정하는 대신 없는 경우를
 *       대비해야 한다.
 */
#define OBJ_DESCR(obj) (obj_descr[(obj).oc_descr_idx].oc_descr)

/**
 * @def is_organic
 * @brief Made of something that was once alive, and so can rot or burn.
 * @note A range comparison, which works only because the materials are ordered with the
 *       organic ones first.
 */
/**
 * @def is_organic
 * @brief 한때 살아 있던 것으로 만들어졌다. 그래서 썩거나 탈 수 있다.
 * @note 범위 비교이며, 재질이 유기물부터 순서대로 놓여 있기 때문에만 성립한다.
 */
#define is_organic(otmp) (objects[otmp->otyp].oc_material <= WOOD)
/**
 * @def is_metallic
 * @brief Made of metal, which decides how magic and corrosion treat it.
 * @note Another range comparison; the metals are contiguous for exactly this reason.
 */
/**
 * @def is_metallic
 * @brief 금속으로 만들어졌다. 그것이 마법과 부식이 그것을 어떻게 대하는지를 정한다.
 * @note 또 하나의 범위 비교다. 금속들이 연속해 있는 것이 바로 이 때문이다.
 */
#define is_metallic(otmp) \
    (objects[otmp->otyp].oc_material >= IRON            \
     && objects[otmp->otyp].oc_material <= MITHRIL)

/**
 * @name Ways an object can be spoiled
 * @brief Which kinds of erosion this object is subject to.
 *
 * Every object takes at most one primary and one secondary kind of damage, decided by its
 * material -- iron rusts and then corrodes, wood burns and then rots. That pairing is why
 * an object needs only two erosion counters however many hazards exist.
 *
 * @note Two of the set, flammability and rottability, are functions in mkobj.c rather than
 *       macros here, because they consider more than the material.
 * @note @c is_crackable also requires the object to be armour: glass that is not worn is
 *       not tracked as breakable, since erosion only matters where it can be repaired.
 * @warning The argument is evaluated more than once throughout.
 * @{
 */
/**
 * @name 물건이 상할 수 있는 방식
 * @brief 이 물건이 어떤 종류의 삭음을 겪을 수 있는지.
 *
 * 모든 물건은 재질에 따라 정해지는 주된 손상 한 가지와 부차적 손상 한 가지만을 겪는다. 철은 녹고 그다음 부식하고,
 * 나무는 타고 그다음 썩는다. 위험이 몇 가지든 물건에 삭음 계수기가 둘만 필요한 이유가 그 짝지음이다.
 *
 * @note 이 묶음 중 가연성과 부패성은 여기의 매크로가 아니라 mkobj.c 의 함수다. 재질 이상을 고려하기 때문이다.
 * @note @c is_crackable 은 물건이 갑옷이기를 함께 요구한다. 착용하지 않는 유리는 깨질 수 있는 것으로 추적되지
 *       않는다. 삭음은 고칠 수 있는 곳에서만 의미가 있기 때문이다.
 * @warning 전반적으로 인자가 한 번 넘게 평가된다.
 * @{
 */
/* primary damage: fire/rust/--- */
/* is_flammable(otmp), is_rottable(otmp) in mkobj.c */
#define is_rustprone(otmp) (objects[otmp->otyp].oc_material == IRON)
#define is_crackable(otmp) \
    (objects[(otmp)->otyp].oc_material == GLASS         \
     && (otmp)->oclass == ARMOR_CLASS) /* erosion_matters() */
/* secondary damage: rot/acid/acid */
#define is_corrodeable(otmp) \
    (objects[otmp->otyp].oc_material == COPPER          \
     || objects[otmp->otyp].oc_material == IRON)
/**
 * @def is_damageable
 * @brief Subject to erosion of any kind at all.
 * @note The question asked when deciding whether an object needs erosion state or can be
 *       shown as simply intact.
 * @warning Expands to five tests, each evaluating the argument again, and two of them are
 *          function calls -- so this is not free and should not sit inside a tight loop.
 */
/**
 * @def is_damageable
 * @brief 어떤 종류의 삭음이든 겪을 수 있다.
 * @note 물건이 삭음 상태를 필요로 하는지, 아니면 그냥 온전한 것으로 보여도 되는지를 정할 때 묻는 질문이다.
 * @warning 다섯 개의 검사로 펼쳐지고 각각이 인자를 다시 평가하며, 그 중 둘은 함수 호출이다. 그래서 값이 싸지
 *          않고, 빡빡한 반복문 안에 두어서는 안 된다.
 */
/* subject to any damage */
#define is_damageable(otmp) \
    (is_rustprone(otmp) || is_flammable(otmp)           \
     || is_rottable(otmp) || is_corrodeable(otmp)       \
     || is_crackable(otmp))
/** @} */

#endif /* OBJCLASS_H */
