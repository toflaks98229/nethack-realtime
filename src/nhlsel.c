/* NetHack 5.0	nhlua.c	$NHDT-Date: 1781973058 2026/06/20 16:30:58 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.71 $ */
/*      Copyright (c) 2018 by Pasi Kallinen */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file nhlsel.c
 * @brief Sets of map squares, as level scripts manipulate them.
 *
 * A level script rarely wants one square; it wants "the floor of this room",
 * "a line from here to there", "everywhere that is not wall". A selection is
 * that idea made concrete -- a set of squares that can be built up, combined,
 * and then used to place terrain, objects or monsters all at once.
 *
 * Selections combine like sets, so a script can describe a region by what it
 * is not, or by the overlap of two shapes, instead of enumerating squares.
 *
 * @note The set is stored as one bit per square of the map, so a selection
 *       costs the same whether it holds one square or every square.
 */

/**
 * @file nhlsel.c
 * @brief 맵 칸들의 집합. 레벨 스크립트가 다루는 방식.
 *
 * 레벨 스크립트가 칸 하나만 원하는 경우는 드물다. "이 방의 바닥", "여기서 저기까지의
 * 선", "벽이 아닌 모든 곳"을 원한다. 선택(selection)은 그 개념을 구체화한 것으로,
 * 쌓아 올리고 결합한 뒤 지형·객체·몬스터를 한꺼번에 배치하는 데 쓸 수 있는 칸들의
 * 집합이다.
 *
 * 선택은 집합처럼 결합되므로, 스크립트는 칸을 하나하나 나열하는 대신 "무엇이
 * 아닌지"로, 또는 두 도형이 겹치는 부분으로 영역을 기술할 수 있다.
 *
 * @note 집합은 맵의 칸마다 1비트로 저장된다. 그래서 선택의 비용은 그것이 한 칸을
 *       담든 모든 칸을 담든 같다.
 */

#include "hack.h"
#include "sp_lev.h"


struct selectionvar *l_selection_check(lua_State *, int);
staticfn struct selectionvar *l_selection_push_new(lua_State *);

/* lua_CFunction prototypes */
staticfn int l_selection_new(lua_State *);
staticfn int l_selection_clone(lua_State *);
staticfn int l_selection_numpoints(lua_State *);
staticfn int l_selection_getpoint(lua_State *);
staticfn int l_selection_setpoint(lua_State *);
staticfn int l_selection_filter_percent(lua_State *);
staticfn int l_selection_rndcoord(lua_State *);
staticfn int l_selection_room(lua_State *);
staticfn int l_selection_getbounds(lua_State *);
staticfn boolean params_sel_2coords(lua_State *, struct selectionvar **,
                                  coordxy *, coordxy *, coordxy *, coordxy *);
staticfn int l_selection_line(lua_State *);
staticfn int l_selection_randline(lua_State *);
staticfn int l_selection_rect(lua_State *);
staticfn int l_selection_fillrect(lua_State *);
staticfn int l_selection_grow(lua_State *);
staticfn int l_selection_filter_mapchar(lua_State *);
staticfn int l_selection_match(lua_State *);
staticfn int l_selection_flood(lua_State *);
staticfn int l_selection_circle(lua_State *);
staticfn int l_selection_ellipse(lua_State *);
staticfn int l_selection_gradient(lua_State *);
staticfn int l_selection_iterate(lua_State *);
staticfn int l_selection_size_description(lua_State *L);
staticfn int l_selection_gc(lua_State *);
staticfn int l_selection_not(lua_State *);
staticfn int l_selection_and(lua_State *);
staticfn int l_selection_or(lua_State *);
staticfn int l_selection_xor(lua_State *);
/* There doesn't seem to be a point in having a l_selection_add since it would
 * do the same thing as l_selection_or. The addition operator is mapped to
 * l_selection_or. */
staticfn int l_selection_sub(lua_State *);
/**
 * @name Unused declarations
 *
 * Disabled rather than deleted, and the existing comment records why: they are unused, and being local to this file that makes one compiler complain about them.
 *
 * One of the two is never defined at all -- only declared -- and it is the one the operator table's disabled entry would need. So the two disabled pieces are related: making a selection directly iterable would require writing that function as
 * well as enabling the table entry.
 *
 * The other has a disabled body further down. It fetched a selection without confirming it was one, which is the difference between it and the checking function that replaced it -- and the reason it should stay disabled rather than being
 * revived as the cheaper alternative.
 */

/**
 * @name 쓰이지 않는 선언
 *
 * 지워지는 대신 비활성화되어 있으며, 기존 주석이 이유를 기록한다. 그것들은 쓰이지 않으며, 이 파일에 국지적이어서 그것이 한 컴파일러가 그것들에 대해 불평하게 만든다.
 *
 * 그 둘 중 하나는 아예 정의되지 않고 선언만 되어 있으며, 그것이 연산자 표의 비활성화된 항목이 필요로 할 것이다. 그래서 그 두 비활성화된 조각은 연관되어 있다. 선택을 곧바로 순회할 수 있게 만드는 것은 그 표 항목을 켜는 것뿐 아니라 그 함수를 쓰는 일까지 필요로 할 것이다.
 *
 * 다른 하나는 더 아래에 비활성화된 본문을 갖는다. 그것은 선택인지 확인하지 않고 선택을 가져왔으며, 그것이 그것과 그것을 대신한 검사 함수의 차이다. 그리고 그것이 그것이 더 값싼 대안으로 되살려지는 대신 비활성화된 채로 머물러야 하는 이유다.
 */

#if 0
/* the following do not appear to currently be
   used and because they are static, the OSX
   compiler is complaining about them. I've
   if ifdef'd out the prototype here and the
   function body below.
 */
staticfn int l_selection_ipairs(lua_State *);
staticfn struct selectionvar *l_selection_to(lua_State *, int);
#endif

/**
 * @name Selection lifetime and the script's view of it
 *
 * A selection handed to a script is two pieces of memory with different owners. The structure itself sits in memory the script's own collector manages, so a script can drop a selection and forget about it. The map of squares inside it does
 * not -- it is an ordinary allocation, and something must free it when the structure goes.
 *
 * That split is the reason for the collector hook and for the copying done whenever a selection is handed over: the structure may be copied freely, but the map it points at must never be shared between two structures, or the first to be
 * collected would free the map the other still points at.
 * @{
 */

/**
 * @brief Fetch the selection at a stack position, refusing anything that is not one.
 * @param L the script state
 * @param index where on the stack to look
 * @return the selection
 * @note Checks the type twice by two different means. The second also confirms the value carries the selection identity, which is what stops a script from passing some other kind of object that happens to be the same size.
 * @note Raises a script error rather than returning null when the value is wrong, so callers may use the result without checking. The error unwinds out of the calling function, which is why code after such a failure is unreachable.
 */
/**
 * @brief 스택 위치의 선택을 가져오며, 선택이 아닌 것은 거부한다.
 * @param L 스크립트 상태
 * @param index 스택의 어디를 볼지
 * @return 그 선택
 * @note 두 가지 다른 방법으로 형을 두 번 검사한다. 두 번째는 그 값이 선택 정체를 지니는지도 확인하며, 그것이 스크립트가 마침 같은 크기인 다른 종류의 객체를 넘기는 것을 막는 것이다.
 * @note 값이 틀렸을 때 널을 돌려주는 대신 스크립트 오류를 일으키므로, 호출자들이 검사 없이 결과를 쓸 수 있다. 그 오류는 호출하는 함수 밖으로 풀려 나가며, 그것이 그런 실패 뒤의 코드가 닿을 수 없는 이유다.
 */
struct selectionvar *
l_selection_check(lua_State *L, int index)
{
    struct selectionvar *sel;

    luaL_checktype(L, index, LUA_TUSERDATA);
    sel = (struct selectionvar *) luaL_checkudata(L, index, "selection");
    if (!sel)
        nhl_error(L, "Selection error");
    return sel;
}

/**
 * @brief Release a selection's map when the script drops the selection.
 * @param L the script state
 * @return no results
 * @note Frees only the map, not the structure. The structure belongs to the script's collector, which is what called this -- freeing it here would be freeing memory the collector is about to reuse.
 * @warning Without this the map would leak on every selection a script created and abandoned, which for a level script that builds shapes in a loop is once per iteration.
 */
/**
 * @brief 스크립트가 선택을 버릴 때 그 선택의 지도를 해제한다.
 * @param L 스크립트 상태
 * @return 결과 없음
 * @note 구조체가 아니라 지도만 해제한다. 구조체는 스크립트의 수집기에 속하며, 그것이 이것을 호출한 것이다. 여기서 그것을 해제하는 것은 수집기가 곧 다시 쓸 메모리를 해제하는 일일 것이다.
 * @warning 이것 없이는 스크립트가 만들고 버린 모든 선택에서 지도가 샐 것이며, 되돌기로 도형을 만드는 레벨 스크립트에는 반복마다 한 번이다.
 */
staticfn int
l_selection_gc(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);

    if (sel)
        selection_free(sel, FALSE);
    return 0;
}

#if 0
staticfn struct selectionvar *
l_selection_to(lua_State *L, int index)
{
    struct selectionvar *sel
                           = (struct selectionvar *) lua_touserdata(L, index);

    if (!sel)
        nhl_error(L, "Selection error");
    return sel;
}
#endif

/**
 * @brief Create an empty selection in the script's memory and leave it on the stack.
 * @param L the script state
 * @return the selection, which is also on the stack
 * @note Made in two steps: an ordinary selection is created, its contents are copied into memory the script owns, and the original is discarded. The detour exists because the script's allocator cannot be handed to the routine that creates
 *       selections, and doing it this way means selection creation has only one implementation.
 * @note The map is copied rather than handed over, and the original's map is then freed with it. The two structures must never point at one map, since each would free it.
 * @note Attaches the selection identity before filling anything in, so a failure part way through still leaves a value the collector will treat correctly.
 */
/**
 * @brief 스크립트의 메모리에 빈 선택을 만들고 그것을 스택에 남긴다.
 * @param L 스크립트 상태
 * @return 그 선택. 스택에도 있다
 * @note 두 단계로 만들어진다. 보통의 선택이 만들어지고, 그 내용이 스크립트가 소유하는 메모리로 복사되고, 원본이 버려진다. 그 우회가 존재하는 것은 스크립트의 할당기가 선택을 만드는 함수에 넘겨질 수 없기 때문이며, 이렇게 하는 것은 선택 만들기가 하나의 구현만 갖는다는 뜻이다.
 * @note 지도는 넘겨지는 대신 복사되고, 그다음 원본의 지도가 그것과 함께 해제된다. 두 구조체가 결코 한 지도를 가리켜서는 안 된다. 각각이 그것을 해제할 것이기 때문이다.
 * @note 무엇을 채우기 전에 선택 정체를 붙이므로, 도중의 실패도 수집기가 옳게 다룰 값을 남긴다.
 */
