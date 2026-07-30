/* NetHack 5.0	nhregex.h	$NHDT-Date: $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: $ */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file nhregex.h
 * @brief Pattern matching, behind an interface that hides which library
 *        provides it.
 *
 * The player writes patterns in configuration -- for menu colouring and message
 * types -- but which regular-expression implementation is available depends on
 * the platform. Several exist in the tree, and this is the interface all of them
 * present, so nothing that matches patterns needs to know which was compiled in.
 *
 * @note @c struct nhregex is deliberately opaque; its contents differ per
 *       implementation, which is the whole point of the indirection.
 * @warning Compiling can fail on a pattern the player wrote, so a compile is a
 *          question to be answered rather than a step to be assumed.
 */

/**
 * @file nhregex.h
 * @brief 어떤 라이브러리가 제공하는지를 감춘 인터페이스 뒤의 패턴 매칭.
 *
 * 플레이어는 설정에 패턴을 쓴다. 메뉴 색칠과 메시지 유형에 쓰인다. 그러나 어떤 정규식
 * 구현을 쓸 수 있는지는 플랫폼에 달려 있다. 트리 안에 여러 구현이 있으며, 이것이 그
 * 모두가 제시하는 인터페이스다. 그래서 패턴을 대조하는 어떤 코드도 무엇이 컴파일되어
 * 들어갔는지 알 필요가 없다.
 *
 * @note @c struct nhregex 는 의도적으로 불투명하다. 내용이 구현마다 다르며, 그것이
 *       이 간접화의 전부다.
 * @warning 컴파일은 플레이어가 쓴 패턴에 대해 실패할 수 있다. 그래서 컴파일은 당연히
 *          지나가는 단계가 아니라 답을 확인해야 하는 질문이다.
 */

#ifndef NHREGEX_H
#define NHREGEX_H

/* ### {cpp,pmatch,posix}regex.c ### */

/**
 * @brief Allocate an unused pattern object.
 * @return A pattern object, or NULL if one could not be allocated.
 * @note Must be released with @c regex_free() whether or not a pattern was
 *       ever compiled into it.
 */
/**
 * @brief 아직 쓰이지 않은 패턴 객체를 할당한다.
 * @return 패턴 객체, 할당하지 못하면 NULL.
 * @note 패턴을 컴파일해 넣었는지와 무관하게 @c regex_free() 로 해제해야 한다.
 */
extern struct nhregex *regex_init(void);

/**
 * @brief Compile a pattern into a pattern object.
 * @param[in]     pattern Pattern text, as the player wrote it.
 * @param[in,out] re      Object to compile into.
 * @retval TRUE  The pattern compiled and may be matched against.
 * @retval FALSE It did not; ask @c regex_error_desc() why.
 */
/**
 * @brief 패턴을 패턴 객체로 컴파일한다.
 * @param[in]     pattern 플레이어가 쓴 그대로의 패턴 문구.
 * @param[in,out] re      컴파일해 넣을 객체.
 * @retval TRUE  컴파일되었고 대조에 쓸 수 있다.
 * @retval FALSE 그렇지 못했다. 이유는 @c regex_error_desc() 에 물을 것.
 */
extern boolean regex_compile(const char *, struct nhregex *) NONNULLARG1;

/**
 * @brief Describe why a compile failed.
 * @param[in]  re     Object whose compile failed.
 * @param[out] outbuf Buffer to write the description into.
 * @return @p outbuf.
 * @warning The caller supplies the buffer and must make it large enough; the
 *          text comes from the underlying library and is not bounded here.
 */
/**
 * @brief 컴파일이 왜 실패했는지 설명한다.
 * @param[in]  re     컴파일이 실패한 객체.
 * @param[out] outbuf 설명을 쓸 버퍼.
 * @return @p outbuf.
 * @warning 버퍼는 호출자가 제공하며 충분히 크게 잡아야 한다. 문구는 하위 라이브러리에서
 *          오고 여기서 길이를 제한하지 않는다.
 */
extern char *regex_error_desc(struct nhregex *, char *) NONNULLARG2;

/**
 * @brief Test a string against a compiled pattern.
 * @param[in] s  String to test.
 * @param[in] re Compiled pattern.
 * @retval TRUE  The string matches.
 * @retval FALSE It does not, or there was nothing to match against.
 * @note Accepts a null pattern and answers FALSE, so callers need not check
 *       whether a compile succeeded before every match.
 */
/**
 * @brief 문자열을 컴파일된 패턴과 대조한다.
 * @param[in] s  대조할 문자열.
 * @param[in] re 컴파일된 패턴.
 * @retval TRUE  일치한다.
 * @retval FALSE 일치하지 않거나, 대조할 대상이 없다.
 * @note 널 패턴을 받아 FALSE 로 답한다. 그래서 호출자가 대조마다 컴파일 성공 여부를
 *       확인하지 않아도 된다.
 */
extern boolean regex_match(const char *, struct nhregex *) NO_NNARGS;

/**
 * @brief Release a pattern object.
 * @param[in,out] re Object to release.
 */
/**
 * @brief 패턴 객체를 해제한다.
 * @param[in,out] re 해제할 객체.
 */
extern void regex_free(struct nhregex *) NONNULLARG1;

#endif /* NHREGEX_H */

/*extern.h*/
