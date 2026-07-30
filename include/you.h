/* NetHack 5.0	you.h	$NHDT-Date: 1781973093 2026/06/20 16:31:33 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.89 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2016. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

#include "attrib.h"
#include "monst.h"
#ifndef PROP_H
#include "prop.h" /* (needed here for util/makedefs.c) */
#endif
/**
 * @file you.h
 * @brief The hero: who they are, what has happened to them, and what they are now.
 *
 * Two different things live here, and telling them apart makes the file much easier to read.
 *
 * The first half describes the kinds of hero the game offers. A role, a race, a gender and an
 * alignment are each a table entry, holding not statistics so much as vocabulary: the names of a
 * role's gods, the titles it earns, who its quest leader is, what its nemesis is called. The hero
 * is then a choice of one from each table, and most of what distinguishes an Archeologist from a
 * Valkyrie is data rather than code.
 *
 * The second half is the hero themselves -- one large structure holding position, health,
 * attributes, skills, and a great deal of history. It is large because the game remembers more
 * about the hero than about anything else: not only the current state but what it was last turn,
 * what the peak was, how many times a voluntary restriction has been broken, which milestones have
 * been reached and in what order.
 *
 * Several fields exist in pairs for that reason. Health has a current value, a maximum, and the
 * highest that maximum has ever been. Position has where the hero is and where they were. This is
 * not redundancy; the difference between the two is what a great many rules are about.
 *
 * @note The role tables are constant, and the hero structure is saved. So a saved game depends on
 *       the layout of the second half but only on the indices into the first.
 * @warning Being polymorphed gives the hero a second set of health and attributes, held alongside
 *          the real ones rather than replacing them. Reading the wrong set is the standard mistake
 *          in this file's neighbourhood: the values are both plausible, and only one is in effect.
 */

/**
 * @file you.h
 * @brief 영웅. 그가 누구인지, 그에게 무슨 일이 있었는지, 그리고 지금 무엇인지.
 *
 * 여기에는 서로 다른 두 가지가 살고 있고, 그 둘을 구별하면 이 파일이 훨씬 읽기 쉬워진다.
 *
 * 전반부는 게임이 제공하는 영웅의 종류를 기술한다. 직업, 종족, 성별, 진영이 각각 표의 항목이며, 능력치보다는 어휘를 담는다. 직업의 신들의 이름,
 * 그것이 얻는 칭호, 퀘스트 지도자가 누구인지, 원수가 무엇이라 불리는지. 그러면 영웅은 각 표에서 하나씩 고른 것이 되고, 고고학자와 발키리를 구별하는 것의
 * 대부분이 코드가 아니라 데이터가 된다.
 *
 * 후반부는 영웅 자신이다. 위치, 체력, 능력치, 기술, 그리고 상당한 양의 이력을 담은 하나의 큰 구조체. 큰 것은 게임이 영웅에 대해 다른 무엇보다 많이
 * 기억하기 때문이다. 현재 상태만이 아니라 지난 턴에 무엇이었는지, 최고치가 얼마였는지, 자발적 제약을 몇 번 깼는지, 어떤 이정표에 어떤 순서로 도달했는지.
 *
 * 그 때문에 여러 필드가 짝으로 존재한다. 체력에는 현재값, 최대값, 그리고 그 최대값이 지금까지 이른 최고값이 있다. 위치에는 영웅이 어디 있는지와 어디
 * 있었는지가 있다. 이것은 잉여가 아니다. 아주 많은 규칙이 다루는 것이 바로 그 둘의 차이다.
 *
 * @note 직업 표들은 상수이고, 영웅 구조체는 저장된다. 그래서 저장 게임은 후반부의 배치에 의존하지만 전반부에 대해서는 색인에만 의존한다.
 * @warning 변신한 영웅은 두 번째 체력과 능력치 묶음을 갖는다. 진짜 것을 대체하는 것이 아니라 나란히 보관된다. 잘못된 묶음을 읽는 것이 이 파일 주변의
 *          전형적인 잘못이다. 두 값 모두 그럴듯하고, 그 중 하나만 유효하다.
 */

#include "skills.h"

/*** Substructures ***/

/**
 * @brief A name that may differ by gender.
 * @note The female form is absent when it is the same as the male one, so a null there means
 *       "unchanged" and not "missing".
 */
/**
 * @brief 성별에 따라 달라질 수 있는 이름.
 * @note 여성형이 남성형과 같을 때는 없다. 그래서 그 자리의 널은 "빠짐"이 아니라 "그대로"를 뜻한다.
 */
struct RoleName {
    const char *m; /* name when character is male */
    const char *f; /* when female; null if same as male */
};

/**
 * @brief How something grows as the hero gains levels.
 *
 * Each stage gives a fixed part and a random part, and there are three stages: what the hero starts
 * with, what each early level adds, and what each later level adds. Growth slows past a cutoff, so
 * two rates are needed rather than one.
 *
 * @note Both a role and a race contribute one of these, and their contributions are added. That is
 *       how a dwarven Valkyrie is sturdier than a human one without either table knowing about the
 *       other.
 */
/**
 * @brief 영웅이 레벨을 올릴 때 무엇이 어떻게 자라는지.
 *
 * 각 단계가 고정된 부분과 무작위 부분을 주며, 단계가 셋이다. 영웅이 처음에 갖는 것, 이른 레벨마다 더해지는 것, 늦은 레벨마다 더해지는 것. 어떤 기준을
 * 넘으면 성장이 느려지므로 하나가 아니라 두 비율이 필요하다.
 *
 * @note 직업과 종족이 각각 하나를 내놓고, 그 기여가 더해진다. 어느 표도 다른 표를 모른 채로 드워프 발키리가 인간 발키리보다 튼튼해지는 방식이다.
 */
struct RoleAdvance {
    /* "fix" is the fixed amount, "rnd" is the random amount */
    xint16 infix, inrnd; /* at character initialization */
    xint16 lofix, lornd; /* gained per level <  gu.urole.xlev */
    xint16 hifix, hirnd; /* gained per level >= gu.urole.xlev */
};

/**
 * @brief Which of the game's unique objects the hero is carrying.
 * @note Cached rather than found by searching the pack, because a great many rules ask about these
 *       every turn -- monsters covet them, and the endgame depends on them.
 * @warning A cache, so it must be brought up to date whenever the pack changes. A stale bit here
 *          claims the hero holds something they do not.
 */
/**
 * @brief 영웅이 게임의 유일한 물건 중 무엇을 지니고 있는지.
 * @note 가방을 뒤져 찾는 대신 캐시된다. 아주 많은 규칙이 매 턴 이것을 묻기 때문이다. 몬스터들이 그것을 탐하고, 종반부가 그것에 달려 있다.
 * @warning 캐시이므로 가방이 바뀔 때마다 갱신되어야 한다. 여기의 낡은 비트는 영웅이 갖지 않은 것을 갖고 있다고 주장한다.
 */
struct u_have {
    Bitfield(amulet, 1);   /* carrying Amulet   */
    Bitfield(bell, 1);     /* carrying Bell     */
    Bitfield(book, 1);     /* carrying Book     */
    Bitfield(menorah, 1);  /* carrying Candelabrum */
    Bitfield(questart, 1); /* carrying the Quest Artifact */
    Bitfield(unused, 3);
};

