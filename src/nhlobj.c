/* NetHack 5.0	nhlobj.c	$NHDT-Date: 1781973058 2026/06/20 16:30:58 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.42 $ */
/*      Copyright (c) 2019 by Pasi Kallinen */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file nhlobj.c
 * @brief NetHack objects as seen from Lua.
 *
 * Level scripts need to create objects, look at them, and place them, but they
 * must not be handed a raw @c struct obj -- a script could outlive the object,
 * or keep a reference to one the game has already freed.
 *
 * So each object reachable from Lua is wrapped in a small userdata that records
 * whether the object still belongs to the game or to the script, and every
 * access checks that the wrapper is still valid before touching anything.
 *
 * @warning Ownership is the whole difficulty here. An object a script created
 *          but never placed must be freed with the script; one that has been
 *          placed on the map belongs to the game and must not be.
 */

/**
 * @file nhlobj.c
 * @brief Lua 에서 바라본 NetHack 객체.
 *
 * 레벨 스크립트는 객체를 만들고, 살펴보고, 배치해야 하지만 날것의
 * @c struct obj 를 건네받아서는 안 된다. 스크립트가 객체보다 오래 살 수도 있고,
 * 게임이 이미 해제한 객체를 계속 참조할 수도 있기 때문이다.
 *
 * 그래서 Lua 에서 닿을 수 있는 객체는 작은 userdata 로 감싸며, 그 객체가 여전히
 * 게임의 것인지 스크립트의 것인지를 기록한다. 모든 접근은 무언가를 건드리기 전에
 * 래퍼가 아직 유효한지 확인한다.
 *
 * @warning 여기서 어려운 것은 전적으로 소유권이다. 스크립트가 만들었지만 배치하지
 *          않은 객체는 스크립트와 함께 해제되어야 하고, 지도에 놓인 객체는 게임의
 *          것이므로 그래서는 안 된다.
 */

#include "hack.h"
#include "sp_lev.h"

struct _lua_obj {
    int state; /* UNUSED */
    struct obj *obj;
};

staticfn struct _lua_obj *l_obj_check(lua_State *, int);
staticfn int l_obj_add_to_container(lua_State *);
staticfn int l_obj_gc(lua_State *);
staticfn int l_obj_getcontents(lua_State *);
staticfn int l_obj_isnull(lua_State *);
staticfn int l_obj_new_readobjnam(lua_State *);
staticfn int l_obj_nextobj(lua_State *);
staticfn int l_obj_objects_to_table(lua_State *);
staticfn int l_obj_placeobj(lua_State *);
staticfn int l_obj_to_table(lua_State *);
staticfn int l_obj_at(lua_State *);
staticfn int l_obj_container(lua_State *);
staticfn int l_obj_timer_has(lua_State *);
staticfn int l_obj_timer_peek(lua_State *);
staticfn int l_obj_timer_stop(lua_State *);
staticfn int l_obj_timer_start(lua_State *);
staticfn int l_obj_bury(lua_State *);
staticfn struct _lua_obj *l_obj_push(lua_State *, struct obj *);

/**
 * @def lobj_is_ok
 * @brief Whether a wrapper still refers to an object a script may touch.
 * @note Three things have to hold, and the third is the interesting one: an object marked as belonging to the script's side of the boundary has been handed over and must not be reached through the wrapper any more. So a
 *       wrapper can be valid, hold an object, and still not be usable.
 */
/**
 * @def lobj_is_ok
 * @brief 래퍼가 여전히 스크립트가 건드려도 되는 객체를 가리키는지.
 * @note 세 가지가 성립해야 하고 세 번째가 흥미로운 것이다. 스크립트 쪽 경계에 속한다고 표시된 객체는 넘겨진 것이며 더는 그 래퍼를 통해 닿아서는 안 된다. 그래서 래퍼가 유효하고 객체를 담고 있으면서도 여전히 쓸 수 없을 수 있다.
 */
#define lobj_is_ok(lo) ((lo) && (lo)->obj && (lo)->obj->where != OBJ_LUAFREE)

/**
 * @brief Fetch the wrapper at a position on the script's stack, refusing anything else.
 * @param L the script
 * @param indx where on the stack to look
 * @return the wrapper
 * @note Every routine here begins with this, because a script may pass anything at all and the alternative to checking is a crash caused by a level file.
 */
/**
 * @brief 스크립트 스택의 어떤 위치에서 래퍼를 가져오며, 그 밖의 것은 거부한다.
 * @param L 그 스크립트
 * @param indx 스택의 어디를 볼지
 * @return 그 래퍼
 * @note 여기의 모든 루틴이 이것으로 시작한다. 스크립트가 무엇이든 넘길 수 있고, 검사하지 않는 것의 대안은 레벨 파일이 일으키는 충돌이기 때문이다.
 */
staticfn struct _lua_obj *
l_obj_check(lua_State *L, int indx)
{
    struct _lua_obj *lo;

    luaL_checktype(L, indx, LUA_TUSERDATA);
    lo = (struct _lua_obj *) luaL_checkudata(L, indx, "obj");
    if (!lo)
        nhl_error(L, "Obj error");
    return lo;
}

/**
 * @brief Release a wrapper the script has finished with, freeing the object if nobody else wants it.
 *
 * The heart of the ownership problem the file header describes. A script may hold several wrappers for one object, so the object counts how many, and only when the last is released and the object belongs to nobody is it
 * actually freed.
 *
 * An object still on the map or in someone's pack is left alone however many wrappers went away, because the game owns it. An object a script created and never placed is owned by nothing once its wrappers are gone, and
 * freeing it here is what stops a level script from leaking every object it made and discarded.
 *
 * @param L the script
 * @return no values, as the script's collector expects
 * @warning Contents are freed with the container. An object inside one the script abandoned is not separately reachable, so it would otherwise be leaked -- but a script that kept a wrapper for the contents alone has that
 *          wrapper's count keeping the contents alive, not the container's.
 * @note Called by the script's own garbage collector at a time the game does not choose. So it must be safe to run at any point, which is why it does nothing beyond counting and freeing.
 */
