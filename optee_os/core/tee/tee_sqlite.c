#include <assert.h>
#include <string.h>
#include <optee_rpc_cmd.h>
#include <kernel/thread.h>
#include <mm/mobj.h>
#include <tee/tee_sqlite.h>
#include <vfs.h>
#include <time.h>

int syscall_vfs_open(const char *path, int oflag, int arg1)
{
	int fd = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t len = 0;
	struct mobj *path_mobj;
	char *va_path;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_OPEN, 0, 0);
	// param: const char *path
	len = strlen(path) + 1;
	path_mobj = thread_rpc_alloc_payload(len);
	if (!path_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (path_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_path = mobj_get_va(path_mobj, 0, len);
	memcpy(va_path, path, len);
	params[1] = THREAD_PARAM_MEMREF(IN, path_mobj, 0, 0);
	// params: __oflag
	params[2] = THREAD_PARAM_VALUE(IN, oflag, arg1, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_open err: %x", result);
	}
	fd = params[0].u.value.b;
exit:
	thread_rpc_free_payload(path_mobj);
	return fd;
}

int syscall_vfs_close(int __fd)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];

	memset(params, 0, sizeof(params));
	// param: fd
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_CLOSE, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __fd, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_close err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

int syscall_vfs_access(const char *__name, int __type)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t len = 0;
	struct mobj *name_mobj;
	char *va_name;

	memset(params, 0, sizeof(params));
	// params: pathname
	len = strlen(__name) + 1;
	name_mobj = thread_rpc_alloc_payload(len);
	if (!name_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (name_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_name = mobj_get_va(name_mobj, 0, len);
	memcpy(va_name, __name, len);
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_ACCESS, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __type, 0, 0);
	params[2] = THREAD_PARAM_MEMREF(IN, name_mobj, 0, 0);
	// params: mode, ret
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_access err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(name_mobj);
	return ret;
}

char *syscall_vfs_getcwd(char *__buf, size_t __size)
{
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	struct mobj *buf_mobj;
	char *va_buf;

	memset(params, 0, sizeof(params));
	// params: buf
	buf_mobj = thread_rpc_alloc_payload(__size);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < __size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_buf = mobj_get_va(buf_mobj, 0, __size);
	memset(va_buf, 0, __size);
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_GETCWD, 0, 0);
	params[1] = THREAD_PARAM_MEMREF(OUT, buf_mobj, 0, 0);
	params[2] = THREAD_PARAM_VALUE(IN, __size, 0, 0);
	// params: size
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_getcwd err: %x", result);
	}
	// getcwd返回的结果和buf的一样
	memcpy(__buf, va_buf, __size);
exit:
	thread_rpc_free_payload(buf_mobj);
	return __buf;
}

int syscall_vfs_stat(const char *__file_name, struct my_stat *__buf)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	char *va_filename;
	size_t __file_name_len;
	struct mobj *file_name_mobj;
	size_t __buf_len;
	struct mobj *buf_mobj;
	struct my_stat *va_buf;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_STAT, 0, 0);
	// params: file_name
	__file_name_len = strlen(__file_name) + 1;
	file_name_mobj = thread_rpc_alloc_payload(__file_name_len);
	if (!file_name_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit2;
	} else if (file_name_mobj->size < __file_name_len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit2;
	}
	va_filename = mobj_get_va(file_name_mobj, 0, __file_name_len);
	memcpy(va_filename, __file_name, __file_name_len);
	params[1] = THREAD_PARAM_MEMREF(IN, file_name_mobj, 0, 0);

	// params: buf
	__buf_len = sizeof(*__buf);
	buf_mobj = thread_rpc_alloc_payload(__buf_len);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < __buf_len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	params[2] = THREAD_PARAM_MEMREF(OUT, buf_mobj, 0, 0);
	va_buf = mobj_get_va(buf_mobj, 0, __buf_len);
	memset(va_buf, 0, __buf_len);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_stat err: %x", result);
	}
	memcpy(__buf, va_buf, __buf_len);
	ret = params[0].u.value.b;
	// IMSG("ret: %d", params[0].u.value.a);
	// IMSG("st size: %d", params[0].u.value.b);
	// IMSG("errno: %d", params[0].u.value.c);
	// IMSG("ret: %x", result);
	// IMSG("I-node number:            %ld\n", va_buf->st_ino);
	// IMSG("Mode:                     %lo (octal)\n", (unsigned long)va_buf->st_mode);
	// IMSG("Link count:               %ld\n", (long)va_buf->st_nlink);
	// IMSG("Ownership:                UID=%ld   GID=%ld\n", (long)va_buf->st_uid, (long)va_buf->st_gid);
	// IMSG("device containing file id:%ld \n", (long)va_buf->st_dev);
	// IMSG("device id:                %ld \n", (long)va_buf->st_rdev);
	// IMSG("File size:                %lld bytes\n", (long long)va_buf->st_size);
	// IMSG("Preferred I/O block size: %ld bytes\n", (long)va_buf->st_blksize);
	// IMSG("Blocks allocated:         %lld\n", (long long)va_buf->st_blocks);
