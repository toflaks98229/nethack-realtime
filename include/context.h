/* NetHack 5.0	context.h	$NHDT-Date: 1781973077 2026/06/20 16:31:17 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.61 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the context structure make sure you increment EDITLEVEL in */
/* patchlevel.h if needed. */

/**
 * @file context.h
 * @brief Things the game is in the middle of.
 *
 * NetHack's commands are not all instantaneous. Digging a hole, eating a meal, engraving a message, opening
 * a tin -- each takes several turns, during which the hero may be interrupted, may move away, may be
 * attacked. So the game has to remember not only its state but its unfinished business, and this file is
 * where that unfinished business lives.
 *
 * Each in-progress activity gets its own small structure, and they share a shape: what is being worked on,
 * how much effort has gone in, how much is still required. That shape is what lets an activity be resumed,
 * and also what lets it be abandoned -- a partly eaten meal and a partly dug hole both have to survive being
 * walked away from.
 *
 * Alongside those are the running counters that belong to no one command: the next identifier to give a
 * monster, when the next attribute check falls due, when clairvoyance will strike next.
 *
 * @note Several of these hold both a pointer to an object and that object's identifier. The pointer is what
 *       is used; the identifier exists because a pointer cannot be saved, so the pair is how a half-finished
 *       action survives saving and restoring.
 * @warning The whole structure is saved with the game, so its layout is part of the save format. The comment
 *          at the top of the file is a standing instruction: changing this means bumping the save format
 *          version.
 */

/**
 * @file context.h
 * @brief 게임이 하던 중인 일들.
 *
 * NetHack 의 명령이 모두 순간적인 것은 아니다. 구멍을 파는 것, 식사를 하는 것, 글을 새기는 것, 통조림을 여는 것. 각각이 여러 턴에 걸치고, 그 동안 영웅은 방해받을
 * 수도, 자리를 옮길 수도, 공격받을 수도 있다. 그래서 게임은 자기 상태만이 아니라 끝내지 못한 일도 기억해야 하며, 이 파일이 그 끝내지 못한 일이 사는 곳이다.
 *
 * 진행 중인 활동마다 작은 구조체가 하나씩 있고, 그것들은 같은 형태를 공유한다. 무엇을 대상으로 하는지, 얼마나 공을 들였는지, 얼마나 더 필요한지. 그 형태가 활동을
 * 이어 갈 수 있게 하는 것이고, 또한 포기할 수 있게 하는 것이다. 반쯤 먹은 식사와 반쯤 판 구멍은 둘 다 자리를 떠나는 것을 견뎌야 한다.
 *
 * 그 곁에는 어느 명령에도 속하지 않는 진행 계수기들이 있다. 몬스터에게 줄 다음 식별자, 다음 능력치 점검이 언제인지, 투시가 다음에 언제 찾아올지.
 *
 * @note 이들 중 여럿은 물건에 대한 포인터와 그 물건의 식별자를 함께 담는다. 쓰이는 것은 포인터다. 식별자가 있는 것은 포인터를 저장할 수 없기 때문이며, 그 짝이 반쯤
 *       끝난 행동이 저장과 복원을 견디는 방식이다.
 * @warning 이 구조체 전체가 게임과 함께 저장되므로 그 배치가 저장 형식의 일부다. 파일 맨 위의 주석은 상시 지시다. 이것을 바꾸는 것은 저장 형식 버전을 올린다는
 *          뜻이다.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

/**
 * @def CONTEXTVERBSZ
 * @brief How long a verb describing an interrupted action may be.
 * @note A verb is kept as text rather than a code because it is shown back to the player when an action is
 *       resumed or abandoned -- "you continue disrobing" -- and the wording differs by what was being taken
 *       off.
 */
/**
 * @def CONTEXTVERBSZ
 * @brief 중단된 행동을 기술하는 동사가 얼마나 길 수 있는지.
 * @note 동사가 코드가 아니라 글로 보관되는 것은, 행동이 이어지거나 포기될 때 플레이어에게 되보여지기 때문이다. "옷 벗기를 계속한다." 그리고 무엇을 벗던 중이었는지에
 *       따라 표현이 달라진다.
 */
