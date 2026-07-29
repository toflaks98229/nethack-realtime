/*Written by Timothy B. Terriberry (tterribe@xiph.org) 1999-2009
  CC0 (Public domain) - see http://creativecommons.org/publicdomain/zero/1.0/
  for details.
  Based on the public domain ISAAC implementation by Robert J. Jenkins Jr.*/

/**
 * @file isaac64.c
 * @brief ISAAC64 암호학적 유사 난수 생성기(CSPRNG) 구현.
 *
 * 64비트 ISAAC 알고리즘의 공개 도메인 구현으로, NetHack 의 결정적 난수원으로
 * 사용된다. @c USE_ISAAC64 가 정의된 빌드에서만 컴파일된다.
 *
 * @note 원저작자의 공개 도메인(CC0) 코드이며, 상류 동기화를 고려하여 선언
 *       순서는 재배치하지 않고 문서화만 추가한다.
 */

/*
 * Changes for NetHack:
 *      include config.h;
 *      skip rest of file if USE_ISAAC64 isn't defined there;
 *      re-do 'inline' handling.
 */
#include "config.h"

#ifdef USE_ISAAC64
#include <string.h>
#include "isaac64.h"

#define ISAAC64_MASK ((uint64_t)0xFFFFFFFFFFFFFFFFULL)

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#if !defined(HAS_INLINE)
#define HAS_INLINE
#endif
#else
# if (defined(__GNUC__) && __GNUC__ >= 2 && !defined(inline))
# define inline __inline__
# endif
#endif
#if !defined(HAS_INLINE) && !defined(inline)
#define inline /*empty*/
#endif

static inline uint32_t lower_bits(uint64_t);
static inline uint32_t upper_bits(uint64_t);
staticfn void isaac64_update(isaac64_ctx *);
staticfn void isaac64_mix(uint64_t[8]);

/**
 * @brief 상태 배열 인덱싱을 위해 하위 비트 필드를 추출한다.
 * @param[in] x 비트를 추출할 64비트 값.
 * @return 비트 3부터 시작하는 @c ISAAC64_SZ_LOG 비트로 만든 인덱스.
 */
/* Extract ISAAC64_SZ_LOG bits (starting at bit 3). */
static inline uint32_t lower_bits(uint64_t x)
{
    return (x & ((ISAAC64_SZ-1) << 3)) >>3;
}

/**
 * @brief 상태 배열 인덱싱을 위해 상위 비트 필드를 추출한다.
 * @param[in] y 비트를 추출할 64비트 값.
 * @return 비트 @c ISAAC64_SZ_LOG+3 부터의 @c ISAAC64_SZ_LOG 비트로 만든 인덱스.
 */
/* Extract next ISAAC64_SZ_LOG bits (starting at bit ISAAC64_SZ_LOG+2). */
static inline uint32_t upper_bits(uint64_t y)
{
    return (y >> (ISAAC64_SZ_LOG+3)) & (ISAAC64_SZ-1);
}

/**
 * @brief ISAAC64 내부 상태를 갱신하여 새로운 난수 블록을 생성한다.
 * @param[in,out] _ctx 갱신할 ISAAC64 컨텍스트. 결과 배열 @c r 이 다시 채워진다.
 */
