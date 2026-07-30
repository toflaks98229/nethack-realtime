/* NetHack 5.0	mextra.h	$NHDT-Date: 1781973082 2026/06/20 16:31:22 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.50 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MEXTRA_H
#define MEXTRA_H

#ifndef ALIGN_H
#include "align.h"
#endif

/**
 * @file mextra.h
 * @brief What a few monsters need to remember, kept off the monsters that do not.
 *
 * Most monsters need nothing beyond their position, health and disposition. A handful need
 * a great deal more: a shopkeeper remembers a bill and a customer's name, a vault guard the
 * corridor it dug and where it was leading the hero, a pet its training and when it last
 * ate. Putting those fields on every monster would cost a level's worth of memory to serve
 * two or three of them.
 *
 * So a monster carries at most a pointer to a bag of extensions, and each extension is
 * itself allocated only when needed. A monster with no extension has no bag at all, which
 * is why every access has to be guarded -- reaching for a shopkeeper's bill on a newt would
 * follow a null pointer twice over.
 *
 * That is what the paired macros at the foot of the file are for. The @c has_ form asks
 * safely; the bare form assumes the answer is yes.
 *
 * @note Each extension records the identity of the monster it belongs to. It is redundant
 *       when everything is well, and that is the point: it makes a bag attached to the
 *       wrong monster detectable rather than silently plausible.
 * @warning An extension is not simply a struct. Adding one means teaching creation,
 *          copying, freeing, saving and restoring about it; the numbered list below is that
 *          checklist, and skipping a step yields a leak or a corrupt save rather than a
 *          compile error.
 */

/**
 * @file mextra.h
 * @brief 몇몇 몬스터가 기억해야 하는 것. 그럴 필요가 없는 몬스터에게서는 떼어 놓는다.
 *
 * 대부분의 몬스터는 위치, 체력, 성향 말고는 아무것도 필요하지 않다. 소수는 훨씬 많은 것을 필요로 한다. 상점 주인은
 * 계산서와 손님의 이름을, 금고 경비는 자기가 판 통로와 영웅을 어디로 이끌던 중인지를, 애완동물은 훈련도와 마지막으로
 * 먹은 때를 기억한다. 그 필드들을 모든 몬스터에 두면 두세 마리를 위해 레벨 하나 분량의 메모리를 쓰게 된다.
 *
 * 그래서 몬스터는 확장 꾸러미를 가리키는 포인터를 최대 하나 지니며, 각 확장 자체도 필요할 때만 할당된다. 확장이 없는
 * 몬스터에게는 꾸러미조차 없다. 그래서 모든 접근이 보호되어야 한다. 뉴트에게서 상점 주인의 계산서를 찾으면 널
 * 포인터를 두 번 따라가게 된다.
 *
 * 파일 끝의 짝지어진 매크로들이 그것을 위한 것이다. @c has_ 형은 안전하게 묻고, 맨 형은 답이 그렇다고 가정한다.
 *
 * @note 각 확장은 자신이 속한 몬스터의 정체를 기록한다. 모든 것이 정상일 때는 잉여이고, 그것이 요점이다. 잘못된
 *       몬스터에 붙은 꾸러미를 조용히 그럴듯한 것이 아니라 감지할 수 있는 것으로 만든다.
 * @warning 확장은 단순한 구조체가 아니다. 하나를 더하는 것은 생성, 복사, 해제, 저장, 복원에 그것을 가르치는 일이다.
 *          아래의 번호 목록이 그 점검표이며, 한 단계를 빠뜨리면 컴파일 오류가 아니라 누수나 손상된 저장이 나온다.
 */