#define CONTEXTVERBSZ 30

/**
 * @brief Hints the game offers once and then stops offering.
 * @note Each is shown the first time the situation arises and never again, so what is recorded is that it has
 *       been shown. That is why these are bit positions in one word rather than separate flags.
 */
/**
 * @brief 게임이 한 번 제시하고 그 뒤로는 제시하지 않는 조언들.
 * @note 각각은 그 상황이 처음 생길 때 보여지고 다시는 보여지지 않는다. 그래서 기록되는 것은 그것이 보여졌다는 사실이다. 이들이 별개의 플래그가 아니라 하나의 워드 안의
 *       비트 위치인 이유가 그것이다.
 */
enum nh_tips {
    TIP_ENHANCE = 0, /* #enhance */
    TIP_SWIM,        /* walking into water */
    TIP_UNTRAP_MON,  /* walking into trapped peaceful */
    TIP_GETPOS,      /* getpos/farlook */

    NUM_TIPS
};

/*
 * The context structure houses things that the game tracks
 * or adjusts during the game, to preserve game state or context.
 *
 * The entire structure is saved with the game.
 *
 */

/**
 * @brief Digging in progress.
 *
 * Effort accumulates rather than a countdown running, because the hero's tool, strength and interruptions all
 * change how fast the hole grows. So the level and square are recorded too: walking away and coming back to a
 * different wall must not credit the previous digging.
 *
 * @note @c chew distinguishes digging with a pick from gnawing through rock as a monster, which progresses
 *       differently and is described differently.
 * @note The last time digging happened is kept so that resuming after a long absence can decide whether the
 *       partial hole is still there.
 */
/**
 * @brief 진행 중인 굴착.
 *
 * 남은 시간이 줄어드는 것이 아니라 공이 누적된다. 영웅의 도구, 힘, 방해가 모두 구멍이 자라는 속도를 바꾸기 때문이다. 그래서 레벨과 칸도 함께 기록된다. 자리를 떠나
 * 다른 벽으로 돌아온 것이 이전의 굴착으로 인정되어서는 안 된다.
 *
 * @note @c chew 는 곡괭이로 파는 것과 몬스터로서 암반을 갉는 것을 구별한다. 진행 방식이 다르고 기술되는 방식도 다르다.
 * @note 마지막으로 판 시점이 보관되므로, 오래 떠났다가 이어 갈 때 그 반쯤 판 구멍이 여전히 있는지 판단할 수 있다.
 */
struct dig_info { /* apply.c, hack.c */
    int effort;
    d_level level;
    coord pos;
    long lastdigtime;
    boolean down, chew, warned, quiet;
};

/**
 * @brief A tin being opened.
 * @note Time spent and time required are separate because how long a tin takes depends on the tool used, and
 *       that is decided once at the start -- so a tin opened with a tin opener does not become slower if the
 *       opener is dropped partway.
 */
/**
 * @brief 열고 있는 통조림.
 * @note 들인 시간과 필요한 시간이 따로 있다. 통조림이 얼마나 걸리는지가 쓰는 도구에 달려 있고 그것이 처음에 한 번 정해지기 때문이다. 그래서 통조림 따개로 열던 통조림이
 *       도중에 따개를 놓았다고 느려지지는 않는다.
 */
struct tin_info {
    struct obj *tin;
    unsigned o_id; /* o_id of tin in save file */
    int usedtime, reqtime;
};

/**
 * @brief A spellbook being studied.
 * @note A countdown rather than accumulated effort, because the difficulty is fixed by the book's level when
 *       reading begins and nothing can speed it up.
 * @warning Interrupting the study of a difficult book is not free, so the remaining count exists to be
 *          resumed rather than restarted.
 */
