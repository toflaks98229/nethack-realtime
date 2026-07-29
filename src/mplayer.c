/* NetHack 5.0	mplayer.c	$NHDT-Date: 1781973057 2026/06/20 16:30:57 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.54 $ */
/*      Copyright (c) Izchak Miller, 1992.                        */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file mplayer.c
 * @brief 몬스터-플레이어(mplayer) 생성 및 대사 처리.
 *
 * 엔드게임 등에서 등장하는 "몬스터 플레이어"(개발자 이름을 딴 적 캐릭터)를
 * 직업별 장비·능력치와 함께 생성하고, 이름 부여 및 전투 대사 출력을 담당한다.
 */

#include "hack.h"

staticfn const char *dev_name(void);
staticfn void get_mplname(struct monst *, char *);
staticfn void mk_mplayer_armor(struct monst *, short);

/** @brief NetHack 개발 기여자 이름 목록(mplayer 이름 부여에 사용). */
/* These are the names of those who
 * contributed to the development of NetHack 3.2/3.3/3.4/3.6.
 *
 * Keep in alphabetical order within teams.
 * Same first name is entered once within each team.
 */
static const char *const developers[] = {
    /* devteam */
    "Alex",    "Dave",   "Dean",    "Derek",   "Eric",    "Izchak",
    "Janet",   "Jessie", "Ken",     "Kevin",   "Michael", "Mike",
    "Pasi",    "Pat",    "Patric",  "Paul",    "Sean",    "Steve",
    "Timo",    "Warwick",
    /* PC team */
    "Bill",    "Eric",   "Keizo",   "Ken",    "Kevin",    "Michael",
    "Mike",    "Paul",   "Stephen", "Steve",  "Timo",     "Yitzhak",
    /* Amiga team */
    "Andy",    "Gregg",  "Janne",   "Keni",   "Mike",     "Olaf",
    "Richard",
    /* Mac team */
    "Andy",    "Chris",  "Dean",    "Jon",    "Jonathan", "Kevin",
    "Wang",
    /* Atari team */
    "Eric",    "Marvin", "Warwick",
    /* NT team */
    "Alex",    "Dion",   "Michael",
    /* OS/2 team */
    "Helge",   "Ron",    "Timo",
    /* VMS team */
    "Joshua",  "Pat",    ""
};

/**
 * @brief 몬스터-플레이어를 생성한다.
 *
 * 직업(role)별로 무기·방어구·기타 아이템과 능력치를 무작위로 부여한다.
 * @p special 이면 엔드게임용으로 더 강하게 만들고 개발자 이름과 가짜
 * 옌더의 부적을 부여한다.
 *
 * @param[in] ptr     생성할 몬스터-플레이어 종 데이터.
 * @param[in] x       생성 위치 x 좌표.
 * @param[in] y       생성 위치 y 좌표.
 * @param[in] special TRUE 이면 엔드게임용 강화 버전으로 생성.
 * @return 생성된 몬스터 포인터, mplayer 종이 아니면 NULL.
 */
