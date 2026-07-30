/* NetHack 5.0	bones.c	$NHDT-Date: 1781973041 2026/06/20 16:30:41 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.159 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985,1993. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file bones.c
 * @brief Leaving a level behind for the next game, and taking one over.
 *
 * When a hero dies, the level may be preserved so that a later game finds it:
 * the corpse, the possessions, and a ghost. This is the only way one game's
 * state reaches another, which is what makes it delicate.
 *
 * Loading is therefore an act of sanitation as much as of restoration. The old
 * hero's belongings must be stripped of anything that would leak knowledge or
 * advantage into the new game -- identification, custom names, artifacts that
 * already exist here -- and the ghost must become an ordinary monster of this
 * level rather than a saved hero.
 *
 * @warning A bones file may have been written by a different version, edited,
 *          or shared between players. Nothing read from it can be trusted the
 *          way the current game's own save can.
 */

/**
 * @file bones.c
 * @brief 다음 게임을 위해 레벨을 남기는 일과, 남겨진 레벨을 이어받는 일.
 *
 * 영웅이 죽으면 그 레벨이 보존되어 나중의 게임이 발견할 수 있다. 시체와 소지품,
 * 그리고 유령이다. 한 게임의 상태가 다른 게임에 닿는 유일한 경로이며, 그래서
 * 다루기가 까다롭다.
 *
 * 따라서 불러오기는 복원인 만큼이나 정화 작업이다. 예전 영웅의 소지품에서 지식이나
 * 이점이 새 게임으로 새어 나갈 만한 것 -- 감정 여부, 붙인 이름, 이 게임에 이미
 * 존재하는 아티팩트 -- 을 모두 걷어내야 하고, 유령은 저장된 영웅이 아니라 이
 * 레벨의 평범한 몬스터가 되어야 한다.
 *
 * @warning 본즈 파일은 다른 버전이 썼거나, 편집되었거나, 플레이어들 사이에
 *          공유되었을 수 있다. 거기서 읽은 어떤 것도 현재 게임 자신의 저장만큼
 *          신뢰할 수 없다.
 */

#include "hack.h"

#ifndef SFCTOOL
staticfn boolean no_bones_level(d_level *);
staticfn void goodfruit(int);
staticfn void resetobjs(struct obj *, boolean);
staticfn void give_to_nearby_mon(struct obj *, coordxy, coordxy) NONNULLARG1;
staticfn boolean fixuporacle(struct monst *) NONNULLARG1;
staticfn void remove_mon_from_bones(struct monst *) NONNULLARG1;
staticfn void set_ghostly_objlist(struct obj *objchain);

/**
 * @brief Whether a level may never be left as bones.
 *
 * Several levels are excluded, and the reasons differ. A special level built from a description would be wrong to overwrite with someone else's version of it. A branch level or the bottom of a dungeon is where the game's
 * structure is, and a stale copy would misdirect a later game. The invocation level is excluded because what is on it must be exactly what the game put there.
 *
 * @param lev the level, which this may reassign to the level being saved
 * @return true if bones must not be left here
 * @note Level one of a dungeon is exempt from the branch exclusion, as the accompanying comment records, because it is the entrance rather than a fork.
 * @warning It may overwrite the level it was given with the one currently being saved. So it is not a pure question about an arbitrary level, and asking it about somewhere else while a save is in progress does not do what it
 *          looks like.
 */
/**
 * @brief 어떤 레벨이 유골로 남겨져서는 결코 안 되는지.
 *
 * 여러 레벨이 제외되며 그 이유가 서로 다르다. 기술로 지어진 특수 레벨은 다른 사람의 판본으로 덮어쓰는 것이 잘못이다. 분기 레벨이나 던전의 맨 아래는 게임의 구조가 있는 곳이고, 낡은 사본이 나중의 게임을 잘못 이끌게 된다. 발동 레벨이 제외되는 것은 그곳에 있는 것이 게임이 둔 것과 정확히 같아야
 * 하기 때문이다.
 *
 * @param lev 그 레벨. 이것이 저장되고 있는 레벨로 다시 배정할 수 있다
 * @return 여기에 유골을 남겨서는 안 되면 참
 * @note 딸린 주석이 기록하듯 던전의 1층은 분기 제외에서 면제된다. 갈림길이 아니라 입구이기 때문이다.
 * @warning 건네받은 레벨을 지금 저장되고 있는 레벨로 덮어쓸 수 있다. 그래서 임의의 레벨에 대한 순수한 질문이 아니며, 저장이 진행 중인 동안 다른 곳에 대해 묻는 것은 보이는 대로 동작하지 않는다.
 */
staticfn boolean
no_bones_level(d_level *lev)
{
    s_level *sptr;

    if (ledger_no(&gs.save_dlevel))
        assign_level(lev, &gs.save_dlevel);

    return (boolean) (((sptr = Is_special(lev)) != 0 && !sptr->boneid)
                      || !svd.dungeons[lev->dnum].boneid
                      /* no bones on the last or multiway branch levels
                         in any dungeon (level 1 isn't multiway) */
                      || Is_botlevel(lev)
                      || (Is_branchlev(lev) && lev->dlevel > 1)
                      /* no bones in the invocation level */
                      || (In_hell(lev)
                          && lev->dlevel == dunlevs_in_dungeon(lev) - 1));
}

/**
 * @brief Mark a player-named fruit as one that must be saved with the level.
 *
 * A game accumulates fruit names, and most of them have no fruit left in the world. Saving all of them would put a list of everything the player ever typed into a file another player will read, so only the names still attached
 * to something are kept.
 *
 * This is how the ones still in use are found: every fruit object on the level being saved marks its own name, and the names left unmarked are the ones to omit.
 *
 * @param id the fruit's identifier, negated
 * @note The marking is done by the sign of the identifier, as the accompanying comment explains -- a negative one means unmarked. So this is not setting a flag but restoring a value to its positive form.
 * @warning The argument arrives negated because that is how an unmarked name is stored. Passing an already-positive identifier marks nothing and looks like it worked.
 */
/**
 * @brief 플레이어가 이름 붙인 과일을 레벨과 함께 저장해야 하는 것으로 표시한다.
 *
 * 게임은 과일 이름을 쌓아 가고, 그 대부분에는 세계에 남은 과일이 없다. 그것들을 다 저장하는 것은 플레이어가 입력한 모든 것의 목록을 다른 플레이어가 읽을 파일에 넣는 일이므로, 여전히 무언가에 붙어 있는 이름만이 보관된다.
 *
 * 이것이 여전히 쓰이는 것을 찾는 방식이다. 저장되고 있는 레벨의 모든 과일 객체가 자기 이름을 표시하며, 표시되지 않은 채로 남은 이름이 빼놓을 것이다.
 *
 * @param id 그 과일의 식별자. 음수화된 것
 * @note 딸린 주석이 설명하듯 그 표시는 식별자의 부호로 이뤄진다. 음수는 표시되지 않았음을 뜻한다. 그래서 이것은 플래그를 세우는 것이 아니라 값을 양수 형태로 되돌리는 것이다.
 * @warning 인자가 음수화된 채로 도착하는 것은 표시되지 않은 이름이 그렇게 저장되기 때문이다. 이미 양수인 식별자를 넘기면 아무것도 표시되지 않으면서 작동한 것처럼 보인다.
 */
/* Call this function for each fruit object saved in the bones level: it marks
 * that particular type of fruit as existing (the marker is that that type's
 * ID is positive instead of negative).  This way, when we later save the
 * chain of fruit types, we know to only save the types that exist.
 */
staticfn void
goodfruit(int id)
{
    struct fruit *f = fruit_from_indx(-id);

    if (f)
        f->fid = id;
}