/**
 * @brief 공부하고 있는 주문서.
 * @note 누적되는 공이 아니라 남은 시간이다. 난이도가 읽기 시작할 때 책의 등급으로 고정되고 그것을 빠르게 할 수 있는 것이 없기 때문이다.
 * @warning 어려운 책의 공부를 중단하는 것은 공짜가 아니다. 그래서 남은 수치는 다시 시작되는 것이 아니라 이어지기 위해 존재한다.
 */
struct book_info {
    struct obj *book; /* last/current book being xscribed */
    unsigned o_id;    /* o_id of book in save file */
    schar delay;      /* moves left for this spell */
};

/**
 * @brief Armour being taken off, which may be several pieces.
 *
 * Removing a suit means first removing the cloak over it, so what is stored is the whole set the hero intends
 * to remove and which piece is being worked on now. That is why there are two masks rather than one.
 *
 * @note The verb is kept as text so the message on resuming matches what was actually being done.
 * @note @c cancelled_don covers the case where the hero was in the middle of putting armour @e on and that was
 *       interrupted -- the half-finished donning has to be undone rather than completed.
 */
/**
 * @brief 벗고 있는 갑옷. 여러 벌일 수 있다.
 *
 * 갑옷 몸통을 벗으려면 그 위의 겉옷을 먼저 벗어야 한다. 그래서 저장되는 것은 영웅이 벗으려 하는 전체 묶음과 지금 다루고 있는 것이 무엇인지다. 마스크가 하나가 아니라 둘인
 * 이유가 그것이다.
 *
 * @note 동사가 글로 보관되므로, 이어 갈 때의 메시지가 실제로 하던 일과 맞는다.
 * @note @c cancelled_don 은 영웅이 갑옷을 @e 입는 도중이었고 그것이 중단된 경우를 담는다. 반쯤 입은 것은 마무리되는 것이 아니라 되돌려져야 한다.
 */
struct takeoff_info {
    long mask;
    long what;
    int delay;
    boolean cancelled_don;
    char disrobing[CONTEXTVERBSZ + 1];
};

/**
 * @brief A meal in progress.
 *
 * Eating is the most interruptible action in the game, and the most consequential to interrupt: the hero may
 * choke, may be satiated, may have started on something they should not finish. So the record holds not only
 * progress but the circumstances at the start.
 *
 * @note @c canchoke is captured at the beginning because choking depends on having been satiated when the meal
 *       started, not on being satiated now -- the meal is what changed that.
 * @note @c doreset asks for the meal to end at the end of this turn rather than immediately, so that whatever
 *       interrupted it can finish first.
 * @warning A tin does not use this. Tins have their own record, and this one holds nothing while a tin is being
 *          opened, as the existing comment sets out.
 */
/**
 * @brief 진행 중인 식사.
 *
 * 먹기는 게임에서 가장 방해받기 쉬운 행동이며, 방해했을 때의 결과가 가장 큰 행동이다. 영웅은 목이 막힐 수도, 배가 부를 수도, 끝내면 안 되는 것을 시작했을 수도 있다. 그래서
 * 이 기록은 진행뿐 아니라 시작 당시의 정황도 담는다.
 *
 * @note @c canchoke 는 시작할 때 붙잡아 둔다. 목이 막히는 것은 지금 배가 부른지가 아니라 식사가 시작될 때 배가 불렀는지에 달려 있기 때문이다. 그것을 바꾼 것이 그 식사다.
 * @note @c doreset 은 식사를 즉시가 아니라 이번 턴 끝에 끝내 달라고 요청한다. 그래서 그것을 방해한 것이 먼저 마무리될 수 있다.
 * @warning 통조림은 이것을 쓰지 않는다. 통조림에는 자기 기록이 있고, 기존 주석이 밝히듯 통조림을 여는 동안 이것은 아무것도 담지 않는다.
 */
struct victual_info {
    struct obj *piece; /* the thing being eaten, or last thing that
                        * was partially eaten, unless that thing was
                        * a tin, which uses the tin structure above,
                        * in which case this should be 0 */
    unsigned o_id;     /* o_id of food object in save file */
    /* doeat() initializes these when piece is valid */
    int usedtime,          /* turns spent eating */
        reqtime;           /* turns required to eat */
    int nmod;              /* coded nutrition per turn */
    Bitfield(canchoke, 1); /* was satiated at beginning */

