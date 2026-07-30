/* NetHack 5.0	youprop.h	$NHDT-Date: 1781973093 2026/06/20 16:31:33 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.50 $ */
/* Copyright (c) 1989 Mike Threepoint                             */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOUPROP_H
#define YOUPROP_H

#include "prop.h"
#include "permonst.h"
#include "mondata.h"

/**
 * @file youprop.h
 * @brief Asking what the hero can currently do.
 *
 * A property such as fire resistance is not a flag. The hero may have it because they ate the right
 * corpse, because they are wearing a ring, because they are polymorphed into something that has it, or
 * they may be prevented from having it by something else -- and all of those can be true at once. So
 * every property is stored as several independent sources and answered as a question rather than read as
 * a value.
 *
 * That is what the naming convention here encodes, and knowing it makes the file readable at a glance:
 *  - a name beginning @c H is the intrinsic source -- earned, or granted by the current form;
 *  - a name beginning @c E is the extrinsic source -- worn or wielded equipment;
 *  - a name beginning @c B is what is blocking the property;
 *  - a bare name is the answer, and is what the rules should almost always use.
 *
 * The bare form is usually just "either source, and nothing blocking", but not always -- and the
 * exceptions are the interesting part of this file. Some consult the hero's steed, some consult the
 * level, some consult the form's species, and some are qualified by a second property. Those are
 * documented individually below.
 *
 * @note The intrinsic word is not a boolean but a set of bits recording @e why the hero has the property:
 *       a timeout, a permanent grant, the current form. Code that only wants to know whether the property
 *       holds should use the bare name; code that needs to know how long it will last must look at the
 *       bits.
 * @warning A blocked property is not an absent one. The sources are still set, so testing @c H or @c E
 *          directly will report a property the hero does not actually have. This is the most common
 *          mistake this file invites.
 */

/**
 * @file youprop.h
 * @brief 영웅이 지금 무엇을 할 수 있는지 묻기.
 *
 * 화염 저항 같은 속성은 플래그가 아니다. 영웅은 알맞은 시체를 먹어서, 반지를 껴서, 그것을 가진 무엇으로 변신해서 그것을 지닐 수 있고, 아니면 다른 무엇에 의해
 * 지니지 못하게 될 수도 있다. 그리고 그 전부가 동시에 참일 수 있다. 그래서 모든 속성은 여러 독립적인 출처로 저장되고, 값으로 읽히는 것이 아니라 질문으로
 * 답해진다.
 *
 * 여기의 이름 규약이 그것을 담고 있으며, 그것을 알면 이 파일이 한눈에 읽힌다.
 *  - @c H 로 시작하는 이름은 내재적 출처다. 얻은 것이거나 현재 형태가 준 것.
 *  - @c E 로 시작하는 이름은 외재적 출처다. 착용하거나 든 장비.
 *  - @c B 로 시작하는 이름은 그 속성을 막고 있는 것.
 *  - 접두어 없는 이름이 답이며, 규칙이 거의 항상 써야 하는 것이다.
 *
 * 접두어 없는 형태는 보통 "출처 중 하나가 있고 막는 것이 없음"이지만, 언제나 그렇지는 않다. 그리고 그 예외들이 이 파일의 흥미로운 부분이다. 어떤 것은 영웅의
 * 탈것을 참조하고, 어떤 것은 레벨을 참조하고, 어떤 것은 그 형태의 종족을 참조하고, 어떤 것은 두 번째 속성에 의해 한정된다. 그것들은 아래에 개별적으로
 * 기록되어 있다.
 *
 * @note 내재적 워드는 논리값이 아니라 영웅이 @e 왜 그 속성을 지니는지를 기록하는 비트 묶음이다. 남은 시간, 영구한 부여, 현재 형태. 속성이 성립하는지만 알고
 *       싶은 코드는 접두어 없는 이름을 써야 하고, 그것이 얼마나 오래갈지 알아야 하는 코드는 비트를 봐야 한다.
 * @warning 막힌 속성은 없는 속성이 아니다. 출처는 여전히 켜져 있으므로, @c H 나 @c E 를 직접 검사하면 영웅이 실제로 갖지 않은 속성을 보고하게 된다. 이 파일이
 *          유발하는 가장 흔한 잘못이다.
 */

/* KMH, intrinsics patch.
 * Reorganized and rewritten for >32-bit properties.
 * HXxx refers to intrinsic bitfields while in human form.
 * EXxx refers to extrinsic bitfields from worn objects.
 * BXxx refers to the cause of the property being blocked.
 * Xxx refers to any source, including polymorph forms.
 * [Post-3.4.3: HXxx now includes a FROMFORM bit to handle
 * intrinsic conferred by being polymorphed.]
 */

/**
 * @def maybe_polyd
 * @brief Choose between two expressions according to whether the hero is polymorphed.
 * @param if_so used when polymorphed
 * @param if_not used otherwise
 * @note Exists because so many of the hero's numbers exist twice -- health, attributes, monster kind --
 *       and nearly every rule that reads one needs this choice. Writing it as a macro keeps the choice
 *       visible at the point of use rather than buried in a helper.
 * @warning Only one branch is evaluated, so an argument with a side effect will sometimes not happen.
 */
/**
 * @def maybe_polyd
 * @brief 영웅이 변신했는지에 따라 두 수식 중 하나를 고른다.
 * @param if_so 변신했을 때 쓰이는 것
 * @param if_not 그렇지 않을 때 쓰이는 것
 * @note 영웅의 여러 숫자가 두 번 존재하기 때문에 있다. 체력, 능력치, 몬스터 종류. 그리고 그 중 하나를 읽는 거의 모든 규칙이 이 선택을 필요로 한다. 매크로로
 *       쓰면 그 선택이 보조 함수 안에 묻히지 않고 쓰이는 자리에서 보인다.
 * @warning 한쪽 가지만 평가된다. 그래서 부작용이 있는 인자는 때때로 실행되지 않는다.
 */
