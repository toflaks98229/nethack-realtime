/* NetHack 5.0	warnings.h	$NHDT-Date: 1781973090 2026/06/20 16:31:30 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.11 $ */
/* Copyright (c) Michael Allison, 2021. */

/**
 * @file warnings.h
 * @brief Turning a specific compiler warning off around specific code.
 *
 * Sometimes code is correct in a way the compiler cannot see -- a format string
 * that is genuinely variable, a conditional that is constant on one platform.
 * Rather than silence such warnings for the whole build, these macros switch one
 * off around the lines that need it and switch it back on afterwards.
 *
 * The pragmas differ per compiler and do not exist at all on some, so every macro
 * has a do-nothing definition. Code may therefore use them unconditionally, which
 * is the point.
 *
 * @warning Each disable must be matched by its restore. An unmatched disable
 *          silences the warning for everything that follows in that translation
 *          unit, which is exactly what this file exists to avoid.
 * @note Suppression can be turned off wholesale with @c DISABLE_WARNING_PRAGMAS,
 *       which is how a build checks what is being hidden.
 */

/**
 * @file warnings.h
 * @brief 특정 코드 주변에서만 특정 컴파일러 경고를 끄기.
 *
 * 어떤 코드는 컴파일러가 볼 수 없는 방식으로 옳다. 진짜로 가변인 형식 문자열이라든지, 어떤
 * 플랫폼에서만 상수가 되는 조건식 같은 것이다. 그런 경고를 빌드 전체에서 잠재우는 대신, 이
 * 매크로들은 필요한 줄 주변에서만 하나를 껐다가 다시 켠다.
 *
 * 프래그마는 컴파일러마다 다르고 일부에는 아예 없으므로, 모든 매크로에 아무것도 하지 않는
 * 정의가 준비되어 있다. 덕분에 코드가 조건 없이 그것을 쓸 수 있으며, 그것이 핵심이다.
 *
 * @warning 끄기는 반드시 짝이 되는 되돌리기와 함께여야 한다. 짝 없는 끄기는 그 번역 단위의
 *          이후 모든 코드에 대해 경고를 잠재우며, 이 파일이 막으려는 것이 바로 그것이다.
 * @note @c DISABLE_WARNING_PRAGMAS 로 억제 자체를 통째로 끌 수 있다. 무엇이 감춰지고
 *       있는지 빌드가 확인하는 방법이다.
 */

#ifndef WARNINGS_H
#define WARNINGS_H

/*
 * If ENABLE_WARNING_PRAGMAS is defined, the checks for various
 * compilers is activated.
 *
 * If a suitable compiler is found, STDC_Pragma_AVAILABLE will be defined.
 * When STDC_Pragma_AVAILABLE is not defined, these are defined as no-ops:
 *     DISABLE_WARNING_UNREACHABLE_CODE
 *     DISABLE_WARNING_CONDEXPR_IS_CONSTANT
 *     DISABLE_WARNING_FORMAT_NONLITERAL
 *       ...
 *     RESTORE_WARNINGS
 *     RESTORE_WARNING_CONDEXPR_IS_CONSTANT
 *     RESTORE_WARNING_FORMAT_NONLITERAL
 *
 */

#if !defined(DISABLE_WARNING_PRAGMAS)
#if defined(__STDC_VERSION__)
#if __STDC_VERSION__ >= 199901L
#define ACTIVATE_WARNING_PRAGMAS
#endif /* __STDC_VERSION >= 199901L */
#endif /* __STDC_VERSION */
#if defined(_MSC_VER)
#ifndef ACTIVATE_WARNING_PRAGMAS
#define ACTIVATE_WARNING_PRAGMAS
#endif
#endif
#if defined(__GNUC__) || defined(__clang__)
#if defined(__cplusplus)
#ifndef ACTIVATE_WARNING_PRAGMAS
#define ACTIVATE_WARNING_PRAGMAS
#endif
#endif /* __cplusplus */
#endif /* __GNUC__ || __clang__ */

#ifdef ACTIVATE_WARNING_PRAGMAS
#if defined(__clang__)
#define DISABLE_WARNING_UNREACHABLE_CODE \
    _Pragma("clang diagnostic push")                                    \
    _Pragma("clang diagnostic ignored \"-Wunreachable-code\"")
