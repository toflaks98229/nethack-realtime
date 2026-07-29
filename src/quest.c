/* NetHack 5.0	quest.c	$NHDT-Date: 1781973063 2026/06/20 16:31:03 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.48 $ */
/*      Copyright 1991, M. Stephenson             */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file quest.c
 * @brief 퀘스트 던전 분기(branch)의 진행 상태 및 대화 처리.
 *
 * 퀘스트 리더/네메시스/수호자와의 대화, 퀘스트 부여 자격(레벨·정렬) 판정,
 * 퀘스트 아티팩트 회수와 완료 처리, 부적격 시 퀘스트 밖으로의 추방 등을
 * 담당한다. 진행 상태는 @c svq.quest_status 에 기록된다.
 *
 * @note static 헬퍼가 공개 함수와 촘촘히 뒤섞여 있어 재배치는 적용하지 않고
 *       정의 위치에서 문서화한다.
 */

#include "hack.h"

/*  quest dungeon branch routines. */

#include "quest.h"

/** @brief 이번 레벨 진입이 처음이 아닌지(같은 레벨에서 재호출) 여부. */
#define Not_firsttime (on_level(&u.uz0, &u.uz))
/** @brief 퀘스트 진행 상태(@c svq.quest_status)의 필드 접근 축약 매크로. */
#define Qstat(x) (svq.quest_status.x)

staticfn void on_start(void);
staticfn void on_locate(void);
staticfn void on_goal(void);
staticfn boolean not_capable(void);
staticfn int is_pure(boolean);
staticfn void expulsion(boolean);
staticfn void chat_with_leader(struct monst *);
staticfn void chat_with_nemesis(void);
staticfn void chat_with_guardian(void);
staticfn void prisoner_speaks(struct monst *);

/**
 * @brief 퀘스트 시작 레벨 진입 시 적절한 안내 메시지를 출력한다.
 * @note 최초 진입과 재진입(준비 상태에 따라)을 구분하여 다른 문구를 낸다.
 */
staticfn void
on_start(void)
{
    if (!Qstat(first_start)) {
        qt_pager("firsttime");
        Qstat(first_start) = TRUE;
    } else if ((u.uz0.dnum != u.uz.dnum) || (u.uz0.dlevel < u.uz.dlevel)) {
        if (Qstat(not_ready) <= 2)
            qt_pager("nexttime");
        else
            qt_pager("othertime");
    }
}

/**
 * @brief 퀘스트 로케이트(locate) 레벨 진입 시 안내 메시지를 출력한다.
 * @note 메시지는 위층에서 내려온 경우를 전제로 하며, 네메시스를 이미
 *       처치했으면 아무것도 출력하지 않는다.
 */
staticfn void
on_locate(void)
{
    /* the locate messages are phrased in a manner such that they only
       make sense when arriving on the level from above */
    boolean from_above = (u.uz0.dlevel < u.uz.dlevel);

    if (Qstat(killed_nemesis)) {
        return;
    } else if (!Qstat(first_locate)) {
        if (from_above)
            qt_pager("locate_first");
        /* if we've arrived from below this will be a lie, but there won't
           be any point in delivering the message upon a return visit from
           above later since the level has now been seen */
        Qstat(first_locate) = TRUE;
    } else {
        if (from_above)
            qt_pager("locate_next");
    }
}

/**
 * @brief 퀘스트 목표(네메시스) 레벨 진입 시 안내 메시지를 출력한다.
 * @note 퀘스트 아티팩트의 존재 여부에 따라 대체 메시지를 요청할 수 있다.
 */