/**
 * @brief Strip a chain of objects of everything that must not cross between games.
 *
 * The sanitation the file header describes, and the most consequential routine here. One routine does both directions because the two are not symmetrical and it is easier to see that they correspond when they sit together.
 *
 * Saving removes knowledge. Whether the old hero knew what something was, whether they knew it was cursed, what letter it had in their pack, what they had chosen to call it -- all of that is theirs and would be an unearned gift
 * to whoever finds it. So the object is reduced to what it is, with nothing about what was known of it.
 *
 * Restoring resolves conflicts. An artifact in the file may already exist in this game, and two of one artifact cannot both be real -- so the incoming one becomes an ordinary object of its kind. A quest artifact from someone
 * else's quest is refused the same way.
 *
 * @param ochain the objects, including anything inside them
 * @param restore false while saving, true while loading
 * @note Recurses into containers, so an object hidden in a bag is not exempt.
 * @note An object caught mid-use is destroyed rather than saved. Its state describes an action that will never finish, and there is nothing sensible for it to become.
 * @warning Some names survive saving, and the accompanying comment explains the difficulty: a statue's or corpse's name may have come from a real player's score record rather than from the dead hero, and there is currently no
 *          way to tell which -- so those are kept and the comment records what would be needed to do better.
 * @warning A user-supplied name that does survive is sanitised rather than trusted, because a bones file may have been edited.
 */
/**
 * @brief 물건 사슬에서 게임 사이를 건너서는 안 되는 모든 것을 벗겨 낸다.
 *
 * 파일 머리말이 기술하는 정화이며, 여기서 결과가 가장 큰 루틴이다. 하나의 루틴이 두 방향을 함께 맡는 것은 그 둘이 대칭이 아니고, 함께 놓여 있을 때 그것들이 서로 대응한다는 점이 더 잘 보이기 때문이다.
 *
 * 저장은 앎을 없앤다. 예전 영웅이 무언가가 무엇인지 알았는지, 저주받았음을 알았는지, 자기 가방에서 어느 글자였는지, 무엇이라 부르기로 했는지. 그 전부가 그의 것이며 그것을 발견한 이에게 벌지 않은 선물이 된다. 그래서 그 객체는 그것이 무엇인지로 줄어들고, 그것에 대해 무엇이 알려졌는지는
 * 아무것도 남지 않는다.
 *
 * 복원은 충돌을 해결한다. 파일 안의 아티팩트가 이 게임에 이미 존재할 수 있고, 하나의 아티팩트가 둘 다 진짜일 수는 없다. 그래서 들어오는 쪽이 그 종류의 평범한 객체가 된다. 다른 사람의 퀘스트에서 온 퀘스트 아티팩트도 같은 방식으로 거부된다.
 *
 * @param ochain 그 물건들. 그 안에 있는 것을 포함해서
 * @param restore 저장 중에는 거짓, 적재 중에는 참
 * @note 용기 안으로 재귀하므로, 가방에 숨은 물건이 면제되지 않는다.
 * @note 쓰는 도중에 걸린 물건은 저장되는 대신 파괴된다. 그 상태는 결코 끝나지 않을 행동을 기술하며, 그것이 무엇이 될 만한 합당한 것이 없다.
 * @warning 어떤 이름들은 저장을 견디며, 딸린 주석이 그 어려움을 설명한다. 조각상이나 시체의 이름은 죽은 영웅이 아니라 실제 플레이어의 점수 기록에서 왔을 수 있고, 현재 어느 쪽인지 알 방법이 없다. 그래서 그것들은 보관되며, 그 주석이 더 잘하려면 무엇이 필요한지를 기록하고 있다.
 * @warning 살아남은 사용자 제공 이름은 믿기는 대신 정화된다. 유골 파일이 편집되었을 수 있기 때문이다.
 */
staticfn void
resetobjs(struct obj *ochain, boolean restore)
{
    struct obj *otmp, *nobj;

    for (otmp = ochain; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->cobj)
            resetobjs(otmp->cobj, restore);
        if (otmp->in_use) {
            obj_extract_self(otmp);
            dealloc_obj(otmp);
            continue;
        }

        if (restore) {
            /* artifact bookkeeping needs to be done during
               restore; other fixups are done while saving */
            if (otmp->oartifact) {
                if (exist_artifact(otmp->otyp, safe_oname(otmp))
                    || is_quest_artifact(otmp)) {
                    /* prevent duplicate--revert to ordinary obj */
                    otmp->oartifact = 0;
                    if (has_oname(otmp))
                        free_oname(otmp);
                } else {
                    artifact_exists(otmp, safe_oname(otmp), TRUE,
                                    ONAME_BONES);
                }
            } else if (has_oname(otmp)) {
                sanitize_name(ONAME(otmp));
            }
            /* 3.6.3: set no_charge for partly eaten food in shop;
               all other items become goods for sale if in a shop */
            if (otmp->oclass == FOOD_CLASS && otmp->oeaten) {
                struct obj *top;
                char *p;
                coordxy ox, oy;

                for (top = otmp; top->where == OBJ_CONTAINED;
                     top = top->ocontainer)
                    continue;
                otmp->no_charge = (top->where == OBJ_FLOOR
                                   && get_obj_location(top, &ox, &oy, 0)
                                   /* can't use costly_spot() since its
                                      result depends upon hero's location */
                                   && inside_shop(ox, oy)
                                   && *(p = in_rooms(ox, oy, SHOPBASE))
                                 && tended_shop(&svr.rooms[*p - ROOMOFFSET]));
            }
        } else { /* saving */
            /* do not zero out o_ids for ghost levels anymore */

            if (objects[otmp->otyp].oc_uses_known)
                otmp->known = 0;
            otmp->dknown = otmp->bknown = 0;
            otmp->rknown = 0;
            otmp->lknown = 0;
            otmp->cknown = 0;
            otmp->tknown = 0;
            otmp->invlet = 0;
            otmp->no_charge = 0;
            otmp->how_lost = LOST_NONE;

            /* strip user-supplied names */
            /* Statue and some corpse names are left intact,
               presumably in case they came from score file.
               [TODO: this ought to be done differently--names
               which came from such a source or came from any
               stoned or killed monster should be flagged in
               some manner; then we could just check the flag
               here and keep "real" names (dead pets, &c) while
               discarding player notes attached to statues.] */
            if (has_oname(otmp)
                && !(otmp->oartifact || otmp->otyp == STATUE
                     || otmp->otyp == SPE_NOVEL
                     || (otmp->otyp == CORPSE
                         && otmp->corpsenm >= SPECIAL_PM))) {
                free_oname(otmp);
            }

            if (otmp->otyp == SLIME_MOLD) {
                goodfruit(otmp->spe);
#ifdef MAIL_STRUCTURES
            } else if (otmp->otyp == SCR_MAIL) {
                /* 0: delivered in-game via external event;
                   1: from bones or wishing; 2: written with marker */
                if (otmp->spe == 0)
                    otmp->spe = 1;
#endif
            } else if (otmp->otyp == EGG) {
                otmp->spe = 0; /* not "laid by you" in next game */
            } else if (otmp->otyp == TIN) {
                /* make tins of unique monster's meat be empty */
                if (ismnum(otmp->corpsenm)
                    && unique_corpstat(&mons[otmp->corpsenm]))
                    otmp->corpsenm = NON_PM;
            } else if (otmp->otyp == CORPSE || otmp->otyp == STATUE) {
                int mnum = otmp->corpsenm;

                /* Discard incarnation details of unique monsters
                   (by passing null instead of otmp for object),
                   shopkeepers (by passing false for revival flag),
                   temple priests, and vault guards in order to
                   prevent corpse revival or statue reanimation. */
                if (has_omonst(otmp)
                    && cant_revive(&mnum, FALSE, (struct obj *) 0)) {
                    free_omonst(otmp);
                    /* mnum is now either human_zombie or doppelganger;
                       for corpses of uniques, we need to force the
                       transformation now rather than wait until a
                       revival attempt, otherwise eating this corpse
                       would behave as if it remains unique */
                    if (mnum == PM_DOPPELGANGER && otmp->otyp == CORPSE)
                        set_corpsenm(otmp, mnum);
                }
            } else if (is_mines_prize(otmp) || is_soko_prize(otmp)) {
                /* achievement tracking; in case prize was moved off its
                   original level (which is always a no-bones level) */
                otmp->nomerge = 0;
            } else if (otmp->otyp == AMULET_OF_YENDOR) {
                /* no longer the real Amulet */
                otmp->otyp = FAKE_AMULET_OF_YENDOR;
                curse(otmp);
            } else if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
                if (otmp->lamplit)
                    end_burn(otmp, TRUE);
                otmp->otyp = WAX_CANDLE;
                otmp->age = 50L; /* assume used */
                if (otmp->spe > 0)
                    otmp->quan = (long) otmp->spe;
                otmp->spe = 0;
                otmp->owt = weight(otmp);
                curse(otmp);
            } else if (otmp->otyp == BELL_OF_OPENING) {
                otmp->otyp = BELL;
                curse(otmp);
            } else if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
                otmp->otyp = SPE_BLANK_PAPER;
                curse(otmp);
            }
        }
    }
}

