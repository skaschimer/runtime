// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*****************************************************************************/
#ifndef _JIT_H_
#define _JIT_H_
/*****************************************************************************/

//
// clr.sln only defines _DEBUG
// The jit uses DEBUG rather than _DEBUG
// So we make sure that _DEBUG implies DEBUG
//
#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif

// Clang-tidy replaces 0 with nullptr in some templated functions, causing a build
// break. Replacing those instances with ZERO avoids this change
#define ZERO 0

#ifdef _MSC_VER
#define CHECK_STRUCT_PADDING                                                                                           \
    0 // Set this to '1' to enable warning C4820 "'bytes' bytes padding added after
      // construct 'member_name'" on interesting structs/classes
#else
#define CHECK_STRUCT_PADDING 0 // Never enable it for non-MSFT compilers
#endif

#if defined(HOST_X86)
#if defined(HOST_ARM)
#error Cannot define both HOST_X86 and HOST_ARM
#endif
#if defined(HOST_AMD64)
#error Cannot define both HOST_X86 and HOST_AMD64
#endif
#if defined(HOST_ARM64)
#error Cannot define both HOST_X86 and HOST_ARM64
#endif
#if defined(HOST_LOONGARCH64)
#error Cannot define both HOST_X86 and HOST_LOONGARCH64
#endif
#if defined(HOST_RISCV64)
#error Cannot define both HOST_X86 and HOST_RISCV64
#endif
#elif defined(HOST_AMD64)
#if defined(HOST_X86)
#error Cannot define both HOST_AMD64 and HOST_X86
#endif
#if defined(HOST_ARM)
#error Cannot define both HOST_AMD64 and HOST_ARM
#endif
#if defined(HOST_ARM64)
#error Cannot define both HOST_AMD64 and HOST_ARM64
#endif
#if defined(HOST_LOONGARCH64)
#error Cannot define both HOST_AMD64 and HOST_LOONGARCH64
#endif
#if defined(HOST_RISCV64)
#error Cannot define both HOST_AMD64 and HOST_RISCV64
#endif
#elif defined(HOST_ARM)
#if defined(HOST_X86)
#error Cannot define both HOST_ARM and HOST_X86
#endif
#if defined(HOST_AMD64)
#error Cannot define both HOST_ARM and HOST_AMD64
#endif
#if defined(HOST_ARM64)
#error Cannot define both HOST_ARM and HOST_ARM64
#endif
#if defined(HOST_LOONGARCH64)
#error Cannot define both HOST_ARM and HOST_LOONGARCH64
#endif
#if defined(HOST_RISCV64)
#error Cannot define both HOST_ARM and HOST_RISCV64
#endif
#elif defined(HOST_ARM64)
#if defined(HOST_X86)
#error Cannot define both HOST_ARM64 and HOST_X86
#endif
#if defined(HOST_AMD64)
#error Cannot define both HOST_ARM64 and HOST_AMD64
#endif
#if defined(HOST_ARM)
#error Cannot define both HOST_ARM64 and HOST_ARM
#endif
#if defined(HOST_LOONGARCH64)
#error Cannot define both HOST_ARM64 and HOST_LOONGARCH64
#endif
#if defined(HOST_RISCV64)
#error Cannot define both HOST_ARM64 and HOST_RISCV64
#endif
#elif defined(HOST_LOONGARCH64)
#if defined(HOST_X86)
#error Cannot define both HOST_LOONGARCH64 and HOST_X86
#endif
#if defined(HOST_AMD64)
#error Cannot define both HOST_LOONGARCH64 and HOST_AMD64
#endif
#if defined(HOST_ARM)
#error Cannot define both HOST_LOONGARCH64 and HOST_ARM
#endif
#if defined(HOST_ARM64)
#error Cannot define both HOST_LOONGARCH64 and HOST_ARM64
#endif
#if defined(HOST_RISCV64)
#error Cannot define both HOST_LOONGARCH64 and HOST_RISCV64
#endif
#elif defined(HOST_RISCV64)
#if defined(HOST_X86)
#error Cannot define both HOST_RISCV64 and HOST_X86
#endif
#if defined(HOST_AMD64)
#error Cannot define both HOST_RISCV64 and HOST_AMD64
#endif
#if defined(HOST_ARM)
#error Cannot define both HOST_RISCV64 and HOST_ARM
#endif
#if defined(HOST_ARM64)
#error Cannot define both HOST_RISCV64 and HOST_ARM64
#endif
#if defined(HOST_LOONGARCH64)
#error Cannot define both HOST_RISCV64 and HOST_LOONGARCH64
#endif
#else
#error Unsupported or unset host architecture
#endif