#define maybe_polyd(if_so, if_not) (Upolyd ? (if_so) : (if_not))

/*** Resistances to troubles ***/
/* With intrinsics and extrinsics */
#define HFire_resistance u.uprops[FIRE_RES].intrinsic
#define EFire_resistance u.uprops[FIRE_RES].extrinsic
#define Fire_resistance (HFire_resistance || EFire_resistance)

#define HCold_resistance u.uprops[COLD_RES].intrinsic
#define ECold_resistance u.uprops[COLD_RES].extrinsic
#define Cold_resistance (HCold_resistance || ECold_resistance)

#define HSleep_resistance u.uprops[SLEEP_RES].intrinsic
#define ESleep_resistance u.uprops[SLEEP_RES].extrinsic
#define Sleep_resistance (HSleep_resistance || ESleep_resistance)

#define HDisint_resistance u.uprops[DISINT_RES].intrinsic
#define EDisint_resistance u.uprops[DISINT_RES].extrinsic
#define Disint_resistance (HDisint_resistance || EDisint_resistance)

#define HShock_resistance u.uprops[SHOCK_RES].intrinsic
#define EShock_resistance u.uprops[SHOCK_RES].extrinsic
#define Shock_resistance (HShock_resistance || EShock_resistance)

#define HPoison_resistance u.uprops[POISON_RES].intrinsic
#define EPoison_resistance u.uprops[POISON_RES].extrinsic
#define Poison_resistance (HPoison_resistance || EPoison_resistance)

#define HDrain_resistance u.uprops[DRAIN_RES].intrinsic
#define EDrain_resistance u.uprops[DRAIN_RES].extrinsic
#define Drain_resistance (HDrain_resistance || EDrain_resistance)

/* Hxxx due to FROMFORM only */
#define HAntimagic u.uprops[ANTIMAGIC].intrinsic
#define EAntimagic u.uprops[ANTIMAGIC].extrinsic
#define Antimagic (HAntimagic || EAntimagic)

#define HAcid_resistance u.uprops[ACID_RES].intrinsic
#define EAcid_resistance u.uprops[ACID_RES].extrinsic
#define Acid_resistance (HAcid_resistance || EAcid_resistance)

#define HStone_resistance u.uprops[STONE_RES].intrinsic
#define EStone_resistance u.uprops[STONE_RES].extrinsic
#define Stone_resistance (HStone_resistance || EStone_resistance)

#define HSick_resistance u.uprops[SICK_RES].intrinsic
#define ESick_resistance u.uprops[SICK_RES].extrinsic
/**
 * @def Sick_resistance
 * @brief Whether the hero resists disease.
 * @note Unlike its siblings, this also asks whether anything worn defends against disease specifically --
 *       so a source outside the property system contributes, and the two sources alone are not the whole
 *       answer.
 */
/**
 * @def Sick_resistance
 * @brief 영웅이 질병에 저항하는지.
 * @note 형제들과 달리, 착용한 것 중 질병을 특별히 막아 주는 것이 있는지도 함께 묻는다. 그래서 속성 체계 밖의 출처가 기여하며, 두 출처만으로는 답의 전부가
 *       아니다.
 */
#define Sick_resistance (HSick_resistance || ESick_resistance \
                         || defended(&gy.youmonst, AD_DISE))

/**
 * @def Invulnerable
 * @brief Whether the hero cannot presently be harmed.
 * @note Intrinsic only, because the only source is praying -- there is no equipment that grants it, so
 *       there is no extrinsic form to test.
 */
/**
 * @def Invulnerable
 * @brief 영웅이 지금 해를 입을 수 없는지.
 * @note 내재적인 것만 있다. 출처가 기도뿐이기 때문이다. 그것을 주는 장비가 없으므로 검사할 외재적 형태도 없다.
 */
/* Intrinsics only */
#define Invulnerable u.uprops[INVULNERABLE].intrinsic /* [Tom] */

/*** Troubles ***/
/**
 * @def Punished
 * @brief Whether the hero is chained to a heavy iron ball.
 * @note Not a property at all but a question about an object's existence, given a property's name so the
 *       rules can ask it the same way as the rest.
 */
/**
 * @def Punished
 * @brief 영웅이 무거운 철구에 묶여 있는지.
 * @note 속성이 전혀 아니라 어떤 물건의 존재에 관한 질문이며, 규칙이 나머지와 같은 방식으로 물을 수 있도록 속성의 이름을 받았다.
 */
/* Pseudo-property */
#define Punished (uball != 0)

/* Many are implemented solely as timeouts (we use just intrinsic) */
#define HStun u.uprops[STUNNED].intrinsic /* timed or FROMFORM */
#define Stunned HStun

#define HConfusion u.uprops[CONFUSION].intrinsic
#define Confusion HConfusion

/**
 * @name Blindness
 * @brief The most intricate property, because there are several distinguishable ways not to see.
 *
 * Blindness is not one condition. The hero may be blinded by an injury that will wear off, blind by the
 * player's own choice at character creation, blind because their current form has no eyes, or wearing a
 * blindfold on purpose. Those differ in what cures them, in what the hero may still do, and in what the
 * game should say -- and one of them is voluntary and can simply be removed.
 *
 * So there are several questions here rather than one, and picking the wrong one is a real bug. The bare
 * @c Blind is what almost every rule wants: can the hero see. @c Blindfolded_only is for the cases where
 * it matters that the hero could see if they chose to.
 *
 * @note Artifact lenses block blindness rather than curing it, which is why a blocking source exists for
 *       this property. The sources stay set while blocked.
 * @note The existing comment explains why @c Blind applies the blocking test to both sources even though
 *       the blocking source and a worn blindfold cannot presently coexist: it is deliberate robustness
 *       against that ceasing to be true.
 * @warning @c Blinded is not the same as @c Blind. The first ignores a blindfold, and so answers false for
 *          a hero who cannot see a thing.
 * @{
 */
