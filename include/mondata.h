/* NetHack 5.0	mondata.h	$NHDT-Date: 1781973083 2026/06/20 16:31:23 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.70 $ */
/* Copyright (c) 1989 Mike Threepoint                             */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file mondata.h
 * @brief Asking what a kind of monster is like.
 *
 * A monster's abilities live in the species table as flag bits, and the game asks
 * about them constantly -- can this fly, does it have hands, will it eat a corpse.
 * These macros are the vocabulary for those questions, so the rules read as English
 * and no caller has to remember which flag word a bit is in.
 *
 * Three sorts of question appear here, and the difference matters:
 *  - a flag test, which is a bit and is cheap;
 *  - a comparison against one or a few named species, used where a trait belongs to
 *    too few monsters to be worth a flag bit;
 *  - a comparison against a range of the species table, which works only because
 *    those species are adjacent in it.
 *
 * @note Every macro here takes a @c permonst pointer -- a kind of monster, not a
 *       monster. It answers what the species is capable of, never what this
 *       individual is currently doing. A flyer that has been grabbed still passes
 *       @c is_flyer.
 * @warning Arguments are evaluated more than once in nearly all of them, so an
 *          argument with a side effect will have it applied repeatedly.
 * @warning A few consult the hero -- their race, the level's ceiling -- so their
 *          answers are not properties of the monster alone and are not constant.
 */

/**
 * @file mondata.h
 * @brief 어떤 종류의 몬스터가 어떤 존재인지 묻기.
 *
 * 몬스터의 능력은 종족 표 안에 플래그 비트로 들어 있고, 게임은 그것을 끊임없이 묻는다. 이것은 날 수
 * 있는가, 손이 있는가, 시체를 먹는가. 이 매크로들은 그 질문들의 어휘다. 그래서 규칙이 영어처럼 읽히고,
 * 어느 호출자도 어떤 비트가 어느 플래그 워드에 있는지 기억할 필요가 없다.
 *
 * 여기에는 세 종류의 질문이 나오며, 그 차이가 중요하다.
 *  - 플래그 검사. 비트 하나이고 값이 싸다.
 *  - 하나 또는 몇 개의 이름 붙은 종족과의 비교. 어떤 특성이 플래그 비트를 하나 쓸 만큼 많은 몬스터에
 *    해당하지 않을 때 쓰인다.
 *  - 종족 표의 한 구간과의 비교. 그 종족들이 표 안에서 인접해 있기 때문에만 성립한다.
 *
 * @note 여기의 모든 매크로는 @c permonst 포인터를 받는다. 몬스터가 아니라 몬스터의 종류다. 종족이
 *       무엇을 할 수 있는지를 답하며, 이 개체가 지금 무엇을 하고 있는지는 결코 답하지 않는다. 붙잡힌
 *       비행 몬스터도 여전히 @c is_flyer 를 통과한다.
 * @warning 거의 전부에서 인자가 한 번 넘게 평가된다. 그래서 부작용이 있는 인자는 그 부작용이 거듭
 *          적용된다.
 * @warning 몇 개는 영웅의 종족이나 레벨의 천장을 참조한다. 그래서 그 답은 몬스터만의 속성이 아니며
 *          고정되어 있지 않다.
 */

#ifndef MONDATA_H
#define MONDATA_H

/* The macros in here take a permonst * as an argument */

/**
 * @def monsndx
 * @brief The species' own index in the species table.
 * @note Read from the entry rather than computed by subtracting from the table's
 *       start, so it stays correct for an entry that is not part of the table.
 */
/**
 * @def monsndx
 * @brief 종족 표 안에서 그 종족 자신의 색인.
 * @note 표의 시작에서 빼는 계산이 아니라 항목에서 읽는다. 그래서 표에 속하지 않은 항목에 대해서도
 *       올바르다.
 */
#define monsndx(ptr) ((ptr)->pmidx)
/** @def verysmall
 *  @brief Smaller than a small monster -- too small to wear armour meant for one. */
/** @def verysmall
 *  @brief 작은 몬스터보다도 작다. 그것을 위해 만든 갑옷을 입기에도 너무 작다. */
#define verysmall(ptr) ((ptr)->msize < MZ_SMALL)
/** @def bigmonst
 *  @brief Large or larger -- big enough that its bulk matters to the rules. */
/** @def bigmonst
 *  @brief 큰 것 이상. 그 부피가 규칙에 영향을 줄 만큼 크다. */
#define bigmonst(ptr) ((ptr)->msize >= MZ_LARGE)

/**
 * @def pm_resistance
 * @brief Whether the species resists a kind of harm.
 * @param ptr the kind of monster
 * @param typ one or more resistance bits
 * @note Passing several bits asks whether it resists @e any of them, not all.
 */
/**
 * @def pm_resistance
 * @brief 그 종족이 어떤 종류의 피해에 저항하는지.
 * @param ptr 몬스터의 종류
 * @param typ 하나 이상의 저항 비트
 * @note 여러 비트를 넘기면 그 중 @e 하나라도 저항하는지를 묻는 것이며, 전부인지를 묻는 것이 아니다.
 */
#define pm_resistance(ptr, typ) (((ptr)->mresists & (typ)) != 0)

#define immune_poisongas(ptr) ((ptr) == &mons[PM_HEZROU]        \
                               || (ptr) == &mons[PM_VROCK])