#if defined(TARGET_X86)
#if defined(TARGET_ARM)
#error Cannot define both TARGET_X86 and TARGET_ARM
#endif
#if defined(TARGET_AMD64)
#error Cannot define both TARGET_X86 and TARGET_AMD64
#endif
#if defined(TARGET_ARM64)
#error Cannot define both TARGET_X86 and TARGET_ARM64
#endif
#if defined(TARGET_LOONGARCH64)
#error Cannot define both TARGET_X86 and TARGET_LOONGARCH64
#endif
#if defined(TARGET_RISCV64)
#error Cannot define both TARGET_X86 and TARGET_RISCV64
#endif
#elif defined(TARGET_AMD64)
#if defined(TARGET_X86)
#error Cannot define both TARGET_AMD64 and TARGET_X86
#endif
#if defined(TARGET_ARM)
#error Cannot define both TARGET_AMD64 and TARGET_ARM
#endif
#if defined(TARGET_ARM64)
#error Cannot define both TARGET_AMD64 and TARGET_ARM64
#endif
#if defined(TARGET_LOONGARCH64)
#error Cannot define both TARGET_AMD64 and TARGET_LOONGARCH64
#endif
#if defined(TARGET_RISCV64)
#error Cannot define both TARGET_AMD64 and TARGET_RISCV64
#endif
#elif defined(TARGET_ARM)
#if defined(TARGET_X86)
#error Cannot define both TARGET_ARM and TARGET_X86
#endif
#if defined(TARGET_AMD64)
#error Cannot define both TARGET_ARM and TARGET_AMD64
#endif
#if defined(TARGET_ARM64)
#error Cannot define both TARGET_ARM and TARGET_ARM64
#endif
#if defined(TARGET_LOONGARCH64)
#error Cannot define both TARGET_ARM and TARGET_LOONGARCH64
#endif
#if defined(TARGET_RISCV64)
#error Cannot define both TARGET_ARM and TARGET_RISCV64
#endif
#elif defined(TARGET_ARM64)
#if defined(TARGET_X86)
#error Cannot define both TARGET_ARM64 and TARGET_X86
#endif
#if defined(TARGET_AMD64)
#error Cannot define both TARGET_ARM64 and TARGET_AMD64
#endif
#if defined(TARGET_ARM)
#error Cannot define both TARGET_ARM64 and TARGET_ARM
#endif
#if defined(TARGET_LOONGARCH64)
#error Cannot define both TARGET_ARM64 and TARGET_LOONGARCH64
#endif
#if defined(TARGET_RISCV64)
#error Cannot define both TARGET_ARM64 and TARGET_RISCV64
#endif
#elif defined(TARGET_LOONGARCH64)
#if defined(TARGET_X86)
#error Cannot define both TARGET_LOONGARCH64 and TARGET_X86
#endif
#if defined(TARGET_AMD64)
#error Cannot define both TARGET_LOONGARCH64 and TARGET_AMD64
#endif
#if defined(TARGET_ARM)
#error Cannot define both TARGET_LOONGARCH64 and TARGET_ARM
#endif
#if defined(TARGET_ARM64)
#error Cannot define both TARGET_LOONGARCH64 and TARGET_ARM64
#endif
#if defined(TARGET_RISCV64)
#error Cannot define both TARGET_LOONGARCH64 and TARGET_RISCV64
#endif
#elif defined(TARGET_RISCV64)
#if defined(TARGET_X86)
#error Cannot define both TARGET_RISCV64 and TARGET_X86
#endif
#if defined(TARGET_AMD64)
#error Cannot define both TARGET_RISCV64 and TARGET_AMD64
#endif
#if defined(TARGET_ARM)
#error Cannot define both TARGET_RISCV64 and TARGET_ARM
#endif
#if defined(TARGET_ARM64)
#error Cannot define both TARGET_RISCV64 and TARGET_ARM64
#endif
#if defined(TARGET_LOONGARCH64)
#error Cannot define both TARGET_RISCV64 and TARGET_LOONGARCH64
#endif

