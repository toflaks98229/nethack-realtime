/* NetHack 5.0	allmain.c	$NHDT-Date: 1781973040 2026/06/20 16:30:40 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.304 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */
/* MODIFIED 2026-07 (real-time fork): added the REALTIME_PROTO shared world
   clock and real-time command pacing; see MODIFICATIONS.md.  This file
   differs from the upstream NetHack distribution. */

/* various code that was replicated in *main.c */

/**
 * @file allmain.c
 * @brief The loop the whole game runs in, and the bookkeeping a turn implies.
 *
 * @c moveloop_core() is where the game actually happens: it hands out movement
 * to the hero and the monsters, lets whoever can act do so, and when everyone
 * has run out, ends the turn and runs everything that a turn passing means --
 * healing, hunger, timed effects, spells ageing, the dungeon stirring.
 *
 * The code here was originally duplicated across each port's main(), which is
 * why it sits in a file of its own rather than with any one subsystem.
 *
 * In this fork the loop is also where real time enters the game: the world is
 * advanced on a wall-clock rather than on the player pressing keys. See
 * MODIFICATIONS.md; everything specific to that is behind @c REALTIME_PROTO,
 * which is defined centrally in @c config.h.
 *
 * @note "Moves" counts turns, not steps; a fast hero acts several times within
 *       one. The distinction matters to every per-turn effect below.
 */

/**
 * @file allmain.c
 * @brief 게임 전체가 도는 루프와, 한 턴이 수반하는 처리들.
 *
 * @c moveloop_core() 가 실제로 게임이 일어나는 곳이다. 영웅과 몬스터에게
 * 이동력을 나눠 주고, 행동할 수 있는 쪽이 행동하게 하며, 모두가 이동력을 다
 * 쓰면 턴을 끝내고 한 턴이 지났다는 것이 뜻하는 모든 일을 실행한다. 회복,
 * 허기, 시한 효과, 주문의 노화, 던전의 움직임 같은 것들이다.
 *
 * 이 코드는 원래 각 포팅의 main() 마다 중복되어 있었고, 그래서 특정 서브시스템에
 * 속하지 않고 별도 파일로 존재한다.
 *
 * 이 포크에서는 이 루프가 실시간이 게임에 들어오는 지점이기도 하다. 세계는
 * 플레이어가 키를 누르는 것이 아니라 벽시계에 따라 전진한다. MODIFICATIONS.md
 * 참고. 그에 해당하는 모든 것은 @c REALTIME_PROTO 뒤에 있으며, 이 스위치는
 * @c config.h 에서 중앙 관리된다.
 *
 * @note "moves" 는 걸음이 아니라 턴을 센다. 빠른 영웅은 한 턴 안에 여러 번
 *       행동한다. 이 구분은 아래의 모든 턴 단위 효과에 영향을 준다.
 */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

staticfn void moveloop_preamble(boolean);
staticfn void u_calc_moveamt(int);
staticfn void maybe_generate_rnd_mon(void);
staticfn void maybe_do_tutorial(void);
#ifdef POSITIONBAR
staticfn void do_positionbar(void);
#endif
staticfn void regen_pw(int);
staticfn void regen_hp(int);
staticfn void interrupt_multi(const char *);

/**
 * @brief Bring the process to a state in which anything else may be called.
 * @param[in] argc Argument count as received by @c main().
 * @param[in] argv Argument vector as received by @c main().
 * @note Runs before options, before the window system, before a game exists;
 *       only things with no prerequisites of their own belong here.
 */
/**
 * @brief 다른 무엇이든 호출할 수 있는 상태로 프로세스를 끌어올린다.
 * @param[in] argc @c main() 이 받은 인자 개수.
 * @param[in] argv @c main() 이 받은 인자 벡터.
 * @note 옵션보다, 윈도우 시스템보다, 게임이 존재하기보다 먼저 실행된다. 자기
 *       나름의 선행 조건이 없는 것들만 여기에 둔다.
 */
/*ARGSUSED*/
void
early_init(int argc, char *argv[])
{
    program_state_init();
#ifdef CRASHREPORT
    /* Do this as early as possible, but let ports do other things first. */
    crashreport_init(argc, argv);
#endif
    decl_globals_init();
    objects_globals_init();
    monst_globals_init();
    sys_early_init();
    runtime_info_init();
    nhUse(argc);
    nhUse(argv[0]);
}

/**
 * @brief Put the world in order before the first turn is taken.
 * @param[in] resuming FALSE for a new game, TRUE when a save was restored.
 * @note A restored game must not repeat what the original start already did,
 *       which is what the flag distinguishes; the level still has to be woken
 *       up either way.
 */
/**
 * @brief 첫 턴이 시작되기 전에 세계를 정돈한다.
 * @param[in] resuming 새 게임이면 FALSE, 저장을 복원한 것이면 TRUE.
 * @note 복원된 게임은 최초 시작이 이미 한 일을 되풀이해서는 안 되며, 이 플래그가
 *       그것을 구분한다. 다만 레벨을 깨우는 일은 어느 쪽이든 필요하다.
 */