/**
 * @name 실명
 * @brief 가장 복잡한 속성이다. 보지 못하는 방식이 서로 구별될 만큼 여러 가지이기 때문이다.
 *
 * 실명은 하나의 상태가 아니다. 영웅은 시간이 지나면 나을 부상으로 눈이 멀 수도, 캐릭터 생성 때 플레이어 자신의 선택으로 눈이 먼 상태일 수도, 현재 형태에 눈이
 * 없어서 못 볼 수도, 일부러 눈가리개를 쓰고 있을 수도 있다. 그것들은 무엇으로 치유되는지, 영웅이 여전히 무엇을 할 수 있는지, 게임이 무엇이라 말해야 하는지가
 * 서로 다르다. 그리고 그 중 하나는 자발적이며 그냥 벗을 수 있다.
 *
 * 그래서 여기에는 하나가 아니라 여러 질문이 있고, 잘못된 것을 고르는 것이 실제 버그가 된다. 접두어 없는 @c Blind 가 거의 모든 규칙이 원하는 것이다. 영웅이
 * 볼 수 있는가. @c Blindfolded_only 는 영웅이 원하면 볼 수 있다는 점이 중요한 경우를 위한 것이다.
 *
 * @note 아티팩트 렌즈는 실명을 치유하는 것이 아니라 막는다. 이 속성에 막는 출처가 존재하는 이유다. 막혀 있는 동안에도 출처는 켜진 채로 있다.
 * @note 기존 주석은 막는 출처와 착용한 눈가리개가 현재 공존할 수 없음에도 @c Blind 가 두 출처 모두에 막힘 검사를 적용하는 이유를 설명한다. 그것이 더는 참이
 *       아니게 될 경우를 대비한 의도적인 견고함이다.
 * @warning @c Blinded 는 @c Blind 와 같지 않다. 앞의 것은 눈가리개를 무시하므로, 아무것도 볼 수 없는 영웅에 대해 거짓으로 답한다.
 * @{
 */
/* Blindness is more complex than other properties */
#define HBlinded u.uprops[BLINDED].intrinsic /* TIMEOUT|FROMOUTSIDE|FROMFORM */
#define EBlinded u.uprops[BLINDED].extrinsic /* W_TOOL */
        /* wearing the Eyes of the Overworld overrides blindness */
#define BBlinded u.uprops[BLINDED].blocked   /* W_TOOL */
        /* non-blindfold: timed effect | u.uroleplay.blind | !haseyes() */
#define Blinded (HBlinded && !BBlinded)
#define BlindedTimeout (HBlinded & TIMEOUT)
#define PermaBlind ((HBlinded & FROMOUTSIDE) != 0L) /* OPTIONS:blind */
        /* worn blindfold (or towel; lenses don't set [BLINDED].extrinsic) */
#define Blindfolded EBlinded
#define Blindfolded_only (Blindfolded && !Blinded)
        /* '#define Blind (Blinded || Blindfolded)' would work, but only
           because BBlinded (conferred by artifact lenses) and Blindfolded
           are mutually exclusive; explicitly applying !BBlinded to both
           internal and external blindness should be more robust in case
           of future changes */
#define Blind ((HBlinded || EBlinded) && !BBlinded)
/** @} */

/*
 * Maladies
 */
#define Sick u.uprops[SICK].intrinsic
#define Stoned u.uprops[STONED].intrinsic
#define Strangled u.uprops[STRANGLED].intrinsic
#define Vomiting u.uprops[VOMITING].intrinsic
#define Glib u.uprops[GLIB].intrinsic
#define Slimed u.uprops[SLIMED].intrinsic /* [Tom] */

/* Hallucination is solely a timeout */
#define HHallucination u.uprops[HALLUC].intrinsic
#define HHalluc_resistance u.uprops[HALLUC_RES].intrinsic
#define EHalluc_resistance u.uprops[HALLUC_RES].extrinsic
#define Halluc_resistance (HHalluc_resistance || EHalluc_resistance)
/**
 * @def Hallucination
 * @brief Whether the hero is currently seeing things that are not there.
 * @note Qualified by a second property rather than by a blocking source: resistance to hallucination is a
 *       property in its own right, so the two are combined here instead of one setting the other's blocked
 *       field.
 */
/**
 * @def Hallucination
 * @brief 영웅이 지금 없는 것을 보고 있는지.
 * @note 막는 출처가 아니라 두 번째 속성에 의해 한정된다. 환각 저항이 그 자체로 하나의 속성이므로, 한쪽이 다른 쪽의 막힘 필드를 켜는 대신 여기서 둘이 결합된다.
 */
#define Hallucination (HHallucination && !Halluc_resistance)

/* Timeout, plus a worn mask */
#define HDeaf u.uprops[DEAF].intrinsic
#define EDeaf u.uprops[DEAF].extrinsic
/**
 * @def Deaf
 * @brief Whether the hero can hear.
 * @note Also consults the permanent handicap chosen at character creation, which is not a property and
 *       cannot be cured -- so this is one of the bare forms that is more than its two sources.
 */
/**
 * @def Deaf
 * @brief 영웅이 들을 수 있는지.
 * @note 캐릭터 생성 때 고른 영구한 불리함도 함께 참조한다. 그것은 속성이 아니고 치유될 수 없다. 그래서 이것은 두 출처 이상인 접두어 없는 형태 중 하나다.
 */
#define Deaf (HDeaf || EDeaf || u.uroleplay.deaf)

#define HFumbling u.uprops[FUMBLING].intrinsic
#define EFumbling u.uprops[FUMBLING].extrinsic
#define Fumbling (HFumbling || EFumbling)