#else
#error Unsupported or unset target architecture
#endif

#ifdef TARGET_64BIT
#ifdef TARGET_X86
#error Cannot define both TARGET_X86 and TARGET_64BIT
#endif // TARGET_X86
#ifdef TARGET_ARM
#error Cannot define both TARGET_ARM and TARGET_64BIT
#endif // TARGET_ARM
#endif // TARGET_64BIT

#if defined(TARGET_X86) || defined(TARGET_AMD64)
#define TARGET_XARCH
#endif

#if defined(TARGET_ARM) || defined(TARGET_ARM64)
#define TARGET_ARMARCH
#endif

// If the UNIX_AMD64_ABI is defined make sure that TARGET_AMD64 is also defined.
#if defined(UNIX_AMD64_ABI)
#if !defined(TARGET_AMD64)
#error When UNIX_AMD64_ABI is defined you must define TARGET_AMD64 defined as well.
#endif
#endif

// If the UNIX_X86_ABI is defined make sure that TARGET_X86 is also defined.
#if defined(UNIX_X86_ABI)
#if !defined(TARGET_X86)
#error When UNIX_X86_ABI is defined you must define TARGET_X86 defined as well.
#endif
#endif

// --------------------------------------------------------------------------------
// IMAGE_FILE_MACHINE_TARGET
// --------------------------------------------------------------------------------

#if defined(TARGET_X86)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_I386
#elif defined(TARGET_AMD64)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_AMD64
#elif defined(TARGET_ARM)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_ARMNT
#elif defined(TARGET_ARM64)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_ARM64 // 0xAA64
#elif defined(TARGET_LOONGARCH64)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_LOONGARCH64 // 0x6264
#elif defined(TARGET_RISCV64)
#define IMAGE_FILE_MACHINE_TARGET IMAGE_FILE_MACHINE_RISCV64 // 0x5064
#else
#error Unsupported or unset target architecture
#endif

typedef ptrdiff_t ssize_t;

// Include the AMD64 unwind codes when appropriate.
#if defined(TARGET_AMD64)
#include "win64unwind.h"
#endif

#include "corhdr.h"
#include "corjit.h"
#include "jitee.h"

#define __OPERATOR_NEW_INLINE  1 // indicate that I will define these
#define __PLACEMENT_NEW_INLINE   // don't bring in the global placement new, it is easy to make a mistake
                                 // with our new(compiler*) pattern.

#include "utilcode.h" // this defines assert as _ASSERTE
#include "host.h"     // this redefines assert for the JIT to use assertAbort
#include "utils.h"
#include "targetosarch.h"

// The late disassembler is built in for certain platforms, for DEBUG builds. It is enabled by using
// DOTNET_JitLateDisasm. It can be built in for non-DEBUG builds if desired.

#if defined(TARGET_ARM64) || defined(TARGET_ARM) || defined(TARGET_X86) || defined(TARGET_AMD64)
#ifdef DEBUG
#define LATE_DISASM 1
#define USE_COREDISTOOLS
#endif // DEBUG
#endif // platforms

#if defined(LATE_DISASM) && (LATE_DISASM == 0)
#undef LATE_DISASM
#endif

#ifdef DEBUG
#define INDEBUG(x)  x
#define DEBUGARG(x) , x
#else
#define INDEBUG(x)
#define DEBUGARG(x)
#endif

#if defined(DEBUG) || defined(LATE_DISASM)
#define INDEBUG_LDISASM_COMMA(x) x,
#else
#define INDEBUG_LDISASM_COMMA(x)
#endif

#if defined(UNIX_AMD64_ABI)
#define UNIX_AMD64_ABI_ONLY_ARG(x) , x
#define UNIX_AMD64_ABI_ONLY(x)     x
#else // !defined(UNIX_AMD64_ABI)
#define UNIX_AMD64_ABI_ONLY_ARG(x)
#define UNIX_AMD64_ABI_ONLY(x)
#endif // defined(UNIX_AMD64_ABI)

