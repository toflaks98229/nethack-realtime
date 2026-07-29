/* NetHack 5.0	calendar.c	$NHDT-Date: 1781973042 2026/06/20 16:30:42 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.3 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2007. */
/* Copyright (c) Robert Patrick Rankin, 1991                      */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file calendar.c
 * @brief 실제 시각·날짜 관련 유틸리티(시간 조회, 달의 위상 등).
 *
 * 현지 시각/날짜를 조회하고, 묘비의 연도, 기록 파일의 날짜, 달의 위상(달 관련
 * 몬스터 반응), 밤/자정 여부 등 게임에서 필요로 하는 시간 정보를 계산한다.
 */

#include "hack.h"

/*
 * Time routines
 *
 * The time is used for:
 *  - seed for rand()
 *  - year on tombstone and yyyymmdd in record file
 *  - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *  - night and midnight (the undead are dangerous at midnight)
 *  - determination of what files are "very old"
 */

/* TIME_type: type of the argument to time(); we actually use &(time_t);
   you might need to define either or both of these to 'long *' in *conf.h */
#ifndef TIME_type
#define TIME_type time_t *
#endif
#ifndef LOCALTIME_type
#define LOCALTIME_type time_t *
#endif

staticfn struct tm *getlt(void);

/**
 * @brief 현재 시각을 @c time_t 값으로 반환한다.
 * @return 현재 달력 시각.
 */
time_t
getnow(void)
{
    time_t datetime = 0;

    (void) time((TIME_type) &datetime);
    return datetime;
}

/**
 * @brief 현재 연도(서기)를 반환한다.
 * @return 서기 연도(예: 2026).
 */
int
getyear(void)
{
    return (1900 + getlt()->tm_year);
}


/**
 * @brief 주어진 시각을 YYYYMMDD 형식의 정수로 변환한다.
 * @param[in] date 변환할 시각. 0이면 현재 현지 시각을 사용한다.
 * @return YYYYMMDD 형식의 날짜 값(예: 20260724).
 */
long
yyyymmdd(time_t date)
{
    long datenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    /* yyyy --> yyyymm */
    datenum = datenum * 100L + (long) (lt->tm_mon + 1);
    /* yyyymm --> yyyymmdd */
    datenum = datenum * 100L + (long) lt->tm_mday;
    return datenum;
}

/**
 * @brief 주어진 시각을 HHMMSS 형식의 정수로 변환한다.
 * @param[in] date 변환할 시각. 0이면 현재 현지 시각을 사용한다.
 * @return HHMMSS 형식의 시각 값(예: 143005).
 */
long
hhmmss(time_t date)
{
    long timenum;
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    timenum = lt->tm_hour * 10000L + lt->tm_min * 100L + lt->tm_sec;
    return timenum;
}

/**
 * @brief 주어진 시각을 "YYYYMMDDHHMMSS" 문자열로 변환한다.
 * @param[in] date 변환할 시각. 0이면 현재 현지 시각을 사용한다.
 * @return 14자리 날짜/시각 문자열.
 * @warning 정적 버퍼를 반환하므로 스레드 안전하지 않으며, 다음 호출 시 덮어써진다.
 */
