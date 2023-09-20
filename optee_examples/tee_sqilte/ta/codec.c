/*
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Name:        codec.cpp
// Purpose:
// Author:      Ulrich Telle
// Modified by:
// Created:     2006-12-06
// RCS-ID:      $$
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence + RSA Data Security license
///////////////////////////////////////////////////////////////////////////////

/// \file codec.cpp Implementation of MD5, RC4 and AES algorithms
*/
/*
 **********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved. **
 **                                                                  **
 ** License to copy and use this software is granted provided that   **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message     **
 ** Digest Algorithm" in all material mentioning or referencing this **
 ** software or this function.                                       **
 **                                                                  **
 ** License is also granted to make and use derivative works         **
 ** provided that such works are identified as "derived from the RSA **
 ** Data Security, Inc. MD5 Message Digest Algorithm" in all         **
 ** material mentioning or referencing the derived work.             **
 **                                                                  **
 ** RSA Data Security, Inc. makes no representations concerning      **
 ** either the merchantability of this software or the suitability   **
 ** of this software for any particular purpose.  It is provided "as **
 ** is" without express or implied warranty of any kind.             **
 **                                                                  **
 ** These notices must be retained in any copies of any part of this **
 ** documentation and/or software.                                   **
 **********************************************************************
 */
// #include <tee_internal_api.h>
// #include <tee_internal_api_extensions.h>
// #include <utee_types.h>
// #include <tee_api.h>
#include "hashmap.h"
#include "codec.h"
#include <tee_api.h>
#if CODEC_TYPE == CODEC_TYPE_GCM128
hashmap *tag;
// int counter;
#endif

// #define malloc(x) TEE_Malloc(x, 0)
// #define free(x) TEE_Free(x)
// #define realloc(x, y) TEE_Realloc((x), (y))
#define aes_gcm_tag_size 16

/*#if CODEC_TYPE == CODEC_TYPE_AES256
#include "sha2.h"
#include "sha2.c"
#endif*/

/*
// ----------------
// MD5 by RSA
// ----------------

// C headers for MD5
*/
// #include <sys/types.h>
#include <string.h>
// #include <stdio.h>
// #include <stdlib.h>

#ifndef SQLITE_FILE_HEADER /* 123456789 123456 */
#define SQLITE_FILE_HEADER "SQLite format 3"
#endif
/*
/// Structure representing an MD5 context while ecrypting. (For internal use only)
*/

// // Rijndael*     m_aes;

#define AES128_KEY_BIT_SIZE 128
#define AES128_KEY_BYTE_SIZE (AES128_KEY_BIT_SIZE / 8)
#define AES256_KEY_BIT_SIZE 256
#define AES256_KEY_BYTE_SIZE (AES256_KEY_BIT_SIZE / 8)

#define AES_TEST_BUFFER_SIZE 4096
#define AES_TEST_KEY_SIZE 16
#define AES_BLOCK_SIZE 16
// char iv[AES_BLOCK_SIZE];
static long pagecounter = 0;
static bool isInit = false;
#include <utee_syscalls.h>

void createMap()
{
  int fdr;
  IMSG("createMap");
  fdr = utee_vfs_open("/tmp/mac.log", 02 | 0100, 0777);
  IMSG("frd %d", fdr);
  unsigned int *tmp_pageno[100];
  unsigned char *tmp_tag[100];
  tag = hm_create(500, 1.0f, 13); // 1308253
  int i = 0;
  tmp_pageno[i] = (int *)TEE_Malloc(sizeof(int), 0);
  while ((utee_vfs_read(fdr, tmp_pageno[i], sizeof(int)) != 0) && (i < 20000))
  {
    utee_vfs_lseek(fdr, (i + 1) * sizeof(int) + i * 16, 0);
    tmp_tag[i] = (char *)TEE_Malloc(sizeof(char) * 16, 0);
    utee_vfs_read(fdr, tmp_tag[i], 16);
    utee_vfs_lseek(fdr, (i + 1) * sizeof(int) + (i + 1) * 16, 0);
    hm_set(tag, tmp_pageno[i], (void *)tmp_tag[i]);
    i++;
    tmp_pageno[i] = (int *)TEE_Malloc(sizeof(int *), 0);
  }
  IMSG("finish creaMap");
  utee_vfs_close(fdr);
}

void free_int(void *value)
{
  TEE_Free((unsigned char *)value);
}