/**
 * @name Not touching the floor
 * @brief Three different ways of being off the ground, and the question they answer
 *        together.
 *
 * They are separate because they behave differently. A flyer chooses to be in the air
 * and can cross a pit. A floater hovers without steering, which is why it is
 * identified by class rather than a flag. A clinger holds the ceiling, so it is off
 * the floor only where there is a ceiling to hold.
 *
 * @note What almost all rules actually want is @c grounded -- whether the ground's
 *       hazards apply -- rather than any one of the three.
 * @{
 */
/**
 * @name 바닥에 닿지 않기
 * @brief 땅에서 떨어져 있는 세 가지 서로 다른 방식과, 그것들이 함께 답하는 질문.
 *
 * 서로 다르게 동작하므로 따로 있다. 비행하는 것은 스스로 공중에 있기를 택하며 구덩이를 건널 수 있다.
 * 떠 있는 것은 방향을 잡지 않고 부유하므로, 플래그가 아니라 계열로 식별된다. 매달리는 것은 천장을
 * 붙잡으므로, 붙잡을 천장이 있는 곳에서만 바닥에서 떨어져 있다.
 *
 * @note 거의 모든 규칙이 실제로 원하는 것은 셋 중 하나가 아니라 @c grounded, 즉 땅의 위험이
 *       적용되는지 여부다.
 * @{
 */
/** @def is_flyer
 *  @brief Flies under its own control. */
/** @def is_flyer
 *  @brief 스스로 조종하며 난다. */
#define is_flyer(ptr) (((ptr)->mflags1 & M1_FLY) != 0L)
/**
 * @def is_floater
 * @brief Hovers without steering.
 * @note Identified by monster class rather than a flag, because floating belongs to
 *       whole classes -- eyes and lights -- rather than to scattered species.
 */
/**
 * @def is_floater
 * @brief 방향을 잡지 않고 떠 있다.
 * @note 플래그가 아니라 몬스터 계열로 식별된다. 부유는 흩어진 종족들이 아니라 계열 전체 -- 눈과 빛 --
 *       에 속하기 때문이다.
 */
#define is_floater(ptr) ((ptr)->mlet == S_EYE || (ptr)->mlet == S_LIGHT)
/** @def is_clinger
 *  @brief Holds onto the ceiling, so does not fall through holes. */
/** @def is_clinger
 *  @brief 천장에 매달린다. 그래서 구멍으로 떨어지지 않는다. */
/* clinger: piercers, mimics, wumpus -- generally don't fall down holes */
#define is_clinger(ptr) (((ptr)->mflags1 & M1_CLING) != 0L)
/**
 * @def grounded
 * @brief Whether the ground's hazards apply to this kind of monster here.
 * @warning Reads the hero's current level to find out whether there is a ceiling, so
 *          the answer depends on where the hero is and is wrong for a monster
 *          elsewhere. A clinger is grounded on a level with no ceiling.
 */
/**
 * @def grounded
 * @brief 이 종류의 몬스터에게 이곳 땅의 위험이 적용되는지.
 * @warning 천장이 있는지 알아내기 위해 영웅의 현재 레벨을 읽는다. 그래서 답이 영웅이 어디 있는지에
 *          달려 있고, 다른 곳에 있는 몬스터에 대해서는 틀리다. 천장이 없는 레벨에서 매달리는 것은
 *          땅에 있는 것으로 취급된다.
 */
#define grounded(ptr) (!is_flyer(ptr) && !is_floater(ptr) \
                       && (!is_clinger(ptr) || !has_ceiling(&u.uz)))
/** @} */
#define is_swimmer(ptr) (((ptr)->mflags1 & M1_SWIM) != 0L)
#define breathless(ptr) (((ptr)->mflags1 & M1_BREATHLESS) != 0L)
#define amphibious(ptr) (((ptr)->mflags1 & M1_AMPHIBIOUS) != 0L)
#define cant_drown(ptr) (is_swimmer(ptr) || amphibious(ptr) || breathless(ptr))
#define passes_walls(ptr) (((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr) (((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr) ((ptr)->mlet == S_GHOST)
#define tunnels(ptr) (((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define needspick(ptr) (((ptr)->mflags1 & M1_NEEDPICK) != 0L)
/**
 * @def hides_under
 * @brief Hides beneath an object.
 * @warning Says only that the species can. It needs an object on the square to hide
 *          under, so a true answer is not a claim that it is hidden.
 */
/**
 * @def hides_under
 * @brief 물건 아래에 숨는다.
 * @warning 그 종족이 그럴 수 있다는 것만을 말한다. 숨으려면 그 칸에 아래로 들어갈 물건이 있어야
 *          하므로, 참이라는 답이 숨어 있다는 주장은 아니다.
 */
/* hides_under() requires an object at the location in order to hide */
#define hides_under(ptr) (((ptr)->mflags1 & M1_CONCEAL) != 0L)
/**
 * @def is_hider
 * @brief Conceals itself somehow.
 * @warning True for mimics, and they hide by looking like something else rather than
 *          by being marked undetected. So code that treats this as "will be found in
 *          the undetected flag" is wrong for them, as the existing comment warns.
 */
/**
 * @def is_hider
 * @brief 어떤 방식으로든 자신을 감춘다.
 * @warning 모방자에 대해서도 참이며, 그들은 미발견 표시가 붙는 방식이 아니라 다른 것처럼 보이는
 *          방식으로 숨는다. 그래서 이것을 "미발견 플래그에서 찾을 수 있다"로 취급하는 코드는 그들에
 *          대해 틀리다. 기존 주석이 경고하는 바다.
 */
/* is_hider() is True for mimics but when hiding they appear as something
   else rather than become mon->mundetected, so use is_hider() with care */