staticfn void
on_goal(void)
{
    if (Qstat(killed_nemesis)) {
        return;
    } else if (!Qstat(made_goal)) {
        qt_pager("goal_first");
        Qstat(made_goal) = 1;
    } else {
        /*
         * Some QT_NEXTGOAL messages reference the quest artifact;
         * find out if it is still present.  If not, request an
         * alternate message (qt_pager() will revert to delivery
         * of QT_NEXTGOAL if current role doesn't have QT_ALTGOAL).
         * Note: if hero is already carrying it, it is treated as
         * being absent from the level for quest message purposes.
         */
        unsigned whichobjchains = ((1 << OBJ_FLOOR)
                                   | (1 << OBJ_MINVENT)
                                   | (1 << OBJ_BURIED));
        struct obj *qarti = find_quest_artifact(whichobjchains);

        qt_pager(qarti ? "goal_next" : "goal_alt");
        if (Qstat(made_goal) < 7)
            Qstat(made_goal)++;
    }
}

/**
 * @brief 퀘스트 특수 레벨 진입 시 상황에 맞는 안내를 분배한다.
 * @note 퀘스트를 이미 완료했거나 같은 레벨 재호출이면 아무 동작도 하지 않는다.
 */
void
onquest(void)
{
    if (u.uevent.qcompleted || Not_firsttime)
        return;
    if (!Is_special(&u.uz))
        return;

    if (Is_qstart(&u.uz))
        on_start();
    else if (Is_qlocate(&u.uz))
        on_locate();
    else if (Is_nemesis(&u.uz))
        on_goal();
    return;
}

/**
 * @brief 퀘스트 네메시스 사망을 상태에 기록하고 메시지를 출력한다.
 */
void
nemdead(void)
{
    if (!Qstat(killed_nemesis)) {
        Qstat(killed_nemesis) = TRUE;
        qt_pager("killed_nemesis");
    }
}

/**
 * @brief 퀘스트 리더 사망을 상태에 기록한다.
 */
void
leaddead(void)
{
    if (!Qstat(killed_leader)) {
        Qstat(killed_leader) = TRUE;
        /* TODO: qt_pager("killed_leader"); ? */
    }
}

/**
 * @brief 퀘스트 아티팩트를 처음 만졌을 때의 메시지·효과를 처리한다.
 * @param[in] obj 만진 퀘스트 아티팩트.
 * @note 최초 1회만 메시지를 출력하고 지혜를 단련시킨다.
 */
void
artitouch(struct obj *obj)
{
    if (!Qstat(touched_artifact)) {
        /* in case we haven't seen the item yet (ie, currently blinded),
           this quest message describes it by name so mark it as seen */
        observe_object(obj);
        /* only give this message once */
        Qstat(touched_artifact) = TRUE;
        qt_pager("gotit");
        exercise(A_WIS, TRUE);
    }
}

/**
 * @brief 퀘스트 던전으로 진입해도 되는지 판정한다(do.c 의 레벨 변경 검사용).
 * @return 진입 자격이 있으면 TRUE, 없으면 FALSE.
 */
/* external hook for do.c (level change check) */
boolean
ok_to_quest(void)
{
    return (boolean) (((Qstat(got_quest) || Qstat(got_thanks))
                       && is_pure(FALSE) > 0) || Qstat(killed_leader));
}

/**
 * @brief 영웅이 퀘스트 최소 레벨에 도달했는지 판정한다.
 * @return 최소 레벨 미만이면 TRUE(자격 없음), 충족하면 FALSE.
 */
staticfn boolean
not_capable(void)
{
    return (boolean) (u.ulevel < MIN_QUEST_LEVEL);
}

/**
 * @brief 영웅의 정렬 순수성(퀘스트 자격)을 판정한다.
 * @param[in] talk TRUE 이고 위저드 모드이면 진단 메시지·보정 프롬프트를 낸다.
 * @return 순수하면 1, 개종했으면 -1, 그 외(정렬 점수 부족 등) 0.
 */