struct monst *
mk_mplayer(struct permonst *ptr, coordxy x, coordxy y, boolean special)
{
    struct monst *mtmp;
    char nam[PL_NSIZ];

    if (!is_mplayer(ptr))
        return ((struct monst *) 0);

    if (MON_AT(x, y))
        (void) rloc(m_at(x, y), RLOC_ERR|RLOC_NOMSG); /* insurance */

    if (!In_endgame(&u.uz))
        special = FALSE;

    if ((mtmp = makemon(ptr, x, y, special ? MM_NOMSG : NO_MM_FLAGS)) != 0) {
        short weapon, armor, cloak, helm, shield;
        int quan;
        struct obj *otmp;

        mtmp->m_lev = (special ? rn1(16, 15) : rnd(16));
        mtmp->mhp = mtmp->mhpmax = d((int) mtmp->m_lev, 10)
                                   + (special ? (30 + rnd(30)) : 30);
        if (special) {
            get_mplname(mtmp, nam);
            mtmp = christen_monst(mtmp, nam);
            /* that's why they are "stuck" in the endgame :-) */
            (void) mongets(mtmp, FAKE_AMULET_OF_YENDOR);
        }
        mtmp->mpeaceful = 0;
        set_malign(mtmp); /* peaceful may have changed again */

        /* default equipment; much of it will be overridden below */
        weapon = !rn2(2) ? LONG_SWORD : rnd_class(SPEAR, BULLWHIP);
        armor  = rnd_class(GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL);
        cloak  = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(OILSKIN_CLOAK, CLOAK_OF_DISPLACEMENT);
        helm   = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY);
        shield = !rn2(8) ? STRANGE_OBJECT
                         : rnd_class(ELVEN_SHIELD, SHIELD_OF_REFLECTION);

        switch (monsndx(ptr)) {
        case PM_ARCHEOLOGIST:
            if (rn2(2))
                weapon = BULLWHIP;
            break;
        case PM_BARBARIAN:
            if (rn2(2)) {
                weapon = rn2(2) ? TWO_HANDED_SWORD : BATTLE_AXE;
                shield = STRANGE_OBJECT;
            }
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            if (helm == HELM_OF_BRILLIANCE)
                helm = STRANGE_OBJECT;
            break;
        case PM_CAVE_DWELLER:
            if (rn2(4))
                weapon = MACE;
            else if (rn2(2))
                weapon = CLUB;
            if (helm == HELM_OF_BRILLIANCE)
                helm = STRANGE_OBJECT;
            break;
        case PM_HEALER:
            if (rn2(4))
                weapon = QUARTERSTAFF;
            else if (rn2(2))
                weapon = rn2(2) ? UNICORN_HORN : SCALPEL;
            if (rn2(4))
                helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_KNIGHT:
            if (rn2(4))
                weapon = LONG_SWORD;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            break;
        case PM_MONK:
            weapon = !rn2(3) ? SHURIKEN : STRANGE_OBJECT;
            armor = STRANGE_OBJECT;
            cloak = ROBE;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_CLERIC:
            if (rn2(2))
                weapon = MACE;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            if (rn2(4))
                cloak = ROBE;
            if (rn2(4))
                helm = rn2(2) ? HELM_OF_BRILLIANCE : HELM_OF_TELEPATHY;
            if (rn2(2))
                shield = STRANGE_OBJECT;
            break;
        case PM_RANGER:
            if (rn2(2))
                weapon = ELVEN_DAGGER;
            break;
        case PM_ROGUE:
            if (rn2(2))
                weapon = rn2(2) ? SHORT_SWORD : ORCISH_DAGGER;
            break;
        case PM_SAMURAI:
            if (rn2(2))
                weapon = KATANA;
            break;
        case PM_TOURIST:
            /* Defaults are just fine */
            break;
        case PM_VALKYRIE:
            if (rn2(2))
                weapon = WAR_HAMMER;
            if (rn2(2))
                armor = rnd_class(PLATE_MAIL, CHAIN_MAIL);
            break;
        case PM_WIZARD:
            if (rn2(4))
                weapon = rn2(2) ? QUARTERSTAFF : ATHAME;
            if (rn2(2)) {
                armor = rn2(2) ? BLACK_DRAGON_SCALE_MAIL
                               : SILVER_DRAGON_SCALE_MAIL;
                cloak = CLOAK_OF_MAGIC_RESISTANCE;
            }
            if (rn2(4))
                helm = HELM_OF_BRILLIANCE;
            shield = STRANGE_OBJECT;
            break;
        default:
            impossible("bad mplayer monster");
            weapon = 0;
            break;
        }

        if (weapon != STRANGE_OBJECT) {
            otmp = mksobj(weapon, TRUE, FALSE);
            otmp->oeroded = otmp->oeroded2 = 0;
            otmp->spe = (special ? rn1(5, 4) : rn2(4));
            if (!rn2(3))
                otmp->oerodeproof = 1;
            else if (!rn2(2))
                otmp->greased = 1;
            /* mk_artifact() with otmp and A_NONE will never return NULL */
            if (special && rn2(2))
                otmp = mk_artifact(otmp, A_NONE, 99, FALSE);
            /* usually increase stack size if stackable weapon */
            if (objects[otmp->otyp].oc_merge && !otmp->oartifact
                && monmightthrowwep(otmp))
                otmp->quan += (long) rn2(is_spear(otmp) ? 4 : 8);
            otmp->owt = weight(otmp);
            /* mplayers knew better than to overenchant Magicbane */
            if (is_art(otmp, ART_MAGICBANE))
                otmp->spe = rnd(4);
            (void) mpickobj(mtmp, otmp);
        }

        if (special) {
            if (!rn2(10))
                (void) mongets(mtmp, rn2(3) ? LUCKSTONE : LOADSTONE);
            mk_mplayer_armor(mtmp, armor);
            mk_mplayer_armor(mtmp, cloak);
            mk_mplayer_armor(mtmp, helm);
            mk_mplayer_armor(mtmp, shield);
            if (weapon == WAR_HAMMER) /* valkyrie: wimpy weapon or Mjollnir */
                mk_mplayer_armor(mtmp, GAUNTLETS_OF_POWER);
            else if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(LEATHER_GLOVES,
                                                 GAUNTLETS_OF_DEXTERITY));
            if (rn2(8))
                mk_mplayer_armor(mtmp, rnd_class(LOW_BOOTS,
                                                 LEVITATION_BOOTS));
            m_dowear(mtmp, TRUE);

            quan = rn2(3) ? rn2(3) : rn2(16);
            while (quan--)
                (void) mongets(mtmp, rnd_class(DILITHIUM_CRYSTAL, JADE));
            /* To get the gold "right" would mean a player can double his
               gold supply by killing one mplayer.  Not good. */
            mkmonmoney(mtmp, rn2(1000));
            quan = rn2(10);
            while (quan--)
                (void) mpickobj(mtmp, mkobj(RANDOM_CLASS, FALSE));
        }
        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_offensive_item(mtmp));
        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_defensive_item(mtmp));
        quan = rnd(3);
        while (quan--)
            (void) mongets(mtmp, rnd_misc_item(mtmp));
    }

    return (mtmp);
}

