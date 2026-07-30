/* NetHack 5.0	iactions.c	$NHDT-Date: 1781973051 2026/06/20 16:30:51 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.4 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2026. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file iactions.c
 * @brief Offering the player what they can actually do with an item.
 *
 * Rather than requiring the player to know that a wand is zapped, a scroll
 * read and a potion quaffed, this builds a menu of the actions that make sense
 * for the item in hand -- and only those, so an item the hero cannot currently
 * use does not offer to be used.
 *
 * The classification helpers answer that question for each kind of action:
 * whether the object can be named, read, applied, and so on, in the hero's
 * present situation.
 *
 * @note What is offered depends on the hero as much as on the object -- being
 *       blind, having no free hands, or standing somewhere unsuitable all
 *       remove entries.
 */

/**
 * @file iactions.c
 * @brief 어떤 물건으로 실제로 무엇을 할 수 있는지 플레이어에게 제시하기.
 *
 * 지팡이는 쏘고 두루마리는 읽고 물약은 마신다는 것을 플레이어가 외우고 있어야
 * 하는 대신, 손에 든 물건에 대해 말이 되는 행동들의 메뉴를 만든다. 오직 그것들만
 * 제시하므로, 지금 쓸 수 없는 물건이 쓰겠느냐고 묻지 않는다.
 *
 * 분류 도우미들이 행동 종류마다 그 질문에 답한다. 지금 영웅의 상황에서 그 물건에
 * 이름을 붙일 수 있는지, 읽을 수 있는지, 사용할 수 있는지 같은 것들이다.
 *
 * @note 무엇이 제시되는지는 물건만큼이나 영웅에 달려 있다. 눈이 멀었거나, 손이
 *       비어 있지 않거나, 적절하지 않은 곳에 서 있으면 항목이 사라진다.
 */

#include "hack.h"

staticfn boolean item_naming_classification(struct obj *, char *, char *);
staticfn int item_reading_classification(struct obj *, char *);
staticfn void ia_addmenu(winid, int, char, const char *);
staticfn void itemactions_pushkeys(struct obj *, int);

enum item_action_actions {
    IA_NONE          = 0,
    IA_UNWIELD, /* hack for 'w-' */
    IA_APPLY_OBJ, /* 'a' */
    IA_DIP_OBJ, /* 'a' on a potion == dip */
    IA_NAME_OBJ, /* 'c' name individual item */
    IA_NAME_OTYP, /* 'C' name item's type */
    IA_DROP_OBJ, /* 'd' */
    IA_EAT_OBJ, /* 'e' */
    IA_ENGRAVE_OBJ, /* 'E' */
    IA_FIRE_OBJ, /* 'f' */
    IA_ADJUST_OBJ, /* 'i' #adjust inventory letter */
    IA_ADJUST_STACK, /* 'I' #adjust with count to split stack */
    IA_SACRIFICE, /* 'O' offer sacrifice */
    IA_BUY_OBJ, /* 'p' pay shopkeeper */
    IA_QUAFF_OBJ,
    IA_QUIVER_OBJ,
    IA_READ_OBJ,
    IA_RUB_OBJ,
    IA_THROW_OBJ,
    IA_TAKEOFF_OBJ,
    IA_TIP_CONTAINER,
    IA_INVOKE_OBJ,
    IA_WIELD_OBJ,
    IA_WEAR_OBJ,
    IA_SWAPWEAPON,
    IA_TWOWEAPON,
    IA_ZAP_OBJ,
    IA_WHATIS_OBJ, /* '/' specify inventory object */
};

/**
 * @brief Work out how to word the two naming entries, and whether either applies.
 *
 * Naming is two different actions that read alike, and the wording has to keep them apart: naming this particular object, and naming every object that looks like it. A player who confuses the two names one dagger and wonders
 * why the others did not change.
 *
 * So both entries are worded to say which they are -- "this specific dagger" against "the type for daggers" -- and the plural is chosen to match, which is why the object's quantity and uniqueness both enter into it.
 *
 * @param obj the object
 * @param onamebuf receives the wording for naming this object, or nothing
 * @param ocallbuf receives the wording for naming its kind, or nothing
 * @return whether either action applies
 * @note An object already named says "rename" rather than "name", so the menu tells the player that something is there to be replaced. The kind-naming entry does the same with a coined word, and the accompanying comment
 *       concedes the alternatives read worse.
 * @note A unique object takes "the" instead of a plural, since there is only one and speaking of its type in the plural would be wrong.
 */
