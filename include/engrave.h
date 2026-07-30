/* NetHack 5.0	engrave.h	$NHDT-Date: 1781973079 2026/06/20 16:31:19 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.22 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file engrave.h
 * @brief An engraving, and the three versions of its text.
 *
 * What is written, what the hero remembers reading, and what it looked like
 * before it wore away are separate texts, because they diverge: an engraving
 * degrades in place while the hero's memory of it does not, so the map can show
 * a mangled Elbereth that the player still believes is intact.
 *
 * How it degrades depends on what it was written with, which is why the medium
 * is stored rather than just the words.
 *
 * @note The text is allocated after the structure rather than separately, which
 *       is what @c engr_text_space() addresses -- so an engraving is one
 *       allocation and @c engr_alloc exists for saving it.
 * @warning Not every square can hold an engraving; @c spot_shows_engravings()
 *          decides, and writing where it says no would leave text nothing
 *          displays.
 */

/**
 * @file engrave.h
 * @brief 각인과, 그 글의 세 가지 판본.
 *
 * 실제로 쓰인 것, 영웅이 읽었다고 기억하는 것, 닳기 전의 모습은 서로 다른 글이다.
 * 이들이 갈라지기 때문이다. 각인은 제자리에서 상해 가지만 영웅의 기억은 그렇지 않으므로,
 * 플레이어가 여전히 온전하다고 믿는 망가진 Elbereth 가 지도에 보일 수 있다.
 *
 * 어떻게 상하는지는 무엇으로 썼는지에 달려 있다. 글자만이 아니라 필기 수단까지 저장하는
 * 이유가 그것이다.
 *
 * @note 글은 별도로 할당되지 않고 구조체 뒤에 이어 할당된다. @c engr_text_space() 가
 *       가리키는 곳이며, 그래서 각인 하나가 한 번의 할당이고 @c engr_alloc 이 저장을
 *       위해 존재한다.
 * @warning 모든 칸이 각인을 담을 수 있는 것은 아니다. @c spot_shows_engravings() 가
 *          판단하며, 아니라고 하는 곳에 쓰면 아무것도 표시하지 않는 글이 남는다.
 */

#ifndef ENGRAVE_H
#define ENGRAVE_H

/**
 * @brief Which of an engraving's three texts is meant.
 * @note They differ because the writing wears away while the hero's memory of it
 *       does not; @c pristine_text is what it said before any of that.
 */
/**
 * @brief 각인의 세 글 중 무엇을 뜻하는지.
 * @note 글씨는 닳아 가지만 영웅의 기억은 그렇지 않아서 서로 달라진다.
 *       @c pristine_text 는 그 모든 일이 있기 전의 내용이다.
 */
enum engraving_texts {
    actual_text,
    remembered_text,
    pristine_text,
    text_states
};

#define engr_text_space(ep) ((char *) ((ep) + 1))

struct engr {
    struct engr *nxt_engr;
    char *engr_txt[text_states];
    coordxy engr_x, engr_y;
    unsigned engr_szeach;  /* length of text including trailing NUL */
    unsigned engr_alloc; /* for save & restore; not length of text */
    long engr_time;    /* moment engraving was (will be) finished */
    xint8 engr_type;
#define DUST 1
#define ENGRAVE 2
#define BURN 3
#define MARK 4
#define ENGR_BLOOD 5
#define HEADSTONE 6
#define N_ENGRAVE 6
    Bitfield(guardobjects, 1); /* if engr_txt is "Elbereth", it is effective
                                * against monsters when an object is present
                                * even when hero isn't (so behaves similarly
                                * to how Elbereth did in 3.4.3) */
    Bitfield(nowipeout, 1);    /* this engraving will not degrade */
    Bitfield(eread, 1);        /* refers to the engaving text: read or felt */
    Bitfield(erevealed, 1);    /* refers to engraving map symbol: revealed */
    /* 4 free bits */
};

#define newengr(lth) \
    (struct engr *) alloc((unsigned) (lth) + (unsigned) sizeof (struct engr))
#define dealloc_engr(engr) free((genericptr_t) (engr))

#define engraving_to_defsym(ep) \
    ((levl[(ep)->engr_x][(ep)->engr_y].typ == CORR) ? S_engrcorr : S_engroom)

#define spot_shows_engravings(x,y) \
    (levl[(x)][(y)].typ == CORR         \
     || levl[(x)][(y)].typ == ICE       \
     || levl[(x)][(y)].typ == ROOM)

#endif /* ENGRAVE_H */