/*
 *  Adding new mextra structures:
 *
 *       1. Add the structure definition and any required macros in this
 *          file above the mextra struct.
 *       2. Add a pointer to your new struct to the mextra struct in this
 *          file.
 *       3. Add a referencing macro at bottom of this file after the mextra
 *          struct (see MGIVENNAME, EGD, EPRI, ESHK, EMIN, or EDOG for
 *          examples).
 *       4. If your new field isn't a pointer and requires a non-zero value
 *          on initialization, add code to init_mextra() in src/makemon.c
 *       5. Create a newXX(mtmp) function and possibly a free_XX(mtmp)
 *          function in an appropriate new or existing source file and add
 *          a prototype for it to include/extern.h.
 *
 *              void newXX(struct monst *);
 *              void free_XX(struct monst *);
 *
 *              void
 *              newXX(mtmp)
 *              struct monst *mtmp;
 *              {
 *                  if (!mtmp->mextra)
 *                      mtmp->mextra = newmextra();
 *                  if (!XX(mtmp)) {
 *                      XX(mtmp) = (struct XX *) alloc(sizeof (struct XX));
 *                      (void) memset((genericptr_t) XX(mtmp),
 *                                    0, sizeof (struct XX));
 *                  }
 *              }
 *
 *       6. Consider adding a new makemon flag MM_XX flag to include/hack.h
 *          and a corresponding change to makemon() if you require your
 *          structure to be added at monster creation time.  Initialize your
 *          struct after a successful return from makemon().
 *
 *              src/makemon.c:  if (mmflags & MM_XX) newXX(mtmp);
 *              your new code:  mon = makemon(&mons[mnum], x, y, MM_XX);
 *
 *       7. Adjust size_monst() in src/wizcmds.c appropriately.
 *       8. Adjust dealloc_mextra() in src/mon.c to clean up
 *          properly during monst deallocation.
 *       9. Adjust copy_mextra() in src/mon.c to make duplicate
 *          copies of your struct or data on another monst struct.
 *      10. Adjust restmon() in src/restore.c to deal with your
 *          struct or data during a restore.
 *      11. Adjust savemon() in src/save.c to deal with your
 *          struct or data during a save.
 */

/***
 **     formerly vault.h -- vault guard extension
 */
/**
 * @def FCSIZ
 * @brief How many squares of temporary corridor a vault guard may have open at once.
 * @note The height plus the width of the map: the guard digs a path from the vault to the
 *       edge, and no such path can be longer than that.
 */
/**
 * @def FCSIZ
 * @brief 금고 경비가 한 번에 열어 둘 수 있는 임시 통로 칸의 수.
 * @note 지도의 높이와 너비의 합. 경비는 금고에서 바깥까지 길을 파는데, 그런 길은 그보다 길 수 없다.
 */
#define FCSIZ (ROWNO + COLNO)
/**
 * @name What the guard saw
 * @brief Things the hero did in the vault that the guard will hold against them.
 * @note Kept because the guard's response depends on it, and because being caught matters
 *       only if there was a witness.
 * @{
 */
/**
 * @name 경비가 본 것
 * @brief 영웅이 금고에서 한 일 중 경비가 문제 삼을 것들.
 * @note 경비의 대응이 그것에 달려 있기 때문에, 그리고 목격자가 있었을 때만 들킨 것이 의미를 갖기 때문에 보관된다.
 * @{
 */
#define GD_EATGOLD 0x01
#define GD_DESTROYGOLD 0x02
/** @} */

/**
 * @brief One square of the corridor a vault guard dug, and what was there before.
 *
 * The guard's corridor is temporary: it is filled in behind as the guard leaves. So each
 * square must remember what it was, or the level would be permanently scarred by having
 * been visited.
 *
 * @note The two saved fields are copies of a map square's own, kept in the same form so
 *       restoring is an assignment rather than a reconstruction.
 */
/**
 * @brief 금고 경비가 판 통로의 칸 하나와, 그 전에 거기 있던 것.
 *
 * 경비의 통로는 임시적이다. 경비가 떠나면서 뒤를 메운다. 그래서 각 칸이 자신이 무엇이었는지 기억해야 한다. 그러지
 * 않으면 방문되었다는 사실만으로 레벨에 영구한 흉터가 남는다.
 *
 * @note 보관되는 두 필드는 지도 칸 자신의 것을 복사한 것이며, 같은 형태로 보관된다. 그래서 복원이 재구성이 아니라
 *       대입이 된다.
 */