/**
 * @brief 스크립트가 다 쓴 래퍼를 놓아주며, 아무도 그 객체를 원하지 않으면 그것을 해제한다.
 *
 * 파일 머리말이 기술하는 소유권 문제의 심장이다. 스크립트는 한 객체에 대해 여러 래퍼를 쥘 수 있으므로 객체가 그것이 몇 개인지 세며, 마지막 것이 놓아지고 그 객체가 아무에게도 속하지 않을 때만 실제로 해제된다.
 *
 * 여전히 지도 위나 누군가의 가방에 있는 객체는 래퍼가 몇 개 사라졌든 그대로 남는다. 게임이 그것을 소유하기 때문이다. 스크립트가 만들고 배치하지 않은 객체는 그 래퍼들이 사라지면 아무것에도 속하지 않으며, 여기서 그것을 해제하는 것이 레벨 스크립트가 만들고 버린 모든 객체를 누수하지 않게
 * 하는 것이다.
 *
 * @param L 그 스크립트
 * @return 스크립트의 수집기가 기대하는 대로 값 없음
 * @warning 내용물은 용기와 함께 해제된다. 스크립트가 버린 용기 안의 객체는 따로 닿을 수 없으므로 그러지 않으면 누수된다. 그런데 내용물만에 대한 래퍼를 쥔 스크립트는 그 래퍼의 개수가 내용물을 살려 두는 것이며, 용기의 것이 아니다.
 * @note 게임이 고르지 않은 시점에 스크립트 자신의 쓰레기 수집기가 호출한다. 그래서 어느 지점에서든 실행되어도 안전해야 하며, 그것이 세기와 해제하기 이상의 아무것도 하지 않는 이유다.
 */
staticfn int
l_obj_gc(lua_State *L)
{
    struct obj *obj, *otmp;
    struct _lua_obj *lo = l_obj_check(L, 1);

    if (lo && (obj = lo->obj) != 0) {
        if (obj->lua_ref_cnt > 0)
            obj->lua_ref_cnt--;
        /* free-floating objects with no other refs are deallocated. */
        if (!obj->lua_ref_cnt
            && (obj->where == OBJ_FREE || obj->where == OBJ_LUAFREE)) {
            if (Has_contents(obj)) {
                while ((otmp = obj->cobj) != 0) {
                    obj_extract_self(otmp);
                    dealloc_obj(otmp);
                }
            }
            obj->where = OBJ_FREE;
            dealloc_obj(obj), obj = 0;
        }
        lo->obj = NULL;
    }
    return 0;
}

/**
 * @brief Wrap an object and put the wrapper on the script's stack.
 * @param L the script
 * @param otmp the object, which may be null
 * @return the new wrapper
 * @note Increments the object's count of wrappers, which is the other half of what the collector decrements. The pairing is what makes the ownership counting work, so a wrapper must never be created except through here.
 * @note A null object is allowed and produces a wrapper referring to nothing, because a script asking for an object that does not exist should receive something it can test rather than an error.
 */
/**
 * @brief 객체를 감싸고 그 래퍼를 스크립트 스택에 올린다.
 * @param L 그 스크립트
 * @param otmp 그 객체. 널일 수 있다
 * @return 새 래퍼
 * @note 그 객체의 래퍼 개수를 올리며, 그것이 수집기가 내리는 것의 나머지 절반이다. 그 짝지음이 소유권 세기를 작동하게 하므로, 래퍼는 결코 여기를 거치지 않고 만들어져서는 안 된다.
 * @note 널 객체가 허용되며 아무것도 가리키지 않는 래퍼를 만든다. 존재하지 않는 객체를 요청한 스크립트는 오류가 아니라 검사할 수 있는 무언가를 받아야 하기 때문이다.
 */
staticfn struct _lua_obj *
l_obj_push(lua_State *L, struct obj *otmp)
{
    struct _lua_obj *lo
        = (struct _lua_obj *) lua_newuserdata(L, sizeof (struct _lua_obj));
    luaL_getmetatable(L, "obj");
    lua_setmetatable(L, -2);

    lo->state = 0;
    lo->obj = otmp;
    if (otmp)
        otmp->lua_ref_cnt++;

    return lo;
}

void
nhl_push_obj(lua_State *L, struct obj *otmp)
{
    (void) l_obj_push(L, otmp);
}

/* local o = obj.new("large chest");
   local cobj = o:contents(); */
staticfn int
l_obj_getcontents(lua_State *L)
{
    struct _lua_obj *lo = l_obj_check(L, 1);
    struct obj *obj = lo->obj;

    if (!obj)
        nhl_error(L, "l_obj_getcontents: no obj");

    (void) l_obj_push(L, obj->cobj);
    return 1;
}

/**
 * @brief Put one object inside another, from a script.
 *
 * @param L the script, with the container and the object on its stack
 * @return no values
 * @warning The object may be merged with something already in the container and cease to exist. That is why the wrapper's count is read before the move and added to whatever survives: the script's references have to end up
 *          pointing at the surviving object, or the collector would later decrement a count on the wrong one and free something the script still holds.
 * @note The container's weight is recomputed here rather than lazily, because the weight of a container is not derived when read -- it is stored, and nothing else would notice that it changed.
 */
/**
 * @brief 스크립트에서, 한 객체를 다른 객체 안에 넣는다.
 *
 * @param L 스택에 용기와 객체가 놓인 그 스크립트
 * @return 값 없음
 * @warning 그 객체가 용기 안에 이미 있는 것과 합쳐져 존재하기를 그만둘 수 있다. 그것이 옮기기 전에 래퍼의 개수를 읽어 살아남은 것에 더하는 이유다. 스크립트의 참조가 살아남은 객체를 가리키게 되어야 하며, 그러지 않으면 수집기가 나중에 잘못된 것의 개수를 내리고 스크립트가 여전히 쥔 것을
 *          해제하게 된다.
 * @note 용기의 무게가 나중이 아니라 여기서 다시 계산된다. 용기의 무게는 읽힐 때 유도되는 것이 아니라 저장되며, 그것이 바뀐 것을 다른 어느 것도 알아채지 못하기 때문이다.
 */
