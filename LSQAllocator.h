//
//  LSQAllocator.h
//  LSQDataStructures
//
//  Created by Павел Литвиненко on 26.08.13.
//  Copyright (c) 2013 Casual Underground. All rights reserved.
//

#ifndef LSQDataStructures_LSQAllocator_h
#define LSQDataStructures_LSQAllocator_h

//________________________________________________________________________________________

#import <CoreFoundation/CoreFoundation.h>
#import <malloc/malloc.h>
#import <mach/mach.h>
#import <stdio.h>

//________________________________________________________________________________________

CF_EXTERN_C_BEGIN

//________________________________________________________________________________________

CF_EXPORT CFAllocatorRef kLSQLocklessAllocator;

//________________________________________________________________________________________

#pragma mark - Macros

#define LSQAllocatorAllocSize(size)    CFAllocatorAllocate(kLSQLocklessAllocator, size, 0);
#define LSQAllocatorAllocType(type)    LSQAllocatorAllocSize(sizeof(type));
#define LSQAllocatorDealloc(ptr)       CFAllocatorDeallocate(kLSQLocklessAllocator, ptr); ptr = NULL;
#define LSQAllocatorRealloc(ptr, size) CFAllocatorReallocate(kLSQLocklessAllocator, ptr, size, 0);

#define LSQCONSTRUCTOR __attribute((ownership_returns(malloc)))
#define LSQDESTRUCTOR  __attribute((ownership_takes(malloc, 1)))

//________________________________________________________________________________________

#pragma mark - Functions

CF_EXPORT void                   LSQAllocatorSetCurrentAllocator(CFAllocatorRef); // Allocator setter
CF_EXPORT struct task_basic_info LSQAllocatorGetMemoryInfo();                     // Get memory info

LSQCONSTRUCTOR CF_EXPORT void * LSQMalloc (size_t size);
LSQCONSTRUCTOR CF_EXPORT void * LSQCalloc (size_t no, size_t size);
LSQDESTRUCTOR  CF_EXPORT void   LSQFree   (void *mem);
               CF_EXPORT void * LSQRealloc(void *mem, size_t size);

//________________________________________________________________________________________

#if defined(__cplusplus)

struct lsq_delete
{
    void operator()(void* x) { LSQFree(x); }
};

#endif

//________________________________________________________________________________________

CF_EXTERN_C_END

//________________________________________________________________________________________

#endif