staticfn void
moveloop_preamble(boolean resuming)
{
    /* if a save file created in normal mode is now being restored in
       explore mode, treat it as normal restore followed by 'X' command
       to use up the save file and require confirmation for explore mode */
    if (resuming && iflags.deferred_X)
        (void) enter_explore_mode();

    /* side-effects from the real world */
    flags.moonphase = phase_of_the_moon();
    if (flags.moonphase == FULL_MOON) {
        You("are lucky!  Full moon tonight.");
        change_luck(1);
    } else if (flags.moonphase == NEW_MOON) {
        pline("Be careful!  New moon tonight.");
    }
    flags.friday13 = friday_13th();
    if (flags.friday13) {
        pline("Watch out!  Bad things can happen on Friday the 13th.");
        change_luck(-1);
    }

    if (!resuming) { /* new game */
        program_state.beyond_savefile_load = 1; /* for TTY_PERM_INVENT */
        svc.context.rndencode = rnd(9000);
        set_wear((struct obj *) 0); /* for side-effects of starting gear */
        reset_justpicked(gi.invent);
        (void) pickup(1);      /* autopickup at initial location */
        /* only matters if someday a character is able to start with
           clairvoyance (wizard with cornuthaum perhaps?); without this,
           first "random" occurrence would always kick in on turn 1 */
        svc.context.seer_turn = (long) rnd(30);
        /* give hero initial movement points; new game only--for restore,
           pending movement points were included in the save file */
        u.umovement = NORMAL_SPEED;
        initrack();
    }
    disp.botlx = TRUE; /* for STATUS_HILITES */
    if (resuming) { /* restoring old game */
        read_engr_at(u.ux, u.uy); /* subset of pickup() */
        fix_shop_damage();
    }

    encumber_msg(); /* in case they auto-picked up something */
    if (gd.defer_see_monsters) {
        gd.defer_see_monsters = FALSE;
        see_monsters();
    }

    u.uz0.dlevel = u.uz.dlevel;
    svc.context.move = 0;

    /* finish processing "--debug:fuzzer" from the command line */
    if (iflags.fuzzerpending) {
        iflags.debug_fuzzer = fuzzer_impossible_panic;
        iflags.fuzzerpending = FALSE;
    }

    program_state.in_moveloop = 1;
    /* for perm_invent preset at startup, display persistent inventory after
       invent is fully populated and the in_moveloop flag has been set */
    if (iflags.perm_invent)
        update_inventory();
}

/**
 * @brief Grant the hero this turn's movement, adjusted for speed and burden.
 * @param[in] wtcap Current encumbrance level, @c UNENCUMBERED upward.
 * @note Speed is granted probabilistically rather than as a fraction: a
 *       hasted hero gains a whole extra action on some turns instead of a
 *       partial one on every turn, which is what makes speed unpredictable
 *       from the player's side.
 * @note Riding hands the question to the steed -- the hero's own speed does
 *       not add to what the steed can do.
 */
/**
 * @brief 속도와 하중을 반영해 이번 턴의 이동력을 영웅에게 지급한다.
 * @param[in] wtcap 현재 하중 단계. @c UNENCUMBERED 이상.
 * @note 속도는 비율이 아니라 확률로 지급된다. 가속된 영웅은 매 턴 일부를 더
 *       받는 것이 아니라 일부 턴에 행동 하나를 통째로 더 얻는다. 플레이어
 *       입장에서 속도를 예측하기 어려운 이유가 이것이다.
 * @note 탈것을 타고 있으면 이 판단은 탈것에게 넘어간다. 영웅 자신의 속도는
 *       탈것이 할 수 있는 일에 더해지지 않는다.
 */
staticfn void
u_calc_moveamt(int wtcap)
{
    int moveamt = 0;

    /* calculate how much time passed. */
    if (u.usteed && u.umoved) {
        /* your speed doesn't augment steed's speed */
        moveamt = mcalcmove(u.usteed, TRUE);
    } else {
        moveamt = gy.youmonst.data->mmove;

        if (Very_fast) { /* speed boots, potion, or spell */
            /* gain a free action on 2/3 of turns */
            if (rn2(3) != 0)
                moveamt += NORMAL_SPEED;
        } else if (Fast) { /* intrinsic */
            /* gain a free action on 1/3 of turns */
            if (rn2(3) == 0)
                moveamt += NORMAL_SPEED;
        }
    }

    switch (wtcap) {
    case UNENCUMBERED:
        break;
    case SLT_ENCUMBER:
        moveamt -= (moveamt / 4);
        break;
    case MOD_ENCUMBER:
        moveamt -= (moveamt / 2);
        break;
    case HVY_ENCUMBER:
        moveamt -= ((moveamt * 3) / 4);
        break;
    case EXT_ENCUMBER:
        moveamt -= ((moveamt * 7) / 8);
        break;
    default:
        break;
    }

    u.umovement += moveamt;
    if (u.umovement < 0)
        u.umovement = 0;
}

/* small chance of generating a new random monster */
staticfn void
maybe_generate_rnd_mon(void)
{
    if (!rn2(u.uevent.udemigod ? 25
             : (depth(&u.uz) > depth(&stronghold_level)) ? 50
             : 70))
        (void) makemon((struct permonst *) 0, 0, 0, NO_MM_FLAGS);
}

#if defined(MICRO) || defined(WIN32)
static int mvl_abort_lev;
#endif
static int mvl_wtcap = 0;
static int mvl_change = 0;

#ifdef REALTIME_PROTO
/*
 * Real-time world clock (stage 2 of turn-based -> real-time conversion).
 *
 * Shared by the console command path below and the win32 tile/GUI port in
 * mswproc.c so the whole game runs off a single clock.  Returns TRUE at most
 * once every RT_TURN_MS of real wall-clock time; callers wait for it before
 * letting the world advance one game turn.  That gives a constant pace no
 * matter how fast (or slow) the player presses keys -- mashing buffers input
 * rather than fast-forwarding time.
 */
boolean
rt_world_tick_ready(void)
{
    static unsigned long last = 0;
    unsigned long now = nt_ticks();

    if (last == 0) { /* first call: start the clock, allow an immediate turn */
        last = now ? now : 1;
        return TRUE;
    }
    if ((unsigned long) (now - last) >= (unsigned long) RT_TURN_MS) {
        last += RT_TURN_MS;
        /* if we fell more than a few turns behind (debugger pause, heavy
           redraw, alt-tab, ...) resync instead of bursting to catch up */
        if ((unsigned long) (now - last) > (unsigned long) (RT_TURN_MS * 4))
            last = now;
        return TRUE;
    }
    return FALSE;
}