/**
 * @name Wounded legs
 * @brief An injury, held in the property system but not obeying its conventions.
 *
 * The intrinsic word holds the time until recovery, as usual. The extrinsic word does not hold an
 * equipment source at all -- it borrows the worn-ring bits to record which leg is hurt, left or right or
 * both. So the two words here are not two sources of one fact but two different facts.
 *
 * @warning The extrinsic word is meaningless while the intrinsic one is zero, with one exception the
 *          existing comment records: the moment the timeout reaches zero and healing is performed, it is
 *          still needed to say which leg to heal.
 * @warning While riding, both apply to the steed rather than to the hero. A rule reading these without
 *          checking for a steed will attribute the mount's injury to the hero.
 * @{
 */
/**
 * @name 다친 다리
 * @brief 속성 체계 안에 들어 있으나 그 규약을 따르지 않는 부상.
 *
 * 내재적 워드는 평소처럼 회복까지의 시간을 담는다. 외재적 워드는 장비 출처를 전혀 담지 않는다. 반지 착용 비트를 빌려 어느 다리가 다쳤는지를, 왼쪽인지 오른쪽인지
 * 양쪽인지를 기록한다. 그래서 여기의 두 워드는 하나의 사실에 대한 두 출처가 아니라 서로 다른 두 사실이다.
 *
 * @warning 내재적 워드가 0인 동안 외재적 워드는 의미가 없다. 기존 주석이 기록하는 예외가 하나 있다. 남은 시간이 0에 이르러 치유가 수행되는 그 순간에는 어느 다리를
 *          치유할지 말하기 위해 여전히 필요하다.
 * @warning 탈것을 타고 있는 동안에는 둘 다 영웅이 아니라 탈것에 적용된다. 탈것을 확인하지 않고 이것을 읽는 규칙은 탈것의 부상을 영웅의 것으로 돌리게 된다.
 * @{
 */
/* HWounded_legs indicates whether wounded leg(s) condition exists and
   holds the timeout for recovery; EWounded_legs uses the worn-ring bits
   to track left vs right vs both and is meaningless when HWounded_legs
   is zero except when timeout has just decremented that to 0 and calls
   heal_legs(); both values apply to steed rather than to hero when riding */
#define HWounded_legs u.uprops[WOUNDED_LEGS].intrinsic
#define EWounded_legs u.uprops[WOUNDED_LEGS].extrinsic
#define Wounded_legs (HWounded_legs || EWounded_legs)
/** @} */

/**
 * @name Sleepiness
 * @brief Liable to fall asleep from time to time.
 * @warning Not "asleep now". This is the tendency, and a hero with it is awake most of the time -- so it
 *          must not be used to decide whether the hero can act.
 * @{
 */
/**
 * @name 졸림
 * @brief 때때로 잠들기 쉬운 상태.
 * @warning "지금 잠들어 있음"이 아니다. 이것은 경향이며, 이것을 지닌 영웅도 대부분의 시간에는 깨어 있다. 그래서 영웅이 행동할 수 있는지를 정하는 데 써서는
 *          안 된다.
 * @{
 */
/* Sleepy: prone to falling asleep periodically; not necessarily asleep now */
#define HSleepy u.uprops[SLEEPY].intrinsic
#define ESleepy u.uprops[SLEEPY].extrinsic
#define Sleepy (HSleepy || ESleepy)
/** @} */

#define HHunger u.uprops[HUNGER].intrinsic
#define EHunger u.uprops[HUNGER].extrinsic
#define Hunger (HHunger || EHunger)

/*** Vision and senses ***/
#define HSee_invisible u.uprops[SEE_INVIS].intrinsic
#define ESee_invisible u.uprops[SEE_INVIS].extrinsic
#define See_invisible (HSee_invisible || ESee_invisible)

#define HTelepat u.uprops[TELEPAT].intrinsic
#define ETelepat u.uprops[TELEPAT].extrinsic
/**
 * @def Blind_telepat
 * @brief Telepathy that works only while the hero cannot see.
 * @note The default kind. Earned telepathy is of no use with eyes open, which is why the name says so
 *       rather than leaving the caller to remember it.
 */
/**
 * @def Blind_telepat
 * @brief 영웅이 볼 수 없는 동안에만 작동하는 텔레파시.
 * @note 기본 종류다. 얻은 텔레파시는 눈을 뜬 채로는 쓸모가 없다. 그래서 호출자가 그것을 기억하도록 남기지 않고 이름이 그렇게 말한다.
 */
#define Blind_telepat (HTelepat || ETelepat)
/**
 * @def Unblind_telepat
 * @brief Telepathy that works with the eyes open.
 * @note Extrinsic only, deliberately: worn equipment grants the stronger form, and an earned intrinsic does
 *       not. That is the whole distinction between the two names.
 */
/**
 * @def Unblind_telepat
 * @brief 눈을 뜬 채로도 작동하는 텔레파시.
 * @note 의도적으로 외재적인 것만이다. 착용한 장비가 더 강한 형태를 주고, 얻은 내재적 능력은 그렇지 않다. 두 이름의 차이는 그것 전부다.
 */
#define Unblind_telepat (ETelepat)

#define HBlnd_resist u.uprops[BLND_RES].intrinsic /* from form */
#define EBlnd_resist u.uprops[BLND_RES].extrinsic /* wielding Sunsword */
#define Blnd_resist (HBlnd_resist || EBlnd_resist)

#define HWarning u.uprops[WARNING].intrinsic
#define EWarning u.uprops[WARNING].extrinsic
#define Warning (HWarning || EWarning)

/* Warning for a specific type of monster */
#define HWarn_of_mon u.uprops[WARN_OF_MON].intrinsic
#define EWarn_of_mon u.uprops[WARN_OF_MON].extrinsic
#define Warn_of_mon (HWarn_of_mon || EWarn_of_mon)

#define HUndead_warning u.uprops[WARN_UNDEAD].intrinsic
#define Undead_warning (HUndead_warning)

