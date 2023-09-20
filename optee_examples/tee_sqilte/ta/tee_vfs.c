
// #if !defined(SQLITE_TEST) || SQLITE_OS_UNIX
// #if defined SQLITE_OS_OTHER

#include "sqlite3.h"
#include "demovfs.h"

#include <assert.h>
#include <string.h>
#include <utee_syscalls.h>
#include <utee_types.h>

SQLITE_API int sqlite3_os_end(void)
{
    return SQLITE_OK;
}

SQLITE_API int sqlite3_os_init(void)
{
    sqlite3_vfs_register(sqlite3_demovfs(), 1);
}

/*
** Size of the write buffer used by journal files in bytes.
*/
#ifndef SQLITE_TEEVFS_BUFFERSZ
#define SQLITE_TEEVFS_BUFFERSZ 8192
#endif

/*
** 该VFS的最大路径长度
*/
#define MAXPATHNAME 512

/*
** 替换原有file句柄
*/
typedef struct DemoFile DemoFile;
struct DemoFile
{
    sqlite3_file base; /* 基类。必须是第一个. */
    int fd;            /* 文件描述符 */

    char *aBuffer;             /* 指向 malloc'd 缓冲区的指针 */
    int nBuffer;               /* zBuffer 中数据的有效字节数 */
    sqlite3_int64 iBufferOfst; /* zBuffer文件中的偏移量 */
};

/*
** 直接写入作为第一个参数传递的文件
*/
static int demoDirectWrite(
    DemoFile *p,       /* 句柄 */
    const void *zBuf,  /* 包含要写入的数据的缓冲区 */
    int iAmt,          /* 要写入的数据大小（以字节为单位） */
    sqlite_int64 iOfst /* 要写入的文件偏移量 */
)
{
    // 从 lseek（） 返回值
    off_t ofst;
    // 从 write（） 返回值
    size_t nWrite;

    ofst = utee_vfs_lseek(p->fd, iOfst, SEEK_SET);
    if (ofst != iOfst)
    {
        return SQLITE_IOERR_WRITE;
    }

    nWrite = utee_vfs_write(p->fd, zBuf, iAmt);
    if (nWrite != iAmt)
    {
        return SQLITE_IOERR_WRITE;
    }

    return SQLITE_OK;
}

/*
** 将 DemoFile.aBuffer 缓冲区的内容刷新到磁盘
*/
static int demoFlushBuffer(DemoFile *p)
{
    int rc = SQLITE_OK;
    if (p->nBuffer)
    {
        rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
        p->nBuffer = 0;
    }
    return rc;
}

/*
** 关闭文件
*/
static int demoClose(sqlite3_file *pFile)
{
    int rc;
    DemoFile *p = (DemoFile *)pFile;
    rc = demoFlushBuffer(p);
    sqlite3_free(p->aBuffer);
    utee_vfs_close(p->fd);
    return rc;
}

/*
** 从文件中读取数据
*/
static int demoRead(
    sqlite3_file *pFile,
    void *zBuf,
    int iAmt,
    sqlite_int64 iOfst)
{
    DemoFile *p = (DemoFile *)pFile;
    // lseek（）返回值
    off_t ofst;
    // read（）返回值
    int nRead;
    // emoFlushBuffer（）返回码
    int rc;

    // 将写入缓冲区中的任何数据刷新到磁盘，以防此操作尝试读取缓冲区中当前缓存的文件区域的数据。
    rc = demoFlushBuffer(p);
    if (rc != SQLITE_OK)
    {
        return rc;
    }

    ofst = utee_vfs_lseek(p->fd, iOfst, SEEK_SET);
    if (ofst != iOfst)
    {
        return SQLITE_IOERR_READ;
    }
    nRead = utee_vfs_read(p->fd, zBuf, iAmt);

    if (nRead == iAmt)
    {
        return SQLITE_OK;
    }
    else if (nRead >= 0)
    {
        if (nRead < iAmt)
        {
            memset(&((char *)zBuf)[nRead], 0, iAmt - nRead);
        }
        return SQLITE_IOERR_SHORT_READ;
    }

    return SQLITE_IOERR_READ;
}

/*
** 将数据写入文件
*/