staticfn struct selectionvar *
l_selection_push_new(lua_State *L)
{
    struct selectionvar *tmp = selection_new();
    struct selectionvar *sel
   = (struct selectionvar *) lua_newuserdata(L, sizeof (struct selectionvar));

    luaL_getmetatable(L, "selection");
    lua_setmetatable(L, -2);

    *sel = *tmp;
    sel->map = dupstr(tmp->map);
    selection_free(tmp, TRUE);

    return sel;
}

/**
 * @brief Hand a copy of an existing selection to the script.
 *
 * How the game gives a script a selection it built itself. A copy rather than the original, because the script's collector will eventually free what it is given and the game's own selection must survive that.
 *
 * @param L the script state
 * @param tmp the selection to copy
 * @note The map is duplicated, so the script's copy and the game's original are independent from this point. Changes a script makes do not reach back.
 * @warning Does not free the original. The caller keeps it and must dispose of it as it normally would -- this is a copy out, not a transfer.
 */
/**
 * @brief 이미 있는 선택의 사본을 스크립트에게 준다.
 *
 * 게임이 스스로 만든 선택을 스크립트에게 주는 방식이다. 원본이 아니라 사본인 것은, 스크립트의 수집기가 결국 자기가 받은 것을 해제할 것이고 게임 자신의 선택은 그것을 넘어 살아남아야 하기 때문이다.
 *
 * @param L 스크립트 상태
 * @param tmp 복사할 선택
 * @note 지도가 복제되므로, 스크립트의 사본과 게임의 원본은 이 시점부터 독립적이다. 스크립트가 하는 변경은 되돌아 닿지 않는다.
 * @warning 원본을 해제하지 않는다. 호출자가 그것을 지니고 보통 하듯 처리해야 한다. 이것은 넘김이 아니라 내보내는 복사다.
 */
void
l_selection_push_copy(lua_State *L, struct selectionvar *tmp)
{
    struct selectionvar *sel
   = (struct selectionvar *) lua_newuserdata(L, sizeof (struct selectionvar));

    luaL_getmetatable(L, "selection");
    lua_setmetatable(L, -2);

    *sel = *tmp;
    sel->map = dupstr(tmp->map);
}


/** @} */

/**
 * @brief The script-visible constructor for an empty selection.
 * @param L the script state
 * @return one result, the new selection
 * @note Holds nothing. A script builds a selection up from empty rather than starting with everything, which is why describing a region by what it is not takes an explicit inversion.
 */
/**
 * @brief 빈 선택을 위한, 스크립트에 보이는 생성자.
 * @param L 스크립트 상태
 * @return 결과 하나. 새 선택
 * @note 아무것도 담지 않는다. 스크립트는 모든 것으로 시작하는 대신 빈 것에서 선택을 쌓아 올리며, 그것이 어떤 영역을 그것이 아닌 것으로 기술하는 데 명시적인 뒤집기가 드는 이유다.
 */
staticfn int
l_selection_new(lua_State *L)
{
    (void) l_selection_push_new(L);
    return 1;
}

/**
 * @brief Return an independent copy of a selection.
 *
 * Needed because the combining operations modify what they are given. A script that wants to keep a shape and also grow it must clone first; without that, growing the shape would destroy the original.
 *
 * @param L the script state
 * @return one result, the copy
 * @note Creates an empty selection and overwrites it, which means the map the constructor just allocated is freed immediately. That is wasteful but keeps the allocation in one place.
 */
/**
 * @brief 선택의 독립적인 사본을 돌려준다.
 *
 * 결합 연산이 자기가 받은 것을 수정하기 때문에 필요하다. 어떤 도형을 지니면서 그것을 키우고도 싶은 스크립트는 먼저 복제해야 한다. 그것 없이 도형을 키우는 것은 원본을 파괴할 것이다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 그 사본
 * @note 빈 선택을 만들고 그것을 덮어쓰는데, 그것은 생성자가 방금 할당한 지도가 곧바로 해제된다는 뜻이다. 그것은 낭비지만 할당을 한 곳에 유지한다.
 */
staticfn int
l_selection_clone(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);
    struct selectionvar *tmp;

    (void) l_selection_new(L);
    tmp = l_selection_check(L, 2);
    if (tmp->map)
        free(tmp->map);
    *tmp = *sel;
    tmp->map = dupstr(sel->map);
    return 1;
}

DISABLE_WARNING_UNREACHABLE_CODE

/**
 * @brief Add a square to a selection, or clear one from it.
 *
 * Heavily overloaded, as the usage lines above record: the selection, the coordinates and the value are each optional, and what is meant is worked out from how many arguments arrived. Called with none it simply makes an empty selection; with
 * two it makes one containing that square.
 *
 * @param L the script state
 * @return one result, the selection
 * @note Coordinates are resolved through the level-script coordinate machinery rather than used directly, so a script inside a room description may give coordinates relative to that room and they mean what the script author intends.
 * @note Coordinates of minus one mean "anywhere", chosen at random by that same machinery. So a script can add a random square without knowing what squares exist.
 * @note The fourth argument being zero clears the square instead of setting it, which is how a script removes a square from a shape it has built.
 * @note As the existing note records, setting several squares in one call is wanted but not implemented.
 */
/**
 * @brief 선택에 칸을 더하거나 거기서 하나를 지운다.
 *
 * 위의 사용법 줄들이 기록하듯 무겁게 겹쳐 정의되어 있다. 선택, 좌표, 값이 각각 선택적이며, 무엇이 뜻해졌는지는 인자가 몇 개 도착했는지로 알아내어진다. 아무것 없이 호출되면 그저 빈 선택을 만들고, 둘과 함께면 그 칸을 담은 것을 만든다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 그 선택
 * @note 좌표는 곧바로 쓰이는 대신 레벨 스크립트 좌표 기계를 통해 풀리므로, 방 서술 안의 스크립트가 그 방에 상대적인 좌표를 줄 수 있고 그것이 스크립트 작성자가 뜻한 것을 뜻한다.
 * @note 마이너스 일인 좌표는 "아무 곳"을 뜻하며, 그 같은 기계가 무작위로 고른다. 그래서 스크립트가 어떤 칸이 존재하는지 모르고도 무작위 칸을 더할 수 있다.
 * @note 네 번째 인자가 영인 것은 칸을 설정하는 대신 지운다. 그것이 스크립트가 자기가 만든 도형에서 칸을 없애는 방법이다.
 * @note 기존 적바림이 기록하듯, 한 번의 호출로 여러 칸을 설정하는 것이 바라지지만 구현되지 않았다.
 */
staticfn int
l_selection_setpoint(lua_State *L)
{
    struct selectionvar *sel = (struct selectionvar *) 0;
    coordxy x = -1, y = -1;
    int val = 1;
    int argc = lua_gettop(L);
    long crd = 0L;

    if (argc == 0) {
        (void) l_selection_new(L);
    } else if (argc == 1) {
        sel = l_selection_check(L, 1);
    } else if (argc == 2) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        lua_pop(L, 2);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
    } else {
        sel = l_selection_check(L, 1);
        x = (coordxy) luaL_checkinteger(L, 2);
        y = (coordxy) luaL_checkinteger(L, 3);
        val = (int) luaL_optinteger(L, 4, 1);
    }

    if (!sel || !sel->map) {
        nhl_error(L, "Selection setpoint error");
        /*NOTREACHED*/
        return 0;
    }

    if (x == -1 && y == -1)
        crd = SP_COORD_PACK_RANDOM(0);
    else
        crd = SP_COORD_PACK(x,y);
    get_location_coord(&x, &y, ANY_LOC,
                       gc.coder ? gc.coder->croom : NULL, crd);
    selection_setpoint(x, y, sel, val);
    lua_settop(L, 1);
    return 1;
}

/**
 * @brief Count the squares in a selection.
 * @param L the script state
 * @return one result, the count
 * @note Counted by walking the squares rather than kept as a running total, since a selection is stored as bits and nothing maintains a count.
 * @note Walks only the recorded bounds, not the whole map. That is what keeps counting a small shape cheap, and it relies on the bounds being kept correct as squares are added.
 */
/**
 * @brief 선택 안의 칸을 센다.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 개수
 * @note 진행 중인 합계로 지녀지는 대신 칸을 걸어가서 세어진다. 선택이 비트로 저장되고 개수를 유지하는 것이 없기 때문이다.
 * @note 맵 전체가 아니라 기록된 경계만 걸어간다. 그것이 작은 도형을 세는 것을 값싸게 유지하는 것이며, 칸이 더해지며 그 경계가 옳게 유지되는 것에 의존한다.
 */
staticfn int
l_selection_numpoints(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);
    coordxy x, y;
    int ret = 0;
    NhRect rect = cg.zeroNhRect;

    selection_getbounds(sel, &rect);

    for (x = rect.lx; x <= rect.hx; x++)
        for (y = rect.ly; y <= rect.hy; y++)
            if (selection_getpoint(x, y, sel))
                ret++;

    lua_settop(L, 0);
    lua_pushinteger(L, ret);
    return 1;
}

/**
 * @brief Ask whether a square is in a selection.
 * @param L the script state
 * @return one result, non-zero if the square is in the selection
 * @note Accepts the coordinates either as two numbers or as a table, through the shared coordinate-reading helper, so a script may pass a position it obtained from elsewhere without unpacking it.
 * @note Resolves the coordinates the same way setting one does, including the random case -- so asking about square minus one, minus one asks about a random square rather than being an error.
 */
/**
 * @brief 어떤 칸이 선택 안에 있는지 묻는다.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 칸이 선택 안에 있으면 영이 아닌 값
 * @note 공유된 좌표 읽기 도우미를 통해 좌표를 두 숫자로든 표로든 받아들이므로, 스크립트가 다른 곳에서 얻은 위치를 풀지 않고 넘길 수 있다.
 * @note 설정하기가 하는 것과 같은 방식으로 좌표를 풀며, 무작위 경우도 포함한다. 그래서 마이너스 일, 마이너스 일 칸에 대해 묻는 것은 오류가 되는 대신 무작위 칸에 대해 묻는 것이다.
 */