exit:
	thread_rpc_free_payload(buf_mobj);
exit2:
	thread_rpc_free_payload(file_name_mobj);
	return ret;
}

int syscall_vfs_fstat(int __fd, struct my_stat *__buf)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t __buf_len;
	struct mobj *buf_mobj;
	struct my_stat *va_buf;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FSTAT, 0, 0);
	// params: fd
	params[1] = THREAD_PARAM_VALUE(IN, __fd, 0, 0);
	// params: buf
	__buf_len = sizeof(*__buf);
	buf_mobj = thread_rpc_alloc_payload(__buf_len);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < __buf_len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	params[2] = THREAD_PARAM_MEMREF(OUT, buf_mobj, 0, 0);
	va_buf = mobj_get_va(buf_mobj, 0, __buf_len);
	memset(va_buf, 0, __buf_len);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fstat err: %x", result);
	}
	memcpy(__buf, va_buf, __buf_len);
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(buf_mobj);
	return ret;
}

ssize_t syscall_vfs_write(int __fd, const void *buf, size_t n)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	size_t len;
	struct mobj *buf_mobj;
	char *va_buf;

	struct thread_param params[3];
	memset(params, 0, sizeof(params));
	// param: fd
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_WRITE, 0, 0);
	// params fd
	params[1] = THREAD_PARAM_VALUE(IN, __fd, n, 0);
	len = n;
	buf_mobj = thread_rpc_alloc_payload(len);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_buf = mobj_get_va(buf_mobj, 0, len);
	memset(va_buf, 0, len);
	memcpy(va_buf, buf, len);
	params[2] = THREAD_PARAM_MEMREF(IN, buf_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_write err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(buf_mobj);
	return ret;
}

