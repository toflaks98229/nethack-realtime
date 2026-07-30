/* NetHack 5.0	monflag.h	$NHDT-Date: 1781973083 2026/06/20 16:31:23 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.33 $ */
/* Copyright (c) 1989 Mike Threepoint                             */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file monflag.h
 * @brief The vocabulary the species table is written in.
 *
 * Every monster in the game is one row of a table, and this file defines what may
 * appear in that row: what noise it makes, what harm it shrugs off, what it can do,
 * what it is, how big it is, and how the game is allowed to create it.
 *
 * The flags are grouped into several words rather than one because there are more than
 * a machine word can hold. The split is historical rather than thematic, which is why
 * asking a question about a monster is done through the named tests in mondata.h --
 * they know which word a bit lives in so the rules do not have to.
 *
 * @note Some values are deliberately combinations of others: a flag meaning "no limbs
 *       at all" is the two flags for no hands and no feet together, and "omnivore" is
 *       both diets. Testing those requires comparing against the whole value, not
 *       asking whether any bit is set.
 * @note The @c MR_ resistances are ordered to match the first eight hero properties, so
 *       one can be converted into the other; prop.h holds that conversion.
 * @warning A flag word is full when its top bit is used. Adding a trait to a full word
 *          means either finding a combination that is genuinely redundant or widening
 *          the field -- and the field's width is part of the save format.
 */

/**
 * @file monflag.h
 * @brief 종족 표가 적혀 있는 어휘.
 *
 * 게임의 모든 몬스터는 표의 한 줄이며, 이 파일은 그 줄에 무엇이 나올 수 있는지를 정의한다. 어떤 소리를
 * 내는지, 어떤 피해를 떨쳐내는지, 무엇을 할 수 있는지, 무엇인지, 얼마나 큰지, 게임이 그것을 어떻게 만들어도
 * 되는지.
 *
 * 플래그는 하나가 아니라 여러 워드로 나뉘어 있다. 기계어 워드 하나가 담을 수 있는 것보다 많기 때문이다. 그
 * 분할은 주제별이 아니라 역사적인 것이고, 그래서 몬스터에 대한 질문은 mondata.h 의 이름 붙은 검사를 통해
 * 이뤄진다. 그것들이 어떤 비트가 어느 워드에 사는지 알고 있으므로 규칙은 알 필요가 없다.
 *
 * @note 일부 값은 의도적으로 다른 값들의 조합이다. "팔다리가 전혀 없음"을 뜻하는 플래그는 손 없음과 발
 *       없음의 두 플래그를 합친 것이고, "잡식"은 두 식성을 합친 것이다. 그것들을 검사하려면 어느 비트가
 *       켜졌는지를 묻는 것이 아니라 값 전체와 비교해야 한다.
 * @note @c MR_ 저항들은 영웅의 앞 여덟 속성과 대응하도록 정렬되어 있어 서로 변환할 수 있다. 그 변환은
 *       prop.h 에 있다.
 * @warning 플래그 워드는 최상위 비트가 쓰이면 가득 찬 것이다. 가득 찬 워드에 특성을 더하려면 정말로 남는
 *          조합을 찾거나 필드를 넓혀야 하며, 필드의 폭은 저장 형식의 일부다.
 */

#ifndef MONFLAG_H
#define MONFLAG_H
/* clang-format off */
/* *INDENT-OFF* */

/**
 * @brief What a monster says or sounds like.
 *
 * Not merely flavour: the value decides which conversation a monster can hold, so a
 * shopkeeper's and a priest's differ here and not in their species. Everything the hero
 * can understand sits above the animal noises, and the boundary is marked so code can
 * ask "is this speech" rather than list the cases.
 *
 * @note @c MS_ANIMAL is not a sound but the mark of where animal noises end; it shares
 *       a value with the last of them.
 * @warning The ordering carries that meaning, so these cannot be sorted or renumbered.
 *          The existing comment notes one value that is already on the wrong side of the
 *          line: a grunt may be a monster speaking its own language.
 */