#define is_hider(ptr) (((ptr)->mflags1 & M1_HIDE) != 0L)
/* piercers cling to the ceiling; lurkers above are hiders but they fly
   so aren't classified as clingers; unfortunately mimics are classified
   as both hiders and clingers but have nothing to do with ceilings;
   wumpuses (not wumpi :-) cling but aren't hiders */
/**
 * @def ceiling_hider
 * @brief Hides on the ceiling specifically, so it drops on what walks beneath.
 * @note Cannot be a single flag because the two traits it combines do not line up:
 *       mimics cling and hide but have nothing to do with ceilings, and lurkers above
 *       hide on the ceiling while flying rather than clinging. Hence the explicit
 *       exclusion of mimics and the inclusion of flyers.
 */
/**
 * @def ceiling_hider
 * @brief 특히 천장에 숨는다. 그래서 그 아래를 지나는 것에게 떨어진다.
 * @note 결합하는 두 특성이 맞물리지 않으므로 단일 플래그가 될 수 없다. 모방자는 매달리고 숨지만
 *       천장과는 무관하고, 위에 숨은 것은 매달리는 것이 아니라 날면서 천장에 숨는다. 모방자를 명시적으로
 *       제외하고 비행하는 것을 포함하는 이유가 그것이다.
 */
#define ceiling_hider(ptr) \
    (is_hider(ptr) && ((is_clinger(ptr) && (ptr)->mlet != S_MIMIC) \
                       || is_flyer(ptr))) /* lurker above */
/**
 * @def haseyes
 * @brief Whether the species has eyes, and so can be blinded or met by a gaze.
 * @note The flag records the absence, so the test is inverted -- having eyes is the
 *       assumption and lacking them is the exception worth recording.
 */
/**
 * @def haseyes
 * @brief 그 종족에게 눈이 있는지. 그래서 눈이 멀 수 있고 응시에 마주칠 수 있는지.
 * @note 플래그가 없음을 기록하므로 검사가 뒤집혀 있다. 눈이 있는 것이 전제이고, 없는 것이 기록할 만한
 *       예외다.
 */
#define haseyes(ptr) (((ptr)->mflags1 & M1_NOEYES) == 0L)
/**
 * @def eyecount
 * @brief How many eyes to speak of: none, one, or two.
 * @note Exists for grammar, to decide whether a message says "eye" or "eyes". That is
 *       why anything with more than two eyes still answers two -- the extra ones make
 *       no difference to the sentence, as the existing comment notes.
 */
/**
 * @def eyecount
 * @brief 말할 때 쓸 눈의 수. 없음, 하나, 또는 둘.
 * @note 문법을 위해 존재한다. 메시지가 "눈"의 단수를 쓸지 복수를 쓸지 정한다. 눈이 둘보다 많은 것도
 *       여전히 둘로 답하는 이유가 그것이다. 기존 주석이 밝히듯 그 이상은 문장에 아무 차이를 만들지
 *       않는다.
 */
/* used to decide whether plural applies so no need for 'more than 2' */
#define eyecount(ptr) \
    (!haseyes(ptr) ? 0                                                     \
     : ((ptr) == &mons[PM_CYCLOPS] || (ptr) == &mons[PM_FLOATING_EYE]) ? 1 \
       : 2)
/**
 * @def nohands
 * @brief No hands, so it cannot wield, wear rings, or open a door.
 * @note Not the same as @c nolimbs -- a horse has limbs but no hands.
 */
/**
 * @def nohands
 * @brief 손이 없다. 그래서 무기를 들 수도, 반지를 낄 수도, 문을 열 수도 없다.
 * @note @c nolimbs 와 같지 않다. 말은 팔다리는 있으나 손은 없다.
 */
#define nohands(ptr) (((ptr)->mflags1 & M1_NOHANDS) != 0L)
/**
 * @def nolimbs
 * @brief No limbs at all, so armour for a body with limbs cannot go on.
 * @warning Tests for @e all of the bits rather than any: the flag is a combination,
 *          and lacking hands alone does not make a monster limbless.
 */
/**
 * @def nolimbs
 * @brief 팔다리가 전혀 없다. 그래서 팔다리 있는 몸을 위한 갑옷을 입힐 수 없다.
 * @warning 어느 하나가 아니라 비트 @e 전부를 검사한다. 그 플래그는 조합이며, 손이 없다는 것만으로
 *          팔다리가 없는 몬스터가 되지는 않는다.
 */
#define nolimbs(ptr) (((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#define notake(ptr) (((ptr)->mflags1 & M1_NOTAKE) != 0L)
#define has_head(ptr) (((ptr)->mflags1 & M1_NOHEAD) == 0L)
#define has_horns(ptr) (num_horns(ptr) > 0)
#define is_whirly(ptr) \
    ((ptr)->mlet == S_VORTEX || (ptr) == &mons[PM_AIR_ELEMENTAL])
#define flaming(ptr)                                                     \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || (ptr) == &mons[PM_FIRE_ELEMENTAL] || (ptr) == &mons[PM_SALAMANDER])
#define is_silent(ptr) ((ptr)->msound == MS_SILENT)
#define unsolid(ptr) (((ptr)->mflags1 & M1_UNSOLID) != 0L)
#define mindless(ptr) (((ptr)->mflags1 & M1_MINDLESS) != 0L)
#define humanoid(ptr) (((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr) (((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define slithy(ptr) (((ptr)->mflags1 & M1_SLITHY) != 0L)
#define is_wooden(ptr) ((ptr) == &mons[PM_WOOD_GOLEM])
#define thick_skinned(ptr) (((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define hug_throttles(ptr) ((ptr) == &mons[PM_ROPE_GOLEM])
#define digests(ptr) \
    (dmgtype_fromattack((ptr), AD_DGST, AT_ENGL) != 0) /* purple w*/
