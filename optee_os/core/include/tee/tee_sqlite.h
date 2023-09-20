#ifndef TEE_SQLITE_H
#define TEE_SQLITE_H

#include <tee_api_types.h>
#include <utee_types.h>

int syscall_vfs_open(const char *__path, int __oflag, int arg1);

int syscall_vfs_close(int fd);

int syscall_vfs_access(const char *__name, int __type);

char *syscall_vfs_getcwd(char *__buf, size_t __size);

int syscall_vfs_stat(const char *__file_name, struct my_stat *__buf);

int syscall_vfs_fstat(int __fd, struct my_stat *__buf);

int syscall_vfs_fcntl_void(int fd, int cmd);

int syscall_vfs_fcntl_int(int fd, int cmd, int pa);

int syscall_vfs_fcntl_flock(int fd, int cmd, struct my_flock *pFlock);

int syscall_vfs_fchown(int __fd, uid_t __owner, gid_t __group);

int syscall_vfs_truncate(const char *path, off_t __size);

ssize_t syscall_vfs_pread(int __fd, void *__buf, size_t __nByte,
			  off_t __offset);

ssize_t syscall_vfs_pwrite(int __fd, const void *buf, size_t n, off_t __offset);

uid_t syscall_vfs_geteuid(void);

off_t syscall_vfs_lseek(int fd, off_t offset, int whence);

int syscall_vfs_fsync(int __fd);

pid_t syscall_vfs_getpid(void);

unsigned int syscall_vfs_sleep(unsigned int __seconds);

// 内存复制问题
char *syscall_vfs_getenv(const char *name);

int syscall_vfs_ftruncate(int __fd, off_t __size);

ssize_t syscall_vfs_write(int __fd, const void *buf, size_t n);

int syscall_vfs_mkdir(const char *__path, __mode_t mode);

int syscall_vfs_rmdir(const char *__path);

int syscall_vfs_unlink(const char *__path,int *errno);

ssize_t syscall_vfs_read(int fd, void *buf, size_t nByte);

int syscall_vfs_fchmod(int __fd, mode_t __mode);

int syscall_vfs_gettimeofday(struct timeval *tv, struct timezone *tz);

int syscall_vfs_utimes(const char *filename, const struct timeval times[2]);

time_t syscall_vfs_time(time_t *t);

void syscall_vfs_usleep(unsigned long usec);

ssize_t syscall_vfs_filesize(int fd);

#endif