struct fakecorridor {
    coordxy fx, fy;
    schar ftyp; /* from struct rm's typ */
    uchar flags; /* also from struct rm; an unsigned 5-bit field there */
};

/**
 * @brief A vault guard: where it is taking the hero, and how the escort is going.
 *
 * The guard does not fight. It appears, asks the hero to follow, and digs a corridor out --
 * so what it needs to remember is a route, how much of it is still open, and how many times
 * it has asked before it stops asking.
 *
 * @note The corridor is a range within the array rather than a list, because it is opened at
 *       one end and closed at the other. Two indices are enough.
 * @note The level of creation is recorded because a guard separated from its vault -- the
 *       hero having gone down the stairs -- must not keep escorting.
 */
/**
 * @brief 금고 경비. 영웅을 어디로 데려가고 있는지, 그리고 그 호송이 어떻게 되어 가는지.
 *
 * 경비는 싸우지 않는다. 나타나서 영웅에게 따라오라고 하고 통로를 파서 밖으로 데려간다. 그래서 기억해야 하는 것은 경로,
 * 그 중 아직 열려 있는 부분, 그리고 묻기를 그만두기 전까지 몇 번 물었는지다.
 *
 * @note 통로는 목록이 아니라 배열 안의 구간이다. 한쪽 끝에서 열리고 다른 쪽 끝에서 닫히기 때문이다. 색인 둘로 충분하다.
 * @note 생성된 레벨이 기록되는 것은, 영웅이 계단을 내려가 자기 금고와 떨어진 경비가 호송을 계속해서는 안 되기 때문이다.
 */
struct egd {
    unsigned parentmid;   /* make clobber-detection possible */
    int fcbeg, fcend;     /* fcend: first unused pos */
    int vroom;            /* room number of the vault */
    coordxy gdx, gdy;     /* goal of guard's walk */
    coordxy ogx, ogy;     /* guard's last position */
    d_level gdlevel;      /* level (& dungeon) guard was created in */
    xint8 warncnt;        /* number of warnings to follow */
    xint8 dropgoldcnt;    /* number of demands to drop gold */
    Bitfield(gddone, 1);  /* true iff guard has released player */
    Bitfield(witness, 2); /* the guard saw you do something */
    Bitfield(unused, 5);
    struct fakecorridor fakecorr[FCSIZ];
};

/***
 **     formerly epri.h -- temple priest extension
 */
/**
 * @brief A temple priest: which shrine is theirs, and how the hero has behaved there.
 *
 * The priest belongs to a shrine rather than the other way round, so the shrine's alignment,
 * room and level are recorded here -- and a priest whose level does not match the hero's is
 * a priest the hero has walked away from.
 *
 * The timestamps exist for the player's sake. Entering a temple produces a message about how
 * the place feels, and repeating it every step would be noise; recording when each was last
 * said lets the game say it only when it means something.
 *
 * @note Stingy donations are counted because the priest's response to a small offering depends
 *       on how many small offerings there have been.
 * @warning A priest with no shrine is not represented here. Roaming priests become minions
 *          instead and use that extension, as the existing comment records -- so the presence
 *          of this extension implies a shrine.
 */
/**
 * @brief 신전 사제. 어느 성소가 그의 것인지, 그리고 영웅이 그곳에서 어떻게 행동했는지.
 *
 * 사제가 성소에 속하며 그 반대가 아니다. 그래서 성소의 진영, 방, 레벨이 여기에 기록된다. 그리고 레벨이 영웅의 것과 맞지
 * 않는 사제는 영웅이 떠나온 사제다.
 *
 * 시각 기록들은 플레이어를 위해 있다. 신전에 들어서면 그곳의 기운에 관한 메시지가 나오는데, 그것을 한 걸음마다 되풀이하면
 * 소음이 된다. 각각이 마지막으로 언제 나왔는지 기록해 두면 의미가 있을 때만 말할 수 있다.
 *
 * @note 인색한 기부가 세어지는 것은, 적은 봉헌에 대한 사제의 반응이 그런 봉헌이 몇 번 있었는지에 달려 있기 때문이다.
 * @warning 성소가 없는 사제는 여기에 표현되지 않는다. 기존 주석이 기록하듯 떠도는 사제는 대신 하수인이 되어 그 확장을
 *          쓴다. 그래서 이 확장이 있다는 것은 성소가 있음을 함축한다.
 */
