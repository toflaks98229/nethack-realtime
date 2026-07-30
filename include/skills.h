/* NetHack 5.0	skills.h	$NHDT-Date: 1781973087 2026/06/20 16:31:27 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.21 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/*-Copyright (c) Pasi Kallinen, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file skills.h
 * @brief What the hero can become practised at.
 *
 * A skill covers a class of weapon or a school of magic rather than an individual
 * object, which is why exchanging one sword for another loses nothing while
 * picking up an unfamiliar kind costs accuracy.
 *
 * Each role has its own ceiling per skill, so advancement moves the hero toward
 * their own limit rather than a universal one -- and a skill a role cannot train
 * is simply absent for them.
 *
 * @note Separated from you.h so that objects.c can include it: an object needs to
 *       name the skill it uses, and objects.c must not pull in the hero.
 * @warning The weapon ordering is used arithmetically, and the sign carries
 *          meaning -- positive for weapons held or fired, negative for the
 *          ammunition they fire -- so these values cannot be reordered without
 *          rewriting weapon.c, as the existing comment states.
 */

/**
 * @file skills.h
 * @brief 영웅이 숙련될 수 있는 것들.
 *
 * 기술은 개별 물건이 아니라 무기 계열이나 마법 계열을 다룬다. 그래서 검을 다른 검으로 바꿔도
 * 잃는 것이 없지만, 익숙하지 않은 종류를 들면 명중이 나빠진다.
 *
 * 직업마다 기술별 상한이 따로 있으므로, 숙련의 상승은 영웅을 보편적 한계가 아니라 자기
 * 자신의 한계 쪽으로 옮긴다. 어떤 직업이 익힐 수 없는 기술은 그에게 아예 존재하지 않는다.
 *
 * @note objects.c 가 포함할 수 있도록 you.h 에서 분리했다. 물건은 자신이 쓰는 기술을 지칭해야
 *       하지만, objects.c 가 영웅을 끌어와서는 안 된다.
 * @warning 무기 순서는 산술적으로 쓰이며 부호가 의미를 지닌다. 들거나 발사하는 무기는 양수,
 *          그것이 발사하는 탄약은 음수다. 그래서 기존 주석이 밝히듯 weapon.c 를 다시 쓰지
 *          않고서는 이 값들의 순서를 바꿀 수 없다.
 */

#ifndef SKILLS_H
#define SKILLS_H

/* Much of this code was taken from you.h.  It is now
 * in a separate file so it can be included in objects.c.
 */

/**
 * @brief Every skill, weapons and spell schools together.
 * @note Also serves as an object's subtype, which is how an object declares which
 *       skill applies to it.
 * @warning @c P_NONE is not a skill but the answer "none applies", so it must be
 *          excluded from any loop over skills.
 */
/**
 * @brief 모든 기술. 무기와 주문 계열을 함께 담는다.
 * @note 물건의 하위 타입으로도 쓰인다. 물건이 자신에게 어떤 기술이 적용되는지 선언하는
 *       방식이다.
 * @warning @c P_NONE 은 기술이 아니라 "해당 없음"이라는 답이다. 기술을 순회하는 반복문에서는
 *          제외해야 한다.
 */
enum p_skills {
    /* Code to denote that no skill is applicable */
    P_NONE = 0,

/* Weapon Skills -- Stephen White
 * Order matters and are used in macros.
 * Positive values denote hand-to-hand weapons or launchers.
 * Negative values denote ammunition or missiles.
 * Update weapon.c if you amend any skills.
 * Also used for oc_subtyp.
 */
    P_DAGGER             =  1,
    P_KNIFE              =  2,
    P_AXE                =  3,
    P_PICK_AXE           =  4,
    P_SHORT_SWORD        =  5,
    P_BROAD_SWORD        =  6,
    P_LONG_SWORD         =  7,
    P_TWO_HANDED_SWORD   =  8,
    P_SABER              =  9, /* Curved sword, includes scimitar */
    P_CLUB               = 10, /* Heavy-shafted bludgeon */
    P_MACE               = 11,
    P_MORNING_STAR       = 12, /* Spiked bludgeon */
    P_FLAIL              = 13, /* Two pieces hinged or chained together */
    P_HAMMER             = 14, /* Heavy head on the end */
    P_QUARTERSTAFF       = 15, /* Long-shafted bludgeon */
    P_POLEARMS           = 16, /* attack two or three steps away */
    P_SPEAR              = 17, /* includes javelin */
    P_TRIDENT            = 18,
    P_LANCE              = 19,
    P_BOW                = 20, /* launchers */
    P_SLING              = 21,
    P_CROSSBOW           = 22,
    P_DART               = 23, /* hand-thrown missiles */
    P_SHURIKEN           = 24,
    P_BOOMERANG          = 25,
    P_WHIP               = 26, /* flexible, one-handed */
    P_UNICORN_HORN       = 27, /* last weapon, two-handed */

