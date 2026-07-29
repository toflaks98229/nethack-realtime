/* NetHack 5.0	rnd.c	$NHDT-Date: 1781973065 2026/06/20 16:31:05 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.41 $ */
/*      Copyright (c) 2004 by Robert Patrick Rankin               */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file rnd.c
 * @brief 게임 전반에서 사용하는 난수 생성 함수들의 모음.
 *
 * @c rn2, @c rnd, @c rnl, @c d 등 게임 로직의 확률/주사위 계산에 쓰이는
 * 핵심 난수 함수와, 게임플레이에 영향을 주지 않는 표시(display) 전용
 * 별도 난수열을 제공한다.
 *
 * 저수준 난수원은 빌드 설정에 따라 달라진다.
 * - @c USE_ISAAC64 정의 시: ISAAC64 알고리즘 기반 결정적 난수원 사용.
 * - 미정의 시: 플랫폼별 @c Rand()(srand/srandom/srand48 계열) 사용.
 *
 * @note 이 파일의 선언 순서는 상당 부분 조건부 컴파일(@c #ifdef)에 의해
 *       규정되므로, static 헬퍼를 파일 끝으로 옮기는 재배치는 적용하지 않는다.
 */

#include "hack.h"

#ifdef USE_ISAAC64
#include "isaac64.h"

staticfn int whichrng(int (*fn)(int));
staticfn int RND(int);
staticfn void set_random(unsigned long, int (*)(int));

#if 0
static isaac64_ctx rng_state;
#endif

/** @brief 하나의 난수열(RNG 인스턴스)을 기술하는 항목. */
struct rnglist_t {
    int (*fn)(int);       /**< 이 난수열을 대표하는 함수 포인터. */
    boolean init;         /**< 초기화 완료 여부. */
    isaac64_ctx rng_state; /**< ISAAC64 내부 상태. */
};

/** @brief 난수열 종류 인덱스: CORE=게임 로직용, DISP=표시 전용. */
enum { CORE = 0, DISP = 1 };

/** @brief 사용 가능한 난수열들의 테이블(@c CORE, @c DISP 순). */
static struct rnglist_t rnglist[] = {
    { rn2, FALSE, { 0 } },                      /* CORE */
    { rn2_on_display_rng, FALSE, { 0 } },       /* DISP */
};

/**
 * @brief 함수 포인터로 @c rnglist 내 해당 난수열의 인덱스를 찾는다.
 *
 * @param[in] fn 찾고자 하는 난수열의 대표 함수 포인터.
 * @return @c rnglist 배열에서의 인덱스.
 * @retval -1 일치하는 난수열이 없을 경우.
 */
staticfn int
whichrng(int (*fn)(int))
{
    int i;

    for (i = 0; i < SIZE(rnglist); ++i)
        if (rnglist[i].fn == fn)
            return i;
    return -1;
}

/**
 * @brief 지정한 난수열의 ISAAC64 상태를 주어진 시드로 초기화한다.
 *
 * @param[in] seed 시드 값. 바이트 단위로 분해되어 ISAAC64 초기화에 사용된다.
 * @param[in] fn   초기화할 난수열의 대표 함수 포인터(@c rn2 또는
 *                 @c rn2_on_display_rng).
 * @warning @p fn 이 등록된 난수열이 아니면 @c panic() 으로 프로그램을 중단한다.
 */
void
init_isaac64(unsigned long seed, int (*fn)(int))
{
    unsigned char new_rng_state[sizeof seed];
    unsigned i;
    int rngindx = whichrng(fn);

    if (rngindx < 0)
        panic("Bad rng function passed to init_isaac64().");

    for (i = 0; i < sizeof seed; i++) {
        new_rng_state[i] = (unsigned char) (seed & 0xFF);
        seed >>= 8;
    }
    isaac64_init(&rnglist[rngindx].rng_state, new_rng_state,
                 (int) sizeof seed);
}

/**
 * @brief CORE 난수열에서 0 이상 @p x 미만의 정수를 뽑는 내부 헬퍼.
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 0 이상 @p x 미만의 난수.
 */
staticfn int
RND(int x)
{
    return (isaac64_next_uint64(&rnglist[CORE].rng_state) % x);
}

/**
 * @brief 표시(display) 전용 난수열에서 0 이상 @p x 미만의 정수를 뽑는다.
 *
 * "메인" @c rn2 와는 다른 난수열을 사용한다. 결과가 게임플레이에 영향을
 * 주지 않고, 플레이어가 메인 난수열을 손쉽게 조작하지 못하게 하려는
 * 경우에 사용한다.
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 0 이상 @p x 미만의 난수.
 */
int
rn2_on_display_rng(int x)
{
    return (isaac64_next_uint64(&rnglist[DISP].rng_state) % x);
}

#else   /* USE_ISAAC64 */

/* "Rand()"s definition is determined by [OS]conf.h */
/** @brief 비-ISAAC64 빌드에서 0 이상 @c x 미만 정수를 뽑는 매크로. */
#if defined(UNIX) || defined(RANDOM)
#define RND(x) ((int) (Rand() % (long) (x)))
#else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x) ((int) ((Rand() >> 3) % (x)))
#endif
/**
 * @brief 표시(display) 전용 난수열에서 0 이상 @p x 미만의 정수를 뽑는다.
 *
 * 비-ISAAC64 빌드용 구현으로, 자체 선형 합동 시드를 사용한다.
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 0 이상 @p x 미만의 난수.
 */
int
rn2_on_display_rng(int x)
{
    static unsigned seed = 1;
    seed *= 2739110765;
    return (int) ((seed >> 16) % (unsigned) x);
}
#endif  /* USE_ISAAC64 */

/**
 * @brief 0 이상 @p x 미만의 난수를 반환한다(핵심 난수 함수).
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 0 이상 @p x 미만의 난수.
 * @warning 개발 빌드에서 @p x 가 0 이하이면 @c impossible() 경고를 내고 0을
 *          반환한다(릴리스 빌드에서는 검사하지 않음).
 */
int
rn2(int x)
{
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    if (x <= 0) {
        impossible("rn2(%d) attempted", x);
        return 0;
    }
    x = RND(x);
    return x;
#else
    return RND(x);
#endif
}

/**
 * @brief 행운(Luck)이 반영된, 0 이상 @p x 미만의 난수를 반환한다.
 *
 * 행운이 좋을수록 결과가 0에 가까워지고, 나쁠수록 (x-1)에 가까워진다.
 * 작은 범위(@p x <= 15)에서는 행운 보정을 완화하여 적용한다.
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 0 이상 @p x 미만의, 행운이 보정된 난수.
 * @warning 개발 빌드에서 @p x 가 0 이하이면 @c impossible() 경고를 내고 0을
 *          반환한다.
 */
int
rnl(int x)
{
    int i, adjustment;

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    if (x <= 0) {
        impossible("rnl(%d) attempted", x);
        return 0;
    }
#endif

    adjustment = Luck;
    if (x <= 15) {
        /* for small ranges, use Luck/3 (rounded away from 0);
           also guard against architecture-specific differences
           of integer division involving negative values */
        adjustment = (abs(adjustment) + 1) / 3 * sgn(adjustment);
        /*
         *       11..13 ->  4
         *        8..10 ->  3
         *        5.. 7 ->  2
         *        2.. 4 ->  1
         *       -1,0,1 ->  0 (no adjustment)
         *       -4..-2 -> -1
         *       -7..-5 -> -2
         *      -10..-8 -> -3
         *      -13..-11-> -4
         */
    }

    i = RND(x);
    if (adjustment && rn2(37 + abs(adjustment))) {
        i -= adjustment;
        if (i < 0)
            i = 0;
        else if (i >= x)
            i = x - 1;
    }
    return i;
}

/**
 * @brief 1 이상 @p x 이하의 난수를 반환한다(주사위 한 번 굴리기).
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 1 이상 @p x 이하의 난수.
 * @warning 개발 빌드에서 @p x 가 0 이하이면 @c impossible() 경고를 내고 1을
 *          반환한다.
 */
int
rnd(int x)
{
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    if (x <= 0) {
        impossible("rnd(%d) attempted", x);
        return 1;
    }
#endif
    x = RND(x) + 1;
    return x;
}

/**
 * @brief 표시(display) 전용 난수열에서 1 이상 @p x 이하의 난수를 반환한다.
 *
 * @param[in] x 상한값(양수여야 한다).
 * @return 1 이상 @p x 이하의 난수.
 */
int
rnd_on_display_rng(int x)
{
    return rn2_on_display_rng(x) + 1;
}

/**
 * @brief NdX 주사위 굴림을 계산한다: dX 를 @p n 번 굴려 합산한다.
 *
 * 결과 범위는 @p n 이상 (@p n * @p x) 이하이다.
 *
 * @param[in] n 주사위 개수.
 * @param[in] x 각 주사위의 면 수.
 * @return @p n 개의 dX 합.
 * @warning 개발 빌드에서 잘못된 인자(@p x<0, @p n<0, 또는 x==0 && n!=0)이면
 *          @c impossible() 경고를 내고 1을 반환한다.
 */
int
d(int n, int x)
{
    int tmp = n;

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    if (x < 0 || n < 0 || (x == 0 && n != 0)) {
        impossible("d(%d,%d) attempted", n, x);
        return 1;
    }
#endif
    while (n--)
        tmp += RND(x);
    return tmp; /* Alea iacta est. -- J.C. */
}

/**
 * @brief 지수적으로 감소하는 분포의 난수를 반환한다.
 *
 * 1 이상 max(@c u.ulevel/3, 5) 이하의 값을 돌려주며, 값이 커질수록
 * 나올 확률이 급격히 낮아진다.
 *
 * @param[in] x 값이 1 증가할 확률의 역수(클수록 증가가 어려움).
 * @return 1 이상 max(@c u.ulevel/3, 5) 이하의 난수.
 */
int
rne(int x)
{
    int tmp, utmp;

    utmp = (u.ulevel < 15) ? 5 : u.ulevel / 3;
    tmp = 1;
    while (tmp < utmp && !rn2(x))
        tmp++;
    return tmp;

    /* was:
     *  tmp = 1;
     *  while (!rn2(x))
     *    tmp++;
     *  return min(tmp, (u.ulevel < 15) ? 5 : u.ulevel / 3);
     * which is clearer but less efficient and stands a vanishingly
     * small chance of overflowing tmp
     */
}

/**
 * @brief 넓은 범위에 걸쳐 요동치는 난수를 반환한다.
 *
 * 입력값 @p i 를 중심으로 곱하거나 나누어 크게 확대/축소된 값을 만든다.
 * 시간 지연 등 값의 편차가 커도 무방한 곳에 사용된다.
 *
 * @param[in] i 기준값.
 * @return @p i 를 기준으로 무작위하게 확대 또는 축소된 값.
 */
int
rnz(int i)
{
    long x = (long) i;
    long tmp = 1000L;

    tmp += rn2(1000);
    tmp *= rne(4);
    if (rn2(2)) {
        x *= tmp;
        x /= 1000;
    } else {
        x *= 1000;
        x /= tmp;
    }
    return (int) x;
}

/* Sets the seed for the random number generator */
#ifdef USE_ISAAC64

/**
 * @brief 난수 생성기의 시드를 설정하는 내부 헬퍼(ISAAC64 빌드).
 *
 * @param[in] seed 시드 값.
 * @param[in] fn   시드를 설정할 난수열의 대표 함수 포인터.
 */
staticfn void
set_random(unsigned long seed,
           int (*fn)(int))
{
    init_isaac64(seed, fn);
}

#else /* USE_ISAAC64 */

/**
 * @brief 난수 생성기의 시드를 설정하는 내부 헬퍼(비-ISAAC64 빌드).
 *
 * 플랫폼에 따라 @c srandom/srand48/srand 중 적절한 함수를 호출한다.
 *
 * @param[in] seed 시드 값.
 * @param[in] fn   사용되지 않음(인터페이스 통일을 위한 인자).
 */
/*ARGSUSED*/
staticfn void
set_random(unsigned long seed,
           int (*fn)(int) UNUSED)
{
    /*
     * The types are different enough here that sweeping the different
     * routine names into one via #defines is even more confusing.
     */
# ifdef RANDOM /* srandom() from sys/share/random.c */
    srandom((unsigned int) seed);
# else
#  if defined(__APPLE__) || defined(BSD) || defined(LINUX) \
    || defined(ULTRIX) || defined(CYGWIN32) /* system srandom() */
#   if defined(BSD) && !defined(POSIX_TYPES) && defined(SUNOS4)
    (void)
#   endif
        srandom((int) seed);
#  else
#   ifdef UNIX /* system srand48() */
    srand48((long) seed);
#   else       /* poor quality system routine */
    srand((int) seed);
#   endif
#  endif
# endif
}
#endif /* USE_ISAAC64 */

/* An appropriate version of this must always be provided in
   port-specific code somewhere. It returns a number suitable
   as seed for the random number generator */
extern unsigned long sys_random_seed(void);

/**
 * @brief 난수 생성기를 초기화한다.
 *
 * 포트별 코드가 제공하는 @c sys_random_seed() 로 시드를 얻어 설정한다.
 *
 * @param[in] fn 초기화할 난수열의 대표 함수 포인터.
 * @note 최초 1회만 호출해야 한다.
 */
void
init_random(int (*fn)(int))
{
    set_random(sys_random_seed(), fn);
}

/**
 * @brief 난수 생성기를 다시 시드하여 난수열을 재편성한다.
 *
 * @param[in] fn 재시드할 난수열의 대표 함수 포인터.
 * @note 시드 생성이 플레이어에게 예측 불가능하다고 확신되는 경우
 *       (@c has_strong_rngseed)에만 실제로 재시드한다.
 */
void
reseed_random(int (*fn)(int))
{
   /* only reseed if we are certain that the seed generation is unguessable
    * by the players. */
    if (has_strong_rngseed)
        init_random(fn);
}

/**
 * @brief 정수 배열을 Fisher–Yates 방식으로 무작위 섞는다.
 *
 * @param[in,out] indices 섞을 대상 정수 배열. 제자리에서 재배열된다.
 * @param[in]     count   배열의 원소 개수.
 */
void
shuffle_int_array(int *indices, int count)
{
    int i, iswap, temp;

    for (i = count - 1; i > 0; i--) {
        if ((iswap = rn2(i + 1)) == i)
            continue;
        temp = indices[i];
        indices[i] = indices[iswap];
        indices[iswap] = temp;
    }
}

/*rnd.c*/
