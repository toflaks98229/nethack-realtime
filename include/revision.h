/* NetHack 5.0	revision.h	$NHDT-Date: 1779927286 2026/05/28 00:14:46 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.2 $ */
/* Copyright (c) Michael Allison, 2026. */
/* NetHack may be freely redistributed.  See license for details. */

/**
 * @file revision.h
 * @brief Old shapes of saved structures, kept so that old save files can still be read.
 *
 * When a saved structure changes shape there are two options: declare every existing save file invalid, or keep the old shape around and convert. This file is the second option.
 * It holds a copy of each structure as it used to be, and the game reads an old save with the old declaration and then converts it forward.
 *
 * Which is why the file is almost entirely a procedure. The long comment below is a step-by-step instruction for adding a revision, and it is that long because the change touches
 * six files: the old declaration goes here, a reader for it goes in two places in the save interface, an entry goes in the serializer's table, and the conversion routine goes with
 * the code that owns the structure. Missing any one of them is a link failure or a save that reads as nonsense.
 *
 * The instructions end with the part that matters most and is easiest to get wrong: the conversion must give every field that is new in the current shape a sensible value. A field
 * that did not exist in the old save has no old value, so what it becomes is a decision, and the instruction says to make it deliberately rather than leave it zero by default.
 *
 * @note The file is included repeatedly, once per revision, with a macro naming which one is wanted -- so it is not a header in the usual sense and has no include guard. Including
 *       it without naming a revision is a deliberate error rather than a no-op.
 * @note The only revision present is a demonstration, guarded off and named to make clear it is not real. It exists so the procedure has a worked example to point at.
 * @warning This mechanism is an alternative to invalidating old saves, not an addition to it. A structure changed without either a revision here or an increment of the edit
 *          counter leaves old saves readable and wrong.
 */

/**
 * @file revision.h
 * @brief 저장되는 구조체의 예전 모양들. 예전 저장 파일을 여전히 읽을 수 있도록 보관된다.
 *
 * 저장되는 구조체의 모양이 바뀔 때 선택지는 둘이다. 기존 모든 저장 파일을 무효로 선언하거나, 예전 모양을 남겨 두고 변환하는 것. 이 파일이 두 번째 선택지다. 각 구조체가 예전에 어땠는지의 사본을 담으며, 게임은 예전 저장을 예전 선언으로 읽고 그다음 앞으로 변환한다.
 *
 * 그래서 이 파일은 거의 전부가 절차다. 아래의 긴 주석이 개정을 추가하는 단계별 지침이며, 그렇게 긴 것은 그 변경이 여섯 파일을 건드리기 때문이다. 예전 선언이 여기로 가고, 그것을 읽는 것이 저장 인터페이스의 두 곳으로 가고, 항목 하나가 직렬화기의 표로 가고, 변환
 * 루틴이 그 구조체를 소유한 코드와 함께 간다. 그 중 하나라도 빠뜨리면 링크 실패이거나 헛소리로 읽히는 저장이다.
 *
 * 지침은 가장 중요하고 가장 틀리기 쉬운 부분으로 끝난다. 변환은 현재 모양에서 새로 생긴 모든 필드에 합당한 값을 주어야 한다. 예전 저장에 없던 필드에는 예전 값이 없으므로 그것이 무엇이 되는지는 결정이며, 그 지침은 기본으로 0으로 남겨 두는 대신 의도적으로 정하라고
 * 말한다.
 *
 * @note 이 파일은 개정마다 한 번씩 되풀이해 포함되며, 어느 것을 원하는지 지칭하는 매크로와 함께다. 그래서 통상적인 의미의 헤더가 아니며 포함 보호가 없다. 개정을 지칭하지 않고 포함하는 것은 아무 일도 없는 것이 아니라 의도적인 오류다.
 * @note 존재하는 개정은 시연용 하나뿐이며, 보호로 막혀 있고 실제가 아님이 분명하도록 이름 붙어 있다. 그 절차가 가리킬 실제 예제를 갖도록 존재한다.
 * @warning 이 기제는 예전 저장을 무효로 만드는 것에 대한 대안이며, 그것에 더하는 것이 아니다. 여기의 개정도 편집 계수기의 증가도 없이 바뀐 구조체는 예전 저장을 읽을 수 있으면서 틀린 상태로 남긴다.
 */