void closeMap()
{
  IMSG("START CLOSEmAP");
  // #if CODEC_TYPE == CODEC_TYPE_GCM128
  int fdw;
  fdw = utee_vfs_open("/tmp/mac.log", 02 | 01000, 0777);
  IMSG("fd %d", fdw);
  hm_dump(tag, fdw, NULL);
  hm_delete(tag, free_int);
  // fprintf(stdout, "ext counter: %d\n", counter);
  // counter=0;
  //    free(tmp_pageno);
  // free(tmp_tag);
  utee_vfs_close(fdw);
  IMSG("finish closeMap");
  //  #endif //CODEC_TYPE == CODEC_TYPE_GCM128
}

// void init_handler(Codec *codec)
// {
//   TEE_Attribute attr;

//   char *key;
//   if (codec->read_key_op_handle != TEE_HANDLE_NULL)
//     TEE_FreeOperation(codec->read_key_op_handle);

//   /* Allocate operation: AES/CTR, mode and size from params */
//   TEE_Result res = TEE_AllocateOperation(&codec->read_key_op_handle,
//                                          TEE_ALG_AES_GCM,
//                                          TEE_MODE_ENCRYPT,
//                                          AES128_KEY_BYTE_SIZE * 8);

//   if (codec->read_key_handle != TEE_HANDLE_NULL)
//   {
//     TEE_FreeTransientObject(codec->read_key_handle);
//   }

//   /* Allocate transient object according to target key size */
//   res = TEE_AllocateTransientObject(TEE_TYPE_AES,
//                                     AES128_KEY_BYTE_SIZE * 8,
//                                     &codec->read_key_handle);
//   key = TEE_Malloc(AES_TEST_KEY_SIZE, 0);
//   if (!key)
//   {
//     res = TEE_ERROR_OUT_OF_MEMORY;
//     goto err;
//   }
//   TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, AES_TEST_KEY_SIZE);
//   res = TEE_PopulateTransientObject(codec->read_key_handle, &attr, 1);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_PopulateTransientObject failed, %x", res);
//     goto err;
//   }

//   res = TEE_SetOperationKey(codec->read_key_op_handle, codec->read_key_handle);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_SetOperationKey failed %x", res);
//     goto err;
//   }

//   ////////////////////////
//   if (codec->write_key_handle != TEE_HANDLE_NULL)
//     TEE_FreeOperation(codec->write_key_handle);

//   /* Allocate operation: AES/CTR, mode and size from params */
//   TEE_Result res = TEE_AllocateOperation(&codec->write_key_op_handle,
//                                          TEE_ALG_AES_GCM,
//                                          TEE_MODE_ENCRYPT,
//                                          AES128_KEY_BYTE_SIZE * 8);

//   if (codec->write_key_handle != TEE_HANDLE_NULL)
//   {
//     TEE_FreeTransientObject(codec->write_key_handle);
//   }

//   /* Allocate transient object according to target key size */
//   res = TEE_AllocateTransientObject(TEE_TYPE_AES,
//                                     AES128_KEY_BYTE_SIZE * 8,
//                                     &codec->write_key_handle);
//   key = TEE_Malloc(AES_TEST_KEY_SIZE, 0);
//   if (!key)
//   {
//     res = TEE_ERROR_OUT_OF_MEMORY;
//     goto err;
//   }
//   TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, AES_TEST_KEY_SIZE);
//   res = TEE_PopulateTransientObject(codec->write_key_handle, &attr, 1);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_PopulateTransientObject failed, %x", res);
//     goto err;
//   }

//   res = TEE_SetOperationKey(codec->write_key_op_handle, codec->write_key_handle);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_SetOperationKey failed %x", res);
//     goto err;
//   }
//   // TEE_Free()

// err:
//   if (codec->read_key_handle != TEE_HANDLE_NULL)
//   {
//     TEE_FreeOperation(codec->read_key_op_handle);
//   }
//   codec->read_key_op_handle = TEE_HANDLE_NULL;
//   if (codec->write_key_handle != TEE_HANDLE_NULL)
//   {
//     TEE_FreeTransientObject(codec->write_key_handle);
//   }
//   codec->write_key_handle = TEE_HANDLE_NULL;
// }