/*
 * Console command input for real-time play.  Buffer any pending keystrokes
 * without blocking, wait (yielding the CPU) until the world clock says it is
 * time for the next turn, then report whether the hero has a queued command
 * to act on (TRUE) or should simply wait this turn (FALSE).  Either way
 * exactly one turn's worth of real time elapses, so the world -- monsters,
 * timeouts, and all -- advances at a constant rate.
 *
 * This is console-only (kbhit/pgetchar); the win32 GUI build does the
 * equivalent against its own event queue in mswproc.c.
 */
staticfn boolean
rt_poll_input_timed(void)
{
#if defined(MICRO) || defined(WIN32CON)
    char last = 0;
    boolean got = FALSE;

    /* Collect keystrokes but keep only the most recent one, so a backlog can't
       accumulate during the wait and then replay ("pre-input"): each tick acts
       on the player's live intent rather than a stale queue. */
    while (kbhit()) {
        last = pgetchar();
        got = TRUE;
    }
    while (!rt_world_tick_ready()) {
        Delay(RT_POLL_MS);
        while (kbhit()) {
            last = pgetchar();
            got = TRUE;
        }
    }
    if (got) {
        cmdq_add_key(CQ_CANNED, last);
        return TRUE;
    }
    return cmdq_peek(CQ_CANNED) ? TRUE : FALSE;
#else
    /* no non-blocking console input here; fall back to the normal command */
    return TRUE;
#endif
}
#endif /* REALTIME_PROTO */

/**
 * @brief Advance the game by as much as can happen before the player is asked
 *        to act again.
 *
 * Movement is an allowance, not a schedule: everyone accumulates it and spends
 * it, and a turn ends only when nobody can act. So this runs the monsters until
 * either one of them yields to the hero or they all run dry, and only in the
 * latter case does a turn actually pass and the per-turn machinery run.
 *
 * @note The once-per-turn block is deliberately long: hunger, healing, timed
 *       effects, prayer, polymorph, shape-shift, level noises and the endgame
 *       countdown all mean "a turn went by" and have nowhere else to live.
 * @warning Order within a turn is load-bearing. Encumbrance is recomputed
 *          after monsters move because their actions can change what the hero
 *          is carrying, and again before the hero acts so the player is told
 *          immediately.
 */
/**
 * @brief 플레이어에게 다시 행동을 묻기 전까지 일어날 수 있는 만큼 게임을
 *        진행시킨다.
 *
 * 이동력은 일정표가 아니라 배당이다. 모두가 그것을 쌓고 소비하며, 아무도 행동할
 * 수 없을 때에야 턴이 끝난다. 그래서 여기서는 몬스터 중 누군가가 영웅에게
 * 차례를 넘기거나 전부 이동력을 소진할 때까지 몬스터를 움직이고, 후자의 경우에만
 * 실제로 턴이 지나가며 턴 단위 처리가 실행된다.
 *
 * @note 턴당 1회 블록이 긴 것은 의도된 것이다. 허기, 회복, 시한 효과, 기도,
 *       변신, 형태 변화, 레벨의 소리, 엔드게임 카운트다운이 모두 "한 턴이
 *       지났다"를 뜻하며 달리 있을 곳이 없다.
 * @warning 턴 안의 순서가 동작을 좌우한다. 하중은 몬스터가 움직인 뒤에 다시
 *          계산되는데, 그들의 행동이 영웅이 지닌 것을 바꿀 수 있기 때문이다.
 *          그리고 영웅이 행동하기 전에 또 한 번 계산해, 플레이어에게 즉시
 *          알린다.
 */