struct epri {
    unsigned parentmid;   /* make clobber-detection possible */
    aligntyp shralign; /* alignment of priest's shrine */
    schar shroom;      /* index in rooms */
    coord shrpos;      /* position of shrine */
    d_level shrlevel;  /* level (& dungeon) of shrine */
    unsigned cheapskate_count; /* number of cheapskate donations */
    long intone_time,  /* used to limit verbosity  +*/
        enter_time,    /*+ of temple entry messages */
        hostile_time,  /* forbidding feeling */
        peaceful_time; /* sense of peace */
};
/* note: roaming priests (no shrine) switch from ispriest to isminion
   (and emin extension) */

/***
 **     formerly eshk.h -- shopkeeper extension
 */
/**
 * @def REPAIR_DELAY
 * @brief How long a shopkeeper waits before mending a broken wall.
 * @note A delay rather than an immediate repair, so that the hero has a chance to use the hole
 *       they made -- and so that the repair is a visible event rather than something that
 *       never appears to have been damaged.
 */
/**
 * @def REPAIR_DELAY
 * @brief 상점 주인이 부서진 벽을 고치기까지 기다리는 시간.
 * @note 즉시 수리가 아니라 지연인 것은, 영웅이 자기가 낸 구멍을 쓸 기회를 갖게 하기 위해서다. 그리고 수리가 애초에
 *       손상된 적이 없어 보이는 일이 아니라 눈에 보이는 사건이 되게 하기 위해서다.
 */
#define REPAIR_DELAY 5 /* minimum delay between shop damage & repair */
/**
 * @def BILLSZ
 * @brief How many separate charges one bill can hold.
 * @warning A hard limit, and the bill is stored inline rather than allocated -- so it accounts
 *          for most of a shopkeeper's size, and cannot be exceeded.
 */
/**
 * @def BILLSZ
 * @brief 하나의 계산서가 담을 수 있는 개별 청구 항목의 수.
 * @warning 고정된 한계이며, 계산서는 할당되지 않고 구조체 안에 직접 들어 있다. 그래서 상점 주인 크기의 대부분을
 *          차지하고, 넘을 수 없다.
 */
#define BILLSZ 200

/**
 * @brief One line of a shop bill.
 *
 * Refers to the object by identity rather than by pointer, because the object may be eaten,
 * broken or thrown away while the debt remains. A charge outlives what it was for.
 *
 * @note The price is per unit and the quantity separate, so a partly used stack can be billed
 *       for exactly what was consumed.
 * @note @c useup distinguishes a charge for something the hero still holds from one for
 *       something already gone -- the second cannot be settled by putting the object back.
 */
/**
 * @brief 상점 계산서의 한 줄.
 *
 * 물건을 포인터가 아니라 정체로 가리킨다. 빚이 남아 있는 동안 물건이 먹히거나 부서지거나 버려질 수 있기 때문이다. 청구는
 * 그 대상보다 오래 남는다.
 *
 * @note 값은 단위당이고 수량이 따로 있다. 그래서 일부만 쓴 묶음에 대해 소비한 만큼만 청구할 수 있다.
 * @note @c useup 은 영웅이 아직 지니고 있는 것에 대한 청구와 이미 없어진 것에 대한 청구를 구별한다. 후자는 물건을 되돌려
 *       놓아 해결할 수 없다.
 */
struct bill_x {
    unsigned bo_id;
    boolean useup;
    long price; /* price per unit */
    long bquan; /* amount used up */
};