#define HSearching u.uprops[SEARCHING].intrinsic
#define ESearching u.uprops[SEARCHING].extrinsic
#define Searching (HSearching || ESearching)

#define HClairvoyant u.uprops[CLAIRVOYANT].intrinsic
#define EClairvoyant u.uprops[CLAIRVOYANT].extrinsic
#define BClairvoyant u.uprops[CLAIRVOYANT].blocked
#define Clairvoyant ((HClairvoyant || EClairvoyant) && !BClairvoyant)

#define HInfravision u.uprops[INFRAVISION].intrinsic
#define EInfravision u.uprops[INFRAVISION].extrinsic
#define Infravision (HInfravision || EInfravision)

#define HDetect_monsters u.uprops[DETECT_MONSTERS].intrinsic
#define EDetect_monsters u.uprops[DETECT_MONSTERS].extrinsic
#define Detect_monsters (HDetect_monsters || EDetect_monsters)

/*** Appearance and behavior ***/
#define Adornment u.uprops[ADORNED].extrinsic

#define HInvis u.uprops[INVIS].intrinsic
#define EInvis u.uprops[INVIS].extrinsic
#define BInvis u.uprops[INVIS].blocked
/**
 * @def Invis
 * @brief Whether the hero is invisible.
 * @note What the world sees. Use this for whether monsters can find the hero.
 */
/**
 * @def Invis
 * @brief 영웅이 투명한지.
 * @note 세상이 보는 바다. 몬스터가 영웅을 찾을 수 있는지에는 이것을 쓴다.
 */
#define Invis ((HInvis || EInvis) && !BInvis)
/**
 * @def Invisible
 * @brief Whether the hero cannot see themselves.
 * @warning A different question from @c Invis, despite the similar name. An invisible hero who can see the
 *          invisible is @c Invis but not @c Invisible -- so this is about display and self-perception, not
 *          about concealment.
 */
/**
 * @def Invisible
 * @brief 영웅이 자신을 볼 수 없는지.
 * @warning 이름이 비슷하지만 @c Invis 와는 다른 질문이다. 투명한 것을 볼 수 있는 투명한 영웅은 @c Invis 이지만 @c Invisible 은 아니다. 그래서 이것은 감춤이
 *          아니라 표시와 자기 지각에 관한 것이다.
 */
#define Invisible (Invis && !See_invisible)
/* Note: invisibility also hides inventory and steed */

#define HDisplaced u.uprops[DISPLACED].intrinsic /* timed from corpse */
#define EDisplaced u.uprops[DISPLACED].extrinsic /* worn cloak */
#define Displaced (HDisplaced || EDisplaced)

#define HStealth u.uprops[STEALTH].intrinsic
#define EStealth u.uprops[STEALTH].extrinsic
/* BStealth has FROMOUTSIDE set if mounted on non-flying steed */
#define BStealth u.uprops[STEALTH].blocked
#define Stealth ((HStealth || EStealth) && !BStealth)

#define HAggravate_monster u.uprops[AGGRAVATE_MONSTER].intrinsic
#define EAggravate_monster u.uprops[AGGRAVATE_MONSTER].extrinsic
#define Aggravate_monster (HAggravate_monster || EAggravate_monster)

#define HConflict u.uprops[CONFLICT].intrinsic
#define EConflict u.uprops[CONFLICT].extrinsic
#define Conflict (HConflict || EConflict)

/*** Transportation ***/
#define HJumping u.uprops[JUMPING].intrinsic
#define EJumping u.uprops[JUMPING].extrinsic
#define Jumping (HJumping || EJumping)

#define HTeleportation u.uprops[TELEPORT].intrinsic
#define ETeleportation u.uprops[TELEPORT].extrinsic
#define Teleportation (HTeleportation || ETeleportation)

#define HTeleport_control u.uprops[TELEPORT_CONTROL].intrinsic
#define ETeleport_control u.uprops[TELEPORT_CONTROL].extrinsic
#define Teleport_control (HTeleport_control || ETeleport_control)

/**
 * @name Being off the ground
 * @brief Levitation and flight, which are different and interact.
 *
 * Levitating means having no control: the hero cannot touch the surface, cannot go underwater, and cannot
 * choose to come down. Flying means having control. So levitation overrides flight rather than adding to
 * it, and the blocked field of flight is set while levitating -- that is how the override is expressed.
 *
 * The blocked fields carry more than a yes: a particular bit means trapped in the floor and another means
 * enclosed in solid rock. The reason matters because it decides what the hero can do about it.
 *
 * @note @c Lev_at_will is the narrow case where levitation can be ended deliberately -- from a blessed
 *       potion or an artifact and from nothing else at the same time. Hence the exact tests rather than a
 *       simple flag: any additional source would make ending it impossible, so the absence of other
 *       sources is part of the condition.
 * @note Flight also consults the steed, since a hero on a flying mount is flying.
 * @{
 */
/**
 * @name 땅에서 떨어져 있기
 * @brief 부양과 비행. 서로 다르고 서로 영향을 준다.
 *
 * 부양은 통제가 없다는 뜻이다. 영웅은 표면에 닿을 수 없고, 물속에 들어갈 수 없고, 스스로 내려오기를 택할 수 없다. 비행은 통제가 있다는 뜻이다. 그래서 부양은
 * 비행에 더해지는 것이 아니라 비행을 무시하며, 부양하는 동안 비행의 막힘 필드가 켜진다. 그 무시가 표현되는 방식이 그것이다.
 *
 * 막힘 필드는 예 하나 이상을 담는다. 특정 비트는 바닥에 갇혔음을, 다른 비트는 단단한 암반에 둘러싸였음을 뜻한다. 그 이유가 영웅이 그것에 대해 무엇을 할 수
 * 있는지를 정하므로 중요하다.
 *
 * @note @c Lev_at_will 은 부양을 의도적으로 끝낼 수 있는 좁은 경우다. 축복받은 물약이나 아티팩트에서 왔고 동시에 다른 어떤 것에서도 오지 않았을 때. 단순한
 *       플래그가 아니라 정확한 검사인 이유가 그것이다. 다른 출처가 하나라도 있으면 끝낼 수 없게 되므로, 다른 출처의 부재가 조건의 일부다.
 * @note 비행은 탈것도 함께 참조한다. 나는 탈것에 탄 영웅은 나는 것이기 때문이다.
 * @{
 */