/**
 * @brief 지정한 수만큼의 몬스터-플레이어를 무작위 위치에 생성한다.
 *
 * @param[in] num     생성할 몬스터-플레이어 수.
 * @param[in] special TRUE 이면 엔드게임용 강화 버전으로 생성.
 * @note @p special 인 경우 @p num 은 @c developers 배열의 중복 없는 이름 수를
 *       넘지 않아야 하며, 넘으면 "Adam/Eve" 이름이 채워진다.
 */
/* create the indicated number (num) of monster-players,
 * randomly chosen, and in randomly chosen (free) locations
 * on the level.  If "special", the size of num should not
 * be bigger than the number of _non-repeated_ names in the
 * developers array, otherwise a bunch of Adams and Eves will
 * fill up the overflow.
 */
void
create_mplayers(int num, boolean special)
{
    int pm, x, y;
    struct monst fakemon;

    fakemon = cg.zeromonst;
    while (num) {
        int tryct = 0;

        /* roll for character class */
        pm = rn1(PM_WIZARD - PM_ARCHEOLOGIST + 1, PM_ARCHEOLOGIST);
        set_mon_data(&fakemon, &mons[pm]);

        /* roll for an available location */
        do {
            x = rn1(COLNO - 4, 2);
            y = rnd(ROWNO - 2);
        } while (!goodpos(x, y, &fakemon, 0) && tryct++ <= 50);

        /* if pos not found in 50 tries, don't bother to continue */
        if (tryct > 50)
            return;

        (void) mk_mplayer(&mons[pm], (coordxy) x, (coordxy) y, special);
        num--;
    }
}

/**
 * @brief 몬스터-플레이어의 전투 대사를 출력한다.
 * @param[in] mtmp 대사를 말할 몬스터-플레이어.
 * @note 평화적(mpeaceful) 상태이면 대사 없이 일반 인간형 대화로 넘어간다.
 *       영웅과 같은 직업이면 다른 대사를 사용한다.
 */
