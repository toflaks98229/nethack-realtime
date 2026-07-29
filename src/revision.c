/* NetHack 5.0	revision.c	$NHDT-Date: 1779927286 2026/05/28 00:14:46 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.1 $ */
/* Copyright (c) Michael Allison, 2026. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file revision.c
 * @brief 세이브 파일 리비전(revision) 증가 처리 및 구조체 업리프트 예제.
 *
 * 오래된 리비전으로 저장된 세이브 파일을 현재 리비전으로 끌어올리기
 * (uplift) 위한 진입 로직을 제공한다. @c DEMO_UPLIFTS 로 감싸인 부분은
 * 구조체 필드 추가 시 업리프트를 구현하는 방법을 보여주는 예제 코드다.
 */

#include "hack.h"

/**
 * @brief 세이브 파일의 리비전 레벨을 확인하여 필요 시 업리프트를 준비한다.
 *
 * 파일 리비전이 0이고 현재 세이브 리비전이 1이면, 복원 루틴이 사용할
 * 업리프트 플래그(@c gu.uplift_needed_rev0_to_rev1)를 설정한다.
 *
 * @param[in]     file_rev_level          세이브 파일에 기록된 리비전 레벨.
 * @param[in]     file_critical_byte_count 임계(critical) 바이트 배열의 길이.
 * @param[in,out] csc                     임계 상태 바이트 배열. 필요 시 마지막
 *                                        바이트를 현재 리비전 레벨로 갱신한다.
 * @return 업리프트가 필요한지 여부.
 * @retval TRUE  리비전 0→1 업리프트가 필요하다.
 * @retval FALSE 업리프트가 필요 없다.
 */
boolean revision_increment(
    int file_rev_level,
    int file_critical_byte_count,
    uchar *csc)
{
    if (file_rev_level == 0 && SAVEFILE_REVISION_LEVEL == 1) {
        /*
         * Revision 1
         * We set a flag gu.uplift_needed_rev0_to_rev1 for use by restore routines.
         *
         */
        gu.uplift_needed_rev0_to_rev1 = 1;
        if (csc[file_critical_byte_count - 1] == 0)
            /* below may not be necessary, or even useful */
            csc[file_critical_byte_count - 1] = SAVEFILE_REVISION_LEVEL;
        return TRUE;
    }
    return FALSE;
}

#ifdef DEMO_UPLIFTS

/* original revision 0 is in include/revision.h */

/** @brief 리비전 1 형태의 예제 구조체(리비전 0 대비 필드가 추가됨). */
/* revision 1 */
struct mystruct {
    int field1;
    int field2;
    char field3;
    long field4;
    int newfielda;
    int newfieldb;
};

/**
 * @brief 리비전 0 구조체를 리비전 1 구조체로 업리프트하는 예제.
 *
 * 기존 필드는 그대로 복사하고, 새로 추가된 필드는 기본값으로 채운다.
 *
 * @param[in]  rev0 원본(리비전 0) 구조체.
 * @param[out] rev1 대상(리비전 1) 구조체.
 */
void
uplift_mystruct_rev0_to_mystruct(struct mystruct_rev0 *rev0,
                                 struct mystruct *rev1)
{
    rev1->field1 = rev0->field1;
    rev1->field2 = rev0->field2;
    rev1->field3 = rev0->field3;
    rev1->field4 = rev0->field4;
    rev1->newfielda = 0; // new field
    rev1->newfieldb = 0; // new field
}

/**
 * @brief 세이브 파일에서 예제 구조체를 읽고 필요 시 업리프트한다.
 *
 * 현재 세이브 리비전에 맞춰 구조체를 읽으며, 업리프트가 필요하면 리비전 0
 * 형태로 읽은 뒤 리비전 1로 변환하고 새 필드에 시작값을 채운다.
 *
 * @param[out] ms 읽어들인 값을 저장할 구조체.
 */
void
read_mystruct(struct mystruct *ms)
{
#ifdef SAVEFILE_REVISION_LEVEL

#if (SAVEFILE_REVISION_LEVEL == 0)
    Sfi_mystruct(nhfp, ms, "mystruct-example");
#elif (SAVEFILE_REVISION_LEVEL == 1)

#define MYSTRUCT_REV0
#include "revision.h"
#undef MYSTRUCT_REV0

    if (!gu.uplift_needed_rev0_to_rev1) {
        Sfi_mystruct(nhfp, ms, "mystruct-example");
    } else {
        struct mystruct_rev0 old_mystruct;

        Sfi_mystruct_rev0(nhfp, &old_mystruct, "mystruct-example");
        uplift_mystruct_rev0_to_mystruct(&old_mystruct, ms);
    }

    /* this could be elsewhere in some other sourcefile for example,
     * where it is needed */
    if (gu.uplift_needed_rev0_to_rev1 == 1) {
        /* Provide suitable starting values for fields that were not
         * present in the previous revision that was read */
        ms->newfielda = 0;  // new field in Rev. 1
        ms->newfieldb = 42; // new field in Rev. 1
    }
#endif
#endif /* SAVEFILE_REVISION_LEVEL */
}

#endif  /* DEMO_UPLIFTS */

/*revision.c*/