/**
 * @brief 두 이름 짓기 항목을 어떻게 표현할지, 그리고 어느 쪽이 적용되는지 알아낸다.
 *
 * 이름 짓기는 비슷하게 읽히는 서로 다른 두 행동이며, 그 표현이 그것들을 갈라 놓아야 한다. 이 특정한 객체에 이름 붙이기, 그리고 그것처럼 보이는 모든 객체에 이름 붙이기. 그 둘을 혼동하는 플레이어는 단검 하나에 이름을 붙이고 나머지가 왜 바뀌지 않았는지 의아해한다.
 *
 * 그래서 두 항목이 자신이 어느 쪽인지 말하도록 표현된다. "이 특정한 단검"과 "단검이라는 종류". 그리고 복수형이 그에 맞게 골라지며, 그것이 객체의 수량과 유일성이 둘 다 여기에 들어오는 이유다.
 *
 * @param obj 그 객체
 * @param onamebuf 이 객체에 이름 붙이기의 표현을 받는다. 또는 아무것도
 * @param ocallbuf 그 종류에 이름 붙이기의 표현을 받는다. 또는 아무것도
 * @return 어느 쪽이든 적용되는지
 * @note 이미 이름이 붙은 객체는 "이름 붙이기"가 아니라 "이름 바꾸기"라고 말한다. 그래서 메뉴가 플레이어에게 대체될 무언가가 있다고 알려 준다. 종류 이름 짓기 항목도 만들어 낸 낱말로 같은 일을 하며, 딸린 주석이 그 대안들이 더 나쁘게 읽힌다고 인정하고 있다.
 * @note 유일한 객체는 복수형 대신 "the"를 받는다. 하나뿐이고 그 종류를 복수로 말하는 것이 틀리기 때문이다.
 */
/* construct text for the menu entries for IA_NAME_OBJ and IA_NAME_OTYP */
staticfn boolean
item_naming_classification(
    struct obj *obj,
    char *onamebuf,
    char *ocallbuf)
{
    static const char
        Name[] = "Name",
        Rename[] = "Rename or un-name",
        Call[] = "Call",
        /* "re-call" seems a bit weird, but "recall" and
           "rename" don't fit for changing a type name */
        Recall[] = "Re-call or un-call";

    onamebuf[0] = ocallbuf[0] = '\0';
    if (name_ok(obj) == GETOBJ_SUGGEST) {
        Sprintf(onamebuf, "%s %s %s",
                (!has_oname(obj) || !*ONAME(obj)) ? Name : Rename,
                the_unique_obj(obj) ? "the"
                : !is_plural(obj) ? "this specific"
                  : "this stack of", /*"these",*/
                simpleonames(obj));
    }
    if (call_ok(obj) == GETOBJ_SUGGEST) {
        char *callname = simpleonames(obj);

        /* prefix known unique item with "the", make all other types plural */
        if (the_unique_obj(obj)) /* treats unID'd fake amulets as if real */
            callname = the(callname);
        else if (!is_plural(obj))
            callname = makeplural(callname);
        Sprintf(ocallbuf, "%s the type for %s",
                (!objects[obj->otyp].oc_uname
                 || !*objects[obj->otyp].oc_uname) ? Call : Recall,
                callname);
    }
    return (*onamebuf || *ocallbuf) ? TRUE : FALSE;
}

/**
 * @brief Whether an object can be read, and what reading it should be called.
 *
 * "Read" covers several unrelated things, and the wording is what keeps a player from expecting the wrong one. Reading a scroll invokes magic; reading a shirt does not; studying a spellbook is a commitment of several turns;
 * examining the one tome that must not be studied casually is deliberately named differently.
 *
 * @param obj the object
 * @param outbuf receives the wording
 * @return the reading action, or none if the object cannot be read
 * @note The scroll wording mentions activating magic only when the hero knows enough for that to be true. An unidentified scroll is offered plainly, so the menu does not tell the player something the hero has not learned.
 * @note Blank paper and the book of the dead are worded specially only once identified, for the same reason -- before that they are just a scroll and a spellbook.
 */
/**
 * @brief 객체를 읽을 수 있는지, 그리고 그것을 읽는 일을 무엇이라 불러야 하는지.
 *
 * "읽기"는 서로 무관한 여러 가지를 덮으며, 그 표현이 플레이어가 잘못된 것을 기대하지 않게 하는 것이다. 두루마리를 읽는 것은 마법을 발동한다. 셔츠를 읽는 것은 그렇지 않다. 주문서를 공부하는 것은 여러 턴의 약속이다. 함부로 공부해서는 안 되는 그 한 권을 살펴보는 일은 일부러 다르게
 * 이름 붙어 있다.
 *
 * @param obj 그 객체
 * @param outbuf 그 표현을 받는다
 * @return 그 읽기 행동. 객체를 읽을 수 없으면 없음
 * @note 두루마리 표현은 영웅이 그것이 참임을 알 만큼 알 때만 마법 발동을 언급한다. 미확인 두루마리는 담담하게 제시되므로, 메뉴가 영웅이 배우지 않은 것을 플레이어에게 알려 주지 않는다.
 * @note 백지와 사자의 서도 감별된 뒤에만 특별하게 표현되며, 같은 이유다. 그전까지 그것들은 그저 두루마리와 주문서다.
 */