/**
 * @brief Things that have happened once and cannot happen again.
 *
 * Each of these gates something: a wish the Amulet grants only once, a quest that cannot be
 * re-offered, a drawbridge tune that stops mattering after the bridge is destroyed. The record is
 * kept because the game must refuse a second time, and refusing requires remembering.
 *
 * @note @c uheard_tune is a state and not a flag -- knowing of the tune, having heard it, and its
 *       having become useless are three different situations the game phrases differently.
 * @note @c uhand_of_elbereth likewise holds which of the three roles' version was granted, since the
 *       title differs.
 */
/**
 * @brief 한 번 일어났고 다시 일어날 수 없는 일들.
 *
 * 각각이 무언가를 잠근다. 부적이 한 번만 들어주는 소원, 다시 제안될 수 없는 퀘스트, 다리가 파괴된 뒤로는 의미가 없어지는 개폐교의 곡조. 게임이 두 번째를
 * 거절해야 하고 거절하려면 기억해야 하므로 이 기록이 보관된다.
 *
 * @note @c uheard_tune 은 플래그가 아니라 상태다. 그 곡조를 안다는 것, 들어 보았다는 것, 그것이 쓸모없어졌다는 것은 게임이 서로 다르게 표현하는 세 가지
 *       상황이다.
 * @note @c uhand_of_elbereth 도 마찬가지로 세 직업 중 어느 판본이 주어졌는지를 담는다. 칭호가 다르기 때문이다.
 */
struct u_event {
    Bitfield(minor_oracle, 1); /* received at least 1 cheap oracle */
    Bitfield(major_oracle, 1); /*  "  expensive oracle */
    Bitfield(read_tribute, 1); /* read a passage from a novel */
    Bitfield(qcalled, 1);      /* called by Quest leader to do task */
    Bitfield(qexpelled, 1);    /* expelled from the Quest dungeon */
    Bitfield(qcompleted, 1);   /* successfully completed Quest task */
    Bitfield(uheard_tune, 2);  /* 1=know about, 2=heard passtune, 3=bridge has
                                * been destroyed so tune has become useless */
    Bitfield(uopened_dbridge, 1);   /* opened the drawbridge */
    Bitfield(invoked, 1);           /* invoked Gate to the Sanctum level */
    Bitfield(gehennom_entered, 1);  /* entered Gehennom via Valley */
    Bitfield(uhand_of_elbereth, 2); /* became Hand of Elbereth */
    Bitfield(udemigod, 1);          /* killed the wiz */
    Bitfield(uvibrated, 1);         /* stepped on "vibrating square" */
    Bitfield(ascended, 1);          /* has offered the Amulet */

    Bitfield(amulet_wish, 1);       /* has gained a wish from the Amulet */
    /* 7 free bits */
};

/*
 * Achievements:  milestones reached during the current game.
 * Numerical order of these matters because they've been encoded in
 * a bitmask in xlogfile.  Reordering would break decoding that.
 * Aside from that, the number isn't significant--they're recorded
 * and eventually disclosed in the order achieved.
 *
 * Since xlogfile could be post-processed by unknown tools, we should
 * limit these to 31 total (it's possible that 32-bit signed longs are
 * the best such tools can offer).  Eventually that is likely to need
 * to change, probably by giving xlogfile an achieve2 field rather
 * than by assuming that 64-bit longs are viable or by squeezing in a
 * 32nd entry by switching to unsigned long.
 */
/**
 * @brief Milestones reached this game.
 *
 * Recorded and disclosed in the order they were reached, so the list tells the story of a game rather
 * than checking boxes. The numbers themselves are not meaningful in play -- what matters is that they
 * never change, because they are written into the log file as a bitmask that outside tools decode.
 *
 * @warning The order is a compatibility constraint, not a convenience. Reordering these breaks the
 *          decoding of every existing log file, as the existing comment states.
 * @warning There is room for thirty-one and no more, because a tool reading the log may only manage a
 *          signed 32-bit value. The existing comment expects a second field rather than a wider one
 *          when that runs out.
 * @note The rank achievements are stored negated when the hero was female at the time, so that
 *       disclosure can use the title that applied then rather than the one that applies now.
 */
/**
 * @brief 이번 게임에서 도달한 이정표.
 *
 * 도달한 순서대로 기록되고 공개되므로, 이 목록은 항목에 표시를 하는 것이 아니라 한 게임의 이야기를 들려준다. 숫자 자체는 플레이에서 의미가 없다. 중요한
 * 것은 그것이 결코 바뀌지 않는다는 점이다. 외부 도구가 해독하는 비트마스크로 로그 파일에 기록되기 때문이다.
 *
 * @warning 순서는 편의가 아니라 호환성 제약이다. 기존 주석이 밝히듯 이것들의 순서를 바꾸면 기존 모든 로그 파일의 해독이 깨진다.
 * @warning 서른한 개까지만 자리가 있고 그 이상은 없다. 로그를 읽는 도구가 부호 있는 32비트 값만 다룰 수 있을지도 모르기 때문이다. 기존 주석은 그것이
 *          소진되면 더 넓은 필드가 아니라 두 번째 필드를 두기를 기대하고 있다.
 * @note 계급 업적은 그때 영웅이 여성이었으면 음수로 저장된다. 그래서 공개할 때 지금 적용되는 칭호가 아니라 그때 적용되던 칭호를 쓸 수 있다.
 */
enum achivements {
    ACH_BELL =  1, /* acquired Bell of Opening */
    ACH_HELL =  2, /* entered Gehennom */
    ACH_CNDL =  3, /* acquired Candelabrum of Invocation */
    ACH_BOOK =  4, /* acquired Book of the Dead */
    ACH_INVK =  5, /* performed invocation to gain access to Sanctum */
    ACH_AMUL =  6, /* acquired The Amulet */
    ACH_ENDG =  7, /* entered end game */
    ACH_ASTR =  8, /* entered Astral Plane */
    ACH_UWIN =  9, /* ascended */
    ACH_MINE_PRIZE = 10, /* acquired Mines' End luckstone */
    ACH_SOKO_PRIZE = 11, /* acquired Sokoban bag or amulet */
    ACH_MEDU = 12, /* killed Medusa */
    ACH_BLND = 13, /* hero was always blond, no, blind */
    ACH_NUDE = 14, /* hero never wore armor */
    /* 1 through 14 were present in 3.6.x; the rest are newer; first,
       some easier ones so less skilled players can have achievements */
    ACH_MINE = 15, /* entered Gnomish Mines */
    ACH_TOWN = 16, /* reached Minetown */
    ACH_SHOP = 17, /* entered a shop */
    ACH_TMPL = 18, /* entered a temple */
    ACH_ORCL = 19, /* consulted the Oracle */
    ACH_NOVL = 20, /* read at least one passage from a Discworld novel */
    ACH_SOKO = 21, /* entered Sokoban */
    ACH_BGRM = 22, /* entered Bigroom (not guaranteed to be in every dgn) */
    /* role's rank titles, beyond first (#0 at level one, not an achievement);
       23..30 are negated if hero is female at the time new rank is gained
       so that disclosing them can use the gender which applied at the time */
    ACH_RNK1 = 23, ACH_RNK2 = 24, ACH_RNK3 = 25, ACH_RNK4 = 26,
    ACH_RNK5 = 27, ACH_RNK6 = 28, ACH_RNK7 = 29, ACH_RNK8 = 30,
    ACH_TUNE = 31, /* discovered the castle drawbridge's open/close tune */
    N_ACH = 32     /* allocate room for 31 plus a slot for 0 terminator */
};
    /*
     * Other potential achievements to track (this comment briefly resided
     * in encodeachieve(topten.c) and has been revised since moving here:
     *  AC <= 0, AC <= -10, AC <= -20 (stop there; lower is better but
     *    not something to encourage with achievements),
     *  got quest summons,
     *  entered quest branch,
     *  chatted with leader,
     *  entered second or lower quest level (implies leader gave the Ok),
     *  entered last quest level,
     *  defeated nemesis (not same as acquiring Bell or artifact),
     *  completed quest (formally, by bringing artifact to leader),
     *  entered rogue level,
     *  entered Fort Ludios level/branch (not guaranteed to be achievable),
     *  entered Medusa level,
     *  entered castle level,
     *  obtained castle wand (handle similarly to mines and sokoban prizes),
     *  passed Valley level (entered-Gehennom already covers Valley itself),
     *  [assorted demon lairs?],
     *  entered Vlad's tower branch,
     *  defeated Vlad (not same as acquiring Candelabrum),
     *  entered Wizard's tower area within relevant level,
     *  defeated Wizard,
     *  found vibrating square,
     *  entered sanctum level (maybe not; too close to performed-invocation),
     *  [defeated Famine, defeated Pestilence, defeated Death]
     */