staticfn int
is_pure(boolean talk)
{
    int purity;
    aligntyp original_alignment = u.ualignbase[A_ORIGINAL];

    if (wizard && talk) {
        if (u.ualign.type != original_alignment) {
            You("are currently %s instead of %s.", align_str(u.ualign.type),
                align_str(original_alignment));
        } else if (u.ualignbase[A_CURRENT] != original_alignment) {
            You("have converted.");
        } else if (u.ualign.record < MIN_QUEST_ALIGN) {
            You("are currently %d and require %d.", u.ualign.record,
                MIN_QUEST_ALIGN);
            if (yn_function("adjust?", (char *) 0, 'y', TRUE) == 'y')
                u.ualign.record = MIN_QUEST_ALIGN;
        }
    }
    purity = (u.ualign.record >= MIN_QUEST_ALIGN
              && u.ualign.type == original_alignment
              && u.ualignbase[A_CURRENT] == original_alignment)
                 ? 1
                 : (u.ualignbase[A_CURRENT] != original_alignment) ? -1 : 0;
    return purity;
}

/**
 * @brief 영웅을 퀘스트 던전의 상위 던전 계단으로 추방한다.
 * @param[in] seal TRUE 이면 퀘스트로 통하는 마법 포탈을 제거하여 봉인한다.
 * @note 영웅이 현재 퀘스트 던전 안에 있고 분기가 하나뿐임을 전제로 한다.
 */
/*
 * Expel the player to the stairs on the parent of the quest dungeon.
 *
 * This assumes that the hero is currently _in_ the quest dungeon and that
 * there is a single branch to and from it.
 */
staticfn void
expulsion(boolean seal)
{
    branch *br;
    d_level *dest;
    struct trap *t;
    int portal_flag = u.uevent.qexpelled ? UTOTYPE_NONE : UTOTYPE_PORTAL;

    br = dungeon_branch("The Quest");
    dest = (br->end1.dnum == u.uz.dnum) ? &br->end2 : &br->end1;
    if (seal)
        portal_flag |= UTOTYPE_RMPORTAL;
    nomul(0); /* stop running */
    schedule_goto(dest, portal_flag, (char *) 0, (char *) 0);
    if (seal) { /* remove the portal to the quest - sealing it off */
        int reexpelled = u.uevent.qexpelled;

        u.uevent.qexpelled = 1;
        remdun_mapseen(quest_dnum);
        /* Delete the near portal now; the far (main dungeon side)
           portal will be deleted as part of arrival on that level.
           If monster movement is in progress, any who haven't moved
           yet will now miss out on a chance to wander through it... */
        for (t = gf.ftrap; t; t = t->ntrap)
            if (t->ttyp == MAGIC_PORTAL)
                break;
        if (t)
            deltrap(t); /* (display might be briefly out of sync) */
        else if (!reexpelled)
            impossible("quest portal already gone?");
    }
}

/**
 * @brief 퀘스트 아티팩트를 리더에게 반환하여 퀘스트 완료를 처리한다.
 *
 * 완료 문구를 아직 주지 않았으면 지금 주고, 이미 주었으면 아티팩트를 계속
 * 지닌 채 포탈로 돌아가는 상황에 대한 메시지를 준다. 소환 아이템(주로 벨)을
 * 리더에게 던지거나 찬 경우에도 호출된다.
 *
 * @param[in] obj 반환된 퀘스트 아티팩트 또는 던져진 유니크/가짜 부적.
 *                옌더의 부적을 소지한 경우 NULL 일 수 있다.
 */
/* Either you've returned to quest leader while carrying the quest
   artifact or you've just thrown it to/at him or her.  If quest
   completion text hasn't been given yet, give it now.  Otherwise
   give another message about the character keeping the artifact
   and using the magic portal to return to the dungeon.  Also called
   if hero throws or kicks an invocation item (probably the Bell)
   at the leader. */