/**
 * @brief Make text from a bones file safe to show on this player's display.
 *
 * The only defence against text another player wrote reaching this player's terminal. A name may contain characters that move the cursor, clear the screen, or otherwise disrupt the display -- accidentally or not -- and a
 * bones file is the one place such text arrives from outside the game.
 *
 * @param namebuf the text, altered in place
 * @note Deliberately not skipped on a single-player installation, and the accompanying comment gives the reason: bones files are exchanged between players by various means, so "only I could have written this" is not a safe
 *       assumption even where only one person plays.
 * @note What is done depends on the display and the player's own setting. A terminal that was told to accept high-bit characters keeps them; one that was not has them replaced, since on such a terminal they may not be
 *       characters at all.
 * @warning Replaces rather than rejects. A name that was disruptive becomes a name full of substitute characters, so the object keeps a name and the player sees that something was amiss with it.
 */
/**
 * @brief 유골 파일에서 온 글을 이 플레이어의 표시부에 보여도 안전하게 만든다.
 *
 * 다른 플레이어가 쓴 글이 이 플레이어의 터미널에 닿는 것에 대한 유일한 방어다. 이름에는 커서를 옮기거나 화면을 지우거나 그 밖의 방식으로 표시부를 흐트러뜨리는 문자가 들어 있을 수 있고 -- 우연이든 아니든 -- 유골 파일이 그런 글이 게임 밖에서 도착하는 유일한 곳이다.
 *
 * @param namebuf 그 글. 제자리에서 바뀐다
 * @note 1인용 설치본에서도 의도적으로 건너뛰지 않으며, 딸린 주석이 그 이유를 밝힌다. 유골 파일은 여러 수단으로 플레이어 사이에 교환되므로, "나만이 이것을 썼을 수 있다"는 것은 한 사람만 플레이하는 곳에서도 안전한 전제가 아니다.
 * @note 무엇을 하는지는 표시부와 플레이어 자신의 설정에 달려 있다. 상위 비트 문자를 받아들이라고 들은 터미널은 그것을 지키고, 그러지 않은 터미널은 그것이 갈아치워진다. 그런 터미널에서는 그것이 아예 문자가 아닐 수 있기 때문이다.
 * @warning 거부하는 것이 아니라 갈아치운다. 흐트러뜨리던 이름이 대체 문자로 가득한 이름이 되므로, 그 객체는 이름을 지키고 플레이어는 그것에 무언가 문제가 있었음을 본다.
 */
/* while loading bones, strip out text possibly supplied by old player
   that might accidentally or maliciously disrupt new player's display */
void
sanitize_name(char *namebuf)
{
    int c;
    boolean strip_8th_bit = (WINDOWPORT(tty)
                             && !iflags.wc_eight_bit_input);

    /* it's tempting to skip this for single-user platforms, since
       only the current player could have left these bones--except
       things like "hearse" and other bones exchange schemes make
       that assumption false */
    while (*namebuf) {
        c = *namebuf & 0177;
        if (c < ' ' || c == '\177') {
            /* non-printable or undesirable */
            *namebuf = '.';
        } else if (c != *namebuf) {
            /* expected to be printable if user wants such things */
            if (strip_8th_bit)
                *namebuf = '_';
        }
        ++namebuf;
    }
}

/* Give object to a random object-liking monster on or adjacent to x,y
   but skipping hero's location.
   If no such monster, place object on floor at x,y. */
staticfn void
/**
 * @brief Let a nearby monster take one of the dead hero's possessions, or leave it on the floor.
 *
 * So that a hero's belongings are not always found in a neat pile where they fell. A monster that collects things and was standing nearby when the hero died plausibly picked something up, and this is that.
 *
 * @param otmp the object
 * @param x,y where the hero died
 * @note One of the eligible monsters is chosen with equal probability without knowing in advance how many there are, by giving each a diminishing chance as it is met. That is why the count is kept as the search proceeds.
 * @note Deliberately does not check that the object is the sort the monster likes, as the accompanying comment states: the monster is imagined to be rifling through what is there and taking whatever looks interesting, which
 *       is a better story than a monster that only ever takes exactly what its species covets.
 * @note Falls back to the floor if no monster will have it, so the object is never lost.
 */
/**
 * @brief 근처 몬스터가 죽은 영웅의 소지품 하나를 가져가게 하거나, 바닥에 남긴다.
 *
 * 영웅의 소지품이 쓰러진 자리에 항상 깔끔한 무더기로 발견되지 않도록. 물건을 모으는 몬스터가 영웅이 죽을 때 근처에 서 있었다면 무언가를 집었을 법하며, 이것이 그것이다.
 *
 * @param otmp 그 물건
 * @param x,y 영웅이 죽은 곳
 * @note 자격 있는 몬스터 중 하나가 몇 마리인지 미리 모른 채로 같은 확률로 골라진다. 마주치는 각각에게 점점 줄어드는 기회를 주어서다. 그것이 탐색이 진행되며 개수가 보관되는 이유다.
 * @note 딸린 주석이 밝히듯 그 물건이 그 몬스터가 좋아하는 종류인지 의도적으로 검사하지 않는다. 그 몬스터는 거기 있는 것을 뒤지며 흥미로워 보이는 것을 가져가는 것으로 상상되며, 그것이 자기 종족이 탐하는 것만을 정확히 가져가는 몬스터보다 나은 이야기다.
 * @note 어느 몬스터도 가져가지 않으면 바닥으로 돌아가므로, 그 물건이 사라지는 일은 결코 없다.
 */
give_to_nearby_mon(struct obj *otmp, coordxy x, coordxy y)
{
    struct monst *mtmp;
    struct monst *selected = (struct monst *) 0;
    int nmon = 0, xx, yy;

    for (xx = x - 1; xx <= x + 1; ++xx) {
        for (yy = y - 1; yy <= y + 1; ++yy) {
            if (!isok(xx, yy))
                continue;
            if (u_at(xx, yy))
                continue;
            if (!(mtmp = m_at(xx, yy)))
                continue;
            /* This doesn't do any checks on otmp to see that it matches the
             * likes_* property, intentionally. Assume that the monster is
             * rifling through and taking things that look interesting. */
            if (!(likes_gold(mtmp->data) || likes_gems(mtmp->data)
                  || likes_objs(mtmp->data) || likes_magic(mtmp->data)))
                continue;
            nmon++;
            if (!rn2(nmon))
                selected = mtmp;
        }
    }
    if (selected && can_carry(selected, otmp))
        add_to_minv(selected, otmp);
    else
        place_object(otmp, x, y);
}