#if defined(TARGET_LOONGARCH64)
#define UNIX_LOONGARCH64_ONLY_ARG(x) , x
#define UNIX_LOONGARCH64_ONLY(x)     x
#else // !TARGET_LOONGARCH64
#define UNIX_LOONGARCH64_ONLY_ARG(x)
#define UNIX_LOONGARCH64_ONLY(x)
#endif // TARGET_LOONGARCH64

#if defined(UNIX_AMD64_ABI)
#define UNIX_AMD64_ABI_ONLY_ARG(x) , x
#define UNIX_AMD64_ABI_ONLY(x)     x
#else // !defined(UNIX_AMD64_ABI)
#define UNIX_AMD64_ABI_ONLY_ARG(x)
#define UNIX_AMD64_ABI_ONLY(x)
#endif // defined(UNIX_AMD64_ABI)

#if defined(UNIX_AMD64_ABI) || defined(TARGET_ARM64) || defined(TARGET_LOONGARCH64) || defined(TARGET_RISCV64)
#define MULTIREG_HAS_SECOND_GC_RET             1
#define MULTIREG_HAS_SECOND_GC_RET_ONLY_ARG(x) , x
#define MULTIREG_HAS_SECOND_GC_RET_ONLY(x)     x
#else // !defined(UNIX_AMD64_ABI)
#define MULTIREG_HAS_SECOND_GC_RET 0
#define MULTIREG_HAS_SECOND_GC_RET_ONLY_ARG(x)
#define MULTIREG_HAS_SECOND_GC_RET_ONLY(x)
#endif // defined(UNIX_AMD64_ABI)

// Arm64 Windows supports FEATURE_ARG_SPLIT, note this is different from
// the official Arm64 ABI.
// Case: splitting 16 byte struct between x7 and stack
// LoongArch64's ABI supports FEATURE_ARG_SPLIT which splitting 16 byte struct between a7 and stack.
#if defined(TARGET_ARM) || defined(TARGET_ARM64) || defined(TARGET_LOONGARCH64) || defined(TARGET_RISCV64)
#define FEATURE_ARG_SPLIT 1
#else
#define FEATURE_ARG_SPLIT 0
#endif

// To get rid of warning 4701 : local variable may be used without being initialized
#define DUMMY_INIT(x) (x)

#define REGEN_SHORTCUTS 0
#define REGEN_CALLPAT   0

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX                                                                           XX
XX                          jit.h                                            XX
XX                                                                           XX
XX   Interface of the JIT with jit.cpp                                       XX
XX                                                                           XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

/*****************************************************************************/
#if defined(DEBUG)
#include "log.h"

#define INFO6 LL_INFO10000   // Did Jit or Inline succeeded?
#define INFO7 LL_INFO100000  // NYI stuff
#define INFO8 LL_INFO1000000 // Weird failures

#endif // DEBUG

typedef class ICorJitInfo* COMP_HANDLE;

const CORINFO_OBJECT_HANDLE NO_OBJECT_HANDLE = nullptr;
const CORINFO_CLASS_HANDLE  NO_CLASS_HANDLE  = nullptr;
const CORINFO_FIELD_HANDLE  NO_FIELD_HANDLE  = nullptr;
const CORINFO_METHOD_HANDLE NO_METHOD_HANDLE = nullptr;

/*****************************************************************************/

typedef unsigned IL_OFFSET;

const IL_OFFSET BAD_IL_OFFSET = 0xffffffff;

const unsigned BAD_VAR_NUM    = UINT_MAX;
const uint16_t BAD_LCL_OFFSET = UINT16_MAX;

// Code can't be more than 2^31 in any direction.  This is signed, so it should be used for anything that is
// relative to something else.
typedef int NATIVE_OFFSET;

// This is the same as the above, but it's used in absolute contexts (i.e. offset from the start).  Also,
// this is used for native code sizes.
typedef unsigned UNATIVE_OFFSET;

// Type used for weights (e.g. block and edge weights)
typedef double weight_t;

// For the following specially handled FIELD_HANDLES we need
//   values that are negative and have the low two bits zero
// See eeFindJitDataOffs and eeGetJitDataOffs in Compiler.hpp
#define FLD_GLOBAL_DS ((CORINFO_FIELD_HANDLE)-4)
#define FLD_GLOBAL_FS ((CORINFO_FIELD_HANDLE)-8)
#define FLD_GLOBAL_GS ((CORINFO_FIELD_HANDLE)-12)

