/* NetHack 5.0	explode.c	$NHDT-Date: 1781973049 2026/06/20 16:30:49 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.128 $ */
/*      Copyright (C) 1990 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file explode.c
 * @brief Explosions: one event that happens to nine squares at once.
 *
 * An explosion is not nine separate attacks. It is resolved as a whole so that
 * everything caught in it is affected by the same blast -- which matters
 * because the blast can destroy the very objects and monsters it is still
 * iterating over.
 *
 * The shape is drawn from a set of tiles that fit together into a 3x3 cloud, so
 * the display shows one explosion rather than nine flashes.
 *
 * @note Resistance is per victim and per damage type, so the same explosion can
 *       kill one creature, singe another and leave a third untouched.
 * @warning The hero may be caught in their own explosion, and an explosion can
 *          be set off by something that dies in it, so neither the source nor
 *          the victims can be assumed to survive the call.
 */

/**
 * @file explode.c
 * @brief 폭발. 아홉 칸에 동시에 일어나는 하나의 사건.
 *
 * 폭발은 아홉 번의 개별 공격이 아니다. 하나의 사건으로 해석되어, 휘말린 모든
 * 것이 같은 폭풍의 영향을 받는다. 이는 중요한데, 그 폭풍이 아직 순회 중인 객체와
 * 몬스터를 파괴할 수 있기 때문이다.
 *
 * 모양은 3x3 구름으로 맞물리는 타일 집합에서 가져온다. 그래서 화면에는 아홉 번의
 * 섬광이 아니라 하나의 폭발이 보인다.
 *
 * @note 저항은 피해자마다, 피해 종류마다 다르다. 같은 폭발이 하나는 죽이고 다른
 *       하나는 그을리며 또 다른 하나는 건드리지 않을 수 있다.
 * @warning 영웅이 자신의 폭발에 휘말릴 수 있고, 폭발이 그 안에서 죽는 무언가에
 *          의해 일어날 수도 있다. 따라서 발생원도 피해자도 이 호출을 넘겨 살아
 *          남는다고 가정할 수 없다.
 */

#include "hack.h"

staticfn int explosionmask(struct monst *, uchar, char) NONNULLARG1;
staticfn void engulfer_explosion_msg(uchar, char);

/**
 * @var explosion
 * @brief The nine display tiles that fit together into one blast cloud.
 *
 * Nine separate symbols rather than one repeated, so the cloud has corners and edges and reads as a single explosion rather than nine flashes.
 *
 * @warning Indexed column first while the screen is addressed row first, as the existing comment warns. So the two subscripts are in the opposite order from every other map array in the game, and transposing them silently draws the cloud with its corners in the
 *          wrong places -- which looks like a rendering fault rather than a wrong index.
 */
/**
 * @var explosion
 * @brief 하나의 폭풍 구름으로 맞물리는 아홉 개의 표시 타일.
 *
 * 하나가 되풀이되는 것이 아니라 아홉 개의 따로 된 기호이므로, 그 구름이 모서리와 변을 가지고 아홉 번의 섬광이 아니라 하나의 폭발로 읽힌다.
 *
 * @warning 기존 주석이 경고하듯 화면이 행 먼저 주소 지어지는 반면 이것은 열 먼저 색인된다. 그래서 두 아래첨자가 게임의 다른 모든 맵 배열과 반대 순서이며, 그것을 바꿔 놓으면 조용히 구름을 모서리가 틀린 자리에 있는 채로 그린다. 그것은 틀린 색인이 아니라 그리기 결함처럼 보인다.
 */
/* Note: Arrays are column first, while the screen is row first */
static const int explosion[3][3] = {
        { S_expl_tl, S_expl_ml, S_expl_bl },
        { S_expl_tc, S_expl_mc, S_expl_bc },
        { S_expl_tr, S_expl_mr, S_expl_br } };

/**
 * @brief What was found at one of the nine squares, and whether it shrugged the blast off.
 *
 * Recorded for every square before any damage is applied, because the shielding animation must be shown for all the squares that shield at once -- and by the time damage has been applied, a monster that resisted may have moved and one that did not may be gone.
 *
 * @note The values are bits and are combined. A square can hold both the hero and a monster -- when the hero is being swallowed -- and both may shield independently.
 * @note The skip value is not a resistance but an absence: a square outside the map. It has to be distinguished from "nothing shielded here", because the shielding animation must not be drawn off the map.
 * @note Zero means either not yet examined or examined and nothing shielded, as the existing comment concedes. Those are the same for the purpose it is used for, but it does mean the array cannot be checked for having been filled in.
 */
/**
 * @brief 아홉 칸 중 하나에서 무엇이 발견되었는지, 그리고 그것이 폭풍을 떨쳐냈는지.
 *
 * 어떤 피해가 적용되기 전에 아홉 칸 모두에 대해 기록된다. 막아내는 움직임이 막아내는 모든 칸에 대해 한꺼번에 보여야 하기 때문이며, 피해가 적용된 시점에는 저항한 몬스터가 옮겨 갔을 수 있고 저항하지 않은 몬스터는 사라졌을 수 있기 때문이다.
 *
 * @note 그 값들은 비트이며 합쳐진다. 한 칸이 영웅과 몬스터를 모두 담을 수 있고, 영웅이 삼켜지고 있을 때 그렇다. 그리고 둘 다 독립적으로 막아낼 수 있다.
 * @note 건너뛰기 값은 저항이 아니라 없음이다. 맵 밖의 칸. 그것은 "여기서 아무것도 막지 않았다"와 구별되어야 한다. 막아내는 움직임이 맵 밖에 그려져서는 안 되기 때문이다.
 * @note 기존 주석이 인정하듯 영은 아직 살펴지지 않았음이거나 살펴졌고 아무것도 막지 않았음을 뜻한다. 그것이 쓰이는 목적에는 그 둘이 같지만, 그것은 그 배열이 채워졌는지 검사될 수 없다는 뜻이기도 하다.
 */
/* what to do at [x+i][y+j] for i=-1,0,1 and j=-1,0,1 */
enum explode_action {
    EXPL_NONE = 0, /* not specified yet or no shield effect needed */
    EXPL_MON  = 1, /* monster is affected */
    EXPL_HERO = 2, /* hero is affected */
    EXPL_SKIP = 4  /* don't apply shield effect (out of bounds) */
};

/**
 * @brief Does this victim resist the blast, so that the shielding animation should be shown for it?
 *
 * Asked before damage, purely to decide what to draw. The two halves -- one for the hero, one for a monster -- ask the same question of different data, because the hero's resistances are properties of the hero and a monster's are properties of its kind.
 *
 * @param m the victim, which may be the hero
 * @param adtyp the damage type
 * @param olet the object class, which matters for one damage type only
 * @return which shielding applies, or none
 * @note Physical blasts are never resisted, so that case deliberately falls through leaving the answer as none. Nothing shields against being hit by debris.
 * @note Disintegration is the one type where the object class matters, and it inverts the usual reasoning: against a wand, what saves a creature is being already dead or a demon rather than having resistance. So the same creature can resist a disintegrating wand
 *       and not a disintegrating breath.
 * @note A monster shifted from vampire form counts as saved from a disintegrating wand while the hero has no equivalent case, so the two halves genuinely differ rather than merely being written twice.
 * @warning An unrecognised damage type is reported and treated as unresisted. That is the safer default -- the blast still happens -- but it means a new damage type added without touching this silently resists nothing.
 */
/**
 * @brief 이 피해자가 폭풍을 저항하여, 그것에 대해 막아내는 움직임이 보여야 하는가?
 *
 * 순전히 무엇을 그릴지 정하기 위해 피해 앞에 물어진다. 두 반쪽, 영웅을 위한 것과 몬스터를 위한 것은 같은 질문을 다른 데이터에 묻는다. 영웅의 저항은 영웅의 성질이고 몬스터의 것은 그 종류의 성질이기 때문이다.
 *
 * @param m 그 피해자. 영웅일 수 있다
 * @param adtyp 피해 종류
 * @param olet 물건 부류. 오직 한 피해 종류에만 중요하다
 * @return 어느 막아내기가 적용되는지, 또는 없음
 * @note 물리적 폭풍은 결코 저항되지 않으므로, 그 경우가 의도적으로 답을 없음으로 남기며 떨어진다. 무엇도 파편에 맞는 것을 막아내지 않는다.
 * @note 분해가 물건 부류가 중요한 유일한 종류이며, 그것은 보통의 추론을 뒤집는다. 지팡이에 대해 생물을 구하는 것은 저항을 가진 것이 아니라 이미 죽었거나 악마인 것이다. 그래서 같은 생물이 분해하는 지팡이는 저항하고 분해하는 숨결은 저항하지 않을 수 있다.
 * @note 흡혈귀 형태에서 바뀐 몬스터가 분해하는 지팡이에서 구해진 것으로 셈되는 반면 영웅에게는 그에 맞먹는 경우가 없다. 그래서 두 반쪽이 그저 두 번 쓰인 것이 아니라 진짜로 다르다.
 * @warning 인식되지 않는 피해 종류는 보고되고 저항되지 않은 것으로 다뤄진다. 그것이 더 안전한 기본값이다. 폭풍이 여전히 일어난다. 그러나 그것은 이것을 건드리지 않고 더해진 새 피해 종류가 조용히 아무것도 저항하지 않는다는 뜻이다.
 */
