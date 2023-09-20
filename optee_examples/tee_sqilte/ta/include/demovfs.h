#include <sqlite3.h>

#define ENOENT 2

#define O_RDONLY 00
#define O_RDWR 02
#ifndef O_CREAT
#define O_CREAT 0100
#endif
#ifndef O_EXCL
#define O_EXCL 0200

#endif

#define F_GETFD 1
#define F_SETFD 2
#define F_GETLK 5
#define F_SETLK 6

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

#define F_OK 0
#define W_OK 2
#define R_OK 4

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

sqlite3_vfs *sqlite3_demovfs(void);

SQLITE_API int sqlite3_os_end(void);

SQLITE_API int sqlite3_os_init(void);