void
moveloop_core(void)
{
    boolean monscanmove = FALSE;

#ifdef SAFERHANGUP
    if (program_state.done_hup)
        end_of_input();
#endif
    get_nh_event();
#ifdef POSITIONBAR
    do_positionbar();
#endif
    if (iflags.pending_customizations)
        maybe_shuffle_customizations();

    dobjsfree();

    if (svc.context.bypasses)
        clear_bypasses();

    if (iflags.sanity_check || iflags.debug_fuzzer)
        sanity_check();

    if (svc.context.resume_wish)
        makewish(); /* clears resume_wish */

    if (svc.context.move) {
        /* actual time passed */
        u.umovement -= NORMAL_SPEED;

        do { /* hero can't move this turn loop */
            encumber_msg();

            svc.context.mon_moving = TRUE;
            do {
                monscanmove = movemon();
                if (u.umovement >= NORMAL_SPEED)
                    break; /* it's now your turn */
            } while (monscanmove);
            svc.context.mon_moving = FALSE;

            /* this needs to be after the monster movement loop in
               case monster actions affected burden, e.g. rehumanize */
            mvl_wtcap = near_capacity();

            if (!monscanmove && u.umovement < NORMAL_SPEED) {
                /* both hero and monsters are out of steam this round */
                struct monst *mtmp;

                /* set up for a new turn */
                gw.were_changes = 0L;
                mcalcdistress(); /* adjust monsters' trap, blind, etc */

                /* reallocate movement rations to monsters; don't need
                   to skip dead monsters here because they will have
                   been purged at end of their previous round of moving */
                for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
                    mtmp->movement += mcalcmove(mtmp, TRUE);

                /* occasionally add another monster; since this takes
                   place after movement has been allotted, the new
                   monster effectively loses its first turn */
                maybe_generate_rnd_mon();

                u_calc_moveamt(mvl_wtcap);
                settrack();

                svm.moves++;
                /*
                 * Never allow 'moves' to grow big enough to wrap.
                 * We don't care what the maximum possible 'long int'
                 * is for the current configuration, we want a value
                 * that is the same for all viable configurations.
                 * When imposing the limit, use a mystic decimal value
                 * instead of a magic binary one such as 0x7fffffffL.
                 */
                if (svm.moves >= 1000000000L) {
                    display_nhwindow(WIN_MESSAGE, TRUE);
                    urgent_pline("The dungeon capitulates.");
                    done(ESCAPED);
                }
                /* 'moves' is misnamed; it represents turns; hero_seq is
                   a value that is distinct every time the hero moves */
                gh.hero_seq = svm.moves << 3;

                if (flags.time && !svc.context.run)
                    disp.time_botl = TRUE; /* 'moves' just changed */

                /********************************/
                /* once-per-turn things go here */
                /********************************/

                l_nhcore_call(NHCORE_MOVELOOP_TURN);

                if (Glib)
                    glibr();
                nh_timeout();
                run_regions();

                if (u.ublesscnt)
                    u.ublesscnt--;

                /* One possible result of prayer is healing.  Whether or
                 * not you get healed depends on your current hit points.
                 * If you are allowed to regenerate during the prayer,
                 * the end-of-prayer calculation messes up on this.
                 * Another possible result is rehumanization, which
                 * requires that encumbrance and movement rate be
                 * recalculated.
                 */
                if (u.uinvulnerable) {
                    /* for the moment at least, you're in tiptop shape */
                    mvl_wtcap = UNENCUMBERED;
                } else if (!Upolyd ? (u.uhp < u.uhpmax)
                           : (u.mh < u.mhmax
                              || gy.youmonst.data->mlet == S_EEL)) {
                    /* maybe heal */
                    regen_hp(mvl_wtcap);
                }

                /* moving around while encumbered is hard work */
                if (mvl_wtcap > MOD_ENCUMBER && u.umoved) {
                    if (!(mvl_wtcap < EXT_ENCUMBER ? svm.moves % 30
                          : svm.moves % 10)) {
                        overexert_hp();
                    }
                }

                regen_pw(mvl_wtcap);

                if (!u.uinvulnerable) {
                    if (Teleportation && !rn2(85)) {
                        coordxy old_ux = u.ux, old_uy = u.uy;

                        tele();
                        if (u.ux != old_ux || u.uy != old_uy) {
                            if (!next_to_u()) {
                                check_leash(old_ux, old_uy);
                            }
                            /* clear doagain keystrokes */
                            cmdq_clear(CQ_CANNED);
                            cmdq_clear(CQ_REPEAT);
                        }
                    }
                    /* delayed change may not be valid anymore */
                    if ((mvl_change == 1 && !Polymorph)
                        || (mvl_change == 2 && u.ulycn == NON_PM))
                        mvl_change = 0;
                    if (Polymorph && !rn2(100))
                        mvl_change = 1;
                    else if (ismnum(u.ulycn) && !Upolyd
                             && !rn2(80 - (20 * night())))
                        mvl_change = 2;
                    if (mvl_change && !Unchanging) {
                        if (gm.multi >= 0) {
                            stop_occupation();
                            if (mvl_change == 1)
                                polyself(POLY_NOFLAGS);
                            else
                                you_were();
                            mvl_change = 0;
                        }
                    }
                }

                if (Searching && !svl.level.flags.noautosearch
                    && gm.multi >= 0)
                    (void) dosearch0(1);
                if (Warning)
                    warnreveal();
                if (gw.were_changes) {
                    /* update innate intrinsics (mainly Drain_resistance) */
                    set_uasmon();
                }
                mkot_trap_warn();
                dosounds();
                do_storms();
                gethungry();
                age_spells();
                exerchk();
                invault();
                if (u.uhave.amulet)
                    amulet();
                if (!rn2(40 + (int) (ACURR(A_DEX) * 3)))
                    u_wipe_engr(rnd(3));
                if (u.uevent.udemigod && !u.uinvulnerable) {
                    if (u.udg_cnt)
                        u.udg_cnt--;
                    if (!u.udg_cnt) {
                        intervene();
                        u.udg_cnt = rn1(200, 50);
                    }
                }
/* XXX This should be recoded to use something like regions - a list of
 * things that are active and need to be handled that is dynamically
 * maintained and not a list of special cases. */
                /* vision will be updated as bubbles move */
                if (Is_waterlevel(&u.uz) || Is_airlevel(&u.uz))
                    movebubbles();
                else if (svl.level.flags.fumaroles)
                    fumaroles();

                /* when immobile, count is in turns */
                if (gm.multi < 0) {
                    runmode_delay_output();
                    if (++gm.multi == 0) { /* finished yet? */
                        unmul((char *) 0);
                        /* if unmul caused a level change, take it now */
                        if (u.utotype)
                            deferred_goto();
                    }
                }
            }
        } while (u.umovement < NORMAL_SPEED); /* hero can't move */

        /******************************************/
        /* once-per-hero-took-time things go here */
        /******************************************/

        gh.hero_seq++; /* moves*8 + n for n == 1..7 */

        /* although we checked for encumbrance above, we need to
           check again for message purposes, as the weight of
           inventory may have changed in, e.g., nh_timeout(); we do
           need two checks here so that the player gets feedback
           immediately if their own action encumbered them */
        encumber_msg();

#ifdef STATUS_HILITES
        if (iflags.hilite_delta)
            status_eval_next_unhilite();
#endif
        if (svm.moves >= svc.context.seer_turn) {
            if ((u.uhave.amulet || Clairvoyant) && !In_endgame(&u.uz)
                && !BClairvoyant)
                do_vicinity_map((struct obj *) 0);
            /* we maintain this counter even when clairvoyance isn't
               taking place; on average, go again 30 turns from now */
            svc.context.seer_turn = svm.moves + (long) rn1(31, 15); /*15..45*/
            /* [it used to be that on every 15th turn, there was a 50%
               chance of farsight, so it could happen as often as every
               15 turns or theoretically never happen at all; but when
               a fast hero got multiple moves on that 15th turn, it
               could actually happen more than once on the same turn!] */
        }
        /* [fast hero who gets multiple moves per turn ends up sinking
           multiple times per turn; is that what we really want?] */
        if (u.utrap && u.utraptype == TT_LAVA)
            sink_into_lava();
        /* when/if hero escapes from lava, he can't just stay there */
        else if (!u.umoved)
            (void) pooleffects(FALSE);

        /* vision while buried or underwater is updated here */
        if (Underwater)
            under_water(0);
        else if (u.uburied)
            under_ground(0);

        see_nearby_monsters();
    } /* actual time passed */

    /****************************************/
    /* once-per-player-input things go here */
    /****************************************/

    clear_splitobjs();

    /* the Amulet of Yendor gives a wish when initially picked up */
    if (u.uhave.amulet && !u.uevent.amulet_wish) {
        u.uevent.amulet_wish = 1;
        display_nhwindow(WIN_MESSAGE, TRUE);
        urgent_pline("The Amulet is bestowing a wish upon you!");
        makewish();
    }

    find_ac();
    if (!svc.context.mv || Blind) {
        /* redo monsters if hallu or wearing a helm of telepathy */
        if (Hallucination) { /* update screen randomly */
            see_monsters();
            see_objects();
            see_traps();
            if (u.uswallow)
                swallowed(0);
        } else if (Unblind_telepat || Warning || Warn_of_mon
                   /* this is needed for the case where you saw a monster
                      due to being next to it while it's in a gas cloud
                      and then you moved away; it should no longer be seen
                      when that happens, even if it hasn't moved */
                   || any_visible_region()) { /* TODO: optimize this */
            see_monsters();
        }
        if (gv.vision_full_recalc)
            vision_recalc(0); /* vision! */
    }
    if (disp.botl || disp.botlx) {
        bot();
        curs_on_u();
    } else if (disp.time_botl) {
        timebot();
        curs_on_u();
    }

    m_everyturn_effect(&gy.youmonst);

    svc.context.move = 1;

    if (gm.multi >= 0 && go.occupation) {
#if defined(MICRO) || defined(WIN32CON)
        mvl_abort_lev = 0;
        if (kbhit()) {
            char ch;

            if ((ch = pgetchar()) == ABORT)
                mvl_abort_lev++;
            else
                cmdq_add_key(CQ_CANNED, ch);
        }
        if (!mvl_abort_lev && (*go.occupation)() == 0)
#else
            if ((*go.occupation)() == 0)
#endif
                go.occupation = 0;
        if (
#if defined(MICRO) || defined(WIN32)
            mvl_abort_lev ||
#endif
            monster_nearby()) {
            stop_occupation();
            reset_eat();
        }
        runmode_delay_output();
        return;
    }

    u.umoved = FALSE;

    if (gm.multi > 0) {
        lookaround();
        runmode_delay_output();
        if (!gm.multi) {
            /* lookaround may clear multi */
            svc.context.move = 0;
            return;
        }
        if (svc.context.mv) {
            if (gm.multi < COLNO && !--gm.multi)
                end_running(TRUE);
            domove();
        } else {
            --gm.multi;
            nhassert(gc.command_count != 0);
            rhack(gc.cmd_key);
        }
    } else if (gm.multi == 0) {
#ifdef MAIL
        ckmailstatus();
#endif
#ifdef REALTIME_PROTO
        /* real-time: paces to the world clock, then acts on a buffered
           command if one is queued; otherwise the hero waits this turn while
           svc.context.move == 1 keeps monsters/timeouts advancing */
        if (rt_poll_input_timed())
            rhack(0);
#else
        rhack(0);
#endif
    }
    if (u.utotype)       /* change dungeon level */
        deferred_goto(); /* after rhack() */

    if (gv.vision_full_recalc)
        vision_recalc(0); /* vision! */
#ifdef CLIPPING
    /* after rhack() and vision_recalc() so that the map is redrawn
       once with correct vision data, not twice (overshoot+correct) */
    cliparound(u.ux, u.uy);
#endif
    /* when running in non-tport mode, this gets done through domove() */
    if ((!svc.context.run || flags.runmode == RUN_TPORT)
        && (gm.multi && (!svc.context.travel ? !(gm.multi % 7)
                        : !(svm.moves % 7L)))) {
        if (flags.time && svc.context.run)
            disp.botl = TRUE;
        /* [should this be flush_screen() instead?] */
        display_nhwindow(WIN_MAP, FALSE);
    }

    if (gl.luacore && nhcb_counts[NHCB_END_TURN]) {
        lua_getglobal(gl.luacore, "nh_callback_run");
        lua_pushstring(gl.luacore, nhcb_name[NHCB_END_TURN]);
        nhl_pcall_handle(gl.luacore, 1, 0, "moveloop_core", NHLpa_panic);
        lua_settop(gl.luacore, 0);
    }
}