void
mplayer_talk(struct monst *mtmp)
{
    static const char
        *same_class_msg[3] = {
            "I can't win, and neither will you!",
            "You don't deserve to win!",
            "Mine should be the honor, not yours!",
        },
        *other_class_msg[3] = {
            "The low-life wants to talk, eh?",
            "Fight, scum!",
            "Here is what I have to say!",
        };

    if (mtmp->mpeaceful)
        return; /* will drop to humanoid talk */

    SetVoice(mtmp, 0, 80, 0);
    verbalize("Talk? -- %s", mtmp->data == &mons[gu.urole.mnum]
                                ? same_class_msg[rn2(3)]
                                : other_class_msg[rn2(3)]);
}

/**
 * @brief 아직 사용되지 않은 개발자 이름을 무작위로 하나 반환한다.
 * @return 선택된 개발자 이름 문자열.
 * @retval NULL 시도 한계 내에서 미사용 이름을 찾지 못한 경우.
 * @note 이미 같은 이름의 mplayer 가 레벨에 존재하면 중복을 피한다.
 */
/* return a randomly chosen developer name */
staticfn const char *
dev_name(void)
{
    int i, m = 0, n = SIZE(developers);
    struct monst *mtmp;
    boolean match;

    do {
        match = FALSE;
        i = rn2(n);
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (!is_mplayer(mtmp->data))
                continue;
            if (!strncmp(developers[i],
                         (has_mgivenname(mtmp)) ? MGIVENNAME(mtmp) : "",
                         strlen(developers[i]))) {
                match = TRUE;
                break;
            }
        }
        m++;
    } while (match && m < 100); /* m for insurance */

    if (match)
        return (const char *) 0;
    return (developers[i]);
}

/**
 * @brief 몬스터-플레이어의 이름("<이름> the <계급>")을 생성하고 성별을 정한다.
 * @param[in]  mtmp 이름을 부여할 몬스터-플레이어.
 * @param[out] nam  생성된 이름을 저장할 버퍼.
 */
staticfn void
get_mplname(struct monst *mtmp, char *nam)
{
    boolean fmlkind = is_female(mtmp->data);
    const char *devnam;

    devnam = dev_name();
    if (!devnam)
        Strcpy(nam, fmlkind ? "Eve" : "Adam");
    else if (fmlkind && !!strcmp(devnam, "Janet"))
        Strcpy(nam, rn2(2) ? "Maud" : "Eve");
    else
        Strcpy(nam, devnam);

    if (fmlkind || !strcmp(nam, "Janet"))
        mtmp->female = 1;
    else
        mtmp->female = 0;
    Strcat(nam, " the ");
    Strcat(nam, rank_of((int) mtmp->m_lev, monsndx(mtmp->data),
                        (boolean) mtmp->female));
}

/**
 * @brief 몬스터-플레이어용 방어구 아이템 하나를 생성해 지급한다.
 * @param[in,out] mon 아이템을 받을 몬스터.
 * @param[in]     typ 생성할 방어구 종류(@c STRANGE_OBJECT 이면 아무것도 안 함).
 * @note 부식 방지/축복/저주/강화 수치가 무작위로 부여된다.
 */
staticfn void
mk_mplayer_armor(struct monst *mon, short typ)
{
    struct obj *obj;

    if (typ == STRANGE_OBJECT)
        return;
    obj = mksobj(typ, FALSE, FALSE);
    obj->oeroded = obj->oeroded2 = 0;
    if (!rn2(3))
        obj->oerodeproof = 1;
    if (!rn2(3))
        curse(obj);
    if (!rn2(3))
        bless(obj);
    /* Most players who get to the endgame who have cursed equipment
     * have it because the wizard or other monsters cursed it, so its
     * chances of having plusses is the same as usual....
     */
    obj->spe = rn2(10) ? (rn2(3) ? rn2(5) : rn1(4, 4)) : -rnd(3);
    (void) mpickobj(mon, obj);
}

/*mplayer.c*/
