/* NetHack 5.0	quest.h	$NHDT-Date: 1781973086 2026/06/20 16:31:26 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.16 $ */
/* Copyright (c) Mike Stephenson 1991.                            */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file quest.h
 * @brief The record of how the hero's quest has gone.
 *
 * The quest is a sequence of encounters that must be remembered: whether the
 * leader has been met, whether they judged the hero ready, whether the
 * assignment was given, the nemesis met, the artifact taken and offered. Each is
 * a fact the game needs later in order not to repeat itself or contradict what
 * already happened.
 *
 * @note The leader's, nemesis's and deity's genders are settled at game start
 *       rather than at creation, so quest text can use pronouns before those
 *       creatures exist.
 * @note The leader is tracked by identity as well as by a death flag, because a
 *       polymorphed or revived leader is still the same individual.
 */

/**
 * @file quest.h
 * @brief 영웅의 퀘스트가 어떻게 진행되었는지에 대한 기록.
 *
 * 퀘스트는 기억해 두어야 하는 만남의 연속이다. 지도자를 만났는지, 그가 영웅이 준비되었다고
 * 판단했는지, 임무가 주어졌는지, 숙적을 만났는지, 아티팩트를 얻어 바쳤는지다. 각각은
 * 게임이 되풀이하지 않고 이미 일어난 일과 모순되지 않기 위해 나중에 필요로 하는 사실이다.
 *
 * @note 지도자와 숙적, 신의 성별은 생성 시점이 아니라 게임 시작 시 정해진다. 그래야
 *       그 생물들이 존재하기 전에도 퀘스트 문구가 대명사를 쓸 수 있다.
 * @note 지도자는 사망 플래그와 함께 정체로도 추적된다. 변신했거나 되살아난 지도자도
 *       여전히 같은 개체이기 때문이다.
 */

#ifndef QUEST_H
#define QUEST_H

/**
 * @brief What has already happened in the quest.
 * @note Packed as bitfields because it is saved; several counters are
 *       deliberately narrow, since the events they count cannot recur often.
 * @warning @c cheater records that the game detected the quest being
 *          circumvented, and outlives the attempt.
 */
/**
 * @brief 퀘스트에서 이미 일어난 일.
 * @note 저장되기 때문에 비트필드로 압축되어 있다. 몇몇 계수기는 의도적으로 폭이 좁다.
 *       그것들이 세는 사건은 자주 되풀이될 수 없기 때문이다.
 * @warning @c cheater 는 퀘스트를 우회하려 한 것이 감지되었음을 기록하며, 그 시도보다
 *          오래 남는다.
 */
struct q_score {              /* Quest "scorecard" */
    Bitfield(first_start, 1); /* only set the first time */
    Bitfield(met_leader, 1);  /* has met the leader */
    Bitfield(not_ready, 3);   /* rejected due to alignment, etc. */
    Bitfield(pissed_off, 1);  /* got the leader angry */
    Bitfield(got_quest, 1);   /* got the quest assignment */
    Bitfield(killed_leader, 1); /* killed the quest leader */

    Bitfield(first_locate, 1); /* only set the first time */
    Bitfield(met_intermed, 1); /* used if the locate is a person */
    Bitfield(got_final, 1);    /* got the final quest assignment */

    Bitfield(made_goal, 3);      /* # of times on goal level */
    Bitfield(met_nemesis, 1);    /* has met the nemesis before */
    Bitfield(killed_nemesis, 1); /* set when the nemesis is killed */
    Bitfield(in_battle, 1);      /* set when nemesis fighting you */

    Bitfield(cheater, 1);          /* set if cheating detected */
    Bitfield(touched_artifact, 1); /* for a special message */
    Bitfield(offered_artifact, 1); /* offered to leader */
    Bitfield(got_thanks, 1);       /* final message from leader */

    /* used by questpgr code when messages want to use pronouns
       (set up at game start instead of waiting until monster creation;
       1 bit each would suffice--nobody involved is actually neuter) */
    Bitfield(ldrgend, 2); /* leader's gender: 0=male, 1=female, 2=neuter */
    Bitfield(nemgend, 2); /* nemesis's gender */
    Bitfield(godgend, 2); /* deity's gender */

    /* keep track of leader presence/absence even if leader is
       polymorphed, raised from dead, etc */
    Bitfield(leader_is_dead, 1);
    unsigned leader_m_id;
};

#define MIN_QUEST_ALIGN 20 /* at least this align.record to start */
/* note: align 20 matches "pious" as reported by enlightenment (cmd.c) */
#define MIN_QUEST_LEVEL 14 /* at least this u.ulevel to start */
/* note: exp.lev. 14 is threshold level for 5th rank (class title, role.c) */

#endif /* QUEST_H */