staticfn int
explosionmask(
    struct monst *m, /* target monster (might be youmonst) */
    uchar adtyp,     /* damage type */
    char olet)       /* object class (only matters for AD_DISN) */
{
    int res = EXPL_NONE;

    if (m == &gy.youmonst) {
        switch (adtyp) {
        case AD_PHYS:
            /* leave 'res' with EXPL_NONE */
            break;
        case AD_MAGM:
            if (Antimagic)
                res = EXPL_HERO;
            break;
        case AD_FIRE:
            if (Fire_resistance)
                res = EXPL_HERO;
            break;
        case AD_COLD:
            if (Cold_resistance)
                res = EXPL_HERO;
            break;
        case AD_DISN:
            if ((olet == WAND_CLASS)
                ? (nonliving(m->data) || is_demon(m->data))
                : Disint_resistance)
                res = EXPL_HERO;
            break;
        case AD_ELEC:
            if (Shock_resistance)
                res = EXPL_HERO;
            break;
        case AD_DRST:
            if (Poison_resistance)
                res = EXPL_HERO;
            break;
        case AD_ACID:
            if (Acid_resistance)
                res = EXPL_HERO;
            break;
        default:
            impossible("explosion type %d?", adtyp);
            break;
        }

    } else {
        /* 'm' is a monster */
        switch (adtyp) {
        case AD_PHYS:
            break;
        case AD_MAGM:
            if (resists_magm(m))
                res = EXPL_MON;
            break;
        case AD_FIRE:
            if (resists_fire(m))
                res = EXPL_MON;
            break;
        case AD_COLD:
            if (resists_cold(m))
                res = EXPL_MON;
            break;
        case AD_DISN:
            if ((olet == WAND_CLASS)
                ? (nonliving(m->data) || is_demon(m->data)
                   || is_vampshifter(m))
                : !!resists_disint(m))
                res = EXPL_MON;
            break;
        case AD_ELEC:
            if (resists_elec(m))
                res = EXPL_MON;
            break;
        case AD_DRST:
            if (resists_poison(m))
                res = EXPL_MON;
            break;
        case AD_ACID:
            if (resists_acid(m))
                res = EXPL_MON;
            break;
        default:
            impossible("explosion type %d?", adtyp);
            break;
        }
    }
    return res;
}

/**
 * @brief Describe what an explosion did to the monster that had swallowed the hero.
 *
 * A separate message because the situation is: the hero has set off an explosion inside something. The monster is not merely hit, it is hit from within, and the wording says so.
 *
 * @param adtyp the damage type
 * @param olet the object class, which distinguishes the two kinds of disintegration
 * @note Two whole sets of wording, chosen by whether the monster digests its prey or merely holds it. The digesting case gets afflictions of the stomach -- heartburn, an upset stomach -- because the blast went off in one; the holding case gets ordinary injuries.
 * @note The holding case says "slightly", because a monster holding the hero is less exposed to a blast inside its grip than one that has swallowed them.
 * @note Says nothing about damage. It is called alongside the damage rather than instead of it, so the amount is applied elsewhere and this only narrates.
 * @warning Reads the swallowing monster from the game state rather than taking it as an argument, so it must be called only while the hero is actually held.
 */
/**
 * @brief 영웅을 삼켰던 몬스터에게 폭발이 무엇을 했는지 서술한다.
 *
 * 따로 된 메시지인 것은 그 상황이 이렇기 때문이다. 영웅이 무엇의 안에서 폭발을 일으켰다. 그 몬스터는 그저 맞은 것이 아니라 안에서 맞았으며, 그 표현이 그렇다고 말한다.
 *
 * @param adtyp 피해 종류
 * @param olet 물건 부류. 두 종류의 분해를 구별한다
 * @note 그 몬스터가 먹이를 소화하는지 그저 붙들고 있는지로 골라지는 두 벌의 온전한 표현이 있다. 소화하는 경우는 속병을 얻는다. 속쓰림, 배탈. 폭풍이 그 안에서 일어났기 때문이다. 붙드는 경우는 보통의 상처를 얻는다.
 * @note 붙드는 경우가 "약간"이라고 말하는데, 영웅을 붙든 몬스터가 삼킨 몬스터보다 그 움큼 안의 폭풍에 덜 드러나 있기 때문이다.
 * @note 피해에 대해 아무것도 말하지 않는다. 그것 대신이 아니라 그것과 나란히 호출되므로, 그 양은 다른 곳에서 적용되고 이것은 이야기만 한다.
 * @warning 삼킨 몬스터를 인자로 취하는 대신 게임 상태에서 읽으므로, 영웅이 실제로 붙들려 있는 동안에만 호출되어야 한다.
 */
staticfn void
engulfer_explosion_msg(uchar adtyp, char olet)
{
    const char *adj = (char *) 0;

    if (digests(u.ustuck->data)) {
        switch (adtyp) {
        case AD_FIRE:
            adj = "heartburn";
            break;
        case AD_COLD:
            adj = "chilly";
            break;
        case AD_DISN:
            if (olet == WAND_CLASS)
                adj = "irradiated by pure energy";
            else
                adj = "perforated";
            break;
        case AD_ELEC:
            adj = "shocked";
            break;
        case AD_DRST:
            adj = "poisoned";
            break;
        case AD_ACID:
            adj = "an upset stomach";
            break;
        default:
            adj = "fried";
            break;
        }
        pline("%s gets %s!", Monnam(u.ustuck), adj);
    } else {
        switch (adtyp) {
        case AD_FIRE:
            adj = "toasted";
            break;
        case AD_COLD:
            adj = "chilly";
            break;
        case AD_DISN:
            if (olet == WAND_CLASS)
                adj = "overwhelmed by pure energy";
            else
                adj = "perforated";
            break;
        case AD_ELEC:
            adj = "shocked";
            break;
        case AD_DRST:
            adj = "intoxicated";
            break;
        case AD_ACID:
            adj = "burned";
            break;
        default:
            adj = "fried";
            break;
        }
        pline("%s gets slightly %s!", Monnam(u.ustuck), adj);
    }
}

/* Note: I had to choose one of three possible kinds of "type" when writing
 * this function: a wand type (like in zap.c), an adtyp, or an object type.
 * Wand types get complex because they must be converted to adtyps for
 * determining such things as fire resistance.  Adtyps get complex in that
 * they don't supply enough information--was it a player or a monster that
 * did it, and with a wand, spell, or breath weapon?  Object types share both
 * these disadvantages....
 *
 * Note: anything with a AT_BOOM AD_PHYS attack uses PHYS_EXPL_TYPE for type.
 *
 * Important note about Half_physical_damage:
 *      Unlike losehp(), explode() makes the Half_physical_damage adjustments
 *      itself, so the caller should never have done that ahead of time.
 *      It has to be done this way because the damage value is applied to
 *      things beside the player. Care is taken within explode() to ensure
 *      that Half_physical_damage only affects the damage applied to the hero.
 */
/**
 * @brief Set off an explosion: draw it, and apply it to everything in its nine squares.
 *
 * The whole of what an explosion is. It resolves as one event rather than nine attacks, which matters because the blast destroys things it has not finished iterating over -- so what is present, and what shields, is determined for all nine squares before any
 * damage is applied.
 *
 * @param x the centre column; the eight surrounding squares are affected too
 * @param y the centre row
 * @param type what kind of blast, in the same encoding the zapping code uses; negative means a wand's object type instead
 * @param dam how much damage
 * @param olet the object class responsible, or one of several special values for burning oil, a monster's own explosion, or a trap
 * @param expltype which colour of blast to draw; negative additionally means the hero should be blamed for the monster at the centre
 * @note The choice of how to identify the blast is discussed at length in the existing comment above, and the conclusion is worth keeping: all three candidates were unsatisfactory. Wand types need converting before resistance can be judged; damage types do not say
 *       who caused it or with what; object types have both faults. So this takes several arguments that overlap rather than one that would suffice.
 * @warning Two arguments carry a second meaning in their sign, and neither is obvious. A negative blast kind is a wand's object type, converted here into a blast kind and remembered separately. A negative colour means the hero should be credited with killing whatever
 *          is at the centre -- used when the hero burns off a slime, so the kill is theirs. A caller passing a computed negative by accident changes what the explosion means.
 * @note Halved physical damage is applied here rather than by the caller, and the existing comment insists on this: the damage figure is applied to monsters as well, so halving it beforehand would halve theirs too. The halving is confined within this routine to the
 *       hero's share.
 * @note A retributive wand strike is reduced for the roles that would understand wands, which is the mechanical form of the idea that such a character mishandles one less badly.
 * @note Being swallowed narrows the explosion to the hero and the swallower alone. Nothing outside is affected, because the blast went off inside something.
 * @note Being held but not swallowed is treated differently again: the holder is reaching across, so it can be caught by the blast twice.
 * @warning Neither the cause nor the victims can be assumed to exist afterwards. A monster that explodes dies in its own blast, and the hero may be killed by an explosion they set off.
 */
