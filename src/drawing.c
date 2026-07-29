/* NetHack 5.0	drawing.c	$NHDT-Date: 1781973047 2026/06/20 16:30:47 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.97 $ */
/* Copyright (c) NetHack Development Team 1992.                   */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file drawing.c
 * @brief 화면 표시에 쓰이는 기본 심볼(문자) 테이블과 심볼 변환 함수.
 *
 * 오브젝트 클래스/몬스터 클래스/경고/지형(furniture) 등에 대한 기본 심볼과
 * 설명 텍스트, 색상 테이블을 정의하고, 문자 심볼을 해당 클래스 인덱스로
 * 변환하는 함수를 제공한다. 관련 정의는 rm.h, objclass.h, sym.h, defsym.h 참고.
 */

#include "config.h"
#include "color.h"
#include "rm.h"
#include "objclass.h"
#include "wintype.h"
#include "sym.h"

extern const struct symparse loadsyms[];
extern const struct class_sym def_oc_syms[MAXOCLASSES];
extern const struct class_sym def_monsyms[MAXMCLASSES];
extern const uchar def_r_oc_syms[MAXOCLASSES];

/* Relevant header information in rm.h, objclass.h, sym.h, defsym.h. */

/* Default object class symbols.  See objclass.h.
 * {symbol, name, explain}
 *     name:    used in object_detect().
 *     explain: used in do_look().
 */
/** @brief 오브젝트 클래스별 기본 심볼 테이블(심볼/이름/설명). */
const struct class_sym def_oc_syms[MAXOCLASSES] = {
    { '\0', "", "" }, /* placeholder for the "random class" */
#define OBJCLASS_DRAWING
#include "defsym.h"
#undef OBJCLASS_DRAWING
};

/** @brief 몬스터 클래스별 기본 심볼 테이블. See sym.h and defsym.h. */
const struct class_sym def_monsyms[MAXMCLASSES] = {
    { '\0', "", "" },
#define MONSYMS_DRAWING
#include "defsym.h"
#undef MONSYMS_DRAWING
};

/** @brief 위험 경고 단계별 심볼/설명/색상 테이블. */
const struct symdef def_warnsyms[WARNCOUNT] = {
    /* white warning  */
    { '0', "unknown creature causing you worry",    CLR_WHITE },
    /* pink warning   */
    { '1', "unknown creature causing you concern",  CLR_RED },
    /* red warning    */
    { '2', "unknown creature causing you anxiety",  CLR_RED },
    /* ruby warning   */
    { '3', "unknown creature causing you disquiet", CLR_RED },
    /* purple warning */
    { '4', "unknown creature causing you alarm",    CLR_MAGENTA },
    /* black warning  */
    { '5', "unknown creature causing you dread",    CLR_BRIGHT_MAGENTA },
};

/*
 *  Default screen symbols with explanations and colors.
 *
 *  If adding to or removing from this list, please note that,
 *  for builds with tile support, there is an array called altlabels[] in
 *  win/share/tilemap.c that requires the same number of elements as
 *  this, in the same order. It is used for tile name matching when
 *  parsing other.txt because some of the useful tile names don't exist
 *  within NetHack itself.
 */
/** @brief 화면 문자별 기본 심볼/설명/색상 테이블(지형·함정 등 포함). */
const struct symdef defsyms[MAXPCHARS + 1] = {
#define PCHAR_DRAWING
#include "defsym.h"
#undef PCHAR_DRAWING
    { 0, NULL, NO_COLOR }
};

/** @brief 로그(rogue) 레벨용 오브젝트 클래스 기본 심볼 테이블. */
/* default rogue level symbols */
const uchar def_r_oc_syms[MAXOCLASSES] = {
/* 0*/ '\0', ILLOBJ_SYM, WEAPON_SYM, ']', /* armor */
       RING_SYM,
/* 5*/ ',',                     /* amulet */
       TOOL_SYM, ':',           /* food */
       POTION_SYM, SCROLL_SYM,
/*10*/ SPBOOK_SYM, WAND_SYM,
       GEM_SYM,                /* gold -- yes it's the same as gems */
       GEM_SYM, ROCK_SYM,
/*15*/ BALL_SYM, CHAIN_SYM, VENOM_SYM
};

/**
 * @brief 문자를 오브젝트 클래스 인덱스로 변환한다.
 *
 * @param[in] ch 변환할 심볼 문자.
 * @return 대응하는 오브젝트 클래스 인덱스.
 * @retval MAXOCLASSES 인식할 수 없는 문자일 경우.
 * @note detect.c, invent.c, o_init.c, objnam.c, options.c, pickup.c,
 *       sp_lev.c, windows.c 등에서 사용된다.
 */
int
def_char_to_objclass(char ch)
{
    int i;

    for (i = 1; i < MAXOCLASSES; i++)
        if (ch == def_oc_syms[i].sym)
            break;
    return i;
}

/**
 * @brief 문자를 몬스터 클래스 인덱스로 변환한다.
 *
 * 가장 먼저 일치하는 항목의 인덱스를 반환한다.
 *
 * @param[in] ch 변환할 심볼 문자.
 * @return 대응하는 몬스터 클래스 인덱스.
 * @retval MAXMCLASSES 인식할 수 없는 문자일 경우.
 * @note detect.c, mondata.c, options.c, pickup.c, sp_lev.c, windows.c
 *       등에서 사용된다.
 */
int
def_char_to_monclass(char ch)
{
    int i;

    for (i = 1; i < MAXMCLASSES; i++)
        if (ch == def_monsyms[i].sym)
            break;
    return i;
}

/**
 * @brief 문자가 지형지물(furniture) 심볼인지 판별한다.
 *
 * @c defsyms[] 배열에서 지형지물이 차지하는 연속 구간을 검색한다.
 *
 * @param[in] ch 판별할 심볼 문자.
 * @return 일치하는 @c defsyms[] 인덱스.
 * @retval -1 지형지물 심볼이 아닐 경우.
 */
int
def_char_is_furniture(char ch)
{
    /* note: these refer to defsyms[] order which is much different from
       levl[][].typ order but both keep furniture in a contiguous block */
    static const char first_furniture[] = "stair", /* "staircase up" */
                      last_furniture[] = "fountain";
    int i;
    boolean furniture = FALSE;

    for (i = 0; i < MAXPCHARS; ++i) {
        if (!furniture) {
            if (!strncmp(defsyms[i].explanation, first_furniture, 5))
                furniture = TRUE;
        }
        if (furniture) {
            if (defsyms[i].sym == (uchar) ch)
                return i;
            if (!strcmp(defsyms[i].explanation, last_furniture))
                break; /* reached last furniture */
        }
    }
    return -1;
}

/*drawing.c*/
