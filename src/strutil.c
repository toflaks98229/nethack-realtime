/* NetHack 5.0	strutil.c	$NHDT-Date: 1709571807 2024/03/04 17:03:27 $  $NHDT-Branch: keni-mdlib-followup $:$NHDT-Revision: 1.0 $ */
/* Copyright (c) Robert Patrick Rankin, 1991                      */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file strutil.c
 * @brief 문자열 관련 유틸리티: 동적 문자열 버퍼(strbuf)와 패턴 매칭.
 *
 * 자동 확장되는 문자열 버퍼 @c strbuf_t 를 다루는 함수들과, 와일드카드
 * (@c *, @c ?)를 지원하는 단순 패턴 매칭 함수(@c pmatch 계열),
 * 그리고 길이 상한을 점검하는 안전한 @c strlen 대체(@c Strlen_)를 제공한다.
 *
 * @note dlb 및 nethack 양쪽에서 사용된다.
 */

#include "hack.h" /* for config.h+extern.h */

staticfn boolean pmatch_internal(const char *, const char *, boolean,
                               const char *);

/**
 * @brief 문자열 버퍼를 초기 상태로 설정한다.
 *
 * @param[out] strbuf 초기화할 버퍼.
 */
void
strbuf_init(strbuf_t *strbuf)
{
    strbuf->str = NULL;
    strbuf->len = 0;
}

/**
 * @brief 문자열 버퍼 끝에 주어진 문자열을 이어 붙인다.
 *
 * 필요한 만큼 저장 공간을 자동으로 확보한 뒤 @p str 을 덧붙인다.
 *
 * @param[in,out] strbuf 대상 버퍼.
 * @param[in]     str    이어 붙일 널 종료 문자열.
 */
void
strbuf_append(strbuf_t *strbuf, const char *str)
{
    int len = (int) strlen(str) + 1;

    strbuf_reserve(strbuf,
                   len + (strbuf->str ? (int) strlen(strbuf->str) : 0));
    Strcat(strbuf->str, str);
}

/**
 * @brief 문자열 버퍼가 @p len 문자를 담을 저장 공간을 갖도록 보장한다.
 *
 * 최초 사용 시 내장 버퍼를 사용하고, 용량이 부족하면 힙에 재할당하여
 * 기존 내용을 복사한다.
 *
 * @param[in,out] strbuf 대상 버퍼.
 * @param[in]     len    확보할 최소 문자 수.
 * @note 힙 재할당이 일어나면 이전 힙 버퍼는 free 된다(내장 버퍼는 예외).
 */
void
strbuf_reserve(strbuf_t *strbuf, int len)
{
    if (strbuf->str == NULL) {
        strbuf->str = strbuf->buf;
        strbuf->str[0] = '\0';
        strbuf->len = (int) sizeof strbuf->buf;
    }

    if (len > strbuf->len) {
        char *oldbuf = strbuf->str;

        strbuf->len = len + (int) sizeof strbuf->buf;
        strbuf->str = (char *) alloc(strbuf->len);
        Strcpy(strbuf->str, oldbuf);
        if (oldbuf != strbuf->buf)
            free((genericptr_t) oldbuf);
    }
}

/**
 * @brief 문자열 버퍼가 할당한 메모리를 해제하고 초기 상태로 되돌린다.
 *
 * @param[in,out] strbuf 비울 버퍼.
 * @note 힙에 할당된 저장 공간만 free 하며, 내장 버퍼는 free 하지 않는다.
 */
void
strbuf_empty(strbuf_t *strbuf)
{
    if (strbuf->str != NULL && strbuf->str != strbuf->buf)
        free((genericptr_t) strbuf->str);
    strbuf_init(strbuf);
}

/**
 * @brief 버퍼 내 모든 개행(@c \\n)을 CRLF(@c \\r\\n)로 변환한다.
 *
 * 변환으로 늘어나는 길이만큼 저장 공간을 확보한 뒤 제자리에서 치환한다.
 *
 * @param[in,out] strbuf 대상 버퍼.
 */
void
strbuf_nl_to_crlf(strbuf_t *strbuf)
{
    if (strbuf->str) {
        int len = (int) strlen(strbuf->str);
        int count = 0;
        char *cp = strbuf->str;

        while (*cp)
            if (*cp++ == '\n')
                count++;
        if (count) {
            strbuf_reserve(strbuf, len + count + 1);
            for (cp = strbuf->str + len + count; count; --cp)
                if ((*cp = cp[-count]) == '\n') {
                    *--cp = '\r';
                    --count;
                }
        }
    }
}