/**
 * @brief 폭발을 일으킨다. 그것을 그리고, 그 아홉 칸의 모든 것에 그것을 적용한다.
 *
 * 폭발이 무엇인지의 전부다. 그것은 아홉 번의 공격이 아니라 하나의 사건으로 해석되며, 그것이 중요한 것은 폭풍이 자기가 아직 순회를 마치지 않은 것을 파괴하기 때문이다. 그래서 무엇이 있고 무엇이 막아내는지가 어떤 피해가 적용되기 전에 아홉 칸 모두에 대해 정해진다.
 *
 * @param x 중심 열. 둘러싼 여덟 칸도 영향받는다
 * @param y 중심 행
 * @param type 어떤 종류의 폭풍인지. 지팡이 쏘기 코드가 쓰는 것과 같은 부호화로. 음수는 대신 지팡이의 물건 종류를 뜻한다
 * @param dam 얼마만큼의 피해
 * @param olet 책임 있는 물건 부류, 또는 불타는 기름, 몬스터 자신의 폭발, 함정을 위한 몇 특별한 값 중 하나
 * @param expltype 어느 색의 폭풍을 그릴지. 음수는 더해서 중심의 몬스터에 대해 영웅이 책임을 져야 함을 뜻한다
 * @note 폭풍을 어떻게 식별할지에 대한 선택이 위의 기존 주석에서 길게 논의되며, 그 결론이 지녀 둘 만하다. 세 후보 모두가 만족스럽지 않았다. 지팡이 종류는 저항이 판단될 수 있기 전에 변환을 필요로 한다. 피해 종류는 누가 무엇으로 그것을 일으켰는지 말하지 않는다. 물건 종류는 두 결함을 모두
 *       가진다. 그래서 이것은 충분할 하나가 아니라 겹치는 여러 인자를 취한다.
 * @warning 두 인자가 자기 부호에 두 번째 뜻을 지니며, 어느 것도 뻔하지 않다. 음수 폭풍 종류는 지팡이의 물건 종류이며, 여기서 폭풍 종류로 변환되고 따로 기억된다. 음수 색은 영웅이 중심에 있는 무엇이든을 죽인 것으로 셈되어야 함을 뜻한다. 영웅이 점액을 태워 없앨 때 쓰이며, 그래서 그 죽임이 그의
 *          것이 된다. 우연히 계산된 음수를 넘기는 호출자는 그 폭발이 뜻하는 것을 바꾼다.
 * @note 반으로 줄어드는 물리 피해가 호출자가 아니라 여기서 적용되며, 기존 주석이 그것을 고집한다. 그 피해 수치가 몬스터에게도 적용되므로, 미리 그것을 반으로 하는 것은 그들의 것도 반으로 할 것이다. 그 반으로 하기는 이 함수 안에서 영웅의 몫으로 한정된다.
 * @note 응징하는 지팡이 폭발이 지팡이를 이해할 만한 직업에는 줄어드는데, 그것은 그런 인물이 지팡이를 덜 나쁘게 잘못 다룬다는 생각의 기계적 형태다.
 * @note 삼켜져 있는 것은 폭발을 영웅과 삼킨 것 둘만으로 좁힌다. 밖의 무엇도 영향받지 않는다. 폭풍이 무엇의 안에서 일어났기 때문이다.
 * @note 삼켜지지 않고 붙들려 있는 것은 또 다르게 다뤄진다. 붙든 것이 건너 뻗고 있으므로, 그것은 폭풍에 두 번 맞을 수 있다.
 * @warning 원인도 피해자도 뒤에 존재한다고 가정할 수 없다. 폭발하는 몬스터는 자기 폭풍에 죽고, 영웅은 자기가 일으킨 폭발에 죽을 수 있다.
 */