/* construct text for the menu entries for IA_READ_OBJ */
staticfn int
item_reading_classification(struct obj *obj, char *outbuf)
{
    int otyp = obj->otyp, res = IA_READ_OBJ;

    *outbuf = '\0';
    if (otyp == FORTUNE_COOKIE) {
        Strcpy(outbuf, "Read the message inside this cookie");
    } else if (otyp == T_SHIRT) {
        Strcpy(outbuf, "Read the slogan on the shirt");
    } else if (otyp == ALCHEMY_SMOCK) {
        Strcpy(outbuf, "Read the slogan on the apron");
    } else if (otyp == HAWAIIAN_SHIRT) {
        Strcpy(outbuf, "Look at the pattern on the shirt");
    } else if (obj->oclass == SCROLL_CLASS) {
        const char *magic = ((obj->dknown
#ifdef MAIL_STRUCTURES
                              && otyp != SCR_MAIL
#endif
                              && (otyp != SCR_BLANK_PAPER
                                  || !objects[otyp].oc_name_known))
                             ? " to activate its magic" : "");

        Sprintf(outbuf, "Read this scroll%s", magic);
    } else if (obj->oclass == SPBOOK_CLASS) {
        boolean novel = (otyp == SPE_NOVEL),
                blank = (otyp == SPE_BLANK_PAPER
                         && objects[otyp].oc_name_known),
                tome = (otyp == SPE_BOOK_OF_THE_DEAD
                        && objects[otyp].oc_name_known);

        Sprintf(outbuf, "%s this %s",
                (novel || blank) ? "Read" : tome ? "Examine" : "Study",
                novel ? simpleonames(obj) /* "novel" or "paperback book" */
                      : tome ? "tome" : "spellbook");
    } else {
        res = IA_NONE;
    }
    return res;
}

/**
 * @brief Add one action to the menu.
 * @param win the menu
 * @param act which action this entry stands for
 * @param let the key that selects it
 * @param txt the wording
 * @note The action travels as the entry's own value rather than being worked out from the key, so the key a player presses and the action it performs are decided in one place -- which is what lets the keys be the ones a player
 *       would already know for those commands.
 */
/**
 * @brief 메뉴에 행동 하나를 더한다.
 * @param win 그 메뉴
 * @param act 이 항목이 나타내는 행동
 * @param let 그것을 고르는 키
 * @param txt 그 표현
 * @note 행동이 키에서 계산되는 대신 그 항목 자신의 값으로 다닌다. 그래서 플레이어가 누르는 키와 그것이 수행하는 행동이 한곳에서 정해지며, 그것이 그 키들을 플레이어가 그 명령에 대해 이미 알고 있을 것으로 만드는 것이다.
 */
staticfn void
ia_addmenu(winid win, int act, char let, const char *txt)
{
    anything any;
    int clr = NO_COLOR;

    any = cg.zeroany;
    any.a_int = act;
    add_menu(win, &nul_glyphinfo, &any, let, 0,
             ATR_NONE, clr, txt, MENU_ITEMFLAGS_NONE);
}

/**
 * @brief Carry out a chosen action by feeding the game the keystrokes for it.
 *
 * Nothing here performs an action. It queues the command and the item's letter as though the player had typed them, and the ordinary command machinery then does the work.
 *
 * That indirection is deliberate and is what makes this whole file cheap to maintain: an action offered here is the same action as the command it corresponds to, with the same prompts, the same checks and the same cost in time.
 * Calling the commands directly would mean this file reimplementing each one's preliminaries and drifting from them.
 *
 * @param otmp the object the action is for
 * @param act which action was chosen
 * @warning An unrecognised action is reported as an internal error rather than ignored, because every menu entry was added by this file and one arriving here unhandled means the menu and this switch have diverged.
 * @note Unwielding is the awkward case: which command performs it depends on which hand the object is in, and the object may be in none of them -- the final branch is unreachable and says so.
 */
/**
 * @brief 골라진 행동을, 그것에 해당하는 키 입력을 게임에 먹여서 수행한다.
 *
 * 여기의 어느 것도 행동을 수행하지 않는다. 플레이어가 입력한 것처럼 그 명령과 물건의 글자를 대기열에 넣고, 그다음 평범한 명령 기제가 그 일을 한다.
 *
 * 그 간접이 의도적이며, 이 파일 전체를 유지하기 값싸게 만드는 것이다. 여기서 제시되는 행동은 그것에 대응하는 명령과 같은 행동이며, 같은 프롬프트, 같은 검사, 같은 시간 비용을 가진다. 명령을 직접 호출하는 것은 이 파일이 각각의 사전 작업을 다시 구현하고 그것들과 어긋나게 된다는 뜻이다.
 *
 * @param otmp 그 행동의 대상 객체
 * @param act 어느 행동이 골라졌는지
 * @warning 알 수 없는 행동은 무시되는 대신 내부 오류로 알려진다. 모든 메뉴 항목이 이 파일이 더한 것이며, 처리되지 않은 채로 여기 도착한 것은 그 메뉴와 이 분기가 갈라졌다는 뜻이기 때문이다.
 * @note 무기를 놓는 것이 어색한 경우다. 어느 명령이 그것을 수행하는지가 그 객체가 어느 손에 있는지에 달려 있고, 그 객체가 어느 손에도 없을 수 있다. 마지막 가지는 닿을 수 없으며 그렇다고 적혀 있다.
 */