/* Puts object inside another object. */
/* local box = obj.new("large chest");
   box:addcontent(obj.new("rock"));
*/
staticfn int
l_obj_add_to_container(lua_State *L)
{
    struct _lua_obj *lobox = l_obj_check(L, 1);
    struct _lua_obj *lo = l_obj_check(L, 2);
    struct obj *otmp;
    int refs;

    if (!lobj_is_ok(lo) || !lobj_is_ok(lobox))
        return 0;

    refs = lo->obj->lua_ref_cnt;

    obj_extract_self(lo->obj);
    otmp = add_to_container(lobox->obj, lo->obj);

    /* was lo->obj merged? */
    if (otmp != lo->obj) {
        lo->obj = otmp;
        lo->obj->lua_ref_cnt += refs;
    }
    lobox->obj->owt = weight(lobox->obj);

    return 0;
}

/**
 * @brief Give an object to the hero, from a script.
 *
 * @param L the script, with the object on its stack
 * @return no values
 * @note The same merging hazard as putting something in a container, and handled the same way -- adding to the pack may merge the object with one the hero already has.
 * @note Declines silently if the object is already in the pack, rather than adding it twice. A script that gives the same object twice has made a mistake, but refusing is better than corrupting the inventory over it.
 * @warning This is a script placing an object into the game's ownership. From here on the game may destroy it, and the wrapper the script holds is only usable while the object remains.
 */
/**
 * @brief 스크립트에서, 영웅에게 객체를 준다.
 *
 * @param L 스택에 그 객체가 놓인 그 스크립트
 * @return 값 없음
 * @note 용기에 무언가를 넣는 것과 같은 합치기 위험이 있고 같은 방식으로 처리된다. 가방에 더하는 것이 그 객체를 영웅이 이미 가진 것과 합칠 수 있다.
 * @note 그 객체가 이미 가방에 있으면 두 번 더하는 대신 조용히 물러난다. 같은 객체를 두 번 주는 스크립트는 잘못한 것이지만, 그것 때문에 소지품을 손상시키기보다 거부하는 것이 낫다.
 * @warning 이것은 스크립트가 객체를 게임의 소유로 놓는 일이다. 여기서부터 게임이 그것을 파괴할 수 있고, 스크립트가 쥔 래퍼는 그 객체가 남아 있는 동안만 쓸 수 있다.
 */
/* Put object into player's inventory */
/* u.giveobj(obj.new("rock")); */
int
nhl_obj_u_giveobj(lua_State *L)
{
    struct _lua_obj *lo = l_obj_check(L, 1);
    struct obj *otmp;
    int refs;

    if (!lobj_is_ok(lo) || lo->obj->where == OBJ_INVENT)
        return 0;

    refs = lo->obj->lua_ref_cnt;

    obj_extract_self(lo->obj);
    otmp = addinv(lo->obj);

    if (otmp != lo->obj) {
        lo->obj->lua_ref_cnt += refs;
        lo->obj = otmp;
    }

    return 0;
}

DISABLE_WARNING_UNREACHABLE_CODE

/**
 * @brief Give a script the facts about a kind of object, as a table.
 *
 * @param L the script, with either an object or a kind on its stack
 * @return one table
 * @note Accepts either an object or a kind number, as the accompanying examples show, because a script asking "what are rocks like" and one asking "what is this rock's kind like" want the same answer. That leniency is the
 *       reason for the argument inspection.
 * @note What it returns is the class row and not the object: weight, price, material, what it is called. So it is the same for every rock and tells a script nothing about the particular one.
 */
/**
 * @brief 스크립트에게 어떤 종류의 객체에 관한 사실을 표로 준다.
 *
 * @param L 스택에 객체나 종류 중 하나가 놓인 그 스크립트
 * @return 표 하나
 * @note 딸린 예시들이 보여 주듯 객체나 종류 번호 중 어느 것도 받아들인다. "돌은 어떤가"를 묻는 스크립트와 "이 돌의 종류는 어떤가"를 묻는 스크립트가 같은 답을 원하기 때문이다. 그 관대함이 인자를 살피는 이유다.
 * @note 반환하는 것은 객체가 아니라 계열의 줄이다. 무게, 값, 재질, 무엇이라 불리는지. 그래서 모든 돌에 대해 같으며 스크립트에게 그 특정한 돌에 대해서는 아무것도 말해 주지 않는다.
 */