staticfn void
maybe_do_tutorial(void)
{
    s_level *sp = find_level("tut-1");

    if (!sp)
        return;

    if (ask_do_tutorial()) {
        assign_level(&u.ucamefrom, &u.uz);
        iflags.nofollowers = TRUE;
        schedule_goto(&sp->dlevel, UTOTYPE_NONE,
                      "Entering the tutorial.", (char *) 0);
        deferred_goto();
        vision_recalc(0);
        docrt();
        iflags.nofollowers = FALSE;
    } else {
        /* no tutorial, so okay to process mention_decor now */
        rcfile_only_this_option(opt_mention_decor);
    }
}

/**
 * @brief Run the game until it ends.
 * @param[in] resuming FALSE for a new game, TRUE when a save was restored.
 * @warning Does not return. The game is left by longjmp or by exiting, not by
 *          this loop finishing.
 */
/**
 * @brief 게임이 끝날 때까지 돌린다.
 * @param[in] resuming 새 게임이면 FALSE, 저장을 복원한 것이면 TRUE.
 * @warning 반환하지 않는다. 게임에서 빠져나가는 것은 longjmp 나 종료를 통해서지,
 *          이 루프가 끝나서가 아니다.
 */
void
moveloop(boolean resuming)
{
    moveloop_preamble(resuming);

    if (!resuming)
        maybe_do_tutorial();

    /* process one deferred option post-tutorial */
    rcfile_only_this_option(opt_mention_decor);

    for (;;) {
        moveloop_core();
    }
}