void
explode(
    coordxy x, coordxy y, /* explosion's location;
                           * adjacent spots are also affected */
    int type,     /* same as in zap.c; -(wand typ) for some WAND_CLASS */
    int dam,      /* damage amount */
    char olet,    /* object class or BURNING_OIL or MON_EXPLODE */
    int expltype) /* explosion type: controls color of explosion glyphs */
{
    int i, j, k, damu = dam;
    boolean starting = 1;
    boolean visible, any_shield;
    int uhurt = 0; /* 0=unhurt, 1=items damaged, 2=you and items damaged */
    const char *str = (const char *) 0;
    struct monst *mtmp, *mdef = 0;
    uchar adtyp;
    int explmask[3][3]; /* 0=normal explosion, 1=do shieldeff, 2=do nothing */
    coordxy xx, yy;
    boolean shopdamage = FALSE, generic = FALSE,
            do_hallu = FALSE, inside_engulfer, grabbed, grabbing;
    coord grabxy;
    char hallu_buf[BUFSZ], killr_buf[BUFSZ];
    short exploding_wand_typ = 0;
    boolean you_exploding = (olet == MON_EXPLODE && type >= 0);
    boolean didmsg = FALSE;

    if (olet == WAND_CLASS) { /* retributive strike */
        /* 'type' is passed as (wand's object type * -1); save
           object type and convert 'type' itself to zap-type */
        if (type < 0) {
            type = -type;
            exploding_wand_typ = (short) type;
            /* most attack wands produce specific explosions;
               other types produce a generic magical explosion */
            if (objects[type].oc_dir == RAY
                && type != WAN_DIGGING && type != WAN_SLEEP) {
                type -= WAN_MAGIC_MISSILE;
                if (type < 0 || type > 9) {
                    impossible("explode: wand has bad zap type (%d).", type);
                    type = 0;
                }
            } else
                type = 0;
        }
        switch (Role_switch) {
        case PM_CLERIC:
        case PM_MONK:
        case PM_WIZARD:
            damu /= 5;
            break;
        case PM_HEALER:
        case PM_KNIGHT:
            damu /= 2;
            break;
        default:
            break;
        }
    } else if (olet == BURNING_OIL) {
        /* used to provide extra information to zap_over_floor() */
        exploding_wand_typ = POT_OIL;
    } else if (olet == SCROLL_CLASS) {
        /* ditto */
        exploding_wand_typ = SCR_FIRE;
    } else if (olet == TRAP_EXPLODE) {
        type = 0; /* hardcoded to generic magic explosion */
    }
    /* muse_unslime: SCR_FIRE */
    if (expltype < 0) {
        /* hero gets credit/blame for killing this monster, not others */
        mdef = m_at(x, y);
        expltype = -expltype;
    }
    /* if hero is engulfed and caused the explosion, only hero and
       engulfer will be affected */
    inside_engulfer = (u.uswallow && type >= 0);
    /* held but not engulfed implies holder is reaching into second spot
       so might get hit by double damage */
    grabbed = grabbing = FALSE;
    if (u.ustuck && !u.uswallow) {
        if (Upolyd && sticks(gy.youmonst.data))
            grabbing = TRUE;
        else
            grabbed = TRUE;
        grabxy.x = u.ustuck->mx;
        grabxy.y = u.ustuck->my;
    } else
        grabxy.x = grabxy.y = 0; /* lint suppression */
    /* FIXME:
     *  It is possible for a grabber to be outside the explosion
     *  radius and reaching inside to hold the hero.  If so, it ought
     *  to take damage (the extra half of double damage).  It is also
     *  possible for poly'd hero to be outside the radius and reaching
     *  in to hold a monster.  Hero should take damage in that situation.
     *
     *  Probably the simplest way to handle this would be to expand
     *  the radius used when collecting targets but exclude everything
     *  beyond the regular radius which isn't reaching inside.  Then
     *  skip harm to gear of any extended targets when inflicting damage.
     */

    if (olet == MON_EXPLODE && !you_exploding) {
        /* when explode() is called recursively, svk.killer.name might change
           so retain a copy of the current value for this explosion */
        str = strcpy(killr_buf, svk.killer.name);
        do_hallu = (Hallucination
                    && (strstri(str, "'s explosion")
                        || strstri(str, "s' explosion")));
    }
    if (type == PHYS_EXPL_TYPE) {
        /* currently only gas spores */
        adtyp = AD_PHYS;
    } else {
        /* If str is e.g. "flaming sphere's explosion" from above, we want to
         * still assign adtyp appropriately, but not replace str. */
        const char *adstr = NULL;

        switch (abs(type) % 10) {
        case 0:
            adstr = "magical blast";
            adtyp = AD_MAGM;
            break;
        case 1:
            adstr = (olet == BURNING_OIL) ? "burning oil"
                     : (olet == SCROLL_CLASS) ? "tower of flame" : "fireball";
            /* fire damage, not physical damage */
            adtyp = AD_FIRE;
            break;
        case 2:
            adstr = "ball of cold";
            adtyp = AD_COLD;
            break;
        case 4:
            adstr = (olet == WAND_CLASS) ? "death field"
                                         : "disintegration field";
            adtyp = AD_DISN;
            break;
        case 5:
            adstr = "ball of lightning";
            adtyp = AD_ELEC;
            break;
        case 6:
            adstr = "poison gas cloud";
            adtyp = AD_DRST;
            break;
        case 7:
            adstr = "splash of acid";
            adtyp = AD_ACID;
            break;
        default:
            impossible("explosion base type %d?", type);
            return;
        }
        if (!str)
            str = adstr;
    }

    any_shield = visible = FALSE;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++) {
            xx = x + i - 1;
            yy = y + j - 1;
            if (!isok(xx, yy)) {
                explmask[i][j] = EXPL_SKIP;
                continue;
            }
            explmask[i][j] = EXPL_NONE;

            if (u_at(xx, yy)) {
                explmask[i][j] = explosionmask(&gy.youmonst, adtyp, olet);
            }
            /* can be both you and mtmp if you're swallowed or riding */
            mtmp = m_at(xx, yy);
            if (!mtmp && u_at(xx, yy))
                mtmp = u.usteed;
            if (mtmp && DEADMONSTER(mtmp))
                mtmp = 0;
            if (mtmp) {
                explmask[i][j] |= explosionmask(mtmp, adtyp, olet);
            }

            if (mtmp && cansee(xx, yy) && !canspotmon(mtmp))
                map_invisible(xx, yy);
            else if (!mtmp)
                (void) unmap_invisible(xx, yy);
            if (cansee(xx, yy))
                visible = TRUE;
            if ((explmask[i][j] & (EXPL_MON | EXPL_HERO)) != 0)
                any_shield = TRUE;
        }

    if (visible) {
        /* Start the explosion */
        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++) {
                if (explmask[i][j] == EXPL_SKIP)
                    continue;
                xx = x + i - 1;
                yy = y + j - 1;
                tmp_at(starting ? DISP_BEAM : DISP_CHANGE,
                       explosion_to_glyph(expltype, explosion[i][j]));
                tmp_at(xx, yy);
                starting = 0;
            }
        curs_on_u(); /* will flush screen and output */

        if (any_shield && flags.sparkle) { /* simulate shield effect */
            for (k = 0; k < SHIELD_COUNT; k++) {
                for (i = 0; i < 3; i++)
                    for (j = 0; j < 3; j++) {
                        xx = x + i - 1;
                        yy = y + j - 1;
                        if ((explmask[i][j] & (EXPL_MON | EXPL_HERO)) != 0)
                            /*
                             * Bypass tmp_at() and send the shield glyphs
                             * directly to the buffered screen.  tmp_at()
                             * will clean up the location for us later.
                             */
                            show_glyph(xx, yy,
                                       cmap_to_glyph(shield_static[k]));
                    }
                curs_on_u(); /* will flush screen and output */
                nh_delay_output();
            }

            /* Cover last shield glyph with blast symbol. */
            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++) {
                    xx = x + i - 1;
                    yy = y + j - 1;
                    if ((explmask[i][j] & (EXPL_MON | EXPL_HERO)) != 0)
                        show_glyph(xx, yy,
                                   explosion_to_glyph(expltype,
                                                      explosion[i][j]));
                }

        } else { /* delay a little bit. */
            nh_delay_output();
            nh_delay_output();
        }

        tmp_at(DISP_END, 0); /* clear the explosion */
    } else {
        if (olet == MON_EXPLODE || olet == TRAP_EXPLODE) {
            str = "explosion";
            generic = TRUE;
        }
        if (!Deaf && olet != SCROLL_CLASS) {
            Soundeffect(se_blast, 75);
            You_hear("a blast.");
            didmsg = TRUE;
        }
    }

    if (!Deaf && !didmsg)
        pline("Boom!");

    /* apply effects to monsters and floor objects first, in case the
       damage to the hero is fatal and leaves bones */
    if (dam) {
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                int itemdmg = 0;

                if (explmask[i][j] == EXPL_SKIP)
                    continue;
                xx = x + i - 1;
                yy = y + j - 1;
                if (u_at(xx, yy)) {
                    uhurt = ((explmask[i][j] & EXPL_HERO) != 0) ? 1 : 2;
                    /* If the player is attacking via polyself into something
                     * with an explosion attack, leave them (and their gear)
                     * unharmed, to avoid punishing them from using such
                     * polyforms creatively */
                    if (!svc.context.mon_moving && you_exploding)
                        uhurt = 0;
                } else if (inside_engulfer) {
                    /* for inside_engulfer, only <u.ux,u.uy> is affected */
                    continue;
                }

                /* Affect the floor unless the player caused the explosion
                 * from inside their engulfer. */
                if (!(u.uswallow && !svc.context.mon_moving))
                    (void) zap_over_floor(xx, yy, type,
                                          &shopdamage, FALSE,
                                          exploding_wand_typ);

                mtmp = m_at(xx, yy);
                if (!mtmp && u_at(xx, yy))
                    mtmp = u.usteed;
                if (!mtmp)
                    continue;
                if (do_hallu) {
                    int tryct = 0;

                    /* replace "gas spore" with a different description
                       for each target (we can't distinguish personal names
                       like "Barney" here in order to suppress "the" below,
                       so avoid any which begins with a capital letter) */
                    do {
                        Sprintf(hallu_buf, "%s explosion",
                                s_suffix(rndmonnam((char *) 0)));
                    } while (*hallu_buf != lowc(*hallu_buf) && ++tryct < 20);
                    str = hallu_buf;
                }
                if (engulfing_u(mtmp)) {
                    engulfer_explosion_msg(adtyp, olet);
                } else if (cansee(xx, yy)) {
                    if (mtmp->m_ap_type)
                        seemimic(mtmp);
                    pline("%s is caught in the %s!", Monnam(mtmp), str);
                }

                itemdmg = destroy_items(mtmp, (int) adtyp, dam);
                if (adtyp == AD_FIRE) {
                    (void) burnarmor(mtmp);
                    ignite_items(mtmp->minvent);
                }

                if ((explmask[i][j] & EXPL_MON) != 0) {
                    /* Damage from ring/wand explosion isn't itself
                     * electrical in nature, nor is damage from freezing
                     * potion really cold in nature, nor is damage from
                     * boiling potion or exploding oil; only burning items
                     * damage is the "same type" as the explosion.  Because
                     * this is imperfect and marginal (burning items only
                     * deal 1 damage), ignore it for golemeffects(). */
                    golemeffects(mtmp, (int) adtyp, dam);
                    mtmp->mhp -= itemdmg; /* item destruction dmg */
                } else {
                    /* Call resist with 0 and do damage manually so 1) we can
                     * get out the message before doing the damage, and 2) we
                     * can call mondied, not killed, if it's not your blast.
                     */
                    int mdam = dam;

                    if (resist(mtmp, olet, 0, FALSE)) {
                        /* inside_engulfer: <xx,yy> == <u.ux,u.uy> */
                        if (cansee(xx, yy) || inside_engulfer)
                            pline("%s resists the %s!", Monnam(mtmp), str);
                        mdam = (dam + 1) / 2;
                    }
                    /* if grabber is reaching into hero's spot and
                       hero's spot is within explosion radius, grabber
                       gets hit by double damage */
                    if (grabbed && mtmp == u.ustuck && next2u(x, y))
                        mdam *= 2;
                    /* being resistant to opposite type of damage makes
                       target more vulnerable to current type of damage
                       (when target is also resistant to current type,
                       we won't get here) */
                    if (resists_cold(mtmp) && adtyp == AD_FIRE)
                        mdam *= 2;
                    else if (resists_fire(mtmp) && adtyp == AD_COLD)
                        mdam *= 2;
                    mtmp->mhp -= mdam + itemdmg;
                }
                if (DEADMONSTER(mtmp)) {
                    int xkflg = ((adtyp == AD_FIRE
                                  && completelyburns(mtmp->data))
                                 ? XKILL_NOCORPSE : 0);

                    if (!svc.context.mon_moving) {
                        xkilled(mtmp, XKILL_GIVEMSG | xkflg);
                    } else if (mdef && mtmp == mdef) {
                        /* 'mdef' killed self trying to cure being turned
                         * into slime due to some action by the player.
                         * Hero gets the credit (experience) and most of
                         * the blame (possible loss of alignment and/or
                         * luck and/or telepathy depending on mtmp) but
                         * doesn't break pacifism.  xkilled()'s message
                         * would be "you killed <mdef>" so give our own.
                         */
                        if (cansee(mtmp->mx, mtmp->my) || canspotmon(mtmp))
                            pline("%s is %s!", Monnam(mtmp),
                                  xkflg ? "burned completely"
                                        : nonliving(mtmp->data) ? "destroyed"
                                                                : "killed");
                        xkilled(mtmp, XKILL_NOMSG | XKILL_NOCONDUCT | xkflg);
                    } else {
                        if (xkflg)
                            adtyp = AD_RBRE; /* no corpse */
                        monkilled(mtmp, "", (int) adtyp);
                    }
                } else if (!svc.context.mon_moving) {
                    /* all affected monsters, even if mdef is set */
                    setmangry(mtmp, TRUE);
                }
            }
        }
    }

    /* Do your injury last */
    if (uhurt) {
        /* give message for any monster-induced explosion
           or player-induced one other than scroll of fire */
        if (flags.verbose && (type < 0 || olet != SCROLL_CLASS)) {
            if (do_hallu) { /* (see explanation above) */
                do {
                    Sprintf(hallu_buf, "%s explosion",
                            s_suffix(rndmonnam((char *) 0)));
                } while (*hallu_buf != lowc(*hallu_buf));
                str = hallu_buf;
            }
            You("are caught in the %s!", str);
            iflags.last_msg = PLNMSG_CAUGHT_IN_EXPLOSION;
        }
        /* do property damage first, in case we end up leaving bones */
        if (adtyp == AD_FIRE)
            burn_away_slime();
        if (Invulnerable) {
            damu = 0;
            You("are unharmed!");
        } else if (adtyp == AD_PHYS || adtyp == AD_ACID)
            damu = Maybe_Half_Phys(damu);
        if (adtyp == AD_FIRE) {
            (void) burnarmor(&gy.youmonst);
            ignite_items(gi.invent);
        }
        (void) destroy_items(&gy.youmonst, (int) adtyp, dam);

        ugolemeffects((int) adtyp, damu);
        if (uhurt == 2) {
            /* if poly'd hero is grabbing another victim, hero takes
               double damage (note: don't rely on u.ustuck here because
               that victim might have been killed when hit by the blast) */
            if (grabbing && dist2((int) grabxy.x, (int) grabxy.y, x, y) <= 2)
                damu *= 2;
            /* hero does not get same fire-resistant vs cold and
               cold-resistant vs fire double damage as monsters [why not?] */
            if (Upolyd)
                u.mh -= damu;
            else
                u.uhp -= damu;
            disp.botl = TRUE;
        }

        /* You resisted the damage, lets not keep that to ourselves */
        if (uhurt == 1)
            monstseesu_ad(adtyp);
        else
            monstunseesu_ad(adtyp);

        if (u.uhp <= 0 || (Upolyd && u.mh <= 0)) {
            if (Upolyd) {
                rehumanize();
            } else {
                if (olet == MON_EXPLODE) {
                    if (generic) /* explosion was unseen; str=="explosion", */
                        ; /* svk.killer.name=="gas spore's explosion". */
                    else if (str != svk.killer.name && str != hallu_buf)
                        Strcpy(svk.killer.name, str);
                    svk.killer.format = KILLED_BY_AN;
                } else if (olet == TRAP_EXPLODE) {
                    svk.killer.format = NO_KILLER_PREFIX;
                    Snprintf(svk.killer.name, sizeof svk.killer.name,
                             "caught %sself in a %s", uhim(),
                             str);
                } else if (type >= 0 && olet != SCROLL_CLASS) {
                    svk.killer.format = NO_KILLER_PREFIX;
                    Snprintf(svk.killer.name, sizeof svk.killer.name,
                             "caught %sself in %s own %s", uhim(),
                             uhis(), str);
                } else {
                    svk.killer.format = (!strcmpi(str, "tower of flame")
                                     || !strcmpi(str, "fireball"))
                                        ? KILLED_BY_AN
                                        : KILLED_BY;
                    Strcpy(svk.killer.name, str);
                }
                if (iflags.last_msg == PLNMSG_CAUGHT_IN_EXPLOSION
                    || iflags.last_msg == PLNMSG_TOWER_OF_FLAME) /*seffects()*/
                    pline("It is fatal.");
                else
                    pline_The("%s is fatal.", str);
                /* Known BUG: BURNING suppresses corpse in bones data,
                   but done does not handle killer reason correctly */
                done((adtyp == AD_FIRE) ? BURNING : DIED);
            }
        }
        exercise(A_STR, FALSE);
    }

    if (shopdamage) {
        pay_for_damage((adtyp == AD_FIRE) ? "burn away"
                          : (adtyp == AD_COLD) ? "shatter"
                             : (adtyp == AD_DISN) ? "disintegrate"
                                : "destroy",
                       FALSE);
    }

    /* explosions are noisy */
    i = dam * dam;
    if (i < 50)
        i = 50; /* in case random damage is very small */
    if (inside_engulfer)
        i = (i + 3) / 4;
    wake_nearto(x, y, i);
}