    /* start_eating() initializes these */
    Bitfield(fullwarn, 1); /* have warned about being full */
    Bitfield(eating, 1);   /* victual currently being eaten */
    Bitfield(doreset, 1);  /* stop eating at end of turn */
};

/**
 * @brief An engraving in progress.
 *
 * The whole text is held along with a pointer into it, because engraving happens a few characters at a time and
 * an interrupted engraving leaves behind exactly what was written so far. That partial result is visible on the
 * map, so where the writing stopped has to be exact.
 *
 * @note The stylus is recorded because it decides both the kind of engraving and how fast it goes, and because
 *       losing it partway must stop the work.
 */
/**
 * @brief 진행 중인 새김.
 *
 * 전체 글이 그 안을 가리키는 포인터와 함께 보관된다. 새김이 몇 글자씩 이뤄지고, 중단된 새김은 그때까지 쓰인 것을 정확히 남기기 때문이다. 그 부분적인 결과가 지도 위에 보이므로,
 * 쓰기가 어디서 멈췄는지가 정확해야 한다.
 *
 * @note 필기구가 기록되는 것은, 그것이 새김의 종류와 속도를 함께 정하고, 도중에 그것을 잃으면 작업이 멈춰야 하기 때문이다.
 */
struct engrave_info {
    char text[BUFSZ];   /* actual text being engraved - doengrave() handles all
                           the possible mutations of this */
    char *nextc;        /* next character(s) in text[] to engrave */
    struct obj *stylus; /* object doing the writing */
    xint8 type;         /* type of engraving (DUST, MARK, etc) */
    coord pos;          /* location the engraving is being placed on */
    int actionct;       /* nth turn spent engraving */
};

/**
 * @brief What the hero is being warned about, and why.
 *
 * Warning of a specific kind of monster can come from an object being carried or from the hero's current form,
 * and the two must be kept apart because they end at different times: dropping the object ends one, and the
 * polymorph ending ends the other.
 *
 * @note The form's warning can be about one exact species rather than a class, which is why a species pointer is
 *       kept as well as the class flags -- and an index alongside it, since a pointer cannot be saved.
 */
/**
 * @brief 영웅이 무엇에 대해 경고받고 있는지, 그리고 왜인지.
 *
 * 특정 종류의 몬스터에 대한 경고는 지니고 있는 물건에서 올 수도, 영웅의 현재 형태에서 올 수도 있다. 둘을 따로 두어야 하는 것은 끝나는 시점이 다르기 때문이다. 물건을 버리면
 * 하나가 끝나고, 변신이 끝나면 다른 하나가 끝난다.
 *
 * @note 형태에서 오는 경고는 계열이 아니라 정확한 한 종족에 관한 것일 수 있다. 그래서 계열 플래그와 함께 종족 포인터가 보관되며, 포인터는 저장될 수 없으므로 그 곁에 색인도
 *       함께 있다.
 */
struct warntype_info {
    unsigned long obj;        /* object warn_of_mon monster type M2 */
    unsigned long polyd;      /* warn_of_mon monster type M2 due to poly */
    struct permonst *species; /* particular species due to poly */
    short speciesidx; /* index of above in mons[] (for save/restore) */
};

/**
 * @brief The last monster a polearm was aimed at.
 * @note Remembered so that applying the polearm again can offer the same target, since a polearm strikes at a
 *       distance and re-choosing every time would be tedious.
 */
/**
 * @brief 장병기로 마지막에 노린 몬스터.
 * @note 장병기를 다시 쓸 때 같은 대상을 제시할 수 있도록 기억된다. 장병기는 떨어진 곳을 치므로, 매번 다시 고르게 하면 번거롭다.
 */
struct polearm_info {
    struct monst *hitmon; /* the monster we tried to hit last */
    unsigned m_id;        /* monster id of hitmon, in save file */
};