class GlobalJitOptions
{
public:
#ifdef FEATURE_HFA
#define FEATURE_HFA_FIELDS_PRESENT
#ifdef CONFIGURABLE_ARM_ABI
    // These are safe to have globals as they cannot change once initialized within the process.
    static LONG compUseSoftFPConfigured;
    static bool compFeatureHfa;
#else  // !CONFIGURABLE_ARM_ABI
    static const bool compFeatureHfa = true;
#endif // CONFIGURABLE_ARM_ABI
#else  // !FEATURE_HFA
    static const bool compFeatureHfa = false;
#endif // FEATURE_HFA

#ifdef FEATURE_HFA
#undef FEATURE_HFA
#endif
};

/*****************************************************************************/

#include "vartype.h"

/*****************************************************************************/

/*****************************************************************************/

/*****************************************************************************/

#define CSE_INTO_HANDLERS 0
#define DUMP_FLOWGRAPHS   DEBUG // Support for creating Xml Flowgraph reports in *.fgx files

/*****************************************************************************/

#define VPTR_OFFS 0 // offset of vtable pointer from obj ptr

/*****************************************************************************/

#define DUMP_GC_TABLES   DEBUG
#define VERIFY_GC_TABLES 0
#define REARRANGE_ADDS   1

#define FUNC_INFO_LOGGING                                                                                              \
    1 // Support dumping function info to a file. In retail, only NYIs, with no function name,
      // are dumped.

/*****************************************************************************/
/*****************************************************************************/
/* Set these to 1 to collect and output various statistics about the JIT */

#define CALL_ARG_STATS 0 // Collect stats about calls and call arguments.
#define COUNT_BASIC_BLOCKS                                                                                             \
    0                         // Create a histogram of basic block sizes, and a histogram of IL sizes in the simple
                              // case of single block methods.
#define DISPLAY_SIZES       0 // Display generated code, data, and GC information sizes.
#define MEASURE_BLOCK_SIZE  0 // Collect stats about basic block and FlowEdge node sizes and memory allocations.
#define MEASURE_FATAL       0 // Count the number of calls to fatal(), including NYIs and noway_asserts.
#define MEASURE_NODE_SIZE   0 // Collect stats about GenTree node allocations.
#define MEASURE_PTRTAB_SIZE 0 // Collect stats about GC pointer table allocations.
#define EMITTER_STATS       0 // Collect stats on the emitter.
#define NODEBASH_STATS      0 // Collect stats on changed gtOper values in GenTree's.
#define COUNT_AST_OPERS     0 // Display use counts for GenTree operators.

#ifdef DEBUG
#define MEASURE_MEM_ALLOC 1 // Collect memory allocation stats.
#define LOOP_HOIST_STATS  1 // Collect loop hoisting stats.
#define TRACK_LSRA_STATS  1 // Collect LSRA stats
#define TRACK_ENREG_STATS 1 // Collect enregistration stats
#else
#define MEASURE_MEM_ALLOC 0 // You can set this to 1 to get memory stats in retail, as well
#define LOOP_HOIST_STATS  0 // You can set this to 1 to get loop hoist stats in retail, as well
#define TRACK_LSRA_STATS  0 // You can set this to 1 to get LSRA stats in retail, as well
#define TRACK_ENREG_STATS 0
#endif

// Timing calls to clr.dll is only available under certain conditions.
#ifndef FEATURE_JIT_METHOD_PERF
#define MEASURE_CLRAPI_CALLS 0 // Can't time these calls without METHOD_PERF.
#endif
#ifdef DEBUG
#define MEASURE_CLRAPI_CALLS 0 // No point in measuring DEBUG code.
#endif
#if !defined(HOST_X86) && !defined(HOST_AMD64)
#define MEASURE_CLRAPI_CALLS 0 // Cycle counters only hooked up on x86/x64.
#endif
#if !defined(_MSC_VER) && !defined(__GNUC__)
#define MEASURE_CLRAPI_CALLS 0 // Only know how to do this with VC and Clang.
#endif

// If none of the above set the flag to 0, it's available.
#ifndef MEASURE_CLRAPI_CALLS
#define MEASURE_CLRAPI_CALLS 0 // Set to 1 to measure time in ICorJitInfo calls.
#endif