/**
 * @brief Recover a little spell energy, if this turn is one of the ones that
 *        does.
 * @param[in] wtcap Current encumbrance level.
 * @note The interval shortens with experience and is kinder to wizards; being
 *       heavily burdened stops natural recovery entirely, though the intrinsic
 *       still works.
 */
/**
 * @brief 이번 턴이 회복이 일어나는 턴이라면 마력을 조금 회복한다.
 * @param[in] wtcap 현재 하중 단계.
 * @note 회복 간격은 경험이 쌓일수록 짧아지며 마법사에게 더 관대하다. 하중이
 *       심하면 자연 회복은 완전히 멈추지만, 고유 능력에 의한 회복은 계속된다.
 */
staticfn void
regen_pw(int wtcap)
{
    if (u.uen < u.uenmax
        && ((wtcap < MOD_ENCUMBER
             && (!(svm.moves % ((MAXULEV + 8 - u.ulevel)
                              * (Role_if(PM_WIZARD) ? 3 : 4)
                              / 6)))) || Energy_regeneration)) {
        int upper = (int) (ACURR(A_WIS) + ACURR(A_INT)) / 15 + 1;

        if (EMagical_breathing)
            upper += 2;

        u.uen += rn1(upper, 1);
        if (u.uen > u.uenmax)
            u.uen = u.uenmax;
        disp.botl = TRUE;
        if (u.uen == u.uenmax)
            interrupt_multi("You feel full of energy.");
    }
}

#define U_CAN_REGEN() (Regeneration || (Sleepy && u.usleep))

/* maybe recover some lost health (or lose some when an eel out of water) */
/**
 * @brief Recover a little health -- or lose some, for an eel out of water.
 * @param[in] wtcap Current encumbrance level.
 * @note Recovery is slower the healthier and the more experienced the hero is,
 *       so it matters most when it is most needed.
 * @warning A polymorphed hero heals @c u.mh, the form's health, and not
 *          @c u.uhp; the two are deliberately kept apart so that surviving as
 *          a monster does not quietly repair the hero underneath.
 */
/**
 * @brief 체력을 조금 회복한다. 물 밖의 뱀장어라면 오히려 잃는다.
 * @param[in] wtcap 현재 하중 단계.
 * @note 영웅이 건강할수록, 경험이 많을수록 회복이 느리다. 그래서 가장 필요할 때
 *       가장 크게 작용한다.
 * @warning 변신 중인 영웅은 @c u.uhp 가 아니라 그 형태의 체력인 @c u.mh 를
 *          회복한다. 둘을 의도적으로 분리해 두어, 몬스터로 버티는 동안 밑에 있는
 *          영웅이 조용히 회복되지 않게 한다.
 */
staticfn void
regen_hp(int wtcap)
{
    int heal = 0;
    boolean reached_full = FALSE,
            encumbrance_ok = (wtcap < MOD_ENCUMBER || !u.umoved);

    if (Upolyd) {
        if (u.mh < 1) { /* shouldn't happen... */
            rehumanize();
        } else if (gy.youmonst.data->mlet == S_EEL
                   && !is_pool(u.ux, u.uy) && !Is_waterlevel(&u.uz)
                   && !Breathless) {
            /* eel out of water loses hp, similar to monster eels;
               as hp gets lower, rate of further loss slows down */
            if (u.mh > 1 && !Regeneration && rn2(u.mh) > rn2(8)
                && (!Half_physical_damage || !(svm.moves % 2L)))
                heal = -1;
        } else if (u.mh < u.mhmax) {
            if (U_CAN_REGEN() || (encumbrance_ok && !(svm.moves % 20L)))
                heal = 1;
        }
        if (heal) {
            disp.botl = TRUE;
            u.mh += heal;
            reached_full = (u.mh == u.mhmax);
        }

    /* !Upolyd */
    } else {
        /* [when this code was in-line within moveloop(), there was
           no !Upolyd check here, so poly'd hero recovered lost u.uhp
           once u.mh reached u.mhmax; that may have been convenient
           for the player, but it didn't make sense for gameplay...] */
        if (u.uhp < u.uhpmax && (encumbrance_ok || U_CAN_REGEN())) {
            heal = (u.ulevel + (int)ACURR(A_CON)) > rn2(100);

            if (U_CAN_REGEN())
                heal += 1;
            if (Sleepy && u.usleep)
                heal++;

            if (heal) {
                disp.botl = TRUE;
                u.uhp += heal;
                if (u.uhp > u.uhpmax)
                    u.uhp = u.uhpmax;
                /* stop voluntary multi-turn activity if now fully healed */
                reached_full = (u.uhp == u.uhpmax);
            }
        }
    }

    if (reached_full)
        interrupt_multi("You are in full health.");
}

#undef U_CAN_REGEN

/**
 * @brief Abandon a multi-turn activity the hero was in the middle of.
 * @note Digging, eating, and the like are represented as an occupation that
 *       resumes each turn; interrupting one has to clear the repeat count and
 *       the "you continue" state as well, or the activity would silently pick
 *       itself back up.
 */
/**
 * @brief 영웅이 진행 중이던 여러 턴짜리 활동을 중단한다.
 * @note 굴착이나 식사 같은 활동은 매 턴 재개되는 occupation 으로 표현된다.
 *       중단하려면 반복 횟수와 "계속한다" 상태까지 함께 지워야 하며, 그러지
 *       않으면 활동이 조용히 다시 이어진다.
 */
