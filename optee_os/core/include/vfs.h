#ifndef __VFS_H
#define __VFS_H

#define OPTEE_VFS_OPEN		U(0)

#define OPTEE_VFS_CLOSE		U(1)

#define OPTEE_VFS_ACCESS	U(2)

#define OPTEE_VFS_GETCWD	U(3)

#define OPTEE_VFS_STAT	    U(4)

#define OPTEE_VFS_WRITE		U(5)

#define OPTEE_VFS_READ  	U(6)

#define OPTEE_VFS_FCHMOD  	U(7)

#define OPTEE_VFS_FCHOWN  	U(8)

#define OPTEE_VFS_MKDIR		U(9)

#define OPTEE_VFS_RMDIR		U(10)

#define OPTEE_VFS_UNLINK	U(11)

#define OPTEE_VFS_FTRUNCATE	U(12)

#define OPTEE_VFS_GETEUID	U(13)

#define OPTEE_VFS_GETPID	U(14)

#define OPTEE_VFS_GETENV	U(15)

#define OPTEE_VFS_LSEEK 	U(16)

#define OPTEE_VFS_FSYNC 	U(17)

#define OPTEE_VFS_SLEEP 	U(18)

#define OPTEE_VFS_PREAD  	U(19)

#define OPTEE_VFS_GETTIMEOFDAY	U(20)

#define OPTEE_VFS_PWRITE		U(21)

#define OPTEE_VFS_TRUNCATE		U(22)

#define OPTEE_VFS_FSTAT	        U(23)

#define OPTEE_VFS_FCNTL_VOID	U(24)

#define OPTEE_VFS_FCNTL_INT	    U(25)

#define OPTEE_VFS_FCNTL_FLOCK	U(26)

#define OPTEE_VFS_UTIMES    	U(27)

#define OPTEE_VFS_TIME    	    U(28)

#define OPTEE_VFS_USLEEP    	U(29)

#define OPTEE_VFS_FILESIZE		U(30)

#endif /* __VFS_H */