/**
 * @brief Put everything the hero was carrying where a later game will find it.
 *
 * What the dead hero leaves behind, and how it is left depends on what became of them. A hero who rose as a monster carries their own possessions; one turned to stone has them sealed inside the statue; otherwise they lie on
 * the floor or in the hands of whatever was standing nearby.
 *
 * @param mtmp the monster the hero became, if they became one
 * @param cont the container to seal everything into, if they were petrified
 * @param x,y where they died
 * @note Called while paying off a shop as well as while saving bones, as the accompanying comment records. The situation is the same in the respect that matters: the hero's possessions have to be disposed of somewhere they
 *       will be found.
 * @note Most objects go to the floor and a minority to a nearby monster, which is what keeps the scene from looking either too tidy or implausibly scattered.
 * @note The container's weight is set afterwards rather than as things go in, because a container's weight is stored and would otherwise be wrong.
 */
/**
 * @brief 영웅이 지니고 있던 모든 것을 나중의 게임이 발견할 곳에 놓는다.
 *
 * 죽은 영웅이 남기는 것이며, 어떻게 남기는지는 그가 무엇이 되었는지에 달려 있다. 몬스터로 일어난 영웅은 자기 소지품을 지닌다. 돌로 변한 영웅은 그것이 조각상 안에 봉해진다. 그렇지 않으면 바닥에 놓이거나 근처에 서 있던 것의 손에 들어간다.
 *
 * @param mtmp 영웅이 몬스터가 되었다면 그 몬스터
 * @param cont 석화되었다면 모든 것을 봉해 넣을 용기
 * @param x,y 그가 죽은 곳
 * @note 딸린 주석이 기록하듯 유골을 저장하는 동안뿐 아니라 상점 값을 청산하는 동안에도 호출된다. 중요한 점에서 상황이 같다. 영웅의 소지품이 발견될 어딘가로 처분되어야 한다.
 * @note 대부분의 물건이 바닥으로 가고 소수가 근처 몬스터에게 간다. 그것이 그 장면이 너무 깔끔해 보이지도, 있을 법하지 않게 흩어져 보이지도 않게 하는 것이다.
 * @note 용기의 무게는 물건이 들어갈 때가 아니라 나중에 설정된다. 용기의 무게는 저장되며, 그러지 않으면 틀리게 된다.
 */
/* called by savebones(); also by finish_paybill(shk.c) */
void
drop_upon_death(
    struct monst *mtmp, /* monster if hero rises as one (non ghost) */
    struct obj *cont,   /* container if hero is turned into a statue */
    coordxy x, coordxy y)
{
    struct obj *otmp;

    /* when dual-wielding, the second weapon gets dropped rather than
       welded if it becomes cursed; ensure that that won't happen here
       by ending dual-wield */
    u.twoweap = FALSE; /* bypass set_twoweap() */

    /* all inventory is dropped (for the normal case), even non-droppable
       things like worn armor and accessories, welded weapon, or cursed
       loadstones */
    while ((otmp = gi.invent) != 0) {
        obj_extract_self(otmp);
        /* when turning into green slime, all gear remains held;
           other types "arise from the dead" do aren't holding
           equipment during their brief interval as a corpse */
        if (!mtmp || is_undead(mtmp->data))
            obj_no_longer_held(otmp);

        /* lamps don't go out when dropped */
        if ((cont || artifact_light(otmp)) && obj_is_burning(otmp))
            end_burn(otmp, TRUE); /* smother in statue */
        otmp->owornmask = 0L;

        if (otmp->otyp == SLIME_MOLD)
            goodfruit(otmp->spe);

        if (rn2(5))
            curse(otmp);
        if (mtmp)
            (void) add_to_minv(mtmp, otmp);
        else if (cont)
            (void) add_to_container(cont, otmp);
        else if (!rn2(8))
            give_to_nearby_mon(otmp, x, y);
        else
            place_object(otmp, x, y);
    }
    if (cont)
        cont->owt = weight(cont);
}

/**
 * @brief Put the Oracle back in her chamber before the level is saved, or say she should be removed.
 *
 * A special case, and worth reading as an example of what saving a level involves. The Oracle's chamber stops being her chamber once the hero has entered it -- the room's type changes when the welcome message is given -- and
 * she may have been teleported out of it besides. A level saved in that state would give the next game an Oracle standing in an ordinary room.
 *
 * So both are undone: the room is made her chamber again and she is put back inside it.
 *
 * @param oracle the Oracle
 * @return false if she is somewhere she should not be at all and must be removed
 * @note She is also made peaceful, because her disposition toward the dead hero is not the next hero's business.
 * @note The accompanying comment explains why the wrong-level case is handled although it cannot currently arise: she does not move, but a strong enough blow could push her onto a hole, and traps do not activate in that
 *       situation today -- which is not something to depend on.
 */
/**
 * @brief 레벨이 저장되기 전에 오라클을 자기 방으로 되돌리거나, 그가 제거되어야 한다고 말한다.
 *
 * 특수한 경우이며, 레벨을 저장하는 일에 무엇이 얽히는지의 예로 읽어 볼 가치가 있다. 오라클의 방은 영웅이 그곳에 들어선 뒤로 그의 방이기를 그만둔다. 환영 메시지가 주어질 때 그 방의 종류가 바뀐다. 게다가 그가 그곳에서 순간이동으로 빠져나갔을 수도 있다. 그 상태로 저장된 레벨은 다음 게임에게
 * 평범한 방에 서 있는 오라클을 주게 된다.
 *
 * 그래서 둘 다 되돌려진다. 그 방이 다시 그의 방이 되고 그가 그 안에 되놓인다.
 *
 * @param oracle 그 오라클
 * @return 그가 아예 있어서는 안 될 곳에 있어 제거되어야 하면 거짓
 * @note 그는 또한 평화롭게 된다. 죽은 영웅에 대한 그의 태도는 다음 영웅의 일이 아니기 때문이다.
 * @note 딸린 주석은 현재 생길 수 없는 잘못된 레벨 경우가 왜 처리되는지 설명한다. 그는 움직이지 않지만 충분히 강한 타격이 그를 구멍 위로 밀 수 있고, 오늘날 그 상황에서 함정이 발동하지 않는다. 그것은 의존할 만한 것이 아니다.
 */
/* possibly restore oracle's room and/or put her back inside it; returns
   False if she's on the wrong level and should be removed, True otherwise */
staticfn boolean
fixuporacle(struct monst *oracle)
{
    coord cc;
    int ridx, o_ridx;

    /* oracle doesn't move, but knight's joust or monk's staggering blow
       could push her onto a hole in the floor; at present, traps don't
       activate in such situation hence she won't fall to another level;
       however, that could change so be prepared to cope with such things */
    if (!Is_oracle_level(&u.uz))
        return FALSE;

    oracle->mpeaceful = 1; /* for behavior toward next character */
    o_ridx = levl[oracle->mx][oracle->my].roomno - ROOMOFFSET;
    if (o_ridx >= 0 && svr.rooms[o_ridx].rtype == DELPHI)
        return TRUE; /* no fixup needed */

    /*
     * The Oracle isn't in DELPHI room.  Either hero entered her chamber
     * and got the one-time welcome message, converting it into an
     * ordinary room, or she got teleported out, or both.  Try to put
     * her back inside her room, if necessary, and restore its type.
     */

    /* find original delphi chamber; should always succeed */
    for (ridx = 0; ridx < SIZE(svr.rooms); ++ridx)
        if (svr.rooms[ridx].orig_rtype == DELPHI)
            break;

    if (o_ridx != ridx && ridx < SIZE(svr.rooms)) {
        /* room found and she's not in it, so try to move her there */
        cc.x = (svr.rooms[ridx].lx + svr.rooms[ridx].hx) / 2;
        cc.y = (svr.rooms[ridx].ly + svr.rooms[ridx].hy) / 2;
        if (enexto(&cc, cc.x, cc.y, oracle->data)) {
            rloc_to(oracle, cc.x, cc.y);
            o_ridx = levl[oracle->mx][oracle->my].roomno - ROOMOFFSET;
        }
        /* [if her room is already full, she might end up outside;
           that's ok, next hero just won't get any welcome message,
           same as used to happen before this fixup was introduced] */
    }
    if (ridx == o_ridx) /* if she's in her room, mark it as such */
        svr.rooms[ridx].rtype = DELPHI;
    return TRUE; /* keep oracle in new bones file */
}