/*****************************************************************************/
/* Portability Defines */
/*****************************************************************************/
#ifdef TARGET_X86
#define JIT32_GCENCODER
#endif

/*****************************************************************************/
#if !defined(DEBUG)

#if DUMP_GC_TABLES
#pragma message("NOTE: this non-debug build has GC ptr table dumping always enabled!")
const bool dspGCtbls = true;
#endif

#endif // !DEBUG

/*****************************************************************************/

#ifdef DEBUG
#define JITDUMP(...)                                                                                                   \
    {                                                                                                                  \
        if (JitTls::GetCompiler()->verbose)                                                                            \
            logf(__VA_ARGS__);                                                                                         \
    }
#define JITDUMPEXEC(x)                                                                                                 \
    {                                                                                                                  \
        if (JitTls::GetCompiler()->verbose)                                                                            \
            x;                                                                                                         \
    }
#define JITLOG(x)                                                                                                      \
    {                                                                                                                  \
        JitLogEE x;                                                                                                    \
    }
#define JITLOG_THIS(t, x)                                                                                              \
    {                                                                                                                  \
        (t)->JitLogEE x;                                                                                               \
    }
#define DBEXEC(flg, expr)                                                                                              \
    if (flg)                                                                                                           \
    {                                                                                                                  \
        expr;                                                                                                          \
    }
#define DISPNODE(t)                                                                                                    \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispTree(t, nullptr, nullptr, true);
#define DISPTREE(t)                                                                                                    \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispTree(t);
#define DISPSTMT(t)                                                                                                    \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispStmt(t);
#define DISPRANGE(range)                                                                                               \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispRange(range);
#define DISPTREERANGE(range, t)                                                                                        \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispTreeRange(range, t);
#define LABELEDDISPTREERANGE(label, range, t)                                                                          \
    JITDUMP(label ":\n");                                                                                              \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->gtDispTreeRange(range, t);                                                              \
    JITDUMP("\n");
#define DISPBLOCK(b)                                                                                                   \
    if (JitTls::GetCompiler()->verbose)                                                                                \
        JitTls::GetCompiler()->fgTableDispBasicBlock(b);
#define VERBOSE JitTls::GetCompiler()->verbose
// Development-time only macros, simplify guards for specified IL methods one wants to debug/add log messages for
#define ISMETHOD(name)     (strcmp(JitTls::GetCompiler()->impInlineRoot()->info.compMethodName, name) == 0)
#define ISMETHODHASH(hash) (JitTls::GetCompiler()->impInlineRoot()->info.compMethodHash() == hash)
#else // !DEBUG
#define JITDUMP(...)
#define JITDUMPEXEC(x)
#define JITLOG(x)
#define JITLOG_THIS(t, x)
#define DBEXEC(flg, expr)
#define DISPNODE(t)
#define DISPTREE(t)
#define DISPSTMT(t)
#define DISPRANGE(range)
#define DISPTREERANGE(range, t)
#define LABELEDDISPTREERANGE(title, range, t)
#define DISPBLOCK(b)
#define VERBOSE 0
#endif // !DEBUG

/*****************************************************************************
 *
 * Double alignment. This aligns ESP to 0 mod 8 in function prolog, then uses ESP
 * to reference locals, EBP to reference parameters.
 * It only makes sense if frameless method support is on.
 * (frameless method support is now always on)
 */

#ifdef TARGET_X86
#define DOUBLE_ALIGN                                                                                                   \
    1 // permit the double alignment of ESP in prolog,
      //  and permit the double alignment of local offsets
#else
#define DOUBLE_ALIGN 0 // no special handling for double alignment
#endif

#ifdef DEBUG

// Forward declarations for UninitializedWord and IsUninitialized are needed by alloc.h
template <typename T>
inline T UninitializedWord(Compiler* comp);

template <typename T>
inline bool IsUninitialized(T data);

#endif // DEBUG

/*****************************************************************************/

#define castto(var, typ) (*(typ*)&var)

/*****************************************************************************/

#ifdef NO_MISALIGNED_ACCESS

#define MISALIGNED_RD_I2(src) (*castto(src, char*) | *castto(src + 1, char*) << 8)

#define MISALIGNED_RD_U2(src) (*castto(src, char*) | *castto(src + 1, char*) << 8)