/**
 * @brief The two halves of the most recently split stack of objects.
 *
 * When a stack is divided, code holding a pointer to the original may now be looking at the wrong half. Recording
 * both halves lets such code find out which is which afterwards, rather than each caller having to arrange to be
 * told.
 *
 * @note Identifiers rather than pointers, because one of the halves may already have been freed by the time this
 *       is consulted.
 */
/**
 * @brief 가장 최근에 나뉜 물건 묶음의 두 조각.
 *
 * 묶음이 나뉘면, 원래 것을 가리키는 포인터를 쥔 코드가 이제 틀린 쪽을 보고 있을 수 있다. 두 조각을 기록해 두면 그런 코드가 나중에 어느 쪽이 어느 쪽인지 알아낼 수 있다. 호출자마다
 * 알림을 받도록 마련할 필요가 없어진다.
 *
 * @note 포인터가 아니라 식별자다. 이것이 참조되는 시점에 두 조각 중 하나가 이미 해제되었을 수 있기 때문이다.
 */
struct obj_split {
    unsigned parent_oid, /* set: splitobj(),         */
             child_oid;  /* reset: clear_splitobjs() */
};

/**
 * @brief State for the tribute content -- the novels and the quotes.
 * @note Carries its own size so that a future version reading an older save can skip past it without knowing what
 *       it contained. That is a deliberate hedge: the tribute content is the part most likely to change shape.
 */
/**
 * @brief 헌정 콘텐츠 -- 소설과 인용문 -- 를 위한 상태.
 * @note 자기 크기를 함께 지닌다. 그래서 미래의 판본이 예전 저장을 읽을 때 그 안에 무엇이 있었는지 모르고도 건너뛸 수 있다. 의도적인 대비다. 헌정 콘텐츠는 형태가 바뀔 가능성이
 *       가장 큰 부분이다.
 */
struct tribute_info {
    size_t tributesz;       /* make it possible to skip this in future */
    boolean enabled;        /* Do we have tributes turned on? */
    Bitfield(bookstock, 1); /* Have we stocked the book? */
    Bitfield(Deathnotice,1);    /* Did Death notice the book? */
    /* Markers for other tributes can go here */
    /* 30 free bits */
};

/**
 * @brief Which passages of a novel have not yet been read.
 *
 * Reading a novel gives a random passage, and the point is not to repeat one. So rather than choosing freshly each
 * time, a subset of the available passages is drawn once and then drawn down -- which is what makes repeated
 * reading feel like progressing through a book.
 *
 * @note The novel is identified so that switching books starts a new subset; the identity is what tells the game
 *       the previous list no longer applies.
 * @warning The subset is redrawn from scratch when it runs out or when the reader changes books, so it is not a
 *          record of everything read. The existing comment is explicit that a new subset may leave out passages
 *          the previous one also left out -- there is no guarantee of eventually seeing all of them.
 */
/**
 * @brief 소설에서 아직 읽지 않은 대목이 무엇인지.
 *
 * 소설을 읽으면 무작위 대목이 나오고, 요점은 같은 것을 되풀이하지 않는 것이다. 그래서 매번 새로 고르는 대신, 가능한 대목 중 일부를 한 번 뽑아 두고 그것을 소진해 나간다. 되풀이해
 * 읽는 것이 책을 읽어 나가는 느낌이 되게 하는 것이다.
 *
 * @note 소설이 식별되므로 책을 바꾸면 새 부분집합이 시작된다. 그 정체가 게임에게 이전 목록이 더는 적용되지 않음을 알려 주는 것이다.
 * @warning 부분집합은 소진되거나 읽는 이가 책을 바꿀 때 처음부터 다시 뽑힌다. 그래서 읽은 것 전부의 기록이 아니다. 기존 주석은 새 부분집합이 이전 것도 빠뜨린 대목을 또 빠뜨릴 수
 *          있다고 명시하고 있다. 결국 전부를 보게 된다는 보장은 없다.
 */