/**
 * @brief Whether this death should leave bones at all.
 *
 * Several unrelated reasons to decline, and they are worth distinguishing because they are not all about the level. Some are structural: the level is one bones must not be left on, or the hero is inside something and has no
 * position to be found at. One is about frequency: bones become rarer the shallower the level, so an early death does not litter the first few levels of every subsequent game.
 *
 * And one is about fairness. A game in exploration mode may be restarted freely, so leaving bones from one would let a player manufacture as many copies of an object as they liked -- which is why that case is refused
 * outright rather than made unlikely.
 *
 * @return true if bones may be saved
 * @note A portal on a level that is not a branch also prevents it, because such a portal is placed by the game for a particular game's structure and a later game's would be elsewhere.
 * @note Debug mode bypasses the rarity but not the other refusals, so bones can be produced deliberately for testing without being able to produce invalid ones.
 */
/**
 * @brief 이번 죽음이 아예 유골을 남겨야 하는지.
 *
 * 물러날 서로 무관한 이유가 여러 가지이며, 그것들을 구별할 가치가 있다. 전부가 레벨에 관한 것은 아니다. 어떤 것은 구조적이다. 그 레벨이 유골을 남겨서는 안 되는 곳이거나, 영웅이 무언가 안에 있어 발견될 위치가 없는 것. 하나는 빈도에 관한 것이다. 레벨이 얕을수록 유골이 드물어지므로, 이른
 * 죽음이 이후 모든 게임의 첫 몇 레벨을 어지럽히지 않는다.
 *
 * 그리고 하나는 공평성에 관한 것이다. 탐색 모드의 게임은 자유롭게 다시 시작될 수 있으므로, 그것에서 유골을 남기면 플레이어가 어떤 물건의 사본을 원하는 만큼 제조할 수 있게 된다. 그것이 그 경우가 가능성이 낮아지는 대신 아예 거부되는 이유다.
 *
 * @return 유골을 저장해도 되면 참
 * @note 분기가 아닌 레벨의 포탈도 그것을 막는다. 그런 포탈은 특정 게임의 구조를 위해 게임이 놓은 것이며, 나중 게임의 것은 다른 곳에 있을 것이기 때문이다.
 * @note 디버그 모드는 그 희소성은 우회하되 다른 거부는 우회하지 않는다. 그래서 시험을 위해 유골을 의도적으로 만들 수 있으면서 유효하지 않은 것을 만들 수는 없다.
 */
/* check whether bones are feasible */
boolean
can_make_bones(void)
{
    struct trap *ttmp;

    if (!flags.bones)
        return FALSE;
    if (ledger_no(&u.uz) <= 0 || ledger_no(&u.uz) > maxledgerno())
        return FALSE;
    if (no_bones_level(&u.uz))
        return FALSE; /* no bones for specific levels */
    if (u.uswallow) {
        return FALSE; /* no bones when swallowed */
    }
    if (!Is_branchlev(&u.uz)) {
        /* no bones on non-branches with portals */
        for (ttmp = gf.ftrap; ttmp; ttmp = ttmp->ntrap)
            if (ttmp->ttyp == MAGIC_PORTAL)
                return FALSE;
    }

    if (depth(&u.uz) <= 0                 /* bulletproofing for endgame */
        || (!rn2(1 + (depth(&u.uz) >> 2)) /* fewer ghosts on low levels */
            && !wizard))
        return FALSE;
    /* don't let multiple restarts generate multiple copies of objects
       in bones files */
    if (discover)
        return FALSE;
    return TRUE;
}

/**
 * @brief Remove a monster that must not be preserved in a bones file.
 *
 * Certain monsters belong to the structure of a game rather than to a level. The Wizard, Medusa, a quest leader or nemesis, Vlad -- each exists once per game and in a particular place, so a copy arriving from someone else's
 * game would either be a second one or be in the wrong place.
 *
 * @param mtmp the monster
 * @note They are removed rather than being made ordinary, because what makes them what they are is not a flag that could be cleared -- a quest nemesis with its role stripped is not a monster the game has any use for.
 * @note The Oracle is the exception and is given a chance to be repaired first, since she is tied to a room rather than to the game's structure. Only if that fails is she removed.
 */
/**
 * @brief 유골 파일에 보존되어서는 안 되는 몬스터를 제거한다.
 *
 * 어떤 몬스터들은 레벨이 아니라 게임의 구조에 속한다. 마법사, 메두사, 퀘스트 지도자나 원수, 블라드. 각각이 게임마다 한 번, 특정한 곳에 존재하므로, 다른 사람의 게임에서 도착한 사본은 두 번째가 되거나 잘못된 곳에 있게 된다.
 *
 * @param mtmp 그 몬스터
 * @note 평범하게 만들어지는 대신 제거된다. 그것들을 그것들이게 하는 것이 지울 수 있는 플래그가 아니기 때문이다. 역할이 벗겨진 퀘스트 원수는 게임이 쓸 데가 없는 몬스터다.
 * @note 오라클이 예외이며 먼저 고쳐질 기회를 받는다. 그는 게임의 구조가 아니라 방에 묶여 있기 때문이다. 그것이 실패할 때만 제거된다.
 */
/* monster might need to be removed before saving a bones file,
   in case these characters are not in their home bases */
staticfn void
remove_mon_from_bones(struct monst *mtmp)
{
    struct permonst *mptr = mtmp->data;

    if (mtmp->iswiz || mptr == &mons[PM_MEDUSA]
        || mptr->msound == MS_NEMESIS || mptr->msound == MS_LEADER
        || is_Vlad(mtmp) /* mptr == &mons[VLAD_THE_IMPALER] || cham == VLAD */
        || (mptr == &mons[PM_ORACLE] && !fixuporacle(mtmp)))
        mongone(mtmp);
}

/**
 * @brief Write out the level the hero died on, as bones for a later game.
 *
 * The whole of leaving bones. It disposes of the hero's possessions, turns the hero into whatever will be found here -- a ghost, a mummy, a statue -- removes the monsters that must not be preserved, strips knowledge from
 * everything that remains, and writes the file.
 *
 * @param how how the hero died, which decides what they become
 * @param when the time of death, recorded in the file
 * @param corpse the hero's corpse, if there is one
 * @warning Destroys the current game's level as it works. It is called while the game is ending and the level is not used afterwards, which is what makes that acceptable -- calling it at any other time would leave the level
 *          stripped of the information the running game depends on.
 * @note What the hero rises as is not always a ghost. The manner of death decides, and the accompanying code shows the cases -- which is why the disposal of possessions has to be told what became of the hero.
 */
/**
 * @brief 영웅이 죽은 레벨을 나중의 게임을 위한 유골로 써 낸다.
 *
 * 유골을 남기는 일 전체다. 영웅의 소지품을 처분하고, 영웅을 여기서 발견될 것 -- 유령, 미라, 조각상 -- 으로 바꾸고, 보존되어서는 안 되는 몬스터를 제거하고, 남은 모든 것에서 앎을 벗겨 내고, 파일을 쓴다.
 *
 * @param how 영웅이 어떻게 죽었는지. 그가 무엇이 될지를 정한다
 * @param when 사망 시각. 파일에 기록된다
 * @param corpse 영웅의 시체. 있다면
 * @warning 일하면서 현재 게임의 레벨을 파괴한다. 게임이 끝나는 동안 호출되고 그 레벨이 뒤에 쓰이지 않으며, 그것이 그것을 받아들일 만하게 하는 것이다. 다른 어느 때에 호출하면 돌아가는 게임이 의존하는 정보가 벗겨진 레벨이 남는다.
 * @note 영웅이 무엇으로 일어나는지가 항상 유령은 아니다. 죽음의 방식이 정하며, 딸린 코드가 그 경우들을 보여 준다. 그것이 소지품 처분이 영웅이 무엇이 되었는지 들어야 하는 이유다.
 */