#define MISALIGNED_WR_I2(dst, val)                                                                                     \
    *castto(dst, char*)     = val;                                                                                     \
    *castto(dst + 1, char*) = val >> 8;

#define MISALIGNED_WR_I4(dst, val)                                                                                     \
    *castto(dst, char*)     = val;                                                                                     \
    *castto(dst + 1, char*) = val >> 8;                                                                                \
    *castto(dst + 2, char*) = val >> 16;                                                                               \
    *castto(dst + 3, char*) = val >> 24;

#else

#define MISALIGNED_RD_I2(src) (*castto(src, short*))
#define MISALIGNED_RD_U2(src) (*castto(src, unsigned short*))

#define MISALIGNED_WR_I2(dst, val) *castto(dst, short*) = val;
#define MISALIGNED_WR_I4(dst, val) *castto(dst, int*) = val;

#define MISALIGNED_WR_ST(dst, val) *castto(dst, ssize_t*) = val;

#endif

/*****************************************************************************/

inline size_t roundUp(size_t size, size_t mult = sizeof(size_t))
{
    assert(mult && ((mult & (mult - 1)) == 0)); // power of two test

    return (size + (mult - 1)) & ~(mult - 1);
}

#ifdef HOST_64BIT
inline unsigned int roundUp(unsigned size, unsigned mult)
{
    return (unsigned int)roundUp((size_t)size, (size_t)mult);
}
#endif // HOST_64BIT

inline unsigned int unsigned_abs(int x)
{
    return ((unsigned int)std::abs(x));
}

#ifdef TARGET_64BIT
inline size_t unsigned_abs(ssize_t x)
{
    return ((size_t)std::abs((int64_t)x));
}

#ifdef __APPLE__
inline size_t unsigned_abs(int64_t x)
{
    return ((size_t)std::abs(x));
}
#endif // __APPLE__
#endif // TARGET_64BIT

/*****************************************************************************/

#include "error.h"

/*****************************************************************************/

#if CHECK_STRUCT_PADDING
#pragma warning(push)
#pragma warning(default : 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif                          // CHECK_STRUCT_PADDING
#include "alloc.h"
#include "target.h"

#if FEATURE_TAILCALL_OPT
// Enable tail call opt for the following IL pattern
//
//     call someFunc
//     jmp/jcc RetBlock
//     ...
//  RetBlock:
//     ret
#define FEATURE_TAILCALL_OPT_SHARED_RETURN 1
#else // !FEATURE_TAILCALL_OPT
#define FEATURE_TAILCALL_OPT_SHARED_RETURN 0
#endif // !FEATURE_TAILCALL_OPT

#define CLFLG_REGVAR    0x00008
#define CLFLG_TREETRANS 0x00100
#define CLFLG_INLINING  0x00200

#if FEATURE_STRUCTPROMOTE
#define CLFLG_STRUCTPROMOTE 0x00400
#else
#define CLFLG_STRUCTPROMOTE 0x00000
#endif

#if defined(TARGET_XARCH) || defined(TARGET_ARM64)
#define FEATURE_LOOP_ALIGN 1
#else
#define FEATURE_LOOP_ALIGN 0
#endif

#define CLFLG_MAXOPT (CLFLG_REGVAR | CLFLG_TREETRANS | CLFLG_INLINING | CLFLG_STRUCTPROMOTE)
#define CLFLG_MINOPT (CLFLG_TREETRANS)

/*****************************************************************************/

extern void dumpILBytes(const BYTE* const codeAddr, unsigned codeSize, unsigned alignSize);

extern unsigned dumpSingleInstr(const BYTE* const codeAddr, IL_OFFSET offs, const char* prefix = nullptr);

extern void dumpILRange(const BYTE* const codeAddr, unsigned codeSize); // in bytes

/*****************************************************************************/

extern int jitNativeCode(CORINFO_METHOD_HANDLE methodHnd,
                         CORINFO_MODULE_HANDLE classHnd,
                         COMP_HANDLE           compHnd,
                         CORINFO_METHOD_INFO*  methodInfo,
                         void**                methodCodePtr,
                         uint32_t*             methodCodeSize,
                         JitFlags*             compileFlags,
                         void*                 inlineInfoPtr);