/* HLevitation has I_SPECIAL set if levitating due to blessed potion
   which allows player to use the '>' command to end levitation early */
#define HLevitation u.uprops[LEVITATION].intrinsic
#define ELevitation u.uprops[LEVITATION].extrinsic
/* BLevitation has I_SPECIAL set if trapped in the floor,
   FROMOUTSIDE set if inside solid rock (or in water on Plane of Water) */
#define BLevitation u.uprops[LEVITATION].blocked
#define Levitation ((HLevitation || ELevitation) && !BLevitation)
/* Can't touch surface, can't go under water; overrides all others */
#define Lev_at_will                                                    \
    (((HLevitation & I_SPECIAL) != 0L || (ELevitation & W_ARTI) != 0L) \
     && (HLevitation & ~(I_SPECIAL | TIMEOUT)) == 0L                   \
     && (ELevitation & ~W_ARTI) == 0L)

/* Flying is overridden by Levitation */
#define HFlying u.uprops[FLYING].intrinsic
#define EFlying u.uprops[FLYING].extrinsic
/* BFlying has I_SPECIAL set if levitating or trapped in the floor or both,
   FROMOUTSIDE set if inside solid rock (or in water on Plane of Water) */
#define BFlying u.uprops[FLYING].blocked
#define Flying                                                      \
    ((HFlying || EFlying || (u.usteed && is_flyer(u.usteed->data))) \
     && !BFlying)
/* May touch surface; does not override any others */
/** @} */

/**
 * @name Crossing water
 * @brief Three different relationships with water, in order of how dry they leave the hero.
 *
 * Walking on water keeps the hero dry and above the surface. Swimming gets them wet but keeps them from
 * going under. Being amphibious lets them go under and survive. The three are separate because they differ
 * in what happens to carried items and in whether the hero can descend at all.
 *
 * @note @c Wwalking is qualified by the level: on the Plane of Water there is no surface to walk on, so it
 *       is simply meaningless there and the test says so rather than leaving each caller to remember.
 * @note @c Amphibious and @c Breathless share their sources but ask different questions -- one is about
 *       surviving underwater, the other about not needing to breathe at all, and a creature may be the
 *       second without being the first by species.
 * @note Swimming and flying both consult the steed, since the mount is what is doing the moving.
 * @{
 */
/**
 * @name 물을 건너기
 * @brief 물과의 세 가지 서로 다른 관계. 영웅을 얼마나 마른 채로 두는지 순서로.
 *
 * 물 위를 걷는 것은 영웅을 마른 채로 표면 위에 둔다. 수영은 젖게 하지만 가라앉지 않게 한다. 양서성은 가라앉아도 살아남게 한다. 셋이 따로 있는 것은 지닌 물건에
 * 무슨 일이 일어나는지와 영웅이 아예 내려갈 수 있는지가 다르기 때문이다.
 *
 * @note @c Wwalking 은 레벨에 의해 한정된다. 물의 평면에는 걸을 표면이 없으므로 그곳에서는 그냥 의미가 없고, 호출자마다 그것을 기억하도록 남기지 않고 검사가
 *       그것을 말한다.
 * @note @c Amphibious 와 @c Breathless 는 출처를 공유하지만 다른 질문을 한다. 하나는 물속에서 살아남는 것에 관한 것이고 다른 하나는 아예 숨 쉴 필요가 없다는
 *       것에 관한 것이며, 종족상 앞의 것이 아니면서 뒤의 것일 수 있다.
 * @note 수영과 비행 모두 탈것을 참조한다. 움직이고 있는 것이 그 탈것이기 때문이다.
 * @{
 */
#define HWwalking u.uprops[WWALKING].intrinsic /* see lava_effects() */
#define EWwalking u.uprops[WWALKING].extrinsic
#define Wwalking ((HWwalking || EWwalking) && !Is_waterlevel(&u.uz))
/* Don't get wet, can't go under water; overrides others except levitation */
/* Wwalking is meaningless on water level */

#define HSwimming u.uprops[SWIMMING].intrinsic
#define ESwimming u.uprops[SWIMMING].extrinsic /* [Tom] */
#define Swimming \
    (HSwimming || ESwimming || (u.usteed && is_swimmer(u.usteed->data)))
/* Get wet, don't go under water unless if amphibious */

#define HMagical_breathing u.uprops[MAGICAL_BREATHING].intrinsic
#define EMagical_breathing u.uprops[MAGICAL_BREATHING].extrinsic
#define Amphibious \
    (HMagical_breathing || EMagical_breathing || amphibious(gy.youmonst.data))
/* Get wet, may go under surface */

#define Breathless \
    (HMagical_breathing || EMagical_breathing || breathless(gy.youmonst.data))

/** @} */

/**
 * @def Underwater
 * @brief Whether the hero is currently beneath the surface.
 * @note Not a property but a situation, named like one for consistency. The existing comment records that
 *       the underlying field is meant to grow into distinguishing wading and swimming from being fully
 *       submerged, which is why both names are in use.
 */
/**
 * @def Underwater
 * @brief 영웅이 지금 수면 아래에 있는지.
 * @note 속성이 아니라 상황이며, 일관성을 위해 속성처럼 이름 붙였다. 기존 주석은 바탕 필드가 걷기와 수영을 완전히 잠긴 것과 구별하는 쪽으로 자라날 예정임을
 *       기록하고 있다. 두 이름이 함께 쓰이는 이유다.
 */