/* save bones and possessions of a deceased adventurer */
void
savebones(int how, time_t when, struct obj *corpse)
{
    coordxy x, y;
    struct trap *ttmp;
    struct monst *mtmp;
    struct fruit *f;
    struct cemetery *newbones;
    char c, *bonesid;
    char whynot[BUFSZ];
    NHFILE *nhfp;

    /* caller has already checked `can_make_bones()' */

    clear_bypasses();
    nhfp = open_bonesfile(&u.uz, &bonesid);
    if (nhfp) {
        close_nhfile(nhfp);
        if (wizard) {
            if (y_n("Bones file already exists.  Replace it?") == 'y') {
                if (delete_bonesfile(&u.uz))
                    goto make_bones;
                else
                    pline("Cannot unlink old bones.");
            }
        }
        /* compression can change the file's name, so must
           wait until after any attempt to delete this file */
        compress_bonesfile();
        return;
    }

 make_bones:
    unleash_all();
    /* new ghost or other undead isn't punished even if hero was;
       end-of-game disclosure has already had a chance to report the
       Punished status so we don't need to preserve it any further */
    if (Punished)
        unpunish(); /* unwear uball, destroy uchain */
    /* in case dismounting kills steed [is that even possible?], do so
       before cleaning up dead monsters */
    if (u.usteed)
        dismount_steed(DISMOUNT_BONES);

    iter_mons(remove_mon_from_bones); /* send various unique monsters away, */
    dmonsfree();                      /* then discard dead or gone monsters */

    forget_engravings(); /* next hero won't have read any engravings yet */
    /* mark all named fruits as nonexistent; if/when we come to instances
       of any of them we'll mark those as existing (using goodfruit()) */
    for (f = gf.ffruit; f; f = f->nextf)
        f->fid = -f->fid;

    set_ghostly_objlist(gi.invent);
    /* dispose of your possessions, usually cursed */
    if (ismnum(u.ugrave_arise)) {
        /* give your possessions to the monster you become */
        gi.in_mklev = TRUE; /* use <u.ux,u.uy> as-is */
        mtmp = makemon(&mons[u.ugrave_arise], u.ux, u.uy, NO_MINVENT);
        gi.in_mklev = FALSE;
        if (!mtmp) { /* arise-type might have been genocided */
            drop_upon_death((struct monst *) 0, (struct obj *) 0, u.ux, u.uy);
            u.ugrave_arise = NON_PM; /* in case caller cares */
            return;
        }
        give_u_to_m_resistances(mtmp);
        mtmp = christen_monst(mtmp, svp.plname);
        newsym(u.ux, u.uy);
        /* ["Your body rises from the dead as an <mname>..." used
           to be given here, but it has been moved to done() so that
           it gets delivered even when savebones() isn't called] */
        drop_upon_death(mtmp, (struct obj *) 0, u.ux, u.uy);
        assert(mtmp != NULL && mtmp->data != NULL); /* static analysis hack */
        /* 'mtmp' now has hero's inventory; if 'mtmp' is a mummy, give it
           a wrapping unless already carrying one */
        if (mtmp->data->mlet == S_MUMMY && !m_carrying(mtmp, MUMMY_WRAPPING))
            (void) mongets(mtmp, MUMMY_WRAPPING);
        m_dowear(mtmp, TRUE);
    } else if (u.ugrave_arise == LEAVESTATUE) {
        struct obj *otmp;

        /* embed your possessions in your statue */
        otmp = mk_named_object(STATUE, &mons[u.umonnum], u.ux, u.uy,
                               svp.plname);

        drop_upon_death((struct monst *) 0, otmp, u.ux, u.uy);
        if (!otmp)
            return; /* couldn't make statue */
        mtmp = (struct monst *) 0;
    } else { /* u.ugrave_arise < LEAVESTATUE */
        /* drop everything */
        drop_upon_death((struct monst *) 0, (struct obj *) 0, u.ux, u.uy);
        /* trick makemon() into allowing monster creation
         * on your location
         */
        gi.in_mklev = TRUE;
        mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, MM_NONAME);
        gi.in_mklev = FALSE;
        if (!mtmp)
            return;
        mtmp = christen_monst(mtmp, svp.plname);
        if (corpse)
            (void) obj_attach_mid(corpse, mtmp->m_id);
    }
    if (mtmp) {
        int i;

        mtmp->m_lev = (u.ulevel ? u.ulevel : 1);
        mtmp->mhp = mtmp->mhpmax = u.uhpmax;
        mtmp->female = flags.female;
        mtmp->msleeping = 1;

        if (!has_ebones(mtmp))
            newebones(mtmp);
        if (has_ebones(mtmp)) {
            for (i = 0; i <= NUM_ROLES; ++i) {
                if (!strcmp(gu.urole.name.m, roles[i].name.m)) {
                    EBONES(mtmp)->role = i;
                    break;
                }
                /* impossible("savebones: bad gu.urole.name.m \"%s\"",
                              gu.urole.name.m); */
            }
            for (i = 0; i <= NUM_RACES; ++i) {
                if (!strcmp(gu.urace.noun, races[i].noun)) {
                    EBONES(mtmp)->race = i;
                    break;
                }
                /* impossible("savebones: bad gu.urace.noun \"%s\"",
                              gu.urace.noun); */
            }
            EBONES(mtmp)->oldalign = u.ualign;
            EBONES(mtmp)->deathlevel = u.ulevel;
            EBONES(mtmp)->luck = u.uluck; /* moreluck not included */
            EBONES(mtmp)->mnum = Role_switch;
            EBONES(mtmp)->female = flags.female;
            EBONES(mtmp)->demigod = u.uevent.udemigod;
            EBONES(mtmp)->crowned = u.uevent.uhand_of_elbereth;
        }
    }
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        set_ghostly_objlist(mtmp->minvent);
        resetobjs(mtmp->minvent, FALSE);
        /* do not zero out m_ids for bones levels any more */
        mtmp->mlstmv = 0L;
        if (mtmp->mtame)
            mtmp->mtame = mtmp->mpeaceful = 0;
        /* observations about the current hero won't apply to future game */
        mtmp->seen_resistance = M_SEEN_NOTHING;
    }
    for (ttmp = gf.ftrap; ttmp; ttmp = ttmp->ntrap) {
        ttmp->madeby_u = 0;
        ttmp->tseen = unhideable_trap(ttmp->ttyp);
    }
    set_ghostly_objlist(fobj);
    resetobjs(fobj, FALSE);
    set_ghostly_objlist(svl.level.buriedobjlist);
    resetobjs(svl.level.buriedobjlist, FALSE);

    /* Hero is no longer on the map. */
    u.ux0 = u.ux, u.uy0 = u.uy;
    u.ux = u.uy = 0;

    /* Clear all memory from the level. */
    for (x = 1; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) {
            levl[x][y].seenv = 0;
            levl[x][y].waslit = 0;
            levl[x][y].glyph = GLYPH_UNEXPLORED;
            svl.lastseentyp[x][y] = 0;
        }

    /* Attach bones info to the current level before saving. */
    newbones = (struct cemetery *) alloc(sizeof *newbones);
    /* entries are '\0' terminated but have fixed length allocations,
       so pre-fill with spaces to initialize any excess room */
    (void) memset((genericptr_t) newbones, ' ', sizeof *newbones);
    /* format name+role,&c, death reason, and date+time;
       gender and alignment reflect final values rather than what the
       character started out as, same as topten and logfile entries */
    Sprintf(newbones->who, "%s-%.3s-%.3s-%.3s-%.3s",
            svp.plname, gu.urole.filecode,
            gu.urace.filecode, genders[flags.female].filecode,
            aligns[1 - u.ualign.type].filecode);
    formatkiller(newbones->how, sizeof newbones->how, how, TRUE);
    Strcpy(newbones->when, yyyymmddhhmmss(when));
    /* final resting place, used to decide when bones are discovered */
    newbones->frpx = u.ux0, newbones->frpy = u.uy0;
    newbones->bonesknown = FALSE;
    /* if current character died on a bones level, the cemetery list
       will have multiple entries, most recent (this dead hero) first */
    newbones->next = svl.level.bonesinfo;
    svl.level.bonesinfo = newbones;
    /* flag these bones if they are being created in wizard mode;
       they might already be flagged as such, even when we're playing
       in normal mode, if this level came from a previous bones file */
    if (wizard)
        svl.level.flags.wizard_bones = 1;

    nhfp = create_bonesfile(&u.uz, &bonesid, whynot);
    if (!nhfp) {
        if (wizard)
            pline1(whynot);
        /* bones file creation problems are silent to the player.
         * Keep it that way, but place a clue into the paniclog.
         */
        paniclog("savebones", whynot);
        return;
    }
    c = (char) (strlen(bonesid) + 1);

    nhfp->mode = WRITING;
    store_version(nhfp);
    Sfo_char(nhfp, &svn.nhuuid[0], "ancestor-nhuuid", sizeof svn.nhuuid);
    /* if a bones pool digit is in use, it precedes the bonesid
       string and isn't recorded in the file */
    Sfo_char(nhfp, &c, "bones_count", 1);
    Sfo_char(nhfp, bonesid, "bonesid", (int) c);       /* DD.nnn */
    savefruitchn(nhfp);
    update_mlstmv(); /* update monsters for eventual restoration */
    savelev(nhfp, ledger_no(&u.uz));
    close_nhfile(nhfp);
    commit_bonesfile(&u.uz);
    compress_bonesfile();
}