// Constants for making sure size_t fit into smaller types.
const size_t MAX_USHORT_SIZE_T   = static_cast<size_t>(static_cast<unsigned short>(-1));
const size_t MAX_UNSIGNED_SIZE_T = static_cast<size_t>(static_cast<unsigned>(-1));

/*****************************************************************************/

class Compiler;

class JitTls
{
#ifdef DEBUG
    Compiler* m_compiler;
    LogEnv    m_logEnv;
    JitTls*   m_next;
#endif

public:
    JitTls(ICorJitInfo* jitInfo);
    ~JitTls();

#ifdef DEBUG
    static LogEnv* GetLogEnv();
#endif

    static Compiler* GetCompiler();
    static void      SetCompiler(Compiler* compiler);
};

#if defined(DEBUG)
//  Include the definition of Compiler for use by these template functions
//
#include "compiler.h"

//****************************************************************************
//
//  Returns a word filled with the JITs allocator default fill value.
//
template <typename T>
inline T UninitializedWord(Compiler* comp)
{
    unsigned char defaultFill = 0xdd;
    if (comp == nullptr)
    {
        comp = JitTls::GetCompiler();
    }
    defaultFill = Compiler::compGetJitDefaultFill(comp);
    assert(defaultFill <= 0xff);
    int64_t word = 0x0101010101010101LL * defaultFill;
    return (T)word;
}

//****************************************************************************
//
//  Tries to determine if this value is coming from uninitialized JIT memory
//    - Returns true if the value matches what we initialized the memory to.
//
//  Notes:
//    - Asserts that use this are assuming that the UninitializedWord value
//      isn't a legal value for 'data'.  Thus using a default fill value of
//      0x00 will often trigger such asserts.
//
template <typename T>
inline bool IsUninitialized(T data)
{
    return data == UninitializedWord<T>(JitTls::GetCompiler());
}

#pragma warning(push)
#pragma warning(disable : 4312)
//****************************************************************************
//
//  Debug template definitions for dspPtr, dspOffset
//    - Used to format pointer/offset values for diffable Disasm
//
template <typename T>
T dspPtr(T p)
{
    return (p == ZERO) ? ZERO : (JitTls::GetCompiler()->opts.dspDiffable ? T(0xD1FFAB1E) : p);
}

template <typename T>
T dspOffset(T o)
{
    return (o == ZERO) ? ZERO : (JitTls::GetCompiler()->opts.dspDiffable ? T(0xD1FFAB1E) : o);
}
#pragma warning(pop)

#else // !defined(DEBUG)

//****************************************************************************
//
//  Non-Debug template definitions for dspPtr, dspOffset
//    - This is a nop in non-Debug builds
//
template <typename T>
T dspPtr(T p)
{
    return p;
}

template <typename T>
T dspOffset(T o)
{
    return o;
}

#endif // !defined(DEBUG)

struct LikelyClassMethodRecord
{
    intptr_t handle;
    UINT32   likelihood;
};

struct LikelyValueRecord
{
    ssize_t value;
    UINT32  likelihood;
};

extern "C" UINT32 WINAPI getLikelyValues(LikelyValueRecord*                     pLikelyValues,
                                         UINT32                                 maxLikelyValues,
                                         ICorJitInfo::PgoInstrumentationSchema* schema,
                                         UINT32                                 countSchemaItems,
                                         BYTE*                                  pInstrumentationData,
                                         int32_t                                ilOffset);

extern "C" UINT32 WINAPI getLikelyClasses(LikelyClassMethodRecord*               pLikelyClasses,
                                          UINT32                                 maxLikelyClasses,
                                          ICorJitInfo::PgoInstrumentationSchema* schema,
                                          UINT32                                 countSchemaItems,
                                          BYTE*                                  pInstrumentationData,
                                          int32_t                                ilOffset);

extern "C" UINT32 WINAPI getLikelyMethods(LikelyClassMethodRecord*               pLikelyMethods,
                                          UINT32                                 maxLikelyMethods,
                                          ICorJitInfo::PgoInstrumentationSchema* schema,
                                          UINT32                                 countSchemaItems,
                                          BYTE*                                  pInstrumentationData,
                                          int32_t                                ilOffset);

/*****************************************************************************/
#endif //_JIT_H_
/*****************************************************************************/