static int demoWrite(
    sqlite3_file *pFile,
    const void *zBuf,
    int iAmt,
    sqlite_int64 iOfst)
{
    DemoFile *p = (DemoFile *)pFile;

    if (p->aBuffer)
    {
        // 指向要写入的剩余数据的指针
        char *z = (char *)zBuf;
        // zBuf的字节数
        int n = iAmt;
        // 要写入的文件偏移量
        sqlite3_int64 i = iOfst;

        while (n > 0)
        {
            // 要复制到缓冲区的字节数
            int nCopy;

            // 如果缓冲区已满，或者此数据未直接写入，先lfush
            // 在已缓冲的数据之后，刷新缓冲区。
            // 如果缓冲区为空，则为无操作.
            if (p->nBuffer == SQLITE_TEEVFS_BUFFERSZ || p->iBufferOfst + p->nBuffer != i)
            {
                int rc = demoFlushBuffer(p);
                if (rc != SQLITE_OK)
                {
                    return rc;
                }
            }
            assert(p->nBuffer == 0 || p->iBufferOfst + p->nBuffer == i);
            p->iBufferOfst = i - p->nBuffer;

            // 将尽可能多的数据复制到缓冲区中
            nCopy = SQLITE_TEEVFS_BUFFERSZ - p->nBuffer;
            if (nCopy > n)
            {
                nCopy = n;
            }
            memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
            p->nBuffer += nCopy;

            n -= nCopy;
            i += nCopy;
            z += nCopy;
        }
    }
    else
    {
        return demoDirectWrite(p, zBuf, iAmt, iOfst);
    }

    return SQLITE_OK;
}

/*
** no op
*/
static int demoTruncate(sqlite3_file *pFile, sqlite_int64 size)
{
#if 0
  if( ftruncate(((DemoFile *)pFile)->fd, size) ) return SQLITE_IOERR_TRUNCATE;
#endif
    return SQLITE_OK;
}

