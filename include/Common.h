#pragma once

//-----------------------------------------------------------------------------
//	The contents of this file is modified versions of EASTL source --
//	https://github.com/electronicarts/EASTL
//-----------------------------------------------------------------------------
//	BSD 3-Clause License
//	
//	Copyright (c) 2019, Electronic Arts
//	All rights reserved.
//	
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//	
//	1. Redistributions of source code must retain the above copyright notice, this
//	list of conditions and the following disclaimer.
//	
//	2. Redistributions in binary form must reproduce the above copyright notice,
//	this list of conditions and the following disclaimer in the documentation
//	and/or other materials provided with the distribution.
//	
//	3. Neither the name of the copyright holder nor the names of its
//	contributors may be used to endorse or promote products derived from
//	this software without specific prior written permission.
//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include <cstdint>
#include <cstdlib>

// Redefine all the required EASTL defines under new prefixes and namespace
// so that these containers can be used independently from EASTL, but also
// to avoid collisions if they are used alongside EASTL.
namespace sw
{

#ifndef CLUSTER_LIKELY
	#if defined(__GNUC__) && (__GNUC__ >= 3)
		#define CLUSTER_LIKELY(x)   __builtin_expect(!!(x), true)
		#define CLUSTER_UNLIKELY(x) __builtin_expect(!!(x), false)
	#else
		#define CLUSTER_LIKELY(x)   (x)
		#define CLUSTER_UNLIKELY(x) (x)
	#endif
#endif

#ifndef CLUSTER_ASSERT_ENABLED
	#define CLUSTER_ASSERT_ENABLED CLUSTER_DEBUG
#endif

#ifndef CLUSTER_ASSERT
	#define CLUSTER_ASSERT(expression)
#endif

// ------------------------------------------------------------------------
// CLUSTER_UNUSED
// 
// Makes compiler warnings about unused variables go away.
//
// Example usage:
//    void Function(int x)
//    {
//        int y;
//        CLUSTER_UNUSED(x);
//        CLUSTER_UNUSED(y);
//    }
//
#ifndef CLUSTER_UNUSED
// The EDG solution below is pretty weak and needs to be augmented or replaced.
// It can't handle the C language, is limited to places where template declarations
// can be used, and requires the type x to be usable as a functions reference argument. 
#if defined(__cplusplus) && defined(__EDG__)
template <typename T>
inline void CLUSTERBaseUnused(T const volatile & x) { (void)x; }
#define CLUSTER_UNUSED(x) CLUSTERBaseUnused(x)
#else
#define CLUSTER_UNUSED(x) (void)x
#endif
#endif

// CLUSTER_PLATFORM_PTR_SIZE
// Platform pointer size; same as sizeof(void*).
// This is not the same as sizeof(int), as int is usually 32 bits on
// even 64 bit platforms.
//
// _WIN64 is defined by Win64 compilers, such as VC++.
// _M_IA64 is defined by VC++ and Intel compilers for IA64 processors.
// __LP64__ is defined by HP compilers for the LP64 standard.
// _LP64 is defined by the GCC and Sun compilers for the LP64 standard.
// __ia64__ is defined by the GCC compiler for IA64 processors.
// __arch64__ is defined by the Sparc compiler for 64 bit processors.
// __mips64__ is defined by the GCC compiler for MIPS processors.
// __powerpc64__ is defined by the GCC compiler for PowerPC processors.
// __64BIT__ is defined by the AIX compiler for 64 bit processors.
// __sizeof_ptr is defined by the ARM compiler (armcc, armcpp).
//
#ifndef CLUSTER_PLATFORM_PTR_SIZE
#if defined(__WORDSIZE) // Defined by some variations of GCC.
#define CLUSTER_PLATFORM_PTR_SIZE ((__WORDSIZE) / 8)
#elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) || defined(__arch64__) || defined(__aarch64__) || defined(__mips64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
#define CLUSTER_PLATFORM_PTR_SIZE 8
#elif defined(__CC_ARM) && (__sizeof_ptr == 8)
#define CLUSTER_PLATFORM_PTR_SIZE 8
#else
#define CLUSTER_PLATFORM_PTR_SIZE 4
#endif
#endif

// CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT
// This defines the minimal alignment that the platform's malloc 
// implementation will return. This should be used when writing custom
// allocators to ensure that the alignment matches that of malloc
#ifndef CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT
#if defined(__APPLE__) && __APPLE__
	#define CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT 16
#elif defined(__ANDROID__) && !defined(__i386__)
	#if defined(__arm__) || defined(__x86_64)
		#define CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT 8
	#else
		#error Unknown processor
	#endif
#elif defined(__NINTENDO__)
	#if defined(__aarch64__) || defined(__AARCH64)
		#define CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT 8
	#else
		#error Unknown processor
	#endif
#else
	#define CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT (CLUSTER_PLATFORM_PTR_SIZE * 2)
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// CLUSTER_ALLOCATOR_MIN_ALIGNMENT
//
// Defined as an integral power-of-2 that's >= 1.
// Identifies the minimum alignment that CLUSTER should assume its allocators
// use. There is code within CLUSTER that decides whether to do a Malloc or
// MallocAligned call and it's typically better if it can use the Malloc call.
// But this requires knowing what the minimum possible alignment is.
#if !defined(CLUSTER_ALLOCATOR_MIN_ALIGNMENT)
#define CLUSTER_ALLOCATOR_MIN_ALIGNMENT CLUSTER_PLATFORM_MIN_MALLOC_ALIGNMENT
#endif

#ifndef CLUSTERAlloc // To consider: Instead of calling through pAllocator, just go directly to operator new, since that's what allocator does.
#define CLUSTERAlloc(allocator, n) (allocator).allocate(n);
#endif

#ifndef CLUSTERAllocTag
#define CLUSTERAllocTag(allocator, n, file, line, functionName) (allocator).allocate(n, 0, file, line, functionName);
#endif

#ifndef CLUSTERAllocAligned
#define CLUSTERAllocAligned(allocator, n, alignment, offset) (allocator).allocate((n), (alignment), (offset))
#endif

#ifndef CLUSTERAllocAlignedTag
#define CLUSTERAllocAlignedTag(allocator, n, alignment, offset, file, line, functionName) (allocator).allocate((n), 0, (alignment), (offset), file, line, functionName)
#endif

#ifndef CLUSTERFree
#define CLUSTERFree(allocator, p, size) (allocator).deallocate((void*)(p), (size)) // Important to cast to void* as p may be non-const.
#endif

#if defined(__GNUC__) // GCC compilers exist for many platforms.
	#define CLUSTER_COMPILER_GNUC    1
	#define CLUSTER_COMPILER_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#elif defined(_MSC_VER)
	#define CLUSTER_COMPILER_MSVC 1
	#define CLUSTER_COMPILER_MICROSOFT 1
	#define CLUSTER_COMPILER_VERSION _MSC_VER
#elif
	#error unrecognised compiler
#endif

// CLUSTER_COMPILER_NO_ALIGNAS
//
// Refers to C++11 alignas.
//
#if !defined(CLUSTER_COMPILER_NO_ALIGNAS)
// Not supported by VC++ as of VS2013.
#if defined(CLUSTER_COMPILER_CPP11_ENABLED) && defined(__clang__) && (CLUSTER_COMPILER_VERSION >= 401) && defined(__apple_build_version__)    // Apple clang 4.1+
// supported.
#elif defined(CLUSTER_COMPILER_CPP11_ENABLED) && defined(__clang__) && (CLUSTER_COMPILER_VERSION >= 300) && !defined(__apple_build_version__) // Clang 3.0+, not including Apple's Clang.
// supported.
#elif defined(CLUSTER_COMPILER_CPP11_ENABLED) && defined(__GNUC__) && (CLUSTER_COMPILER_VERSION >= 4008)   // GCC 4.8+
// supported.
#else
#define CLUSTER_COMPILER_NO_ALIGNAS 1
#endif
#endif

#if !defined(CLUSTER_ALIGN_MAX)          // If the user hasn't globally set an alternative value...
#if defined(CLUSTER_PROCESSOR_ARM)                       // ARM compilers in general tend to limit automatic variables to 8 or less.
#define CLUSTER_ALIGN_MAX_STATIC    1048576
#define CLUSTER_ALIGN_MAX_AUTOMATIC       1          // Typically they support only built-in natural aligment types (both arm-eabi and apple-abi).
#elif defined(CLUSTER_PLATFORM_APPLE)
#define CLUSTER_ALIGN_MAX_STATIC    1048576
#define CLUSTER_ALIGN_MAX_AUTOMATIC      16
#else
#define CLUSTER_ALIGN_MAX_STATIC    1048576          // Arbitrarily high value. What is the actual max?
#define CLUSTER_ALIGN_MAX_AUTOMATIC 1048576
#endif
#endif

// EDG intends to be compatible with GCC but has a bug whereby it 
// fails to support calling a constructor in an aligned declaration when 
// using postfix alignment attributes. Prefix works for alignment, but does not align
// the size like postfix does.  Prefix also fails on templates.  So gcc style post fix
// is still used, but the user will need to use CLUSTER_POSTFIX_ALIGN before the constructor parameters.
#if defined(__GNUC__) && (__GNUC__ < 3)
#define CLUSTER_ALIGN_OF(type) ((size_t)__alignof__(type))
#define CLUSTER_ALIGN(n)
#define CLUSTER_PREFIX_ALIGN(n)
#define CLUSTER_POSTFIX_ALIGN(n) __attribute__((aligned(n)))
#define CLUSTER_ALIGNED(variable_type, variable, n) variable_type variable __attribute__((aligned(n)))
#define CLUSTER_PACKED __attribute__((packed))

// GCC 3.x+, IBM, and clang support prefix attributes.
#elif (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__xlC__) || defined(__clang__)
#define CLUSTER_ALIGN_OF(type) ((size_t)__alignof__(type))
#define CLUSTER_ALIGN(n) __attribute__((aligned(n)))
#define CLUSTER_PREFIX_ALIGN(n)
#define CLUSTER_POSTFIX_ALIGN(n) __attribute__((aligned(n)))
#define CLUSTER_ALIGNED(variable_type, variable, n) variable_type variable __attribute__((aligned(n)))
#define CLUSTER_PACKED __attribute__((packed))

// Metrowerks supports prefix attributes.
// Metrowerks does not support packed alignment attributes.
#elif (defined(CLUSTER_COMPILER_MSVC) && (CLUSTER_COMPILER_VERSION >= 1300))
#define CLUSTER_ALIGN_OF(type) ((size_t)__alignof(type))
#define CLUSTER_ALIGN(n) __declspec(align(n))
#define CLUSTER_PREFIX_ALIGN(n) CLUSTER_ALIGN(n)
#define CLUSTER_POSTFIX_ALIGN(n)
#define CLUSTER_ALIGNED(variable_type, variable, n) CLUSTER_ALIGN(n) variable_type variable
#define CLUSTER_PACKED // See CLUSTER_PRAGMA_PACK_VC for an alternative.

// Arm brand compiler
#elif defined(CLUSTER_COMPILER_ARM)
#define CLUSTER_ALIGN_OF(type) ((size_t)__ALIGNOF__(type))
#define CLUSTER_ALIGN(n) __align(n)
#define CLUSTER_PREFIX_ALIGN(n) __align(n)
#define CLUSTER_POSTFIX_ALIGN(n)
#define CLUSTER_ALIGNED(variable_type, variable, n) __align(n) variable_type variable
#define CLUSTER_PACKED __packed

#else // Unusual compilers
#error unrecognised compiler
#endif

///////////////////////////////////////////////////////////////////////
// aligned_storage
//
// The aligned_storage transformation trait provides a type that is 
// suitably aligned to store an object whose size is does not exceed length 
// and whose alignment is a divisor of alignment. When using aligned_storage, 
// length must be non-zero, and alignment must >= alignment_of<T>::value 
// for some type T. We require the alignment value to be a power-of-two.
//
// GCC versions prior to 4.4 don't properly support this with stack-based
// variables. The EABase CLUSTER_ALIGN_MAX_AUTOMATIC define identifies the 
// extent to which stack (automatic) variables can be aligned for the 
// given compiler/platform combination.
//
// Example usage:
//     aligned_storage<sizeof(Widget), alignment_of(Widget)>::type widget;
//     Widget* pWidget = new(&widget) Widget;
//
//     aligned_storage<sizeof(Widget), 64>::type widgetAlignedTo64;
//     Widget* pWidget = new(&widgetAlignedTo64) Widget;
//
//     aligned_storage<sizeof(Widget), alignment_of(Widget)>::type widgetArray[37];
//     Widget* pWidgetArray = new(widgetArray) Widget[37];
///////////////////////////////////////////////////////////////////////

#if defined(CLUSTER_COMPILER_GNUC) && (CLUSTER_COMPILER_VERSION >= 4008)
// New versions of GCC do not support using 'alignas' with a value greater than 128.
// However, this code using the GNU standard alignment attribute works properly.
template<size_t N, size_t Align = CLUSTER_ALIGN_OF(double)>
struct aligned_storage
{
	struct type { unsigned char mCharData[N]; } CLUSTER_ALIGN(Align);
};
#elif !defined(CLUSTER_COMPILER_NO_ALIGNAS) // If C++11 alignas is supported...
template<size_t N, size_t Align = CLUSTER_ALIGN_OF(double)>
struct aligned_storage
{
	typedef struct {
		alignas(Align) unsigned char mCharData[N];
	} type;
};

#elif defined(CLUSTER_COMPILER_MSVC) || (defined(CLUSTER_COMPILER_GNUC) && (CLUSTER_COMPILER_VERSION < 4007))
// At some point GCC fixed their attribute(align) to support non-literals, though it's not clear what version aside from being no later than 4.7 and no earlier than 4.2.
// Some compilers don't allow you to to use CLUSTER_ALIGNED with anything by a numeric literal, 
// so we can't use the simpler code like we do further below for other compilers. We support
// only up to so much of an alignment value here.
template<size_t N, size_t Align>
struct aligned_storage_helper { struct type{ unsigned char mCharData[N]; }; };

template<size_t N> struct aligned_storage_helper<N,    2> { struct CLUSTER_ALIGN(   2) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,    4> { struct CLUSTER_ALIGN(   4) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,    8> { struct CLUSTER_ALIGN(   8) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,   16> { struct CLUSTER_ALIGN(  16) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,   32> { struct CLUSTER_ALIGN(  32) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,   64> { struct CLUSTER_ALIGN(  64) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,  128> { struct CLUSTER_ALIGN( 128) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,  256> { struct CLUSTER_ALIGN( 256) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N,  512> { struct CLUSTER_ALIGN( 512) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N, 1024> { struct CLUSTER_ALIGN(1024) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N, 2048> { struct CLUSTER_ALIGN(2048) type{ mutable unsigned char mCharData[N]; }; };
template<size_t N> struct aligned_storage_helper<N, 4096> { struct CLUSTER_ALIGN(4096) type{ mutable unsigned char mCharData[N]; }; };

template<size_t N, size_t Align = CLUSTER_ALIGN_OF(double)>
struct aligned_storage
{
	typedef typename aligned_storage_helper<N, Align>::type type;
};

#else
template<size_t N, size_t Align = CLUSTER_ALIGN_OF(double)>
struct aligned_storage
{
	union type
	{
		unsigned char mCharData[N];
		struct CLUSTER_ALIGN(Align) mStruct{ }; 
	};
};
#endif

#if defined(CLUSTER_COMPILER_NO_TEMPLATE_ALIASES)
#define CLUSTER_ALIGNED_STORAGE_T(N, Align) typename sw::aligned_storage_t<N, Align>::type
#else
template <size_t N, size_t Align = CLUSTER_ALIGN_OF(double)>
using aligned_storage_t = typename aligned_storage<N, Align>::type;
#define CLUSTER_ALIGNED_STORAGE_T(N, Align) sw::aligned_storage_t<N, Align>
#endif

// CLUSTER_COUNT_LEADING_ZEROES
//
// Count leading zeroes in an integer.
//
#ifndef CLUSTER_COUNT_LEADING_ZEROES
#if   defined(__GNUC__)
#if (EA_PLATFORM_PTR_SIZE == 8)
#define CLUSTER_COUNT_LEADING_ZEROES __builtin_clzll
#else
#define CLUSTER_COUNT_LEADING_ZEROES __builtin_clz
#endif
#endif

#ifndef CLUSTER_COUNT_LEADING_ZEROES
static inline int CLUSTER_count_leading_zeroes(uint64_t x)
{
	if(x)
	{
		int n = 0;
		if(x & UINT64_C(0xFFFFFFFF00000000)) { n += 32; x >>= 32; }
		if(x & 0xFFFF0000)                   { n += 16; x >>= 16; }
		if(x & 0xFFFFFF00)                   { n +=  8; x >>=  8; }
		if(x & 0xFFFFFFF0)                   { n +=  4; x >>=  4; }
		if(x & 0xFFFFFFFC)                   { n +=  2; x >>=  2; }
		if(x & 0xFFFFFFFE)                   { n +=  1;           }
		return 63 - n;
	}
	return 64;
}

static inline int CLUSTER_count_leading_zeroes(uint32_t x)
{
	if(x)
	{
		int n = 0;
		if(x <= 0x0000FFFF) { n += 16; x <<= 16; }
		if(x <= 0x00FFFFFF) { n +=  8; x <<=  8; }
		if(x <= 0x0FFFFFFF) { n +=  4; x <<=  4; }
		if(x <= 0x3FFFFFFF) { n +=  2; x <<=  2; }
		if(x <= 0x7FFFFFFF) { n +=  1;           }
		return n;
	}
	return 32;
}

#define CLUSTER_COUNT_LEADING_ZEROES CLUSTER_count_leading_zeroes
#endif
#endif

/// allocate_memory
///
/// This is a memory allocation dispatching function.
/// To do: Make aligned and unaligned specializations.
///        Note that to do this we will need to use a class with a static
///        function instead of a standalone function like below.
///
template <typename Allocator>
inline void* allocate_memory_internal(Allocator& a, size_t n, size_t alignment, size_t alignmentOffset, const char* f, int l, const char* sf)
{
	void *result;
	if (alignment <= CLUSTER_ALLOCATOR_MIN_ALIGNMENT)
	{
		result = CLUSTERAllocTag(a, n, f, l, sf);
		// Ensure the result is correctly aligned.  An assertion likely indicates a mismatch between CLUSTER_ALLOCATOR_MIN_ALIGNMENT and the minimum alignment
		// of CLUSTERAlloc.  If there is a mismatch it may be necessary to define CLUSTER_ALLOCATOR_MIN_ALIGNMENT to be the minimum alignment of CLUSTERAlloc, or
		// to increase the alignment of CLUSTERAlloc to match CLUSTER_ALLOCATOR_MIN_ALIGNMENT.
		CLUSTER_ASSERT((reinterpret_cast<size_t>(result)& ~(alignment - 1)) == reinterpret_cast<size_t>(result));
	}
	else
	{
		result = CLUSTERAllocAlignedTag(a, n, alignment, alignmentOffset, f, l, sf);
		// Ensure the result is correctly aligned.  An assertion here may indicate a bug in the allocator.
		auto resultMinusOffset = (char*)result - alignmentOffset;
		CLUSTER_UNUSED(resultMinusOffset);
		CLUSTER_ASSERT((reinterpret_cast<size_t>(resultMinusOffset)& ~(alignment - 1)) == reinterpret_cast<size_t>(resultMinusOffset));
	}
	return result;
}

}

#define sw_allocate_memory(a, n, alignment, alignmentOffset) sw::allocate_memory_internal(a, n, alignment, alignmentOffset, __FILE__, __LINE__, __FUNCTION__)

//-----------------------------------------------------------------------------

#define CLUSTER_OFFSETOF(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))