/**
 * @brief 몬스터가 무엇을 말하는지, 또는 어떤 소리를 내는지.
 *
 * 단순한 분위기가 아니다. 이 값이 몬스터가 어떤 대화를 할 수 있는지를 정하므로, 상점 주인과 사제의 차이는
 * 종족이 아니라 여기서 갈린다. 영웅이 이해할 수 있는 것은 모두 동물 소리보다 위에 놓이고, 그 경계가 표시되어
 * 있어 코드가 경우들을 열거하지 않고 "이것이 말인가"를 물을 수 있다.
 *
 * @note @c MS_ANIMAL 은 소리가 아니라 동물 소리가 끝나는 지점의 표시다. 그 마지막 것과 값을 공유한다.
 * @warning 순서가 그 의미를 지니므로 정렬하거나 번호를 다시 매길 수 없다. 기존 주석은 이미 경계의 잘못된
 *          쪽에 있는 값 하나를 밝히고 있다. 으르렁거림은 몬스터가 자기 언어로 말하는 것일 수 있다.
 */
enum ms_sounds {
    MS_SILENT   =  0,   /* makes no sound */
    MS_BARK     =  1,   /* if full moon, may howl */
    MS_MEW      =  2,   /* mews or hisses */
    MS_ROAR     =  3,   /* roars */
    MS_BELLOW   =  4,   /* adult male crocodiles; hatchlings 'chirp' */
    MS_GROWL    =  5,   /* growls */
    MS_SQEEK    =  6,   /* squeaks, as a rodent */
    MS_SQAWK    =  7,   /* squawks, as a bird */
    MS_CHIRP    =  8,   /* baby crocodile */
    MS_HISS     =  9,   /* hisses */
    MS_BUZZ     = 10,   /* buzzes (killer bee) */
    MS_GRUNT    = 11,   /* grunts (or speaks own language) */
    MS_NEIGH    = 12,   /* neighs, as an equine */
    MS_MOO      = 13,   /* minotaurs, rothes */
    MS_WAIL     = 14,   /* wails, as a tortured soul */
    MS_GURGLE   = 15,   /* gurgles, as liquid or through saliva */
    MS_BURBLE   = 16,   /* burbles (jabberwock) */
    MS_TRUMPET  = 17,   /* trumpets (elephant) */
    MS_ANIMAL   = 17,   /* up to here are animal noises */
    /* FIXME? the grunt "speaks own language" case above
       shouldn't be classified as animal */
    MS_SHRIEK   = 18,   /* wakes up others */
    MS_BONES    = 19,   /* rattles bones (skeleton) */
    MS_LAUGH    = 20,   /* grins, smiles, giggles, and laughs */
    MS_MUMBLE   = 21,   /* says something or other */
    MS_IMITATE  = 22,   /* imitates others (leocrotta) */
    MS_WERE     = 23,   /* lycanthrope in human form */
    MS_ORC      = 24,   /* intelligent brutes */
    /* from here onward, speech can be comprehended */
    MS_HUMANOID = 25,   /* generic traveling companion */
    MS_ARREST   = 26,   /* "Stop in the name of the law!" (Kops) */
    MS_SOLDIER  = 27,   /* army and watchmen expressions */
    MS_GUARD    = 28,   /* "Please drop that gold and follow me." */
    MS_DJINNI   = 29,   /* "Thank you for freeing me!" */
    MS_NURSE    = 30,   /* "Take off your shirt, please." */
    MS_SEDUCE   = 31,   /* "Hello, sailor." (Nymphs) */
    MS_VAMPIRE  = 32,   /* vampiric seduction, Vlad's exclamations */
    MS_BRIBE    = 33,   /* asks for money, or berates you */
    MS_CUSS     = 34,   /* berates (demons) or intimidates (Wiz) */
    MS_RIDER    = 35,   /* astral level special monsters */
    MS_LEADER   = 36,   /* your class leader */
    MS_NEMESIS  = 37,   /* your nemesis */
    MS_GUARDIAN = 38,   /* your leader's guards */
    MS_SELL     = 39,   /* demand payment, complain about shoplifters */
    MS_ORACLE   = 40,   /* do a consultation */
    MS_PRIEST   = 41,   /* ask for contribution; do cleansing */
    MS_SPELL    = 42,   /* spellcaster not matching any of the above */
    MS_BOAST    = 43,   /* giants */
    MS_GROAN    = 44,   /* zombies groan */
};