    /* Spell Skills added by Larry Stewart-Zerba */
    P_ATTACK_SPELL       = 28,
    P_HEALING_SPELL      = 29,
    P_DIVINATION_SPELL   = 30,
    P_ENCHANTMENT_SPELL  = 31,
    P_CLERIC_SPELL       = 32,
    P_ESCAPE_SPELL       = 33,
    P_MATTER_SPELL       = 34,

    /* Other types of combat */
    P_BARE_HANDED_COMBAT = 35, /* actually weaponless; gloves are ok */
    P_TWO_WEAPON_COMBAT  = 36, /* pair of weapons, one in each hand */
    P_RIDING             = 37, /* How well you control your steed */

    P_NUM_SKILLS         = 38
};

#define P_MARTIAL_ARTS P_BARE_HANDED_COMBAT /* Role distinguishes */

#define P_FIRST_WEAPON P_DAGGER
#define P_LAST_WEAPON P_UNICORN_HORN

#define P_FIRST_SPELL P_ATTACK_SPELL
#define P_LAST_SPELL P_MATTER_SPELL

#define P_LAST_H_TO_H P_RIDING
#define P_FIRST_H_TO_H P_BARE_HANDED_COMBAT

/* These roles qualify for a martial arts bonus */
#define martial_bonus() (Role_if(PM_SAMURAI) || Role_if(PM_MONK))

/*
 * These are the standard weapon skill levels.  It is important that
 * the lowest "valid" skill be 1.  The code calculates the
 * previous amount to practice by calling  practice_needed_to_advance()
 * with the current skill-1.  To work out for the UNSKILLED case,
 * a value of 0 needed.
 */
enum skill_levels {
    P_ISRESTRICTED = 0, /* unskilled and can't be advanced */
    P_UNSKILLED    = 1, /* unskilled so far but can be advanced */
    /* Skill levels Basic/Advanced/Expert had long been used by
       Heroes of Might and Magic (tm) and its sequels... */
    P_BASIC        = 2,
    P_SKILLED      = 3,
    P_EXPERT       = 4,
    /* when the skill system was adopted into nethack, levels beyond expert
       were unnamed and just used numbers.  Devteam coined them Master and
       Grand Master.  Sometime after that, Heroes of Might and Magic IV (tm)
       was released and had two more levels which use these same names. */
    P_MASTER       = 5, /* Unarmed combat/martial arts only */
    P_GRAND_MASTER = 6  /* ditto */
};

#define practice_needed_to_advance(level) ((level) * (level) *20)

/* The hero's skill in various weapons. */
struct skills {
    xint16 skill;
    xint16 max_skill;
    unsigned short advance;
};

#define P_SKILL(type) (u.weapon_skills[type].skill)
#define P_MAX_SKILL(type) (u.weapon_skills[type].max_skill)
#define P_ADVANCE(type) (u.weapon_skills[type].advance)
#define P_RESTRICTED(type) (u.weapon_skills[type].skill == P_ISRESTRICTED)

#define P_SKILL_LIMIT 60 /* Max number of skill advancements */

/* Initial skill matrix structure; used in u_init.c and weapon.c */
struct def_skill {
    xint16 skill;
    xint16 skmax;
};

#endif /* SKILLS_H */