void
stop_occupation(void)
{
    if (go.occupation) {
        if (!maybe_finished_meal(TRUE))
            You("stop %s.", go.occtxt);
        go.occupation = (int (*)(void)) 0;
        disp.botl = TRUE; /* in case u.uhs changed */
        nomul(0);
    } else if (gm.multi >= 0) {
        nomul(0);
    }
    cmdq_clear(CQ_CANNED);
}

void
init_sound_disp_gamewindows(void)
{
    int menu_behavior = MENU_BEHAVE_STANDARD;

    activate_chosen_soundlib();

    if (iflags.wc_splash_screen && !flags.randomall) {
        SoundAchievement(0, sa2_splashscreen, 0);
        /* ToDo: new splash screen invocation will go here */
    } else {
        SoundAchievement(0, sa2_newgame_nosplash, 0);
    }

#ifdef CHANGE_COLOR
    /* init_nhwindows() has already been called, so before
       creating the windows, check to see if there are any
       palette entries to alter */
    change_palette();
#endif

    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    if (VIA_WINDOWPORT()) {
        status_initialize(FALSE);
    } else {
        WIN_STATUS = create_nhwindow(NHW_STATUS);
    }
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);
    if (WIN_INVEN != WIN_ERR)
        adjust_menu_promptstyle(WIN_INVEN, &iflags.menu_headings);

#ifdef TTY_PERM_INVENT
    if (WINDOWPORT(tty) && WIN_INVEN != WIN_ERR) {
        menu_behavior = MENU_BEHAVE_PERMINV;
        prepare_perminvent(WIN_INVEN);
    }
#endif
    /* in case of early quit where WIN_INVEN could be destroyed before
       ever having been used, use it here to pacify the Qt interface */
    start_menu(WIN_INVEN, menu_behavior), end_menu(WIN_INVEN, (char *) 0);

#ifdef MAC68K
    /* This _is_ the right place for this - maybe we will
     * have to split init_sound_disp_gamewindows into
     * create_gamewindows and show_gamewindows to get rid of this ifdef...
     */
    if (!strcmp(windowprocs.name, "mac"))
        SanePositions();
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
#ifndef STATUS_HILITES
    display_nhwindow(WIN_STATUS, FALSE);
#endif
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
#ifdef TTY_PERM_INVENT
    if (iflags.perm_invent_pending)
        check_perm_invent_again();
#endif
}

/**
 * @brief Build a fresh game: the hero, their possessions, and the first level.
 * @note Everything that only ever happens once lives here -- naming, the
 *       starting inventory, the first pet, the initial dungeon layout -- which
 *       is why restoring a save deliberately does not come through this path.
 */
/**
 * @brief 새 게임을 구성한다. 영웅과 그 소지품, 그리고 첫 레벨.
 * @note 오직 한 번만 일어나는 모든 것이 여기에 있다. 이름 짓기, 시작 소지품,
 *       첫 애완동물, 최초 던전 배치 등이다. 저장을 복원할 때 이 경로를 의도적으로
 *       거치지 않는 이유가 그것이다.
 */
void
newgame(void)
{
    int i;

#ifdef SYSCF
    time_t last_reroll_time;
    time_t cur_reroll_time;
    int rerolls_this_second = 0;
# if defined(BSD) && !defined(POSIX_TYPES)
#  define GET_REROLL_TIME(t) (void) time((long *) t);
# else
#  define GET_REROLL_TIME(t) (void) time(t);
# endif
#endif /* defined(SYSCF) */

    /* make sure welcome messages are given before noticing monsters */
    notice_mon_off();
    disp.botlx = TRUE;
    svc.context.ident = 2;  /* id 1 is reserved for gy.youmonst */
    svc.context.warnlevel = 1;
    svc.context.next_attrib_check = 600L; /* arbitrary first setting */
    svc.context.tribute.enabled = TRUE;   /* turn on 3.6 tributes    */
    svc.context.tribute.tributesz = sizeof(struct tribute_info);
    get_nhuuid();

    for (i = LOW_PM; i < NUMMONS; i++)
        svm.mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

    init_objects(); /* must be before u_init() */

    flags.pantheon = -1; /* role_init() will reset this */
    role_init();         /* must be before init_dungeons(), u_init(),
                          * and init_artifacts() */

    init_dungeons();  /* must be before u_init() to avoid rndmonst()
                       * creating odd monsters for any tins and eggs
                       * in hero's initial inventory */
    init_artifacts(); /* before u_init() in case $WIZKIT specifies
                       * any artifacts */
    u_init_misc();

    l_nhcore_init();  /* create a Lua state that lasts until end of game */
    reset_glyphmap(gm_newgame);
#ifndef NO_SIGNAL
    (void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
    if (iflags.news)
        display_file(NEWS, FALSE);
#endif
    /* quest_init();  --  Now part of role_init() */

    mklev();
    u_on_upstairs();
    vision_reset();          /* set up internals for level (after mklev) */
    check_special_room(FALSE);

    if (MON_AT(u.ux, u.uy))
        mnexto(m_at(u.ux, u.uy), RLOC_NOMSG);
    (void) makedog();

    u_init_inventory_attrs();
    docrt();
    flush_screen(1);
    bot();

#ifdef SYSCF
    GET_REROLL_TIME(&last_reroll_time);
#endif

    while (u.uroleplay.reroll && reroll_menu()) {
#ifdef SYSCF
        if (sysopt.maxrerollrate > 0) {
        check_reroll_time:
            GET_REROLL_TIME(&cur_reroll_time);

            if (last_reroll_time != cur_reroll_time) {
                last_reroll_time = cur_reroll_time;
                rerolls_this_second = 1;
            } else {
                if (rerolls_this_second >= sysopt.maxrerollrate) {
                    if (!paranoid_query(TRUE, "Continue rerolling?"))
                        break;
                    goto check_reroll_time;
                }
                ++rerolls_this_second;
            }
        }
#endif

        ++u.uroleplay.numrerolls;
        u_init_inventory_attrs();
        bot();
    }
    u_init_skills_discoveries();

    if (wizard) {
        read_wizkit();
        obj_delivery(FALSE); /* finish wizkit */
    }

    if (flags.legacy) {
        com_pager(u.uroleplay.pauper ? "pauper_legacy" : "legacy");
    }

    urealtime.realtime = 0L;
    urealtime.start_timing = getnow();
#ifdef INSURANCE
    save_currentstate();
#endif
    program_state.something_worth_saving++; /* useful data now exists */

    /* Success! */
    welcome(TRUE);
    notice_mon_on(); /* now we can notice monsters */
    if (a11y.glyph_updates)
        (void) dolookaround();
    else
        notice_all_mons(TRUE);
    return;
}

/* show "welcome [back] to NetHack" message at program startup */
void
welcome(boolean new_game) /* false => restoring an old game */
{
    char buf[BUFSZ];
    boolean currentgend = Upolyd ? u.mfemale : flags.female,
            adrift = (u.ualign.type != u.ualignbase[A_CURRENT]);

    l_nhcore_call(new_game ? NHCORE_START_NEW_GAME : NHCORE_RESTORE_OLD_GAME);

    /* skip "welcome back" if restoring a doomed character */
    if (!new_game && Upolyd && ugenocided()) {
        /* death via self-genocide is pending */
        pline("You're back, but you still feel %s inside.", udeadinside());
        return;
    }

    if (Hallucination)
        pline("NetHack is filmed in front of an undead studio audience.");

    /*
     * The "welcome back" message always describes your innate form
     * even when polymorphed or wearing a helm of opposite alignment.
     * Alignment is shown unconditionally for new games; for restores
     * it's only shown if it has changed from its original value.
     * Sex is shown for new games except when it is redundant; for
     * restores it's only shown if different from its original value.
     */
    *buf = '\0';
#if 0
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT])
        Sprintf(eos(buf), " %s", align_str(u.ualignbase[A_ORIGINAL]));