int syscall_vfs_mkdir(const char *path, __mode_t mode)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t len;
	struct mobj *path_mobj;
	char *va_path;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_MKDIR, 0, 0);
	// params: pathname
	len = strlen(path) + 1;
	path_mobj = thread_rpc_alloc_payload(len);
	if (!path_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (path_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_path = mobj_get_va(path_mobj, 0, len);
	memcpy(va_path, path, len);
	params[1] = THREAD_PARAM_MEMREF(IN, path_mobj, 0, 0);
	params[2] = THREAD_PARAM_VALUE(IN, mode, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_mkdir err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(path_mobj);
	return ret;
}

int syscall_vfs_rmdir(const char *path)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	size_t len;
	char *va_path;
	struct mobj *path_mobj;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_RMDIR, 0, 0);
	len = strlen(path) + 1;
	// params: pathname
	path_mobj = thread_rpc_alloc_payload(len);
	if (!path_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (path_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_path = mobj_get_va(path_mobj, 0, len);
	memcpy(va_path, path, len);
	params[1] = THREAD_PARAM_MEMREF(IN, path_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_rmdir err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(path_mobj);
	return ret;
}

int syscall_vfs_unlink(const char *__name, int *errno)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	struct mobj *name_mobj;
	char *va_name;
	size_t len;

	memset(params, 0, sizeof(params));
	// params: pathname
	len = strlen(__name) + 1;
	name_mobj = thread_rpc_alloc_payload(len);
	if (!name_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (name_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_name = mobj_get_va(name_mobj, 0, len);
	memcpy(va_name, __name, len);
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_UNLINK, 0, 0);
	params[1] = THREAD_PARAM_MEMREF(IN, name_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_unlink err: %x", result);
	}
	ret = params[0].u.value.b;
	*errno = params[0].u.value.c;
exit:
	thread_rpc_free_payload(name_mobj);
	return ret;
}

int syscall_vfs_ftruncate(int __fd, off_t __size)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	memset(params, 0, sizeof(params));
	// params: fd
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FTRUNCATE, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __fd, __size, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_ftruncate err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

ssize_t syscall_vfs_read(int __fd, void *__buf, size_t __nByte)
{
	ssize_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	char *va_buf;
	struct mobj *buf_mobj;

	memset(params, 0, sizeof(params));
	// params: pathname
	buf_mobj = thread_rpc_alloc_payload(__nByte);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < __nByte) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_buf = mobj_get_va(buf_mobj, 0, __nByte);
	memset(va_buf, 0, __nByte);
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_READ, 0, 0);
	params[1] = THREAD_PARAM_MEMREF(OUT, buf_mobj, 0, 0);
	params[2] = THREAD_PARAM_VALUE(IN, __fd, __nByte, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_read err: %x", result);
	}
	memcpy(__buf, va_buf, __nByte);
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(buf_mobj);
	return ret;
}

int syscall_vfs_fchmod(int __fd, mode_t __mode)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FCHMOD, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __fd, __mode, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fchmod err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

int syscall_vfs_fchown(int __fd, uid_t __owner, gid_t __group)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FCHOWN, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __fd, __owner, __group);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fchown err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

uid_t syscall_vfs_geteuid(void)
{
	uid_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[1];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_GETEUID, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 1, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_geteuid err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

pid_t syscall_vfs_getpid(void)
{
	pid_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[1];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_GETPID, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 1, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_getpid err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

char *syscall_vfs_getenv(const char *name)
{
	char *ret = NULL;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t name_len;
	struct mobj *name_mobj;
	char *va_name;
	size_t ret_len;
	struct mobj *ret_mobj;
	char *va_ret;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_GETENV, 0, 0);
	name_len = strlen(name) + 1;
	name_mobj = thread_rpc_alloc_payload(name_len);
	if (!name_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (name_mobj->size < name_len) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	}
	va_name = mobj_get_va(name_mobj, 0, name_len);
	memcpy(va_name, name, name_len);
	params[1] = THREAD_PARAM_MEMREF(IN, name_mobj, 0, 0);
	params[2] = THREAD_PARAM_MEMREF(OUT, 0, 0, 0);
	// 一定失败, 主要为了获取大小
	result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params);
	ret_len = params[0].u.value.b;
	ret_mobj = thread_rpc_alloc_payload(ret_len);
	if (!ret_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit2;
	} else if (ret_mobj->size < ret_len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit2;
	}
	params[2] = THREAD_PARAM_MEMREF(OUT, ret_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_getenv err: %x", result);
	}
	va_ret = mobj_get_va(ret_mobj, 0, ret_len);
	ret = malloc(ret_len);
	memcpy(ret, va_ret, ret_len);
exit2:
	thread_rpc_free_payload(ret_mobj);
exit:
	thread_rpc_free_payload(name_mobj);
	return ret;
}

off_t syscall_vfs_lseek(int fd, off_t offset, int whence)
{
	off_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_LSEEK, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, fd, offset, whence);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_lseek err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

int syscall_vfs_fsync(int __fd)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FSYNC, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __fd, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fsync err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