/**
 * @brief One object in flight while a pile is being scattered.
 *
 * A list of these exists only for the duration of a scattering. Every object gets one, and they are all advanced a step at a time so the objects appear to fly outward together rather than one landing before the next sets off.
 *
 * That simultaneity is the whole reason for the structure. Moving each object to its destination in turn would be simpler and would look wrong.
 *
 * @note Each entry carries the object's own position rather than reading it from the object, because an object in flight is not on the map -- it has been taken up and has not landed.
 * @note The remaining range is per object, so objects thrown from one pile stop at different distances.
 * @note The stopped flag is kept rather than the entry being removed, because the list is walked until every entry has stopped and removing during the walk would complicate it.
 */
/**
 * @brief 무더기가 흩뿌려지는 동안 날고 있는 물건 하나.
 *
 * 이것들의 목록은 흩뿌리기 동안에만 존재한다. 모든 물건이 하나를 얻고, 그것들이 모두 한 걸음씩 나아가므로 물건들이 하나가 내려앉고 다음이 출발하는 것이 아니라 함께 밖으로 날아가는 것처럼 보인다.
 *
 * 그 동시성이 이 구조체의 이유 전부다. 각 물건을 차례로 그 목적지로 옮기는 것이 더 단순하겠지만 틀리게 보일 것이다.
 *
 * @note 각 항목이 물건의 위치를 물건에서 읽는 대신 스스로 지닌다. 날고 있는 물건은 맵 위에 없기 때문이다. 그것은 들어올려졌고 내려앉지 않았다.
 * @note 남은 사거리가 물건마다이므로, 한 무더기에서 던져진 물건들이 다른 거리에서 멈춘다.
 * @note 항목이 없어지는 대신 멈춤 표시가 지녀지는데, 그 목록이 모든 항목이 멈출 때까지 걸어지고 걸어가는 동안 없애는 것이 그것을 복잡하게 만들 것이기 때문이다.
 */
struct scatter_chain {
    struct scatter_chain *next; /* pointer to next scatter item */
    struct obj *obj;            /* pointer to the object        */
    coordxy ox;                 /* location of                  */
    coordxy oy;                 /*      item                    */
    schar dx;                   /* direction of                 */
    schar dy;                   /*      travel                  */
    int range;                  /* range of object              */
    boolean stopped;            /* flag for in-motion/stopped   */
};

/*
 * scflags:
 *      VIS_EFFECTS     Add visual effects to display
 *      MAY_HITMON      Objects may hit monsters
 *      MAY_HITYOU      Objects may hit hero
 *      MAY_HIT         Objects may hit you or monsters
 *      MAY_DESTROY     Objects may be destroyed at random
 *      MAY_FRACTURE    Stone objects can be fractured (statues, boulders)
 */