/* Get a table of object class data. */
/* local odata = obj.class(otbl.otyp); */
/* local odata = obj.class(obj.new("rock")); */
/* local odata = o:class(); */
staticfn int
l_obj_objects_to_table(lua_State *L)
{
    int argc = lua_gettop(L);
    int otyp = -1;
    struct objclass *o;

    if (argc != 1) {
        nhl_error(L, "l_obj_objects_to_table: Wrong args");
        /*NOTREACHED*/
        return 0;
    }

    if (lua_type(L, 1) == LUA_TNUMBER) {
        otyp = (int) luaL_checkinteger(L, 1);
    } else if (lua_type(L, 1) == LUA_TUSERDATA) {
        struct _lua_obj *lo = l_obj_check(L, 1);
        if (lo && lo->obj)
            otyp = lo->obj->otyp;
    }
    lua_pop(L, 1);

    if (otyp == -1) {
        nhl_error(L, "l_obj_objects_to_table: Wrong args");
        /*NOTREACHED*/
        return 0;
    }

    o = &objects[otyp];

    lua_newtable(L);

    if (OBJ_NAME(objects[otyp]))
        nhl_add_table_entry_str(L, "name", OBJ_NAME(objects[otyp]));
    if (OBJ_DESCR(objects[otyp]))
        nhl_add_table_entry_str(L, "descr",
                                OBJ_DESCR(objects[otyp]));
    if (o->oc_uname)
        nhl_add_table_entry_str(L, "uname", o->oc_uname);

    nhl_add_table_entry_int(L, "name_known", o->oc_name_known);
    nhl_add_table_entry_int(L, "merge", o->oc_merge);
    nhl_add_table_entry_int(L, "uses_known", o->oc_uses_known);
    nhl_add_table_entry_int(L, "encountered", o->oc_encountered);
    nhl_add_table_entry_int(L, "magic", o->oc_magic);
    nhl_add_table_entry_int(L, "charged", o->oc_charged);
    nhl_add_table_entry_int(L, "unique", o->oc_unique);
    nhl_add_table_entry_int(L, "nowish", o->oc_nowish);
    nhl_add_table_entry_int(L, "big", o->oc_big);
    /* TODO: oc_bimanual, oc_bulky */
    nhl_add_table_entry_int(L, "tough", o->oc_tough);
    nhl_add_table_entry_int(L, "dir", o->oc_dir); /* TODO: convert to text */
    nhl_add_table_entry_str(L, "material", materialnm[o->oc_material]);
    /* TODO: oc_subtyp, oc_skill, oc_armcat */
    nhl_add_table_entry_int(L, "oprop", o->oc_oprop);
    nhl_add_table_entry_char(L, "class",
                             def_oc_syms[(uchar) o->oc_class].sym);
    nhl_add_table_entry_int(L, "delay", o->oc_delay);
    nhl_add_table_entry_int(L, "color", o->oc_color); /* TODO: text? */
    nhl_add_table_entry_int(L, "prob", o->oc_prob);
    nhl_add_table_entry_int(L, "weight", o->oc_weight);
    nhl_add_table_entry_int(L, "cost", o->oc_cost);
    nhl_add_table_entry_int(L, "damage_small", o->oc_wsdam);
    nhl_add_table_entry_int(L, "damage_large", o->oc_wldam);
    /* TODO: oc_oc1, oc_oc2, oc_hitbon, a_ac, a_can, oc_level */
    nhl_add_table_entry_int(L, "nutrition", o->oc_nutrition);

    return 1;
}

RESTORE_WARNING_UNREACHABLE_CODE

/**
 * @brief Give a script a plain table holding everything about one object.
 *
 * A snapshot rather than a view. Every field is copied out, so a script that keeps the table keeps the values as they were -- which is deliberate: a table cannot become stale in the dangerous way a wrapper can, because it
 * refers to nothing.
 *
 * @param L the script, with the object on its stack
 * @return one table
 * @note An object that no longer exists yields a table saying so rather than an error, so a script may examine something it is not sure is still there.
 * @warning Changing the table changes nothing. It is a copy, and a script that expects to alter an object by writing into its table will find the object unchanged.
 */
/**
 * @brief 스크립트에게 한 객체에 관한 모든 것을 담은 평범한 표를 준다.
 *
 * 뷰가 아니라 스냅숏이다. 모든 필드가 복사되어 나오므로, 그 표를 쥔 스크립트는 그때의 값을 쥔다. 의도적이다. 표는 래퍼가 그럴 수 있는 위험한 방식으로 낡아질 수 없다. 아무것도 가리키지 않기 때문이다.
 *
 * @param L 스택에 그 객체가 놓인 그 스크립트
 * @return 표 하나
 * @note 더는 존재하지 않는 객체는 오류가 아니라 그렇다고 말하는 표를 낸다. 그래서 스크립트가 아직 있는지 확실하지 않은 것을 살펴볼 수 있다.
 * @warning 그 표를 바꾸는 것은 아무것도 바꾸지 않는다. 그것은 사본이며, 표에 써서 객체를 바꾸려는 스크립트는 그 객체가 그대로임을 알게 된다.
 */
/* Create a lua table representation of the object, unpacking all the
   object fields.
   local o = obj.new("rock");
   local otbl = o:totable(); */