unsigned int syscall_vfs_sleep(unsigned int __seconds)
{
	int ret = 0;
	TEE_Result result = TEE_SUCCESS;

	struct thread_param params[2];
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_SLEEP, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, __seconds, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_sleep err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

ssize_t syscall_vfs_pread(int __fd, void *__buf, size_t __nByte, off_t __offset)
{
	ssize_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	struct mobj *buf_mobj;
	char *va_buf;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_PREAD, 0, 0);
	// params: pathname
	buf_mobj = thread_rpc_alloc_payload(__nByte);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < __nByte) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_buf = mobj_get_va(buf_mobj, 0, __nByte);
	memset(va_buf, 0, __nByte);
	params[1] = THREAD_PARAM_MEMREF(IN, buf_mobj, 0, 0);
	params[2] = THREAD_PARAM_VALUE(IN, __fd, __nByte, __offset);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_pread err: %x", result);
	}
	va_buf = mobj_get_va(buf_mobj, 0, __nByte);
	memcpy(__buf, va_buf, __nByte);
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(buf_mobj);
	return ret;
}

ssize_t syscall_vfs_pwrite(int __fd, const void *buf, size_t n, off_t __offset)
{
	ssize_t ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t len;
	struct mobj *buf_mobj;
	char *va_path;

	memset(params, 0, sizeof(params));
	// param: fd
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_PWRITE, 0, 0);
	// params fd
	params[1] = THREAD_PARAM_VALUE(IN, __fd, n, __offset);
	len = n;
	buf_mobj = thread_rpc_alloc_payload(len);
	if (!buf_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (buf_mobj->size < len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_path = mobj_get_va(buf_mobj, 0, len);
	memcpy(va_path, buf, len);
	params[2] = THREAD_PARAM_MEMREF(IN, buf_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_pwrite err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(buf_mobj);
	return ret;
}

int syscall_vfs_truncate(const char *path, off_t __size)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	char *va_path;
	size_t path_len;
	struct mobj *path_mobj;

	memset(params, 0, sizeof(params));
	// params: fd
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FTRUNCATE, 0, 0);
	path_len = strlen(path) + 1;
	path_mobj = thread_rpc_alloc_payload(path_len);
	if (!path_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (path_mobj->size < path_len) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_path = mobj_get_va(path_mobj, 0, path_len);
	memcpy(va_path, path, path_len);
	params[1] = THREAD_PARAM_MEMREF(IN, path_mobj, 0, 0);
	params[2] = THREAD_PARAM_VALUE(IN, __size, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_truncate err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	thread_rpc_free_payload(path_mobj);
	return ret;
}

int syscall_vfs_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[3];
	size_t tv_size;
	size_t tz_size;
	struct mobj *tv_mobj;
	struct mobj *tz_mobj;
	struct timeval *va_tv;
	struct timeval *va_tz;

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_GETTIMEOFDAY, 0, 0);
	tv_size = sizeof(*tv);
	tv_mobj = thread_rpc_alloc_payload(tv_size);
	if (!tv_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (tv_mobj->size < tv_size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_tv = mobj_get_va(tv_mobj, 0, tv_size);
	memset(va_tv, 0, tv_size);

	tz_size = sizeof(*tz);
	tz_mobj = thread_rpc_alloc_payload(tz_size);
	if (!tz_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit2;
	} else if (tz_mobj->size < tz_size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit2;
	}
	va_tz = mobj_get_va(tz_mobj, 0, tz_size);
	memset(va_tz, 0, tz_size);
	params[1] = THREAD_PARAM_MEMREF(IN, tv_mobj, 0, 0);
	params[2] = THREAD_PARAM_MEMREF(IN, tz_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_gettimeofday err: %x", result);
	}
	// 复制回结果
	memcpy(tv, va_tv, tv_size);
	// tz常为null
	if (tz != NULL) {
		memcpy(tz, va_tz, tz_size);
	}
	ret = params[0].u.value.b;
exit2:
	thread_rpc_free_payload(tz_mobj);
exit:
	thread_rpc_free_payload(tv_mobj);
	return ret;
}

int syscall_vfs_fcntl_void(int fd, int cmd)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FCNTL_VOID, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, fd, cmd, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fcntl err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

int syscall_vfs_fcntl_int(int fd, int cmd, int pa)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FCNTL_INT, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, fd, cmd, pa);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fcntl_int err: %x", result);
	}
	ret = params[0].u.value.b;
	return ret;
}