// void reset_key(TEE_OperationHandle op_handle, TEE_ObjectHandle key_handle, char *key, size_t key_sz)
// {
//   if (!isInit)
//   {
//     isInit = true;
//     init_handler(op_handle, key_handle);
//   }
//   IMSG("op_handle");
//   TEE_Attribute attr;
//   TEE_Result res;
//   TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, key_sz);
//   IMSG("TEE_InitRefAttribute");
//   TEE_ResetTransientObject(key_handle);
//   IMSG("TEE_ResetTransientObject");
//   res = TEE_PopulateTransientObject(key_handle, &attr, 1);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_PopulateTransientObject failed, %x", res);
//     return;
//   }
//   IMSG("TEE_ResetOperation");
//   TEE_ResetOperation(op_handle);
//   res = TEE_SetOperationKey(op_handle, key_handle);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_PopulateTransientObject failed, %x", res);
//     return;
//   }
//   char iv[GCM_IV_LEN];
//   memset(iv, 0, GCM_IV_LEN);
//   IMSG("TEE_AEInit");
//   res = TEE_AEInit(op_handle, iv, GCM_IV_LEN, 128, 0, 0);
//   if (res != TEE_SUCCESS)
//   {
//     EMSG("TEE_PopulateTransientObject failed, %x", res);
//     return;
//   }
// }

void reset(Codec *codec, int isenc, char *key, size_t ket_size)
{
  TEE_Attribute attr;
  if (codec->test_op_handle != TEE_HANDLE_NULL)
  {
    TEE_FreeOperation(codec->test_op_handle);
  }
  int mode;
  if (isenc == 1)
  {
    mode = TEE_MODE_ENCRYPT;
  }
  else
  {
    mode = TEE_MODE_DECRYPT;
  }
  /* Allocate operation: AES/CTR, mode and size from params */
  TEE_Result res = TEE_AllocateOperation(&codec->test_op_handle,
                                         TEE_ALG_AES_GCM,
                                         mode,
                                         AES128_KEY_BYTE_SIZE * 8);

  if (codec->test_key_handle != TEE_HANDLE_NULL)
  {
    TEE_FreeTransientObject(codec->test_key_handle);
  }
  /* Allocate transient object according to target key size */
  res = TEE_AllocateTransientObject(TEE_TYPE_AES,
                                    AES128_KEY_BYTE_SIZE * 8,
                                    &codec->test_key_handle);
  // key = TEE_Malloc(AES_TEST_KEY_SIZE, 0);
  // if (!key)
  // {
  //   res = TEE_ERROR_OUT_OF_MEMORY;
  //   goto err;
  // }

  TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, ket_size);
  res = TEE_PopulateTransientObject(codec->test_key_handle, &attr, 1);
  if (res != TEE_SUCCESS)
  {
    EMSG("TEE_PopulateTransientObject failed, %x", res);
    // goto err;
  }
  res = TEE_SetOperationKey(codec->test_op_handle, codec->test_key_handle);
  if (res != TEE_SUCCESS)
  {
    EMSG("TEE_SetOperationKey failed %x", res);
    // goto err;
  }
  res = TEE_AEInit(codec->test_op_handle, iv, AES_BLOCK_SIZE, 128, 0, 0);
}