/**
 * @brief How long the game has actually been played, in real time.
 *
 * Accumulated rather than computed from a start time, because a game is saved and restored and the
 * hours in between are not playing time. So the total is banked and a new interval begins each time
 * the game resumes.
 *
 * @note Kept separately from the turn count, which measures game time. A long game in turns may be a
 *       short one in hours and vice versa.
 */
/**
 * @brief 실제 시간으로 게임이 얼마나 플레이되었는지.
 *
 * 시작 시각에서 계산되는 것이 아니라 누적된다. 게임은 저장되고 복원되며, 그 사이의 시간은 플레이 시간이 아니다. 그래서 총합이 적립되고 게임이 다시 시작될
 * 때마다 새 구간이 시작된다.
 *
 * @note 게임 시간을 재는 턴 수와 따로 보관된다. 턴으로는 긴 게임이 시간으로는 짧을 수 있고 그 반대일 수도 있다.
 */
struct u_realtime {
    long   realtime;     /* accumulated playing time in seconds */
    time_t start_timing; /* time game was started or restored or 'realtime'
                            was last updated (savegamestate for checkpoint) */
    time_t finish_time;  /* end of 'realtime' interval: time of save or
                            end of game; used for topten/logfile/xlogfile */
};

/* KMH, conduct --
 * These are voluntary challenges.  Each field denotes the number of
 * times a challenge has been violated.
 */
/**
 * @brief How often each voluntary restriction has been broken.
 *
 * A player may choose to play without ever eating meat, or without ever hitting anything with a
 * weapon. The game does not enforce these -- it counts. So a count of zero at the end is the
 * achievement, and the counts exist so a player can be told exactly how close they came.
 *
 * @note Counts rather than flags, because "broken once by accident" and "abandoned entirely" are worth
 *       distinguishing when the game is disclosed.
 * @note The reserved fields and the one named for a former use exist to keep the layout stable, since
 *       this structure is saved. Removing a field would invalidate old saves.
 */
/**
 * @brief 각 자발적 제약이 몇 번 깨졌는지.
 *
 * 플레이어는 고기를 결코 먹지 않기로, 또는 무기로 무엇도 때리지 않기로 할 수 있다. 게임은 그것을 강제하지 않는다. 세기만 한다. 그래서 끝에 0이라는 값이
 * 곧 업적이며, 플레이어가 얼마나 아까웠는지 정확히 들을 수 있도록 개수가 존재한다.
 *
 * @note 플래그가 아니라 개수다. 게임이 공개될 때 "실수로 한 번 깼음"과 "완전히 포기했음"을 구별할 가치가 있기 때문이다.
 * @note 예약된 필드들과 이전 용도의 이름이 남은 필드는 배치를 안정하게 유지하기 위해 있다. 이 구조체가 저장되므로, 필드를 없애면 예전 저장이 무효가 된다.
 */
struct u_conduct {     /* number of times... */
    long unvegetarian; /* eaten any animal */
    long unvegan;      /* ... or any animal byproduct */
    long food;         /* ... or any comestible */
    long gnostic;      /* used prayer, priest, or altar */
    long weaphit;      /* hit a monster with a weapon */
    long killer;       /* killed a monster yourself */
    long literate;     /* read something (other than BotD) */
    long polypiles;    /* polymorphed an object */
    long polyselfs;    /* transformed yourself */
    long wishes;       /* used a wish */
    long wisharti;     /* wished for an artifact */
    long hf_reserved1; /* hf uses for elbereth;for hf savefile compatiblity */
    long sokocheat;    /* violated special 'rules' in Sokoban */
    long pets;         /* obtained a pet */
    /* genocides already listed at end of game */
    long reserved1;
    long reserved2;
    long reserved3;
    long reserved4;
};

/**
 * @brief Handicaps the player accepted before the game began.
 *
 * Unlike conduct, these are enforced rather than counted: a hero who chose to be blind stays blind and
 * cannot be cured. They are declared at the start because they change how the game is set up, not
 * merely how it is scored.
 *
 * @note @c nudist is the exception -- it is a claim that can still be broken, and so is closer to a
 *       conduct than to a handicap.
 */
/**
 * @brief 게임이 시작되기 전에 플레이어가 받아들인 불리함.
 *
 * 계율과 달리 이들은 세어지는 것이 아니라 강제된다. 눈이 멀기를 택한 영웅은 계속 눈이 먼 채로 있고 치유될 수 없다. 게임이 어떻게 채점되는지만이 아니라
 * 어떻게 준비되는지를 바꾸므로 시작할 때 선언된다.
 *
 * @note @c nudist 는 예외다. 여전히 깨질 수 있는 주장이며, 그래서 불리함보다 계율에 가깝다.
 */
struct u_roleplay {
    boolean blind;   /* permanently blind */
    boolean nudist;  /* has not worn any armor, ever */
    boolean deaf;    /* permanently deaf */
    boolean pauper;  /* no starting inventory */
    boolean reroll;  /* starting inventory/attr rerolling enabled */
    boolean reserved1;
    boolean reserved2;
    boolean reserved3;
    long numbones;   /* # of bones files loaded */
    long numrerolls; /* # of rerolls used */
};

/**
 * @brief Everything that makes one role different from another.
 *
 * Mostly names and indices rather than numbers. A role's entry says what its gods are called, what
 * titles it earns, which monster is its quest leader, which artifact it seeks -- so a new role is
 * largely a matter of writing an entry and the accompanying text files, which is what the reminders at
 * the foot of the structure list.
 *
 * The numbers that are here are the ones that cannot be expressed as text: how attributes are rolled,
 * how health and energy advance, and the spellcasting penalties that make a Wizard a caster and a
 * Barbarian not.
 *
 * @note Every reference to a monster or artifact is an index, so the role table does not have to be
 *       built after the monster table.
 * @note The rank titles are gendered names, which is why they use the two-form name type rather than
 *       plain strings.
 */
