/*
 * Copyright (c) 2015, 2021, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include <my_global.h>

#include "mysql41_hash.h"

#if defined(HAVE_OPENSSL)
#include <openssl/sha.h>

int mysql_mysql41_hash_reset(SHA_CTX *context)
{
    return SHA1_Init(context);
}


int mysql_mysql41_hash_input(SHA_CTX *context, const uint8 *message_array,
                     unsigned length)
{
    return SHA1_Update(context, message_array, length);
}


int mysql_mysql41_hash_result(SHA_CTX *context,
                      uint8 Message_Digest[MYSQL41_HASH_SIZE])
{
    return SHA1_Final(Message_Digest, context);
}

#endif /* HAVE_OPENSSL */

/**
  Wrapper function to compute mysql41_hash message digest.

  @param digest [out]  Computed mysql41_hash digest
  @param buf    [in]   Message to be computed
  @param len    [in]   Length of the message

  @return              void
*/
void compute_mysql41_hash(uint8 *digest, const char *buf, size_t len)
{
#if defined(HAVE_OPENSSL)
  SHA_CTX mysql41_hash_context;

  mysql_mysql41_hash_reset(&mysql41_hash_context);
  mysql_mysql41_hash_input(&mysql41_hash_context, (const uint8 *) buf, len);
  mysql_mysql41_hash_result(&mysql41_hash_context, digest);
#endif /* HAVE_OPENSSL */
}


/**
  Wrapper function to compute mysql41_hash message digest for
  two messages in order to emulate mysql41_hash(msg1, msg2).

  @param digest [out]  Computed mysql41_hash digest
  @param buf1   [in]   First message
  @param len1   [in]   Length of first message
  @param buf2   [in]   Second message
  @param len2   [in]   Length of second message

  @return              void
*/
void compute_mysql41_hash_multi(uint8 *digest, const char *buf1, int len1,
                             const char *buf2, int len2)
{
#if defined(HAVE_OPENSSL)
  SHA_CTX mysql41_hash_context;

  mysql_mysql41_hash_reset(&mysql41_hash_context);
  mysql_mysql41_hash_input(&mysql41_hash_context, (const uint8 *) buf1, len1);
  mysql_mysql41_hash_input(&mysql41_hash_context, (const uint8 *) buf2, len2);
  mysql_mysql41_hash_result(&mysql41_hash_context, digest);
#endif /* HAVE_OPENSSL */
}

