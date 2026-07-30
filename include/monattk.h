/* NetHack 5.0	monattk.h	$NHDT-Date: 1781973083 2026/06/20 16:31:23 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.23 $ */
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */

/**
 * @file monattk.h
 * @brief How a creature strikes, and what its blow does.
 *
 * Two independent vocabularies. The attack type says the manner -- a bite, a
 * gaze, a breath, an engulf -- and the damage type says the consequence --
 * physical harm, fire, paralysis, theft. A species pairs them, so the same
 * consequence can arrive by very different means.
 *
 * @warning Both orderings are load-bearing, and the existing comments say why:
 *          attack types past @c AT_BUTT are worth extra experience, so their
 *          position is a rule; damage types 1 through 10 are the ray types used
 *          by @c buzz(), so reordering them would require rewriting it.
 * @note @c AT_ANY and @c AD_ANY are wildcards for asking questions, not things a
 *       creature can have.
 */

/**
 * @file monattk.h
 * @brief 생물이 어떻게 때리는가, 그리고 그 타격이 무엇을 하는가.
 *
 * 서로 독립된 두 어휘다. 공격 종류는 방식을 말한다. 물기, 응시, 브레스, 삼키기다. 피해
 * 종류는 결과를 말한다. 물리적 상해, 불, 마비, 절도다. 종이 이 둘을 짝지으므로, 같은
 * 결과가 전혀 다른 방식으로 도달할 수 있다.
 *
 * @warning 두 순서 모두 동작을 좌우하며, 기존 주석이 이유를 밝힌다. @c AT_BUTT 이후의
 *          공격 종류는 추가 경험치를 주므로 그 위치가 하나의 규칙이다. 피해 종류 1부터
 *          10까지는 @c buzz() 가 쓰는 광선 종류이므로, 순서를 바꾸면 그것을 다시 써야
 *          한다.
 * @note @c AT_ANY 와 @c AD_ANY 는 질문을 위한 와일드카드이며, 생물이 실제로 가질 수 있는
 *       것이 아니다.
 */

#ifndef MONATTK_H
#define MONATTK_H

/**
 * @brief The manner in which a creature attacks.
 * @note Some are not actions the creature takes: @c AT_NONE marks a passive
 *       creature that only harms what touches it, @c AT_BOOM fires when the
 *       creature dies, and @c AT_EXPL on proximity.
 * @note @c AT_WEAP and @c AT_MAGC sit far from the rest because they mean "use
 *       what it is carrying" and "cast", so the actual effect comes from
 *       elsewhere.
 */
/**
 * @brief 생물이 공격하는 방식.
 * @note 일부는 생물이 취하는 행동이 아니다. @c AT_NONE 은 자신에게 닿은 것만 해치는 수동적
 *       생물을 표시하고, @c AT_BOOM 은 그 생물이 죽을 때, @c AT_EXPL 은 근접했을 때
 *       발동한다.
 * @note @c AT_WEAP 과 @c AT_MAGC 가 나머지와 멀리 떨어져 있는 것은 그것들이 "지닌 것을
 *       쓴다"와 "시전한다"를 뜻하기 때문이다. 실제 효과는 다른 곳에서 온다.
 */
/*      Add new attack types below - ordering affects experience (exper.c).
 *      Attacks > AT_BUTT are worth extra experience.
 */
#define AT_ANY (-1) /* fake attack; dmgtype_fromattack wildcard */
#define AT_NONE 0   /* passive monster (ex. acid blob) */
#define AT_CLAW 1   /* claw (punch, hit, etc.) */
#define AT_BITE 2   /* bite */
#define AT_KICK 3   /* kick */
#define AT_BUTT 4   /* head butt (ex. a unicorn) */
#define AT_TUCH 5   /* touches */
#define AT_STNG 6   /* sting */
#define AT_HUGS 7   /* crushing bearhug */
#define AT_SPIT 10  /* spits substance - ranged */
#define AT_ENGL 11  /* engulf (swallow or by a cloud) */
#define AT_BREA 12  /* breath - ranged */
#define AT_EXPL 13  /* explodes - proximity */
#define AT_BOOM 14  /* explodes when killed */
#define AT_GAZE 15  /* gaze - ranged */
#define AT_TENT 16  /* tentacles */

#define AT_WEAP 254 /* uses weapon */
#define AT_MAGC 255 /* uses magic spell(s) */

/**
 * @brief Whether an attack reaches beyond the adjacent square.
 * @param atyp An attack type.
 * @retval TRUE  It is a ranged attack -- spit, breath, gaze or spell.
 * @retval FALSE It requires contact.
 * @note Argument is evaluated several times, so it must be a simple expression.
 */