staticfn int
l_selection_getpoint(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);
    coordxy x, y;
    lua_Integer ix, iy;
    int val;
    long crd;

    lua_remove(L, 1); /* sel */
    if (!nhl_get_xy_params(L, &ix, &iy)) {
        nhl_error(L, "l_selection_getpoint: Incorrect params");
        /*NOTREACHED*/
        return 0;
    }
    x = (coordxy) ix;
    y = (coordxy) iy;

    if (x == -1 && y == -1)
        crd = SP_COORD_PACK_RANDOM(0);
    else
        crd = SP_COORD_PACK(x,y);
    get_location_coord(&x, &y, ANY_LOC,
                       gc.coder ? gc.coder->croom : NULL, crd);

    val = selection_getpoint(x, y, sel);
    lua_settop(L, 0);
    lua_pushnumber(L, val);
    return 1;
}

RESTORE_WARNING_UNREACHABLE_CODE

/**
 * @name Set operations on selections
 *
 * These are what make selections worth having: a script describes a region as the overlap, union or difference of shapes rather than by listing squares.
 *
 * Each returns a new selection and leaves its arguments unchanged, which is why a script can combine the same shape twice. The arguments are removed from the stack afterwards so only the result remains.
 *
 * @note Each walks only the combined bounds of its arguments rather than the whole map, so combining two small shapes stays cheap regardless of map size.
 * @{
 */

/**
 * @brief Invert a selection: every square not in it, and none that is.
 *
 * The reason a script can say "everywhere that is not wall". Called with no argument at all it yields a selection holding every square, which is the inversion of nothing -- so this doubles as the way to start from everything.
 *
 * @param L the script state
 * @return one result, the inverted selection
 * @note Clones before inverting, so the original is left alone. Without the clone a script that inverted a shape would find the shape gone.
 * @warning Inverting is over the whole map, not over the selection's bounds. The result holds every square outside the original shape, including everything beyond its edges, so the bounds of an inverted selection are the map's.
 */
/**
 * @brief 선택을 뒤집는다. 그 안에 없는 모든 칸, 그리고 있는 칸은 하나도 없이.
 *
 * 스크립트가 "벽이 아닌 모든 곳"이라고 말할 수 있는 이유다. 인자 없이 호출되면 모든 칸을 담은 선택을 내는데, 그것은 아무것도 아닌 것의 뒤집기다. 그래서 이것은 모든 것에서 시작하는 방법도 겸한다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 뒤집힌 선택
 * @note 뒤집기 전에 복제하므로 원본이 그대로 남는다. 그 복제 없이 도형을 뒤집은 스크립트는 그 도형이 사라진 것을 발견할 것이다.
 * @warning 뒤집기는 선택의 경계가 아니라 맵 전체에 걸쳐 이루어진다. 결과는 원래 도형 밖의 모든 칸을 담으며, 그 가장자리를 넘어선 모든 것을 포함한다. 그래서 뒤집힌 선택의 경계는 맵의 경계다.
 */
staticfn int
l_selection_not(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel, *sel2;

    if (argc == 0) {
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
        selection_clear(sel, 1);
    } else {
        (void) l_selection_check(L, 1);
        (void) l_selection_clone(L);
        sel2 = l_selection_check(L, 2);
        selection_not(sel2);
        lua_remove(L, 1);
    }
    return 1;
}

/* local sel = selection.area(4,5, 40,10) & selection.rect(7,8, 60,14); */
/**
 * @brief The squares in both selections.
 * @param L the script state
 * @return one result, the overlap
 * @note Bound to the ampersand operator, so a script writes the intersection rather than calling this by name.
 * @note Does not set the result's bounds explicitly, unlike its three companions. An intersection can only be smaller than either input, and adding each square updates the bounds as it goes, so the result's bounds are correct by
 *       construction.
 */
/**
 * @brief 두 선택 모두에 있는 칸들.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 겹침
 * @note 앤드 기호 연산자에 묶여 있으므로, 스크립트는 이것을 이름으로 호출하는 대신 교집합을 쓴다.
 * @note 세 짝과 달리 결과의 경계를 명시적으로 설정하지 않는다. 교집합은 어느 입력보다도 작을 수밖에 없고, 각 칸을 더하는 것이 그때그때 경계를 갱신하므로, 결과의 경계는 만들어짐으로써 옳다.
 */
staticfn int
l_selection_and(lua_State *L)
{
    int x, y;
    struct selectionvar *sela = l_selection_check(L, 1);
    struct selectionvar *selb = l_selection_check(L, 2);
    struct selectionvar *selr = l_selection_push_new(L);
    NhRect rect = cg.zeroNhRect;

    rect_bounds(sela->bounds, selb->bounds, &rect);

    for (x = rect.lx; x <= rect.hx; x++)
        for (y = rect.ly; y <= rect.hy; y++) {
            int val = (selection_getpoint(x, y, sela)
                       & selection_getpoint(x, y, selb));

            selection_setpoint(x, y, selr, val);
        }

    lua_remove(L, 1);
    lua_remove(L, 1);
    return 1;
}

/**
 * @brief The squares in either selection.
 * @param L the script state
 * @return one result, the union
 * @note Bound to the vertical bar operator.
 * @note Assigns the combined bounds directly rather than recalculating them. A union reaches exactly as far as its two inputs together, so the combined rectangle is already the right answer and walking the result to find it would be wasted
 *       work.
 */
/**
 * @brief 두 선택 중 어느 쪽에든 있는 칸들.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 합집합
 * @note 수직선 연산자에 묶여 있다.
 * @note 경계를 다시 계산하는 대신 합쳐진 경계를 곧바로 배정한다. 합집합은 자기 두 입력이 함께 이르는 만큼 정확히 이르므로, 합쳐진 직사각형이 이미 옳은 답이며 그것을 찾자고 결과를 걸어가는 것은 헛된 일이 될 것이다.
 */
staticfn int
l_selection_or(lua_State *L)
{
    int x,y;
    struct selectionvar *sela = l_selection_check(L, 1);
    struct selectionvar *selb = l_selection_check(L, 2);
    struct selectionvar *selr = l_selection_push_new(L);
    NhRect rect = cg.zeroNhRect;

    rect_bounds(sela->bounds, selb->bounds, &rect);

    for (x = rect.lx; x <= rect.hx; x++)
        for (y = rect.ly; y <= rect.hy; y++) {
            int val = (selection_getpoint(x, y, sela)
                       | selection_getpoint(x, y, selb));

            selection_setpoint(x, y, selr, val);
        }
    selr->bounds = rect;

    lua_remove(L, 1);
    lua_remove(L, 1);
    return 1;
}

/**
 * @brief The squares in one selection but not both.
 * @param L the script state
 * @return one result, the symmetric difference
 * @note Bound to the tilde operator.
 * @note Recalculates the result's bounds by walking it, and the existing comment gives the reason: where the two shapes coincide the result is empty, so it can be smaller or oddly shaped compared with either input. Neither input's bounds nor
 *       their combination describes it, so the bounds must be measured.
 */
/**
 * @brief 한 선택에는 있지만 둘 모두에는 없는 칸들.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 대칭 차집합
 * @note 물결표 연산자에 묶여 있다.
 * @note 결과를 걸어가며 그 경계를 다시 계산하며, 기존 주석이 그 이유를 준다. 두 도형이 겹치는 곳에서 결과는 비어 있으므로, 그것은 어느 입력보다도 작거나 이상한 모양일 수 있다. 어느 입력의 경계도 그것들의 합도 그것을 서술하지 않으므로, 경계는 재어져야 한다.
 */
staticfn int
l_selection_xor(lua_State *L)
{
    int x,y;
    struct selectionvar *sela = l_selection_check(L, 1);
    struct selectionvar *selb = l_selection_check(L, 2);
    struct selectionvar *selr = l_selection_push_new(L);
    NhRect rect = cg.zeroNhRect;

    rect_bounds(sela->bounds, selb->bounds, &rect);

    for (x = rect.lx; x <= rect.hx; x++)
        for (y = rect.ly; y <= rect.hy; y++) {
            int val = (selection_getpoint(x, y, sela)
                       ^ selection_getpoint(x, y, selb));

            selection_setpoint(x, y, selr, val);
        }
    /* this may have created a smaller or irregular selection with
     * bounds_dirty set to true - update its boundaries */
    selection_recalc_bounds(selr);

    lua_remove(L, 1);
    lua_remove(L, 1);
    return 1;
}

/**
 * @brief The squares in the first selection but not the second.
 *
 * How a script cuts a hole in a shape -- take the room, subtract its middle. As the existing comment records, the result holds what is in the first and not the second.
 *
 * @param L the script state
 * @return one result, the difference
 * @note Bound to the minus operator, and unlike the other three it is not symmetric: the order of the arguments decides which shape is cut from which.
 * @note Computed as the symmetric difference narrowed to the first shape rather than by testing directly, which comes to the same thing.
 * @note Recalculates the bounds for the same reason the symmetric difference does: cutting a hole can leave the shape smaller or irregular, as the existing comment notes.
 */
/**
 * @brief 첫 선택에는 있지만 두 번째에는 없는 칸들.
 *
 * 스크립트가 도형에 구멍을 내는 방법이다. 방을 취하고 그 가운데를 뺀다. 기존 주석이 기록하듯 결과는 첫 번째에 있고 두 번째에 없는 것을 담는다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 그 차집합
 * @note 마이너스 연산자에 묶여 있으며, 다른 셋과 달리 대칭이 아니다. 인자의 순서가 어느 도형이 어느 도형에서 잘려 나가는지를 정한다.
 * @note 곧바로 검사하는 대신 첫 도형으로 좁혀진 대칭 차집합으로 계산되며, 그것은 같은 것에 이른다.
 * @note 대칭 차집합이 그러는 것과 같은 이유로 경계를 다시 계산한다. 기존 주석이 적듯 구멍을 내는 것은 도형을 더 작거나 불규칙하게 남길 수 있다.
 */
staticfn int
l_selection_sub(lua_State *L)
{
    int x,y;
    struct selectionvar *sela = l_selection_check(L, 1);
    struct selectionvar *selb = l_selection_check(L, 2);
    struct selectionvar *selr = l_selection_push_new(L);
    NhRect rect = cg.zeroNhRect;

    rect_bounds(sela->bounds, selb->bounds, &rect);

    for (x = rect.lx; x <= rect.hx; x++)
        for (y = rect.ly; y <= rect.hy; y++) {
            coordxy a_pt = selection_getpoint(x, y, sela);
            coordxy b_pt = selection_getpoint(x, y, selb);
            int val = (a_pt ^ b_pt) & a_pt;
            selection_setpoint(x, y, selr, val);
        }
    /* this may have created a smaller or irregular selection with
     * bounds_dirty set to true - update its boundaries */
    selection_recalc_bounds(selr);

    lua_remove(L, 1);
    lua_remove(L, 1);
    return 1;
}