/**
 * @name Resistances a corpse can convey
 * @brief What the species shrugs off -- and what eating it may grant.
 *
 * These serve two purposes at once, which is why they are ordered as they are: they say
 * what harms a monster, and they say what the hero may gain from its corpse. Their order
 * matches the first eight hero properties so the two can be converted into each other
 * with @c res_to_mr in prop.h.
 *
 * @warning Reordering these silently changes which property a corpse conveys.
 * @{
 */
/**
 * @name 시체가 전할 수 있는 저항
 * @brief 그 종족이 무엇을 떨쳐내는지, 그리고 그것을 먹으면 무엇을 얻을 수 있는지.
 *
 * 이들은 두 가지 일을 동시에 한다. 그것이 이런 순서인 이유다. 무엇이 몬스터에게 해를 입히는지를 말하고,
 * 영웅이 그 시체에서 무엇을 얻을 수 있는지를 말한다. 순서가 영웅의 앞 여덟 속성과 대응하므로 prop.h 의
 * @c res_to_mr 로 서로 변환할 수 있다.
 *
 * @warning 이들의 순서를 바꾸면 시체가 어떤 속성을 전하는지가 조용히 바뀐다.
 * @{
 */
#define MR_FIRE         0x01 /* resists fire */
#define MR_COLD         0x02 /* resists cold */
#define MR_SLEEP        0x04 /* resists sleep */
#define MR_DISINT       0x08 /* resists disintegration */
#define MR_ELEC         0x10 /* resists electricity */
#define MR_POISON       0x20 /* resists poison */
#define MR_ACID         0x40 /* resists acid */
#define MR_STONE        0x80 /* resists petrification */
/* NB: the above resistances correspond to the first 8 hero properties in
   prop_types (FIRE_RES through STONE_RES), which can be converted to their
   MR_foo equivalents with the macro res_to_mr() defined in prop.h */
/** @} */
/* other resistances: magic, sickness */
/* other conveyances: teleport, teleport control, telepathy */

/**
 * @name Abilities an individual monster holds
 * @brief Things that are true of this monster rather than of its kind.
 *
 * A monster may float because it put on a ring, or see the invisible because it was
 * created that way. These live in the same numbering as the resistances above but occupy
 * the higher bits, so an individual's abilities and its species' resistances can be
 * carried together.
 *
 * @note @c MR2_FUMBLING is an affliction rather than an ability; it shares this space
 *       because it too belongs to the individual.
 * @{
 */
/**
 * @name 개별 몬스터가 지닌 능력
 * @brief 그 종류가 아니라 이 몬스터에게 참인 것들.
 *
 * 몬스터는 반지를 껴서 떠 있을 수도, 그렇게 만들어져서 투명한 것을 볼 수도 있다. 이들은 위의 저항들과 같은
 * 번호 체계에 있으면서 더 높은 비트를 차지한다. 그래서 개체의 능력과 그 종족의 저항을 함께 지닐 수 있다.
 *
 * @note @c MR2_FUMBLING 은 능력이 아니라 결함이다. 그것도 개체에 속하기 때문에 이 공간을 함께 쓴다.
 * @{
 */
/* individual resistances */
#define MR2_SEE_INVIS   0x0100 /* see invisible */
#define MR2_LEVITATE    0x0200 /* levitation */
#define MR2_WATERWALK   0x0400 /* water walking */
#define MR2_MAGBREATH   0x0800 /* magical breathing */
#define MR2_DISPLACED   0x1000 /* displaced */
#define MR2_STRENGTH    0x2000 /* gauntlets of power */
#define MR2_FUMBLING    0x4000 /* clumsy */
/** @} */