staticfn int
l_obj_to_table(lua_State *L)
{
    struct _lua_obj *lo = l_obj_check(L, 1);
    struct obj *obj = lo->obj;

    lua_newtable(L);

    if (!obj || obj->where == OBJ_LUAFREE) {
        nhl_add_table_entry_int(L, "NO_OBJ", 1);
        return 1;
    }

    nhl_add_table_entry_int(L, "has_contents", Has_contents(obj));
    nhl_add_table_entry_int(L, "is_container", Is_container(obj));
    nhl_add_table_entry_int(L, "o_id", obj->o_id);
    nhl_add_table_entry_int(L, "ox", obj->ox);
    nhl_add_table_entry_int(L, "oy", obj->oy);
    nhl_add_table_entry_int(L, "otyp", obj->otyp);
    if (OBJ_NAME(objects[obj->otyp]))
        nhl_add_table_entry_str(L, "otyp_name", OBJ_NAME(objects[obj->otyp]));
    if (OBJ_DESCR(objects[obj->otyp]))
        nhl_add_table_entry_str(L, "otyp_descr",
                                OBJ_DESCR(objects[obj->otyp]));
    nhl_add_table_entry_int(L, "owt", obj->owt);
    nhl_add_table_entry_int(L, "quan", obj->quan);
    nhl_add_table_entry_int(L, "spe", obj->spe);

    if (obj->otyp == STATUE)
        nhl_add_table_entry_int(L, "historic",
                                (obj->spe & CORPSTAT_HISTORIC) != 0);
    if (obj->otyp == CORPSE || obj->otyp == STATUE) {
        nhl_add_table_entry_int(L, "male",
                                (obj->spe & CORPSTAT_MALE) != 0);
        nhl_add_table_entry_int(L, "female",
                                (obj->spe & CORPSTAT_FEMALE) != 0);
    }

    nhl_add_table_entry_char(L, "oclass",
                             def_oc_syms[(uchar) obj->oclass].sym);
    nhl_add_table_entry_char(L, "invlet", obj->invlet);
    /* TODO: nhl_add_table_entry_char(L, "oartifact", obj->oartifact);*/
    nhl_add_table_entry_int(L, "where", obj->where);
    /* TODO: nhl_add_table_entry_int(L, "timed", obj->timed); */
    nhl_add_table_entry_int(L, "cursed", obj->cursed);
    nhl_add_table_entry_int(L, "blessed", obj->blessed);
    nhl_add_table_entry_int(L, "unpaid", obj->unpaid);
    nhl_add_table_entry_int(L, "no_charge", obj->no_charge);
    nhl_add_table_entry_int(L, "known", obj->known);
    nhl_add_table_entry_int(L, "dknown", obj->dknown);
    nhl_add_table_entry_int(L, "bknown", obj->bknown);
    nhl_add_table_entry_int(L, "rknown", obj->rknown);
    nhl_add_table_entry_int(L, "tknown", obj->tknown);
    if (obj->oclass == POTION_CLASS)
        nhl_add_table_entry_int(L, "odiluted", obj->odiluted);
    else
        nhl_add_table_entry_int(L, "oeroded", obj->oeroded);
    nhl_add_table_entry_int(L, "oeroded2", obj->oeroded2);
    /* TODO: orotten, norevive */
    nhl_add_table_entry_int(L, "oerodeproof", obj->oerodeproof);
    nhl_add_table_entry_int(L, "olocked", obj->olocked);
    nhl_add_table_entry_int(L, "obroken", obj->obroken);
    if (is_poisonable(obj))
        nhl_add_table_entry_int(L, "opoisoned", obj->opoisoned);
    else
        nhl_add_table_entry_int(L, "otrapped", obj->otrapped);
    /* TODO: degraded_horn */
    nhl_add_table_entry_int(L, "recharged", obj->recharged);
    /* TODO: on_ice */
    nhl_add_table_entry_int(L, "lamplit", obj->lamplit);
    nhl_add_table_entry_int(L, "globby", obj->globby);
    nhl_add_table_entry_int(L, "greased", obj->greased);
    nhl_add_table_entry_int(L, "nomerge", obj->nomerge);
    nhl_add_table_entry_int(L, "how_lost", obj->how_lost);
    nhl_add_table_entry_int(L, "in_use", obj->in_use);
    nhl_add_table_entry_int(L, "bypass", obj->bypass);
    nhl_add_table_entry_int(L, "cknown", obj->cknown);
    nhl_add_table_entry_int(L, "lknown", obj->lknown);
    nhl_add_table_entry_int(L, "corpsenm", obj->corpsenm);
    if (obj->corpsenm != NON_PM
        && (obj->otyp == TIN || obj->otyp == CORPSE || obj->otyp == EGG
            || obj->otyp == FIGURINE || obj->otyp == STATUE))
        nhl_add_table_entry_str(L, "corpsenm_name",
                                mons[obj->corpsenm].pmnames[NEUTRAL]);
    /* TODO: leashmon, fromsink, novelidx, record_achieve_special */
    nhl_add_table_entry_int(L, "usecount", obj->usecount);
    /* TODO: spestudied */
    nhl_add_table_entry_int(L, "oeaten", obj->oeaten);
    nhl_add_table_entry_int(L, "age", obj->age);
    nhl_add_table_entry_int(L, "owornmask", obj->owornmask);
    /* TODO: more of oextra */
    nhl_add_table_entry_int(L, "has_oname", has_oname(obj));
    if (has_oname(obj))
        nhl_add_table_entry_str(L, "oname", ONAME(obj));

    return 1;
}

DISABLE_WARNING_UNREACHABLE_CODE

/**
 * @brief Create an object from a description, as a wish would.
 *
 * A script names what it wants the way a player naming a wish would, and gets the object that produces. Reusing the wishing routine rather than a separate creation path is why a level file can ask for "a blessed +2 long
 * sword" and get exactly that.
 *
 * @param L the script, with either a name or a table of properties on its stack
 * @return one wrapper
 * @note Accepts a table as well as a name, which is the form to prefer for anything the script assembles -- building a name as text and parsing it back is fragile where the parts are already separate.
 * @warning The object created belongs to the script until it is placed. If the script discards its wrapper without placing it, the object is freed -- which is correct, and is also why an object created and forgotten is not a
 *          leak here.
 */
/**
 * @brief 기술에서 객체를 만든다. 소원이 그렇게 하듯.
 *
 * 스크립트는 소원을 말하는 플레이어가 하듯 자신이 원하는 것을 지칭하고, 그것이 만들어 내는 객체를 받는다. 별도의 생성 경로가 아니라 소원 루틴을 재사용하는 것이, 레벨 파일이 "축복받은 +2 장검"을 요청해 정확히 그것을 받을 수 있는 이유다.
 *
 * @param L 스택에 이름이나 속성 표 중 하나가 놓인 그 스크립트
 * @return 래퍼 하나
 * @note 이름뿐 아니라 표도 받아들이며, 스크립트가 조립하는 것에는 그 형태가 나은 쪽이다. 이미 부분이 나뉘어 있는 곳에서 이름을 글로 만들어 되파싱하는 것은 취약하다.
 * @warning 만들어진 객체는 배치되기 전까지 스크립트에 속한다. 스크립트가 그것을 배치하지 않고 래퍼를 버리면 그 객체는 해제된다. 그것이 올바르며, 만들고 잊은 객체가 여기서 누수가 아닌 이유이기도 하다.
 */
/* create a new object via wishing routine */
/* local o = obj.new("rock"); */
/* local o = obj.new({ id = "food ration", class = "%" }); */
staticfn int
l_obj_new_readobjnam(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 1 && lua_type(L, 1) == LUA_TSTRING) {
        char buf[BUFSZ];
        struct obj *otmp;

        Sprintf(buf, "%s", luaL_checkstring(L, 1));
        lua_pop(L, 1);
        if ((otmp = readobjnam(buf, NULL)) == &hands_obj)
            otmp = NULL;
        (void) l_obj_push(L, otmp);
        return 1;
    } else if (argc == 1 && lua_type(L, 1) == LUA_TTABLE) {
        short id = get_table_objtype(L);
        xint16 class = get_table_objclass(L);
        struct obj *otmp;

        if (id >= FIRST_OBJECT) {
            otmp = mksobj(id, TRUE, FALSE);
        } else {
            class = def_char_to_objclass(class);
            if (class >= MAXOCLASSES)
                class = RANDOM_CLASS;
            otmp = mkobj(class, FALSE);
        }
        lua_pop(L, 1);
        (void) l_obj_push(L, otmp);
        return 1;
    } else
        nhl_error(L, "l_obj_new_readobjname: Wrong args");
    /*NOTREACHED*/
    return 0;
}