#define enfolds(ptr) \
    (dmgtype_fromattack((ptr), AD_WRAP, AT_ENGL) != 0) /* 't' */
#define slimeproof(ptr) \
    ((ptr) == &mons[PM_GREEN_SLIME] || flaming(ptr) || noncorporeal(ptr))
#define lays_eggs(ptr) (((ptr)->mflags1 & M1_OVIPAROUS) != 0L)
#define eggs_in_water(ptr) \
    (lays_eggs(ptr) && (ptr)->mlet == S_EEL && is_swimmer(ptr))
#define regenerates(ptr) (((ptr)->mflags1 & M1_REGEN) != 0L)
#define perceives(ptr) (((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define can_teleport(ptr) (((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr) (((ptr)->mflags1 & M1_TPORT_CNTRL) != 0L)
#define telepathic(ptr)                                                \
    ((ptr) == &mons[PM_FLOATING_EYE] || (ptr) == &mons[PM_MIND_FLAYER] \
     || (ptr) == &mons[PM_MASTER_MIND_FLAYER])
#define is_armed(ptr) attacktype(ptr, AT_WEAP)
#define acidic(ptr) (((ptr)->mflags1 & M1_ACID) != 0L)
#define poisonous(ptr) (((ptr)->mflags1 & M1_POIS) != 0L)
#define carnivorous(ptr) (((ptr)->mflags1 & M1_CARNIVORE) != 0L)
#define herbivorous(ptr) (((ptr)->mflags1 & M1_HERBIVORE) != 0L)
#define metallivorous(ptr) (((ptr)->mflags1 & M1_METALLIVORE) != 0L)
#define polyok(ptr) (((ptr)->mflags2 & M2_NOPOLY) == 0L)
#define is_shapeshifter(ptr) (((ptr)->mflags2 & M2_SHAPESHIFTER) != 0L)
#define is_undead(ptr) (((ptr)->mflags2 & M2_UNDEAD) != 0L)
#define is_were(ptr) (((ptr)->mflags2 & M2_WERE) != 0L)
#define is_elf(ptr) (((ptr)->mflags2 & M2_ELF) != 0L)
#define is_dwarf(ptr) (((ptr)->mflags2 & M2_DWARF) != 0L)
#define is_gnome(ptr) (((ptr)->mflags2 & M2_GNOME) != 0L)
#define is_orc(ptr) (((ptr)->mflags2 & M2_ORC) != 0L)
#define is_human(ptr) (((ptr)->mflags2 & M2_HUMAN) != 0L)
/**
 * @name Questions about the hero's race
 * @brief Whether a species is the hero's own kind, or one that kind gets on with.
 *
 * These read the hero's race, so they are not properties of the monster: the same
 * species answers differently for an elven hero and a dwarven one. That is how a
 * race's traditional friends and enemies are expressed without a table per race.
 *
 * @warning Not usable before the hero's race is chosen, and not meaningful when
 *          describing a monster in the abstract.
 * @{
 */
/**
 * @name 영웅의 종족에 관한 질문
 * @brief 어떤 종족이 영웅과 같은 부류인지, 또는 그 부류와 잘 지내는 부류인지.
 *
 * 이들은 영웅의 종족을 읽으므로 몬스터의 속성이 아니다. 같은 종족이 엘프 영웅과 드워프 영웅에게 다르게
 * 답한다. 종족마다 표를 두지 않고 그 종족의 전통적인 벗과 원수를 표현하는 방식이다.
 *
 * @warning 영웅의 종족이 정해지기 전에는 쓸 수 없고, 몬스터를 추상적으로 기술할 때는 의미가 없다.
 * @{
 */
/** @def your_race
 *  @brief The species is the hero's own kind. */
/** @def your_race
 *  @brief 그 종족이 영웅과 같은 부류다. */
#define your_race(ptr) (((ptr)->mflags2 & gu.urace.selfmask) != 0L)
#define is_bat(ptr)                                         \
    ((ptr) == &mons[PM_BAT] || (ptr) == &mons[PM_GIANT_BAT] \
     || (ptr) == &mons[PM_VAMPIRE_BAT])
/**
 * @def is_bird
 * @brief A bird: in the bat class but not actually a bat.
 * @note Birds have no class of their own -- they share the bats' -- so being a bird is
 *       defined by exclusion rather than by a flag.
 */
/**
 * @def is_bird
 * @brief 새. 박쥐 계열에 있으나 실제로 박쥐는 아닌 것.
 * @note 새에게는 자기 계열이 없다. 박쥐의 계열을 함께 쓴다. 그래서 새라는 것이 플래그가 아니라 배제로
 *       정의된다.
 */