/** @} */

/**
 * @brief Keep a given percentage of a selection's squares, chosen at random.
 *
 * How a level script scatters something. A script wanting boulders across half a room takes the room and thins it, rather than picking positions one at a time.
 *
 * @param L the script state
 * @return one result, the thinned selection
 * @note The percentage is applied to each square independently, so the result holds about that fraction rather than exactly it. A script needing an exact count cannot use this.
 * @note The intermediate selection is copied into the script's memory and then freed, since the routine doing the work returns an ordinary selection which the script's collector cannot manage.
 */
/**
 * @brief 선택의 칸 가운데 주어진 백분율을 무작위로 골라 남긴다.
 *
 * 레벨 스크립트가 무언가를 흩뿌리는 방법이다. 방 절반에 걸친 바위를 원하는 스크립트는 위치를 하나하나 고르는 대신 방을 취하고 그것을 솎아낸다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 솎아진 선택
 * @note 백분율이 각 칸에 독립적으로 적용되므로, 결과는 그 비율을 정확히가 아니라 대략 담는다. 정확한 개수가 필요한 스크립트는 이것을 쓸 수 없다.
 * @note 중간 선택이 스크립트의 메모리로 복사되고 그다음 해제된다. 그 일을 하는 함수가 스크립트의 수집기가 관리할 수 없는 보통의 선택을 돌려주기 때문이다.
 */
staticfn int
l_selection_filter_percent(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = l_selection_check(L, 1);
    int p = (int) luaL_checkinteger(L, 2);
    struct selectionvar *tmp;

    tmp = selection_filter_percent(sel, p);
    lua_pop(L, argc);
    l_selection_push_copy(L, tmp);
    selection_free(tmp, TRUE);

    return 1;
}

/**
 * @brief Pick one square of a selection at random, optionally removing it.
 *
 * The usual way a script places a series of things without repeating a position: ask for a square, remove it, ask again.
 *
 * @param L the script state
 * @return one result, a table holding the coordinates
 * @note The removal is the second argument and is optional, so the same call serves both "where is somewhere in this shape" and "give me squares one at a time".
 * @note The coordinates are converted back to the frame the script uses -- relative to the current room, or to the map's origin outside one -- which is the reverse of what happens to coordinates coming in. Without that, a script would put
 *       something at a position it did not mean.
 * @note An empty selection yields minus one for both coordinates, which the script must recognise. It is not an error, because a shape thinned at random may legitimately end up empty.
 * @note Refreshes which room is current before converting, since a script may have moved on since the selection was made.
 */
/**
 * @brief 선택의 한 칸을 무작위로 고르고, 선택적으로 그것을 없앤다.
 *
 * 스크립트가 위치를 되풀이하지 않고 여러 가지를 놓는 보통의 방법이다. 칸을 요청하고, 그것을 없애고, 다시 요청한다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 좌표를 담은 표
 * @note 없애기가 두 번째 인자이며 선택적이므로, 같은 호출이 "이 도형 안 어딘가는 어디인가"와 "칸을 하나씩 달라" 모두를 섬긴다.
 * @note 좌표가 스크립트가 쓰는 틀로 되돌려 변환된다. 현재 방에 상대적으로, 또는 방 밖에서는 맵의 원점에 상대적으로. 그것은 들어오는 좌표에 일어나는 일의 반대다. 그것 없이는 스크립트가 자기가 뜻하지 않은 위치에 무언가를 놓을 것이다.
 * @note 빈 선택은 두 좌표 모두에 마이너스 일을 내며, 스크립트가 그것을 알아보아야 한다. 그것은 오류가 아니다. 무작위로 솎아진 도형이 정당하게 비게 될 수 있기 때문이다.
 * @note 변환하기 전에 어느 방이 현재인지를 새로 하는데, 선택이 만들어진 뒤로 스크립트가 옮겨 갔을 수 있기 때문이다.
 */
staticfn int
l_selection_rndcoord(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);
    int removeit = (int) luaL_optinteger(L, 2, 0);
    coordxy x = -1, y = -1;
    selection_rndcoord(sel, &x, &y, removeit);
    if (!(x == -1 && y == -1)) {
        update_croom();
        if (gc.coder && gc.coder->croom) {
            x -= gc.coder->croom->lx;
            y -= gc.coder->croom->ly;
        } else {
            x -= gx.xstart;
            y -= gy.ystart;
        }
    }
    lua_settop(L, 0);
    lua_newtable(L);
    nhl_add_table_entry_int(L, "x", x);
    nhl_add_table_entry_int(L, "y", y);
    return 1;
}

/**
 * @brief A selection holding a room's squares.
 *
 * The commonest starting point for a level script: describe a region by naming the room rather than its corners, so the description survives the room being placed somewhere else.
 *
 * @param L the script state
 * @return one result, the selection
 * @note With an argument it names a room by number; with none it means the room the script is currently describing. The second form is what makes a room description reusable.
 * @note An out-of-range room number is taken as no room rather than refused, which yields whatever the underlying routine makes of that. A script with a bad room number gets a selection rather than an error.
 */
/**
 * @brief 어떤 방의 칸들을 담은 선택.
 *
 * 레벨 스크립트의 가장 흔한 출발점이다. 영역을 그 모서리가 아니라 방을 이름 지어 기술하므로, 그 기술이 방이 다른 곳에 놓이는 것을 넘어 살아남는다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 그 선택
 * @note 인자와 함께면 번호로 방을 이름 짓고, 없으면 스크립트가 지금 기술하고 있는 방을 뜻한다. 두 번째 형태가 방 서술을 다시 쓸 수 있게 만드는 것이다.
 * @note 범위를 벗어난 방 번호는 거부되는 대신 방 없음으로 여겨지며, 그것이 밑의 함수가 그것으로 만드는 무엇이든을 낸다. 틀린 방 번호의 스크립트는 오류가 아니라 선택을 얻는다.
 */
staticfn int
l_selection_room(lua_State *L)
{
    struct selectionvar *sel;
    int argc = lua_gettop(L);
    struct mkroom *croom = NULL;

    if (argc == 1) {
        int i = luaL_checkinteger(L, -1);

        croom = (i >= 0 && i < svn.nroom) ? &svr.rooms[i] : NULL;
    }

    sel = selection_from_mkroom(croom);

    l_selection_push_copy(L, sel);
    selection_free(sel, TRUE);

    return 1;
}

/**
 * @brief The rectangle enclosing a selection.
 * @param L the script state
 * @return one result, a table of the four edges
 * @note Describes where the shape is, not what it is -- an irregular shape and the rectangle around it report the same bounds. A script that wants the shape must ask about squares.
 * @note These are the game's own coordinates, not converted to the script's frame as a random square would be. A script comparing them against coordinates it wrote itself will find them offset.
 */
/**
 * @brief 선택을 감싸는 직사각형.
 * @param L 스크립트 상태
 * @return 결과 하나. 네 변의 표
 * @note 도형이 무엇인지가 아니라 어디 있는지를 서술한다. 불규칙한 도형과 그것을 둘러싼 직사각형은 같은 경계를 보고한다. 도형을 원하는 스크립트는 칸에 대해 물어야 한다.
 * @note 이것들은 게임 자신의 좌표이며, 무작위 칸이 그러듯 스크립트의 틀로 변환되지 않는다. 그것을 자기가 쓴 좌표와 견주는 스크립트는 그것들이 어긋나 있는 것을 발견할 것이다.
 */
staticfn int
l_selection_getbounds(lua_State *L)
{
    struct selectionvar *sel = l_selection_check(L, 1);
    NhRect rect = cg.zeroNhRect;

    selection_getbounds(sel, &rect);
    lua_settop(L, 0);
    lua_newtable(L);
    nhl_add_table_entry_int(L, "lx", rect.lx);
    nhl_add_table_entry_int(L, "ly", rect.ly);
    nhl_add_table_entry_int(L, "hx", rect.hx);
    nhl_add_table_entry_int(L, "hy", rect.hy);
    return 1;
}

/**
 * @brief Read the arguments common to the two-corner shapes: a selection and four coordinates.
 *
 * The shapes that go between two points all accept the same two forms, as the existing comment records -- given a selection to add to, or given none and creating one. Reading that once is what keeps the four shapes from each getting the
 * overloading slightly wrong.
 *
 * @param L the script state
 * @param sel receives the selection
 * @param x1 receives the first column
 * @param y1 receives the first row
 * @param x2 receives the second column
 * @param y2 receives the second row
 * @return whether the arguments made sense
 * @note Leaves only the selection on the stack, having removed the coordinates, as the existing comment records. Callers rely on that: they clone what is left rather than tracking positions.
 * @note Any count other than four or five is refused. There is no partial form -- both corners are required -- so a script giving three coordinates gets an error rather than a guess.
 * @note Does not resolve the coordinates. Each caller does that itself, because resolution depends on the shape being drawn.
 */
/**
 * @brief 두 모서리 도형들에 공통된 인자를 읽는다. 선택과 네 좌표.
 *
 * 두 점 사이를 가는 도형들이 모두 같은 두 형태를 받아들이며, 기존 주석이 그것을 기록한다. 더할 선택이 주어지거나, 주어지지 않고 하나를 만드는 것. 그것을 한 번 읽는 것이 네 도형이 각각 그 겹쳐 정의를 조금씩 틀리게 하는 것을 막는 것이다.
 *
 * @param L 스크립트 상태
 * @param sel 선택을 받는다
 * @param x1 첫 열을 받는다
 * @param y1 첫 행을 받는다
 * @param x2 두 번째 열을 받는다
 * @param y2 두 번째 행을 받는다
 * @return 인자가 뜻이 통했는지
 * @note 기존 주석이 기록하듯 좌표를 없앤 뒤 스택에 선택만 남긴다. 호출자들이 그것에 의존한다. 그들은 위치를 좇는 대신 남은 것을 복제한다.
 * @note 넷이나 다섯이 아닌 어떤 개수도 거부된다. 부분적인 형태는 없다. 두 모서리 모두가 요구되므로, 좌표 셋을 주는 스크립트는 짐작이 아니라 오류를 얻는다.
 * @note 좌표를 풀지 않는다. 각 호출자가 스스로 그것을 하는데, 풀기가 그려지고 있는 도형에 달려 있기 때문이다.
 */