/**
 * @brief 한 직업을 다른 직업과 다르게 만드는 모든 것.
 *
 * 대부분 숫자가 아니라 이름과 색인이다. 직업의 항목은 그 신들이 무엇이라 불리는지, 어떤 칭호를 얻는지, 어떤 몬스터가 그 퀘스트 지도자인지, 어떤 아티팩트를
 * 찾는지를 말한다. 그래서 새 직업을 만드는 일은 대체로 항목 하나와 딸린 텍스트 파일들을 쓰는 일이며, 구조체 끝의 알림 목록이 그것을 적어 두고 있다.
 *
 * 여기 있는 숫자들은 텍스트로 표현할 수 없는 것들이다. 능력치를 어떻게 굴리는지, 체력과 기력이 어떻게 오르는지, 그리고 마법사를 시전자로 만들고 야만인을
 * 그렇지 않게 만드는 주문 시전 벌점.
 *
 * @note 몬스터나 아티팩트에 대한 모든 참조가 색인이다. 그래서 직업 표가 몬스터 표보다 나중에 만들어져야 하지는 않는다.
 * @note 계급 칭호는 성별이 있는 이름이다. 그래서 평범한 문자열이 아니라 두 형태를 갖는 이름 타입을 쓴다.
 */
/*** Unified structure containing role information ***/
struct Role {
    /*** Strings that name various things ***/
    struct RoleName name;    /* the role's name (from u_init.c) */
    struct RoleName rank[9]; /* names for experience levels (from botl.c) */
    const char *lgod, *ngod, *cgod; /* god names (from pray.c) */
    const char *filecode;           /* abbreviation for use in file names */
    const char *homebase; /* quest leader's location (from questpgr.c) */
    const char *intermed; /* quest intermediate goal (from questpgr.c) */

    /*** Indices of important monsters and objects ***/
    short mnum,    /* index (PM_) of role (botl.c) */
        petnum,    /* PM_ of preferred pet (NON_PM == random) */
        ldrnum,    /* PM_ of quest leader (questpgr.c) */
        guardnum,  /* PM_ of quest guardians (questpgr.c) */
        neminum,   /* PM_ of quest nemesis (questpgr.c) */
        enemy1num, /* specific quest enemies (NON_PM == random) */
        enemy2num;
    char enemy1sym, /* quest enemies by class (S_) */
        enemy2sym;
    short questarti; /* index (ART_) of quest artifact (questpgr.c) */

    /**
     * @brief Which races, genders and alignments this role may be combined with.
     *
     * One field holds all three because they are all constraints on the same choice, and character
     * creation checks them together: not every race may take every role, and a role may be restricted
     * in alignment.
     *
     * @note The race bits are the same values monsters use for race, and the alignment bits the same
     *       values alignment uses, so no conversion is needed when comparing a role against a monster
     *       or an altar.
     * @warning Three different masks share this field. Testing a gender bit against the race mask, or
     *          vice versa, compiles and answers.
     */
    /**
     * @brief 이 직업이 어떤 종족, 성별, 진영과 결합될 수 있는지.
     *
     * 하나의 필드가 셋을 함께 담는 것은 그것들이 모두 같은 선택에 대한 제약이고 캐릭터 생성이 그것을 함께 확인하기 때문이다. 모든 종족이 모든 직업을 가질 수
     * 있는 것은 아니고, 직업이 진영에서 제한될 수 있다.
     *
     * @note 종족 비트는 몬스터가 종족에 쓰는 값과 같고 진영 비트는 진영이 쓰는 값과 같다. 그래서 직업을 몬스터나 제단과 비교할 때 변환이 필요하지 않다.
     * @warning 세 개의 서로 다른 마스크가 이 필드를 공유한다. 성별 비트를 종족 마스크와 검사하거나 그 반대로 하는 것이 컴파일되고 답을 낸다.
     */
    /*** Bitmasks ***/
    short allow;                  /* bit mask of allowed variations */
#define ROLE_RACEMASK  0x0ff8     /* allowable races */
#define ROLE_GENDMASK  0xf000     /* allowable genders */
#define ROLE_MALE      0x1000
#define ROLE_FEMALE    0x2000
#define ROLE_NEUTER    0x4000
#define ROLE_ALIGNMASK AM_MASK    /* allowable alignments */
#define ROLE_LAWFUL    AM_LAWFUL
#define ROLE_NEUTRAL   AM_NEUTRAL
#define ROLE_CHAOTIC   AM_CHAOTIC

    /*** Attributes (from attrib.c and exper.c) ***/
    xint16 attrbase[A_MAX];    /* lowest initial attributes */
    xint16 attrdist[A_MAX];    /* distribution of initial attributes */
    struct RoleAdvance hpadv; /* hit point advancement */
    struct RoleAdvance enadv; /* energy advancement */
    xint16 xlev;               /* cutoff experience level */
    xint16 initrecord;         /* initial alignment record */

    /*** Spell statistics (from spell.c) ***/
    int spelbase; /* base spellcasting penalty */
    int spelheal; /* penalty (-bonus) for healing spells */
    int spelshld; /* penalty for wearing any shield */
    int spelarmr; /* penalty for wearing metal armour */
    int spelstat; /* which stat (A_) is used */
    int spelspec; /* spell (SPE_) the class excels at */
    int spelsbon; /* penalty (-bonus) for that spell */

    /*** Properties in variable-length arrays ***/
    /* intrinsics (see attrib.c) */
    /* initial inventory (see u_init.c) */
    /* skills (see u_init.c) */

    /*** Don't forget to add... ***/
    /* quest leader, guardians, nemesis (monst.c) */
    /* quest artifact (artilist.h) */
    /* quest dungeon definition (dat/Xyz.dat) */
    /* quest text (dat/quest.txt) */
    /* dictionary entries (dat/data.bas) */
};

extern const struct Role roles[]; /* table of available roles */
/**
 * @def Role_if
 * @brief Whether the hero is of a given role.
 * @param X a monster index, since a role is identified by the monster that represents it
 * @note Compares the hero's role and not their current form. A Valkyrie polymorphed into a dragon is
 *       still a Valkyrie by this test, which is almost always what a rule wants.
 */
/**
 * @def Role_if
 * @brief 영웅이 주어진 직업인지.
 * @param X 몬스터 색인. 직업은 그것을 대표하는 몬스터로 식별된다
 * @note 영웅의 현재 형태가 아니라 직업을 비교한다. 용으로 변신한 발키리도 이 검사로는 여전히 발키리이며, 규칙이 원하는 것은 거의 항상 그것이다.
 */
#define Role_if(X) (gu.urole.mnum == (X))
/**
 * @def Role_switch
 * @brief The hero's role, for use as the subject of a multi-way branch.
 * @note Exists so that code branching on many roles reads as one decision rather than a chain of
 *       comparisons.
 */
/**
 * @def Role_switch
 * @brief 영웅의 직업. 여러 갈래 분기의 대상으로 쓰기 위한 것.
 * @note 여러 직업으로 갈라지는 코드가 비교의 연쇄가 아니라 하나의 결정으로 읽히도록 존재한다.
 */
#define Role_switch (gu.urole.mnum)

/**
 * @name Unmade choices
 * @brief The two ways a character creation choice can be absent.
 * @note Not chosen and asked-to-be-random are different: the first must still be resolved by asking or
 *       defaulting, the second is already an instruction. Both are negative so they cannot be mistaken
 *       for an index.
 * @{
 */
/**
 * @name 정해지지 않은 선택
 * @brief 캐릭터 생성 선택이 없을 수 있는 두 가지 방식.
 * @note 고르지 않음과 무작위로 해 달라는 것은 다르다. 앞의 것은 여전히 묻거나 기본값을 써서 결정해야 하고, 뒤의 것은 이미 지시다. 둘 다 음수여서 색인으로
 *       오인될 수 없다.
 * @{
 */
/* used during initialization for race, gender, and alignment
   as well as for character class */
#define ROLE_NONE (-1)
#define ROLE_RANDOM (-2)
/** @} */

/*** Unified structure specifying race information ***/