struct novel_tracking { /* for choosing random passage when reading novel */
    unsigned id;        /* novel oid from previous passage selection */
    int count;          /* number of passage indices available in pasg[] */
    xint8 pasg[30];     /* pasg[0..count-1] are passage indices */
    /* tribute file is allowed to have more than 30 passages for a novel;
       if it does, reading will first choose a random subset of 30 of them;
       reading all 30 or switching to a different novel and then back again
       will pick a new subset, independent of previous ones so might not
       contain all--or even any--of the ones left out of the prior subset;
       chatting with Death works the same as reading a novel except that
       sometimes he delivers one of a few hardcoded messages rather than a
       passage from the Death Quotes section of dat/tribute */
};

/**
 * @brief Which of the special prizes have been claimed, and what they turned out to be.
 *
 * The prize at the end of the mines and the one in Sokoban are placed as particular objects, and taking @e that
 * object is the achievement -- not taking another of the same kind. So the object's identity is recorded when the
 * level is built and compared when something is picked up.
 *
 * @note The object's kind is kept alongside its identity because the log entry names what was found, and by then
 *       the object may be gone.
 * @note @c minetn_reached exists purely to stop re-checking: reaching Minetown is tested every time the hero
 *       arrives somewhere, and once recorded the test can be skipped.
 */
/**
 * @brief 특별한 상 중 어느 것이 얻어졌는지, 그리고 그것이 무엇으로 드러났는지.
 *
 * 광산 끝의 상과 소코반의 상은 특정한 물건으로 놓이며, @e 그 물건을 얻는 것이 업적이다. 같은 종류의 다른 물건을 얻는 것이 아니다. 그래서 레벨이 지어질 때 그 물건의 정체가 기록되고
 * 무언가를 집을 때 비교된다.
 *
 * @note 물건의 종류가 정체와 함께 보관되는 것은, 로그 항목이 무엇을 찾았는지 이름을 적고 그때쯤이면 그 물건이 사라졌을 수 있기 때문이다.
 * @note @c minetn_reached 는 순전히 재검사를 막기 위해 있다. 광산 마을에 도달했는지는 영웅이 어딘가에 도착할 때마다 검사되며, 한 번 기록되면 그 검사를 건너뛸 수 있다.
 */
struct achievement_tracking {
    unsigned mines_prize_oid,   /* luckstone->o_id */
             soko_prize_oid,    /* {bag or amulet}->o_id */
             castle_prize_old;  /* wand->o_id; not yet implemented */
    /* record_achievement() wants the item type for livelog() event */
    short    mines_prize_otyp,  /* luckstone */
             soko_prize_otyp,   /* bag of holding or amulet of reflection */
             castle_prize_otyp; /* strange object (someday wand of wishing) */
    boolean minetn_reached;     /* avoid redundant checking for town entry */
};

/**
 * @brief Running tallies of monsters encountered, for the end-of-game account.
 * @note Counted rather than derived, because a monster seen and then killed leaves nothing to count afterwards.
 */
/**
 * @brief 마주친 몬스터의 누적 집계. 게임 종료 시의 보고를 위한 것.
 * @note 유도되지 않고 세어진다. 보았다가 죽인 몬스터는 나중에 셀 것을 남기지 않기 때문이다.
 */
struct lifelists {
    long total_seen_upclose;    /* count of critters seen up close */
    long total_photographed;    /* count of critters photographed (tourists) */
};

/**
 * @brief Everything the game is in the middle of, gathered in one place.
 *
 * Two sorts of thing sit side by side here. The in-progress activities are the sub-structures above. The plain
 * fields are the running state that belongs to no single command: the next identifier to hand out, when the next
 * periodic check is due, whether it is currently the monsters' turn.
 *
 * @note @c ident is the source of every monster's identity and only ever increases, so an identifier is never
 *       reused within a game -- which is what lets a saved reference be resolved unambiguously.
 * @note @c run holds a manner of moving rather than a distance: which of the several run and travel commands is in
 *       effect, since each stops for different reasons.
 * @note @c mon_moving is how a great deal of code tells whether it is acting for the hero or for a monster, since
 *       the same routines serve both.
 * @note @c resume_wish exists because a wish prompt is one of the few places the game can be left mid-question,
 *       and the wish must still be granted on return.
 * @warning @c move and @c mv are distinct despite the names, as are @c travel and @c travel1 -- the second of each
 *          pair marks the first step, which is treated differently from the rest.
 */