/**
 * @brief The topmost object on a square, as a wrapper.
 * @param L the script, with the coordinates on its stack
 * @return one wrapper, referring to nothing if the square is empty
 * @note The coordinates are converted from the script's own frame of reference, because a level file describes positions relative to the map it is drawing rather than to the whole level.
 * @note Only the topmost. A script wanting the rest walks them with the next-object routine, which is why that exists separately.
 */
/**
 * @brief 어떤 칸의 맨 위 객체. 래퍼로.
 * @param L 스택에 좌표가 놓인 그 스크립트
 * @return 래퍼 하나. 그 칸이 비어 있으면 아무것도 가리키지 않는다
 * @note 좌표는 스크립트 자신의 기준계에서 변환된다. 레벨 파일이 위치를 레벨 전체가 아니라 자신이 그리고 있는 지도에 상대적으로 기술하기 때문이다.
 * @note 맨 위의 것만이다. 나머지를 원하는 스크립트는 다음 객체 루틴으로 그것들을 돈다. 그것이 따로 존재하는 이유다.
 */
/* Get the topmost object on the map at x,y */
/* local o = obj.at(x, y); */
staticfn int
l_obj_at(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 2) {
        coordxy x, y;

        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        cvt_to_abscoord(&x, &y);

        lua_pop(L, 2);
        (void) l_obj_push(L, svl.level.objects[x][y]);
        return 1;
    } else
        nhl_error(L, "l_obj_at: Wrong args");
    /*NOTREACHED*/
    return 0;
}

/**
 * @brief Put a script's object onto the map.
 *
 * The moment ownership passes from the script to the game, and therefore the moment the file header's whole difficulty is resolved for that object: from here the game will free it when it should be freed, and the script's
 * wrapper is valid only while it remains.
 *
 * @param L the script, with the object and the coordinates on its stack
 * @return no values
 * @note The same merging hazard as the other placement routines -- an object put where a like one already lies may cease to exist, and the wrapper is corrected to the survivor.
 */
/**
 * @brief 스크립트의 객체를 지도 위에 놓는다.
 *
 * 소유권이 스크립트에서 게임으로 넘어가는 순간이며, 따라서 그 객체에 대해 파일 머리말의 어려움 전체가 해소되는 순간이다. 여기서부터 게임이 해제되어야 할 때 그것을 해제하며, 스크립트의 래퍼는 그것이 남아 있는 동안만 유효하다.
 *
 * @param L 스택에 객체와 좌표가 놓인 그 스크립트
 * @return 값 없음
 * @note 다른 배치 루틴들과 같은 합치기 위험이 있다. 같은 것이 이미 놓인 곳에 놓인 객체는 존재하기를 그만둘 수 있고, 래퍼는 살아남은 것으로 바로잡힌다.
 */
/* Place an object on the map at (x,y).
   local o = obj.new("rock");
   o:placeobj(u.ux, u.uy); */
staticfn int
l_obj_placeobj(lua_State *L)
{
    int argc = lua_gettop(L);
    struct _lua_obj *lo = l_obj_check(L, 1);
    coordxy x, y;

    if (argc != 3)
        nhl_error(L, "l_obj_placeobj: Wrong args");

    x = (coordxy) luaL_checkinteger(L, 2);
    y = (coordxy) luaL_checkinteger(L, 3);
    cvt_to_abscoord(&x, &y);

    lua_pop(L, 3);

    if (lobj_is_ok(lo)) {
        obj_extract_self(lo->obj);
        place_object(lo->obj, x, y);
        newsym(x, y);
    }

    return 0;
}

RESTORE_WARNING_UNREACHABLE_CODE

/**
 * @brief Step along a chain of objects, or start at the beginning of the level's.
 *
 * @param L the script, with nothing, an object, or an object and a flag on its stack
 * @return one wrapper
 * @warning Two different chains, chosen by the flag, and they are not interchangeable. One walks the objects on the same square; the other walks every object on the level. A script wanting what is under its feet and taking the
 *          second will visit the whole level and appear to work.
 * @note Called with nothing it starts at the level's first object, which is how a script iterates without being handed a starting point.
 */
/**
 * @brief 객체 사슬을 따라 나아가거나, 레벨의 사슬 처음에서 시작한다.
 *
 * @param L 스택에 아무것도, 객체 하나, 또는 객체와 플래그가 놓인 그 스크립트
 * @return 래퍼 하나
 * @warning 플래그로 골라지는 서로 다른 두 사슬이며, 그것들은 바꿔 쓸 수 없다. 하나는 같은 칸의 객체들을 돌고, 다른 하나는 레벨의 모든 객체를 돈다. 발밑에 있는 것을 원하며 두 번째를 택한 스크립트는 레벨 전체를 방문하면서 작동하는 것처럼 보이게 된다.
 * @note 아무것도 없이 호출되면 레벨의 첫 객체에서 시작한다. 그것이 스크립트가 시작점을 건네받지 않고 순회하는 방식이다.
 */
/* Get the next object in the object chain */
/* local o = obj.at(x, y);
   local o2 = o:next(true);
   local firstobj = obj.next();
*/
staticfn int
l_obj_nextobj(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 0) {
        (void) l_obj_push(L, fobj);
    } else {
        struct _lua_obj *lo = l_obj_check(L, 1);
        boolean use_nexthere = FALSE;

        if (argc == 2)
            use_nexthere = lua_toboolean(L, 2);

        if (lo && lo->obj)
            (void) l_obj_push(L, (use_nexthere && lo->obj->where == OBJ_FLOOR)
                                  ? lo->obj->nexthere
                                  : lo->obj->nobj);
    }
    return 1;
}

/* Get the container object is in */
/* local box = o:container(); */
staticfn int
l_obj_container(lua_State *L)
{
    struct _lua_obj *lo = l_obj_check(L, 1);

    if (lo && lo->obj && lo->obj->where == OBJ_CONTAINED)
        (void) l_obj_push(L, lo->obj->ocontainer);
    else
        (void) l_obj_push(L, NULL);
    return 1;
}