int syscall_vfs_fcntl_flock(int fd, int cmd, struct my_flock *pFlock)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct mobj *flock_mobj;
	struct my_flock *va_flock;
	struct thread_param params[3];
	size_t flock_size;
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FCNTL_FLOCK, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, fd, cmd, 0);
	flock_size = sizeof(*pFlock);
	flock_mobj = thread_rpc_alloc_payload(flock_size);
	if (!flock_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (flock_mobj->size < flock_size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_flock = mobj_get_va(flock_mobj, 0, flock_size);
	memset(va_flock, 0, flock_size);
	params[2] = THREAD_PARAM_MEMREF(IN, flock_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_fcntl_flock err: %x", result);
	}
	ret = params[0].u.value.b;
exit:
	memcpy(pFlock, va_flock, flock_size);
	thread_rpc_free_payload(flock_mobj);
	return ret;
}

int syscall_vfs_utimes(const char *filename, const struct timeval times[2])
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;

	struct thread_param params[3];
	struct mobj *filename_mobj;
	size_t filename_size;
	char *va_filename;

	size_t times_size;
	struct mobj *times_mobj;
	struct timeval *va_times;
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_UTIMES, 0, 0);
	// filename
	filename_size = strlen(filename) + 1;
	filename_mobj = thread_rpc_alloc_payload(filename_size);
	if (!filename_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit;
	} else if (filename_mobj->size < filename_size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit;
	}
	va_filename = mobj_get_va(filename_mobj, 0, filename_size);
	memset(va_filename, 0, filename_size);
	memcpy(va_filename, filename, filename_size);
	params[1] = THREAD_PARAM_MEMREF(IN, filename_mobj, 0, 0);

	times_size = sizeof(times);
	times_mobj = thread_rpc_alloc_payload(times_size);
	if (!times_mobj) {
		result = TEE_ERROR_OUT_OF_MEMORY;
		goto exit1;
	} else if (times_mobj->size < times_size) {
		result = TEE_ERROR_SHORT_BUFFER;
		goto exit1;
	}
	va_times = mobj_get_va(times_mobj, 0, times_size);
	memset(va_times, 0, times_size);
	memcpy(va_times, times, times_size);
	params[2] = THREAD_PARAM_MEMREF(IN, times_mobj, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 3, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_utimes err: %x", result);
	}
	ret = params[0].u.value.b;
exit1:
	thread_rpc_free_payload(times_mobj);
exit:
	thread_rpc_free_payload(filename_mobj);
	return ret;
}

time_t syscall_vfs_time(time_t *t)
{
	int ret = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[1];
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_TIME, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 1, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_time err: %x", result);
	}
	ret = params[0].u.value.b;
	if (t != NULL) {
		memcpy(t, &ret, sizeof(ret));
	}
	return ret;
}

void syscall_vfs_usleep(unsigned long usec)
{
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];
	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_USLEEP, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, usec, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_usleep err: %x", result);
	}
	return;
}

ssize_t syscall_vfs_filesize(int fd)
{
	ssize_t sz = -1;
	TEE_Result result = TEE_SUCCESS;
	struct thread_param params[2];

	memset(params, 0, sizeof(params));
	params[0] = THREAD_PARAM_VALUE(INOUT, OPTEE_VFS_FILESIZE, 0, 0);
	params[1] = THREAD_PARAM_VALUE(IN, fd, 0, 0);
	if ((result = thread_rpc_cmd(OPTEE_RPC_CMD_VFS, 2, params)) !=
	    TEE_SUCCESS) {
		EMSG("syscall_vfs_filesize err: %x", result);
	}
	sz = params[0].u.value.b;
	return sz;
}