/**
 * @name What a monster can do and what it is made of
 * @brief The first flag word: movement, senses, body, and diet.
 *
 * These describe the creature itself -- how it gets about, what it can perceive, what
 * parts it has, what it will eat, and what eating it does to the eater.
 *
 * @note Three values here are combinations and must be compared as wholes: @c M1_NOLIMBS
 *       is the no-hands and no-legs bits together, and @c M1_OMNIVORE is both diets.
 *       Treating them as single bits gives the wrong answer for a monster with only one
 *       of the pair.
 * @warning This word is full -- @c M1_METALLIVORE uses its top bit, which is why it needs
 *          an unsigned spelling on standard compilers.
 * @{
 */
/**
 * @name 몬스터가 무엇을 할 수 있고 무엇으로 이루어져 있는지
 * @brief 첫 번째 플래그 워드. 이동, 감각, 신체, 식성.
 *
 * 이들은 생물 자체를 기술한다. 어떻게 돌아다니는지, 무엇을 지각할 수 있는지, 어떤 부위를 가졌는지, 무엇을
 * 먹는지, 그리고 그것을 먹은 자에게 무슨 일이 일어나는지.
 *
 * @note 여기서 세 값은 조합이며 전체로 비교해야 한다. @c M1_NOLIMBS 는 손 없음과 다리 없음 비트를 합친
 *       것이고, @c M1_OMNIVORE 는 두 식성을 합친 것이다. 그것들을 단일 비트로 취급하면 짝 중 하나만 가진
 *       몬스터에 대해 틀린 답이 나온다.
 * @warning 이 워드는 가득 찼다. @c M1_METALLIVORE 가 최상위 비트를 쓰며, 그래서 표준을 따르는 컴파일러에서
 *          부호 없는 표기가 필요하다.
 * @{
 */
#define M1_FLY          0x00000001L /* can fly or float */
#define M1_SWIM         0x00000002L /* can traverse water */
#define M1_AMORPHOUS    0x00000004L /* can flow under doors */
#define M1_WALLWALK     0x00000008L /* can phase through rock */
#define M1_CLING        0x00000010L /* can cling to ceiling */
#define M1_TUNNEL       0x00000020L /* can tunnel through rock */
#define M1_NEEDPICK     0x00000040L /* needs pick to tunnel */
#define M1_CONCEAL      0x00000080L /* hides under objects */
#define M1_HIDE         0x00000100L /* mimics, blends in with ceiling */
#define M1_AMPHIBIOUS   0x00000200L /* can survive underwater */
#define M1_BREATHLESS   0x00000400L /* doesn't need to breathe */
#define M1_NOTAKE       0x00000800L /* cannot pick up objects */
#define M1_NOEYES       0x00001000L /* no eyes to gaze into or blind */
#define M1_NOHANDS      0x00002000L /* no hands to handle things */
#define M1_NOLIMBS      0x00006000L /* no arms/legs to kick/wear on */
#define M1_NOHEAD       0x00008000L /* no head to behead */
#define M1_MINDLESS     0x00010000L /* has no mind--golem, zombie, mold */
#define M1_HUMANOID     0x00020000L /* has humanoid head/arms/torso */
#define M1_ANIMAL       0x00040000L /* has animal body */
#define M1_SLITHY       0x00080000L /* has serpent body */
#define M1_UNSOLID      0x00100000L /* has no solid or liquid body */
#define M1_THICK_HIDE   0x00200000L /* has thick hide or scales */
#define M1_OVIPAROUS    0x00400000L /* can lay eggs */
#define M1_REGEN        0x00800000L /* regenerates hit points */
#define M1_SEE_INVIS    0x01000000L /* can see invisible creatures */
#define M1_TPORT        0x02000000L /* can teleport */
#define M1_TPORT_CNTRL  0x04000000L /* controls where it teleports to */
#define M1_ACID         0x08000000L /* acidic to eat */
#define M1_POIS         0x10000000L /* poisonous to eat */
#define M1_CARNIVORE    0x20000000L /* eats corpses */
#define M1_HERBIVORE    0x40000000L /* eats fruits */
#define M1_OMNIVORE     0x60000000L /* eats both */
#ifdef NHSTDC
#define M1_METALLIVORE  0x80000000UL /* eats metal */
#else
#define M1_METALLIVORE  0x80000000L /* eats metal */
#endif
/** @} */