void
finish_quest(struct obj *obj) /* quest artifact or thrown unique item or faux
                               * AoY; possibly null if carrying the Amulet */
{
    struct obj *otmp;

    if (obj && !is_quest_artifact(obj)) {
        /* tossed an invocation item (or [fake] AoY) at the quest leader */
        if (Deaf)
            return; /* optional (unlike quest completion) so skip if deaf */
        /* do ID first so that the message identifying the item will refer to
           it by name (and so justify the ID we already gave...) */
        fully_identify_obj(obj);
        /* update_inventory() is not necessary or helpful here because item
           was thrown, so isn't currently in inventory anyway */
        if (obj->otyp == AMULET_OF_YENDOR) {
            qt_pager("hasamulet");
        } else if (obj->otyp == FAKE_AMULET_OF_YENDOR) {
            verbalize(
      "Sorry to say, this is a mere imitation of the true Amulet of Yendor.");
        } else {
            verbalize("Ah, I see you've found %s.", the(xname(obj)));
        }
        return;
    }

    if (u.uhave.amulet) {
        /* has the amulet in inventory -- most likely the player has already
           completed the quest and stopped in on her way back up, but it's not
           impossible to have gotten the amulet before formally presenting the
           quest artifact to the leader. */
        qt_pager("hasamulet");
        /* leader IDs the real amulet but ignores any fakes */
        if ((otmp = carrying(AMULET_OF_YENDOR)) != (struct obj *) 0) {
            fully_identify_obj(otmp);
            update_inventory();
        }
    } else {
        /* normal quest completion; threw artifact or walked up carrying it */
        qt_pager(!Qstat(got_thanks) ? "offeredit" : "offeredit2");
        /* should have obtained bell during quest;
           if not, suggest returning for it now */
        if ((otmp = carrying(BELL_OF_OPENING)) == 0)
            com_pager("quest_complete_no_bell");
    }
    Qstat(got_thanks) = TRUE;

    if (obj) {
        u.uevent.qcompleted = 1; /* you did it! */
        /* behave as if leader imparts sufficient info about the
           quest artifact */
        fully_identify_obj(obj);
        update_inventory();
    }
}

/**
 * @brief 퀘스트 리더와의 대화를 규칙에 따라 처리한다.
 *
 * 진행 상태(부적 소지, 아티팩트 회수, 자격 판정 등)에 따라 격려·퀘스트 부여·
 * 추방·완료 등 서로 다른 반응을 분기한다.
 *
 * @param[in,out] mtmp 대화 상대인 퀘스트 리더.
 */
staticfn void
chat_with_leader(struct monst *mtmp)
{
    if (!mtmp->mpeaceful || Qstat(pissed_off))
        return;

    /*  Rule 0: Cheater checks. */
    if (u.uhave.questart && !Qstat(met_nemesis))
        Qstat(cheater) = TRUE;

    /*  It is possible for you to get the amulet without completing
     *  the quest.  If so, try to induce the player to quest.
     */
    if (Qstat(got_thanks)) {
        /* Rule 1: You've gone back with/without the amulet. */
        if (u.uhave.amulet)
            finish_quest((struct obj *) 0);

        /* Rule 2: You've gone back before going for the amulet. */
        else
            qt_pager("posthanks");

    /* Rule 3: You've got the artifact and are back to return it. */
    } else if (u.uhave.questart) {
        struct obj *otmp;

        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (is_quest_artifact(otmp))
                break;

        finish_quest(otmp);

    /* Rule 4: You haven't got the artifact yet. */
    } else if (Qstat(got_quest)) {
        qt_pager("encourage");

    /* Rule 5: You aren't yet acceptable - or are you? */
    } else {
        int purity = 0;

        if (!Qstat(met_leader)) {
            qt_pager("leader_first");
            Qstat(met_leader) = TRUE;
            Qstat(not_ready) = 0;
        } else
            qt_pager("leader_next");

        /* the quest leader might have passed through the portal into
           the regular dungeon; none of the remaining make sense there */
        if (!on_level(&u.uz, &qstart_level))
            return;

        if (not_capable()) {
            qt_pager("badlevel");
            exercise(A_WIS, TRUE);
            expulsion(FALSE);
        } else if ((purity = is_pure(TRUE)) < 0) {
            if (!Qstat(pissed_off)) {
                com_pager("banished");
                Qstat(pissed_off) = TRUE;
                expulsion(FALSE);

                /* being expelled is hardly an achievement but none of the
                   other livelog classifications fit */
                livelog_printf(LL_ACHIEVE,
                               "%s has expelled you from the quest",
                               noit_mon_nam(mtmp));
            }
        } else if (purity == 0) {
            qt_pager("badalign");
            Qstat(not_ready) = 1;
            exercise(A_WIS, TRUE);
            expulsion(FALSE);
        } else { /* You are worthy! */
            qt_pager("assignquest");
            exercise(A_WIS, TRUE);
            Qstat(got_quest) = TRUE;

            /* phrasing is a bit clumsy but allows #chronicle to provide a
               clue to players who are reaching the quest for first time;
               matters most for Home 1 that has stairs down which aren't
               easily found */
            livelog_printf(LL_ACHIEVE,
                     "%s has granted access to proceed deeper into the quest",
                           noit_mon_nam(mtmp));
        }
    }
}

