/* NetHack 5.0	utf8map.c	*/
/* Copyright (c) Michael Allison, 2021. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file utf8map.c
 * @brief 글리프(glyph)의 UTF-8/유니코드 표현 매핑 및 커스터마이징 처리.
 *
 * "U+NNNN" 형식의 유니코드 값 파싱, 글리프에 대한 UTF-8 문자열 매핑 저장/해제,
 * @c \\GNNNNNNNN 참조를 실제 UTF-8 시퀀스로 치환하는 변환, 사용자 정의
 * 유니코드 표현 항목 관리 등을 제공한다. 대부분 @c ENHANCED_SYMBOLS 빌드에서만
 * 컴파일된다.
 */

#include "hack.h"

#ifdef ENHANCED_SYMBOLS

extern const struct symparse loadsyms[];
extern struct enum_dump monsdump[];
extern struct enum_dump objdump[];
extern glyph_map glyphmap[MAX_GLYPH];
extern const char *const known_handling[];        /* symbols.c */

/* hexdd[] is defined in decl.c */

/**
 * @brief "U+NNNN" 형식의 문자열을 유니코드 코드포인트 정수로 변환한다.
 * @param[in] cp 변환할 문자열(예: "U+1F600"). NULL 이거나 형식이 아니면 0 반환.
 * @return 파싱된 유니코드 코드포인트, 유효하지 않으면 0.
 */
int
unicode_val(const char *cp)
{
    const char *dp;
    int cval = 0, dcount;

    if (cp && *cp) {
        cval = dcount = 0;
        if ((*cp == 'U' || *cp == 'u')
            && cp[1] == '+' && cp[2] && (dp = strchr(hexdd, cp[2])) != 0) {
            cp += 2; /* move past the 'U' and '+' */
            do {
                cval = (cval * 16) + ((int) (dp - hexdd) / 2);
            } while (*++cp && (dp = strchr(hexdd, *cp)) != 0 && ++dcount < 7);
        }
    }
    return cval;
}

/**
 * @brief 글리프 매핑에 유니코드 표현(UTF-32 코드포인트 + UTF-8 문자열)을 설정한다.
 * @param[in,out] gmap    표현을 설정할 글리프 매핑.
 * @param[in]     utf32ch UTF-32 코드포인트.
 * @param[in]     utf8str 대응하는 UTF-8 문자열.
 * @return 성공 시 1, @p gmap 이 NULL 이거나 @p utf32ch 가 0이면 0.
 * @note 필요 시 표현 구조체를 새로 할당하며, 기존 UTF-8 문자열은 해제 후 교체한다.
 */
int
set_map_u(glyph_map *gmap, uint32 utf32ch, const uint8 *utf8str)
{
    glyph_map *tmpgm = gmap;

    if (!tmpgm || !utf32ch)
        return 0;

    if (gmap->u == 0) {
        gmap->u =
            (struct unicode_representation *) alloc(sizeof *gmap->u);
        gmap->u->utf8str = 0;
    }
    if (gmap->u->utf8str != 0) {
        free(gmap->u->utf8str);
        gmap->u->utf8str = 0;
    }
    gmap->u->utf8str = (uint8 *) dupstr((const char *) utf8str);
    gmap->u->utf32ch = utf32ch;
    return 1;
}

/**
 * @brief 모든 글리프 매핑의 유니코드 표현 메모리를 해제한다.
 * @note 해제 후 화면 버퍼(@c gg.gbuf)의 해당 포인터도 NULL 로 지워 use-after-free
 *       를 방지한다.
 */
void
free_all_glyphmap_u(void)
{
    int glyph;
    int x, y;

    for (glyph = 0; glyph < MAX_GLYPH; ++glyph) {
        if (glyphmap[glyph].u) {
            if (glyphmap[glyph].u->utf8str) {
                free(glyphmap[glyph].u->utf8str);
                glyphmap[glyph].u->utf8str = 0;
            }
            free(glyphmap[glyph].u);
            glyphmap[glyph].u = 0;
        }
    }
    /* Prevent use after free from gg.gbuf */
    for (y = 0; y < ROWNO; ++y) {
        for (x = 0; x < COLNO; ++x) {
            gg.gbuf[y][x].glyphinfo.gm.u = NULL;
        }
    }
}

/**
 * @brief 문자열 내 @c \\GNNNNNNNN 글리프 참조를 실제 UTF-8 시퀀스로 치환한다.
 *
 * 윈도우 포트가 글리프 표현을 문자열에 직접 삽입하고자 할 때 사용한다.
 *
 * @param[out] buf      결과를 담을 버퍼.
 * @param[in]  bufsz    결과 버퍼의 크기(널 종료 공간 포함).
 * @param[in]  str      변환할 원본 문자열. NULL 이면 빈 문자열을 만든다.
 * @param[out] retflags NULL 이 아니면, UTF-8 시퀀스를 삽입했는지(1) 심볼로
 *                      대체했는지(0)를 마지막 처리 기준으로 기록한다.
 * @return 결과가 기록된 @p buf 를 그대로 반환한다.
 * @warning 결과는 @p bufsz-1 바이트를 넘지 않도록 잘린다.
 */