/**
 * @brief Throw a pile of objects outward from a square.
 *
 * What a blast does to the things lying where it went off. The objects fly in random directions to distances depending on the force, and may hit whatever they pass, be destroyed, or in the case of stone be broken apart.
 *
 * Which of those may happen is chosen by the caller through the flags, described in the existing comment above. That is because the same routine serves several causes with different consequences -- a boulder pushed apart, a bag of holding failing, a wand of
 * digging aimed downward -- and they do not all injure bystanders or destroy goods.
 *
 * @param sx the column the objects are on
 * @param sy the row
 * @param blastforce how hard they are thrown, which decides how far they can go
 * @param scflags which effects are permitted
 * @param obj a single object to scatter instead of the whole pile, or none for all of them
 * @return how many objects were scattered
 * @note Scattering a single object rather than the pile is a distinct mode, not a special case of the general one. It is used where one object bursts and the rest of the pile is not involved.
 * @note Objects that were a shop's property are tracked, because scattering them out of the shop makes them stolen -- so the shopkeeper must be told even though the hero did not carry anything out.
 * @note Every object is advanced one step per pass rather than each being flown to its destination in turn, so they appear to travel together.
 * @warning Objects may be destroyed or used up during the scattering, so the caller must not hold a pointer to anything in the pile across this call. Passing a single object means passing something that may not exist when it returns.
 */
/**
 * @brief 한 칸에서 물건 무더기를 밖으로 던진다.
 *
 * 폭풍이 자기가 일어난 자리에 놓인 것들에 하는 일이다. 물건들이 무작위 방향으로 힘에 따라 달라지는 거리만큼 날아가며, 자기가 지나는 무엇이든 맞힐 수도, 파괴될 수도, 돌인 경우에는 부서질 수도 있다.
 *
 * 그중 무엇이 일어날 수 있는지는 호출자가 표시를 통해 고르며, 위의 기존 주석이 그것을 서술한다. 같은 함수가 다른 결과를 가진 여러 원인을 섬기기 때문이다. 밀려 부서진 바위, 실패한 담는 가방, 아래로 겨눈 굴착의 지팡이. 그리고 그것들이 모두 옆의 것을 다치게 하거나 상품을 파괴하지는 않는다.
 *
 * @param sx 물건들이 있는 열
 * @param sy 그 행
 * @param blastforce 그것들이 얼마나 세게 던져지는지. 얼마나 멀리 갈 수 있는지를 정한다
 * @param scflags 어느 효과가 허용되는지
 * @param obj 무더기 전체 대신 흩뿌릴 단 하나의 물건, 또는 그것들 모두에 없음
 * @return 몇 개의 물건이 흩뿌려졌는지
 * @note 무더기가 아니라 하나의 물건을 흩뿌리는 것은 일반적인 것의 특별한 경우가 아니라 별개의 방식이다. 그것은 물건 하나가 터지고 무더기의 나머지가 관여하지 않는 곳에 쓰인다.
 * @note 상점의 소유였던 물건이 좇아지는데, 그것을 상점 밖으로 흩뿌리는 것이 그것을 훔쳐진 것으로 만들기 때문이다. 그래서 영웅이 아무것도 들고 나가지 않았음에도 주인이 알려져야 한다.
 * @note 각각이 차례로 자기 목적지로 날아가는 대신 모든 물건이 한 번 지나갈 때마다 한 걸음 나아가므로, 그것들이 함께 가는 것처럼 보인다.
 * @warning 흩뿌리는 동안 물건이 파괴되거나 다 쓰일 수 있으므로, 호출자는 이 호출을 넘어 그 무더기의 무엇에 대한 포인터도 지녀서는 안 된다. 하나의 물건을 넘기는 것은 그것이 돌아올 때 존재하지 않을 수 있는 무엇을 넘기는 것을 뜻한다.
 */
long
scatter(
    coordxy sx, coordxy sy,  /* location of objects to scatter */
    int blastforce,          /* force behind the scattering */
    unsigned int scflags,
    struct obj *obj)         /* only scatter this obj */
{
    struct obj *otmp;
    int tmp;
    int farthest = 0;
    uchar typ;
    long qtmp;
    boolean used_up;
    boolean individual_object = obj ? TRUE : FALSE;
    boolean shop_origin, lostgoods = FALSE;
    struct monst *mtmp, *shkp = 0;
    struct scatter_chain *stmp, *stmp2 = 0;
    struct scatter_chain *schain = (struct scatter_chain *) 0;
    long total = 0L;

    if (individual_object && (obj->ox != sx || obj->oy != sy))
        impossible("scattered object <%d,%d> not at scatter site <%d,%d>",
                   obj->ox, obj->oy, sx, sy);

    shop_origin = ((shkp = shop_keeper(*in_rooms(sx, sy, SHOPBASE))) != 0
                && costly_spot(sx, sy));
    if (shop_origin)
        credit_report(shkp, 0, TRUE);   /* establish baseline, without msgs */

    while ((otmp = (individual_object ? obj
                                      : svl.level.objects[sx][sy])) != 0) {
        if (otmp == uball || otmp == uchain) {
            boolean waschain = (otmp == uchain);

            Soundeffect(se_chain_shatters, 25);
            pline_The("chain shatters!");
            unpunish();
            if (waschain)
                continue;
        }
        if (otmp->quan > 1L) {
            qtmp = otmp->quan - 1L;
            if (qtmp > LARGEST_INT)
                qtmp = LARGEST_INT;
            qtmp = (long) rnd((int) qtmp);
            otmp = splitobj(otmp, qtmp);
        } else {
            obj = (struct obj *) 0; /* all used */
        }
        obj_extract_self(otmp);
        used_up = FALSE;

        /* 9 in 10 chance of fracturing boulders or statues */
        if ((scflags & MAY_FRACTURE) != 0
            && (otmp->otyp == BOULDER || otmp->otyp == STATUE)
            && rn2(10)) {
            if (otmp->otyp == BOULDER) {
                if (cansee(sx, sy)) {
                    pline("%s apart.", Tobjnam(otmp, "break"));
                } else {
                    Soundeffect(se_stone_breaking, 100);
                    You_hear("stone breaking.");
                }
                fracture_rock(otmp);
                place_object(otmp, sx, sy);
                if ((otmp = sobj_at(BOULDER, sx, sy)) != 0) {
                    /* another boulder here, restack it to the top */
                    obj_extract_self(otmp);
                    place_object(otmp, sx, sy);
                }
            } else {
                struct trap *trap;

                if ((trap = t_at(sx, sy)) && trap->ttyp == STATUE_TRAP)
                    deltrap(trap);
                if (cansee(sx, sy)) {
                    pline("%s.", Tobjnam(otmp, "crumble"));
                } else {
                    Soundeffect(se_stone_crumbling, 100);
                    You_hear("stone crumbling.");
                }
                (void) break_statue(otmp);
                place_object(otmp, sx, sy); /* put fragments on floor */
            }
            newsym(sx, sy); /* in case it's beyond radius of 'farthest' */
            used_up = TRUE;

            /* 1 in 10 chance of destruction of obj; glass, egg destruction */
        } else if ((scflags & MAY_DESTROY) != 0
                   && (!rn2(10) || (objects[otmp->otyp].oc_material == GLASS
                                    || otmp->otyp == EGG))) {
            if (breaks(otmp, sx, sy))
                used_up = TRUE;
        }

        if (!used_up) {
            stmp = (struct scatter_chain *) alloc(sizeof *stmp);
            stmp->next = (struct scatter_chain *) 0;
            stmp->obj = otmp;
            stmp->ox = sx;
            stmp->oy = sy;
            tmp = rn2(N_DIRS); /* get the direction */
            stmp->dx = xdir[tmp];
            stmp->dy = ydir[tmp];
            tmp = blastforce - (otmp->owt / 40);
            if (tmp < 1)
                tmp = 1;
            stmp->range = rnd(tmp); /* anywhere up to that determ. by wt */
            if (farthest < stmp->range)
                farthest = stmp->range;
            stmp->stopped = FALSE;
            if (!schain)
                schain = stmp;
            else
                stmp2->next = stmp;
            stmp2 = stmp;
        }
    }