void CodecAES(Codec *codec, int page, int encrypt_type, int encrypt, const unsigned char encryptionKey[KEYLENGTH],
              unsigned char *datain, int datalen, unsigned char *dataout, unsigned char *p_mac, int useWriteKey)
{
  // 准备
  // uint8_t ctr_initial[MAX_IV_SIZE];
  uint8_t gcm_initial[GCM_IV_LEN];
  // memset(ctr_initial, 0, MAX_IV_SIZE);
  memset(gcm_initial, 0, GCM_IV_LEN);
  const uint8_t *p_src = (const uint8_t *)datain;
  const uint32_t src_len = datalen;
  uint8_t *p_dsts = (uint8_t *)dataout;
  // const uint32_t ctr_inc_bits = 128;
  int keyLength = KEYLENGTH;

  uint8_t *sgx_keys = (uint8_t *)TEE_Malloc(KEYLENGTH, 0);
  memcpy(sgx_keys, encryptionKey, 16);
  void *p_out_mac = TEE_Malloc(aes_gcm_tag_size, 0);
  memset(p_out_mac, 0, aes_gcm_tag_size);
  // sgx_aes_gcm_128bit_tag_t *p_out_mac = (sgx_aes_gcm_128bit_tag_t *)malloc(sizeof(sgx_aes_gcm_128bit_tag_t));
  // memset(p_out_mac, 0, sizeof(sgx_aes_gcm_128bit_tag_t));
  // sgx_aes_gcm_128bit_tag_t *p_in_mac = (sgx_aes_gcm_128bit_tag_t *)malloc(sizeof(sgx_aes_gcm_128bit_tag_t));
  void *p_in_mac = TEE_Malloc(aes_gcm_tag_size, 0);
  memset(p_in_mac, 0, sizeof(aes_gcm_tag_size));
  size_t tag_len = aes_gcm_tag_size;
  size_t p_src_len = src_len;
  TEE_Result rc;
  char *zErrMsg = 0;

  if (encrypt_type == 1)
  {
    // if (encrypt)
    // {
    //   rc = sgx_aes_ctr_encrypt((const sgx_aes_ctr_128bit_key_t *)sgx_keys, p_src, src_len, ctr_initial, ctr_inc_bits, p_dsts);
    //   if (rc != SGX_SUCCESS)
    //   {
    //     fprintf(stderr, "ctr_enc error: %d\n", rc);
    //   }
    // }
    // else
    // {
    //   rc = sgx_aes_ctr_decrypt((const sgx_aes_gcm_128bit_key_t *)sgx_keys, p_src, src_len, ctr_initial, ctr_inc_bits, p_dsts);
    //   if (rc != SGX_SUCCESS)
    //   {
    //     fprintf(stderr, "ctr_dec error: %d\n", rc);
    //   }
    // }
  }
  else
  {

    // TEE_OperationHandle op = (useWriteKey) ? codec->write_key_op_handle : codec->read_key_op_handle;
    if (encrypt)
    {
      // reset_key(codec, encryptionKey, KEYLENGTH);
      reset(codec, encrypt, (useWriteKey) ? codec->m_writeKey : codec->m_readKey, KEYLENGTH);
      rc = TEE_AEEncryptFinal(codec->test_op_handle, p_src, src_len, p_dsts, &p_src_len, p_out_mac, &tag_len);
      // rc = sgx_rijndael128GCM_encrypt((const sgx_aes_gcm_128bit_key_t *)sgx_keys, p_src, src_len, p_dsts, gcm_initial, GCM_IV_LEN, NULL, 0, p_out_mac);
      // reset(codec, encrypt, (useWriteKey) ? codec->m_writeKey : codec->m_readKey, KEYLENGTH);
      memcpy(p_mac, (char *)p_out_mac, aes_gcm_tag_size);
      if (rc != TEE_SUCCESS)
      {
        EMSG("TEE_AEDecryptFinal ERROR ,%x", rc);
        // fprintf(stderr, "gcm_enc error: %d\n", rc);
        // ;
      }
    }
    else
    {

      reset(codec, encrypt, (useWriteKey) ? codec->m_writeKey : codec->m_readKey, KEYLENGTH);
      // memcpy(p_in_mac, (const sgx_aes_gcm_128bit_tag_t *)p_mac, sizeof(sgx_aes_gcm_128bit_tag_t));
      memcpy(p_in_mac, (char *)p_mac, aes_gcm_tag_size);
      // rc = sgx_rijndael128GCM_decrypt((const sgx_aes_gcm_128bit_key_t *)sgx_keys, p_src, src_len, p_dsts, gcm_initial, GCM_IV_LEN, NULL, 0, (const sgx_aes_gcm_128bit_tag_t *)p_in_mac);
      rc = TEE_AEDecryptFinal(codec->test_op_handle, p_src, src_len, p_dsts, &p_src_len, p_in_mac, aes_gcm_tag_size);
      // fprintf(stdout,"page_index %d\n", page);
      if (rc != TEE_SUCCESS)
      {
        EMSG("TEE_AEDecryptFinal ERROR ,%x", rc);
        // fprintf(stderr, "gcm_dec error: %d\n", rc);
      }
    }
  }
  TEE_Free(sgx_keys);
  TEE_Free(p_out_mac);
  TEE_Free(p_in_mac);
  /* It is a good idea to check the error code */
  // if (len < 0)
  //{
  /* AES: Error on encrypting. */
  //}
}
void CodecInit(Codec *codec)
{
  codec->m_isEncrypted = 0;
  codec->m_hasReadKey = 0;
  codec->m_hasWriteKey = 0;
}