#define is_bird(ptr) ((ptr)->mlet == S_BAT && !is_bat(ptr))
#define is_giant(ptr) (((ptr)->mflags2 & M2_GIANT) != 0L)
#define is_golem(ptr) ((ptr)->mlet == S_GOLEM)
#define is_domestic(ptr) (((ptr)->mflags2 & M2_DOMESTIC) != 0L)
#define is_demon(ptr) (((ptr)->mflags2 & M2_DEMON) != 0L)
#define is_mercenary(ptr) (((ptr)->mflags2 & M2_MERC) != 0L)
#define is_male(ptr) (((ptr)->mflags2 & M2_MALE) != 0L)
#define is_female(ptr) (((ptr)->mflags2 & M2_FEMALE) != 0L)
#define is_neuter(ptr) (((ptr)->mflags2 & M2_NEUTER) != 0L)
#define is_wanderer(ptr) (((ptr)->mflags2 & M2_WANDER) != 0L)
#define always_hostile(ptr) (((ptr)->mflags2 & M2_HOSTILE) != 0L)
#define always_peaceful(ptr) (((ptr)->mflags2 & M2_PEACEFUL) != 0L)
/** @def race_hostile
 *  @brief The hero's race is traditionally at odds with this species. */
/** @def race_hostile
 *  @brief 영웅의 종족이 전통적으로 이 종족과 대립한다. */
#define race_hostile(ptr) (((ptr)->mflags2 & gu.urace.hatemask) != 0L)
/** @def race_peaceful
 *  @brief The hero's race is traditionally on good terms with this species. */
/** @def race_peaceful
 *  @brief 영웅의 종족이 전통적으로 이 종족과 사이가 좋다. */
#define race_peaceful(ptr) (((ptr)->mflags2 & gu.urace.lovemask) != 0L)
/** @} */
#define extra_nasty(ptr) (((ptr)->mflags2 & M2_NASTY) != 0L)
#define strongmonst(ptr) (((ptr)->mflags2 & M2_STRONG) != 0L)
#define can_breathe(ptr) attacktype(ptr, AT_BREA)
#define cantwield(ptr) (nohands(ptr) || verysmall(ptr))
/**
 * @def could_twoweap
 * @brief Has more than one weapon attack, so a hero in this form may fight with two.
 * @note Counts matching attack slots rather than assuming which slots hold weapon
 *       attacks, because the old version assumed the first slot always did -- true of
 *       the table as it stood, but only by accident.
 * @warning Examines the first three attack slots only. A form whose second weapon
 *          attack sits further along would be missed; the existing comment records
 *          this as a deliberate trade against checking all six.
 */
/**
 * @def could_twoweap
 * @brief 무기 공격이 둘 이상이다. 그래서 이 형태의 영웅은 둘로 싸울 수 있다.
 * @note 어느 칸에 무기 공격이 들어 있는지 가정하지 않고 해당하는 공격 칸을 센다. 이전 방식은 첫 칸이
 *       항상 그렇다고 가정했는데, 당시 표에서는 사실이었지만 우연히 그랬을 뿐이다.
 * @warning 앞의 세 공격 칸만 살핀다. 두 번째 무기 공격이 그보다 뒤에 있는 형태는 놓친다. 기존 주석이
 *          여섯 칸을 다 검사하는 것에 대한 의도적인 절충으로 이를 기록하고 있다.
 */
#define could_twoweap(ptr) \
    ((  ((ptr)->mattk[0].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[1].aatyp == AT_WEAP)              \
      + ((ptr)->mattk[2].aatyp == AT_WEAP)  ) > 1)
#define cantweararm(ptr) (breakarm(ptr) || sliparm(ptr))
#define throws_rocks(ptr) (((ptr)->mflags2 & M2_ROCKTHROW) != 0L)
#define type_is_pname(ptr) (((ptr)->mflags2 & M2_PNAME) != 0L)
#define is_lord(ptr) (((ptr)->mflags2 & M2_LORD) != 0L)
#define is_prince(ptr) (((ptr)->mflags2 & M2_PRINCE) != 0L)
#define is_ndemon(ptr) \
    (is_demon(ptr) && (((ptr)->mflags2 & (M2_LORD | M2_PRINCE)) == 0L))
#define is_dlord(ptr) (is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr) (is_demon(ptr) && is_prince(ptr))
#define is_minion(ptr) (((ptr)->mflags2 & M2_MINION) != 0L)
#define likes_gold(ptr) (((ptr)->mflags2 & M2_GREEDY) != 0L)
#define likes_gems(ptr) (((ptr)->mflags2 & M2_JEWELS) != 0L)
#define likes_objs(ptr) (((ptr)->mflags2 & M2_COLLECT) != 0L || is_armed(ptr))
#define likes_magic(ptr) (((ptr)->mflags2 & M2_MAGIC) != 0L)
#define webmaker(ptr) \
    ((ptr) == &mons[PM_CAVE_SPIDER] || (ptr) == &mons[PM_GIANT_SPIDER])
#define is_unicorn(ptr) ((ptr)->mlet == S_UNICORN && likes_gems(ptr))
#define is_longworm(ptr)                                                   \
    (((ptr) == &mons[PM_BABY_LONG_WORM]) || ((ptr) == &mons[PM_LONG_WORM]) \
     || ((ptr) == &mons[PM_LONG_WORM_TAIL]))
#define is_covetous(ptr) (((ptr)->mflags3 & M3_COVETOUS))
#define infravision(ptr) (((ptr)->mflags3 & M3_INFRAVISION))
#define infravisible(ptr) (((ptr)->mflags3 & M3_INFRAVISIBLE))
#define is_displacer(ptr) (((ptr)->mflags3 & M3_DISPLACES) != 0L)
/**
 * @def is_mplayer
 * @brief One of the player-monsters -- a hero's role appearing as a monster.
 * @warning A pointer range comparison, so it holds only because those species are
 *          contiguous in the table. Moving one of them out of the run breaks this
 *          silently.
 */
