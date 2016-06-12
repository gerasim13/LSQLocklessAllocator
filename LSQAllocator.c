//
//  LSQAllocator.c
//  LSQDataStructures
//
//  Created by Павел Литвиненко on 26.08.13.
//  Copyright (c) 2013 Casual Underground. All rights reserved.
//

#include "LSQAllocator.h"

//________________________________________________________________________________________

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#include "nedmalloc.h"
#pragma clang diagnostic pop

//________________________________________________________________________________________

CFAllocatorRef kLSQLocklessAllocator = NULL;

//________________________________________________________________________________________

#ifdef NEDMALLOC_H
    #if defined(__cplusplus) && !defined(NO_NED_NAMESPACE)
        #define ___MALLOC(no, size)   nedalloc::nedmalloc((size_t)size)
        #define ___CALLOC(no, size)   nedalloc::nedcalloc((size_t)no, (size_t)size)
        #define ___REALLOC(mem, size) nedalloc::nedrealloc(mem, (size_t)size)
        #define ___FREE(mem)          nedalloc::nedfree(mem)
    #else
        #define ___MALLOC(no, size)   nedmalloc((size_t)size)
        #define ___CALLOC(no, size)   nedcalloc((size_t)no, (size_t)size)
        #define ___REALLOC(mem, size) nedrealloc(mem, (size_t)size)
        #define ___FREE(mem)          nedfree(mem)
    #endif
#else
    #define ___MALLOC(no, size)   malloc((size_t)size)
    #define ___CALLOC(no, size)   calloc((size_t)no, (size_t)size)
    #define ___REALLOC(mem, size) realloc(mem, (size_t)size)
    #define ___FREE(mem)          free(mem)
#endif

//________________________________________________________________________________________

#ifdef NEDMALLOC_H

//________________________________________________________________________________________

static malloc_zone_t          __zone;
static malloc_introspection_t __zone_introspect;

//________________________________________________________________________________________

#pragma mark - malloc_introspection_t callbacks

static kern_return_t ned_enumerator(task_t task, void *ptr, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder)
{
    return 0;
}

static size_t ned_good_size(malloc_zone_t *zone, size_t size)
{
	size_t ret;
	void *ptr;
    
	/*
	 * Actually create an object of the appropriate size, then find out
	 * how large it could have been without moving up to the next size
	 * class.
	 */
	ptr = nedmalloc(size);
	if (ptr != NULL)
    {
        ret = nedblksize(0, (void *RESTRICT)ptr, 0);
		nedfree(ptr);
	}
    else
    {
        ret = size;
    }
	return (ret);
}

static boolean_t ned_check(malloc_zone_t *zone)
{
    return true;
}

static void ned_print(malloc_zone_t *zone, boolean_t verbose)
{
    
}
static void ned_log(malloc_zone_t *zone, void *address)
{
    
}

static void ned_force_lock(malloc_zone_t *zone)
{
}

static void ned_force_unlock(malloc_zone_t *zone)
{
}

static void ned_statistics(malloc_zone_t *zone, malloc_statistics_t *stats)
{
    
}

static boolean_t ned_zone_locked(malloc_zone_t *zone)
{
    return false;
}

static boolean_t ned_enable_discharge_checking(malloc_zone_t *zone)
{
    return false;
}

static void	ned_disable_discharge_checking(malloc_zone_t *zone)
{
    
}

static void	ned_discharge(malloc_zone_t *zone, void *memory)
{
    
}

static void ned_enumerate_discharged_pointers(malloc_zone_t *zone, void (^report_discharged)(void *memory, void *info))
{
    
}

//________________________________________________________________________________________

#pragma mark - malloc_zone_t callbacks

static size_t ned_size(struct _malloc_zone_t *zone, const void *ptr)
{
    return nedblksize(0, (void *RESTRICT)ptr, 0);
}

static void * ned_malloc(struct _malloc_zone_t *zone, size_t size)
{
    return nedmalloc(size);
}

static void * ned_calloc(struct _malloc_zone_t *zone, size_t num_items, size_t size)
{
    return nedcalloc(num_items, size);
}
static void * ned_valloc(struct _malloc_zone_t *zone, size_t size)
{
    return nedmalloc(size);
}

static void ned_free(struct _malloc_zone_t *zone, void *ptr)
{
    nedfree(ptr);
}

static void * ned_realloc(struct _malloc_zone_t *zone, void *ptr, size_t size)
{
    return nedrealloc(ptr, size);
}

static void * ned_destroy(struct _malloc_zone_t *zone)
{
    // This function should never be called.
	assert(false);
	return (NULL);
}

static unsigned ned_batch_malloc(malloc_zone_t *zone, size_t size, void **results, unsigned num_requested)
{
	// Don't bother implementing this interface, since it isn't required.
	return (0);
}

static void ned_batch_free(malloc_zone_t *zone, void **to_be_freed, unsigned num)
{
	unsigned i;
	for (i = 0; i < num; i++) ned_free(zone, to_be_freed[i]);
}

static void * ned_memalign(struct _malloc_zone_t *zone, size_t alignment, size_t size)
{
    return nedmemalign(alignment, size);
}

static void ned_free_definite_size(malloc_zone_t *zone, void *ptr, size_t size)
{
	assert(nedblksize(0, (void *RESTRICT)ptr, 0) == size);
    nedfree(ptr);
}