/*
 * Supporting revisions to NetHack structs via incremental
 * uplifts, rather than incrementing EDITLEVEL and breaking
 * savefile compatibility.
 *
 * Here is the contrived example struct we'll use for
 * this document:
 *
 * The old revision:           The new revision:
 *
 *       struct mystruct {           struct mystruct {
 *           int field1;                 int field1;
 *           int field2;                 int field2;
 *           char field3;                char field3;
 *           long field4;                long field4;
 *        };                             int newfielda;
 *                                       int newfieldb;
 *                                    };
 *
 * Steps (using mystruct as an example name)
 *
 *  1. Paste an exact copy of the struct declaration as it is/was
 *     in the previous revision into include/revision.h, shrouded by
 *     #if defined(MYSTRUCT_REV0), and tack a '_rev0' suffix onto the
 *     name of the struct. For example,
 *
 *       #if defined(MYSTRUCT_REV0)
 *       struct mystruct_rev0 {
 *           int field1;
 *           int field2;
 *           char field3;
 *           long field4;
 *        };
 *
 *    That goes just ahead of this existing placeholder:
 *
 *       #elif defined(XXX_REV0)
 *
 *  2. Place a comment ahead of the mystruct_rev0 declaration
 *     that explains what has changed, and why the revision
 *     is required.
 *
 *  3. Immediately following the revised struct declaration in the
 *     original header file where it is located, add the following:
 *
 *       #define MYSTRUCT_REV0
 *       #include "revision.h"
 *       #undef MYSTRUCT_REV0
 *
 *  4. In include/savefile.h, add a new prototype for a function
 *     to read the previous revision from the savefile, ideally
 *     immediately following the prototype for the current active
 *     one that hasn't got the '_rev0' suffix:
 *
 *       extern void sfi_mystruct(NHFILE *, struct mystruct *,
 *                                const char *);
 *       extern void sfi_mystruct_rev0(NHFILE *, struct mystruct_rev0  *,
 *                                     const char *);
 *
 *  5. Also in include/savefile.h, in the '#if NH_C < 202300L' block,
 *     add an entry below the one that should already exist for the
 *     struct that hasn't got the '_rev0' suffix:
 *
 *       #define Sfi_mystruct(a,b,c) sfi_rm(a, b, c)
 *       #define Sfi_mystruct_rev0(a, b, c) sfi_mystruct_rev0(a, b, c)
 *
 *  6. Also in include/savefile.h, in the '#define sfi(nhfp, dt, tag)'
 *     generic function definition, add an entry below the one
 *     that should already exist for the struct name that hasn't
 *     got the '_rev0' suffix:
 *
 *       struct mystruct *      : sfi_mystruct,         \
 *       struct mystruct_rev0 * : sfi_mysruct_rev0,     \
 *
 *  7. Also in include/savefile.h, a little further down in the
 *     'Sfi_' macro definitions, add one below the existing
 *     one for the struct without the '_rev0' suffix:
 *
 *       #define Sfi_mystruct(a,b,c) sfi(a, b, c)
 *       #define Sfi_mystruct_rev0(a, b, c) sfi(a, b, c)
 *
 *     Only the input 'Sfi_' is required, because the old revision
 *     will never be written out to a file, only read in from a
 *     a file, so no 'Sfo_' is needed.
 *
 *  8. In include/sfmacros.h, add a new entry below the existing
 *     entry that exists without the '_rev0' suffix:
 *
 *       SF_C(struct, mystruct)
 *       SF_C(struct, mystruct_rev0)
 *
 *  9. In include/sfprocs.h, add a new SF_PROTO_C entry below the
 *     existing entry that already exists without the '_rev0' suffix:
 *
 *       SF_PROTO_C(struct, mystruct);
 *       SF_PROTO_C(struct, mystruct_rev0);
 *
 * 10. Also in include/sfprocs.h, add a new SF_ENTRY_C entry below the
 *     entry that already exists without the '_rev0' suffix:
 *
 *       SF_ENTRY_C(struct, mystruct);
 *       SF_ENTRY_C(struct, mystruct_rev0);
 *
 * 11. In src/sfbase.c, add a prototype for a 'norm_ptrs' stub function
 *     below the entry that already exists without the '_rev0' suffix:
 *
 *       void norm_ptrs_mystruct(struct mystruct *d_mystruct);
 *       void norm_ptrs_mystruct_rev0(struct mystruct_rev0 *d_mystruct);
 *
 * 12. Also in src/sfbase.c, add a 'norm_ptrs' stub function below the
 *     stub function that already exists without the '_rev0' suffix:
 *
 *       void
 *       norm_ptrs_mystruct(struct mystruct *d_mystruct UNUSED)
 *       {
 *       }
 *
 *       void
 *       norm_ptrs_mystruct_rev0(struct mystruct_rev0 *d_mystruct_rev0 UNUSED)
 *       {
 *       }
 *
 * 13. In src/sfstruct.c, add entries to the 'historical' section below the
 *     existing entry that doesn't have a '_rev0' suffix. You need an 'sfo_'
 *     entry and an 'sfi_' entry here, because they have to match function
 *     pointers in another struct. The 'sfo_' function will not be called
 *     from anywhere.
 *
 *         historical_sfo_mystruct,
 *         historical_sfo_mystruct_rev0,
 *     ...
 *         historical_sfi_mystruct,
 *         historical_sfi_mystruct_rev0,
 *
 * 14. In the C source file where the 'Sfi_' call is made for your struct,
 *     the current code likely has a line for reading the struct from
 *     a file, similar the one shown below for the mystruct example:
 *
 *       Sfi_mystruct(nhfp, &svl.mystruct, "mystruct-example");
 *
 *     That single line will need to modified to become a code block similar
 *     to this, so that the uplift function will get called:
 *
 *       if (!gu.uplift_needed_rev0_to_rev1) {
 *            Sfi_mystruct(nhfp, &svl.mystruct, "mystruct-example");
 *          } else {
 *              struct mystruct_rev0 old_mystruct;
 *
 *              Sfi_mystruct_rev0(nhfp, &old_mystruct, "mystruct-example");
 *              uplift_mystruct_rev0_to_mystruct(&old_mystruct, &svl.mystruct);
 *          }
 *       }
 *
 * 15. Shortly after the reading of the struct by the 'Sfi_' function,
 *     if the newer revision of the struct added some new fields, new code
 *     will be needed to initialize the new fields to sane values.
 *     There is no data for the new fields in the existing savefile.
 *
 *        if (gu.uplift_needed_rev0_to_rev1 == 1) {
 *            svl.mystruct.newfielda = sanevalue1;
 *            svl.mystruct.newfieldb = sanevalue2;
 *        }
 *
 * 16. In include/extern.h, add a prototype to the ' ### revision.c ###'
 *     section for the supporting uplift function:
 *
 *       extern void uplift_mystruct_rev0_to_mystruct(struct mystruct_rev0 *,
 *                                                    struct mystruct *);
 *
 * 17. Add the uplift function to src/revision.c.  It needs to do
 *     field-by-field copies of the fields that exist in the old
 *     revision and the new revision. For now, it likely needs to
 *     be handcrafted using your knowledge of the struct's old
 *     and new revisions.
 *
 *     Be sure that any new fields present in the newer revision
 *     of the struct get set to sane values or initialized to
 *     zero. Do whatever suits those fields best, based on your
 *     knowledge of the struct.
 *
 *       void
 *       uplift_mystruct_rev0_to_mystruct(struct mystruct_rev0 *rev0,
 *                                        struct mystruct *rev1)
 *       {
 *           rev1->field1      = rev0->field1;
 *           rev1->field2      = rev0->field2;
 *           rev1->field3      = rev0->field3;
 *           rev1->field4      = rev0->field4;
 *
 *           rev1->newfielda = 0;   // new field
 *           rev1->newfieldb = 42;  // new field
 *       }
 *
 */