/**
 * @brief 게임이 하던 중인 모든 것을 한곳에 모은 것.
 *
 * 여기에는 두 종류가 나란히 있다. 진행 중인 활동은 위의 하위 구조체들이다. 평범한 필드들은 어느 한 명령에도 속하지 않는 진행 상태다. 다음에 내줄 식별자, 다음 주기적 점검이 언제인지,
 * 지금이 몬스터의 차례인지.
 *
 * @note @c ident 는 모든 몬스터의 정체가 나오는 곳이며 오직 증가만 한다. 그래서 한 게임 안에서 식별자가 재사용되는 일이 결코 없고, 그것이 저장된 참조를 모호함 없이 풀 수 있게
 *       하는 것이다.
 * @note @c run 은 거리가 아니라 이동 방식을 담는다. 여러 달리기와 여행 명령 중 어느 것이 유효한지. 각각이 서로 다른 이유로 멈추기 때문이다.
 * @note @c mon_moving 은 아주 많은 코드가 자신이 영웅을 위해 움직이는지 몬스터를 위해 움직이는지 판별하는 방식이다. 같은 루틴이 둘 다를 맡기 때문이다.
 * @note @c resume_wish 가 있는 것은, 소원 프롬프트가 게임이 질문 중간에 떠날 수 있는 몇 안 되는 곳이고 돌아왔을 때 그 소원이 여전히 이뤄져야 하기 때문이다.
 * @warning @c move 와 @c mv 는 이름이 비슷하지만 서로 다르며, @c travel 과 @c travel1 도 그렇다. 각 짝의 두 번째는 첫 걸음을 표시하며, 그것은 나머지와 다르게 다뤄진다.
 */
struct context_info {
    unsigned ident;         /* social security number for each monster */
    unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
    unsigned run;           /* 0: h (etc), 1: H (etc), 2: fh (etc),
                             * 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF-,
                             * 8: travel */
    unsigned startingpet_mid; /* monster id number for initial pet */
    int current_fruit;      /* fruit->fid corresponding to svp.pl_fruit[] */
    int mysteryforce;       /* adjusts how often "mysterious force" kicks in */
    int rndencode;          /* randomized escape sequence introducer */
    int startingpet_typ;    /* monster type for initial pet */
    int warnlevel;          /* threshold (digit) to warn about unseen mons */
    long next_attrib_check; /* next attribute check */
    long seer_turn;         /* when random clairvoyance will next kick in */
    long snickersnee_turn;  /* Snickersnee last used to distance attack */
    long stethoscope_seq;   /* when a stethoscope was last used; first use
                             * during a move takes no time, second uses move */
    boolean travel;  /* find way automatically to u.tx,u.ty */
    boolean travel1; /* first travel step */
    boolean forcefight;
    boolean nopick; /* do not pickup objects (as when running) */
    boolean made_amulet;
    boolean mon_moving; /* monsters' turn to move */
    boolean move;
    boolean mv;
    boolean bypasses;    /* bypass flag is set on at least one fobj */
    boolean door_opened; /* set to true if door was opened during test_move */
    boolean resume_wish; /* game was exited while in wish prompt */
    unsigned long tips;
    struct dig_info digging;
    struct victual_info victual;
    struct engrave_info engraving;
    struct tin_info tin;
    struct book_info spbook;
    struct takeoff_info takeoff;
    struct warntype_info warntype;
    struct polearm_info polearm;
    struct obj_split objsplit; /* track most recently split object stack */
    struct tribute_info tribute;
    struct novel_tracking novel;
    struct achievement_tracking achieveo;
    struct lifelists lifelist;
    char jingle[5 + 1];
};

#endif /* CONTEXT_H */