staticfn boolean
params_sel_2coords(lua_State *L, struct selectionvar **sel,
                   coordxy *x1, coordxy *y1, coordxy *x2, coordxy *y2)
{
    int argc = lua_gettop(L);

    if (argc == 4) {
        (void) l_selection_new(L);
        *x1 = (coordxy) luaL_checkinteger(L, 1);
        *y1 = (coordxy) luaL_checkinteger(L, 2);
        *x2 = (coordxy) luaL_checkinteger(L, 3);
        *y2 = (coordxy) luaL_checkinteger(L, 4);
        *sel = l_selection_check(L, 5);
        lua_remove(L, 1);
        lua_remove(L, 1);
        lua_remove(L, 1);
        lua_remove(L, 1);
        return TRUE;
    } else if (argc == 5) {
        *sel = l_selection_check(L, 1);
        *x1 = (coordxy) luaL_checkinteger(L, 2);
        *y1 = (coordxy) luaL_checkinteger(L, 3);
        *x2 = (coordxy) luaL_checkinteger(L, 4);
        *y2 = (coordxy) luaL_checkinteger(L, 5);
        lua_pop(L, 4);
        return TRUE;
    }
    return FALSE;
}

/**
 * @name Shapes drawn between two corners
 *
 * A straight line, a rectangle's outline, and a filled rectangle. All three take the same arguments and all three add to a clone rather than to what they were given, so the shape passed in survives.
 * @{
 */

/**
 * @brief Add a straight line between two squares.
 * @param L the script state
 * @return one result, the selection with the line added
 * @note Clones before drawing, so the selection the script passed in is unchanged. Every shape here does the same, which is what lets a script build several shapes from one starting selection.
 */
/**
 * @brief 두 칸 사이에 직선을 더한다.
 * @param L 스크립트 상태
 * @return 결과 하나. 선이 더해진 선택
 * @note 그리기 전에 복제하므로, 스크립트가 넘긴 선택은 바뀌지 않는다. 여기의 모든 도형이 같은 일을 하며, 그것이 스크립트가 하나의 출발 선택에서 여러 도형을 만들 수 있게 하는 것이다.
 */
staticfn int
l_selection_line(lua_State *L)
{
    struct selectionvar *sel = NULL;
    coordxy x1, y1, x2, y2;

    if (!params_sel_2coords(L, &sel, &x1, &y1, &x2, &y2)) {
        nhl_error(L, "selection.line: illegal arguments");
    }

    get_location_coord(&x1, &y1, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x1, y1));
    get_location_coord(&x2, &y2, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x2, y2));

    (void) l_selection_clone(L);
    sel = l_selection_check(L, 2);
    selection_do_line(x1,y1,x2,y2, sel);
    return 1;
}

/**
 * @brief Add a rectangle's outline -- its edges, not its interior.
 * @param L the script state
 * @return one result, the selection with the outline added
 * @note Drawn as four lines rather than by a rectangle routine, which is why the corners are covered twice. Setting a square already set is harmless, so nothing needs to avoid it.
 * @note The hollow rectangle is the one a script wants for a wall; the filled one is a separate call. Confusing the two gives a solid block where a wall was meant.
 */
/**
 * @brief 직사각형의 윤곽을 더한다. 그 안쪽이 아니라 변들을.
 * @param L 스크립트 상태
 * @return 결과 하나. 윤곽이 더해진 선택
 * @note 직사각형 함수가 아니라 네 선으로 그려지며, 그것이 모서리가 두 번 덮이는 이유다. 이미 설정된 칸을 설정하는 것은 무해하므로, 그것을 피할 필요가 있는 것은 없다.
 * @note 빈 직사각형이 스크립트가 벽에 원하는 것이고, 채워진 것은 따로 된 호출이다. 그 둘을 혼동하면 벽이 뜻해진 곳에 꽉 찬 덩이가 나온다.
 */
staticfn int
l_selection_rect(lua_State *L)
{
    struct selectionvar *sel = NULL;
    coordxy x1, y1, x2, y2;

    if (!params_sel_2coords(L, &sel, &x1, &y1, &x2, &y2)) {
        nhl_error(L, "selection.rect: illegal arguments");
    }

    get_location_coord(&x1, &y1, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x1, y1));
    get_location_coord(&x2, &y2, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x2, y2));

    (void) l_selection_clone(L);
    sel = l_selection_check(L, 2);
    selection_do_line(x1, y1, x2, y1, sel);
    selection_do_line(x1, y1, x1, y2, sel);
    selection_do_line(x2, y1, x2, y2, sel);
    selection_do_line(x1, y2, x2, y2, sel);
    return 1;
}

/**
 * @brief Add every square within a rectangle.
 *
 * The workhorse for describing a region. Also reachable under a second name, as the usage lines record, because "area" reads better than "fillrect" in a level description.
 *
 * @param L the script state
 * @return one result, the selection with the area added
 * @note A rectangle one column wide is handled separately, setting squares directly instead of drawing lines. Drawing a line from a square to itself is a degenerate case, and this avoids relying on how it behaves.
 */
/**
 * @brief 직사각형 안의 모든 칸을 더한다.
 *
 * 영역을 기술하는 일꾼이다. 사용법 줄들이 기록하듯 두 번째 이름으로도 닿을 수 있는데, 레벨 서술에서 "area"가 "fillrect"보다 잘 읽히기 때문이다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 영역이 더해진 선택
 * @note 한 열 너비의 직사각형은 따로 다뤄지며, 선을 그리는 대신 칸을 곧바로 설정한다. 한 칸에서 자기 자신으로 선을 그리는 것은 퇴화한 경우이며, 이것은 그것이 어떻게 행동하는지에 의존하는 것을 피한다.
 */
staticfn int
l_selection_fillrect(lua_State *L)
{
    struct selectionvar *sel = NULL;
    int y;
    coordxy x1, y1, x2, y2;

    if (!params_sel_2coords(L, &sel, &x1, &y1, &x2, &y2)) {
        nhl_error(L, "selection.fillrect: illegal arguments");
    }

    get_location_coord(&x1, &y1, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x1, y1));
    get_location_coord(&x2, &y2, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x2, y2));

    (void) l_selection_clone(L);
    sel = l_selection_check(L, 2);
    if (x1 == x2) {
        for (y = y1; y <= y2; y++)
            selection_setpoint(x1, y, sel, 1);
    } else {
        for (y = y1; y <= y2; y++)
            selection_do_line(x1, y, x2, y, sel);
    }
    return 1;
}

/**
 * @brief Add a wandering line between two squares.
 *
 * A line that does not go straight, used where a straight one would look built rather than found -- a cave passage, a river. The roughness argument says how far it may stray.
 *
 * @param L the script state
 * @return one result, the selection with the line added
 * @note Reads its own arguments instead of using the shared helper, because it takes a fifth value the other shapes do not. The two forms are distinguished by whether the first argument is a number, since the count alone does not settle it.
 * @note The recursion depth is fixed in the call rather than exposed, so a script controls how rough the line is but not how finely it is subdivided.
 * @note As the existing notes record, accepting the corners as tables is wanted and not implemented.
 */
/**
 * @brief 두 칸 사이에 굽이치는 선을 더한다.
 *
 * 곧게 가지 않는 선이며, 곧은 선이 발견된 것이 아니라 지어진 것처럼 보일 곳에 쓰인다. 동굴 통로, 강. 거칠기 인자가 그것이 얼마나 벗어날 수 있는지 말한다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 선이 더해진 선택
 * @note 공유된 도우미를 쓰는 대신 자기 인자를 스스로 읽는데, 다른 도형들이 갖지 않는 다섯 번째 값을 취하기 때문이다. 두 형태는 첫 인자가 숫자인지로 구별되는데, 개수만으로는 정해지지 않기 때문이다.
 * @note 재귀 깊이가 드러나는 대신 호출 안에 고정되어 있으므로, 스크립트는 선이 얼마나 거친지는 다스리지만 얼마나 곱게 나뉘는지는 다스리지 않는다.
 * @note 기존 적바림들이 기록하듯, 모서리를 표로 받아들이는 것이 바라지지만 구현되지 않았다.
 */
staticfn int
l_selection_randline(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel;
    coordxy x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int roughness = 7;

    if (argc == 6) {
        (void) l_selection_check(L, 1);
        x1 = (coordxy) luaL_checkinteger(L, 2);
        y1 = (coordxy) luaL_checkinteger(L, 3);
        x2 = (coordxy) luaL_checkinteger(L, 4);
        y2 = (coordxy) luaL_checkinteger(L, 5);
        roughness = (int) luaL_checkinteger(L, 6);
        lua_pop(L, 5);
    } else if (argc == 5 && lua_type(L, 1) == LUA_TNUMBER) {
        x1 = (coordxy) luaL_checkinteger(L, 1);
        y1 = (coordxy) luaL_checkinteger(L, 2);
        x2 = (coordxy) luaL_checkinteger(L, 3);
        y2 = (coordxy) luaL_checkinteger(L, 4);
        roughness = (int) luaL_checkinteger(L, 5);
        lua_pop(L, 5);
        (void) l_selection_new(L);
        (void) l_selection_check(L, 1);
    }

    get_location_coord(&x1, &y1, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x1, y1));
    get_location_coord(&x2, &y2, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x2, y2));

    (void) l_selection_clone(L);
    sel = l_selection_check(L, 2);
    selection_do_randline(x1, y1, x2, y2, roughness, 12, sel);
    return 1;
}

/** @} */

/**
 * @brief Expand a selection outward by one square.
 *
 * How a script thickens a shape or finds its border: grow it and subtract the original, and what remains is the ring around it.
 *
 * @param L the script state
 * @return one result, the grown selection
 * @note Growth may be restricted to one direction, or left to chance. The direction names are matched against a list, so a misspelled direction is reported by the script machinery rather than silently meaning "all".
 * @note The two arrays must stay in step -- a name's position is what selects its direction value -- and nothing enforces that beyond their being written side by side.
 */
/**
 * @brief 선택을 한 칸 밖으로 넓힌다.
 *
 * 스크립트가 도형을 두껍게 하거나 그 테두리를 찾는 방법이다. 그것을 키우고 원본을 빼면, 남는 것이 그 둘레의 고리다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 넓혀진 선택
 * @note 넓히기는 한 방향으로 제한될 수도, 운에 맡겨질 수도 있다. 방향 이름이 목록과 견주어 맞춰지므로, 잘못 쓴 방향은 조용히 "모두"를 뜻하는 대신 스크립트 기계가 보고한다.
 * @note 두 배열이 발을 맞춰 머물러야 한다. 이름의 위치가 그 방향 값을 고르는 것이다. 그리고 그것들이 나란히 쓰여 있는 것 말고는 무엇도 그것을 강제하지 않는다.
 */