#endif /* !SFCTOOL */

/**
 * @brief Look for a bones file for this level and load it if one is usable.
 *
 * The other end of leaving bones, and the more suspicious of the two. Everything in the file was written by another game, so the loading is a series of refusals: a file from a different version, a file whose identifier does
 * not match the level, a file the player should not be given.
 *
 * @return whether a bones level was loaded
 * @note A file that is rejected is usually removed, because a file that cannot be used will never become usable and leaving it means testing it again on every visit.
 * @note Failure is normal. Most levels have no bones file, so this returning nothing is the common case and not an error.
 * @warning Nothing in the file can be trusted the way the game's own save can, as the file header states. What is read here is passed through the stripping and conflict-resolving routines above before it becomes part of the
 *          game.
 */
/**
 * @brief 이 레벨의 유골 파일을 찾아보고 쓸 만한 것이 있으면 적재한다.
 *
 * 유골을 남기는 것의 반대쪽 끝이며, 둘 중 더 의심하는 쪽이다. 그 파일의 모든 것이 다른 게임이 쓴 것이므로, 적재는 일련의 거부다. 다른 판본의 파일, 식별자가 레벨과 맞지 않는 파일, 플레이어에게 주어져서는 안 되는 파일.
 *
 * @return 유골 레벨이 적재되었는지
 * @note 거부된 파일은 보통 제거된다. 쓸 수 없는 파일은 결코 쓸 수 있게 되지 않으며, 그것을 남기는 것은 방문할 때마다 그것을 다시 검사한다는 뜻이기 때문이다.
 * @note 실패가 정상이다. 대부분의 레벨에는 유골 파일이 없으므로, 이것이 아무것도 반환하지 않는 것이 흔한 경우이고 오류가 아니다.
 * @warning 파일 머리말이 밝히듯 그 파일의 어느 것도 게임 자신의 저장처럼 믿을 수 없다. 여기서 읽힌 것은 게임의 일부가 되기 전에 위의 벗겨 내기와 충돌 해결 루틴을 거친다.
 */
int
getbones(void)
{
    int ok;
    NHFILE *nhfp = (NHFILE *) 0;
    char c = 0, *bonesid,
         oldbonesid[40] = { 0 }; /* was [10]; more should be safer */
    char ancestor_nhuuid[SIZE(svn.nhuuid)];

#ifndef SFCTOOL
    if (discover) /* save bones files for real games */
        return 0;

    if (!flags.bones)
        return 0;
    /* wizard check added by GAN 02/05/87 */
    if (rn2(3) /* only once in three times do we find bones */
        && !wizard)
        return 0;
    if (no_bones_level(&u.uz))
        return 0;
#endif /* !SFCTOOL */

    nhfp = open_bonesfile(&u.uz, &bonesid);
    if (!nhfp)
        return 0;
    if (nhfp && nhfp->structlevel && nhfp->fd < 0)
        return 0;
    if (nhfp && nhfp->fieldlevel) {
        if (nhfp->style.deflt && !nhfp->fpdef)
        return 0;
    }

    program_state.reading_bonesfile = 1;
    if (validate(nhfp, gb.bones, FALSE, 0) != SF_UPTODATE) {
        if (!wizard)
            pline("Discarding unusable bones; no need to panic...");
        ok = FALSE;
        program_state.reading_bonesfile = 0;
    } else {
        ok = TRUE;
        if (wizard) {
            if (y_n("Get bones?") == 'n') {
                close_nhfile(nhfp);
                compress_bonesfile();
                program_state.reading_bonesfile = 0;
                return 0;
            }
        }
        Sfi_char(nhfp, &ancestor_nhuuid[0], "ancestor-nhuuid",
                 sizeof ancestor_nhuuid);
        Sfi_char(nhfp, &c, "bones_count", 1); /* length incl. '\0' */
            if ((unsigned) c <= sizeof oldbonesid) {
                Sfi_char(nhfp, oldbonesid, "bonesid", (int) c);
            } else {
                if (wizard)
                    debugpline2("Abandoning bones , %u > %u.",
                                (unsigned) c, (unsigned) sizeof oldbonesid);
                close_nhfile(nhfp);
                compress_bonesfile();
                /* ToDo: maybe unlink these problematic bones? */
                program_state.reading_bonesfile = 0;
                return 0;
            }
        if (strcmp(bonesid, oldbonesid) != 0) {
            char errbuf[BUFSZ];

            Sprintf(errbuf, "This is bones level '%s', not '%s'!",
                    oldbonesid, bonesid);
            if (wizard) {
                pline1(errbuf);
                ok = FALSE; /* won't die of trickery */
            }
            program_state.reading_bonesfile = 0;
            trickery(errbuf);
        } else {
            struct monst *mtmp;

            getlev(nhfp, 0, 0);

            /* Note that getlev() now keeps tabs on unique
             * monsters such as demon lords, and tracks the
             * birth counts of all species just as makemon()
             * does.  If a bones monster is extinct or has been
             * subject to genocide, their mhpmax will be
             * set to the magic DEFUNCT_MONSTER cookie value.
             */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (has_mgivenname(mtmp))
                    sanitize_name(MGIVENNAME(mtmp));
                if (mtmp->mhpmax == DEFUNCT_MONSTER) {
                    if (wizard) {
                        debugpline1("Removing defunct monster %s from bones.",
                                    mtmp->data->pmnames[NEUTRAL]);
                    }
                    mongone(mtmp);
                } else
                    /* to correctly reset named artifacts on the level */
                    resetobjs(mtmp->minvent, TRUE);
            }
            resetobjs(fobj, TRUE);
            resetobjs(svl.level.buriedobjlist, TRUE);
            fix_shop_damage();
        }
    }
    close_nhfile(nhfp);
    program_state.reading_bonesfile = 0;
    sanitize_engravings();
    u.uroleplay.numbones++;

    if (wizard) {
        if (y_n("Unlink bones?") == 'n') {
            compress_bonesfile();
            return ok;
        }
    }
    if (!delete_bonesfile(&u.uz)) {
        /* When N games try to simultaneously restore the same
         * bones file, N-1 of them will fail to delete it
         * (the first N-1 under AmigaDOS, the last N-1 under UNIX).
         * So no point in a mysterious message for a normal event
         * -- just generate a new level for those N-1 games.
         */
        /* pline("Cannot unlink bones."); */
        return 0;
    }
    return ok;
}