/**
 * @brief Everything that makes one race different from another.
 *
 * Four forms of the name are kept rather than one, because English needs them all: the noun, the
 * adjective, the collective, and the word for a single member. The game builds sentences from these, so
 * a race that supplied only a noun would produce awkward text.
 *
 * The three masks are the interesting part. A race records not only its own identifying bit but which
 * races it is always at peace with and which it is always hostile to -- so racial enmity is a property
 * of the race rather than a table of pairs.
 *
 * @note Its mummy and zombie forms are recorded because the hero's remains must be of the right race,
 *       and those are separate species.
 * @note Attribute limits here are floors and ceilings, not amounts. A race constrains what a role's
 *       rolls may produce rather than replacing them.
 */
/**
 * @brief 한 종족을 다른 종족과 다르게 만드는 모든 것.
 *
 * 이름을 하나가 아니라 네 형태로 보관한다. 영어가 그 전부를 필요로 하기 때문이다. 명사, 형용사, 집합명사, 그리고 한 구성원을 가리키는 낱말. 게임이 이것들로
 * 문장을 짓기 때문에, 명사만 제공하는 종족은 어색한 글을 만들게 된다.
 *
 * 흥미로운 부분은 세 개의 마스크다. 종족은 자신을 식별하는 비트만이 아니라 항상 평화로운 종족과 항상 적대적인 종족을 함께 기록한다. 그래서 종족 간 원한이
 * 짝의 표가 아니라 종족의 속성이 된다.
 *
 * @note 미라와 좀비 형태가 기록되는 것은 영웅의 잔해가 올바른 종족이어야 하고 그것들이 별개의 종족이기 때문이다.
 * @note 여기의 능력치 한계는 양이 아니라 하한과 상한이다. 종족은 직업의 굴림을 대체하는 것이 아니라 그것이 낼 수 있는 값을 제한한다.
 */
struct Race {
    /*** Strings that name various things ***/
    const char *noun;           /* noun ("human", "elf") */
    const char *adj;            /* adjective ("human", "elven") */
    const char *coll;           /* collective ("humanity", "elvenkind") */
    const char *filecode;       /* code for filenames */
    struct RoleName individual; /* individual as a noun ("man", "elf") */

    /*** Indices of important monsters and objects ***/
    short mnum,    /* PM_ as a monster */
        mummynum,  /* PM_ as a mummy */
        zombienum; /* PM_ as a zombie */

    /*** Bitmasks ***/
    short allow;    /* bit mask of allowed variations */
    short selfmask, /* your own race's bit mask */
        lovemask,   /* bit mask of always peaceful */
        hatemask;   /* bit mask of always hostile */

    /*** Attributes ***/
    xint16 attrmin[A_MAX];     /* minimum allowable attribute */
    xint16 attrmax[A_MAX];     /* maximum allowable attribute */
    struct RoleAdvance hpadv; /* hit point advancement */
    struct RoleAdvance enadv; /* energy advancement */
#if 0 /* DEFERRED */
    int   nv_range;           /* night vision range */
    int   xray_range;         /* X-ray vision range */
#endif

    /*** Properties in variable-length arrays ***/
    /* intrinsics (see attrib.c) */

    /*** Don't forget to add... ***/
    /* quest leader, guardians, nemesis (monst.c) */
    /* quest dungeon definition (dat/Xyz.dat) */
    /* quest text (dat/quest.txt) */
    /* dictionary entries (dat/data.bas) */
};

extern const struct Race races[]; /* Table of available races */
#define Race_if(X) (gu.urace.mnum == (X))
#define Race_switch (gu.urace.mnum)

/**
 * @brief A gender, held chiefly for its pronouns.
 *
 * Exists so that message text can be written once with a pronoun looked up rather than written twice
 * with a conditional. The three pronoun forms are separate fields because English inflects them
 * differently and no rule derives one from another.
 */
/**
 * @brief 성별. 주로 그 대명사들을 위해 보관된다.
 *
 * 메시지 글을 조건문과 함께 두 번 쓰는 대신 대명사를 찾아 쓰며 한 번만 쓸 수 있도록 존재한다. 세 대명사 형태가 각각의 필드인 것은 영어가 그것들을 다르게
 * 굴절시키고 어느 규칙도 하나에서 다른 하나를 유도하지 않기 때문이다.
 */
/*** Unified structure specifying gender information ***/
struct Gender {
    const char *adj;      /* male/female/neuter */
    const char *he;       /* he/she/it */
    const char *him;      /* him/her/it */
    const char *his;      /* his/her/its */
    const char *filecode; /* file code */
    short allow;          /* equivalent ROLE_ mask */
};
/**
 * @def ROLE_GENDERS
 * @brief How many genders a player may choose from.
 * @note Fewer than the table holds. The neuter entry exists for monsters, and the existing comment notes
 *       what would have to change to offer it to players.
 */
/**
 * @def ROLE_GENDERS
 * @brief 플레이어가 고를 수 있는 성별의 수.
 * @note 표가 담고 있는 것보다 적다. 중성 항목은 몬스터를 위해 있고, 기존 주석이 그것을 플레이어에게 제시하려면 무엇을 바꿔야 하는지 밝히고 있다.
 */
#define ROLE_GENDERS 2    /* number of permitted player genders
                             increment to 3 if you allow neuter roles */

extern const struct Gender genders[]; /* table of available genders */
/**
 * @name Pronouns for the hero
 * @brief The hero's pronoun in each grammatical form.
 * @note Simple, because the hero's gender is always known -- there is no question of not being able to
 *       see them.
 * @{
 */
/**
 * @name 영웅의 대명사
 * @brief 각 문법 형태에서의 영웅의 대명사.
 * @note 단순하다. 영웅의 성별은 언제나 알려져 있고, 그를 볼 수 없는 경우는 문제가 되지 않는다.
 * @{
 */
/* pronouns for the hero */
#define uhe()      (genders[flags.female ? 1 : 0].he)
#define uhim()     (genders[flags.female ? 1 : 0].him)
#define uhis()     (genders[flags.female ? 1 : 0].his)
/** @} */

/**
 * @name Pronoun selection flags
 * @brief What to take into account when choosing a monster's pronoun.
 * @note @c PRONOUN_HALLU matters because a hallucinating hero sees a different monster than is there,
 *       and the pronoun must match what they think they see rather than what is real.
 * @note @c PRONOUN_NO_IT asks for a gendered pronoun even when the monster cannot be seen -- for the
 *       cases where the hero knows perfectly well who it is and "it" would read as ignorance.
 * @{
 */
/**
 * @name 대명사 선택 플래그
 * @brief 몬스터의 대명사를 고를 때 무엇을 고려할지.
 * @note @c PRONOUN_HALLU 가 중요한 것은, 환각 상태의 영웅은 실제 있는 것과 다른 몬스터를 보기 때문이다. 대명사는 실제가 아니라 그가 본다고 여기는 것과
 *       맞아야 한다.
 * @note @c PRONOUN_NO_IT 은 몬스터가 보이지 않을 때에도 성별 있는 대명사를 요청한다. 영웅이 그것이 누구인지 잘 알고 있어 "그것"이 무지처럼 읽힐 경우를
 *       위한 것이다.
 * @{
 */
/* pronoun_gender() flag masks */
#define PRONOUN_NORMAL 0 /* none of the below */
#define PRONOUN_NO_IT  1
#define PRONOUN_HALLU  2
/** @} */

/**
 * @name Pronouns for a monster
 * @brief A monster's pronoun, chosen according to what the hero can tell.
 *
 * Not a lookup but a judgement: an unseen monster is "it" regardless of its sex, because the hero has no
 * grounds to say otherwise. The @c noit_ forms override that where the hero does have grounds.
 *
 * @warning The @c mtmp argument is evaluated more than once.
 * @{
 */