#define Underwater (u.uinwater)
/* Note that Underwater and u.uinwater are both used in code.
   The latter form is for later implementation of other in-water
   states, like swimming, wading, etc. */

#define HPasses_walls u.uprops[PASSES_WALLS].intrinsic
#define EPasses_walls u.uprops[PASSES_WALLS].extrinsic
#define Passes_walls (HPasses_walls || EPasses_walls)

/*** Physical attributes ***/
#define HSlow_digestion u.uprops[SLOW_DIGESTION].intrinsic
#define ESlow_digestion u.uprops[SLOW_DIGESTION].extrinsic
#define Slow_digestion (HSlow_digestion || ESlow_digestion) /* KMH */

#define HHalf_spell_damage u.uprops[HALF_SPDAM].intrinsic
#define EHalf_spell_damage u.uprops[HALF_SPDAM].extrinsic
#define Half_spell_damage (HHalf_spell_damage || EHalf_spell_damage)

/*
 * Physical damage
 *
 * Damage is NOT physical damage if (in order of priority):
 * 1. it already qualifies for some other special category
 *    for which a special resistance already exists in the game
 *    including: cold, fire, shock, acid, and magic.
 *    Note that fire is extended to include all non-acid forms of
 *    burning, even boiling water since that is already dealt with
 *    by fire resistance, and in most or all cases is caused by fire.
 * 2. it doesn't leave a mark. Marks include destruction of, or
 *    damage to, an internal organ (including the brain),
 *    lacerations, bruises, crushed body parts, bleeding.
 *
 * The following were evaluated and determined _NOT_ to be
 * susceptible to Half_physical_damage protection:
 *   Being caught in a fireball                      [fire damage]
 *   Sitting in lava                                 [lava damage]
 *   Thrown potion (acid)                            [acid damage]
 *   Splattered burning oil from thrown potion       [fire damage]
 *   Mixing water and acid                           [acid damage]
 *   Molten lava (entering or being splashed)        [lava damage]
 *   boiling water from a sink                       [fire damage]
 *   Fire traps                                      [fire damage]
 *   Scrolls of fire (confused and otherwise)        [fire damage]
 *   Alchemical explosion                            [not physical]
 *   System shock                                    [shock damage]
 *   Bag of holding explosion                        [magical]
 *   Being undead-turned by your god                 [magical]
 *   Level-drain                                     [magical]
 *   Magical explosion of a magic trap               [magical]
 *   Sitting on a throne with a bad effect           [magical]
 *   Contaminated water from a sink                  [poison/sickness]
 *   Contact-poisoned spellbooks                     [poison/sickness]
 *   Eating acidic/poisonous/mildly-old corpses      [poison/sickness]
 *   Eating a poisoned weapon while polyselfed       [poison/sickness]
 *   Engulfing a zombie or mummy (AT_ENGL in hmonas) [poison/sickness]
 *   Quaffed potions of sickness, lit oil, acid      [poison/sickness]
 *   Pyrolisks' fiery gaze                           [fire damage]
 *   Any passive attack                              [most don't qualify]
 */

/**
 * @name Halved physical damage
 * @brief Protection against blows that leave a mark.
 * @note Only physical damage qualifies, and the long comment above is the definition rather than a
 *       description: damage is not physical if some other resistance already covers it, or if it leaves no
 *       mark. The list of rejected cases is there so that a new source of damage can be classified the same
 *       way rather than by intuition.
 * @{
 */
/**
 * @name 절반이 되는 물리 피해
 * @brief 흔적을 남기는 타격에 대한 보호.
 * @note 물리 피해만 해당하며, 위의 긴 주석은 기술이 아니라 정의다. 다른 저항이 이미 다루는 피해거나 흔적을 남기지 않는 피해는 물리 피해가 아니다. 제외된 경우의
 *       목록은 새로운 피해원이 직관이 아니라 같은 방식으로 분류될 수 있도록 있는 것이다.
 * @{
 */
#define HHalf_physical_damage u.uprops[HALF_PHDAM].intrinsic
#define EHalf_physical_damage u.uprops[HALF_PHDAM].extrinsic
#define Half_physical_damage (HHalf_physical_damage || EHalf_physical_damage)
/** @} */

#define HRegeneration u.uprops[REGENERATION].intrinsic
#define ERegeneration u.uprops[REGENERATION].extrinsic
#define Regeneration (HRegeneration || ERegeneration)

#define HEnergy_regeneration u.uprops[ENERGY_REGENERATION].intrinsic
#define EEnergy_regeneration u.uprops[ENERGY_REGENERATION].extrinsic
#define Energy_regeneration (HEnergy_regeneration || EEnergy_regeneration)

#define HProtection u.uprops[PROTECTION].intrinsic
#define EProtection u.uprops[PROTECTION].extrinsic
#define Protection (HProtection || EProtection)

#define HProtection_from_shape_changers \
    u.uprops[PROT_FROM_SHAPE_CHANGERS].intrinsic
#define EProtection_from_shape_changers \
    u.uprops[PROT_FROM_SHAPE_CHANGERS].extrinsic
#define Protection_from_shape_changers \
    (HProtection_from_shape_changers || EProtection_from_shape_changers)

#define HPolymorph u.uprops[POLYMORPH].intrinsic
#define EPolymorph u.uprops[POLYMORPH].extrinsic
#define Polymorph (HPolymorph || EPolymorph)

#define HPolymorph_control u.uprops[POLYMORPH_CONTROL].intrinsic
#define EPolymorph_control u.uprops[POLYMORPH_CONTROL].extrinsic
#define Polymorph_control (HPolymorph_control || EPolymorph_control)

#define HUnchanging u.uprops[UNCHANGING].intrinsic
#define EUnchanging u.uprops[UNCHANGING].extrinsic
#define Unchanging (HUnchanging || EUnchanging) /* KMH */

