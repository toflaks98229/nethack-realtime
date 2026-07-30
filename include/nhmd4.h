/* NetHack 5.0	nhmd4.h	$NHDT-Date: 1781973084 2026/06/20 16:31:24 $	$NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.2 $ */
/*- Copyright (c) Kenneth Lorber, Kensington, Maryland, 2024 */
/* NetHack may be freely redistributed.  See license for details. */

// Derived from:
/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security,
 * Inc. MD4 Message-Digest Algorithm.
 *
 * Written by Solar Designer <solar@openwall.com> in 2001, and placed in
 * the public domain.  See md4.c for more information.
 */

/**
 * @file nhmd4.h
 * @brief A message digest, used to identify data rather than to secure it.
 *
 * Bundled so the game does not depend on a crypto library being present. It is
 * used for things like recognizing that two files are the same, which is a
 * question of identity, not of secrecy.
 *
 * @warning MD4 is broken for any security purpose and must not be used as
 *          though it were not. It is here because it is small and settled, not
 *          because it is strong.
 * @note The three routines are the usual init/update/final sequence, so data may
 *       be digested in pieces without holding it all at once.
 */

/**
 * @file nhmd4.h
 * @brief 데이터를 보호하기 위해서가 아니라 식별하기 위해 쓰는 메시지 다이제스트.
 *
 * 암호 라이브러리가 있어야만 동작하지 않도록 함께 넣어 두었다. 두 파일이 같은 것인지
 * 알아보는 일 같은 데 쓰이며, 그것은 비밀 유지가 아니라 동일성의 문제다.
 *
 * @warning MD4 는 어떤 보안 목적으로도 깨진 알고리즘이며, 그렇지 않은 것처럼 써서는
 *          안 된다. 여기 있는 것은 강하기 때문이 아니라 작고 변하지 않기 때문이다.
 * @note 세 루틴은 통상적인 init/update/final 순서다. 그래서 데이터를 한꺼번에 쥐지
 *       않고 조각으로 나누어 처리할 수 있다.
 */

#ifndef NHMD4_H
#define NHMD4_H

#define NHMD4_DIGEST_LENGTH 128
#define NHMD4_RESULTLEN (128 / 8) /* 16 */

typedef uint32_t quint32;

struct nhmd4_context {
    quint32 lo, hi;
    quint32 a, b, c, d;
    unsigned char buffer[64];
    quint32 block[NHMD4_RESULTLEN];
};
typedef struct nhmd4_context NHMD4_CTX;

extern void nhmd4_init(NHMD4_CTX *ctx);
extern void nhmd4_update(NHMD4_CTX *, const unsigned char *, size_t);
extern void nhmd4_final(NHMD4_CTX *, unsigned char result[NHMD4_RESULTLEN]);

#endif /* NHMD4_H */

/*nhmd4.h*/