char *
yyyymmddhhmmss(time_t date)
{
    long datenum;
    static char datestr[15];
    struct tm *lt;

    if (date == 0)
        lt = getlt();
    else
        lt = localtime((LOCALTIME_type) &date);

    /* just in case somebody's localtime supplies (year % 100)
       rather than the expected (year - 1900) */
    if (lt->tm_year < 70)
        datenum = (long) lt->tm_year + 2000L;
    else
        datenum = (long) lt->tm_year + 1900L;
    Snprintf(datestr, sizeof datestr, "%04ld%02d%02d%02d%02d%02d",
             datenum, lt->tm_mon + 1,
             lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    //debugpline1("yyyymmddhhmmss() produced date string %s", datestr);
    return datestr;
}

/**
 * @brief "YYYYMMDDHHMMSS" 문자열을 @c time_t 값으로 변환한다.
 * @param[in] buf 14자리 날짜/시각 문자열.
 * @return 변환된 시각. 형식이 올바르지 않거나 변환에 실패하면 0.
 */
time_t
time_from_yyyymmddhhmmss(char *buf)
{
    int k;
    time_t timeresult = (time_t) 0;
    struct tm t, *lt;
    char *d, *p, y[5], mo[3], md[3], h[3], mi[3], s[3];

    if (buf && strlen(buf) == 14) {
        d = buf;
        p = y; /* year */
        for (k = 0; k < 4; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mo; /* month */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = md; /* day */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = h; /* hour */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = mi; /* minutes */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        p = s; /* seconds */
        for (k = 0; k < 2; ++k)
            *p++ = *d++;
        *p = '\0';
        lt = getlt();
        if (lt) {
            t = *lt;
            t.tm_year = atoi(y) - 1900;
            t.tm_mon = atoi(mo) - 1;
            t.tm_mday = atoi(md);
            t.tm_hour = atoi(h);
            t.tm_min = atoi(mi);
            t.tm_sec = atoi(s);
            timeresult = mktime(&t);
        }
        if (timeresult == (time_t) -1)
            ;
#if 0
TODO: set_debugpline1, debugpline1 -> function pointer
            debugpline1("time_from_yyyymmddhhmmss(%s) would have returned -1",
                        buf ? buf : "");
#endif
        else
            return timeresult;
    }
    return (time_t) 0;
}

/*
 * moon period = 29.53058 days ~= 30, year = 365.2422 days
 * days moon phase advances on first day of year compared to preceding year
 *      = 365.2422 - 12*29.53058 ~= 11
 * years in Metonic cycle (time until same phases fall on the same days of
 *      the month) = 18.6 ~= 19
 * moon phase on first day of year (epact) ~= (11*(year%19) + 29) % 30
 *      (29 as initial condition)
 * current phase in days = first day phase + days elapsed in year
 * 6 moons ~= 177 days
 * 177 ~= 8 reported phases * 22
 * + 11/22 for rounding
 */
/**
 * @brief 오늘의 달의 위상(phase)을 계산한다.
 *
 * 메톤 주기(Metonic cycle)와 에팩트(epact)를 이용한 근사 계산이다.
 *
 * @return 0~7 사이의 위상 값(0: 신월/new, 4: 보름/full).
 */
int
phase_of_the_moon(void) /* 0-7, with 0: new, 4: full */
{
    struct tm *lt = getlt();
    int epact, diy, goldn;

    diy = lt->tm_yday;
    goldn = (lt->tm_year % 19) + 1;
    epact = (11 * goldn + 18) % 30;
    if ((epact == 25 && goldn > 11) || epact == 24)
        epact++;

    return ((((((diy + epact) * 6) + 11) % 177) / 22) & 7);
}

/**
 * @brief 오늘이 13일의 금요일인지 판별한다.
 * @return 13일의 금요일이면 TRUE, 아니면 FALSE.
 */
boolean
friday_13th(void)
{
    struct tm *lt = getlt();

    /* tm_wday (day of week; 0==Sunday) == 5 => Friday */
    return (boolean) (lt->tm_wday == 5 && lt->tm_mday == 13);
}

/**
 * @brief 현재 시각이 밤인지 판별한다.
 * @return 밤(22시~5시)이면 참(0이 아님), 아니면 0.
 */
int
night(void)
{
    int hour = getlt()->tm_hour;

    return (hour < 6 || hour > 21);
}

/**
 * @brief 현재 시각이 자정(0시대)인지 판별한다.
 * @return 자정이면 참(0이 아님), 아니면 0.
 */
int
midnight(void)
{
    return (getlt()->tm_hour == 0);
}

/**
 * @brief 현재 현지 시각을 분해한 @c tm 구조체 포인터를 반환한다.
 * @return 현지 시각을 나타내는 정적 @c tm 구조체 포인터.
 * @warning 표준 라이브러리의 정적 버퍼를 반환하므로 스레드 안전하지 않다.
 */
staticfn struct tm *
getlt(void)
{
    time_t date = getnow();

    return localtime((LOCALTIME_type) &date);
}

/* calendar.c */