#ifndef SFCTOOL

/**
 * @brief Whether this level's bones came from a game played under a given name.
 *
 * Asked so a player is not handed their own remains, which would let a game feed itself equipment. The check is against the record of who is buried here rather than against the file, because a level may hold bones from several
 * games.
 *
 * @param name the player name to look for
 * @return true if that name is among those buried here
 * @note A hyphen is appended before comparing, as the accompanying comment explains, so that one name is not matched by another it happens to be a prefix of. Without it a player called "Al" would be refused every level buried
 *       by "Alice".
 */
/**
 * @brief 이 레벨의 유골이 주어진 이름으로 플레이된 게임에서 왔는지.
 *
 * 플레이어가 자기 잔해를 건네받지 않도록 물어진다. 그것은 게임이 스스로에게 장비를 먹일 수 있게 한다. 그 검사는 파일이 아니라 여기 묻힌 이가 누구인지의 기록에 대한 것이다. 레벨이 여러 게임의 유골을 담을 수 있기 때문이다.
 *
 * @param name 찾을 플레이어 이름
 * @return 그 이름이 여기 묻힌 이들 중에 있으면 참
 * @note 딸린 주석이 설명하듯 비교하기 전에 붙임표가 덧붙여진다. 그래서 어떤 이름이 마침 그것의 접두어인 다른 이름에 걸리지 않는다. 그것 없이는 "Al"이라는 플레이어가 "Alice"가 묻은 모든 레벨을 거부받게 된다.
 */
/* check whether current level contains bones from a particular player */
boolean
bones_include_name(const char *name)
{
    struct cemetery *bp;
    size_t len;
    char buf[BUFSZ];

    /* prepare buffer by appending terminal hyphen to name, to avoid partial
     * matches producing false positives */
    Strcpy(buf, name);
    Strcat(buf, "-");
    len = strlen(buf);

    for (bp = svl.level.bonesinfo; bp; bp = bp->next) {
        if (!strncmp(bp->who, buf, len))
            return TRUE;
    }

    return FALSE;
}

/* set the ghostly bit in a list of objects */
staticfn void
set_ghostly_objlist(struct obj *objchain)
{
    while (objchain) {
        objchain->ghostly = 1;
        objchain = objchain->nobj;
    }
}

/**
 * @brief React to the hero picking up something that came from another game.
 *
 * The mark exists so that a few objects can be adjusted at the moment they are taken up rather than when the level is loaded. A bow made for one hand does not suit the other, and saying so as it is picked up is a small piece
 * of characterisation that would be lost if it were said on arrival.
 *
 * @param obj the object
 * @note The mark is always cleared, whether anything was said or not. So this happens once per object, which is what makes the message a remark about acquiring it rather than a property of the object.
 * @note The accompanying comment records that the mark has no other use. So an object that came from bones is indistinguishable from any other once it has been picked up, which is deliberate.
 */
/**
 * @brief 영웅이 다른 게임에서 온 무언가를 집은 일에 반응한다.
 *
 * 그 표시가 존재하는 것은, 몇몇 객체가 레벨이 적재될 때가 아니라 집히는 순간에 조정될 수 있게 하기 위함이다. 한쪽 손을 위해 만들어진 활은 다른 손에 맞지 않으며, 그것이 집힐 때 그렇게 말하는 것은 도착할 때 말했다면 잃어버릴 작은 성격 묘사다.
 *
 * @param obj 그 객체
 * @note 무언가가 말해졌든 아니든 그 표시는 항상 지워진다. 그래서 이것이 객체마다 한 번 일어나며, 그것이 그 메시지를 객체의 속성이 아니라 그것을 얻는 일에 대한 언급으로 만드는 것이다.
 * @note 딸린 주석은 그 표시에 다른 용도가 없다고 기록한다. 그래서 유골에서 온 객체는 집힌 뒤로 다른 어느 것과도 구별되지 않으며, 그것은 의도적이다.
 */
/* This is called when a marked object from a bones file is picked-up.
   Some could result in a message, and the obj->ghostly flag is always
   cleared. obj->ghostly has no other usage at this time. */
void
fix_ghostly_obj(struct obj *obj)
{
    if (!obj->ghostly)
        return;
    switch(obj->otyp) {
        /* asymmetrical weapons */
        case BOW:
        case ELVEN_BOW:
        case ORCISH_BOW:
        case YUMI:
        case BOOMERANG:
            You("make adjustments to %s to suit your %s hand.",
                the(xname(obj)),
                URIGHTY ? "right" : "left");
            break;
        default:
            break;
    }
    obj->ghostly = 0;
}

/**
 * @brief Give a monster the record of which hero it used to be.
 * @param mtmp the monster
 * @note Attached only to the remains of a previous hero, which is why it is one of the optional extensions rather than a field on every monster -- a level's worth of ordinary monsters would otherwise each carry room for a
 *       hero's identity.
 * @note Records the monster's own identity in the extension as well. That is redundant while everything is well, and is what makes an extension attached to the wrong monster detectable.
 */
/**
 * @brief 몬스터에게 자신이 예전에 어느 영웅이었는지의 기록을 준다.
 * @param mtmp 그 몬스터
 * @note 이전 영웅의 잔해에만 붙으며, 그것이 이것이 모든 몬스터의 필드가 아니라 선택적 확장 중 하나인 이유다. 그러지 않으면 레벨 하나 분량의 평범한 몬스터가 각각 영웅의 정체를 담을 자리를 지니게 된다.
 * @note 그 확장에 몬스터 자신의 정체도 기록한다. 모든 것이 정상일 때는 잉여이며, 그것이 잘못된 몬스터에 붙은 확장을 감지할 수 있게 하는 것이다.
 */
void
newebones(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!EBONES(mtmp)) {
        EBONES(mtmp) = (struct ebones *) alloc(
            sizeof (struct ebones));
        (void) memset((genericptr_t) EBONES(mtmp), 0,
                      sizeof (struct ebones));
        EBONES(mtmp)->parentmid = mtmp->m_id;
    }
}

/**
 * @brief Release a monster's record of which hero it was.
 * @param mtmp the monster
 * @warning Currently called from nowhere, as the accompanying comment states. It exists for completeness -- every optional extension has a matching release, and the machinery that frees a monster's extensions expects one to
 *          exist even where nothing calls it individually.
 */
/**
 * @brief 몬스터가 어느 영웅이었는지의 기록을 놓아준다.
 * @param mtmp 그 몬스터
 * @warning 딸린 주석이 밝히듯 현재 어디서도 호출되지 않는다. 완결성을 위해 존재한다. 모든 선택적 확장에는 짝이 되는 해제가 있고, 몬스터의 확장을 해제하는 기제는 개별적으로 그것을 호출하는 것이 없는 곳에서도 그것이 존재하기를 기대한다.
 */
/* this is not currently used */
void
free_ebones(struct monst *mtmp)
{
    if (mtmp->mextra && EBONES(mtmp)) {
        free((genericptr_t) EBONES(mtmp));
        EBONES(mtmp) = (struct ebones *) 0;
    }
}

#endif /* SFCTOOL */

/*bones.c*/