//________________________________________________________________________________________

#pragma mark - Malloc Zone Wrapper

static malloc_zone_t * malloc_ned_zone()
{
    __zone.size               = (void *)ned_size;
	__zone.malloc             = (void *)ned_malloc;
	__zone.calloc             = (void *)ned_calloc;
	__zone.valloc             = (void *)ned_valloc;
	__zone.free               = (void *)ned_free;
	__zone.realloc            = (void *)ned_realloc;
	__zone.destroy            = (void *)ned_destroy;
	__zone.zone_name          = "nedmalloc_zone";
	__zone.batch_malloc       = ned_batch_malloc;
	__zone.batch_free         = ned_batch_free;
	__zone.introspect         = &__zone_introspect;
	__zone.version            = 7;
	__zone.memalign           = ned_memalign;
	__zone.free_definite_size = ned_free_definite_size;
    
	__zone_introspect.enumerator                    = ned_enumerator;
	__zone_introspect.good_size                     = ned_good_size;
	__zone_introspect.check                         = ned_check;
	__zone_introspect.print                         = ned_print;
	__zone_introspect.log                           = ned_log;
	__zone_introspect.force_lock                    = (void *)ned_force_lock;
	__zone_introspect.force_unlock                  = (void *)ned_force_unlock;
	__zone_introspect.statistics                    = ned_statistics;
	__zone_introspect.zone_locked                   = ned_zone_locked;
    __zone_introspect.enable_discharge_checking     = ned_enable_discharge_checking;
    __zone_introspect.disable_discharge_checking    = ned_disable_discharge_checking;
    __zone_introspect.discharge                     = ned_discharge;
    __zone_introspect.enumerate_discharged_pointers = ned_enumerate_discharged_pointers;
    
	return (&__zone);
}

//________________________________________________________________________________________

#endif

//________________________________________________________________________________________

inline void * LSQMalloc (size_t size)
{
    return ___MALLOC(no, size);
}

inline void * LSQCalloc (size_t no, size_t size)
{
    return ___CALLOC(no, size);
}

inline void * LSQRealloc(void *mem, size_t size)
{
    return ___REALLOC(mem, size);
}

inline void LSQFree(void *mem)
{
    ___FREE(mem);
}

//________________________________________________________________________________________

#pragma mark - CFAllocator Callbacks

static CFStringRef LSQLocklessAllocatorCopyDescription(const void *info)
{
    CFStringRef format = CFStringCreateWithCString(NULL, "LSQLocklessAllocator <%p>", kCFStringEncodingUTF8);
    CFStringRef ret    = CFStringCreateWithFormat(NULL, NULL, format, info);
    CFRelease(format);
    return ret;
}

//static const void * LSQLocklessAllocatorRetain(const void *info)
//{
//    return info;
//}
//
//static void LSQLocklessAllocatorRelease(const void *info)
//{
//    return;
//}

static void * LSQLocklessAllocatorAllocate(CFIndex allocSize, CFOptionFlags hint, void *info)
{
    malloc_zone_t *zone = (malloc_zone_t *)info;
    return malloc_zone_malloc(zone, allocSize);
}

static void * LSQLocklessAllocatorReallocate(void *ptr, CFIndex newsize, CFOptionFlags hint, void *info)
{
    malloc_zone_t *zone = (malloc_zone_t *)info;
    return malloc_zone_realloc(zone, ptr, newsize);
}

static void LSQLocklessAllocatorDeallocate(void *ptr, void *info)
{
    malloc_zone_t *zone = (malloc_zone_t *)info;
    malloc_zone_free(zone, ptr);
}

static CFIndex LSQLocklessAllocatorPreferredSize(CFIndex size, CFOptionFlags hint, void *info)
{
    malloc_zone_t *zone = (malloc_zone_t *)info;
    return zone->introspect->good_size(zone, size);
}

// This function return pointer to lockless allocator
CFAllocatorRef NewLSQLocklessAllocator()
{
    #ifdef NEDMALLOC_H
        // neddisablethreadcache(0);
        // Create new malloc zone
        malloc_zone_t *allocatorZone = malloc_ned_zone();
    #else
        malloc_zone_t *allocatorZone = malloc_default_zone();
    #endif
    // Create allocator context
    CFAllocatorContext context =
    {
        0,
        (void*)allocatorZone,
        NULL,//LSQLocklessAllocatorRetain,
        NULL,//LSQLocklessAllocatorRelease,
        LSQLocklessAllocatorCopyDescription,
        LSQLocklessAllocatorAllocate,
        LSQLocklessAllocatorReallocate,
        LSQLocklessAllocatorDeallocate,
        LSQLocklessAllocatorPreferredSize
    };
    // Create allocator
    return CFAllocatorCreate(kCFAllocatorUseContext, &context);
}

struct task_basic_info LSQAllocatorGetMemoryInfo()
{
    struct task_basic_info info;
    mach_msg_type_number_t size = sizeof(info);
    task_info(mach_task_self(),
              TASK_BASIC_INFO,
              (task_info_t)&info,
              &size);
    return info;
}

//________________________________________________________________________________________

#pragma mark - CFAllocator wrapper

// Set LSQLocklessAllocator to Defaul Allocator
__attribute__((constructor)) static void LSQAllocator(void)
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        kLSQLocklessAllocator = NewLSQLocklessAllocator();
    });
}

//________________________________________________________________________________________