#define DISABLE_WARNING_FORMAT_NONLITERAL \
    _Pragma("clang diagnostic push")                                    \
    _Pragma("clang diagnostic ignored \"-Wformat-nonliteral\"")
#define DISABLE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_FORMAT_NONLITERAL _Pragma("clang diagnostic pop")
#define RESTORE_WARNING_UNREACHABLE_CODE _Pragma("clang diagnostic pop")
#define RESTORE_WARNINGS _Pragma("clang diagnostic pop")
#define STDC_Pragma_AVAILABLE

#elif defined(__GNUC__)
/* unlike in clang, -Wunreachable-code does not function in later versions
   of gcc [this may be an issue of requiring -O1 or higher] */
#define DISABLE_WARNING_UNREACHABLE_CODE \
    _Pragma("GCC diagnostic push")                                      \
    _Pragma("GCC diagnostic ignored \"-Wunreachable-code\"")
#define DISABLE_WARNING_FORMAT_NONLITERAL \
    _Pragma("GCC diagnostic push")                                      \
    _Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")
#define DISABLE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_FORMAT_NONLITERAL _Pragma("GCC diagnostic pop")
#define RESTORE_WARNING_UNREACHABLE_CODE _Pragma("GCC diagnostic pop")
#define RESTORE_WARNINGS _Pragma("GCC diagnostic pop")
#define STDC_Pragma_AVAILABLE

#elif defined(_MSC_VER)
#if _MSC_VER > 1916
#define DISABLE_WARNING_UNREACHABLE_CODE \
    _Pragma("warning( push )")                                  \
    _Pragma("warning( disable : 4702 )")
#define DISABLE_WARNING_FORMAT_NONLITERAL \
    _Pragma("warning( push )")                                  \
    _Pragma("warning( disable : 4774 )")
#define DISABLE_WARNING_CONDEXPR_IS_CONSTANT \
    _Pragma("warning( push )")                                  \
    _Pragma("warning( disable : 4127 )")
#define RESTORE_WARNING_CONDEXPR_IS_CONSTANT _Pragma("warning( pop )")
#define RESTORE_WARNING_FORMAT_NONLITERAL _Pragma("warning( pop )")
#define RESTORE_WARNING_UNREACHABLE_CODE _Pragma("warning( pop )")
#define RESTORE_WARNINGS _Pragma("warning( pop )")
#define STDC_Pragma_AVAILABLE
#else  /* Visual Studio prior to 2019 below */
#define DISABLE_WARNING_UNREACHABLE_CODE \
    __pragma(warning(push))                                     \
    __pragma(warning(disable:4702))
#define DISABLE_WARNING_FORMAT_NONLITERAL \
    __pragma(warning(push))                                     \
    __pragma(warning(disable:4774))
#define DISABLE_WARNING_CONDEXPR_IS_CONSTANT \
    __pragma(warning(push))                                     \
    __pragma(warning(disable:4127))
#define RESTORE_WARNING_CONDEXPR_IS_CONSTANT __pragma(warning(pop))
#define RESTORE_WARNING_FORMAT_NONLITERAL __pragma(warning(pop))
#define RESTORE_WARNING_UNREACHABLE_CODE __pragma(warning(pop))
#define RESTORE_WARNINGS  __pragma(warning(pop))
#define STDC_Pragma_AVAILABLE
#endif /* vs2019 or vs2017 */

#endif /* various compiler detections */
#endif /* ACTIVATE_WARNING_PRAGMAS */
#else  /* DISABLE_WARNING_PRAGMAS */
#if defined(STDC_Pragma_AVAILABLE)
#undef STDC_Pragma_AVAILABLE
#endif
#endif /* DISABLE_WARNING_PRAGMAS */

#if !defined(STDC_Pragma_AVAILABLE)
#define DISABLE_WARNING_UNREACHABLE_CODE
#define DISABLE_WARNING_FORMAT_NONLITERAL
#define DISABLE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_CONDEXPR_IS_CONSTANT
#define RESTORE_WARNING_FORMAT_NONLITERAL
#define RESTORE_WARNING_UNREACHABLE_CODE
#define RESTORE_WARNINGS
#endif

#endif /* WARNINGS_H */