staticfn void isaac64_update(isaac64_ctx *_ctx){
  uint64_t *m;
  uint64_t *r;
  uint64_t  a;
  uint64_t  b;
  uint64_t  x;
  uint64_t  y;
  int       i;
  m=_ctx->m;
  r=_ctx->r;
  a=_ctx->a;
  b=_ctx->b+(++_ctx->c);
  for(i=0;i<ISAAC64_SZ/2;i++){
    x=m[i];
    a=~(a^a<<21)+m[i+ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a>>5)+m[i+ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a<<12)+m[i+ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a>>33)+m[i+ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
  }
  for(i=ISAAC64_SZ/2;i<ISAAC64_SZ;i++){
    x=m[i];
    a=~(a^a<<21)+m[i-ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a>>5)+m[i-ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a<<12)+m[i-ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
    x=m[++i];
    a=(a^a>>33)+m[i-ISAAC64_SZ/2];
    m[i]=y=m[lower_bits(x)]+a+b;
    r[i]=b=m[upper_bits(y)]+x;
  }
  _ctx->b=b;
  _ctx->a=a;
  _ctx->n=ISAAC64_SZ;
}

/**
 * @brief 8개 64비트 값을 서로 섞어 시드 초기화 시 확산을 수행한다.
 * @param[in,out] _x 섞을 8원소 배열. 제자리에서 변형된다.
 */
staticfn void isaac64_mix(uint64_t _x[8]){
  static const unsigned char SHIFT[8]={9,9,23,15,14,20,17,14};
  int i;
  for(i=0;i<8;i++){
    _x[i]-=_x[(i+4)&7];
    _x[(i+5)&7]^=_x[(i+7)&7]>>SHIFT[i];
    _x[(i+7)&7]+=_x[i];
    i++;
    _x[i]-=_x[(i+4)&7];
    _x[(i+5)&7]^=_x[(i+7)&7]<<SHIFT[i];
    _x[(i+7)&7]+=_x[i];
  }
}


/**
 * @brief ISAAC64 컨텍스트를 초기화하고 주어진 시드로 준비한다.
 * @param[out] _ctx   초기화할 컨텍스트.
 * @param[in]  _seed  시드 바이트 배열.
 * @param[in]  _nseed 시드 바이트 길이.
 */
void isaac64_init(isaac64_ctx *_ctx,const unsigned char *_seed,int _nseed){
  _ctx->a=_ctx->b=_ctx->c=0;
  memset(_ctx->r,0,sizeof(_ctx->r));
  isaac64_reseed(_ctx,_seed,_nseed);
}

/**
 * @brief 기존 컨텍스트를 새 시드로 다시 초기화(reseed)한다.
 * @param[in,out] _ctx   재시드할 컨텍스트.
 * @param[in]     _seed  시드 바이트 배열.
 * @param[in]     _nseed 시드 바이트 길이(@c ISAAC64_SEED_SZ_MAX 로 상한 처리).
 */
void isaac64_reseed(isaac64_ctx *_ctx,const unsigned char *_seed,int _nseed){
  uint64_t *m;
  uint64_t *r;
  uint64_t  x[8];
  int       i;
  int       j;
  m=_ctx->m;
  r=_ctx->r;
  if(_nseed>ISAAC64_SEED_SZ_MAX)_nseed=ISAAC64_SEED_SZ_MAX;
  for(i=0;i<_nseed>>3;i++){
    r[i]^=(uint64_t)_seed[i<<3|7]<<56|(uint64_t)_seed[i<<3|6]<<48|
     (uint64_t)_seed[i<<3|5]<<40|(uint64_t)_seed[i<<3|4]<<32|
     (uint64_t)_seed[i<<3|3]<<24|(uint64_t)_seed[i<<3|2]<<16|
     (uint64_t)_seed[i<<3|1]<<8|_seed[i<<3];
  }
  _nseed-=i<<3;
  if(_nseed>0){
    uint64_t ri;
    ri=_seed[i<<3];
    for(j=1;j<_nseed;j++)ri|=(uint64_t)_seed[i<<3|j]<<(j<<3);
    r[i++]^=ri;
  }
  x[0]=x[1]=x[2]=x[3]=x[4]=x[5]=x[6]=x[7]=(uint64_t)0x9E3779B97F4A7C13ULL;
  for(i=0;i<4;i++)isaac64_mix(x);
  for(i=0;i<ISAAC64_SZ;i+=8){
    for(j=0;j<8;j++)x[j]+=r[i+j];
    isaac64_mix(x);
    memcpy(m+i,x,sizeof(x));
  }
  for(i=0;i<ISAAC64_SZ;i+=8){
    for(j=0;j<8;j++)x[j]+=m[i+j];
    isaac64_mix(x);
    memcpy(m+i,x,sizeof(x));
  }
  isaac64_update(_ctx);
}

/**
 * @brief 다음 64비트 난수를 반환한다.
 * @param[in,out] _ctx 난수를 뽑을 컨텍스트(소진 시 자동으로 재갱신).
 * @return 0 이상 2^64 미만의 균등 난수.
 */
uint64_t isaac64_next_uint64(isaac64_ctx *_ctx){
  if(!_ctx->n)isaac64_update(_ctx);
  return _ctx->r[--_ctx->n];
}

/**
 * @brief 0 이상 @p _n 미만의, 모듈로 편향이 없는 균등 난수를 반환한다.
 * @param[in,out] _ctx 난수를 뽑을 컨텍스트.
 * @param[in]     _n   상한값(양수여야 한다).
 * @return 0 이상 @p _n 미만의 균등 난수.
 * @note 편향 제거를 위해 거부 표본추출(rejection sampling)을 사용한다.
 */
uint64_t isaac64_next_uint(isaac64_ctx *_ctx,uint64_t _n){
  uint64_t r;
  uint64_t v;
  uint64_t d;
  do{
    r=isaac64_next_uint64(_ctx);
    v=r%_n;
    d=r-v;
  }
  while(((d+_n-1)&ISAAC64_MASK)<d);
  return v;
}
#endif /* USE_ISAAC64 */

/*isaac64.c*/