/**
 * @name What a monster is, and how it behaves toward the hero
 * @brief The second flag word: kind, rank, sex, disposition, and appetites.
 *
 * Where the first word says what a creature can do, this one says what it is -- its race
 * and rank among its own kind -- and how it starts out disposed toward the hero.
 *
 * @note The race bits are reused as monster races (@c MH_HUMAN and its siblings) rather
 *       than duplicated, which is why they must stay within the role race mask. The
 *       existing comment expects them to become their own field one day.
 * @note @c M2_HOSTILE and @c M2_PEACEFUL fix the starting disposition; a species with
 *       neither is decided by alignment when it is created.
 * @warning This word is also full, at @c M2_MAGIC.
 * @{
 */
/**
 * @name 몬스터가 무엇인지, 그리고 영웅에게 어떻게 대하는지
 * @brief 두 번째 플래그 워드. 부류, 지위, 성별, 성향, 그리고 탐하는 것.
 *
 * 첫 워드가 생물이 무엇을 할 수 있는지를 말한다면, 이 워드는 그것이 무엇인지 -- 그 종족과 자기 부류 안에서의
 * 지위 -- 와 영웅에게 어떤 성향으로 시작하는지를 말한다.
 *
 * @note 종족 비트는 복제되지 않고 몬스터 종족(@c MH_HUMAN 과 그 형제들)으로 재사용된다. 그래서 직업 종족
 *       마스크 안에 머물러야 한다. 기존 주석은 언젠가 그것들이 자기 필드가 되기를 기대하고 있다.
 * @note @c M2_HOSTILE 과 @c M2_PEACEFUL 은 시작 성향을 고정한다. 둘 다 없는 종족은 생성될 때 진영에 따라
 *       정해진다.
 * @warning 이 워드도 @c M2_MAGIC 에서 가득 찼다.
 * @{
 */
#define M2_NOPOLY       0x00000001L /* players mayn't poly into one */
#define M2_UNDEAD       0x00000002L /* is walking dead */
#define M2_WERE         0x00000004L /* is a lycanthrope */
#define M2_HUMAN        0x00000008L /* is a human */
#define M2_ELF          0x00000010L /* is an elf */
#define M2_DWARF        0x00000020L /* is a dwarf */
#define M2_GNOME        0x00000040L /* is a gnome */
#define M2_ORC          0x00000080L /* is an orc */
#define M2_DEMON        0x00000100L /* is a demon */
#define M2_MERC         0x00000200L /* is a guard or soldier */
#define M2_LORD         0x00000400L /* is a lord to its kind */
#define M2_PRINCE       0x00000800L /* is an overlord to its kind */
#define M2_MINION       0x00001000L /* is a minion of a deity */
#define M2_GIANT        0x00002000L /* is a giant */
#define M2_SHAPESHIFTER 0x00004000L /* is a shapeshifting species */
#define M2_MALE         0x00010000L /* always male */
#define M2_FEMALE       0x00020000L /* always female */
#define M2_NEUTER       0x00040000L /* neither male nor female */
#define M2_PNAME        0x00080000L /* monster name is a proper name */
#define M2_HOSTILE      0x00100000L /* always starts hostile */
#define M2_PEACEFUL     0x00200000L /* always starts peaceful */
#define M2_DOMESTIC     0x00400000L /* can be tamed by feeding */
#define M2_WANDER       0x00800000L /* wanders randomly */
#define M2_STALK        0x01000000L /* follows you to other levels */
#define M2_NASTY        0x02000000L /* extra-nasty monster (more xp) */
#define M2_STRONG       0x04000000L /* strong (or big) monster */
#define M2_ROCKTHROW    0x08000000L /* throws boulders */
#define M2_GREEDY       0x10000000L /* likes gold */
#define M2_JEWELS       0x20000000L /* likes gems */
#define M2_COLLECT      0x40000000L /* picks up weapons and food */
#ifdef NHSTDC
#define M2_MAGIC        0x80000000UL /* picks up magic items */
#else
#define M2_MAGIC        0x80000000L /* picks up magic items */
#endif
/** @} */