/**
 * @brief 퀘스트 리더가 말을 건다(공격받았을 때 포함).
 * @param[in,out] mtmp 퀘스트 리더.
 * @note 리더가 적대적이 되면 상태를 갱신하되, 관련 문구는 한 번만 출력한다.
 */
void
leader_speaks(struct monst *mtmp)
{
    /* maybe you attacked leader? */
    if (!mtmp->mpeaceful) {
        if (!Qstat(pissed_off)) {
            /* again, don't end it permanently if the leader gets angry
             * since you're going to have to kill him to go questing... :)
             * ...but do only show this crap once. */
            qt_pager("leader_last");
        }
        Qstat(pissed_off) = TRUE;
        mtmp->mstrategy &= ~STRAT_WAITMASK; /* end the inaction */
    }
    /* the quest leader might have passed through the portal into the
       regular dungeon; if so, mustn't perform "backwards expulsion" */
    if (!on_level(&u.uz, &qstart_level))
        return;

    if (!Qstat(pissed_off))
        chat_with_leader(mtmp);
}

/**
 * @brief 퀘스트 네메시스와의 대화(주로 조롱)를 처리한다.
 */
staticfn void
chat_with_nemesis(void)
{
    /*  The nemesis will do most of the talking, but... */
    qt_pager("discourage");
    if (!Qstat(met_nemesis))
        Qstat(met_nemesis++);
}

/**
 * @brief 퀘스트 네메시스가 상황에 맞는 대사를 말한다.
 * @note 전투 중이면 무작위 저주를 내뱉고, 그렇지 않으면 진행도에 따라
 *       다른 도발 문구를 사용한다.
 */
void
nemesis_speaks(void)
{
    if (!Qstat(in_battle)) {
        if (u.uhave.questart)
            qt_pager("nemesis_wantsit");
        else if (Qstat(made_goal) == 1 || !Qstat(met_nemesis))
            qt_pager("nemesis_first");
        else if (Qstat(made_goal) < 4)
            qt_pager("nemesis_next");
        else if (Qstat(made_goal) < 7)
            qt_pager("nemesis_other");
        else if (!rn2(5))
            qt_pager("discourage");
        if (Qstat(made_goal) < 7)
            Qstat(made_goal)++;
        Qstat(met_nemesis) = TRUE;
    } else /* he will spit out random maledictions */
        if (!rn2(5))
        qt_pager("discourage");
}

/**
 * @brief 죽어가는 네메시스 주위에 유독 가스 구름을 생성한다.
 * @param[in] mx,my 가스 구름의 중심 좌표.
 * @note 영웅이 네메시스를 막 처치했더라도 가스 구름의 책임이 영웅에게
 *       돌아가지 않도록 몬스터 행동으로 처리한다.
 */