void CodecSetIsEncrypted(Codec *codec, int isEncrypted)
{
  codec->m_isEncrypted = isEncrypted;
}

void CodecSetHasReadKey(Codec *codec, int hasReadKey)
{
  codec->m_hasReadKey = hasReadKey;
}

void CodecSetHasWriteKey(Codec *codec, int hasWriteKey)
{
  codec->m_hasWriteKey = hasWriteKey;
}

void CodecSetBtree(Codec *codec, Btree *bt)
{
  codec->m_bt = bt;
}

int CodecIsEncrypted(Codec *codec)
{
  return codec->m_isEncrypted;
}

int CodecHasReadKey(Codec *codec)
{
  return codec->m_hasReadKey;
}

int CodecHasWriteKey(Codec *codec)
{
  return codec->m_hasWriteKey;
}

Btree *CodecGetBtree(Codec *codec)
{
  return codec->m_bt;
}

void CodecWriteTag(Codec *codec, unsigned char *inTag, int nTaglen)
{
  memcpy(codec->m_tag, inTag, nTaglen);
}
void CodecGetTag(Codec *codec, unsigned char *outTag, int nTaglen)
{
  memcpy(outTag, codec->m_tag, nTaglen);
}

unsigned char *
CodecGetPageBuffer(Codec *codec)
{
  return &codec->m_page[4];
}

void CodecCopy(Codec *codec, Codec *other)
{
  int j;
  codec->m_isEncrypted = other->m_isEncrypted;
  codec->m_hasReadKey = other->m_hasReadKey;
  codec->m_hasWriteKey = other->m_hasWriteKey;
  for (j = 0; j < KEYLENGTH; j++)
  {
    codec->m_readKey[j] = other->m_readKey[j];
    codec->m_writeKey[j] = other->m_writeKey[j];
  }
  for (j = 0; j < 16; j++)
  {
    codec->m_tag[j] = other->m_tag[j];
  }
  codec->m_bt = other->m_bt;
  // RijndaelInvalidate(codec->m_aes);
}

void CodecCopyKey(Codec *codec, int read2write)
{
  int j;
  if (read2write)
  {
    for (j = 0; j < KEYLENGTH; j++)
    {
      codec->m_writeKey[j] = codec->m_readKey[j];
    }
  }
  else
  {
    for (j = 0; j < KEYLENGTH; j++)
    {
      codec->m_readKey[j] = codec->m_writeKey[j];
    }
  }
}

void CodecGenerateReadKey(Codec *codec, char *userPassword, int passwordLength)
{
  // reset_key(codec->read_key_op_handle, codec->read_key_handle, userPassword, passwordLength);
  CodecGenerateEncryptionKey(codec, userPassword, passwordLength, codec->m_readKey);
}

void CodecGenerateWriteKey(Codec *codec, char *userPassword, int passwordLength)
{
  // reset_key(codec->write_key_op_handle, codec->write_key_handle, userPassword, passwordLength);
  CodecGenerateEncryptionKey(codec, userPassword, passwordLength, codec->m_writeKey);
}

void CodecGenerateEncryptionKey(Codec *codec, char *userPassword, int passwordLength,
                                unsigned char encryptionKey[KEYLENGTH])
{
  memcpy(encryptionKey, userPassword, KEYLENGTH);
}