    while (farthest-- > 0) {
        for (stmp = schain; stmp; stmp = stmp->next) {
            if ((stmp->range-- > 0) && (!stmp->stopped)) {
                gt.thrownobj = stmp->obj; /* mainly in case it kills hero */
                gb.bhitpos.x = stmp->ox + stmp->dx;
                gb.bhitpos.y = stmp->oy + stmp->dy;
                if (isok(gb.bhitpos.x, gb.bhitpos.y))
                    typ = levl[gb.bhitpos.x][gb.bhitpos.y].typ;
                else
                    typ = STONE;
                if (!isok(gb.bhitpos.x, gb.bhitpos.y)) {
                    gb.bhitpos.x -= stmp->dx;
                    gb.bhitpos.y -= stmp->dy;
                    stmp->stopped = TRUE;
                } else if (!ZAP_POS(typ)
                           || closed_door(gb.bhitpos.x, gb.bhitpos.y)) {
                    gb.bhitpos.x -= stmp->dx;
                    gb.bhitpos.y -= stmp->dy;
                    stmp->stopped = TRUE;
                } else if ((mtmp = m_at(gb.bhitpos.x, gb.bhitpos.y)) != 0) {
                    if (scflags & MAY_HITMON) {
                        stmp->range--;
                        if (ohitmon(mtmp, stmp->obj, 1, FALSE)) {
                            stmp->obj = (struct obj *) 0;
                            stmp->stopped = TRUE;
                        }
                    }
                } else if (u_at(gb.bhitpos.x, gb.bhitpos.y)) {
                    if (scflags & MAY_HITYOU) {
                        int dam, hitvalu, hitu;

                        if (gm.multi)
                            nomul(0);
                        dam = dmgval(stmp->obj, &gy.youmonst);
                        hitvalu = 8 + stmp->obj->spe;
                        if (bigmonst(gy.youmonst.data))
                            hitvalu++;
                        hitu = thitu(hitvalu, Maybe_Half_Phys(dam),
                                     &stmp->obj, (char *) 0);
                        if (!stmp->obj)
                            stmp->stopped = TRUE;
                        if (hitu) {
                            stmp->range -= 3;
                            stop_occupation();
                        }
                    }
                } else {
                    if (scflags & VIS_EFFECTS) {
                        /* tmp_at(gb.bhitpos.x, gb.bhitpos.y); */
                        /* nh_delay_output(); */
                    }
                }
                stmp->ox = gb.bhitpos.x;
                stmp->oy = gb.bhitpos.y;
                if (IS_SINK(levl[stmp->ox][stmp->oy].typ))
                    stmp->stopped = TRUE;
                gt.thrownobj = (struct obj *) 0;
            }
        }
    }
    for (stmp = schain; stmp; stmp = stmp2) {
        coordxy x, y;
        boolean obj_left_shop = FALSE;

        stmp2 = stmp->next;
        x = stmp->ox;
        y = stmp->oy;
        if (stmp->obj) {
            if (x != sx || y != sy) {
                total += stmp->obj->quan;
                obj_left_shop = (shop_origin && !costly_spot(x, y));
            }
            if (!flooreffects(stmp->obj, x, y, "land")) {
                if (obj_left_shop
                    && strchr(u.urooms, *in_rooms(u.ux, u.uy, SHOPBASE))) {
                    /* At the moment this only takes on gold. While it is
                       simple enough to call addtobill for other items that
                       leave the shop due to scatter(), by default the hero
                       will get billed for the full shopkeeper asking-price
                       on the object's way out of shop. That can leave the
                       hero in a pickle. Even if the hero then manages to
                       retrieve the item and drop it back inside the shop,
                       the owed charges will only be reduced at that point
                       by the lesser shopkeeper buying-price.
                       The non-gold situation will likely get adjusted
                       further.
                     */
                    if (stmp->obj->otyp == GOLD_PIECE) {
                        addtobill(stmp->obj, FALSE, FALSE, TRUE);
                        lostgoods = TRUE;
                    }
                }
                place_object(stmp->obj, x, y);
                stackobj(stmp->obj);
            }
        }
        free((genericptr_t) stmp);
        newsym(x, y);
    }
    newsym(sx, sy);
    if (u_at(sx, sy) && u.uundetected && hides_under(gy.youmonst.data))
        (void) hideunder(&gy.youmonst);
    if (((mtmp = m_at(sx, sy)) != 0) && mtmp->mtrapped)
        mtmp->mtrapped = 0;
    maybe_unhide_at(sx, sy);
    if (lostgoods) /* implies shop_origin and therefore shkp valid */
        credit_report(shkp, 1, FALSE);
    return total;
}

/**
 * @brief Spread burning oil from a square to those around it.
 *
 * A fiery explosion with its damage rolled here rather than passed in, since the amount is a property of the oil rather than of whatever set it off.
 *
 * @param x the column
 * @param y the row
 * @param diluted_oil whether the oil was watered down, which reduces the damage
 * @note As the existing comment sets out, this ought to know how the oil was spilled and in which direction: a kicked flask should splatter differently from a thrown one landing. It does not, and produces an ordinary explosion in all cases -- so the comment records
 *       an intended improvement rather than a defect.
 * @note The blast kind is given as a bare number with the arithmetic that produces it written out beside it, described in the code as a fudge. The proper expression is not available here, so the number is spelled out along with how to check it.
 */
/**
 * @brief 불타는 기름을 한 칸에서 그 둘레로 퍼뜨린다.
 *
 * 피해가 넘겨지는 대신 여기서 굴려지는 불의 폭발이다. 그 양이 그것을 일으킨 무엇이든의 성질이 아니라 기름의 성질이기 때문이다.
 *
 * @param x 그 열
 * @param y 그 행
 * @param diluted_oil 그 기름이 물에 묶였는지. 피해를 줄인다
 * @note 기존 주석이 밝히듯 이것은 기름이 어떻게 쏟아졌고 어느 방향인지 알아야 한다. 차인 병은 던져져 내려앉는 병과 다르게 튀어야 한다. 그것은 그러지 않고 모든 경우에 보통의 폭발을 낸다. 그래서 그 주석은 결함이 아니라 의도된 개선을 기록한다.
 * @note 폭풍 종류가 그것을 내는 산술이 옆에 풀어 쓰인 맨 숫자로 주어지며, 코드에서 임시 조치라고 서술된다. 제대로 된 식을 여기서 쓸 수 없으므로, 그 숫자가 그것을 확인하는 방법과 함께 풀어 쓰여 있다.
 */
void
splatter_burning_oil(coordxy x, coordxy y, boolean diluted_oil)
{
    int dmg = d(diluted_oil ? 3 : 4, 4);

/* ZT_SPELL(ZT_FIRE) = ZT_SPELL(AD_FIRE-1) = 10+(2-1) = 11 */
#define ZT_SPELL_O_FIRE 11 /* value kludge, see zap.c */
    explode(x, y, ZT_SPELL_O_FIRE, dmg, BURNING_OIL, EXPL_FIERY);
}

/**
 * @brief A lit flask of oil explodes.
 *
 * The ordering here is the whole point, and the existing comment states it: the flask must stop being a light source before the explosion, because the explosion may kill the hero and a death triggers the writing of a bones file. A bones file recording a light source
 * that no longer exists would be a broken file, restored into a game where something is lighting an area from nowhere.
 *
 * @param obj the flask
 * @param x the column
 * @param y the row
 * @note Reads whether the oil was diluted before extinguishing it, since extinguishing may free or alter the object.
 * @note Records how the object was lost before the explosion rather than after, for the same reason as the extinguishing: afterwards may be too late.
 * @warning Complains if the flask was not lit. Unlit oil does not explode, so reaching here with one means a caller decided to explode something that cannot -- reported rather than silently producing a blast.
 */
/**
 * @brief 불붙은 기름병이 폭발한다.
 *
 * 여기서의 순서가 요점 전부이며, 기존 주석이 그것을 밝힌다. 그 병은 폭발 전에 광원이기를 그쳐야 한다. 폭발이 영웅을 죽일 수 있고, 죽음이 유골 파일 쓰기를 일으키기 때문이다. 더는 존재하지 않는 광원을 기록한 유골 파일은 망가진 파일일 것이며, 무언가가 아무것도 없는 데서 어떤 영역을 비추고 있는 게임으로 복원될 것이다.
 *
 * @param obj 그 병
 * @param x 그 열
 * @param y 그 행
 * @note 그것을 끄기 전에 기름이 물에 묶였는지 읽는다. 끄는 것이 그 물건을 해제하거나 바꿀 수 있기 때문이다.
 * @note 그 물건이 어떻게 잃어졌는지를 폭발 뒤가 아니라 앞에 기록한다. 끄기와 같은 이유다. 뒤는 너무 늦을 수 있다.
 * @warning 그 병이 불붙어 있지 않았으면 불평한다. 불붙지 않은 기름은 폭발하지 않으므로, 그런 것과 함께 여기 이르는 것은 호출자가 폭발할 수 없는 무엇을 폭발시키기로 정했다는 뜻이다. 조용히 폭풍을 내는 대신 보고된다.
 */