/**
 * @name 몬스터의 대명사
 * @brief 영웅이 알 수 있는 바에 따라 골라진 몬스터의 대명사.
 *
 * 조회가 아니라 판단이다. 보이지 않는 몬스터는 그 성별과 무관하게 "그것"이다. 영웅에게 달리 말할 근거가 없기 때문이다. @c noit_ 형태들은 영웅에게 근거가
 * 있는 곳에서 그것을 무시한다.
 *
 * @warning @c mtmp 인자가 한 번 넘게 평가된다.
 * @{
 */
/* corresponding pronouns for monsters; yields "it" when mtmp can't be seen */
#define mhe(mtmp)  (genders[pronoun_gender(mtmp, PRONOUN_HALLU)].he)
#define mhim(mtmp) (genders[pronoun_gender(mtmp, PRONOUN_HALLU)].him)
#define mhis(mtmp) (genders[pronoun_gender(mtmp, PRONOUN_HALLU)].his)
/* override "it" if reason is lack of visibility rather than neuter species */
#define noit_mhe(mtmp) \
    (genders[pronoun_gender(mtmp, (PRONOUN_NO_IT | PRONOUN_HALLU))].he)
#define noit_mhim(mtmp) \
    (genders[pronoun_gender(mtmp, (PRONOUN_NO_IT | PRONOUN_HALLU))].him)
#define noit_mhis(mtmp) \
    (genders[pronoun_gender(mtmp, (PRONOUN_NO_IT | PRONOUN_HALLU))].his)
/** @} */

/**
 * @brief An alignment, with both the name of the principle and the adjective for a follower.
 * @note Two names because the game speaks of both -- of law itself and of a lawful hero -- and neither
 *       form derives from the other.
 * @note Carries both the role mask bit and the alignment value, so this table is the bridge between the
 *       two ways alignment is written elsewhere.
 */
/**
 * @brief 진영. 그 원리의 이름과 그것을 따르는 자를 가리키는 형용사를 함께 지닌다.
 * @note 게임이 둘 다를 말하므로 이름이 둘이다. 법 자체와 법을 따르는 영웅. 어느 형태도 다른 형태에서 유도되지 않는다.
 * @note 직업 마스크 비트와 진영 값을 함께 지닌다. 그래서 이 표가 다른 곳에서 진영이 적히는 두 방식 사이의 다리다.
 */
/*** Unified structure specifying alignment information ***/
struct Align {
    const char *noun;     /* law/balance/chaos */
    const char *adj;      /* lawful/neutral/chaotic */
    const char *filecode; /* file code */
    short allow;          /* equivalent ROLE_ mask */
    aligntyp value;       /* equivalent A_ value */
};
#define ROLE_ALIGNS 3     /* number of permitted player alignments */

extern const struct Align aligns[]; /* table of available alignments */

/**
 * @brief What is holding the hero, when something is.
 * @note Being stuck is one situation with a countdown, so the kind and the remaining time are separate
 *       fields -- the kind decides what the hero must do to get free and what messages describe the
 *       struggle.
 * @warning The kind is meaningful only while the countdown is non-zero. A stale kind left behind after
 *          escaping still reads as a valid trap.
 */
/**
 * @brief 무언가가 영웅을 붙잡고 있을 때, 그것이 무엇인지.
 * @note 붙잡힌 것은 남은 시간이 딸린 하나의 상황이므로 종류와 남은 시간이 각각의 필드다. 종류가 영웅이 벗어나기 위해 무엇을 해야 하는지와 그 발버둥을
 *       어떤 메시지로 기술할지를 정한다.
 * @warning 종류는 남은 시간이 0이 아닐 때만 의미가 있다. 벗어난 뒤 남겨진 낡은 종류도 여전히 유효한 함정으로 읽힌다.
 */
enum utraptypes {
    TT_NONE       = 0,
    TT_BEARTRAP   = 1,
    TT_PIT        = 2,
    TT_WEB        = 3,
    TT_LAVA       = 4,
    TT_INFLOOR    = 5,
    TT_BURIEDBALL = 6
};

/**
 * @brief How the hero is about to change level, and what must happen along with it.
 *
 * Bits rather than a choice, because the manner of arriving and the housekeeping that goes with it are
 * separate facts: falling through a trapdoor is one bit, and removing the portal behind you is another.
 *
 * @note @c UTOTYPE_DEFERRED marks a level change that has been decided but not carried out. A level
 *       cannot be changed in the middle of whatever caused it, so the intention is recorded and acted on
 *       at a safe point.
 * @warning A gap in the numbering separates the manners of arrival from the modifiers, so these are not
 *          consecutive and should not be iterated over.
 */
/**
 * @brief 영웅이 곧 레벨을 옮기려 하는 방식과, 그와 함께 일어나야 하는 일.
 *
 * 택일이 아니라 비트다. 도착하는 방식과 그에 딸린 정리 작업이 서로 다른 사실이기 때문이다. 뚜껑문으로 떨어지는 것이 한 비트이고, 뒤의 포탈을 없애는 것이
 * 또 하나다.
 *
 * @note @c UTOTYPE_DEFERRED 는 결정되었으나 아직 수행되지 않은 레벨 이동을 표시한다. 그것을 일으킨 일의 도중에 레벨을 바꿀 수는 없으므로, 의도가 기록되고
 *       안전한 지점에서 실행된다.
 * @warning 번호에 난 틈이 도착 방식과 수정자를 나누고 있다. 그래서 이 값들은 연속하지 않으며 순회해서는 안 된다.
 */
enum utotypes {
    UTOTYPE_NONE     = 0x00,
    UTOTYPE_ATSTAIRS = 0x01,
    UTOTYPE_FALLING  = 0x02,
    UTOTYPE_PORTAL   = 0x04,
    UTOTYPE_RMPORTAL = 0x10,  /* remove portal */
    UTOTYPE_DEFERRED = 0x20   /* deferred_goto */
};

/**
 * @brief The hero.
 *
 * The largest structure in the game, and the reason is that the game remembers more about the hero than
 * about anything else. Not just the present state but the previous one, the peak, and the history:
 * where they were last turn, which rooms they just left, what the highest their health has ever been,
 * how many times they have died.
 *
 * A few groups are worth pointing out.
 *
 * Position comes in several forms -- where the hero is, where they were, which direction they are
 * moving, where they are travelling to. The difference between the current and previous position is what
 * lets the game notice entering a shop or leaving a room, which is why both are kept rather than one.
 *
 * The rooms are kept as short lists rather than single values because a hero can be in more than one at
 * once, and because "rooms entered this turn" and "rooms occupied now" answer different questions.
 *
 * Polymorph doubles several things. Health, attributes and monster number all exist twice: the hero's own
 * and the form's. The originals are kept intact so that the form can end.
 *
 * Attributes come in six parallel sets -- current, maximum, bonus, exercise, temporary change, and the
 * countdown on that change -- because an attribute is not a number but the result of several
 * independently changing influences.
 *
 * @note Both @c ustuck and @c usteed are pointers to monsters, and both are additionally stored as
 *       identifiers for saving, since a pointer means nothing in a saved file.
 * @note @c umonst is a placeholder for future work, not something in use.
 * @warning The polymorph copies are the trap here. Health while polymorphed is not the hero's health,
 *          and reading the wrong one gives a plausible number that is not in effect.
 */