/**
 * @name What a monster covets, and how patiently it waits
 * @brief The third flag word: greed for particular artifacts, and two odd behaviours.
 *
 * A few monsters do not merely fight -- they want a specific thing the hero may be
 * carrying, and will pursue it across the dungeon. Each such desire is a bit, and
 * @c M3_COVETOUS is all of them together so the rules can ask "does it want anything"
 * without listing them.
 *
 * @note @c M3_WANTSALL and @c M3_COVETOUS are the same value. Two names exist because the
 *       question differs: one asks which artifacts, the other whether the monster is of
 *       the covetous sort at all.
 * @warning @c M3_WAITFORU and @c M3_CLOSE are not covetousness, and the gap in the bit
 *          numbering separates them from it so @c M3_COVETOUS does not catch them.
 * @{
 */
/**
 * @name 몬스터가 무엇을 탐하는지, 그리고 얼마나 참고 기다리는지
 * @brief 세 번째 플래그 워드. 특정 아티팩트에 대한 탐욕과 두 가지 독특한 행동.
 *
 * 몇몇 몬스터는 단지 싸우는 것이 아니다. 영웅이 지니고 있을지 모르는 특정한 것을 원하며, 던전을 건너 그것을
 * 쫓는다. 그런 욕망 하나하나가 비트이고, @c M3_COVETOUS 는 그 전부를 합친 것이다. 그래서 규칙이 그것들을
 * 열거하지 않고 "무언가를 원하는가"를 물을 수 있다.
 *
 * @note @c M3_WANTSALL 과 @c M3_COVETOUS 는 같은 값이다. 질문이 다르기 때문에 두 이름이 있다. 하나는 어떤
 *       아티팩트인지를 묻고, 다른 하나는 애초에 그 몬스터가 탐하는 부류인지를 묻는다.
 * @warning @c M3_WAITFORU 와 @c M3_CLOSE 는 탐욕이 아니며, 비트 번호에 난 틈이 그것들을 탐욕에서 떼어 놓는다.
 *          그래서 @c M3_COVETOUS 가 그것들을 잡지 않는다.
 * @{
 */
#define M3_WANTSAMUL    0x0001 /* would like to steal the amulet */
#define M3_WANTSBELL    0x0002 /* wants the bell */
#define M3_WANTSBOOK    0x0004 /* wants the book */
#define M3_WANTSCAND    0x0008 /* wants the candelabrum */
#define M3_WANTSARTI    0x0010 /* wants the quest artifact */
#define M3_WANTSALL     0x001f /* wants any major artifact */
#define M3_WAITFORU     0x0040 /* waits to see you or get attacked */
#define M3_CLOSE        0x0080 /* lets you close unless attacked */

#define M3_COVETOUS     0x001f /* wants something */
#define M3_WAITMASK     0x00c0 /* waiting... */

/* Infravision is currently implemented for players only */
#define M3_INFRAVISION  0x0100 /* has infravision */
#define M3_INFRAVISIBLE 0x0200 /* visible by infravision */

#define M3_DISPLACES    0x0400 /* moves monsters out of its way */
/** @} */

/**
 * @name Monster size
 * @brief How big the creature is, on a scale the rules can compare.
 *
 * Size decides what armour fits, what can be swallowed, how much room a body takes. The
 * comments give the heights the steps were chosen from.
 *
 * @note The values are consecutive except the last: @c MZ_GIGANTIC skips ahead, marking
 *       that it is off the scale rather than one step beyond huge. Any code that steps
 *       through sizes one at a time will miss it.
 * @note @c MZ_HUMAN is another name for the medium step, used where "human-sized" reads
 *       better than a number.
 * @{
 */