/**
 * @brief Whether a wrapper is one a script may still use.
 *
 * The only safe way for a script to find out. A wrapper may refer to nothing, or to an object the game has taken over, or to one that has been freed -- and from the script's side those are indistinguishable without asking.
 *
 * @param L the script, with the wrapper on its stack
 * @return true if the wrapper is not usable
 * @warning The sense is inverted relative to the name a reader might expect: it answers "is this unusable", so a script treating a true answer as "this is fine" has it backwards.
 */
/**
 * @brief 래퍼가 스크립트가 여전히 쓸 수 있는 것인지.
 *
 * 스크립트가 그것을 알아낼 유일하게 안전한 방법이다. 래퍼는 아무것도 가리키지 않을 수도, 게임이 넘겨받은 객체를 가리킬 수도, 해제된 객체를 가리킬 수도 있다. 그리고 스크립트 쪽에서는 묻지 않고 그것들을 구별할 수 없다.
 *
 * @param L 스택에 그 래퍼가 놓인 그 스크립트
 * @return 그 래퍼를 쓸 수 없으면 참
 * @warning 그 의미가 독자가 기대할 이름과 반대다. "이것을 쓸 수 없는가"에 답하므로, 참이라는 답을 "이것은 괜찮다"로 취급하는 스크립트는 그것을 거꾸로 쥔 것이다.
 */
/* Is the object a null? */
/* local badobj = o:isnull(); */
staticfn int
l_obj_isnull(lua_State *L)
{
    struct _lua_obj *lo = l_obj_check(L, 1);

    lua_pushboolean(L, !lobj_is_ok(lo));
    return 1;
}

DISABLE_WARNING_UNREACHABLE_CODE

/**
 * @name Object timers, from a script
 * @brief Ask about, look at, stop and start the timers attached to an object.
 *
 * A script needs these because a level may want to arrange something to happen later -- an egg that hatches on a schedule, a corpse that does not rot. Giving a script control over timers is how a level describes an event
 * rather than a state.
 *
 * @note Every one of them checks that the named timer is a kind that belongs to an object at all. A script naming a timer that belongs to a square would otherwise attach it to an object, and the timer would later fire on
 *       something of the wrong type.
 * @note Asking about a timer on an object that does not exist answers "no" rather than failing, so a script may ask without checking first.
 * @{
 */
/**
 * @name 스크립트에서의 객체 타이머
 * @brief 객체에 붙은 타이머를 묻고, 들여다보고, 멈추고, 시작한다.
 *
 * 스크립트가 이것들을 필요로 하는 것은, 레벨이 나중에 무언가가 일어나도록 마련하고 싶어 할 수 있기 때문이다. 정해진 때에 부화하는 알, 썩지 않는 시체. 스크립트에게 타이머의 통제를 주는 것이 레벨이 상태가 아니라 사건을 기술하는 방식이다.
 *
 * @note 이들 하나하나가 지칭된 타이머가 아예 객체에 속하는 종류인지 검사한다. 그러지 않으면 칸에 속한 타이머를 지칭한 스크립트가 그것을 객체에 붙이게 되고, 그 타이머가 나중에 잘못된 타입의 것에 대해 발동하게 된다.
 * @note 존재하지 않는 객체의 타이머에 대해 묻는 것은 실패하는 대신 "없음"으로 답한다. 그래서 스크립트가 먼저 확인하지 않고 물을 수 있다.
 * @{
 */
/* does object have a timer of certain type? */
/* local hastimer = o:has_timer("rot-organic"); */
staticfn int
l_obj_timer_has(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 2) {
        struct _lua_obj *lo = l_obj_check(L, 1);
        short timertype = nhl_get_timertype(L, 2);

        if (timer_is_obj(timertype) && lo && lo->obj) {
            lua_pushboolean(L, obj_has_timer(lo->obj, timertype));
            return 1;
        } else {
            lua_pushboolean(L, FALSE);
            return 1;
        }
    } else
        nhl_error(L, "l_obj_timer_has: Wrong args");
    return 0;
}

/* peek at an object timer. return the turn when timer triggers.
   returns 0 if no such timer attached to the object. */
/* local timeout = o:peek_timer("hatch-egg"); */
staticfn int
l_obj_timer_peek(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 2) {
        struct _lua_obj *lo = l_obj_check(L, 1);
        short timertype = nhl_get_timertype(L, 2);

        if (timer_is_obj(timertype) && lo && lo->obj) {
            lua_pushinteger(L, peek_timer(timertype, obj_to_any(lo->obj)));
            return 1;
        } else {
            lua_pushinteger(L, 0);
            return 1;
        }
    } else
        nhl_error(L, "l_obj_timer_peek: Wrong args");
    /*NOTREACHED*/
    return 0;
}

/* stop object timer(s). return the turn when timer triggers.
   returns 0 if no such timer attached to the object.
   without a timer type parameter, stops all timers for the object. */
/* local timeout = o:stop_timer("rot-organic"); */
/* o:stop_timer(); */
staticfn int
l_obj_timer_stop(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 1) {
        struct _lua_obj *lo = l_obj_check(L, 1);

        if (lo && lo->obj)
            obj_stop_timers(lo->obj);
        return 0;

    } else if (argc == 2) {
        struct _lua_obj *lo = l_obj_check(L, 1);
        short timertype = nhl_get_timertype(L, 2);

        if (timer_is_obj(timertype) && lo && lo->obj) {
            lua_pushinteger(L, stop_timer(timertype, obj_to_any(lo->obj)));
            return 1;
        } else {
            lua_pushinteger(L, 0);
            return 1;
        }
    } else
        nhl_error(L, "l_obj_timer_stop: Wrong args");
    return 0;
}

RESTORE_WARNING_UNREACHABLE_CODE