staticfn int
l_selection_grow(lua_State *L)
{
    static const char *const growdirs[] = {
        "all", "random", "north", "west", "east", "south", NULL
    };
    static const int growdirs2i[] = {
        W_ANY, W_RANDOM, W_NORTH, W_WEST, W_EAST, W_SOUTH, 0
    };
    struct selectionvar *sel;
    int dir, argc = lua_gettop(L);

    (void) l_selection_check(L, 1);
    dir = growdirs2i[luaL_checkoption(L, 2, "all", growdirs)];

    if (argc == 2)
        lua_pop(L, 1); /* get rid of growdir */

    (void) l_selection_clone(L);
    sel = l_selection_check(L, 2);
    selection_do_grow(sel, dir);
    return 1;
}


/**
 * @brief Keep only the squares of a selection whose terrain is a given kind.
 *
 * Reads the map as it currently stands, which is what makes this different from every other operation here -- the others describe shapes, this one asks what has already been built. So a script can take a region and keep only its floor.
 *
 * @param L the script state
 * @return one result, the filtered selection
 * @note The terrain is named by the same character a map drawing uses, so a script's filter reads like the map it was drawn from.
 * @note Lighting may also be required, defaulting to not caring. As the existing note records, the special lighting values are not handled yet.
 * @note An unrecognised terrain character is reported rather than matching nothing, since a filter that silently matched nothing would look like a region that happened to be empty.
 * @warning Depends on when it is called. The same filter run before and after terrain is placed gives different answers, which is a property of reading the map rather than a fault.
 */
/**
 * @brief 선택의 칸 가운데 지형이 주어진 종류인 것만 남긴다.
 *
 * 맵을 현재 상태로 읽으며, 그것이 여기의 다른 모든 연산과 이것을 다르게 만드는 것이다. 다른 것들은 도형을 기술하고, 이것은 무엇이 이미 지어졌는지 묻는다. 그래서 스크립트가 어떤 영역을 취하고 그 바닥만 남길 수 있다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 걸러진 선택
 * @note 지형이 맵 그림이 쓰는 것과 같은 문자로 이름 지어지므로, 스크립트의 걸름이 그것이 그려진 맵처럼 읽힌다.
 * @note 조명도 요구될 수 있고, 개의치 않는 것이 기본값이다. 기존 적바림이 기록하듯 특별한 조명 값은 아직 다뤄지지 않는다.
 * @note 인식되지 않는 지형 문자는 아무것도 맞지 않는 대신 보고된다. 조용히 아무것도 맞지 않은 걸름은 마침 비어 있던 영역처럼 보일 것이기 때문이다.
 * @warning 언제 호출되는지에 달려 있다. 지형이 놓이기 전과 뒤에 실행된 같은 걸름이 다른 답을 주며, 그것은 결함이 아니라 맵을 읽는 것의 성질이다.
 */
staticfn int
l_selection_filter_mapchar(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = l_selection_check(L, 1);
    char *mapchr = dupstr(luaL_checkstring(L, 2));
    coordxy typ = check_mapchr(mapchr);
    int lit = (int) luaL_optinteger(L, 3, -2); /* TODO: special lit values */
    struct selectionvar *tmp;

    if (typ == INVALID_TYPE)
        nhl_error(L, "Erroneous map char");

    tmp = selection_filter_mapchar(sel, typ, lit);
    lua_pop(L, argc);
    l_selection_push_copy(L, tmp);
    selection_free(tmp, TRUE);

    if (mapchr)
        free(mapchr);

    return 1;
}

/**
 * @brief Find every place on the map where a small drawn pattern occurs.
 *
 * The pattern is given as a picture -- a few lines of map characters -- and the result holds every square where that picture fits. So a script can say "wherever a wall meets a floor like this" and get all such places at once.
 *
 * @param L the script state
 * @return one result, a selection of the matching squares
 * @note Reads the map as it stands, like the terrain filter, so it finds what has been built rather than describing a shape.
 * @note The bounds must be recalculated afterwards, and the existing comment gives the specific reason: squares are set across the whole map including ones that do not match, so unless the very first square matched the bounds would be left
 *       in their initial impossible state.
 * @note Accepts only the pattern, with no selection to narrow the search. The whole map is always searched.
 * @note A malformed pattern is reported with the parser's own message rather than a generic one, so a script author sees what was wrong with their picture.
 */
/**
 * @brief 맵에서 작게 그려진 무늬가 나타나는 모든 곳을 찾는다.
 *
 * 무늬는 그림으로 주어진다. 맵 문자 몇 줄. 그리고 결과는 그 그림이 들어맞는 모든 칸을 담는다. 그래서 스크립트가 "벽이 바닥과 이렇게 만나는 어디든"이라고 말하고 그런 모든 곳을 한꺼번에 얻을 수 있다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 맞는 칸들의 선택
 * @note 지형 걸름처럼 맵을 현재 상태로 읽으므로, 도형을 기술하는 대신 무엇이 지어졌는지 찾는다.
 * @note 경계가 뒤에 다시 계산되어야 하며, 기존 주석이 그 구체적인 이유를 준다. 맞지 않는 칸을 포함해 맵 전체에 걸쳐 칸이 설정되므로, 맨 첫 칸이 맞지 않았다면 경계가 그 처음의 있을 수 없는 상태로 남겨질 것이다.
 * @note 찾기를 좁힐 선택 없이 무늬만 받아들인다. 언제나 맵 전체가 찾아진다.
 * @note 잘못된 무늬는 일반적인 메시지가 아니라 파서 자신의 메시지로 보고되므로, 스크립트 작성자가 자기 그림의 무엇이 틀렸는지 본다.
 */
staticfn int
l_selection_match(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    struct mapfragment *mf = (struct mapfragment *) 0;
    int x, y;

    if (argc == 1) {
        const char *err;
        char *mapstr = dupstr(luaL_checkstring(L, 1));
        lua_pop(L, 1);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);

        mf = mapfrag_fromstr(mapstr);
        free(mapstr);

        if ((err = mapfrag_error(mf)) != NULL) {
            nhl_error(L, err);
            /*NOTREACHED*/
        }

    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }

    for (y = 0; y <= sel->hei; y++)
        for (x = 1; x < sel->wid; x++)
            selection_setpoint(x, y, sel, mapfrag_match(mf, x,y) ? 1 : 0);

    /* unless the (0, 1) coordinate is a match, this would wind up with a
       selection with lx=COLNO, hx=0, etc, so fix the boundaries */
    selection_recalc_bounds(sel);

    mapfrag_free(&mf);

    return 1;
}


/**
 * @brief Select the whole connected region of like terrain reachable from one square.
 *
 * Answers "everywhere I could walk from here without crossing anything different", which is how a script gets a cave's interior without knowing its shape.
 *
 * @param L the script state
 * @return one result, the flooded selection
 * @note The terrain to spread through is taken from the starting square itself, not given as an argument. So flooding from a floor square finds floor and flooding from water finds water -- the starting square decides what the region is made of.
 * @note Diagonal movement is optional and off by default, which changes the answer: two regions touching only at a corner are one region with diagonals and two without.
 * @note A starting square off the map yields an empty selection rather than an error, so a script computing a position need not check it first.
 */
/**
 * @brief 한 칸에서 닿을 수 있는, 같은 지형으로 이어진 영역 전체를 고른다.
 *
 * "여기서 다른 무엇도 건너지 않고 걸어갈 수 있는 모든 곳"에 답하며, 그것이 스크립트가 동굴의 모양을 모르고도 그 안쪽을 얻는 방법이다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 채워진 선택
 * @note 퍼져 나갈 지형은 인자로 주어지는 대신 시작 칸 자신에게서 취해진다. 그래서 바닥 칸에서 채우면 바닥을 찾고 물에서 채우면 물을 찾는다. 시작 칸이 그 영역이 무엇으로 이루어졌는지를 정한다.
 * @note 대각선 이동이 선택적이며 기본으로 꺼져 있고, 그것이 답을 바꾼다. 모서리에서만 닿는 두 영역은 대각선과 함께면 하나의 영역이고 없이면 둘이다.
 * @note 맵 밖의 시작 칸은 오류가 아니라 빈 선택을 내므로, 위치를 계산하는 스크립트가 그것을 먼저 검사할 필요가 없다.
 */
staticfn int
l_selection_flood(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    coordxy x = 0, y = 0;
    boolean diagonals = FALSE;

    if (argc == 2 || argc == 3) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        if (argc == 3)
            diagonals = lua_toboolean(L, 3);
        lua_pop(L, argc);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }

    get_location_coord(&x, &y, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x, y));

    if (isok(x, y)) {
        set_floodfillchk_match_under(levl[x][y].typ);
        selection_floodfill(sel, x, y, diagonals);
    }
    return 1;
}


/**
 * @brief Add a circle, hollow or filled, around a centre.
 * @param L the script state
 * @return one result, the selection with the circle added
 * @note Implemented as an ellipse with both radii equal, so the two share one drawing routine and cannot disagree about what a round shape looks like.
 * @note The filled flag is inverted before being passed on, because the routine underneath is told whether to draw only the outline rather than whether to fill.
 * @warning Unlike the line and rectangle shapes, this adds to the selection it was given rather than to a clone. A script that passes a selection and expects it back unchanged gets it modified. The two forms differ in this: called without a
 *          selection it creates one, so only the form that supplies one is affected.
 * @note As the existing notes record, the filled flag ought to be a boolean rather than a number.
 */
/**
 * @brief 중심 둘레에 원을 더한다. 비었거나 채워진.
 * @param L 스크립트 상태
 * @return 결과 하나. 원이 더해진 선택
 * @note 두 반지름이 같은 타원으로 구현되므로, 그 둘이 하나의 그리기 함수를 공유하고 둥근 도형이 어떻게 보이는지에 대해 어긋날 수 없다.
 * @note 채우기 표시가 넘겨지기 전에 뒤집힌다. 밑의 함수가 채울지가 아니라 윤곽만 그릴지를 듣기 때문이다.
 * @warning 선과 직사각형 도형과 달리, 이것은 복제가 아니라 자기가 받은 선택에 더한다. 선택을 넘기고 그것을 바뀌지 않은 채로 돌려받기를 기대하는 스크립트는 수정된 것을 얻는다. 두 형태가 이 점에서 다르다. 선택 없이 호출되면 하나를 만들므로, 하나를 내주는 형태만이 영향을 받는다.
 * @note 기존 적바림들이 기록하듯, 채우기 표시는 숫자가 아니라 참거짓이어야 한다.
 */