/* helper routine if a window port wants to embed any UTF-8 sequences
   for the glyph representation in the string in place of the \GNNNNNNNN
   reference */
char *
mixed_to_utf8(char *buf, size_t bufsz, const char *str, int *retflags)
{
    char *put = buf;
    glyph_info glyphinfo = nul_glyphinfo;

    if (!str)
        return strcpy(buf, "");

    while (*str && put < (buf + bufsz) - 1) {
        if (*str == '\\') {
            int dcount, so, ggv;
            const char *save_str;

            save_str = str++;
            switch (*str) {
            case 'G': /* glyph value \GXXXXNNNN*/
                if ((dcount = decode_glyph(str + 1, &ggv))) {
                    str += (dcount + 1);
                    map_glyphinfo(0, 0, ggv, 0, &glyphinfo);
                    if (glyphinfo.gm.u && glyphinfo.gm.u->utf8str) {
                        uint8 *ucp = glyphinfo.gm.u->utf8str;

                        while (*ucp && put < (buf + bufsz) - 1)
                            *put++ = *ucp++;
                        if (retflags)
                            *retflags = 1;
                    } else {
                        so = glyphinfo.gm.sym.symidx;
                        *put++ = gs.showsyms[so];
                        if (retflags)
                            *retflags = 0;
                    }
                    /* 'str' is ready for the next loop iteration and
                        '*str' should not be copied at the end of this
                        iteration */
                    continue;
                } else {
                    /* possible forgery - leave it the way it is */
                    str = save_str;
                }
                break;
            case '\\':
                break;
            case '\0':
                /* String ended with '\\'.  This can happen when someone
                    names an object with a name ending with '\\', drops the
                    named object on the floor nearby and does a look at all
                    nearby objects. */
                /* brh - should we perhaps not allow things to have names
                    that contain '\\' */
                str = save_str;
                break;
            }
        }
        if (put < (buf + bufsz) - 1)
            *put++ = *str++;
    }
    *put = '\0';
    return buf;
}

/**
 * @brief 사용자 정의 유니코드 표현(urep) 항목을 추가하거나 갱신한다.
 *
 * 동일 글리프에 대한 항목이 이미 있으면 갱신하고, 없으면 새 항목을 만들어
 * 커스터마이징 목록에 추가한다.
 *
 * @param[in] customization_name 커스터마이징 이름.
 * @param[in] glyphidx           대상 글리프 인덱스.
 * @param[in] utf32ch            UTF-32 코드포인트(0이면 표현 제거).
 * @param[in] utf8str            대응하는 UTF-8 문자열.
 * @param[in] which_set          적용할 그래픽 세트.
 * @return 항상 1(성공).
 */
int
add_custom_urep_entry(
    const char *customization_name,
    int glyphidx,
    uint32 utf32ch,
    const uint8 *utf8str,
    enum graphics_sets which_set)
{
    struct symset_customization *gdc
        = &gs.sym_customizations[which_set][custom_ureps];
    struct customization_detail *details, *newdetails = 0;


    if (!gdc->details) {
        gdc->customization_name = dupstr(customization_name);
        gdc->custtype = custom_ureps;
        gdc->details = 0;
        gdc->details_end = 0;
    }
    details = find_matching_customization(customization_name,
                                          custom_ureps, which_set); /* FIXME */
    if (details) {
        while (details) {
            if (details->content.urep.glyphidx == glyphidx) {
                if (details->content.urep.u.utf8str)
                    free(details->content.urep.u.utf8str);
                if (utf32ch) {
                    details->content.urep.u.utf8str =
                        (uint8 *) dupstr((const char *) utf8str);
                    details->content.urep.u.utf32ch = utf32ch;
                } else {
                    details->content.urep.u.utf8str = (uint8 *) 0;
                    details->content.urep.u.utf32ch = 0;
                }
                return 1;
            }
            details = details->next;
        }
    }
    /* create new details entry */
    newdetails = (struct customization_detail *) alloc(
                                        sizeof (struct customization_detail));
    newdetails->content.urep.glyphidx = glyphidx;
    if (utf8str && *utf8str) {
        newdetails->content.urep.u.utf8str =
            (uint8 *) dupstr((const char *) utf8str);
    } else {
        newdetails->content.urep.u.utf8str =
            (uint8 *) 0;
    }
    newdetails->content.urep.u.utf32ch = utf32ch;
    newdetails->next = (struct customization_detail *) 0;
    if (gdc->details == NULL) {
        gdc->details = newdetails;
    } else {
        gdc->details_end->next = newdetails;
    }
    gdc->details_end = newdetails;
    gdc->count++;
    return 1;
}
#endif /* ENHANCED_SYMBOLS */

/**
 * @brief 사용자 정의 심볼을 초기화하고 현재 그래픽 세트에 다시 적용한다.
 * @note @c ENHANCED_SYMBOLS 빌드가 아니면 아무 동작도 하지 않는다.
 */
void
reset_customsymbols(void)
{
#ifdef ENHANCED_SYMBOLS
    free_all_glyphmap_u();
    apply_customizations(gc.currentgraphics, do_custom_symbols);
#endif
}

/* utf8map.c */