/* create cloud of stinking gas around dying nemesis */
void
nemesis_stinks(coordxy mx, coordxy my)
{
    boolean save_mon_moving = svc.context.mon_moving;

    /*
     * Some nemeses (determined by caller) release a cloud of noxious
     * gas when they die.  Don't make the hero be responsible for such
     * a cloud even if hero has just killed nemesis.
     */
    svc.context.mon_moving = TRUE;
    create_gas_cloud(mx, my, 5, 8);
    svc.context.mon_moving = save_mon_moving;
}

/**
 * @brief 퀘스트 수호자(guardian)와의 대화를 처리한다.
 * @note 아티팩트 회수·네메시스 처치 여부에 따라 다른 문구를 사용한다.
 */
staticfn void
chat_with_guardian(void)
{
    /*  These guys/gals really don't have much to say... */
    if (u.uhave.questart && Qstat(killed_nemesis))
        qt_pager("guardtalk_after");
    else
        qt_pager("guardtalk_before");
}

/**
 * @brief 갇힌 죄수(prisoner) 몬스터를 깨우고 해방시킨다.
 * @param[in,out] mtmp 대상 몬스터(죄수가 아니면 무시).
 * @note 해방 시 영웅의 신이 기뻐하여 정렬이 오르지만, 경비병들은 분노한다.
 */
staticfn void
prisoner_speaks(struct monst *mtmp)
{
    if (mtmp->data == &mons[PM_PRISONER]
        && (mtmp->mstrategy & STRAT_WAITMASK)) {
        /* Awaken the prisoner */
        if (canseemon(mtmp))
            pline("%s speaks:", Monnam(mtmp));
        SetVoice(mtmp, 0, 80, 0);
        verbalize("I'm finally free!");
        mtmp->mstrategy &= ~STRAT_WAITMASK;
        mtmp->mpeaceful = 1;

        /* Your god is happy... */
        adjalign(3);

        /* ...But the guards are not */
        (void) angry_guards(FALSE);
    }
    return;
}

/**
 * @brief 영웅이 퀘스트 캐릭터에게 말을 걸 때(#chat) 적절한 대화로 분배한다.
 * @param[in,out] mtmp 대화 상대 몬스터(리더/네메시스/수호자).
 */
void
quest_chat(struct monst *mtmp)
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
        chat_with_leader(mtmp);
        /* leader might have become pissed during the chat */
        if (Qstat(pissed_off))
            setmangry(mtmp, FALSE);
        return;
    }
    switch (mtmp->data->msound) {
    case MS_NEMESIS:
        chat_with_nemesis();
        break;
    case MS_GUARDIAN:
        chat_with_guardian();
        break;
    default:
        impossible("quest_chat: Unknown quest character %s.", mon_nam(mtmp));
    }
}

/**
 * @brief 퀘스트 캐릭터가 (영웅의 행동에 반응하여) 먼저 말을 거는 경우를 분배한다.
 * @param[in,out] mtmp 말을 거는 몬스터(리더/네메시스/죄수).
 */
void
quest_talk(struct monst *mtmp)
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
        leader_speaks(mtmp);
        return;
    }
    switch (mtmp->data->msound) {
    case MS_NEMESIS:
        nemesis_speaks();
        break;
    case MS_DJINNI:
        prisoner_speaks(mtmp);
        break;
    default:
        break;
    }
}

/**
 * @brief 네메시스가 영웅과 교전 중인지 상태를 갱신한다.
 * @param[in] mtmp 검사할 몬스터(네메시스일 때만 의미 있음).
 */
void
quest_stat_check(struct monst *mtmp)
{
    if (mtmp->data->msound == MS_NEMESIS)
        Qstat(in_battle) = (!helpless(mtmp) && monnear(mtmp, u.ux, u.uy));
}

#undef Not_firsttime
#undef Qstat

/*quest.c*/