void
explode_oil(struct obj *obj, coordxy x, coordxy y)
{
    boolean diluted_oil = obj->odiluted;

    if (!obj->lamplit)
        impossible("exploding unlit oil");
    end_burn(obj, TRUE);
    obj->how_lost = LOST_EXPLODING;
    splatter_burning_oil(x, y, diluted_oil);
}

/**
 * @brief Choose which colour of blast to draw for a given damage type.
 *
 * The two are not the same thing and there are fewer colours than damage types, so this is a narrowing rather than a translation. Several damage types share one appearance.
 *
 * @param adtyp the damage type
 * @return the blast appearance
 * @note Electricity is drawn as a magical blast, and the existing comment concedes why: there is no electrical appearance, and magical is the closest available. So an electrical explosion looks like a magical one, which is a limitation of the tile set rather than a
 *       statement about the damage.
 * @note All the draining and disease types are drawn as noxious, grouping every damage that harms without burning or freezing under one appearance.
 * @note Physical damage maps to noxious as well, because the only physical explosion in the game is a gas spore's, as the comment beside it notes. That is a case rather than a rule, and a new physical explosion would inherit an appearance chosen for something else.
 * @warning An unrecognised damage type is reported and drawn as fiery. So a new damage type added without touching this produces a blast that looks like fire whatever it does.
 */
/**
 * @brief 주어진 피해 종류에 어느 색의 폭풍을 그릴지 고른다.
 *
 * 그 둘은 같은 것이 아니고 피해 종류보다 색이 적으므로, 이것은 번역이 아니라 좁히기다. 여러 피해 종류가 하나의 모습을 나눈다.
 *
 * @param adtyp 피해 종류
 * @return 폭풍의 모습
 * @note 전기가 마법 폭풍으로 그려지며, 기존 주석이 이유를 인정한다. 전기의 모습이 없고 마법이 쓸 수 있는 가장 가까운 것이다. 그래서 전기 폭발이 마법 폭발처럼 보이는데, 그것은 피해에 대한 진술이 아니라 타일 묶음의 한계다.
 * @note 모든 빨아냄과 질병 종류가 유해한 것으로 그려져, 태우거나 얼리지 않고 해치는 모든 피해를 하나의 모습 아래 묶는다.
 * @note 물리 피해도 유해한 것에 대응되는데, 옆의 주석이 적듯 게임에서 유일한 물리 폭발이 가스 홀씨의 것이기 때문이다. 그것은 규칙이 아니라 하나의 경우이며, 새 물리 폭발은 다른 것을 위해 골라진 모습을 물려받을 것이다.
 * @warning 인식되지 않는 피해 종류는 보고되고 불의 것으로 그려진다. 그래서 이것을 건드리지 않고 더해진 새 피해 종류는 자기가 무엇을 하든 불처럼 보이는 폭풍을 낸다.
 */
int
adtyp_to_expltype(const int adtyp)
{
    switch(adtyp) {
    case AD_ELEC:
        /* Electricity isn't magical, but there currently isn't an electric
         * explosion type. Magical is the next best thing. */
    case AD_SPEL:
    case AD_DREN:
    case AD_ENCH:
        return EXPL_MAGICAL;
    case AD_FIRE:
        return EXPL_FIERY;
    case AD_COLD:
        return EXPL_FROSTY;
    case AD_DRST:
    case AD_DRDX:
    case AD_DRCO:
    case AD_DISE:
    case AD_PEST:
    case AD_PHYS: /* gas spore */
        return EXPL_NOXIOUS;
    default:
        impossible("adtyp_to_expltype: bad explosion type %d", adtyp);
        return EXPL_FIERY;
    }
}

/**
 * @brief A monster destroys itself in a real explosion.
 *
 * For the monsters that genuinely blow up -- spheres, gas spores -- rather than those that merely produce a flash, as the existing comment distinguishes. Shared between the cases of a monster exploding at the hero and at another monster, so both behave identically.
 *
 * The monster is killed before the explosion rather than by it. That is deliberate and the existing comment gives the reason: otherwise it would appear to be caught in its own blast, which reads as a mistake.
 *
 * @param mon the exploding monster
 * @param mattk the attack describing the explosion's damage and type
 * @note Damage comes from the attack's dice, or from the monster's level when the attack gives only a die size, or is nothing when it gives neither. The middle case is how an explosion scales with a monster that has no fixed strength.
 * @note The killer's name is set before the explosion because the explosion may kill the hero, and because the explosion's own messages use it. It cannot be set afterwards on either count.
 * @note Checks whether the monster is already dead before killing it, and the existing comment explains when that happens: this is also reached from an explosion that goes off because the monster died. Killing an already-dead monster twice would be a second death.
 * @warning The blast kind is computed with arithmetic that presents the explosion as a monster's breath, which it is not -- the code says as much and calls it the closest analogue. The accompanying note observes that macros for this kind of conversion exist and are
 *          not used here, so this expression must be kept in step with them by hand.
 */
/**
 * @brief 몬스터가 진짜 폭발로 스스로를 파괴한다.
 *
 * 기존 주석이 구별하듯, 그저 섬광을 내는 것이 아니라 진짜로 터지는 몬스터들, 구체와 가스 홀씨를 위한 것이다. 몬스터가 영웅에게 폭발하는 경우와 다른 몬스터에게 폭발하는 경우 사이에 공유되므로, 둘이 똑같이 행동한다.
 *
 * 몬스터는 폭발에 의해서가 아니라 폭발 전에 죽는다. 그것은 의도된 것이며 기존 주석이 이유를 준다. 그러지 않으면 그것이 자기 폭풍에 휘말린 것처럼 보일 것이고, 그것은 실수로 읽힌다.
 *
 * @param mon 폭발하는 몬스터
 * @param mattk 그 폭발의 피해와 종류를 서술하는 공격
 * @note 피해는 그 공격의 주사위에서, 또는 공격이 주사위 크기만 줄 때는 몬스터의 단계에서 오거나, 어느 것도 주지 않을 때는 없음이다. 가운데 경우가 정해진 힘이 없는 몬스터와 함께 폭발이 커지는 방식이다.
 * @note 죽인 자의 이름이 폭발 전에 설정되는데, 그 폭발이 영웅을 죽일 수 있고 그 폭발 자신의 메시지가 그것을 쓰기 때문이다. 그 어느 셈으로도 뒤에 설정될 수 없다.
 * @note 죽이기 전에 그 몬스터가 이미 죽었는지 검사하며, 기존 주석이 그것이 언제 일어나는지 설명한다. 이것은 몬스터가 죽었기 때문에 일어나는 폭발에서도 닿아진다. 이미 죽은 몬스터를 두 번 죽이는 것은 두 번째 죽음일 것이다.
 * @warning 폭풍 종류가 그 폭발을 몬스터의 숨결로 내놓는 산술로 계산되는데, 그것은 그렇지 않다. 코드가 그렇게 말하며 그것을 가장 가까운 유사물이라고 부른다. 딸린 적바림이 이런 종류의 변환을 위한 매크로가 존재하며 여기서 쓰이지 않는다고 살피므로, 이 식은 손으로 그것들과 발을 맞춰 두어야 한다.
 */
void
mon_explodes(
    struct monst *mon,
    struct attack *mattk)
{
    int dmg;
    int type;
    if (mattk->damn) {
        dmg = d((int) mattk->damn, (int) mattk->damd);
    }
    else if (mattk->damd) {
        dmg = d((int) mon->data->mlevel + 1, (int) mattk->damd);
    }
    else {
        dmg = 0;
    }

    if (mattk->adtyp == AD_PHYS) {
        type = PHYS_EXPL_TYPE;
    }
    else if (mattk->adtyp >= AD_MAGM && mattk->adtyp <= AD_SPC2) {
        /* The -1, +20, *-1 math is to set it up as a 'monster breath' type
         * for the explosions (it isn't, but this is the closest analogue). */
        /* FIXME: there are macros for kind of thing... */
        type = -((mattk->adtyp - 1) + 20);
    }
    else {
        impossible("unknown type for mon_explode %d", mattk->adtyp);
        return;
    }

    /* Kill it now so it won't appear to be caught in its own explosion.
     * Must check to see if already dead - which happens if this is called
     * from an AT_BOOM attack upon death. */
    if (!DEADMONSTER(mon)) {
        mondead(mon);
    }

    /* This might end up killing you, too; you never know...
     * also, it is used in explode() messages */
    Sprintf(svk.killer.name, "%s explosion",
            s_suffix(pmname(mon->data, Mgender(mon))));
    svk.killer.format = KILLED_BY_AN;

    explode(mon->mx, mon->my, type, dmg, MON_EXPLODE,
            adtyp_to_expltype(mattk->adtyp));

    /* reset killer */
    svk.killer.name[0] = '\0';
}

/*explode.c*/