#define HFast u.uprops[FAST].intrinsic
#define EFast u.uprops[FAST].extrinsic
/**
 * @def Fast
 * @brief Whether the hero is quicker than usual at all.
 */
/**
 * @def Fast
 * @brief 영웅이 평소보다 조금이라도 빠른지.
 */
#define Fast (HFast || EFast)
/**
 * @def Very_fast
 * @brief Whether the hero has the stronger form of speed.
 * @note The two speeds are distinguished by @e how the property was acquired rather than by a separate
 *       property: a permanently earned intrinsic gives the weaker speed, and everything else gives the
 *       stronger. Hence the mask that excludes the permanent bit and keeps the rest.
 * @warning Depends on the intrinsic word being a set of reasons and not a boolean. Code that sets it to a
 *          plain true would silently grant the stronger speed.
 */
/**
 * @def Very_fast
 * @brief 영웅이 더 강한 형태의 속도를 지니는지.
 * @note 두 속도는 별개의 속성이 아니라 그 속성을 @e 어떻게 얻었는지로 구별된다. 영구히 얻은 내재적 능력은 약한 속도를 주고, 그 밖의 모든 것은 강한 속도를 준다.
 *       영구 비트를 제외하고 나머지를 남기는 마스크가 있는 이유가 그것이다.
 * @warning 내재적 워드가 논리값이 아니라 이유의 묶음이라는 데 의존한다. 그것을 그냥 참으로 설정하는 코드는 조용히 강한 속도를 부여하게 된다.
 */
#define Very_fast ((HFast & ~INTRINSIC) || EFast)

#define HReflecting u.uprops[REFLECTING].intrinsic
#define EReflecting u.uprops[REFLECTING].extrinsic
#define Reflecting (HReflecting || EReflecting)

/**
 * @name Equipment-only properties
 * @brief Three properties that can only come from something worn.
 * @note No intrinsic form exists for these, so there is nothing to combine and the extrinsic word is the
 *       whole answer. That is a fact about the game rather than an omission: none of the three can be
 *       earned.
 * @{
 */
/**
 * @name 장비에서만 오는 속성
 * @brief 착용한 것에서만 올 수 있는 세 속성.
 * @note 이들에는 내재적 형태가 없으므로 결합할 것이 없고 외재적 워드가 답의 전부다. 그것은 빠뜨림이 아니라 게임에 관한 사실이다. 셋 중 어느 것도 얻을 수 없다.
 * @{
 */
#define Free_action u.uprops[FREE_ACTION].extrinsic /* [Tom] */

#define Fixed_abil u.uprops[FIXED_ABIL].extrinsic /* KMH */

#define Lifesaved u.uprops[LIFESAVED].extrinsic
/** @} */

/*
 * Some pseudo-properties.
 */

/* the code will needs lots of updating to use this so leave it commented
#define Riding (u.usteed != NULL)
*/

/**
 * @def Unaware
 * @brief Whether the hero is not perceiving anything at all.
 * @note Covers being unconscious and having fainted, which are separate conditions with the same
 *       consequence for perception.
 * @note The leading test on the occupation counter is redundant but deliberate: it is cheap and false most
 *       of the time, so the two function calls are usually skipped, as the existing comment records.
 */
/**
 * @def Unaware
 * @brief 영웅이 아무것도 지각하지 못하고 있는지.
 * @note 의식을 잃은 것과 기절한 것을 함께 덮는다. 별개의 상태이지만 지각에 대한 결과는 같다.
 * @note 앞에 붙은 진행 계수기 검사는 잉여이지만 의도적이다. 값이 싸고 대부분의 경우 거짓이므로, 기존 주석이 기록하듯 두 함수 호출이 보통 건너뛰어진다.
 */
/* unconscious() includes u.usleep but not is_fainted(); the multi test is
   redundant but allows the function calls to be skipped most of the time */
#define Unaware (gm.multi < 0 && (unconscious() || is_fainted()))

/**
 * @def Hate_silver
 * @brief Whether silver burns the hero.
 * @note Asks two different things: whether the hero is a lycanthrope at all, and whether their current form
 *       hates silver. Both are needed because a lycanthrope in human form is still vulnerable.
 */
/**
 * @def Hate_silver
 * @brief 은이 영웅을 태우는지.
 * @note 서로 다른 두 가지를 묻는다. 영웅이 아예 늑인간인지, 그리고 그의 현재 형태가 은을 싫어하는지. 인간 형태의 늑인간도 여전히 취약하므로 둘 다 필요하다.
 */
#define Hate_silver (u.ulycn >= LOW_PM || hates_silver(gy.youmonst.data))

/**
 * @def Half_gas_damage
 * @brief Whether a damp towel over the face is blunting noxious fumes.
 * @note Not a property but a check on one specific object in one specific state, given a property's name so
 *       the rules can ask it like the rest. The towel must be damp -- a dry one does nothing, which is why
 *       the enchantment is tested as well as the kind.
 */
/**
 * @def Half_gas_damage
 * @brief 얼굴에 두른 젖은 수건이 유해한 가스를 누그러뜨리고 있는지.
 * @note 속성이 아니라 특정 상태의 특정 물건 하나에 대한 검사이며, 규칙이 나머지처럼 물을 수 있도록 속성의 이름을 받았다. 수건은 젖어 있어야 한다. 마른 것은 아무
 *       효과가 없으며, 그래서 종류와 함께 강화도 검사된다.
 */
/* _The_Hitchhikers_Guide_to_the_Galaxy_ on uses for 'towel': "wrap it round
   your head to ward off noxious fumes" [we require it to be damp or wet] */
#define Half_gas_damage \
    (ublindf && ublindf->otyp == TOWEL && ublindf->spe > 0)

#endif /* YOUPROP_H */