/**
 * @brief A shopkeeper: the shop, the accounts, and the customer.
 *
 * By far the largest extension, because a shopkeeper is the most stateful thing in the game.
 * It holds four different kinds of money owed -- what was stolen, what is credited, what is
 * owed for using goods, and what was taken from the shop's own gold -- because they are
 * settled differently and a single total could not be.
 *
 * The shop itself is remembered by room, door and level, so a shopkeeper knows when it has
 * been lured away from its own doorway.
 *
 * @note The customer's name is stored rather than a reference, so that the shopkeeper still
 *       knows whom it was dealing with after that hero is gone.
 * @note A cached pointer to the start of the bill exists alongside the bill; it must be
 *       re-established on restore, since a saved pointer means nothing.
 */
/**
 * @brief 상점 주인. 상점, 회계, 그리고 손님.
 *
 * 단연 가장 큰 확장이다. 상점 주인이 게임에서 가장 많은 상태를 지닌 존재이기 때문이다. 빚진 돈을 네 종류로 나누어 담는다.
 * 도둑맞은 것, 적립된 것, 물건을 써서 빚진 것, 상점 자신의 금화에서 가져간 것. 각각이 다르게 청산되므로 하나의 총액으로는
 * 될 수 없다.
 *
 * 상점 자체는 방, 문, 레벨로 기억된다. 그래서 상점 주인은 자기 문간에서 꾀여 나왔을 때를 안다.
 *
 * @note 손님의 이름이 참조가 아니라 글로 저장된다. 그래서 그 영웅이 사라진 뒤에도 상점 주인이 누구와 거래하던 중이었는지
 *       안다.
 * @note 계산서와 나란히 그 시작을 가리키는 캐시 포인터가 있다. 저장된 포인터는 아무 의미가 없으므로 복원 시 다시
 *       세워야 한다.
 */
struct eshk {
    unsigned parentmid;   /* make clobber-detection possible */
    long robbed;          /* amount stolen by most recent customer */
    long credit;          /* amount credited to customer */
    long debit;           /* amount of debt for using unpaid items */
    long loan;            /* shop-gold picked (part of debit) */
    int shoptype;         /* the value of svr.rooms[shoproom].rtype */
    schar shoproom;       /* index in svr.rooms; set by inshop() */
    schar unused;         /* to force alignment for stupid compilers */
    boolean following;    /* following customer since he owes us sth */
    boolean surcharge;    /* angry shk inflates prices */
    boolean dismiss_kops; /* pacified shk sends kops away */
    coord shk;            /* usual position shopkeeper */
    coord shd;            /* position shop door */
    d_level shoplevel;    /* level (& dungeon) of his shop */
    int billct;           /* no. of entries of bill[] in use */
    struct bill_x bill[BILLSZ];
    struct bill_x *bill_p;  /* &(ESHK(shkp)->bill[0]) */
    long break_seq;         /* hero_seq value at time of object breakage */
    boolean seq_peaceful;   /* shkp->mpeaceful at start of break_seq */
    int visitct;            /* nr of visits by most recent customer */
    char customer[PL_NSIZ]; /* most recent customer */
    char shknam[PL_NSIZ];
};

/***
 **     formerly emin.h -- minion extension
 */
/**
 * @brief A minion of a god: whose servant it is, and whether it has turned.
 *
 * A minion's alignment is its own rather than its species', since the same kind of Angel may
 * serve any god. That is why it is recorded per monster.
 *
 * @note @c renegade covers the case that would otherwise be contradictory: a servant of the
 *       hero's own god that is nonetheless hostile. Without it, sharing an alignment would
 *       imply peace.
 * @note Also used for roaming priests, who have no shrine and so cannot use the priest
 *       extension.
 */
/**
 * @brief 신의 하수인. 누구의 종인지, 그리고 돌아섰는지.
 *
 * 하수인의 진영은 종족의 것이 아니라 그 자신의 것이다. 같은 종류의 천사가 어느 신을 섬길 수도 있기 때문이다. 그것이
 * 몬스터마다 기록되는 이유다.
 *
 * @note @c renegade 는 그러지 않으면 모순이 될 경우를 담는다. 영웅과 같은 신의 종이면서도 적대적인 것. 그것이 없으면
 *       진영을 공유한다는 것이 곧 평화를 함축하게 된다.
 * @note 성소가 없어 사제 확장을 쓸 수 없는 떠도는 사제에게도 쓰인다.
 */