staticfn int
l_selection_circle(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    coordxy x = 0, y = 0;
    int r = 0, filled = 0;

    if (argc == 3) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        r = (int) luaL_checkinteger(L, 3);
        lua_pop(L, 3);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
        filled = 0;
    } else if (argc == 4 && lua_type(L, 1) == LUA_TNUMBER) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        r = (int) luaL_checkinteger(L, 3);
        filled = (int) luaL_checkinteger(L, 4); /* TODO: boolean*/
        lua_pop(L, 4);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
    } else if (argc == 4 || argc == 5) {
        sel = l_selection_check(L, 1);
        x = (coordxy) luaL_checkinteger(L, 2);
        y = (coordxy) luaL_checkinteger(L, 3);
        r = (int) luaL_checkinteger(L, 4);
        filled = (int) luaL_optinteger(L, 5, 0); /* TODO: boolean */
    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }

    get_location_coord(&x, &y, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x, y));

    selection_do_ellipse(sel, x, y, r, r, !filled);

    lua_settop(L, 1);
    return 1;
}

/**
 * @brief Add an ellipse with independent horizontal and vertical radii.
 * @param L the script state
 * @return one result, the selection with the ellipse added
 * @note Two radii rather than one, which matters on a map whose squares are taller than they are wide: a shape that is to look round on screen needs a wider horizontal radius than vertical.
 * @note The two forms are told apart by whether the first argument is a number as well as by the count, since four and five arguments can each mean either.
 * @warning Adds to the given selection rather than a clone, the same as the circle.
 */
/**
 * @brief 가로와 세로 반지름이 독립적인 타원을 더한다.
 * @param L 스크립트 상태
 * @return 결과 하나. 타원이 더해진 선택
 * @note 하나가 아니라 두 반지름인 것은, 칸이 넓이보다 높이가 큰 맵에서 중요하다. 화면에서 둥글게 보여야 하는 도형은 세로보다 넓은 가로 반지름을 필요로 한다.
 * @note 두 형태는 개수뿐 아니라 첫 인자가 숫자인지로도 구별된다. 넷과 다섯 인자가 각각 어느 쪽이든을 뜻할 수 있기 때문이다.
 * @warning 원과 마찬가지로 복제가 아니라 주어진 선택에 더한다.
 */
staticfn int
l_selection_ellipse(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    coordxy x = 0, y = 0;
    int r1 = 0, r2 = 0, filled = 0;

    if (argc == 4) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        r1 = (int) luaL_checkinteger(L, 3);
        r2 = (int) luaL_checkinteger(L, 4);
        lua_pop(L, 4);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
        filled = 0;
    } else if (argc == 5 && lua_type(L, 1) == LUA_TNUMBER) {
        x = (coordxy) luaL_checkinteger(L, 1);
        y = (coordxy) luaL_checkinteger(L, 2);
        r1 = (int) luaL_checkinteger(L, 3);
        r2 = (int) luaL_checkinteger(L, 4);
        filled = (int) luaL_optinteger(L, 5, 0); /* TODO: boolean */
        lua_pop(L, 5);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
    } else if (argc == 5 || argc == 6) {
        sel = l_selection_check(L, 1);
        x = (coordxy) luaL_checkinteger(L, 2);
        y = (coordxy) luaL_checkinteger(L, 3);
        r1 = (int) luaL_checkinteger(L, 4);
        r2 = (int) luaL_checkinteger(L, 5);
        filled = (int) luaL_optinteger(L, 6, 0); /* TODO: boolean */
    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }

    get_location_coord(&x, &y, ANY_LOC, gc.coder ? gc.coder->croom : NULL,
                       SP_COORD_PACK(x, y));

    selection_do_ellipse(sel, x, y, r1, r2, !filled);

    lua_settop(L, 1);
    return 1;
}

/**
 * @brief Add squares thinning out with distance from a centre or a line.
 *
 * A shape with a soft edge: everything close in is selected, everything far out is not, and in between the chance falls off with distance. Used where a hard boundary would look drawn -- the edge of a swamp, scattered rubble around a
 * collapse.
 *
 * The falloff is described by two distances, and the accompanying comment states it exactly: within the inner distance every square is selected, and from there to the outer distance the chance drops in a straight line to nothing.
 *
 * @param L the script state
 * @return one result, the gradient selection
 * @note Takes a table only, and the existing comment explains why: there are too many independently optional values for a positional form to be readable. This is the one shape here where naming the arguments was judged worth the departure.
 * @note The centre may be a line rather than a point, by giving a second coordinate pair. That yields a band rather than a blob, which is what a river bank or a corridor edge needs.
 * @note The outer distance is required while the inner defaults to nothing, and the existing comment gives the reason: there is no sensible default for how far the falloff reaches, whereas starting it at the centre is the obvious default.
 * @note A missing second coordinate is filled in from the first, turning the line into a point. As the existing comment records, the pair of minus ones is the only position treated as absent, deliberately -- a gradient centred off the map is
 *       allowed, and a designer who genuinely needs that one square can give it as the first coordinate instead.
 */
/**
 * @brief 중심이나 선에서 멀어지며 옅어지는 칸들을 더한다.
 *
 * 부드러운 가장자리를 가진 도형이다. 가까운 안쪽은 모두 골라지고, 먼 밖은 골라지지 않으며, 그 사이에서 가능성이 거리와 함께 떨어진다. 딱딱한 경계가 그려진 것처럼 보일 곳에 쓰인다. 늪의 끝, 무너진 곳 둘레에 흩어진 잔해.
 *
 * 떨어짐은 두 거리로 기술되며, 딸린 주석이 그것을 정확히 밝힌다. 안쪽 거리 안에서는 모든 칸이 골라지고, 거기서 바깥 거리까지 가능성이 직선으로 영까지 떨어진다.
 *
 * @param L 스크립트 상태
 * @return 결과 하나. 그 기울기 선택
 * @note 표만 취하며, 기존 주석이 이유를 설명한다. 자리로 정해지는 형태가 읽힐 수 있기에는 독립적으로 선택적인 값이 너무 많다. 이것은 여기서 인자를 이름 짓는 것이 그 벗어남을 들일 만하다고 판단된 유일한 도형이다.
 * @note 중심은 점이 아니라 선일 수 있으며, 두 번째 좌표 짝을 줌으로써다. 그것은 방울이 아니라 띠를 내는데, 그것이 강 언덕이나 통로 가장자리가 필요로 하는 것이다.
 * @note 바깥 거리가 요구되는 반면 안쪽은 없음이 기본값이며, 기존 주석이 그 이유를 준다. 떨어짐이 얼마나 멀리 이르는지에 대한 분별 있는 기본값이 없는 반면, 그것을 중심에서 시작하는 것은 뻔한 기본값이다.
 * @note 없는 두 번째 좌표는 첫 것으로 채워져 선을 점으로 만든다. 기존 주석이 기록하듯 마이너스 일의 짝이 없음으로 다뤄지는 유일한 위치이며, 그것은 의도적이다. 맵 밖에 중심을 둔 기울기가 허용되고, 진짜로 그 한 칸이 필요한 설계자는 그것을 대신 첫 좌표로 줄 수 있다.
 */
staticfn int
l_selection_gradient(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    /* if x2 and y2 aren't set, the gradient has a single center point of x,y;
     * if they are set, the gradient is centered on a (x,y) to (x2,y2) line */
    coordxy x = 0, y = 0, x2 = -1, y2 = -1;
    /* points are always added within mindist of the center; the chance for a
     * point between mindist and maxdist to be added to the selection starts
     * at 100% at mindist and decreases linearly to 0% at maxdist */
    coordxy mindist = 0, maxdist = 0;
    long type = 0;
    static const char *const gradtypes[] = {
        "radial", "square", NULL
    };
    static const int gradtypes2i[] = {
        SEL_GRADIENT_RADIAL, SEL_GRADIENT_SQUARE, -1
    };

    if (argc == 1 && lua_type(L, 1) == LUA_TTABLE) {
        lcheck_param_table(L);
        type = gradtypes2i[get_table_option(L, "type", "radial", gradtypes)];
        x = (coordxy) get_table_int(L, "x");
        y = (coordxy) get_table_int(L, "y");
        x2 = (coordxy) get_table_int_opt(L, "x2", -1);
        y2 = (coordxy) get_table_int_opt(L, "y2", -1);
        cvt_to_abscoord(&x, &y);
        cvt_to_abscoord(&x2, &y2);
        /* maxdist is required because there's no obvious default value for
         * it, whereas mindist has an obvious default of 0 */
        maxdist = get_table_int(L, "maxdist");
        mindist = get_table_int_opt(L, "mindist", 0);

        lua_pop(L, 1);
        (void) l_selection_new(L);
        sel = l_selection_check(L, 1);
    } else {
        nhl_error(L, "selection.gradient requires table argument");
        /* NOTREACHED */
    }

    /* someone might conceivably want to draw a gradient somewhere off-map. So
     * the only coordinate that's "illegal" for that is (-1,-1).
     * If a level designer really needs to draw a gradient line using that
     * coordinate, they can do so by setting regular x and y to -1. */
    if (x2 == -1 && y2 == -1) {
        x2 = x;
        y2 = y;
    }

    selection_do_gradient(sel, x, y, x2, y2, type, mindist, maxdist);
    lua_settop(L, 1);
    return 1;
}

DISABLE_WARNING_UNREACHABLE_CODE

/**
 * @brief Call a script function once for each square in a selection.
 *
 * What turns a shape into an action. A script builds a selection describing where something should happen, then iterates it to do the thing at each square.
 *
 * @param L the script state
 * @return no results
 * @note The coordinates handed to the function are in the script's own frame -- relative to the current room or map -- as the existing comment records, and absolute only when no map or room has been established. So the function receives
 *       coordinates in the same terms the script wrote, which is what lets it pass them straight back to another call.
 * @note Stops the whole iteration if the script function raises an error, and the existing comment gives the reason: continuing would call the same faulty function for every remaining square and bury the first error under hundreds like it.
 * @note Collects garbage once per row. The function may create a selection or an object wrapper on each call, and without this a large shape could accumulate thousands before the collector ran on its own.
 * @note Walks only the recorded bounds, and starts at column one rather than zero, since column zero is not part of the playable map.
 */
