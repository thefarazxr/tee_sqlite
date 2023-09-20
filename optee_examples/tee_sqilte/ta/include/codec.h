/*
///////////////////////////////////////////////////////////////////////////////
// Name:        codec.h
// Purpose:
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-12-06
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file codec.h Interface of the codec class
*/
// #include <time.h>

#ifndef _CODEC_H_
#define _CODEC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__BORLANDC__)
#define __STDC__ 1
#endif

#if defined(__BORLANDC__)
#undef __STDC__
#endif

#define CODEC_TYPE_CTR128 1
#define CODEC_TYPE_GCM128 2
#ifndef CODEC_TYPE
#define CODEC_TYPE CODEC_TYPE_GCM128
#endif

/*
// ATTENTION: Macro similar to that in pager.c
// TODO: Check in case of new version of SQLite
*/
#define WX_PAGER_MJ_PGNO(x) ((PENDING_BYTE / (x)) + 1)

#ifdef __cplusplus
} /* End of the 'extern "C"' block */
#endif
// #include "rijndael.h"
#define KEYLENGTH 16
#define GCM_IV_LEN 12

#define _MAX_KEY_COLUMNS (256 / 32)
#define _MAX_ROUNDS 14
#define MAX_IV_SIZE 16
/* We assume that unsigned int is 32 bits long....  */
typedef unsigned char UINT8;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
// #define SQLITE_OMIT_LOCALTIME 1
// #define SQLITE_OS_OTHER 1
// #define SQLITE_THREADSAFE 0
// #define SQLITE_TEMP_STORE 3
// #include "sqlite3.c"

// struct Btree {
//   sqlite3 *db;       /* The database connection holding this btree */
//   BtShared *pBt;     /* Sharable content of this btree */
//   u8 inTrans;        /* TRANS_NONE, TRANS_READ or TRANS_WRITE */
//   u8 sharable;       /* True if we can share pBt with another db */
//   u8 locked;         /* True if db currently has pBt locked */
//   int wantToLock;    /* Number of nested calls to sqlite3BtreeEnter() */
//   int nBackup;       /* Number of backup operations reading this btree */
//   Btree *pNext;      /* List of other sharable Btrees from the same db */
//   Btree *pPrev;      /* Back pointer of the same list */
// #ifndef SQLITE_OMIT_SHARED_CACHE
//   BtLock lock;       /* Object used to lock page 1 */
// #endif
// };

/*typedef struct{
    time_t tv_sec;
    syscall_slong_t tv_nsec;
}timespec;*/

// struct Btree {
//   sqlite3 *db;       /* The database connection holding this btree */
//   BtShared *pBt;     /* Sharable content of this btree */
//   u8 inTrans;        /* TRANS_NONE, TRANS_READ or TRANS_WRITE */
//   u8 sharable;       /* True if we can share pBt with another db */
//   u8 locked;         /* True if db currently has pBt locked */
//   int wantToLock;    /* Number of nested calls to sqlite3BtreeEnter() */
//   int nBackup;       /* Number of backup operations reading this btree */
//   Btree *pNext;      /* List of other sharable Btrees from the same db */
//   Btree *pPrev;      /* Back pointer of the same list */
// #ifndef SQLITE_OMIT_SHARED_CACHE
//   BtLock lock;       /* Object used to lock page 1 */
// #endif
// };

unsigned char iv[MAX_IV_SIZE]; // 16?
#include <tee_api_types.h>
typedef struct _Codec
{
  int m_isEncrypted;
  int m_hasReadKey;
  unsigned char m_readKey[KEYLENGTH];
  int m_hasWriteKey;
  unsigned char m_writeKey[KEYLENGTH];
  unsigned char m_tag[16];
  TEE_OperationHandle read_key_op_handle; /* AES ciphering operation */
  TEE_ObjectHandle read_key_handle;   /* transient object to load the key */
  TEE_OperationHandle write_key_op_handle; /* AES ciphering operation */
  TEE_ObjectHandle write_key_handle;   /* transient object to load the key */
  TEE_OperationHandle dec_op_handle; /* AES ciphering operation */
  TEE_ObjectHandle dec_key_handle;   /* transient object to load the key */
  // // Rijndael*     m_aes;
  TEE_OperationHandle test_op_handle; /* AES ciphering operation */
  TEE_ObjectHandle test_key_handle;   /* transient object to load the key */
  Btree *m_bt; /* Pointer to B-tree used by DB */
  unsigned char m_page[SQLITE_MAX_PAGE_SIZE + 24];
} Codec;

void CodecInit(Codec *codec);
void CodecTerm(Codec *codec);

void CodecCopy(Codec *codec, Codec *other);

void CodecGenerateReadKey(Codec *codec, char *userPassword, int passwordLength);

void CodecGenerateWriteKey(Codec *codec, char *userPassword, int passwordLength);

#if CODEC_TYPE == CODEC_TYPE_GCM128
void CodecEncrypt(Codec *codec, int page, unsigned char *data, int len, int useWriteKey, hashmap *tag);
#else
void CodecEncrypt(Codec *codec, int page, unsigned char *data, int len, int useWriteKey);
#endif
#if CODEC_TYPE == CODEC_TYPE_GCM128
void CodecDecrypt(Codec *codec, int page, unsigned char *data, int len, hashmap *tag);
#else
void CodecDecrypt(Codec *codec, int page, unsigned char *data, int len);
#endif

void CodecCopyKey(Codec *codec, int read2write);

void CodecSetIsEncrypted(Codec *codec, int isEncrypted);
void CodecSetHasReadKey(Codec *codec, int hasReadKey);
void CodecSetHasWriteKey(Codec *codec, int hasWriteKey);
void CodecSetBtree(Codec *codec, Btree *bt);

int CodecIsEncrypted(Codec *codec);
int CodecHasReadKey(Codec *codec);
int CodecHasWriteKey(Codec *codec);
Btree *CodecGetBtree(Codec *codec);
unsigned char *CodecGetPageBuffer(Codec *codec);

void CodecGenerateEncryptionKey(Codec *codec, char *userPassword, int passwordLength,
                                unsigned char encryptionKey[KEYLENGTH]);

void CodecPadPassword(Codec *codec, char *password, int pswdlen, unsigned char pswd[32]);

void CodecRC4(Codec *codec, unsigned char *key, int keylen,
              unsigned char *textin, int textlen,
              unsigned char *textout);

void CodecGetMD5Binary(Codec *codec, unsigned char *data, int length, unsigned char *digest);
void CodecWriteTag(Codec *codec, unsigned char *inTag, int nTaglen);
void CodecGetTag(Codec *codec, unsigned char *outTag, int nTaglen);

void CodecGenerateInitialVector(Codec *codec, int seed, unsigned char iv[16]);

void CodecAES(Codec *codec, int page, int encrypt_type, int encrypt, const unsigned char encryptionKey[KEYLENGTH],
              unsigned char *datain, int datalen, unsigned char *dataout, unsigned char *p_mac, int useWriteKey);

#endif