struct emin {
    unsigned parentmid;   /* make clobber-detection possible */
    aligntyp min_align; /* alignment of minion */
    boolean renegade;   /* hostile co-aligned priest or Angel */
};

/***
 **     formerly edog.h -- pet extension
 */
/**
 * @brief How a pet regards a piece of food, best first.
 *
 * Ordered so that a pet choosing between two things can simply prefer the lower value, which
 * is why this is a ranking and not a set of categories.
 *
 * @warning The order is the meaning. @c POISON and @c TABU sit at the bottom because a pet
 *          should reach them only when nothing else exists, and @c APPORT is above them
 *          because a pet would rather fetch something than eat something harmful.
 */
/**
 * @brief 애완동물이 음식을 어떻게 보는지. 좋은 것부터.
 *
 * 두 가지 중에서 고르는 애완동물이 그저 더 낮은 값을 선호하면 되도록 정렬되어 있다. 이것이 분류가 아니라 순위인 이유다.
 *
 * @warning 순서가 곧 의미다. @c POISON 과 @c TABU 가 맨 아래에 있는 것은 애완동물이 다른 것이 전혀 없을 때만 그것에
 *          닿아야 하기 때문이고, @c APPORT 가 그 위에 있는 것은 애완동물이 해로운 것을 먹기보다 무언가를 물어 오는 편을
 *          택하기 때문이다.
 */
/*      various types of pet food, the lower the value, the better liked */
enum dogfood_types {
    DOGFOOD = 0,
    CADAVER = 1,
    ACCFOOD = 2,
    MANFOOD = 3,
    APPORT  = 4,
    POISON  = 5,
    UNDEF   = 6,
    TABU    = 7
};

/**
 * @brief A pet: how well trained, how well fed, and how well treated.
 *
 * A pet's loyalty is earned and can be lost, so what is kept here is a history rather than a
 * state. Abuse is counted, deaths are counted, and the moment it will next be hungry is
 * recorded -- because a starving pet turns wild, and that has to be foreseeable rather than
 * sudden.
 *
 * @note The last object dropped and where is remembered so the pet can be credited for
 *       fetching rather than merely for standing near something.
 * @note The maximum health lost to starvation is tracked separately from health itself, so
 *       that feeding the pet can give it back.
 * @note @c killed_by_u records an attempt and not a success; a pet that survived the hero
 *       trying to kill it does not forget.
 */
/**
 * @brief 애완동물. 얼마나 훈련되었는지, 얼마나 잘 먹었는지, 얼마나 잘 대접받았는지.
 *
 * 애완동물의 충성은 얻어지는 것이며 잃을 수도 있다. 그래서 여기에 보관되는 것은 상태가 아니라 이력이다. 학대가 세어지고,
 * 죽음이 세어지고, 다음에 배고파질 시점이 기록된다. 굶주린 애완동물은 야생으로 돌아가며, 그것은 갑작스러운 일이 아니라
 * 예견할 수 있는 일이어야 하기 때문이다.
 *
 * @note 마지막으로 무엇을 어디에 떨어뜨렸는지 기억한다. 그래서 애완동물이 그저 무언가 옆에 서 있는 것이 아니라 물어 온
 *       것으로 인정받을 수 있다.
 * @note 굶주림으로 잃은 최대 체력이 체력 자체와 따로 추적된다. 그래서 먹이면 되돌려 줄 수 있다.
 * @note @c killed_by_u 는 성공이 아니라 시도를 기록한다. 영웅이 죽이려 한 것을 견뎌 낸 애완동물은 그것을 잊지 않는다.
 */
struct edog {
    unsigned parentmid;       /* make clobber-detection possible */
    long droptime;            /* moment dog dropped object */
    unsigned dropdist;        /* dist of dropped obj from @ */
    int apport;               /* amount of training */
    long whistletime;         /* last time he whistled */
    long hungrytime;          /* will get hungry at this time */
    coord ogoal;              /* previous goal location */
    int abuse;                /* track abuses to this pet */
    int revivals;             /* count pet deaths */
    int mhpmax_penalty;       /* while starving, points reduced */
    Bitfield(killed_by_u, 1); /* you attempted to kill him */
};

