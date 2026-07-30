/* NetHack 5.0	timeout.h	$NHDT-Date: 1781973089 2026/06/20 16:31:29 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.27 $ */
/* Copyright 1994, Dean Luick                                     */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file timeout.h
 * @brief Scheduled events, and what each one is attached to.
 *
 * A timer is not simply a countdown. It belongs to something -- a level, an
 * object, a monster -- and that ownership decides its fate: an object's timer
 * travels with the object, a level's timer stays behind when the hero leaves,
 * and either is cancelled when its owner ceases to exist.
 *
 * The behaviour is stored as an index into a table rather than as a pointer,
 * because timers are written to save and bones files and a pointer could not be.
 *
 * @warning Those indices are in the files, so the ordering below is data format,
 *          not convenience: inserting, removing or reordering an entry requires
 *          bumping @c EDITLEVEL, as the existing comment states. Appending is
 *          the only safe change.
 * @note The same names appear again, spelled differently, in nhlua.c so that
 *       level scripts can name timers -- both lists must be kept in step.
 */

/**
 * @file timeout.h
 * @brief 예약된 사건들과, 각각이 무엇에 붙어 있는지.
 *
 * 타이머는 단순한 카운트다운이 아니다. 무언가에 -- 레벨, 객체, 몬스터에 -- 속하며, 그
 * 소속이 운명을 정한다. 객체의 타이머는 객체와 함께 이동하고, 레벨의 타이머는 영웅이
 * 떠나도 그 자리에 남으며, 어느 쪽이든 주인이 사라지면 취소된다.
 *
 * 행동은 포인터가 아니라 표에 대한 색인으로 저장된다. 타이머가 저장 파일과 본즈 파일에
 * 기록되는데 포인터는 쓸 수 없기 때문이다.
 *
 * @warning 그 색인들이 파일 안에 들어간다. 따라서 아래 순서는 편의가 아니라 데이터
 *          형식이다. 항목을 끼워 넣거나 없애거나 순서를 바꾸면 기존 주석이 말하듯
 *          @c EDITLEVEL 을 올려야 한다. 뒤에 덧붙이는 것만이 안전한 변경이다.
 * @note 같은 이름들이 철자를 달리해 nhlua.c 에도 나타난다. 레벨 스크립트가 타이머를
 *       이름으로 지칭할 수 있게 하기 위함이며, 두 목록은 항상 맞춰야 한다.
 */

#ifndef TIMEOUT_H
#define TIMEOUT_H

/**
 * @brief What a timer does when it comes due.
 * @param arg   The thing the timer was attached to, untyped.
 * @param when  The turn at which it fired.
 * @warning May destroy what it was attached to, and may create or cancel other
 *          timers, so the list cannot be iterated naively while firing.
 */
/**
 * @brief 타이머가 만기되었을 때 하는 일.
 * @param arg   타이머가 붙어 있던 대상. 타입이 없다.
 * @param when  발동한 턴.
 * @warning 붙어 있던 대상을 파괴할 수 있고, 다른 타이머를 만들거나 취소할 수도 있다.
 *          그래서 발동 중에 목록을 순진하게 순회할 수 없다.
 */
/* generic timeout function */
typedef void (*timeout_proc)(ANY_P *, long);

/* kind of timer */
enum timer_type {
    TIMER_NONE = 0,
    TIMER_LEVEL = 1,   /* event specific to level [melting ice] */
    TIMER_GLOBAL = 2,  /* event follows current play [not used] */
    TIMER_OBJECT = 3,  /* event follows an object [various] */
    TIMER_MONSTER = 4, /* event follows a monster [not used] */
    NUM_TIMER_KINDS    /* 5 */
};

/* save/restore timer ranges */
#define RANGE_LEVEL 0  /* save/restore timers staying on level */
#define RANGE_GLOBAL 1 /* save/restore timers following global play */

/*
 * Timeout functions.  Add an enum here, then put it in the table
 * in timeout.c.  "One more level of indirection will fix everything."
 * Also add it to timerstr[] in nhl_get_timertype(nhlua.c); the entries
 * there match these but are spelled differently.
 *
 * Note:  if any are inserted, removed, or reordered then EDITLEVEL
 * needs to be incremented because timeout indices get written into save
 * and bones files if any timers are present while saving.  (Adding new
 * ones at the end isn't restricted this way since new indices won't be
 * present in old data.)
 */
enum timeout_types {
    ROT_ORGANIC = 0, /* for buried organics */
    ROT_CORPSE,
    REVIVE_MON,
    ZOMBIFY_MON,
    BURN_OBJECT,
    HATCH_EGG,
    FIG_TRANSFORM,
    SHRINK_GLOB,
    MELT_ICE_AWAY,

    NUM_TIME_FUNCS
};

#define timer_is_pos(ttype) ((ttype) == MELT_ICE_AWAY)
#define timer_is_obj(ttype) ((ttype) == ROT_ORGANIC      \
                             || (ttype) == ROT_CORPSE    \
                             || (ttype) == REVIVE_MON    \
                             || (ttype) == ZOMBIFY_MON   \
                             || (ttype) == BURN_OBJECT   \
                             || (ttype) == HATCH_EGG     \
                             || (ttype) == FIG_TRANSFORM \
                             || (ttype) == SHRINK_GLOB)

/* used in timeout.c */
typedef struct fe {
    struct fe *next;          /* next item in chain */
    long timeout;             /* when we time out */
    unsigned long tid;        /* timer ID */
    short kind;               /* kind of use */
    short func_index;         /* what to call when we time out */
    anything arg;             /* pointer to timeout argument */
    Bitfield(needs_fixup, 1); /* does arg need to be patched? */
} timer_element;

#endif /* TIMEOUT_H */
