/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 */

#ifndef UTEE_TYPES_H
#define UTEE_TYPES_H

#include <inttypes.h>
#include <sys/types.h>
#include <tee_api_defines.h>

// extern int errno;

// /* fcntl start */

// #ifndef _INC_FCNTL
// #define _INC_FCNTL

// /* open/fcntl.  */
// #define O_ACCMODE 0003
// #define O_RDONLY 00
// #define O_WRONLY 01
// #define O_RDWR 02
// #ifndef O_CREAT
// #define O_CREAT 0100 /* Not fcntl.  */
// #endif
// #ifndef O_EXCL
// #define O_EXCL 0200 /* Not fcntl.  */
// #endif
// #ifndef O_NOCTTY
// #define O_NOCTTY 0400 /* Not fcntl.  */
// #endif
// #ifndef O_TRUNC
// #define O_TRUNC 01000 /* Not fcntl.  */
// #endif
// #ifndef O_APPEND
// #define O_APPEND 02000
// #endif
// #ifndef O_NONBLOCK
// #define O_NONBLOCK 04000
// #endif
// #ifndef O_NDELAY
// #define O_NDELAY O_NONBLOCK
// #endif
// #ifndef O_SYNC
// #define O_SYNC 04010000
// #endif
// #define O_FSYNC O_SYNC
// #ifndef O_ASYNC
// #define O_ASYNC 020000
// #endif
// #ifndef __O_LARGEFILE
// #define __O_LARGEFILE 0100000
// #endif

// #ifndef __O_DIRECTORY
// #define __O_DIRECTORY 0200000
// #endif
// #ifndef __O_NOFOLLOW
// #define __O_NOFOLLOW 0400000
// #endif
// #ifndef __O_CLOEXEC
// #define __O_CLOEXEC 02000000
// #endif
// #ifndef __O_DIRECT
// #define __O_DIRECT 040000
// #endif
// #ifndef __O_NOATIME
// #define __O_NOATIME 01000000
// #endif
// #ifndef __O_PATH
// #define __O_PATH 010000000
// #endif
// #ifndef __O_DSYNC
// #define __O_DSYNC 010000
// #endif
// #ifndef __O_TMPFILE
// #define __O_TMPFILE 020200000
// #endif

// #endif

// #define F_GETFD 1
// #define F_SETFD 2
// #define F_GETLK 5
// #define F_SETLK 6

// #define F_RDLCK 0 /* Read lock.  */
// #define F_WRLCK 1 /* Write lock.  */
// #define F_UNLCK 2 /* Remove lock.  */

// #define F_OK 0
// #define W_OK 2
// #define R_OK 4
// /* fcntl end */

// /* unistd */

// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2

/* unistd */

enum utee_time_category {
	UTEE_TIME_CAT_SYSTEM = 0,
	UTEE_TIME_CAT_TA_PERSISTENT,
	UTEE_TIME_CAT_REE
};

enum utee_entry_func {
	UTEE_ENTRY_FUNC_OPEN_SESSION = 0,
	UTEE_ENTRY_FUNC_CLOSE_SESSION,
	UTEE_ENTRY_FUNC_INVOKE_COMMAND,
	/*
	 * UTEE_ENTRY_FUNC_DUMP_MEMSTATS
	 * [out] value[0].a  Byte size currently allocated
	 * [out] value[0].b  Max bytes allocated since stats reset
	 * [out] value[1].a  Pool byte size
	 * [out] value[1].b  Number of failed allocation requests
	 * [out] value[2].a  Biggest byte size which allocation failed
	 * [out] value[2].b  Biggest byte size which allocation succeeded
	 */
	UTEE_ENTRY_FUNC_DUMP_MEMSTATS,
};

/*
 * Cache operation types.
 * Used when extensions TEE_CacheClean() / TEE_CacheFlush() /
 * TEE_CacheInvalidate() are used
 */