/**
 * @brief 영웅.
 *
 * 게임에서 가장 큰 구조체이며, 그 이유는 게임이 영웅에 대해 다른 무엇보다 많이 기억하기 때문이다. 현재 상태만이 아니라 이전 상태, 최고치, 그리고 이력.
 * 지난 턴에 어디 있었는지, 방금 어느 방을 떠났는지, 체력이 지금까지 이른 최고값이 얼마인지, 몇 번 죽었는지.
 *
 * 짚어 둘 만한 몇 묶음이 있다.
 *
 * 위치는 여러 형태로 있다. 영웅이 어디 있는지, 어디 있었는지, 어느 방향으로 움직이는지, 어디로 여행하는지. 현재 위치와 이전 위치의 차이가 게임이 상점에
 * 들어섰거나 방을 떠났음을 알아채게 하는 것이다. 하나가 아니라 둘이 보관되는 이유다.
 *
 * 방은 단일 값이 아니라 짧은 목록으로 보관된다. 영웅이 한 번에 둘 이상의 방에 있을 수 있고, "이번 턴에 들어선 방"과 "지금 있는 방"이 서로 다른 질문에
 * 답하기 때문이다.
 *
 * 변신은 여러 가지를 두 배로 만든다. 체력, 능력치, 몬스터 번호가 모두 두 번 존재한다. 영웅 자신의 것과 그 형태의 것. 원래의 것은 그 형태가 끝날 수 있도록
 * 온전히 보관된다.
 *
 * 능력치는 여섯 개의 나란한 묶음으로 있다. 현재, 최대, 보너스, 단련, 일시적 변화, 그리고 그 변화의 남은 시간. 능력치가 숫자 하나가 아니라 독립적으로 변하는
 * 여러 영향의 결과이기 때문이다.
 *
 * @note @c ustuck 과 @c usteed 는 둘 다 몬스터 포인터이며, 저장을 위해 식별자로도 함께 저장된다. 저장 파일에서 포인터는 아무 의미가 없기 때문이다.
 * @note @c umonst 는 앞으로의 작업을 위한 자리 표시이며 쓰이고 있는 것이 아니다.
 * @warning 여기서의 함정은 변신용 사본이다. 변신 중의 체력은 영웅의 체력이 아니며, 잘못된 것을 읽으면 유효하지 않은 그럴듯한 숫자가 나온다.
 */
/*** Information about the player ***/
struct you {
    coordxy ux, uy;     /* current map coordinates */
    int dx, dy, dz;     /* x,y,z deltas; direction of move (or zap or ... )
                         * usually +1 or 0 or -1 */
    coordxy tx, ty;     /* destination of travel */
    coordxy ux0, uy0;   /* previous ux,uy */
    d_level uz, uz0;    /* your level on this and the previous turn */
    d_level utolev;     /* level monster teleported you to, or uz */
    uchar utotype;      /* bitmask of goto_level() flags for utolev */
    d_level ucamefrom;  /* level where you came from; used for tutorial */
    boolean umoved;     /* changed map location (post-move) */
    int last_str_turn;  /* 0: none, 1: half turn, 2: full turn
                         * +: turn right, -: turn left */
    int ulevel;         /* 1 to MAXULEV (30) */
    int ulevelmax;      /* highest level, but might go down (to throttle
                         * lost level recovery via blessed full healing) */
    int ulevelpeak;     /* really highest level reached; never does down */
    unsigned utrap;     /* trap timeout */
    unsigned utraptype; /* defined if utrap nonzero. one of utraptypes */
    char urooms[5];         /* rooms (roomno + 3) occupied now */
    char urooms0[5];        /* ditto, for previous position */
    char uentered[5];       /* rooms (roomno + 3) entered this turn */
    char ushops[5];         /* shop rooms (roomno + 3) occupied now */
    char ushops0[5];        /* ditto, for previous position */
    char ushops_entered[5]; /* ditto, shops entered this turn */
    char ushops_left[5];    /* ditto, shops exited this turn */

    int uhunger;  /* refd only in eat.c and shk.c (also insight.c) */
    unsigned uhs; /* hunger state - see eat.c */

    struct prop uprops[LAST_PROP + 1];

    unsigned umconf;
    Bitfield(usick_type, 2);
#define SICK_VOMITABLE 0x01
#define SICK_NONVOMITABLE 0x02
#define SICK_ALL 0x03

    /* These ranges can never be more than MAX_RANGE (vision.h). */
    int nv_range;   /* current night vision range */
    int xray_range; /* current xray vision range */
    int unblind_telepat_range;

/*
 * These variables are valid globally only when punished and blind.
 */
#define BC_BALL 0x01  /* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02 /* bit mask for chain in 'bc_felt' below */
    int bglyph;       /* glyph under the ball */
    int cglyph;       /* glyph under the chain */
    int bc_order;     /* ball & chain order [see bc_order() in ball.c] */
    int bc_felt;      /* mask for ball/chain being felt */

    int umonster;               /* hero's "real" monster num */
    int umonnum;                /* current monster number */

    int mh, mhmax,              /* current and max hit points when polyd */
        mtimedone;              /* no. of turns until polymorph times out */
    struct attribs macurr,      /* for monster attribs */
                   mamax;       /* for monster attribs */
    int ulycn;                  /* lycanthrope type */

    unsigned ucreamed;
    unsigned uswldtim;          /* time you have been swallowed */

    Bitfield(uswallow, 1);      /* true if swallowed */
    Bitfield(uinwater, 1);      /* if you're currently in water (only
                                   underwater possible currently) */
    Bitfield(uundetected, 1);   /* if you're a hiding monster/piercer */
    Bitfield(mfemale, 1);       /* saved human value of flags.female */
    Bitfield(uinvulnerable, 1); /* you're invulnerable (praying) */
    Bitfield(uburied, 1);       /* you're buried */
    Bitfield(uedibility, 1);    /* blessed food detect; sense unsafe food */
    Bitfield(uhandedness, 1); /* There is no advantage for either handedness.
                                 The distinction is only for flavor variation
                                 and for use in messages. */
#define RIGHT_HANDED 0x00
#define LEFT_HANDED  0x01