#if defined(MYSTRUCT_REV0)
#ifdef DEMO_UPLIFTS
/*
 * struct mystruct_rev0
 *
 * This is the predecessor for 'struct mystruct'
 * Revisions to 'struct mystruct' that 'struct mystruct_rev0' does not have:
 *   int newfielda;
 *   int newfieldb;
 *
 * The uplift function is:
 *     void uplift_mystruct_rev0_to_mystruct(struct *mystruct_rev0,
 *                                           struct *mystruct);
 */

struct mystruct_rev0 {
    int field1;
    int field2;
    char field3;
    long field4;
};
#endif /* DEMO_UPLIFTS */

/**
 * @note The placeholder branch below is where a new revision's block goes -- the instructions above direct you to insert just ahead of it, so it marks the end of the list rather
 *       than doing anything.
 * @warning The final branch is deliberate: including this file without naming a revision is an error rather than a harmless no-op, because doing so silently would mean a
 *          revision block that was never compiled.
 */
/**
 * @note 아래의 자리 표시 분기가 새 개정의 블록이 들어갈 곳이다. 위의 지침은 그 바로 앞에 끼워 넣으라고 지시하므로, 그것은 무엇을 하는 것이 아니라 목록의 끝을 표시한다.
 * @warning 마지막 분기는 의도적이다. 개정을 지칭하지 않고 이 파일을 포함하는 것은 무해한 무동작이 아니라 오류다. 조용히 그렇게 되면 결코 컴파일되지 않은 개정 블록을 뜻하게 되기 때문이다.
 */
#elif defined(XXX_REV0)

#else
#error Unproductive inclusion of revision.h
#endif
/* revision.h */