/* start an object timer. */
/* o:start_timer("hatch-egg", 10); */
staticfn int
l_obj_timer_start(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 3) {
        struct _lua_obj *lo = l_obj_check(L, 1);
        short timertype = nhl_get_timertype(L, 2);
        long when = luaL_checkinteger(L, 3);

        if (timer_is_obj(timertype) && lo && lo->obj && when > 0) {
            if (obj_has_timer(lo->obj, timertype))
                stop_timer(timertype, obj_to_any(lo->obj));
            start_timer(when, TIMER_OBJECT, timertype, obj_to_any(lo->obj));
        }
    } else
        nhl_error(L, "l_obj_timer_start: Wrong args");
    return 0;
}

/** @} */

/**
 * @brief Bury an object, and say whether it survived being buried.
 *
 * @param L the script, with the object and optionally coordinates on its stack
 * @return true if the object is gone
 * @warning The return value is not success but destruction, as the existing comment records: some objects merge into the ground when buried and cease to exist. A script that buries something and then uses its wrapper must
 *          check this, because the wrapper refers to freed memory once the answer is true.
 * @note Coordinates are optional -- without them the object is buried where it already is, which is the usual case for something the script has just placed.
 */
/**
 * @brief 객체를 묻고, 그것이 묻히는 것을 견뎠는지 말한다.
 *
 * @param L 스택에 객체와, 선택적으로 좌표가 놓인 그 스크립트
 * @return 그 객체가 사라졌으면 참
 * @warning 기존 주석이 기록하듯 반환값은 성공이 아니라 파괴다. 어떤 객체는 묻힐 때 땅에 합쳐져 존재하기를 그만둔다. 무언가를 묻고 그 래퍼를 쓰는 스크립트는 이것을 확인해야 한다. 답이 참인 순간 그 래퍼는 해제된 메모리를 가리킨다.
 * @note 좌표는 선택적이다. 그것이 없으면 객체는 이미 있는 곳에 묻히며, 그것이 스크립트가 방금 놓은 것에 대한 통상적인 경우다.
 */
/* bury an obj. returns true if object is gone (merged with ground),
   false otherwise. */
/* local ogone = o:bury(); */
/* local ogone = o:bury(5,5); */
staticfn int
l_obj_bury(lua_State *L)
{
    int argc = lua_gettop(L);
    boolean dealloced = FALSE;
    struct _lua_obj *lo = l_obj_check(L, 1);
    coordxy x = 0, y = 0;

    if (argc == 1) {
        x = lo->obj->ox;
        y = lo->obj->oy;
    } else if (argc == 3) {
        x = (coordxy) lua_tointeger(L, 2);
        y = (coordxy) lua_tointeger(L, 3);
        cvt_to_abscoord(&x, &y);
    } else
        nhl_error(L, "l_obj_bury: Wrong args");

    if (lobj_is_ok(lo) && isok(x, y)) {
        lo->obj->ox = x;
        lo->obj->oy = y;
        (void) bury_an_obj(lo->obj, &dealloced);
    }
    lua_pushboolean(L, dealloced);
    return 1;
}

static const struct luaL_Reg l_obj_methods[] = {
    { "new", l_obj_new_readobjnam },
    { "isnull", l_obj_isnull },
    { "at", l_obj_at },
    { "next", l_obj_nextobj },
    { "totable", l_obj_to_table },
    { "class", l_obj_objects_to_table },
    { "placeobj", l_obj_placeobj },
    { "container", l_obj_container },
    { "contents", l_obj_getcontents },
    { "addcontent", l_obj_add_to_container },
    { "has_timer", l_obj_timer_has },
    { "peek_timer", l_obj_timer_peek },
    { "stop_timer", l_obj_timer_stop },
    { "start_timer", l_obj_timer_start },
    { "bury", l_obj_bury },
    { NULL, NULL }
};

static const luaL_Reg l_obj_meta[] = {
    { "__gc", l_obj_gc },
    { NULL, NULL }
};

/**
 * @brief Make the object interface available to scripts.
 *
 * Installs the method table and the metatable that binds the collector to it. That binding is what makes ownership work at all: without a collector attached, a script discarding a wrapper would leave the object's count of
 * wrappers permanently too high and the object would never be freed.
 *
 * @param L the script
 * @return the number of values left on the stack, as the script's module loading expects
 * @note Both instance methods and plain ones live in one table, as the accompanying comment notes. A script may write either form and the same routine serves.
 */
/**
 * @brief 객체 인터페이스를 스크립트가 쓸 수 있게 만든다.
 *
 * 메서드 표와, 수집기를 그것에 묶는 메타테이블을 설치한다. 그 묶음이 소유권을 아예 작동하게 하는 것이다. 수집기가 붙어 있지 않으면, 래퍼를 버린 스크립트가 그 객체의 래퍼 개수를 영구히 너무 높게 남기고 그 객체는 결코 해제되지 않는다.
 *
 * @param L 그 스크립트
 * @return 스크립트의 모듈 적재가 기대하는 대로, 스택에 남은 값의 개수
 * @note 딸린 주석이 밝히듯 인스턴스 메서드와 평범한 메서드가 하나의 표에 함께 산다. 스크립트는 어느 형태로든 적을 수 있고 같은 루틴이 그것을 맡는다.
 */
int
l_obj_register(lua_State *L)
{
    /* Table of instance methods (e.g. an_object:isnull())
       and static methods (e.g. obj.new("dagger")). */
    luaL_newlib(L, l_obj_methods);

    /* metatable = { __name = "obj", __gc = l_obj_gc } */
    luaL_newmetatable(L, "obj");
    luaL_setfuncs(L, l_obj_meta, 0);
    /* metatable.__index points at the object method table. */
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "__index");

    /* Don't let lua code mess with the real metatable.
       Instead offer a fake one that only contains __gc. */
    luaL_newlib(L, l_obj_meta);
    lua_setfield(L, -2, "__metatable");

    /* We don't need the metatable anymore. It's safe in the
       Lua registry for use by luaL_setmetatable. */
    lua_pop(L, 1);

    /* global obj = the method table we created at the start */
    lua_setglobal(L, "obj");
    return 0;
}

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef lobj_is_ok

/*nhlobj.c*/