/**
 * @def is_mplayer
 * @brief 플레이어 몬스터 중 하나. 영웅의 직업이 몬스터로 나타난 것.
 * @warning 포인터 범위 비교다. 그래서 그 종족들이 표 안에서 연속해 있기 때문에만 성립한다. 그 중 하나를
 *          연속 구간 밖으로 옮기면 조용히 깨진다.
 */
#define is_mplayer(ptr) \
    (((ptr) >= &mons[PM_ARCHEOLOGIST]) && ((ptr) <= &mons[PM_WIZARD]))
#define is_watch(ptr) \
    ((ptr) == &mons[PM_WATCHMAN] || (ptr) == &mons[PM_WATCH_CAPTAIN])
#define is_rider(ptr)                                      \
    ((ptr) == &mons[PM_DEATH] || (ptr) == &mons[PM_FAMINE] \
     || (ptr) == &mons[PM_PESTILENCE])
/**
 * @def is_placeholder
 * @brief An entry that exists to be a corpse rather than to be generated.
 *
 * A zombie or mummy leaves behind the remains of what it was, and what it was needs a
 * table entry even though nothing of that kind walks the dungeon. These are those
 * entries.
 *
 * @warning Not every species that looks like a bare race name is one: dwarves and
 *          gnomes are ordinary monsters, as the existing comment points out.
 */
/**
 * @def is_placeholder
 * @brief 생성되기 위해서가 아니라 시체가 되기 위해 존재하는 항목.
 *
 * 좀비나 미라는 자신이 무엇이었는지의 잔해를 남기고, 그 무엇이었는지는 그런 종류가 던전을 걸어 다니지
 * 않더라도 표 항목을 필요로 한다. 이들이 그 항목들이다.
 *
 * @warning 맨 종족 이름처럼 보이는 종족이 전부 그런 것은 아니다. 기존 주석이 지적하듯 드워프와 노움은
 *          평범한 몬스터다.
 */
/* note: placeholder monsters are used for corpses of zombies and mummies;
   PM_DWARF and PM_GNOME are normal monsters, not placeholders */
#define is_placeholder(ptr)                             \
    ((ptr) == &mons[PM_ORC] || (ptr) == &mons[PM_GIANT] \
     || (ptr) == &mons[PM_ELF] || (ptr) == &mons[PM_HUMAN])
/* return TRUE if the monster tends to revive */
#define is_reviver(ptr) (is_rider(ptr) || (ptr)->mlet == S_TROLL)
/* monsters whose corpses and statues need special handling;
   note that high priests and the Wizard of Yendor are flagged
   as unique even though they really aren't; that's ok here */
#define unique_corpstat(ptr) (((ptr)->geno & G_UNIQ) != 0)

/**
 * @def emits_light
 * @brief How far this kind of monster lights the dungeon, or zero for not at all.
 * @return the radius in squares, not a yes or no
 * @warning Despite reading like a test, this yields a distance. It happens that every
 *          non-zero answer is currently 1, so treating it as a boolean works today and
 *          would stop working the moment a brighter monster is added.
 * @note The ranges were cut to 1 for speed on the plane of fire; the existing comment
 *       notes that the reasoning dates from 1990s hardware and predates smoke blocking
 *       line of sight, so it may no longer hold.
 */
/**
 * @def emits_light
 * @brief 이 종류의 몬스터가 던전을 얼마나 멀리 밝히는지. 전혀 밝히지 않으면 0.
 * @return 칸 단위 반지름. 예/아니오가 아니다
 * @warning 검사처럼 읽히지만 거리를 낸다. 지금은 0이 아닌 답이 모두 1이므로 논리값으로 취급해도 오늘은
 *          통하고, 더 밝은 몬스터가 추가되는 순간 통하지 않게 된다.
 * @note 불의 평면에서의 속도 때문에 범위가 1로 줄었다. 기존 주석은 그 판단이 1990년대 하드웨어에서
 *       나왔고 연기가 시야를 막게 된 것보다 앞선다고 밝히고 있으므로, 더는 유효하지 않을 수 있다.
 */
/* this returns the light's range, or 0 if none; if we add more light-emitting
   monsters, we'll likely have to add a new light range field to mons[] */
#define emits_light(ptr)                                          \
    (((ptr)->mlet == S_LIGHT || (ptr) == &mons[PM_FLAMING_SPHERE] \
      || (ptr) == &mons[PM_SHOCKING_SPHERE]                       \
      || (ptr) == &mons[PM_BABY_GOLD_DRAGON]                      \
      || (ptr) == &mons[PM_FIRE_VORTEX])                          \
         ? 1                                                      \
         : ((ptr) == &mons[PM_FIRE_ELEMENTAL]                     \
            || (ptr) == &mons[PM_GOLD_DRAGON]) ? 1 : 0)
    /* [Note: the light ranges above were reduced to 1 for performance,
     *  otherwise screen updating on the plane of fire slowed to a crawl.
     *  Note too: that was with 1990s hardware and before fumarole smoke
     *  blocking line of sight was added, so might no longer be necessary.] */
#define likes_lava(ptr) \
    (ptr == &mons[PM_FIRE_ELEMENTAL] || ptr == &mons[PM_SALAMANDER])
#define pm_invisible(ptr) \
    ((ptr) == &mons[PM_STALKER] || (ptr) == &mons[PM_BLACK_LIGHT])

/* could probably add more */
#define likes_fire(ptr)                                                  \
    ((ptr) == &mons[PM_FIRE_VORTEX] || (ptr) == &mons[PM_FLAMING_SPHERE] \
     || likes_lava(ptr))