/* set up a command to execute on a specific item next */
staticfn void
itemactions_pushkeys(struct obj *otmp, int act)
{
    switch (act) {
    default:
        impossible("Unknown item action %d", act);
        break;
    case IA_NONE:
        break;
    case IA_UNWIELD:
        cmdq_add_ec(CQ_CANNED, (otmp == uwep) ? dowield
                    : (otmp == uswapwep) ? remarm_swapwep
                      : (otmp == uquiver) ? dowieldquiver
                        : donull); /* can't happen */
        cmdq_add_key(CQ_CANNED, HANDS_SYM);
        break;
    case IA_APPLY_OBJ:
        cmdq_add_ec(CQ_CANNED, doapply);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_DIP_OBJ:
        /* #altdip instead of normal #dip - takes potion to dip into
           first (the inventory item instigating this) and item to
           be dipped second, also ignores floor features such as
           fountain/sink so we don't need to force m-prefix here */
        cmdq_add_ec(CQ_CANNED, dip_into);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_NAME_OBJ:
    case IA_NAME_OTYP:
        cmdq_add_ec(CQ_CANNED, docallcmd);
        cmdq_add_key(CQ_CANNED, (act == IA_NAME_OBJ) ? 'i' : 'o');
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_DROP_OBJ:
        cmdq_add_ec(CQ_CANNED, dodrop);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_EAT_OBJ:
        /* start with m-prefix; for #eat, it means ignore floor food
           if present and eat food from invent */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, doeat);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_ENGRAVE_OBJ:
        cmdq_add_ec(CQ_CANNED, doengrave);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_FIRE_OBJ:
        cmdq_add_ec(CQ_CANNED, dofire);
        break;
    case IA_ADJUST_OBJ:
        cmdq_add_ec(CQ_CANNED, doorganize); /* #adjust */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_ADJUST_STACK:
        cmdq_add_ec(CQ_CANNED, adjust_split); /* #altadjust */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_SACRIFICE:
        cmdq_add_ec(CQ_CANNED, dosacrifice);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_BUY_OBJ:
        cmdq_add_ec(CQ_CANNED, dopay);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_QUAFF_OBJ:
        /* start with m-prefix; for #quaff, it means ignore fountain
           or sink if present and drink a potion from invent */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, dodrink);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_QUIVER_OBJ:
        cmdq_add_ec(CQ_CANNED, dowieldquiver);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_READ_OBJ:
        cmdq_add_ec(CQ_CANNED, doread);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_RUB_OBJ:
        cmdq_add_ec(CQ_CANNED, dorub);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_THROW_OBJ:
        cmdq_add_ec(CQ_CANNED, dothrow);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_TAKEOFF_OBJ:
        cmdq_add_ec(CQ_CANNED, ia_dotakeoff); /* #altdotakeoff */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_TIP_CONTAINER:
        /* start with m-prefix to skip floor containers;
           for menustyle:Traditional when more than one floor container
           is present, player will get a #tip menu and have to pick
           the "tip something being carried" choice, then this item
           will be already chosen from inventory; suboptimal but
           possibly an acceptable tradeoff since combining item actions
           with use of traditional ggetobj() is an unlikely scenario */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, dotip);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_INVOKE_OBJ:
        cmdq_add_ec(CQ_CANNED, doinvoke);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WIELD_OBJ:
        cmdq_add_ec(CQ_CANNED, dowield);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WEAR_OBJ:
        cmdq_add_ec(CQ_CANNED, dowear);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_SWAPWEAPON:
        cmdq_add_ec(CQ_CANNED, doswapweapon);
        break;
    case IA_TWOWEAPON:
        cmdq_add_ec(CQ_CANNED, dotwoweapon);
        break;
    case IA_ZAP_OBJ:
        cmdq_add_ec(CQ_CANNED, dozap);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WHATIS_OBJ:
        cmdq_add_ec(CQ_CANNED, dowhatis); /* "/" command */
        cmdq_add_key(CQ_CANNED, 'i');     /* "i" == item from inventory */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    }
}

/**
 * @brief Offer the player everything they could do with one item, and do what they choose.
 *
 * The file's one public routine. It asks, for each action the game has, whether that action makes sense for this object in the hero's present circumstances, builds a menu of the ones that do, and queues the chosen one.
 *
 * What makes the menu worth having is what it leaves out. An action is offered only if it would actually work, so the menu is a statement about the situation as well as the object: a wand with no charges left does not offer to be
 * zapped, and a hero with no free hands is not offered a weapon to wield. A player can therefore read the menu to learn why something is not possible.
 *
 * @param otmp the object
 * @return the command result -- whether time passed, which depends on the action chosen rather than on this
 * @note Nothing here consumes a turn. The chosen action is queued and performed afterwards by the ordinary command machinery, so its cost in time is that command's and not this menu's.
 * @note The classification helpers above supply the wording as well as the answer, because for several actions the right words depend on facts they have already established.
 */
/**
 * @brief 어떤 물건으로 플레이어가 할 수 있는 모든 것을 제시하고, 그가 고른 것을 한다.
 *
 * 이 파일의 유일한 공개 루틴이다. 게임이 가진 각 행동에 대해 그 행동이 영웅의 현재 정황에서 이 객체에 대해 말이 되는지 묻고, 말이 되는 것들의 메뉴를 만들고, 골라진 것을 대기열에 넣는다.
 *
 * 그 메뉴를 가질 가치가 있게 만드는 것은 그것이 빼놓는 것이다. 행동은 실제로 통할 때만 제시되므로, 그 메뉴는 객체에 대한 것만이 아니라 상황에 대한 선언이다. 충전이 다한 지팡이는 쏘겠느냐고 묻지 않고, 손이 빈 데가 없는 영웅에게 들 무기가 제시되지 않는다. 그래서 플레이어가 그 메뉴를
 * 읽어 무언가가 왜 불가능한지 알 수 있다.
 *
 * @param otmp 그 객체
 * @return 명령 결과. 시간이 지났는지이며, 이것이 아니라 골라진 행동에 달려 있다
 * @note 여기의 어느 것도 턴을 소비하지 않는다. 골라진 행동은 대기열에 들어가 나중에 평범한 명령 기제가 수행하므로, 그 시간 비용은 이 메뉴의 것이 아니라 그 명령의 것이다.
 * @note 위의 분류 보조 함수들은 답뿐 아니라 표현도 제공한다. 여러 행동에 대해 알맞은 낱말이 그것들이 이미 확정한 사실에 달려 있기 때문이다.
 */