#if CODEC_TYPE == CODEC_TYPE_GCM128
void CodecEncrypt(Codec *codec, int page, unsigned char *data, int len, int useWriteKey, hashmap *tag)
{
  IMSG("codecencrypt");
  unsigned char dbHeader[8];
  int offset = 0;
  unsigned char *key = (useWriteKey) ? codec->m_writeKey : codec->m_readKey;

  int i;
  unsigned char p_mac[16];
  memset(p_mac, 0, 16);
  unsigned char p_in_hash[32];
  memset(p_in_hash, 0, 32);
  unsigned char p_hash[32];
  memset(p_hash, 0, 32);
  if (page == 1)
  {
    memcpy(dbHeader, data + 16, 8);
    offset = 16;
    CodecAES(codec, page, CODEC_TYPE, 1, key, data, 16, data, p_mac, useWriteKey);
    int pageHeader = page - 1;
    IMSG("page no: %d, tag %s",page, p_mac);
    hm_set(tag, &pageHeader, p_mac);
  }
  CodecAES(codec, page, CODEC_TYPE, 1, key, data + offset, len - offset, data + offset, p_mac, useWriteKey);

  hm_set(tag, &page, p_mac);
  IMSG("page no: %d, tag %s",page, p_mac);
  if (page == 1)
  {
    memcpy(data + 8, data + 16, 8);
    memcpy(data + 16, dbHeader, 8);
  }
}
#else
void CodecEncrypt(Codec *codec, int page, unsigned char *data, int len, int useWriteKey)
{
  unsigned char dbHeader[8];
  int offset = 0;
  unsigned char *key = (useWriteKey) ? codec->m_writeKey : codec->m_readKey;
  int i;
  unsigned char p_mac[16];
  memset(p_mac, 0, 16);
  unsigned char p_in_hash[32];
  memset(p_in_hash, 0, 32);
  unsigned char p_hash[32];
  memset(p_hash, 0, 32);
  if (page == 1)
  {
    memcpy(dbHeader, data + 16, 8);
    offset = 16;
    CodecAES(codec, page, CODEC_TYPE, 1, key, data, 16, data, p_mac);
  }
  CodecAES(codec, page, CODEC_TYPE, 1, key, data + offset, len - offset, data + offset, p_mac);
  if (page == 1)
  {
    memcpy(data + 8, data + 16, 8);
    memcpy(data + 16, dbHeader, 8);
  }
}
#endif

#if CODEC_TYPE == CODEC_TYPE_GCM128
void CodecDecrypt(Codec *codec, int page, unsigned char *data, int len, hashmap *tag)
{
  IMSG("CodecDecrypt");
  unsigned char dbHeader[8];
  int dbPageSize;
  unsigned char p_in_mac[16];
  memset(p_in_mac, 0, 16);
  int offset = 0;
  int i = 0;
  int j = 0;

  if (page == 1)
  {
    memcpy(dbHeader, data + 16, 8);
    dbPageSize = (dbHeader[0] << 8) | (dbHeader[1] << 16);
    if ((dbPageSize >= 512) && (dbPageSize <= SQLITE_MAX_PAGE_SIZE) && (((dbPageSize - 1) & dbPageSize) == 0) &&
        (dbHeader[5] == 0x40) && (dbHeader[6] == 0x20) && (dbHeader[7] == 0x20))
    {
      memcpy(data + 16, data + 8, 8);
      offset = 16;
    }
  }
  unsigned char *tmp;
  tmp = (unsigned char *)hm_get(tag, &page);
  IMSG("GET TAG CODE:%d %s",page,  tmp);
  if (tmp == NULL)
  {
    // fprintf(stdout, "page %d not match!\n ", page);
  }
  else
  {
    memcpy(p_in_mac, tmp, 16);
  }

  CodecAES(codec, page, CODEC_TYPE, 0, codec->m_readKey, data + offset, len - offset, data + offset, p_in_mac, 0);
  if (page == 1 && offset != 0)
  {
    if (memcmp(dbHeader, data + 16, 8) == 0)
    {
      memcpy(data, SQLITE_FILE_HEADER, 16);
    }
  }
}
#else
void CodecDecrypt(Codec *codec, int page, unsigned char *data, int len)
{
  unsigned char dbHeader[8];
  int dbPageSize;
  unsigned char p_in_mac[16];
  memset(p_in_mac, 0, 16);
  int offset = 0;
  int i = 0;
  int j = 0;

  if (page == 1)
  {
    memcpy(dbHeader, data + 16, 8);
    dbPageSize = (dbHeader[0] << 8) | (dbHeader[1] << 16);
    if ((dbPageSize >= 512) && (dbPageSize <= SQLITE_MAX_PAGE_SIZE) && (((dbPageSize - 1) & dbPageSize) == 0) &&
        (dbHeader[5] == 0x40) && (dbHeader[6] == 0x20) && (dbHeader[7] == 0x20))
    {
      memcpy(data + 16, data + 8, 8);
      offset = 16;
    }
  }
  CodecAES(codec, page, CODEC_TYPE, 0, codec->m_readKey, data + offset, len - offset, data + offset, p_in_mac);
  if (page == 1 && offset != 0)
  {
    if (memcmp(dbHeader, data + 16, 8) == 0)
    {
      memcpy(data, SQLITE_FILE_HEADER, 16);
    }
  }
}
#endif