/**
 * @def touch_petrifies
 * @brief Touching this kind of monster or its remains turns flesh to stone.
 * @note Deliberately excludes Medusa, whose gaze petrifies but whose body is safe to
 *       handle.
 */
/**
 * @def touch_petrifies
 * @brief 이 종류의 몬스터나 그 잔해에 닿으면 살이 돌로 변한다.
 * @note 메두사를 의도적으로 제외한다. 그 응시는 석화시키지만 그 몸은 만져도 안전하다.
 */
#define touch_petrifies(ptr) \
    ((ptr) == &mons[PM_COCKATRICE] || (ptr) == &mons[PM_CHICKATRICE])
/**
 * @def flesh_petrifies
 * @brief Eating this kind of monster turns flesh to stone.
 * @note A wider question than @c touch_petrifies, and the difference is Medusa: safe to
 *       carry, fatal to eat. Code about handling wants the touch test; code about
 *       eating wants this one.
 */
/**
 * @def flesh_petrifies
 * @brief 이 종류의 몬스터를 먹으면 살이 돌로 변한다.
 * @note @c touch_petrifies 보다 넓은 질문이며, 그 차이가 메두사다. 지고 다니기는 안전하고 먹으면
 *       치명적이다. 다루는 것에 관한 코드는 접촉 검사를, 먹는 것에 관한 코드는 이것을 원한다.
 */
/* Medusa doesn't pass touch_petrifies() but does petrify if eaten */
#define flesh_petrifies(pm) (touch_petrifies(pm) || (pm) == &mons[PM_MEDUSA])

/**
 * @def passes_rocks
 * @brief Rock moves through this kind of monster, so a thrown stone does it no harm.
 * @note Requires being able to pass through walls @e and being solid, which is what
 *       excludes ghosts. Including them would have exempted them from missiles
 *       generally and needed a further exception for blessed stones, as the existing
 *       comment explains -- so the narrow definition is the point.
 */
/**
 * @def passes_rocks
 * @brief 돌이 이 종류의 몬스터를 통과한다. 그래서 던진 돌이 해를 입히지 않는다.
 * @note 벽을 통과할 수 있으면서 @e 동시에 실체가 있어야 한다. 유령이 제외되는 이유다. 유령을 포함하면
 *       기존 주석이 설명하듯 투사체 전반에서 면제되고 축복받은 돌에 대한 예외가 또 필요해진다. 좁은
 *       정의가 핵심인 것이다.
 */
/* missiles made of rocks don't harm these: xorns and earth elementals
   (but not ghosts and shades because that would impact all missile use
   and also require an exception for blessed rocks/gems/boulders) */
#define passes_rocks(ptr) (passes_walls(ptr) && !unsolid(ptr))

#define is_mind_flayer(ptr) \
    ((ptr) == &mons[PM_MIND_FLAYER] || (ptr) == &mons[PM_MASTER_MIND_FLAYER])

#define is_vampire(ptr) ((ptr)->mlet == S_VAMPIRE)

#define hates_light(ptr) ((ptr) == &mons[PM_GREMLIN])

/* used to vary a few messages */
#define weirdnonliving(ptr) (is_golem(ptr) || (ptr)->mlet == S_VORTEX)
#define nonliving(ptr) \
    (is_undead(ptr) || (ptr) == &mons[PM_MANES] || weirdnonliving(ptr))

/* no corpse (ie, blank scrolls) if killed by fire; special case instakill  */
#define completelyburns(ptr) \
    ((ptr) == &mons[PM_PAPER_GOLEM] || (ptr) == &mons[PM_STRAW_GOLEM])
#define completelyrots(ptr) \
    ((ptr) == &mons[PM_WOOD_GOLEM] || (ptr) == &mons[PM_LEATHER_GOLEM])
#define completelyrusts(ptr) ((ptr) == &mons[PM_IRON_GOLEM])

/**
 * @name Dietary conduct
 * @brief Whether eating this kind of monster breaks a vow.
 *
 * A player may undertake to eat no animal products, and the game must honour that
 * across corpses, tins, and being eaten alive by something that digests.
 *
 * @note Written as a list of monster classes with named exceptions rather than a flag
 *       because it follows from what a monster is made of, which the table already
 *       records indirectly. The existing comment concedes a flag might be cleaner.
 * @note @c vegetarian is the weaker vow and so is defined in terms of @c vegan --
 *       anything acceptable to a vegan is acceptable here.
 * @{
 */
/**
 * @name 식사 관련 계율
 * @brief 이 종류의 몬스터를 먹는 것이 맹세를 깨는지.
 *
 * 플레이어는 동물성 산물을 먹지 않기로 할 수 있고, 게임은 시체와 통조림, 그리고 소화하는 것에게 산 채로
 * 먹히는 경우까지 그것을 지켜야 한다.
 *
 * @note 플래그가 아니라 이름 붙은 예외가 딸린 몬스터 계열 목록으로 적혀 있다. 몬스터가 무엇으로 이루어져
 *       있는지에서 따라 나오고, 표가 이미 그것을 간접적으로 기록하기 때문이다. 기존 주석은 플래그가 더
 *       깔끔할 수 있음을 인정하고 있다.
 * @note @c vegetarian 은 더 약한 맹세이므로 @c vegan 을 이용해 정의된다. 비건에게 허용되는 것은 여기서도
 *       허용된다.
 * @{
 */