/***
 **     extension tracking a player's remnant monster (ghost, mummy etc.)
 */
/**
 * @brief Who a monster used to be, when it is the remains of a previous hero.
 *
 * When a hero dies, the level may be saved and given to another game, and what is left of them
 * appears there as a ghost or a mummy. That monster is no longer a hero -- it has monster
 * statistics -- but the game still needs to say who it was, so the player can be told whose
 * grave they are standing on.
 *
 * @note The level at death is kept separately from the monster's own level because the two
 *       differ: the remains are weaker than the hero was, and it is the hero's achievement
 *       that is worth reporting.
 * @note Whether they were crowned or had become a demigod is kept because it changes how the
 *       remains are described, not what they can do.
 */
/**
 * @brief 몬스터가 이전 영웅의 잔해일 때, 그가 누구였는지.
 *
 * 영웅이 죽으면 그 레벨이 저장되어 다른 게임에 주어질 수 있고, 그에게 남은 것이 그곳에 유령이나 미라로 나타난다. 그
 * 몬스터는 더는 영웅이 아니다. 몬스터의 능력치를 가진다. 그래도 게임은 그가 누구였는지 말해야 한다. 플레이어가 누구의
 * 무덤 위에 서 있는지 알 수 있도록.
 *
 * @note 죽을 때의 레벨이 몬스터 자신의 레벨과 따로 보관된다. 둘이 다르기 때문이다. 잔해는 그 영웅보다 약하며, 보고할
 *       가치가 있는 것은 그 영웅의 성취다.
 * @note 왕관을 받았는지, 반신이 되었는지가 보관되는 것은, 그것이 잔해가 무엇을 할 수 있는지가 아니라 어떻게 기술되는지를
 *       바꾸기 때문이다.
 */
struct ebones {
    unsigned parentmid;     /* make clobber-detection possible */
    uchar role;             /* index into roles[] */
    uchar race;             /* index into races[] */
    align oldalign;         /* character alignment */
    uchar deathlevel;       /* level when dying (m_lev may differ) */
    schar luck;             /* luck when dying */
    short mnum;             /* monster type */
    Bitfield(female, 1);    /* was female */
    Bitfield(demigod, 1);   /* had killed wiz or invoked */
    Bitfield(crowned, 1);   /* had been crowned */
};

/***
 **     mextra.h -- collection of all monster extensions
 */
/**
 * @brief The bag of extensions, each present only if needed.
 *
 * The bag itself is allocated when a monster needs any extension at all, and each member is
 * allocated separately. So a monster may have the bag and still lack every extension in it --
 * a named newt has the bag for its name and nothing else.
 *
 * @note @c mcorpsenm is stored inline rather than as a pointer because it is a single number.
 *       It is also shared three ways: for a mimic posing as a statue or corpse it says which
 *       monster, for one posing as a slime mold which fruit, and for one posing as an altar
 *       which alignment. The disguise decides the reading, as the existing comment records.
 * @warning Every member may be null. Reaching one without asking first is the mistake the
 *          @c has_ macros exist to prevent.
 */
/**
 * @brief 확장 꾸러미. 각 항목은 필요할 때만 존재한다.
 *
 * 꾸러미 자체는 몬스터가 어떤 확장이든 필요할 때 할당되고, 각 멤버는 따로 할당된다. 그래서 몬스터가 꾸러미를 가지고도
 * 그 안의 확장을 전부 갖지 않을 수 있다. 이름 붙은 뉴트는 그 이름을 담기 위한 꾸러미만 가지고 다른 것은 갖지 않는다.
 *
 * @note @c mcorpsenm 은 숫자 하나이므로 포인터가 아니라 구조체 안에 직접 들어 있다. 그리고 세 가지로 공유된다. 조각상이나
 *       시체로 위장한 모방자에게는 어느 몬스터인지, 점균으로 위장한 것에게는 어느 과일인지, 제단으로 위장한 것에게는 어느
 *       진영인지를 말한다. 기존 주석이 기록하듯 위장이 그 해석을 정한다.
 * @warning 모든 멤버가 널일 수 있다. 먼저 묻지 않고 하나에 손을 뻗는 것이 @c has_ 매크로들이 막기 위해 존재하는 잘못이다.
 */