/**
 * @brief 선택 안의 각 칸마다 스크립트 함수를 한 번 호출한다.
 *
 * 도형을 행동으로 바꾸는 것이다. 스크립트가 무언가 일어나야 할 곳을 기술하는 선택을 만들고, 그다음 그것을 순회해 각 칸에서 그 일을 한다.
 *
 * @param L 스크립트 상태
 * @return 결과 없음
 * @note 기존 주석이 기록하듯 그 함수에 주어지는 좌표는 스크립트 자신의 틀에 있으며, 현재 방이나 맵에 상대적이고, 맵이나 방이 세워지지 않았을 때만 절대적이다. 그래서 그 함수는 스크립트가 쓴 것과 같은 말로 좌표를 받으며, 그것이 그것을 다른 호출로 곧바로 되넘길 수 있게 하는 것이다.
 * @note 스크립트 함수가 오류를 일으키면 순회 전체를 멈추며, 기존 주석이 이유를 준다. 이어 가는 것은 남은 모든 칸에 같은 결함 있는 함수를 호출하고 첫 오류를 그것 같은 수백 개 아래 묻을 것이다.
 * @note 행마다 한 번 쓰레기를 수집한다. 그 함수가 호출마다 선택이나 물건 감싸기를 만들 수 있고, 이것 없이는 큰 도형이 수집기가 스스로 돌기 전에 수천 개를 쌓을 수 있다.
 * @note 기록된 경계만 걸어가며, 영이 아니라 첫째 열에서 시작한다. 영째 열은 놀 수 있는 맵의 일부가 아니기 때문이다.
 */
staticfn int
l_selection_iterate(lua_State *L)
{
    int argc = lua_gettop(L);
    struct selectionvar *sel = (struct selectionvar *) 0;
    int x, y;
    NhRect rect = cg.zeroNhRect;

    if (argc == 2 && lua_type(L, 2) == LUA_TFUNCTION) {
        sel = l_selection_check(L, 1);
        selection_getbounds(sel, &rect);
        for (y = rect.ly; y <= rect.hy; y++) {
            for (x = max(1,rect.lx); x <= rect.hx; x++)
                if (selection_getpoint(x, y, sel)) {
                    coordxy tmpx = x, tmpy = y;
                    cvt_to_relcoord(&tmpx, &tmpy);
                    lua_pushvalue(L, 2);
                    lua_pushinteger(L, tmpx);
                    lua_pushinteger(L, tmpy);
                    if (nhl_pcall_handle(L, 2, 0, "l_selection_iterate",
                                         NHLpa_impossible)) {
                        /* abort loops to prevent possible error cascade */
                        goto out;
                    }
                }
            lua_gc(L, LUA_GCCOLLECT, 0);
        }
    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }
 out:
    return 0;
}

/**
 * @brief Describe a selection's size in words.
 * @param L the script state
 * @return one result, the description
 * @note Exists for the messages a level shows a player -- a region described as "a large area" rather than as a count of squares. A number would be precise and mean nothing to the person reading it.
 */
/**
 * @brief 선택의 크기를 말로 기술한다.
 * @param L 스크립트 상태
 * @return 결과 하나. 그 기술
 * @note 레벨이 플레이어에게 보이는 메시지를 위해 존재한다. 칸의 개수가 아니라 "넓은 지역"으로 기술되는 영역. 숫자는 정확하겠지만 그것을 읽는 사람에게 아무 뜻도 아닐 것이다.
 */
staticfn int
l_selection_size_description(lua_State *L)
{
    int argc = lua_gettop(L);

    if (argc == 1) {
        struct selectionvar *sel = l_selection_check(L, 1);
        char buf[BUFSZ];

        lua_pushstring(L, selection_size_description(sel, buf));
        return 1;
    } else {
        nhl_error(L, "wrong parameters");
        /*NOTREACHED*/
    }
    return 0;
}

RESTORE_WARNING_UNREACHABLE_CODE

/**
 * @var l_selection_methods
 * @brief The names a script calls these by.
 *
 * The whole script-visible interface. A name here can be used either as a plain function or as a method on a selection, which is why almost every function above accepts the selection as an optional first argument -- the two spellings reach
 * the same code.
 *
 * @note Two names lead to the same function: filling a rectangle is also reachable as "area", because that is what a level description usually means by it.
 */
/**
 * @var l_selection_methods
 * @brief 스크립트가 이것들을 부르는 이름들.
 *
 * 스크립트에 보이는 인터페이스 전부다. 여기의 이름은 맨 함수로도 선택에 대한 메서드로도 쓰일 수 있으며, 그것이 위의 거의 모든 함수가 선택을 선택적인 첫 인자로 받아들이는 이유다. 두 표기가 같은 코드에 이른다.
 *
 * @note 두 이름이 같은 함수로 이끈다. 직사각형 채우기는 "area"로도 닿을 수 있는데, 레벨 서술이 보통 그것으로 뜻하는 것이 그것이기 때문이다.
 */
static const struct luaL_Reg l_selection_methods[] = {
    { "new", l_selection_new },
    { "clone", l_selection_clone },
    { "get", l_selection_getpoint },
    { "set", l_selection_setpoint },
    { "numpoints", l_selection_numpoints },
    { "negate", l_selection_not },
    { "percentage", l_selection_filter_percent },
    { "rndcoord", l_selection_rndcoord },
    { "line", l_selection_line },
    { "randline", l_selection_randline },
    { "rect", l_selection_rect },
    { "fillrect", l_selection_fillrect },
    { "area", l_selection_fillrect },
    { "grow", l_selection_grow },
    { "filter_mapchar", l_selection_filter_mapchar },
    { "match", l_selection_match },
    { "floodfill", l_selection_flood },
    { "circle", l_selection_circle },
    { "ellipse", l_selection_ellipse },
    { "gradient", l_selection_gradient },
    { "iterate", l_selection_iterate },
    { "bounds", l_selection_getbounds },
    { "room", l_selection_room },
    { "describe_size", l_selection_size_description },
    { NULL, NULL }
};

/**
 * @var l_selection_meta
 * @brief The operators a script may use on selections, and the collector hook.
 *
 * Binding the set operations to operators is what lets a level description read as arithmetic on shapes -- room minus middle, this or that -- instead of as a chain of function calls. That readability is the point of the whole selection idea.
 *
 * @note Negation appears twice, under both the arithmetic and the bitwise negation operators, so either spelling inverts a selection. A script author is unlikely to know which one this ought to be.
 * @note Addition is bound to union, as the existing comment records. Adding shapes is what a script author would try, and union is what they would mean by it.
 * @note Subtraction is the only asymmetric operator here, so it is the only one where the order matters.
 * @note The collector hook is registered alongside the operators because it is the same kind of thing -- behaviour attached to the type rather than a method called by name.
 * @note As the existing note records, making a selection directly iterable through the standard mechanism is wanted and not done; iteration goes through a named call instead.
 */
/**
 * @var l_selection_meta
 * @brief 스크립트가 선택에 쓸 수 있는 연산자들, 그리고 수집기 갈고리.
 *
 * 집합 연산을 연산자에 묶는 것이 레벨 서술을 함수 호출의 사슬이 아니라 도형에 대한 산술로 읽히게 하는 것이다. 방 빼기 가운데, 이것 또는 저것. 그 읽힘이 선택이라는 발상 전체의 요점이다.
 *
 * @note 뒤집기가 산술 부정 연산자와 비트 부정 연산자 양쪽에 두 번 나타나므로, 어느 표기로든 선택을 뒤집는다. 스크립트 작성자가 이것이 어느 쪽이어야 하는지 알 가능성은 낮다.
 * @note 기존 주석이 기록하듯 더하기가 합집합에 묶여 있다. 도형을 더하는 것이 스크립트 작성자가 시도할 것이고, 합집합이 그가 그것으로 뜻할 것이다.
 * @note 빼기가 여기서 유일한 비대칭 연산자이므로, 순서가 중요한 유일한 것이다.
 * @note 수집기 갈고리가 연산자와 나란히 등록되는 것은 그것이 같은 종류의 것이기 때문이다. 이름으로 호출되는 메서드가 아니라 형에 붙은 행동.
 * @note 기존 적바림이 기록하듯, 선택을 표준 방식으로 곧바로 순회할 수 있게 만드는 것이 바라지지만 이루어지지 않았다. 순회는 대신 이름 지어진 호출을 통한다.
 */
static const luaL_Reg l_selection_meta[] = {
    { "__gc", l_selection_gc },
    { "__unm", l_selection_not },
    { "__band", l_selection_and },
    { "__bor", l_selection_or },
    { "__bxor", l_selection_xor },
    { "__bnot", l_selection_not },
    { "__add", l_selection_or }, /* this aliases + to be the same as | */
    { "__sub", l_selection_sub },
    /* TODO: http://lua-users.org/wiki/MetatableEvents
       { "__ipairs", l_selection_ipairs },
    */
    { NULL, NULL }
};

/**
 * @brief Make selections available to a script.
 *
 * Builds the method table, creates the type identity the checking code looks for, attaches the operators to it, and points the type's lookup back at the method table -- which is what lets a selection be used both as a value with methods and
 * as a name to call functions on.
 *
 * @param L the script state
 * @return the number of values left for the script
 * @note The type identity is created by name here. That name is the same one the checking function demands, so the two must agree or every selection a script is handed would be rejected as the wrong type.
 * @note Must run before any script that uses selections. Nothing checks that, so a script loaded too early would find the whole facility missing rather than broken.
 */
/**
 * @brief 선택을 스크립트에서 쓸 수 있게 만든다.
 *
 * 메서드 표를 만들고, 검사 코드가 찾는 형 정체를 만들고, 연산자를 그것에 붙이고, 그 형의 찾기를 메서드 표로 되돌려 가리킨다. 그것이 선택이 메서드를 가진 값으로도, 함수를 호출할 이름으로도 쓰일 수 있게 하는 것이다.
 *
 * @param L 스크립트 상태
 * @return 스크립트를 위해 남겨진 값의 개수
 * @note 형 정체가 여기서 이름으로 만들어진다. 그 이름은 검사 함수가 요구하는 것과 같은 것이므로, 그 둘이 일치해야 한다. 그러지 않으면 스크립트가 받는 모든 선택이 틀린 형으로 거부될 것이다.
 * @note 선택을 쓰는 어떤 스크립트보다 먼저 실행되어야 한다. 무엇도 그것을 검사하지 않으므로, 너무 이르게 불려온 스크립트는 그 편의가 망가진 것이 아니라 아예 없는 것을 발견할 것이다.
 */
int
l_selection_register(lua_State *L)
{
    /* Table of instance methods and static methods. */
    luaL_newlib(L, l_selection_methods);

    /* metatable = { __name = "selection", __gc = l_selection_gc } */
    luaL_newmetatable(L, "selection");
    luaL_setfuncs(L, l_selection_meta, 0);

    /* metatable.__index points at the selection method table. */
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, "__index");

    /* Don't let lua code mess with the real metatable.
       Instead offer a fake one that only contains __gc. */
    luaL_newlib(L, l_selection_meta);
    lua_setfield(L, -2, "__metatable");

    /* We don't need the metatable anymore. It's safe in the
       Lua registry for use by luaL_setmetatable. */
    lua_pop(L, 1);

    /* global selection = the method table we created at the start */
    lua_setglobal(L, "selection");

    return 0;
}