/* Show menu of possible actions hero could do with item otmp */
int
itemactions(struct obj *otmp)
{
    int n, act = IA_NONE;
    winid win;
    char buf[BUFSZ], buf2[BUFSZ];
    menu_item *selected;
    struct monst *mtmp;
    const char *light = otmp->lamplit ? "Extinguish" : "Light";
    boolean already_worn = (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) != 0;

    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);

    /* -: unwield; picking current weapon offers an opportunity for 'w-'
       to wield bare/gloved hands; likewise for 'Q-' with quivered item(s) */
    if (otmp == uwep || otmp == uswapwep || otmp == uquiver) {
        const char *verb = (otmp == uquiver) ? "Quiver" : "Wield",
                   *action = (otmp == uquiver) ? "un-ready" : "un-wield",
                   *which = is_plural(otmp) ? "these" : "this",
                   *what = ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp))
                            ? "weapon" : "item");
        /*
         * TODO: if uwep is ammo, tell player that to shoot instead of toss,
         *       the corresponding launcher must be wielded;
         */
        Sprintf(buf,  "%s '%c' to %s %s %s",
                verb, HANDS_SYM, action, which,
                is_plural(otmp) ? makeplural(what) : what);
        ia_addmenu(win, IA_UNWIELD, '-', buf);
    }

    /* a: apply */
    if (otmp->oclass == COIN_CLASS)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Flip a coin");
    else if (otmp->otyp == CREAM_PIE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Hit yourself with this cream pie");
    else if (otmp->otyp == BULLWHIP)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Lash out with this whip");
    else if (otmp->otyp == GRAPPLING_HOOK)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Grapple something with this hook");
    else if (otmp->otyp == BAG_OF_TRICKS && objects[otmp->otyp].oc_name_known)
        /* bag of tricks skips this unless discovered */
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Reach into this bag");
    else if (Is_container(otmp))
        /* bag of tricks gets here only if not yet discovered */
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Open this container");
    else if (otmp->otyp == CAN_OF_GREASE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use the can to grease an item");
    else if (otmp->otyp == LOCK_PICK
             || otmp->otyp == CREDIT_CARD
             || otmp->otyp == SKELETON_KEY)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this tool to pick a lock");
    else if (otmp->otyp == TINNING_KIT)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this kit to tin a corpse");
    else if (otmp->otyp == LEASH) {
        if (!otmp->leashmon) {
            Strcpy(buf, "Attach this leash to a pet");
        } else {
            mtmp = find_mid(otmp->leashmon, FM_FMON);
            if (!mtmp) /* assume this won't happen */
                panic("Can't find leash's monster");
            Sprintf(buf, "Detach this leash from %s", some_mon_nam(mtmp));
        }
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == SADDLE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Place this saddle on a pet");
    else if (otmp->otyp == MAGIC_WHISTLE
             || otmp->otyp == TIN_WHISTLE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Blow this whistle");
    else if (otmp->otyp == EUCALYPTUS_LEAF)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this leaf as a whistle");
    else if (otmp->otyp == STETHOSCOPE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Listen through the stethoscope");
    else if (otmp->otyp == MIRROR)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Show something its reflection");
    else if (otmp->otyp == BELL || otmp->otyp == BELL_OF_OPENING)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Ring the bell");
    else if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
        Sprintf(buf, "%s the candelabrum", light);
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == WAX_CANDLE || otmp->otyp == TALLOW_CANDLE) {
        boolean multiple = (otmp->quan == 1L) ? FALSE : TRUE;
        const char *s = multiple ? "these" : "this";
        struct obj *o = carrying(CANDELABRUM_OF_INVOCATION);

        if (o && o->spe < 7)
            Sprintf(buf, "Attach %s to your candelabrum, or %s %s", s,
                    !otmp->lamplit ? "light" : "extinguish", /* [lowercase] */
                    multiple ? "them" : "it");
        else
            Sprintf(buf, "%s %s %s", light, s, simpleonames(otmp));
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == OIL_LAMP || otmp->otyp == MAGIC_LAMP
               || otmp->otyp == BRASS_LANTERN) {
        Sprintf(buf, "%s this light source", light);
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == POT_OIL && objects[otmp->otyp].oc_name_known) {
        Sprintf(buf, "%s this oil", light);
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->oclass == POTION_CLASS) {
        /* FIXME? this should probably be moved to 'D' rather than be 'a' */
        Sprintf(buf, "Dip something into %s potion%s",
                is_plural(otmp) ? "one of these" : "this", plur(otmp->quan));
        ia_addmenu(win, IA_DIP_OBJ, 'a', buf);
    } else if (otmp->otyp == EXPENSIVE_CAMERA)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Take a photograph");
    else if (otmp->otyp == TOWEL)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Clean yourself off with this towel");
    else if (otmp->otyp == CRYSTAL_BALL)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Peer into this crystal ball");
    else if (otmp->otyp == MAGIC_MARKER)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Write on something with this marker");
    else if (otmp->otyp == FIGURINE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Make this figurine transform");
    else if (otmp->otyp == UNICORN_HORN)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this unicorn horn");
    else if (otmp->otyp == HORN_OF_PLENTY
             && objects[otmp->otyp].oc_name_known)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Blow into the horn of plenty");
    else if (otmp->otyp >= WOODEN_FLUTE && otmp->otyp <= DRUM_OF_EARTHQUAKE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Play this musical instrument");
    else if (otmp->otyp == LAND_MINE || otmp->otyp == BEARTRAP)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Arm this trap");
    else if (otmp->otyp == PICK_AXE || otmp->otyp == DWARVISH_MATTOCK)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Dig with this digging tool");
    else if (otmp->oclass == WAND_CLASS)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Break this wand");

    /* 'c', 'C' - call an item or its type something */
    if (item_naming_classification(otmp, buf, buf2)) {
        if (*buf)
            ia_addmenu(win, IA_NAME_OBJ, 'c', buf);
        if (*buf2)
            ia_addmenu(win, IA_NAME_OTYP, 'C', buf2);
    }

    /* d: drop item, works on everything except worn items; those will
       always have a takeoff/remove choice so we don't have to worry
       about the menu maybe being empty when 'd' is suppressed */
    if (!already_worn) {
        Sprintf(buf, "Drop this %s", (otmp->quan > 1L) ? "stack" : "item");
        ia_addmenu(win, IA_DROP_OBJ, 'd', buf);
    }

    /* e: eat item */
    if (otmp->otyp == TIN) {
        Sprintf(buf, "Open %s%s and eat the contents",
                (otmp->quan > 1L) ? "one of these tins" : "this tin",
                (otmp->otyp == TIN && uwep && uwep->otyp == TIN_OPENER)
                ? " with your tin opener" : "");
        ia_addmenu(win, IA_EAT_OBJ, 'e', buf);
    } else if (is_edible(otmp)) {
        Sprintf(buf, "Eat %s", (otmp->quan > 1L) ? "one of these" : "this");
        ia_addmenu(win, IA_EAT_OBJ, 'e', buf);
    }

    /* E: engrave with item */
    if (otmp->otyp == TOWEL) {
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E',
                   "Wipe the floor with this towel");
    } else if (otmp->otyp == MAGIC_MARKER) {
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E',
                   "Scribble graffiti on the floor");
    } else if (otmp->oclass == WEAPON_CLASS || otmp->oclass == WAND_CLASS
             || otmp->oclass == GEM_CLASS || otmp->oclass == RING_CLASS) {
        Sprintf(buf, "%s on the %s with %s",
                (is_blade(otmp) || otmp->oclass == WAND_CLASS
                 || ((otmp->oclass == GEM_CLASS || otmp->oclass == RING_CLASS)
                     && objects[otmp->otyp].oc_tough)) ? "Engrave" : "Write",
                surface(u.ux, u.uy),
                (otmp->quan > 1L) ? "one of these items" : "this item");
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E', buf);
    }

    /* f: fire quivered ammo */
    if (otmp == uquiver) {
        boolean shoot = ammo_and_launcher(otmp, uwep);

        /* FIXME: see the multi-shot FIXME about "one of" for 't: throw' */
        Sprintf(buf, "%s %s", shoot ? "Shoot" : "Throw",
                (otmp->quan > 1L) ? "one of these" : "this");
        if (shoot) {
            assert(uwep != NULL);
            Sprintf(eos(buf), " with your wielded %s", simpleonames(uwep));
        }
        ia_addmenu(win, IA_FIRE_OBJ, 'f', buf);
    }

    /* i: #adjust inventory letter; gold can't be adjusted unless there
       is some in a slot other than '$' (which shouldn't be possible) */
    if (otmp->oclass != COIN_CLASS || check_invent_gold("item-action"))
        ia_addmenu(win, IA_ADJUST_OBJ, 'i',
                   "Adjust inventory by assigning new letter");
    /* I: #adjust inventory item by splitting its stack  */
    if (otmp->quan > 1L && otmp->oclass != COIN_CLASS)
        ia_addmenu(win, IA_ADJUST_STACK, 'I',
                   "Adjust inventory by splitting this stack");

    /* O: offer sacrifice */
    if (IS_ALTAR(levl[u.ux][u.uy].typ) && !u.uswallow) {
        /* FIXME: this doesn't match #offer's likely candidates, which don't
           include corpses on Astral and don't include amulets off Astral */
        if (otmp->otyp == CORPSE)
            ia_addmenu(win, IA_SACRIFICE, 'O',
                       "Offer this corpse as a sacrifice at this altar");
        else if (otmp->otyp == AMULET_OF_YENDOR
                 || otmp->otyp == FAKE_AMULET_OF_YENDOR)
            ia_addmenu(win, IA_SACRIFICE, 'O',
                       "Offer this amulet as a sacrifice at this altar");
    }

    /* p: pay for unpaid utems */
    if (otmp->unpaid
        /* FIXME: should also handle player owned container (so not
           flagged 'unpaid') holding shop owned items */
        && (mtmp = shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE))) != 0
        && inhishop(mtmp)) {
        Sprintf(buf, "Buy this unpaid %s",
                (otmp->quan > 1L) ? "stack" : "item");
        ia_addmenu(win, IA_BUY_OBJ, 'p', buf);
    }

    /* P: put on accessory */
    if (!already_worn) {
        /* if 'otmp' is worn, we'll skip 'P' and show 'R' below;
           if not worn, we show 'P - Put on this <simple-item>' if
           the slot is available, or 'P - <unavailable>'; for the latter,
           'P' will fail but we don't want to omit the choice because
           item actions can be used to learn commands */
        *buf = '\0';
        if (otmp->oclass == AMULET_CLASS) {
            Strcpy(buf, !uamul ? "Put this amulet on"
                               : "[already wearing an amulet]");
        } else if (otmp->oclass == RING_CLASS || otmp->otyp == MEAT_RING) {
            if (!uleft || !uright)
                Strcpy(buf, "Put this ring on");
            else
                Sprintf(buf, "[both ring %s in use]",
                        makeplural(body_part(FINGER)));
        } else if (otmp->otyp == BLINDFOLD || otmp->otyp == TOWEL
                   || otmp->otyp == LENSES) {
            if (ublindf)
                Strcpy(buf, "[already wearing eyewear]");
            else if (otmp->otyp == LENSES)
                Strcpy(buf, "Put these lenses on");
            else
                Sprintf(buf, "Put this on%s",
                        (otmp->otyp == TOWEL) ? " to blindfold yourself" : "");
        }
        if (*buf)
            ia_addmenu(win, IA_WEAR_OBJ, 'P', buf);
    }

    /* q: drink item */
    if (otmp->oclass == POTION_CLASS) {
        Sprintf(buf, "Quaff (drink) %s",
                (otmp->quan > 1L) ? "one of these potions" : "this potion");
        ia_addmenu(win, IA_QUAFF_OBJ, 'q', buf);
    }

    /* Q: quiver throwable item */
    if ((otmp->oclass == GEM_CLASS || otmp->oclass == WEAPON_CLASS)
        && otmp != uquiver) {
        Sprintf(buf, "Quiver this %s for easy %s with \'f\'ire",
                (otmp->quan > 1L) ? "stack" : "item",
                ammo_and_launcher(otmp, uwep) ? "shooting" : "throwing");
        ia_addmenu(win, IA_QUIVER_OBJ, 'Q', buf);
    }

    /* r: read item */
    if (item_reading_classification(otmp, buf) == IA_READ_OBJ)
        ia_addmenu(win, IA_READ_OBJ, 'r', buf);

    /* R: remove accessory or rub item */
    if (otmp->owornmask & W_ACCESSORY) {
        Sprintf(buf, "Remove this %s",
                (otmp->owornmask & W_AMUL) ? "amulet"
                : (otmp->owornmask & W_RING) ? "ring"
                  : (otmp->owornmask & W_TOOL) ? "eyewear"
                    : "accessory"); /* catchall -- can't happen */
        ia_addmenu(win, IA_TAKEOFF_OBJ, 'R', buf);
    }
    if (otmp->otyp == OIL_LAMP || otmp->otyp == MAGIC_LAMP
        || otmp->otyp == BRASS_LANTERN) {
        Sprintf(buf, "Rub this %s", simpleonames(otmp));
        ia_addmenu(win, IA_RUB_OBJ, 'R', buf);
    } else if (otmp->oclass == GEM_CLASS && is_graystone(otmp))
        ia_addmenu(win, IA_RUB_OBJ, 'R', "Rub something on this stone");

    /* t: throw item */
    if (!already_worn) {
        boolean shoot = ammo_and_launcher(otmp, uwep);

        /*
         * FIXME:
         *  'one of these' should be changed to 'some of these' when there
         *  is the possibility of a multi-shot volley but we don't have
         *  any way to determine that except by actually calculating the
         *  volley count and that could randomly yield 1 here and 2..N
         *  while throwing or vice versa.
         */
        Sprintf(buf, "%s %s%s", shoot ? "Shoot" : "Throw",
                (otmp->quan == 1L) ? "this item"
                : (otmp->otyp == GOLD_PIECE) ? "them"
                  : "one of these",
                /* if otmp is quivered, we've already listed
                   'f - shoot|throw this item' as a choice;
                   if 't' is duplicating that, say so ('t' and 'f'
                   behavior differs for throwing a stack of gold) */
                (otmp == uquiver && (otmp->otyp != GOLD_PIECE
                                     || otmp->quan == 1L))
                ? " (same as 'f')" : "");
        ia_addmenu(win, IA_THROW_OBJ, 't', buf);
    }

    /* T: take off armor, tip carried container */
    if (otmp->owornmask & W_ARMOR)
        ia_addmenu(win, IA_TAKEOFF_OBJ, 'T', "Take off this armor");
    if ((Is_container(otmp) && (Has_contents(otmp) || !otmp->cknown))
        || (otmp->otyp == HORN_OF_PLENTY && (otmp->spe > 0 || !otmp->known)))
        ia_addmenu(win, IA_TIP_CONTAINER, 'T',
                   "Tip all the contents out of this container");

    /* V: invoke */
    if ((otmp->otyp == FAKE_AMULET_OF_YENDOR && !otmp->known)
        || otmp->oartifact || objects[otmp->otyp].oc_unique
        /* non-artifact crystal balls don't have any unique power but
           the #invoke command lists them as likely candidates */
        || otmp->otyp == CRYSTAL_BALL)
        ia_addmenu(win, IA_INVOKE_OBJ, 'V',
                   "Try to invoke a unique power of this object");

    /* w: wield, hold in hands, works on everything but with different
       advice text; not mentioned for things that are already wielded */
    if (otmp == uwep || cantwield(gy.youmonst.data)) {
        ; /* either already wielded or can't wield anything; skip 'w' */
    } else if (otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
               || is_wet_towel(otmp) || otmp->otyp == HEAVY_IRON_BALL) {
        Sprintf(buf, "Wield this %s as your weapon",
                (otmp->quan > 1L) ? "stack" : "item");
        ia_addmenu(win, IA_WIELD_OBJ, 'w', buf);
    } else if (otmp->otyp == TIN_OPENER) {
        ia_addmenu(win, IA_WIELD_OBJ, 'w',
                   "Wield the tin opener to easily open tins");
    } else if (!already_worn) {
        /* originally this was using "hold this item in your hands" but
           there's no concept of "holding an item", plus it unwields
           whatever item you already have wielded so use "wield this item" */
        Sprintf(buf, "Wield this %s in your %s",
                (otmp->quan > 1L) ? "stack" : "item",
                /* only two-handed weapons and unicorn horns care about
                   pluralizing "hand" and they won't reach here, but plural
                   sounds better when poly'd into something with "claw" */
                makeplural(body_part(HAND)));
        ia_addmenu(win, IA_WIELD_OBJ, 'w', buf);
    }

    /* W: wear armor */
    if (!already_worn) {
        if (otmp->oclass == ARMOR_CLASS) {
            /* if 'otmp' is worn we skip 'W' (and show 'T' above instead);
               if it isn't, we either show "W - wear this" if otmp's slot
               isn't populated, or "W - [already wearing <simple-armor>]";
               for the latter, picking 'W' will fail but we don't want to
               omit 'W' in this situation */
            long Wmask = armcat_to_wornmask(objects[otmp->otyp].oc_armcat);
            struct obj *o = wearmask_to_obj(Wmask);

            if (!o)
                Strcpy(buf, "Wear this armor");
            else
                Sprintf(buf, "[already wearing %s]", an(armor_simple_name(o)));

            ia_addmenu(win, IA_WEAR_OBJ, 'W', buf);
        }
    }

    /* x: Swap main and readied weapon */
    if (otmp == uwep && uswapwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Swap this with your alternate weapon");
    else if (otmp == uwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Ready this as an alternate weapon");
    else if (otmp == uswapwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Swap this with your main weapon");

    /* this is based on TWOWEAPOK() in wield.c; we don't call can_two_weapon()
       because it is very verbose; attempting to two-weapon might be rejected
       but we screen out most reasons for rejection before offering it as a
       choice */
#define MAYBETWOWEAPON(obj) \
    ((((obj)->oclass == WEAPON_CLASS)                           \
      ? !(is_launcher(obj) || is_ammo(obj) || is_missile(obj))  \
      : is_weptool(obj))                                        \
     && !bimanual(obj))

    /* X: Toggle two-weapon mode on or off */
    if ((otmp == uwep || otmp == uswapwep)
        /* if already two-weaponing, no special checks needed to toggle off */
        && (u.twoweap
        /* but if not, try to filter most "you can't do that" here */
            || (could_twoweap(gy.youmonst.data) && !uarms
                && uwep && MAYBETWOWEAPON(uwep)
                && uswapwep && MAYBETWOWEAPON(uswapwep)))) {
        Sprintf(buf, "Toggle two-weapon combat %s", u.twoweap ? "off" : "on");
        ia_addmenu(win, IA_TWOWEAPON, 'X', buf);
    }

#undef MAYBETWOWEAPON

    /* z: Zap wand */
    if (otmp->oclass == WAND_CLASS)
        ia_addmenu(win, IA_ZAP_OBJ, 'z',
                   "Zap this wand to release its magic");

    /* ?: Look up an item in the game's database */
    if (ia_checkfile(otmp)) {
        Sprintf(buf, "Look up information about %s",
                (otmp->quan > 1L) ? "these" : "this");
        ia_addmenu(win, IA_WHATIS_OBJ, '/', buf);
    }

    Sprintf(buf, "Do what with %s?", the(cxname(otmp)));
    end_menu(win, buf);

    n = select_menu(win, PICK_ONE, &selected);

    if (n > 0) {
        act = selected[0].item.a_int;
        free((genericptr_t) selected);

        itemactions_pushkeys(otmp, act);
    }
    destroy_nhwindow(win);

    /* finish the 'i' command:  no time elapses and cancelling without
       selecting an action doesn't matter */
    return ECMD_OK;
}

/*iactions.c*/