/**
 * @brief 길이 상한을 점검하는 안전한 @c strlen 대체 구현.
 *
 * 문자열이 비정상적으로 길면(@c LARGEST_INT 이상) @c panic() 으로 중단한다.
 * dlb 와 nethack 양쪽에서 사용된다.
 *
 * @param[in] str  길이를 잴 널 종료 문자열.
 * @param[in] file 호출 위치의 소스 파일명(진단용).
 * @param[in] line 호출 위치의 소스 라인 번호(진단용).
 * @return 문자열의 길이(부호 없는 값).
 * @warning 길이가 @c LARGEST_INT 에 도달하면 @c panic() 으로 프로그램을 중단한다.
 */
unsigned
Strlen_(
    const char *str,
    const char *file,
    int line)
{
    const char *p;
    size_t len;

    /* strnlen(str, LARGEST_INT) w/o requiring posix.1 headers or libraries */
    for (p = str, len = 0; len < LARGEST_INT; ++len)
        if (*p++ == '\0')
            break;

    if (len == LARGEST_INT)
        panic("%s:%d string too long", file, line);
    return (unsigned) len;
}

/**
 * @brief 대소문자를 구분하는 와일드카드 패턴 매칭.
 *
 * @param[in] patrn 패턴 문자열(@c * 은 0개 이상, @c ? 는 임의의 한 문자와 일치).
 * @param[in] strng 검사할 대상 문자열.
 * @return 일치 여부.
 * @retval TRUE  @p strng 이 @p patrn 과 일치한다.
 * @retval FALSE 일치하지 않는다.
 */
boolean
pmatch(const char *patrn, const char *strng)
{
    return pmatch_internal(patrn, strng, FALSE, (const char *) 0);
}

/**
 * @brief 대소문자를 구분하지 않는 와일드카드 패턴 매칭.
 *
 * @param[in] patrn 패턴 문자열(@c * 은 0개 이상, @c ? 는 임의의 한 문자와 일치).
 * @param[in] strng 검사할 대상 문자열.
 * @return 일치 여부.
 * @retval TRUE  @p strng 이 @p patrn 과 일치한다.
 * @retval FALSE 일치하지 않는다.
 */
boolean
pmatchi(const char *patrn, const char *strng)
{
    return pmatch_internal(patrn, strng, TRUE, (const char *) 0);
}

/**
 * @brief @c pmatch(), @c pmatchi(), @c pmatchz() 의 공통 구현부.
 *
 * 문자열을 패턴에 대해 매칭한다. @c * 은 0개 이상, @c ? 는 임의의 한 문자와
 * 일치한다. 꼬리 재귀는 @c goto 로 최적화되어 있다.
 *
 * @param[in] patrn 패턴 문자열.
 * @param[in] strng 검사할 대상 문자열.
 * @param[in] ci    TRUE 이면 대소문자 무시, FALSE 이면 구분.
 * @param[in] sk    매칭 시 건너뛸(무시할) 문자들의 집합. NULL 이면 사용 안 함.
 * @return 일치 여부.
 * @retval TRUE  일치한다.
 * @retval FALSE 일치하지 않는다.
 */
staticfn boolean
pmatch_internal(const char *patrn, const char *strng,
                boolean ci,     /* True => case-insensitive,
                                   False => case-sensitive */
                const char *sk) /* set of characters to skip */
{
    char s, p;
    /*
     *  Simple pattern matcher:  '*' matches 0 or more characters, '?' matches
     *  any single character.  Returns TRUE if 'strng' matches 'patrn'.
     */
 pmatch_top:
    if (!sk) {
        s = *strng++;
        p = *patrn++; /* get next chars and pre-advance */
    } else {
        /* fuzzy match variant of pmatch; particular characters are ignored */
        do {
            s = *strng++;
        } while (strchr(sk, s));
        do {
            p = *patrn++;
        } while (strchr(sk, p));
    }
    if (!p)                           /* end of pattern */
        return (boolean) (s == '\0'); /* matches iff end of string too */
    else if (p == '*')                /* wildcard reached */
        return (boolean) ((!*patrn
                           || pmatch_internal(patrn, strng - 1, ci, sk))
                          ? TRUE
                          : s ? pmatch_internal(patrn - 1, strng, ci, sk)
                              : FALSE);
    else if ((ci ? lowc(p) != lowc(s) : p != s) /* check single character */
             && (p != '?' || !s))               /* & single-char wildcard */
        return FALSE;                           /* doesn't match */
    else                 /* return pmatch_internal(patrn, strng, ci, sk); */
        goto pmatch_top; /* optimize tail recursion */
}

/*strutil.c*/