struct mextra {
    char *mgivenname;
    struct egd *egd;
    struct epri *epri;
    struct eshk *eshk;
    struct emin *emin;
    struct edog *edog;
    struct ebones *ebones;
    int mcorpsenm; /* obj->corpsenm for mimic posing as statue or corpse,
                    * obj->spe (fruit index) for one posing as a slime mold,
                    * or an alignment mask for one posing as an altar */
};

/**
 * @name Reaching an extension
 * @brief The unguarded accessors, for use once the extension is known to be there.
 * @warning Each dereferences the bag without checking it exists, then yields a member that may
 *          itself be null. Use only after the matching @c has_ test, or where the monster's
 *          kind guarantees the extension.
 * @{
 */
/**
 * @name 확장에 접근하기
 * @brief 보호되지 않은 접근자들. 확장이 있다는 것이 확인된 뒤에 쓴다.
 * @warning 각각이 꾸러미의 존재를 확인하지 않고 역참조한 뒤, 그 자체가 널일 수 있는 멤버를 낸다. 짝이 되는 @c has_ 검사
 *          뒤에, 또는 몬스터의 종류가 그 확장을 보장하는 곳에서만 쓸 것.
 * @{
 */
#define MGIVENNAME(mon) ((mon)->mextra->mgivenname)
#define EGD(mon) ((mon)->mextra->egd)
#define EPRI(mon) ((mon)->mextra->epri)
#define ESHK(mon) ((mon)->mextra->eshk)
#define EMIN(mon) ((mon)->mextra->emin)
#define EDOG(mon) ((mon)->mextra->edog)
#define EBONES(mon) ((mon)->mextra->ebones)
#define MCORPSENM(mon) ((mon)->mextra->mcorpsenm)
/** @} */

/**
 * @name Asking whether an extension is there
 * @brief The safe tests: does this monster have this extension at all.
 *
 * Each checks the bag before the member, so it is safe on any monster. These are what makes the
 * whole arrangement usable -- a shopkeeper test that works on a newt.
 *
 * @note @c has_mcorpsenm differs from the rest: the member is a number held inline, so its
 *       absence is a specific "no monster" value rather than a null pointer.
 * @warning The monster argument is evaluated twice.
 * @{
 */
/**
 * @name 확장이 있는지 묻기
 * @brief 안전한 검사들. 이 몬스터가 이 확장을 아예 가지고 있는지.
 *
 * 각각이 멤버보다 먼저 꾸러미를 확인하므로 어떤 몬스터에게든 안전하다. 이들이 이 구조 전체를 쓸 수 있게 만드는 것이다.
 * 뉴트에게도 통하는 상점 주인 검사.
 *
 * @note @c has_mcorpsenm 은 나머지와 다르다. 그 멤버는 구조체 안에 직접 든 숫자이므로, 없음이 널 포인터가 아니라 "그런
 *       몬스터 없음"을 뜻하는 특정 값이다.
 * @warning 몬스터 인자가 두 번 평가된다.
 * @{
 */
#define has_mgivenname(mon) ((mon)->mextra && MGIVENNAME(mon))
#define has_egd(mon)   ((mon)->mextra && EGD(mon))
#define has_epri(mon)  ((mon)->mextra && EPRI(mon))
#define has_eshk(mon)  ((mon)->mextra && ESHK(mon))
#define has_emin(mon)  ((mon)->mextra && EMIN(mon))
#define has_edog(mon)  ((mon)->mextra && EDOG(mon))
#define has_ebones(mon) ((mon)->mextra && EBONES(mon))
#define has_mcorpsenm(mon) ((mon)->mextra && MCORPSENM(mon) != NON_PM)
/** @} */

#endif /* MEXTRA_H */