/**
 * @name 몬스터 크기
 * @brief 규칙이 비교할 수 있는 척도 위에서 생물이 얼마나 큰지.
 *
 * 크기는 어떤 갑옷이 맞는지, 무엇이 삼켜질 수 있는지, 몸이 얼마나 자리를 차지하는지를 정한다. 주석에 각 단계를
 * 정할 때 쓴 키가 적혀 있다.
 *
 * @note 마지막을 빼면 값들이 연속한다. @c MZ_GIGANTIC 은 건너뛰어 있으며, 거대한 것보다 한 단계 위가 아니라
 *       척도를 벗어났음을 나타낸다. 크기를 하나씩 밟아 나가는 코드는 그것을 놓친다.
 * @note @c MZ_HUMAN 은 중간 단계의 다른 이름이다. 숫자보다 "사람 크기"라고 읽는 것이 나은 곳에서 쓴다.
 * @{
 */
#define MZ_TINY         0 /* < 2' */
#define MZ_SMALL        1 /* 2-4' */
#define MZ_MEDIUM       2 /* 4-7' */
#define MZ_HUMAN        MZ_MEDIUM /* human-sized */
#define MZ_LARGE        3 /* 7-12' */
#define MZ_HUGE         4 /* 12-25' */
#define MZ_GIGANTIC     7 /* off the scale */
/** @} */

/**
 * @name Monster races
 * @brief The races a monster may belong to, named for use where a race is meant.
 * @note Aliases of the corresponding flag bits rather than separate values, so a
 *       monster's race is not stored apart from its flags.
 * @warning Must stay within the role race mask, since the same values describe the hero's
 *          race. The existing comment expects this to become its own field eventually.
 * @{
 */
/**
 * @name 몬스터 종족
 * @brief 몬스터가 속할 수 있는 종족들. 종족을 뜻하는 곳에서 쓰도록 이름 붙인 것.
 * @note 별개의 값이 아니라 해당 플래그 비트의 별칭이다. 그래서 몬스터의 종족이 그 플래그와 따로 저장되지
 *       않는다.
 * @warning 같은 값들이 영웅의 종족도 기술하므로 직업 종족 마스크 안에 머물러야 한다. 기존 주석은 이것이
 *          결국 자기 필드가 되기를 기대하고 있다.
 * @{
 */
/* Monster races -- must stay within ROLE_RACEMASK */
/* Eventually this may become its own field */
#define MH_HUMAN        M2_HUMAN
#define MH_ELF          M2_ELF
#define MH_DWARF        M2_DWARF
#define MH_GNOME        M2_GNOME
#define MH_ORC          M2_ORC
/** @} */

/**
 * @name How the game may create a monster
 * @brief Where and how often a species appears, and whether it leaves a body.
 *
 * These say what the level generator is allowed to do. A species may be unique, or
 * confined to the lower dungeon, or excluded from it, or created only by name. The lowest
 * bits are a frequency rather than flags -- how often it turns up when nothing has asked
 * for it in particular.
 *
 * @note Constant for the whole game; what changes during play is recorded separately.
 * @warning @c G_FREQ is a mask over a small number, not a single bit. Testing it as a flag
 *          asks only whether the frequency is non-zero.
 * @warning @c G_IGNORE does not belong to this set at all -- it controls the per-game
 *          record and is merely passed through the same argument, as the existing comment
 *          admits.
 * @{
 */
/**
 * @name 게임이 몬스터를 만들 수 있는 방식
 * @brief 어떤 종족이 어디에 얼마나 자주 나타나는지, 그리고 시체를 남기는지.
 *
 * 이들은 레벨 생성기가 무엇을 해도 되는지를 말한다. 어떤 종족은 유일하고, 어떤 것은 던전 아래쪽에 갇혀 있고,
 * 어떤 것은 거기서 제외되고, 어떤 것은 이름으로 지정될 때만 만들어진다. 아래쪽 비트들은 플래그가 아니라
 * 빈도다. 아무도 그것을 특별히 요청하지 않았을 때 얼마나 자주 나타나는지.
 *
 * @note 게임 전체에 걸쳐 고정된다. 플레이 중에 변하는 것은 따로 기록된다.
 * @warning @c G_FREQ 는 단일 비트가 아니라 작은 수 위의 마스크다. 그것을 플래그로 검사하면 빈도가 0이 아닌지만
 *          묻는 것이 된다.
 * @warning @c G_IGNORE 는 이 묶음에 전혀 속하지 않는다. 게임별 기록을 제어하며, 기존 주석이 인정하듯 같은
 *          인자를 통해 전달되기만 한다.
 * @{
 */