    unsigned udg_cnt;           /* how long you have been demigod */
    struct u_event uevent;      /* certain events have happened */
    struct u_have uhave;        /* you're carrying special objects */
    struct u_conduct uconduct;  /* KMH, conduct */
    struct u_roleplay uroleplay;
    struct attribs acurr,       /* your current attributes (eg. str)*/
                    aexe,       /* for gain/loss via "exercise" */
                    abon,       /* your bonus attributes (eg. str) */
                    amax,       /* your max attributes (eg. str) */
                   atemp,       /* used for temporary loss/gain */
                   atime;       /* used for loss/gain countdown */
    align ualign;               /* character alignment */
/**
 * @name Alignment history
 * @brief Which alignment the hero has now and which they started with.
 * @note Two are kept because conversion is possible and reversible in its consequences: a converted hero
 *       is judged against their new god but the original still matters, notably for whether crowning is
 *       available. So the array holds both rather than overwriting.
 * @note @c CONVERT is the size of that array and not an index into it, despite sitting among the indices.
 * @{
 */
/**
 * @name 진영 이력
 * @brief 영웅이 지금 어떤 진영인지와 무엇으로 시작했는지.
 * @note 개종이 가능하고 그 결과가 되돌려질 수 있으므로 둘이 보관된다. 개종한 영웅은 새 신에 의해 판단되지만 원래의 것도 여전히 중요하며, 특히 왕관을 받을
 *       수 있는지에 그렇다. 그래서 배열이 덮어쓰는 대신 둘 다를 담는다.
 * @note @c CONVERT 는 색인들 사이에 놓여 있지만 그 배열의 색인이 아니라 크기다.
 * @{
 */
#define CONVERT    2
#define A_ORIGINAL 1
#define A_CURRENT  0
    aligntyp ualignbase[CONVERT]; /* for ualign conversion record */
/** @} */
    schar uluck, moreluck;        /* luck and luck bonus */
    /* default u.uluck is 0 except on special days (full moon: +1, Fri 13: -1,
       both: 0); equilibrium for luck timeout is changed to those values,
       but Luck max and min stay at 10+3 and -10-3 even on those days */
/**
 * @def Luck
 * @brief The hero's luck as the rules see it: what they have earned plus what they are carrying.
 * @warning Not the same as the stored luck. The stored value drifts back toward equilibrium over time
 *          while the carried bonus does not, so the two must not be confused -- and the maximum applies
 *          to the stored part only, meaning the total can exceed it.
 */
/**
 * @def Luck
 * @brief 규칙이 보는 영웅의 운. 그가 쌓은 것과 지니고 있는 것의 합.
 * @warning 저장된 운과 같지 않다. 저장된 값은 시간이 흐르며 평형으로 되돌아가지만 지닌 보너스는 그렇지 않다. 그래서 둘을 혼동해서는 안 되며, 최대값은 저장된
 *          부분에만 적용되므로 총합은 그것을 넘을 수 있다.
 */
#define Luck (u.uluck + u.moreluck)
/**
 * @def LUCKADD
 * @brief How much a luckstone shifts luck, in either direction.
 * @note One value for both because a cursed stone hurts by exactly as much as a blessed one helps.
 */
/**
 * @def LUCKADD
 * @brief 행운석이 운을 어느 방향으로든 얼마나 움직이는지.
 * @note 저주받은 돌이 해를 입히는 만큼 정확히 축복받은 돌이 도움을 주므로 둘에 하나의 값이다.
 */
#define LUCKADD    3  /* value of u.moreluck when carrying luck stone;
                       * +3 when blessed or uncursed, -3 when cursed */
/**
 * @name Luck bounds
 * @brief The limits on earned luck.
 * @note Symmetrical, and unaffected by the special days. The existing comment records that a full moon
 *       moves the equilibrium luck decays toward, not these bounds -- so a lucky day does not raise the
 *       ceiling.
 * @{
 */
/**
 * @name 운의 한계
 * @brief 쌓은 운에 대한 상한과 하한.
 * @note 대칭이며, 특별한 날에 영향받지 않는다. 기존 주석은 보름달이 운이 되돌아가는 평형점을 움직이는 것이지 이 한계를 움직이는 것이 아니라고 기록하고 있다.
 *       그래서 운수 좋은 날이 천장을 올리지는 않는다.
 * @{
 */
#define LUCKMAX   10  /* maximum value of u.uluck */
#define LUCKMIN (-10) /* minimum value of u.uluck */
/** @} */
    schar uhitinc;
    schar udaminc;
    schar uac;
#define AC_MAX    99  /* abs(u.uac) <= 99; likewise for monster AC */
    uchar uspellprot;        /* protection by SPE_PROTECTION */
    uchar usptime;           /* #moves until uspellprot-- */
    uchar uspmtime;          /* #moves between uspellprot-- */
    int uhp, uhpmax,         /* hit points, aka health */
        uhppeak;             /* highest value of uhpmax so far */
    int uen, uenmax,         /* magical energy, aka spell power */
        uenpeak;             /* highest value of uenmax so far */
    xint16 uhpinc[MAXULEV],  /* increases to uhpmax for each level gain */
          ueninc[MAXULEV];   /* increases to uenmax for each level gain */
    int ugangr;              /* if the gods are angry at you */
    int ugifts;              /* number of artifacts bestowed */
    int ublessed, ublesscnt; /* blessing/duration from #pray */
    long umoney0;
    long uspare1;
    long uexp, urexp;        /* exper pts for gaining levels and for score */
    long ucleansed;          /* to record moves when player was cleansed */
    long usleep;             /* sleeping; monstermove you last started */
    int uinvault;
    struct monst *ustuck;    /* engulfer or grabber, maybe grabbee if Upolyd */
    struct monst *usteed;    /* mount when riding */
    unsigned ustuck_mid;     /* u.ustuck->m_id, used during save/restore */
    unsigned usteed_mid;     /* u.usteed->m_id, used during save/restore */
    long ugallop;            /* turns steed will run after being kicked */
    int urideturns;          /* time spent riding, for skill advancement */
    int umortality;          /* how many times you died */
    int ugrave_arise;    /* you die and become something aside from a ghost */
    int weapon_slots;        /* unused skill slots */
    int skills_advanced;     /* # of advances made so far */
    xint16 skill_record[P_SKILL_LIMIT]; /* skill advancements */
    struct skills weapon_skills[P_NUM_SKILLS];
    boolean twoweap;         /* KMH -- Using two-weapon combat */
    short mcham;             /* vampire mndx if shapeshifted to bat/cloud */
    short umovement;         /* instead of youmonst.movement */
    schar uachieved[N_ACH];  /* list of achievements in the order attained */
    struct monst *umonst;    /* for future conversion of &gy.youmonst to u.umonst */
}; /* end of `struct you' */


/* _hitmon_data: Info for when hero hits a monster */
/* The basic reason we need all these booleans is that we don't want
 * a "hit" message when a monster dies, so we have to know how much
 * damage it did _before_ outputting a hit message, but any messages
 * associated with the damage don't come out until _after_ outputting
 * a hit message.
 *
 * More complications:  first_weapon_hit() should be called before
 * xkilled() in order to have the gamelog messages in the right order.
 * So it can't be deferred until end of known_hitum() as was originally
 * done.
 */
struct _hitmon_data {
    int dmg;  /* damage */
    int thrown;
    int twohits; /* 0: 1 of 1; 1: 1 of 2; 2: 2 of 2 */
    int dieroll;
    struct permonst *mdat;
    boolean use_weapon_skill;
    boolean train_weapon_skill;
    int barehand_silver_rings;
    boolean silvermsg;
    boolean silverobj;
    boolean lightobj;
    int material;
    int jousting;
    boolean hittxt;
    boolean get_dmg_bonus;
    boolean unarmed;
    boolean hand_to_hand;
    boolean ispoisoned;
    boolean unpoisonmsg;
    boolean needpoismsg;
    boolean poiskilled;
    boolean already_killed;
    boolean offmap;
    boolean destroyed;
    boolean dryit;
    boolean doreturn;
    boolean retval;
    char saved_oname[BUFSZ];
};

#define Upolyd (u.umonnum != u.umonster)
#define Ugender ((Upolyd ? u.mfemale : flags.female) ? 1 : 0)

/* point px,py is adjacent to (or same location as) hero */
#define next2u(px,py) (distu((px),(py)) <= 2)
/* is monster on top of or next to hero? */
#define m_next2u(m) (distu((m)->mx,(m)->my) <= 2)
/* hero at (x,y)? */
#define u_at(x,y) ((x) == u.ux && (y) == u.uy)

#define URIGHTY (u.uhandedness == RIGHT_HANDED)
#define ULEFTY (u.uhandedness == LEFT_HANDED)
#define RING_ON_PRIMARY (ULEFTY ? uleft : uright)
#define RING_ON_SECONDARY (ULEFTY ? uright : uleft)

#endif /* YOU_H */