/*
** 将文件的内容同步
*/
static int demoSync(sqlite3_file *pFile, int flags)
{
    DemoFile *p = (DemoFile *)pFile;
    int rc;

    rc = demoFlushBuffer(p);
    if (rc != SQLITE_OK)
    {
        return rc;
    }

    rc = utee_vfs_fsync(p->fd);
    return (rc == 0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** 将文件的大小（以字节为单位）写入 *pSize
*/
static int demoFileSize(sqlite3_file *pFile, sqlite_int64 *pSize)
{
    DemoFile *p = (DemoFile *)pFile;
    // fstat（） 返回码
    ssize_t rc; 
    // struct my_stat sStat; /* Output of fstat() call */

    // 将缓冲区的内容刷新到磁盘。
    rc = demoFlushBuffer(p);
    if (rc != SQLITE_OK)
    {
        return rc;
    }

    // rc = utee_vfs_fstat(p->fd, &sStat);
    rc = utee_vfs_filesize(p->fd);
    if (rc == -1)
    {
        return SQLITE_IOERR_FSTAT;
    }

    *pSize = rc;
    return SQLITE_OK;
}

/*
** no-op
*/
static int demoLock(sqlite3_file *pFile, int eLock)
{
    return SQLITE_OK;
}
static int demoUnlock(sqlite3_file *pFile, int eLock)
{
    return SQLITE_OK;
}
static int demoCheckReservedLock(sqlite3_file *pFile, int *pResOut)
{
    *pResOut = 0;
    return SQLITE_OK;
}

/*
** no op
*/
static int demoFileControl(sqlite3_file *pFile, int op, void *pArg)
{
    return SQLITE_NOTFOUND;
}

/*
** no op
*/
static int demoSectorSize(sqlite3_file *pFile)
{
    return 0;
}
static int demoDeviceCharacteristics(sqlite3_file *pFile)
{
    return 0;
}

/*
** 打开文件句柄
*/
static int demoOpen(
    sqlite3_vfs *pVfs,   /* VFS */
    const char *zName,   /* 要打开的文件，或临时文件 */
    sqlite3_file *pFile, /* 指向要填充的文件结构的指针 */
    int flags,           /* SQLITE_OPEN_XXX */
    int *pOutFlags       /* SQLITE_OPEN_XXX标志（或null） */
)
{
    static const sqlite3_io_methods demoio = {
        1,                        /* iVersion */
        demoClose,                /* xClose */
        demoRead,                 /* xRead */
        demoWrite,                /* xWrite */
        demoTruncate,             /* xTruncate */
        demoSync,                 /* xSync */
        demoFileSize,             /* xFileSize */
        demoLock,                 /* xLock */
        demoUnlock,               /* xUnlock */
        demoCheckReservedLock,    /* xCheckReservedLock */
        demoFileControl,          /* xFileControl */
        demoSectorSize,           /* xSectorSize */
        demoDeviceCharacteristics /* xDeviceCharacteristics */
    };
    // 初始化句柄
    DemoFile *p = (DemoFile *)pFile; 
    // 要传递给 open（）调用的标志
    int oflags = 0;                 
    char *aBuf = 0;

    if (zName == 0)
    {
        return SQLITE_IOERR;
    }

    if (flags & SQLITE_OPEN_MAIN_JOURNAL)
    {
        aBuf = (char *)sqlite3_malloc(SQLITE_TEEVFS_BUFFERSZ);
        if (!aBuf)
        {
            return SQLITE_NOMEM;
        }
    }

    if (flags & SQLITE_OPEN_EXCLUSIVE)
        oflags |= O_EXCL;
    if (flags & SQLITE_OPEN_CREATE)
        oflags |= O_CREAT;
    if (flags & SQLITE_OPEN_READONLY)
        oflags |= O_RDONLY;
    if (flags & SQLITE_OPEN_READWRITE)
        oflags |= O_RDWR;

    memset(p, 0, sizeof(DemoFile));
    p->fd = utee_vfs_open(zName, oflags, 0600);
    if (p->fd < 0)
    {
        sqlite3_free(aBuf);
        return SQLITE_CANTOPEN;
    }
    p->aBuffer = aBuf;

    if (pOutFlags)
    {
        *pOutFlags = flags;
    }
    p->base.pMethods = &demoio;
    return SQLITE_OK;
}

/*
** 删除由参数 zPath 标识的文件。如果目录同步参数为非零，则确保文件系统修改删除
** 文件在返回之前已同步到磁盘。
*/
static int demoDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync)
{
    int rc; /* Return code */
    int errno = 0;
    rc = utee_vfs_unlink(zPath, &errno);
    if (rc != 0 && errno == ENOENT)
    {
        return SQLITE_OK;
    }

    if (rc == 0 && dirSync)
    {
        // 要传递给 open（） 调用的标志
        int dfd; 
        char *zSlash;
        // 包含文件 zPath 的目录的名称
        char zDir[MAXPATHNAME + 1]; 

        // 从已删除文件的路径中找出目录名称。
        sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
        zDir[MAXPATHNAME] = '\0';
        zSlash = strrchr(zDir, '/');
        if (zSlash)
        {
            // Open a file-descriptor on the directory. Sync. Close.
            zSlash[0] = 0;
            dfd = utee_vfs_open(zDir, O_RDONLY, 0);
            if (dfd < 0)
            {
                rc = -1;
            }
            else
            {
                rc = utee_vfs_fsync(dfd);
                utee_vfs_close(dfd);
            }
        }
    }
    return (rc == 0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#ifndef F_OK
#define F_OK 0
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif

/*
** 查询文件系统以查看命名文件是否存在、可读或既可读又可写。
*/
static int demoAccess(
    sqlite3_vfs *pVfs,
    const char *zPath,
    int flags,
    int *pResOut)
{
    int rc;             
    int eAccess = F_OK; 

    assert(flags == SQLITE_ACCESS_EXISTS      
           || flags == SQLITE_ACCESS_READ      
           || flags == SQLITE_ACCESS_READWRITE 
    );

    if (flags == SQLITE_ACCESS_READWRITE)
        eAccess = R_OK | W_OK;
    if (flags == SQLITE_ACCESS_READ)
        eAccess = R_OK;

    rc = utee_vfs_access(zPath, eAccess);
    *pResOut = (rc == 0);
    return SQLITE_OK;
}

/*
** 参数 zPath 指向包含文件路径的以 nul 结尾的字符串。如果 zPath 是绝对路径，则按原样将其复制到输出
** 缓冲区。否则，如果是相对路径，则等效的全路径写入输出缓冲区。
**
** 此函数假定路径是 UNIX 样式。具体来说，就是：
**
**   1.路径组件由“/”分隔。和
**   2.完整路径以“/”字符开头。
*/
static int demoFullPathname(
    sqlite3_vfs *pVfs, /* VFS */
    const char *zPath, /* 输入路径（可能是相对路径） */
    int nPathOut,      /* 输出缓冲区的大小（以字节为单位） */
    char *zPathOut     /* 指向输出缓冲区的指针 */
)
{
    char zDir[MAXPATHNAME + 1];
    if (zPath[0] == '/')
    {
        zDir[0] = '\0';
    }
    else
    {
        if (utee_vfs_getcwd(zDir, sizeof(zDir)) == 0)
        {
            return SQLITE_IOERR;
        }
    }
    zDir[MAXPATHNAME] = '\0';

    sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
    zPathOut[nPathOut - 1] = '\0';
    return SQLITE_OK;
}

/*
** no op
*/
static void *demoDlOpen(sqlite3_vfs *pVfs, const char *zPath)
{
    return 0;
}
static void demoDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg)
{
    sqlite3_snprintf(nByte, zErrMsg, "Loadable extensions are not supported");
    zErrMsg[nByte - 1] = '\0';
}
static void (*demoDlSym(sqlite3_vfs *pVfs, void *pH, const char *z))(void)
{
    return 0;
}
static void demoDlClose(sqlite3_vfs *pVfs, void *pHandle)
{
    return;
}

/*
** 参数 zByte 指向大小为缓冲区 nByte 字节。填充此内容带有伪随机数据的缓冲区。
*/
static int demoRandomness(sqlite3_vfs *pVfs, int nByte, char *zByte)
{
    return SQLITE_OK;
}

/*
** 睡眠至少n微微秒。返回（近似）数字微秒的睡眠。
*/
static int demoSleep(sqlite3_vfs *pVfs, int nMicro)
{
    utee_vfs_sleep(nMicro / 1000000);
    utee_vfs_usleep(nMicro % 1000000);
    return nMicro;
}

/*
** 将 *pTime 设置为以儒略日表示的当前 UTC 时间。如果成功，则SQLITE_OK，否则为错误代码。
**
**   http://en.wikipedia.org/wiki/Julian_day
**
*/
static int demoCurrentTime(sqlite3_vfs *pVfs, double *pTime)
{
    time_t t = utee_vfs_time(0);
    *pTime = t / 86400.0 + 2440587.5;
    return SQLITE_OK;
}

/*
** 该函数返回指向此文件中实现的 VFS 的指针。
*/
sqlite3_vfs *sqlite3_demovfs(void)
{
    static sqlite3_vfs demovfs = {
        1,                /* iVersion */
        sizeof(DemoFile), /* szOsFile */
        MAXPATHNAME,      /* mxPathname */
        0,                /* pNext */
        "demo",           /* zName */
        0,                /* pAppData */
        demoOpen,         /* xOpen */
        demoDelete,       /* xDelete */
        demoAccess,       /* xAccess */
        demoFullPathname, /* xFullPathname */
        demoDlOpen,       /* xDlOpen */
        demoDlError,      /* xDlError */
        demoDlSym,        /* xDlSym */
        demoDlClose,      /* xDlClose */
        demoRandomness,   /* xRandomness */
        demoSleep,        /* xSleep */
        demoCurrentTime,  /* xCurrentTime */
    };
    return &demovfs;
}

// #endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */

#ifdef SQLITE_TEST

#if defined(INCLUDE_SQLITE_TCL_H)
#include "sqlite_tcl.h"
#else
#include "tcl.h"
#ifndef SQLITE_TCLAPI
#define SQLITE_TCLAPI
#endif
#endif

#if SQLITE_OS_UNIX
static int SQLITE_TCLAPI register_demovfs(
    ClientData clientData, /* 指向sqlite3_enable_XXX函数的指针 */
    Tcl_Interp *interp,    /* 调用此命令的 TCL 解释器 */
    int objc,              /* 参数数 */
    Tcl_Obj *CONST objv[]  /* Command arguments */
)
{
    sqlite3_vfs_register(sqlite3_demovfs(), 1);
    return TCL_OK;
}
static int SQLITE_TCLAPI unregister_demovfs(
    ClientData clientData, 
    Tcl_Interp *interp,   
    int objc,              
    Tcl_Obj *CONST objv[]  
)
{
    sqlite3_vfs_unregister(sqlite3_demovfs());
    return TCL_OK;
}

/*
** 向 TCL 解释器注册命令.
*/
int Sqlitetest_demovfs_Init(Tcl_Interp *interp)
{
    Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
    Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
    return TCL_OK;
}

#else
int Sqlitetest_demovfs_Init(Tcl_Interp *interp)
{
    return TCL_OK;
}
#endif

#endif /* SQLITE_TEST */