/* for mons[].geno (constant during game) */
#define G_UNIQ          0x1000 /* generated only once */
#define G_NOHELL        0x0800 /* not generated in "hell" */
#define G_HELL          0x0400 /* generated only in "hell" */
#define G_NOGEN         0x0200 /* generated only specially */
#define G_SGROUP        0x0080 /* appear in small groups normally */
#define G_LGROUP        0x0040 /* appear in large groups normally */
#define G_GENO          0x0020 /* can be genocided */
#define G_NOCORPSE      0x0010 /* no corpse left ever */
#define G_FREQ          0x0007 /* creation frequency mask */
/* note: G_IGNORE controls handling of mvitals[].mvflags bits but is
   passed to mkclass() as if it dealt with mons[].geno bits */
#define G_IGNORE        0x8000 /* for mkclass(), ignore G_GENOD|G_EXTINCT */
/** @} */

/**
 * @name What has happened to a species this game
 * @brief The running record: met, wiped out, or used up.
 *
 * Unlike the flags above, these change as the game goes on. They are how genocide is
 * permanent and how a species can be exhausted, and they are what makes a monster's
 * absence from later levels meaningful rather than chance.
 *
 * @note Genocide and extinction differ in cause but not in effect, so @c G_GONE covers
 *       both for code that only needs to know nothing more will appear.
 * @warning These share a field with @c G_NOCORPSE from the set above, so the numbering of
 *          the two sets must not collide.
 * @{
 */
/**
 * @name 이번 게임에서 어떤 종족에게 무슨 일이 있었는지
 * @brief 진행 중의 기록. 만났는지, 절멸당했는지, 다 소진되었는지.
 *
 * 위의 플래그들과 달리 이들은 게임이 진행되면서 변한다. 절멸이 영구적인 이유이고, 어떤 종족이 소진될 수 있는
 * 이유다. 그리고 이후 레벨에서 어떤 몬스터가 없는 것이 우연이 아니라 의미를 갖게 만드는 것이다.
 *
 * @note 절멸과 멸종은 원인은 다르나 결과는 같다. 그래서 더 나타나지 않는다는 것만 알아야 하는 코드를 위해
 *       @c G_GONE 이 둘을 함께 덮는다.
 * @warning 이들은 위 묶음의 @c G_NOCORPSE 와 필드를 공유한다. 그래서 두 묶음의 번호가 충돌해서는 안 된다.
 * @{
 */
/* for svm.mvitals[].mvflags (variant during game), along with G_NOCORPSE */
#define G_KNOWN         0x04 /* have been encountered */
#define G_GENOD         0x02 /* have been genocided */
#define G_EXTINCT       0x01 /* population control; create no more */
#define G_GONE          (G_GENOD | G_EXTINCT)
#define MV_KNOWS_EGG    0x08 /* player recognizes egg of this monster type */
/** @} */

/**
 * @brief A monster's sex, where the game needs one.
 * @note Neutral is a third value and not a missing one: some monsters genuinely have no
 *       sex, and the species table can say so.
 * @warning Used to index the species' name array, so the order is fixed by that use.
 */
/**
 * @brief 게임이 필요로 하는 곳에서의 몬스터의 성별.
 * @note 중성은 빠진 값이 아니라 세 번째 값이다. 어떤 몬스터는 실제로 성별이 없고, 종족 표가 그것을 말할 수 있다.
 * @warning 종족의 이름 배열을 색인하는 데 쓰이므로, 순서가 그 용도에 의해 고정되어 있다.
 */
enum mgender { MALE, FEMALE, NEUTRAL,
               NUM_MGENDERS };

/* *INDENT-ON* */
/* clang-format on */
#endif /* MONFLAG_H */