/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr)                                                 \
    ((ptr)->mlet == S_BLOB || (ptr)->mlet == S_JELLY               \
     || (ptr)->mlet == S_FUNGUS || (ptr)->mlet == S_VORTEX         \
     || (ptr)->mlet == S_LIGHT                                     \
     || ((ptr)->mlet == S_ELEMENTAL && (ptr) != &mons[PM_STALKER]) \
     || ((ptr)->mlet == S_GOLEM && (ptr) != &mons[PM_FLESH_GOLEM]  \
         && (ptr) != &mons[PM_LEATHER_GOLEM]) || noncorporeal(ptr))
#define vegetarian(ptr) \
    (vegan(ptr)         \
     || ((ptr)->mlet == S_PUDDING && (ptr) != &mons[PM_BLACK_PUDDING]))
/** @} */

/** @def corpse_eater
 *  @brief Will eat a corpse it finds, rather than only fresh prey. */
/** @def corpse_eater
 *  @brief 갓 잡은 먹이만이 아니라 발견한 시체도 먹는다. */
#define corpse_eater(ptr)                    \
    (ptr == &mons[PM_PURPLE_WORM]            \
     || ptr == &mons[PM_BABY_PURPLE_WORM]    \
     || ptr == &mons[PM_GHOUL]               \
     || ptr == &mons[PM_PIRANHA])

/**
 * @def befriend_with_obj
 * @brief Whether offering this object would win this kind of monster over.
 *
 * Each case is a balance decision rather than a fact about animals. Monkeys accept only
 * bananas, and only for taming, because otherwise food would be a cheap way to disarm
 * their theft attack. Horses accept food that is always vegetarian, which excludes
 * corpses and tins of vegetarian creatures -- so a lucky corpse is not a free steed.
 *
 * @param ptr the kind of monster
 * @param obj the object being offered
 * @warning Both arguments are evaluated several times.
 */
/**
 * @def befriend_with_obj
 * @brief 이 물건을 내주는 것이 이 종류의 몬스터의 마음을 얻는지.
 *
 * 각 경우는 동물에 관한 사실이 아니라 균형에 관한 결정이다. 원숭이는 바나나만, 그리고 길들이는 데만
 * 받아들인다. 그러지 않으면 음식이 그 도둑질 공격을 무력화하는 값싼 수단이 되기 때문이다. 말은 언제나
 * 채식인 음식만 받아들이며, 그것은 시체와 채식 생물의 통조림을 제외한다. 그래서 운 좋게 얻은 시체가 공짜
 * 탈것이 되지는 않는다.
 *
 * @param ptr 몬스터의 종류
 * @param obj 내주는 물건
 * @warning 두 인자 모두 여러 번 평가된다.
 */
/* monkeys are tamable via bananas but not pacifiable via food,
   otherwise their theft attack could be nullified too easily;
   dogs and cats can be tamed by anything they like to eat and are
   pacified by any other food;
   horses can be tamed by always-veggy food or lichen corpses but
   not tamed or pacified by other corpses or tins of veggy critters */
#define befriend_with_obj(ptr, obj) \
    (((ptr) == &mons[PM_MONKEY] || (ptr) == &mons[PM_APE])               \
     ? (obj)->otyp == BANANA                                             \
     : (is_domestic(ptr) && (obj)->oclass == FOOD_CLASS                  \
        && ((ptr)->mlet != S_UNICORN                                     \
            || objects[(obj)->otyp].oc_material == VEGGY                 \
            || ((obj)->otyp == CORPSE && (obj)->corpsenm == PM_LICHEN))))

/**
 * @def pmname
 * @brief The species' name for a given gender, falling back to the neutral one.
 * @param ptr the kind of monster
 * @param g the gender wanted
 * @return the name; never null, because the neutral name always exists
 * @note Most species have only the neutral name, so a gendered name is the exception
 *       and its absence is normal rather than an error.
 * @warning Defined only under @c PMNAME_MACROS; otherwise the function of the same name
 *          is used, so this must stay equivalent to it.
 */
/**
 * @def pmname
 * @brief 주어진 성별에 대한 그 종족의 이름. 없으면 중성 이름으로 돌아간다.
 * @param ptr 몬스터의 종류
 * @param g 원하는 성별
 * @return 이름. 중성 이름은 항상 존재하므로 결코 널이 아니다
 * @note 대부분의 종족은 중성 이름만 가진다. 그래서 성별 이름이 예외이고, 그것이 없는 것은 오류가 아니라
 *       정상이다.
 * @warning @c PMNAME_MACROS 아래에서만 정의된다. 그렇지 않으면 같은 이름의 함수가 쓰이므로, 이것은 그것과
 *          동등하게 유지되어야 한다.
 */
#ifdef PMNAME_MACROS
#define pmname(ptr,g) ((((g) == MALE || (g) == FEMALE) && (ptr)->pmnames[g]) \
                        ? (ptr)->pmnames[g] : (ptr)->pmnames[NEUTRAL])
#endif
/**
 * @def monsym
 * @brief The character this kind of monster is drawn as by default.
 * @warning Reads the default table, not the one in use, so a player's symbol changes
 *          are not reflected. Use it to speak about a monster class, not to draw one.
 */
/**
 * @def monsym
 * @brief 이 종류의 몬스터가 기본으로 그려지는 문자.
 * @warning 사용 중인 표가 아니라 기본 표를 읽는다. 그래서 플레이어의 심볼 변경이 반영되지 않는다. 그리기
 *          위해서가 아니라 몬스터 계열에 대해 말하기 위해 쓴다.
 */
#define monsym(ptr) (def_monsyms[(int) (ptr)->mlet].sym)

#endif /* MONDATA_H */
