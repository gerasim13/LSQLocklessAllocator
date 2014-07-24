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

#pragma mark - Functions

CF_EXPORT void                   LSQAllocatorSetCurrentAllocator(CFAllocatorRef); // Allocator setter
CF_EXPORT struct task_basic_info LSQAllocatorGetMemoryInfo();                     // Get memory info

CF_INLINE void * LSQMalloc (size_t size);
CF_INLINE void * LSQCalloc (size_t no, size_t size);
CF_INLINE void * LSQRealloc(void *mem, size_t size);
CF_INLINE void   LSQFree   (void *mem);

//________________________________________________________________________________________

#pragma mark - Macros

#define LSQAllocatorAllocSize(size)    CFAllocatorAllocate(kLSQLocklessAllocator, size, 0);
#define LSQAllocatorAllocType(type)    LSQAllocatorAllocSize(sizeof(type));
#define LSQAllocatorDealloc(ptr)       CFAllocatorDeallocate(kLSQLocklessAllocator, ptr); ptr = NULL;
#define LSQAllocatorRealloc(ptr, size) CFAllocatorReallocate(kLSQLocklessAllocator, ptr, size, 0);

//________________________________________________________________________________________

CF_EXTERN_C_END

//________________________________________________________________________________________

#endif