/**
 * @brief 어떤 공격이 인접한 칸을 넘어 닿는지.
 * @param atyp 공격 종류.
 * @retval TRUE  원거리 공격이다. 뱉기, 브레스, 응시, 주문 중 하나다.
 * @retval FALSE 접촉이 필요하다.
 * @note 인자가 여러 번 평가되므로 단순한 식이어야 한다.
 */
#define DISTANCE_ATTK_TYPE(atyp) ((atyp) == AT_SPIT \
                                  || (atyp) == AT_BREA \
                                  || (atyp) == AT_MAGC \
                                  || (atyp) == AT_GAZE)

/*      Add new damage types below.
 *
 *      Note that 1-10 correspond to the types of attack used in buzz().
 *      Please don't disturb the order unless you rewrite the buzz() code.
 */
#define AD_ANY (-1) /* fake damage; attacktype_fordmg wildcard */
#define AD_PHYS 0   /* ordinary physical */
#define AD_MAGM 1   /* magic missiles */
#define AD_FIRE 2   /* fire damage */
#define AD_COLD 3   /* frost damage */
#define AD_SLEE 4   /* sleep ray */
#define AD_DISN 5   /* disintegration (death ray) */
#define AD_ELEC 6   /* shock damage */
#define AD_DRST 7   /* drains str (poison) */
#define AD_ACID 8   /* acid damage */
#define AD_SPC1 9   /* for extension of buzz() */
#define AD_SPC2 10  /* for extension of buzz() */
#define AD_BLND 11  /* blinds (yellow light) */
#define AD_STUN 12  /* stuns */
#define AD_SLOW 13  /* slows */
#define AD_PLYS 14  /* paralyzes */
#define AD_DRLI 15  /* drains life levels (Vampire) */
#define AD_DREN 16  /* drains magic energy */
#define AD_LEGS 17  /* damages legs (xan) */
#define AD_STON 18  /* petrifies (Medusa, cockatrice) */
#define AD_STCK 19  /* sticks to you (mimic) */
#define AD_SGLD 20  /* steals gold (leppie) */
#define AD_SITM 21  /* steals item (nymphs) */
#define AD_SEDU 22  /* seduces & steals multiple items */
#define AD_TLPT 23  /* teleports you (Quantum Mech.) */
#define AD_RUST 24  /* rusts armour (Rust Monster)*/
#define AD_CONF 25  /* confuses (Umber Hulk) */
#define AD_DGST 26  /* digests opponent (trapper, etc.) */
#define AD_HEAL 27  /* heals opponent's wounds (nurse) */
#define AD_WRAP 28  /* special "stick" for eels */
#define AD_WERE 29  /* confers lycanthropy */
#define AD_DRDX 30  /* drains dexterity (quasit) */
#define AD_DRCO 31  /* drains constitution */
#define AD_DRIN 32  /* drains intelligence (mind flayer) */
#define AD_DISE 33  /* confers diseases */
#define AD_DCAY 34  /* decays organics (brown Pudding) */
#define AD_SSEX 35  /* Succubus seduction (extended) */
#define AD_HALU 36  /* causes hallucination */
#define AD_DETH 37  /* for Death only */
#define AD_PEST 38  /* for Pestilence only */
#define AD_FAMN 39  /* for Famine only */
#define AD_SLIM 40  /* turns you into green slime */
#define AD_ENCH 41  /* remove enchantment (disenchanter) */
#define AD_CORR 42  /* corrode armor (black pudding) */
#define AD_POLY 43  /* polymorph the target (genetic engineer) */

#define AD_CLRC 240 /* random clerical spell */
#define AD_SPEL 241 /* random magic spell */
#define AD_RBRE 242 /* random breath weapon */

#define AD_SAMU 252 /* hits, may steal Amulet (Wizard) */
#define AD_CURS 253 /* random curse (ex. gremlin) */

struct mhitm_data {
    int damage;
    int hitflags; /* M_ATTK_DEF_DIED | M_ATTK_AGR_DIED | ... */
    boolean done;
    boolean permdmg;
    int specialdmg;
    int dieroll;
};

/*
 *  Monster-to-monster attacks.  When a monster attacks another (mattackm),
 *  any or all of the following can be returned.  See mattackm() for more
 *  details.
 */
#define M_ATTK_MISS 0x0     /* aggressor missed */
#define M_ATTK_HIT 0x1      /* aggressor hit defender */
#define M_ATTK_DEF_DIED 0x2 /* defender died */
#define M_ATTK_AGR_DIED 0x4 /* aggressor died */
#define M_ATTK_AGR_DONE 0x8 /* aggressor is done with their turn */

#endif /* MONATTK_H */