enum utee_cache_operation {
	TEE_CACHECLEAN = 0,
	TEE_CACHEFLUSH,
	TEE_CACHEINVALIDATE,
};

struct utee_params {
	uint64_t types;
	/* vals[n * 2]	   corresponds to either value.a or memref.buffer
	 * vals[n * 2 + ]  corresponds to either value.b or memref.size
	 * when converting to/from struct tee_ta_param
	 */
	uint64_t vals[TEE_NUM_PARAMS * 2];
};

struct utee_attribute {
	uint64_t a; /* also serves as a pointer for references */
	uint64_t b; /* also serves as a length for references */
	uint32_t attribute_id;
};

struct utee_object_info {
	uint32_t obj_type;
	uint32_t obj_size;
	uint32_t max_obj_size;
	uint32_t obj_usage;
	uint32_t data_size;
	uint32_t data_pos;
	uint32_t handle_flags;
};

// type which optee not supported start
typedef unsigned int __pid_t;
typedef __pid_t pid_t;
typedef unsigned int __uid_t;
typedef __uid_t uid_t;
typedef unsigned int __gid_t;
typedef __gid_t gid_t;
typedef unsigned long __dev_t;
typedef __dev_t dev_t;
typedef unsigned long __ino_t;
typedef unsigned long __ino64_t;
typedef unsigned int __mode_t;
typedef __mode_t mode_t;
typedef unsigned int __nlink_t;
typedef long __off_t;
typedef __off_t off_t;
typedef long __off64_t;
// typedef long int __clock_t;
// typedef unsigned long int __rlim_t;
// typedef unsigned long long int __rlim64_t;
// typedef unsigned int __id_t;
typedef long time_t;
typedef time_t __time_t;
// typedef unsigned int __useconds_t;
// typedef long int __suseconds_t;
typedef unsigned int __blksize_t;
// typedef long int __blkcnt_t;
typedef long __blkcnt64_t;
typedef int __syscall_slong_t;
// typedef unsigned long __syscall_ulong_t;
typedef long ssize_t;
// type which optee not supported end

typedef unsigned int uid_t;
typedef long off_t;
typedef unsigned int gid_t;
typedef unsigned int pid_t;

struct timespec {
	time_t tv_sec;
	long tv_nsec;
};

struct my_flock {
	short l_type;
	short l_whence;
	long l_start;
	long l_len;
	int l_pid;
	//__ARCH_FLOCK_PAD
};

struct my_stat {
	__dev_t st_dev; /* Device.  */
	__ino_t st_ino; /* File serial number.	*/
	__mode_t st_mode; /* File mode.  */
	__nlink_t st_nlink; /* Link count.  */
	__uid_t st_uid; /* User ID of the file's owner.	*/
	__gid_t st_gid; /* Group ID of the file's group.*/
	__dev_t st_rdev; /* Device number, if device.  */
	unsigned long __pad1;
	__off64_t st_size; /* Size of file, in bytes.  */
	__blksize_t st_blksize; /* Optimal block size for I/O.  */
	unsigned int __pad2;
	__blkcnt64_t st_blocks; /* Number 512-byte blocks allocated. */
	/* Nanosecond resolution timestamps are stored in a format
     equivalent to 'struct timespec'.  This is the type used
     whenever possible but the Unix namespace rules do not allow the
     identifier 'timespec' to appear in the <sys/stat.h> header.
     Therefore we have to handle the use of this header in strictly
     standard-compliant sources special.  */
	struct timespec st_atim; /* Time of last access.  */
	struct timespec st_mtim; /* Time of last modification.  */
	struct timespec st_ctim; /* Time of last status change.  */
#define st_atime st_atim.tv_sec /* Backward compatibility.  */
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
	__syscall_slong_t __glibc_reserved[3];
} __attribute__((packed));

struct timeval {
	long tv_sec;
	long tv_usec;
};
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

#endif /* UTEE_TYPES_H */