#else
    /*
     * 2026-04-24
     * GitHub issue https://github.com/NetHack/NetHack/issues/537
     * "Judging by the comment above, it should display your new alignment
     *  if it was changed, so align_str(u.ualignbase[A_CURRENT]) would
     *  probably be more appropriate. This won't affect the new game message."
     *
     * That is followed by a suggestion to revisit the matter (paraphrased):
     * "That's actually intentional; the comment oversimplifies.
     *  When it was implemented, it may have been the only way to tell that
     *  you had converted alignment. Now ^X mentions your starting alignment
     *  if base alignment has been changed, so revisiting this welcome back
     *  message."
     */
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT] || adrift)
        Sprintf(eos(buf), " %s%s",
                adrift ? "adrift " : "",
                adrift ? align_str(u.ualign.type)
                       : align_str(u.ualignbase[A_CURRENT]));
#endif
    if (!gu.urole.name.f
        && (new_game
            ? (gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            : currentgend != flags.initgend))
        Sprintf(eos(buf), " %s", genders[currentgend].adj);
    Sprintf(eos(buf), " %s %s", gu.urace.adj,
            (currentgend && gu.urole.name.f) ? gu.urole.name.f
                                             : gu.urole.name.m);

    pline(new_game ? "%s %s, welcome to NetHack!  You are a%s."
                   : "%s %s, the%s, welcome back to NetHack!",
          Hello((struct monst *) 0), svp.plname, buf);

    if (new_game) {
        /* guarantee that 'major' event category is never empty */
        livelog_printf(LL_ACHIEVE, "%s the%s entered the dungeon",
                       svp.plname, buf);
    } else {
        /* if restoring in Gehennom, give same hot/smoky message as when
           first entering it */
        hellish_smoke_mesg();
        /* remind player of the level annotation, like in goto_level() */
        print_level_annotation();
    }
}

#ifdef POSITIONBAR
staticfn void
do_positionbar(void)
{
    /* FIXME: this will break if any coordinate is too big for (char);
       the sys/msdos/vid*.c code uses (unsigned char) which is less
       vulnerable but not guaranteed to be able to hold coordxy values;
       also, there doesn't appear to be any need for this to be static,
       nor to contain pairs of (> or <) and x; it could just be a full
       line of spaces and > or < characters with update_positionbar()
       revised to reconstruct the x values for non-space characters */
    static char pbar[COLNO];
    char *p;
    stairway *stway;
    coordxy x, y;
    int glyph, symbol;

    p = pbar;
    /* TODO: use the same method as getpos() so objects don't cover stairs */
    /* FIXME: traversing 'stairs' list ignores mimics that pose as stairs */
    for (stway = gs.stairs; stway; stway = stway->next) {
        x = stway->sx;
        y = stway->sy;
        glyph = levl[x][y].glyph;
        symbol = glyph_to_cmap(glyph);

        if (is_cmap_stairs(symbol)) {
            *p++ = (stway->up ? '<' : '>');
            *p++ = (char) x;
        }
     }

    /* hero location */
    if (u.ux) {
        *p++ = '@';
        *p++ = u.ux;
    }
    /* fence post */
    *p = 0;

    update_positionbar(pbar);
}
#endif

staticfn void
interrupt_multi(const char *msg)
{
    if (gm.multi > 0 && !svc.context.travel && !svc.context.run) {
        nomul(0);
        if (flags.verbose && msg)
            Norep("%s", msg);
    }
}

/* convert from time_t to number of seconds */
long
timet_to_seconds(time_t ttim)
{
    /* for Unix-based and Posix-compliant systems, a cast to 'long' would
       suffice but the C Standard doesn't require time_t to be that simple */
    return timet_delta(ttim, (time_t) 0);
}

/* calculate the difference in seconds between two time_t values */
long
timet_delta(time_t etim, time_t stim) /* end and start times */
{
    /* difftime() is a STDC routine which returns the number of seconds
       between two time_t values as a 'double' */
    return (long) difftime(etim, stim);
}

/*allmain.c*/
