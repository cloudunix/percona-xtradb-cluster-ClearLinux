/* Copyright (c) 2018, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>
#include <algorithm>
#include <memory>
#include <string>
#include "include/my_rnd.h"
#include "sql/stream_cipher.h"

/*
  Tests of replication ciphers used in binary log encryption.
*/

namespace stream_cipher_unittest {

/**
  A test data set that is passed to the test functions.
*/
struct Test_data_set {
  const unsigned char *source_ref = nullptr;
  const int stream_size = 0;
  const unsigned char *encrypted_ref = nullptr;
  const unsigned char *key = nullptr;
  /**
    The test data set constructor.

    @param source_ref_arg The buffer to be encrypted.
    @param stream_size_arg The size of the buffer to be encrypted.
    @param encrypted_ref_arg A buffer with the source_ref_arg buffer encrypted.
    @param key_arg The key that shall be used to open the ciphers.
  */
  Test_data_set(const unsigned char *source_ref_arg, const int stream_size_arg,
                const unsigned char *encrypted_ref_arg = nullptr,
                const unsigned char *key_arg = nullptr)
      : source_ref(source_ref_arg),
        stream_size(stream_size_arg),
        encrypted_ref(encrypted_ref_arg),
        key(key_arg) {}
};

struct Random_test_data_set : public Test_data_set {
  /**
    The random test data set constructor.

    The source_ref_arg buffer will be initialized with random data.

    @param source_ref_arg The buffer to be encrypted.
    @param stream_size_arg The size of the buffer to be encrypted.
  */
  Random_test_data_set(unsigned char *source_ref_arg, const int stream_size_arg)
      : Test_data_set(source_ref_arg, stream_size_arg) {
    my_rand_buffer(source_ref_arg, stream_size_arg);
  }
};

/* Binlog issue */
const unsigned char binlog_issue_ref[] = {
    0xFE, 0x62, 0x69, 0x6E, 0xEB, 0xCB, 0xB5, 0x5B, 0x0F, 0x01, 0x00, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
    0x00, 0x38, 0x2E, 0x30, 0x2E, 0x31, 0x34, 0x2D, 0x74, 0x72, 0x2D, 0x64,
    0x65, 0x62, 0x75, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x0D, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x60, 0x00,
    0x04, 0x1A, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x02, 0x00, 0x00,
    0x00, 0x0A, 0x0A, 0x0A, 0x2A, 0x2A, 0x00, 0x12, 0x34, 0x00, 0x0A, 0x01,
    0xEE, 0xFB, 0xD1, 0x2A, 0xEB, 0xCB, 0xB5, 0x5B, 0x23, 0x01, 0x00, 0x00,
    0x00, 0x1F, 0x00, 0x00, 0x00, 0x9B, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD3, 0xB1, 0xF8, 0xE5, 0xEB,
    0xCB, 0xB5, 0x5B, 0x22, 0x01, 0x00, 0x00, 0x00, 0x4D, 0x00, 0x00, 0x00,
    0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7E, 0xEA, 0xDD, 0xBE, 0x62, 0x77, 0x05, 0xBD, 0x8E, 0x38, 0x01, 0x00,
    0x56, 0x16, 0xCD, 0xBA, 0xEB, 0xCB, 0xB5, 0x5B, 0x02, 0x01, 0x00, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x00, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0D,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x2F, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0xA0, 0x45, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x03, 0x73, 0x74, 0x64, 0x04, 0xFF, 0x00, 0xFF, 0x00,
    0xFF, 0x00, 0x0C, 0x01, 0x74, 0x65, 0x73, 0x74, 0x00, 0x11, 0xCA, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0xFF, 0x00, 0x13, 0x00, 0x74,
    0x65, 0x73, 0x74, 0x00, 0x43, 0x52, 0x45, 0x41, 0x54, 0x45, 0x20, 0x54,
    0x41, 0x42, 0x4C, 0x45, 0x20, 0x74, 0x31, 0x20, 0x28, 0x63, 0x31, 0x20,
    0x49, 0x4E, 0x54, 0x29, 0xFB, 0x98, 0xC5, 0x50, 0xEC, 0xCB, 0xB5, 0x5B,
    0x22, 0x01, 0x00, 0x00, 0x00, 0x4F, 0x00, 0x00, 0x00, 0xA7, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0xE0, 0xE0,
    0xBE, 0x62, 0x77, 0x05, 0xFC, 0x13, 0x01, 0x8E, 0x38, 0x01, 0x00, 0xA1,
    0xD4, 0x48, 0xBC, 0xEC, 0xCB, 0xB5, 0x5B, 0x02, 0x01, 0x00, 0x00, 0x00,
    0x4D, 0x00, 0x00, 0x00, 0xF4, 0x01, 0x00, 0x00, 0x08, 0x00, 0x0D, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x1F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0xA0, 0x45, 0x00, 0x00, 0x00,
    0x00, 0x06, 0x03, 0x73, 0x74, 0x64, 0x04, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x00, 0x12, 0xFF, 0x00, 0x13, 0x00, 0x74, 0x65, 0x73, 0x74, 0x00, 0x42,
    0x45, 0x47, 0x49, 0x4E, 0xA6, 0x27, 0x90, 0x66, 0xEC, 0xCB, 0xB5, 0x5B,
    0x13, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x24, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
    0x74, 0x65, 0x73, 0x74, 0x00, 0x02, 0x74, 0x31, 0x00, 0x01, 0x03, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x9E, 0xBA, 0xF3, 0xF0, 0xEC, 0xCB, 0xB5, 0x5B,
    0x1E, 0x01, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x4C, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02,
    0x00, 0x01, 0xFF, 0x00, 0x02, 0x00, 0x00, 0x00, 0x6A, 0x07, 0xB0, 0x80,
    0xEC, 0xCB, 0xB5, 0x5B, 0x10, 0x01, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00,
    0x00, 0x6B, 0x02, 0x00, 0x00, 0x00, 0x00, 0xCB, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x30, 0xFF, 0xD6, 0x8A};
const unsigned char binlog_issue_encrypted[] = {
    0xA0, 0xC9, 0x08, 0x5D, 0x2C, 0x1D, 0x8C, 0x49, 0x12, 0x90, 0xA6, 0x59,
    0xEB, 0xE8, 0xB6, 0xE4, 0x96, 0x41, 0x80, 0x82, 0xD2, 0xD9, 0x37, 0xDE,
    0x26, 0x52, 0xA4, 0xFE, 0xB8, 0xE0, 0x40, 0x6A, 0x8B, 0xC4, 0xDF, 0x25,
    0x8E, 0xDC, 0x18, 0xD6, 0x87, 0x90, 0xD6, 0xFD, 0xE5, 0xDA, 0xE2, 0xD2,
    0x8A, 0x5F, 0x21, 0x9F, 0xFD, 0xAB, 0x87, 0xAE, 0xA2, 0xDD, 0x89, 0x60,
    0xDC, 0x01, 0x58, 0xCE, 0xE0, 0xB1, 0xC3, 0x66, 0x2D, 0xF5, 0x71, 0x3B,
    0xB2, 0xD8, 0xE3, 0x57, 0xEF, 0x27, 0x6F, 0x18, 0xE1, 0xF7, 0xB3, 0x39,
    0x45, 0x8D, 0x9C, 0x22, 0xB7, 0x2C, 0x02, 0x8D, 0x0C, 0xCB, 0x02, 0xFE,
    0xFC, 0xE0, 0xEB, 0xCB, 0x52, 0xE0, 0x5D, 0x47, 0xD3, 0x08, 0xB1, 0x8F,
    0x75, 0xEC, 0xF1, 0xD0, 0xA9, 0x52, 0xEF, 0x95, 0x82, 0x0F, 0xF5, 0x03,
    0x7D, 0x5B, 0x10, 0x61, 0x6D, 0x1D, 0xAA, 0x84, 0x55, 0x95, 0x89, 0x6B,
    0x1C, 0xD5, 0x76, 0x16, 0xF0, 0x32, 0x6C, 0xA1, 0x0D, 0xD4, 0xDB, 0x97,
    0x66, 0x8E, 0xED, 0xB5, 0xAE, 0xFD, 0x18, 0xA6, 0xBC, 0x62, 0x08, 0x92,
    0x03, 0xDE, 0x2E, 0xAA, 0x65, 0xB5, 0x16, 0x6B, 0x15, 0xD2, 0x3C, 0x9F,
    0x76, 0xEE, 0x71, 0x55, 0x51, 0x4F, 0xC0, 0xE7, 0x3B, 0x5E, 0xD0, 0xC5,
    0x01, 0x7C, 0xEC, 0xEF, 0x3F, 0xE6, 0xBD, 0x8F, 0xA7, 0xBE, 0x8F, 0x78,
    0xB1, 0xCC, 0x18, 0x5A, 0x32, 0x32, 0x39, 0x42, 0x7F, 0xA6, 0x8D, 0xBF,
    0xBD, 0x62, 0xD3, 0x26, 0x0C, 0xEC, 0xDF, 0x08, 0x12, 0xD7, 0x6F, 0x8B,
    0xE8, 0x1A, 0x79, 0x83, 0xD0, 0x10, 0xFB, 0xED, 0x62, 0x4A, 0x0E, 0xA6,
    0xE9, 0x0C, 0xFD, 0x05, 0x59, 0x79, 0xD4, 0x8D, 0x0B, 0x34, 0xC9, 0x5E,
    0xE1, 0x00, 0x04, 0xCA, 0x7B, 0xC8, 0xEE, 0x89, 0x40, 0xA4, 0x14, 0xF6,
    0x8C, 0xEE, 0xFD, 0x66, 0x11, 0x7E, 0x5B, 0x86, 0xBE, 0x48, 0xC6, 0x25,
    0x7A, 0x9E, 0x2C, 0x91, 0x9C, 0xB2, 0x08, 0xA4, 0x66, 0x39, 0xE4, 0x91,
    0x1C, 0x80, 0xB4, 0xFD, 0xC7, 0x52, 0x97, 0xC7, 0x38, 0x84, 0x1D, 0x76,
    0xEA, 0x4A, 0x71, 0xEA, 0x04, 0xCE, 0xEB, 0x28, 0x2B, 0xDC, 0x76, 0x9F,
    0xBC, 0x3D, 0x43, 0xF5, 0xC9, 0x1F, 0x62, 0xDF, 0x38, 0xF8, 0x4F, 0xBE,
    0x1B, 0xD2, 0x0A, 0x12, 0xA3, 0x47, 0x95, 0x59, 0x6C, 0x7E, 0x37, 0x98,
    0xA1, 0x15, 0x2A, 0x24, 0x74, 0x8F, 0x18, 0x97, 0xCC, 0x2A, 0x19, 0xF6,
    0xBC, 0x76, 0x1F, 0xCA, 0x5B, 0xB1, 0xB9, 0xE3, 0xB3, 0xA3, 0x37, 0x37,
    0x23, 0x70, 0x91, 0xC2, 0x9B, 0xB8, 0x15, 0xB3, 0xA4, 0x6E, 0x4E, 0x16,
    0xAE, 0xAA, 0x4D, 0x3C, 0x3E, 0x97, 0x83, 0xD0, 0x2D, 0x0D, 0x67, 0x72,
    0xA0, 0x0C, 0x7B, 0x45, 0x55, 0xC6, 0xDC, 0x62, 0x73, 0xDC, 0xC7, 0xEB,
    0x8B, 0x95, 0x07, 0x4F, 0xD4, 0xAB, 0xC9, 0x42, 0x25, 0x1D, 0x7D, 0xAE,
    0xFC, 0x3E, 0x8A, 0x4B, 0x83, 0xF9, 0xF9, 0xA4, 0xE4, 0xEB, 0xA6, 0x28,
    0x29, 0x48, 0x55, 0x83, 0x58, 0xB9, 0x74, 0x2E, 0x38, 0xD2, 0x30, 0xEC,
    0x5E, 0x61, 0x68, 0xDB, 0x94, 0x00, 0x6D, 0xE6, 0x01, 0xB2, 0x5F, 0x29,
    0xC3, 0x6D, 0xBB, 0x5D, 0xD8, 0x17, 0x45, 0xB2, 0xA7, 0xDE, 0x84, 0x79,
    0xDD, 0xFF, 0x6C, 0x9E, 0x50, 0x3A, 0xF5, 0x97, 0x45, 0x0A, 0x87, 0xC1,
    0x38, 0x37, 0x3A, 0x18, 0x0E, 0x31, 0x78, 0x47, 0x3C, 0xC1, 0x05, 0x35,
    0x48, 0x3B, 0xCD, 0x01, 0x62, 0x03, 0x3A, 0x62, 0xB3, 0x3B, 0xF1, 0xC8,
    0x82, 0x13, 0x7E, 0x25, 0xD0, 0x61, 0xAD, 0x8A, 0x89, 0x0E, 0xDE, 0x33,
    0xD4, 0x83, 0x0F, 0x6C, 0x97, 0x4A, 0xEC, 0xA4, 0x7C, 0x74, 0xC9, 0xCA,
    0x50, 0x08, 0x56, 0x15, 0x9C, 0xE3, 0xC0, 0x1E, 0xF5, 0x43, 0x6B, 0x98,
    0xFD, 0x1B, 0x33, 0x74, 0xCD, 0xD7, 0x5D, 0xBB, 0xC6, 0x54, 0x3B, 0xB3,
    0x8B, 0x53, 0x86, 0x3C, 0x31, 0x83, 0x3B, 0xD1, 0xC5, 0x77, 0x0B, 0x82,
    0x7F, 0x5F, 0xC2, 0x8A, 0x4D, 0x32, 0xD9, 0x6A, 0x3C, 0x9C, 0xB0, 0x0D,
    0x53, 0x34, 0xFE, 0xDB, 0x47, 0xB5, 0x9D, 0xD1, 0x76, 0x9A, 0x4E, 0x4C,
    0x41, 0xF2, 0xB6, 0xA1, 0x24, 0x57, 0x66, 0xC7, 0xAE, 0x73, 0x15, 0x7A,
    0x53, 0x07, 0xE4, 0x63, 0xB0, 0x5B, 0xE0, 0x84, 0x46, 0xDE, 0x49, 0x59,
    0xDA, 0xA3, 0x31, 0xFC, 0x01, 0xA8, 0x3D, 0x58, 0x48, 0xB4, 0xCD, 0x37,
    0x01, 0x67, 0x50, 0x48, 0xB2, 0x81, 0x85, 0x79, 0x84, 0x1F, 0x49, 0x38,
    0x30, 0x6A, 0x7D, 0x48, 0x6D, 0xF2, 0x1D};
const int binlog_issue_size = sizeof(binlog_issue_encrypted);
Test_data_set binlog_issue(binlog_issue_ref, binlog_issue_size,
                           binlog_issue_encrypted);

/* Data for the Basic tests */
const int basic_size = 128;
const unsigned char basic_source_ref[basic_size] = {
    0xFE, 0x62, 0x69, 0x6E, 0xEB, 0xCB, 0xB5, 0x5B, 0x0F, 0x01, 0x00, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
    0x00, 0x38, 0x2E, 0x30, 0x2E, 0x31, 0x34, 0x2D, 0x74, 0x72, 0x2D, 0x64,
    0x65, 0x62, 0x75, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x0D, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x60, 0x00,
    0x04, 0x1A, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x02, 0x00, 0x00,
    0x00, 0x0A, 0x0A, 0x0A, 0x2A, 0x2A, 0x00, 0x12, 0x34, 0x00, 0x0A, 0x01,
    0xEE, 0xFB, 0xD1, 0x2A, 0xEB, 0xCB, 0xB5, 0x5B};
/* Encrypted references are also available to Basic tests */
const unsigned char aes_ctr_encrypted_ref[] = {
    0xa0, 0xc9, 0x08, 0x5d, 0x2c, 0x1d, 0x8c, 0x49, 0x12, 0x90, 0xa6, 0x59,
    0xeb, 0xe8, 0xb6, 0xe4, 0x96, 0x41, 0x80, 0x82, 0xd2, 0xd9, 0x37, 0xde,
    0x26, 0x52, 0xa4, 0xfe, 0xb8, 0xe0, 0x40, 0x6a, 0x8b, 0xc4, 0xdf, 0x25,
    0x8e, 0xdc, 0x18, 0xd6, 0x87, 0x90, 0xd6, 0xfd, 0xe5, 0xda, 0xe2, 0xd2,
    0x8a, 0x5f, 0x21, 0x9f, 0xfd, 0xab, 0x87, 0xae, 0xa2, 0xdd, 0x89, 0x60,
    0xdc, 0x01, 0x58, 0xce, 0xe0, 0xb1, 0xc3, 0x66, 0x2d, 0xf5, 0x71, 0x3b,
    0xb2, 0xd8, 0xe3, 0x57, 0xef, 0x27, 0x6f, 0x18, 0xe1, 0xf7, 0xb3, 0x39,
    0x45, 0x8d, 0x9c, 0x22, 0xb7, 0x2c, 0x02, 0x8d, 0x0c, 0xcb, 0x02, 0xfe,
    0xfc, 0xe0, 0xeb, 0xcb, 0x52, 0xe0, 0x5d, 0x47, 0xd3, 0x08, 0xb1, 0x8f,
    0x75, 0xec, 0xf1, 0xd0, 0xa9, 0x52, 0xef, 0x95, 0x82, 0x0f, 0xf5, 0x03,
    0x7d, 0x5b, 0x10, 0x61, 0x6d, 0x1d, 0xaa, 0x84};
Test_data_set aes_ctr_basic(basic_source_ref, basic_size,
                            aes_ctr_encrypted_ref);

/* Same password used to protect rpl_nogtid_encryption_master-bin.000002 */
const unsigned char aes_ctr_password[] = {
    0x14, 0x99, 0x6d, 0x52, 0xe3, 0x49, 0x4d, 0xb7, 0x81, 0x6a, 0x77,
    0x5e, 0xa9, 0x05, 0x28, 0x17, 0xd3, 0x98, 0x31, 0x6d, 0x17, 0xd4,
    0xd8, 0xee, 0xfa, 0x3b, 0x24, 0xf3, 0x96, 0x50, 0x32, 0x5c};
const unsigned char aes_ctr_password_encrypted_ref[] = {
    0x46, 0x94, 0x9a, 0xe7, 0x5e, 0x35, 0xe5, 0x41, 0x50, 0x67, 0xd1, 0x99,
    0x16, 0xf9, 0x4f, 0x2c, 0xcf, 0x7c, 0x66, 0xaf, 0x11, 0x11, 0x4a, 0x04,
    0x08, 0x08, 0x81, 0x46, 0xed, 0x2d, 0x73, 0x16, 0xf0, 0x64, 0x2a, 0xdc,
    0xf8, 0x02, 0xf2, 0x06, 0x48, 0xfa, 0xd8, 0xa1, 0x38, 0xcb, 0x0c, 0x00,
    0x33, 0x1c, 0x82, 0x0d, 0x13, 0x1b, 0x92, 0x4d, 0x1d, 0xcb, 0x2b, 0x03,
    0x9d, 0x33, 0x06, 0x16, 0x59, 0x4f, 0xc7, 0x76, 0xb8, 0x68, 0x88, 0x6a,
    0x9c, 0xa3, 0x10, 0xfa, 0x8c, 0x0e, 0xfc, 0xe0, 0x43, 0x05, 0x1a, 0xe7,
    0x9c, 0x51, 0x18, 0xc0, 0x3b, 0xd2, 0x34, 0xd0, 0xf4, 0xb9, 0x0e, 0xc6,
    0xf7, 0x75, 0x39, 0xec, 0xd8, 0x98, 0xa6, 0x59, 0xa8, 0x91, 0x60, 0x5c,
    0x1b, 0xa0, 0xc3, 0x1a, 0x68, 0x2a, 0x5b, 0x9c, 0x30, 0x83, 0xb1, 0xe2,
    0xda, 0x30, 0x42, 0x6f, 0xaf, 0x9d, 0xc7, 0xce};
Test_data_set aes_ctr_password_basic(basic_source_ref, basic_size,
                                     aes_ctr_password_encrypted_ref,
                                     aes_ctr_password);

/* Only source reference is available to random tests */
const int max_size = 32 * 1024;
unsigned char max_source_ref[max_size];
Random_test_data_set max(max_source_ref, max_size);

/**
  Sets the key to be used when opening the ciphers.

  @param key_str Key_string to be used when opening the ciphers.
  @param key Key to be assigned to key_str. A key filled with zeros will be
             used when key is null.
*/
template <typename T>
void SetKeyStr(Key_string &key_str, const unsigned char *key) {
  if (key) {
    key_str.assign(key, T::PASSWORD_LENGTH);
  } else {
    const unsigned char new_key[T::PASSWORD_LENGTH]{0};
    key_str.assign(new_key, T::PASSWORD_LENGTH);
  }
}

/**
  A simple test decrypting 4, 19 and 25 bytes in sequence and then decrypting
  the rest of the buffer.

  - 4 = binlog magic;
  - 19 = FDE header;
  - 25 = FDE body;

  @param ds The test data set.
*/
template <typename T>
void Binlog(Test_data_set ds) {
  Key_string key_str;
  SetKeyStr<T>(key_str, ds.key);

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[ds.stream_size]);
  my_rand_buffer(buffer.get(), ds.stream_size);
  /* Decryption will be performed into same encrypted buffer */
  unsigned char *encrypted = buffer.get();
  unsigned char *decrypted = buffer.get();

  std::unique_ptr<Stream_cipher> encryptor = T::get_encryptor();
  std::unique_ptr<Stream_cipher> decryptor = T::get_decryptor();

  /* Encrypt all and compare with encrypted reference if provided */
  EXPECT_FALSE(encryptor->open(key_str, 0));
  EXPECT_FALSE(encryptor->encrypt(encrypted, ds.source_ref, ds.stream_size));
  if (ds.encrypted_ref) {
    EXPECT_FALSE(memcmp(encrypted, ds.encrypted_ref, ds.stream_size));
  }

  /* Decrypt as a stream 4, 19, 25 and the rest of the buffer */
  EXPECT_FALSE(decryptor->open(key_str, 0));
  EXPECT_FALSE(decryptor->decrypt(decrypted, encrypted, 4));
  EXPECT_FALSE(decryptor->decrypt(decrypted + 4, encrypted + 4, 19));
  EXPECT_FALSE(decryptor->decrypt(decrypted + 4 + 19, encrypted + 4 + 19, 25));
  EXPECT_FALSE(decryptor->decrypt(decrypted + (25 + 19 + 4),
                                  encrypted + (25 + 19 + 4),
                                  ds.stream_size - (25 + 19 + 4)));
  EXPECT_FALSE(memcmp(decrypted, ds.source_ref, ds.stream_size));
}

/**
  Encrypt the source to a new buffer and decrypts the new buffer into itself.

  @param ds The test data set.
*/
template <typename T>
void EncryptAndDecrypt(Test_data_set ds) {
  Key_string key_str;
  SetKeyStr<T>(key_str, ds.key);

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[ds.stream_size]);
  my_rand_buffer(buffer.get(), ds.stream_size);
  /* Decryption will be performed into same encrypted buffer */
  unsigned char *encrypted = buffer.get();
  unsigned char *decrypted = buffer.get();

  std::unique_ptr<Stream_cipher> encryptor = T::get_encryptor();
  std::unique_ptr<Stream_cipher> decryptor = T::get_decryptor();

  /* Encrypt all and compare with encrypted reference if provided */
  EXPECT_FALSE(encryptor->open(key_str, 0));
  EXPECT_FALSE(encryptor->encrypt(encrypted, ds.source_ref, ds.stream_size));
  if (ds.encrypted_ref) {
    EXPECT_FALSE(memcmp(encrypted, ds.encrypted_ref, ds.stream_size));
  }

  /* Decrypt all and compare with source reference */
  EXPECT_FALSE(decryptor->open(key_str, 0));
  EXPECT_FALSE(decryptor->decrypt(decrypted, encrypted, ds.stream_size));
  EXPECT_FALSE(memcmp(decrypted, ds.source_ref, ds.stream_size));
}

/**
  This is the standard binary log encryption behavior.

  It feeds the encryptor cipher with a stream in pieces and also feeds the
  decryptor cipher with a stream in pieces.

  Decryption is done to the same buffer provided as its source.

  @param ds The test data set.
*/
template <typename T>
void SequentialEncryptAndDecrypt(Test_data_set ds) {
  Key_string key_str;
  SetKeyStr<T>(key_str, ds.key);

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[ds.stream_size]);
  my_rand_buffer(buffer.get(), ds.stream_size);
  /* Decryption will be performed into same encrypted buffer */
  unsigned char *encrypted = buffer.get();
  unsigned char *decrypted = buffer.get();

  /* Buffer to store an encrypted reference */
  std::unique_ptr<unsigned char[]> full_encrypted_buffer(
      new unsigned char[ds.stream_size]);
  my_rand_buffer(full_encrypted_buffer.get(), ds.stream_size);
  unsigned char *full_encrypted = full_encrypted_buffer.get();

  std::unique_ptr<Stream_cipher> encryptor = T::get_encryptor();
  std::unique_ptr<Stream_cipher> decryptor = T::get_decryptor();

  /* Encrypt all and compare with encrypted reference if provided */
  EXPECT_FALSE(encryptor->open(key_str, 0));
  EXPECT_FALSE(
      encryptor->encrypt(full_encrypted, ds.source_ref, ds.stream_size));
  if (ds.encrypted_ref) {
    EXPECT_FALSE(memcmp(full_encrypted, ds.encrypted_ref, ds.stream_size));
  }

  /* Decrypt all and compare with source reference */
  EXPECT_FALSE(decryptor->open(key_str, 0));
  EXPECT_FALSE(decryptor->decrypt(decrypted, full_encrypted, ds.stream_size));
  EXPECT_FALSE(memcmp(ds.source_ref, decrypted, ds.stream_size));

  /* Encryption and decryption will be performed in step_size chunks */
  for (int step_size = 1;
       step_size <= std::min(ds.stream_size, T::PASSWORD_LENGTH * 8);
       step_size++) {
    my_rand_buffer(buffer.get(), ds.stream_size);
    EXPECT_FALSE(encryptor->set_stream_offset(0));
    EXPECT_FALSE(decryptor->set_stream_offset(0));
    int offset = 0;
    while (offset < ds.stream_size) {
      /* Process next chunk up to the stream size */
      int length = std::min(step_size, ds.stream_size - offset);
      EXPECT_FALSE(encryptor->encrypt(encrypted + offset,
                                      ds.source_ref + offset, length));
      EXPECT_FALSE(memcmp(encrypted + offset, full_encrypted + offset, length));
      EXPECT_FALSE(
          decryptor->decrypt(decrypted + offset, encrypted + offset, length));
      EXPECT_FALSE(memcmp(ds.source_ref + offset, decrypted + offset, length));
      offset = offset + length;
    }
  }
}

/**
  Test if the ciphers handle encrypting and decrypting many chunk sizes.

  Decryption is done to the same buffer provided as its source.

  @param ds The test data set.
*/
template <typename T>
void MultiLengthEncryptAndDecrypt(Test_data_set ds) {
  Key_string key_str;
  SetKeyStr<T>(key_str, ds.key);

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[ds.stream_size]);
  my_rand_buffer(buffer.get(), ds.stream_size);
  /* Decryption will be performed into same encrypted buffer */
  unsigned char *encrypted = buffer.get();
  unsigned char *decrypted = buffer.get();

  /* Buffer to store an encrypted reference */
  std::unique_ptr<unsigned char[]> full_encrypted_buffer(
      new unsigned char[ds.stream_size]);
  my_rand_buffer(full_encrypted_buffer.get(), ds.stream_size);
  unsigned char *full_encrypted = full_encrypted_buffer.get();

  std::unique_ptr<Stream_cipher> encryptor = T::get_encryptor();
  std::unique_ptr<Stream_cipher> decryptor = T::get_decryptor();

  /* Encrypt all and compare with encrypted reference if provided */
  EXPECT_FALSE(encryptor->open(key_str, 0));
  EXPECT_FALSE(
      encryptor->encrypt(full_encrypted, ds.source_ref, ds.stream_size));
  if (ds.encrypted_ref) {
    EXPECT_FALSE(memcmp(full_encrypted, ds.encrypted_ref, ds.stream_size));
  }

  /* Decrypt all and compare with source reference */
  EXPECT_FALSE(decryptor->open(key_str, 0));
  EXPECT_FALSE(decryptor->decrypt(decrypted, full_encrypted, ds.stream_size));
  EXPECT_FALSE(memcmp(ds.source_ref, decrypted, ds.stream_size));

  for (int length = 0; length <= ds.stream_size; length++) {
    my_rand_buffer(buffer.get(), ds.stream_size);
    /* Encrypt the length and compare with encrypted reference */
    EXPECT_FALSE(encryptor->set_stream_offset(0));
    EXPECT_FALSE(encryptor->encrypt(encrypted, ds.source_ref, length));
    EXPECT_FALSE(memcmp(encrypted, full_encrypted, length));
    /* Decrypt the length and compare with source reference */
    EXPECT_FALSE(decryptor->set_stream_offset(0));
    EXPECT_FALSE(decryptor->decrypt(decrypted, encrypted, length));
    EXPECT_FALSE(memcmp(ds.source_ref, decrypted, length));
  }
}

/**
  Test if the ciphers are able to (re)start encrypting in the middle of the
  stream.

  Decryption is done to the same buffer provided as its source.

  @param ds The test data set.
*/
template <typename T>
void SeekAndEncryptAndDecrypt(Test_data_set ds) {
  Key_string key_str;
  SetKeyStr<T>(key_str, ds.key);

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[ds.stream_size]);
  my_rand_buffer(buffer.get(), ds.stream_size);
  /* Decryption will be performed into same encrypted buffer */
  unsigned char *encrypted = buffer.get();
  unsigned char *decrypted = buffer.get();

  /* Buffer to store an encrypted reference */
  std::unique_ptr<unsigned char[]> full_encrypted_buffer(
      new unsigned char[ds.stream_size]);
  my_rand_buffer(full_encrypted_buffer.get(), ds.stream_size);
  unsigned char *full_encrypted = full_encrypted_buffer.get();

  /* The source will be changed during the test */
  std::unique_ptr<unsigned char[]> source_buffer(
      new unsigned char[ds.stream_size]);
  unsigned char *source = source_buffer.get();

  std::unique_ptr<Stream_cipher> encryptor = T::get_encryptor();
  std::unique_ptr<Stream_cipher> decryptor = T::get_decryptor();

  /* Encrypt all and compare with encrypted reference if provided */
  EXPECT_FALSE(encryptor->open(key_str, 0));
  EXPECT_FALSE(
      encryptor->encrypt(full_encrypted, ds.source_ref, ds.stream_size));
  if (ds.encrypted_ref) {
    EXPECT_FALSE(memcmp(full_encrypted, ds.encrypted_ref, ds.stream_size));
  }

  /* Decrypt all and compare with source reference */
  EXPECT_FALSE(decryptor->open(key_str, 0));
  EXPECT_FALSE(decryptor->decrypt(decrypted, full_encrypted, ds.stream_size));
  EXPECT_FALSE(memcmp(ds.source_ref, decrypted, ds.stream_size));

  for (int length = 1;
       length <= std::min(ds.stream_size, T::PASSWORD_LENGTH * 8); length++) {
    for (int offset = 0;
         offset < std::min(ds.stream_size, T::PASSWORD_LENGTH * 8) - length;
         offset++) {
      /* Encrypt all source buffer */
      memcpy(source, ds.source_ref, ds.stream_size);
      EXPECT_FALSE(encryptor->set_stream_offset(0));
      EXPECT_FALSE(encryptor->encrypt(encrypted, source, ds.stream_size));
      /*
        Change some part of the source and encrypt everything to generate an
        encrypted reference of it.
      */
      my_rand_buffer(source + offset, length);
      EXPECT_FALSE(encryptor->set_stream_offset(0));
      EXPECT_FALSE(encryptor->encrypt(full_encrypted, source, ds.stream_size));
      /* Re-encrypt the changed part and compare with encrypted reference */
      EXPECT_FALSE(encryptor->set_stream_offset(offset));
      EXPECT_FALSE(
          encryptor->encrypt(encrypted + offset, source + offset, length));
      EXPECT_FALSE(memcmp(encrypted, full_encrypted, ds.stream_size));
      /* Decrypt everything and compare with source reference */
      EXPECT_FALSE(decryptor->set_stream_offset(0));
      EXPECT_FALSE(decryptor->decrypt(decrypted, encrypted, ds.stream_size));
      EXPECT_FALSE(memcmp(source, decrypted, ds.stream_size));
      /* Decrypt only the changed part and compare with source reference */
      memcpy(encrypted, full_encrypted, ds.stream_size);
      EXPECT_FALSE(decryptor->set_stream_offset(offset));
      EXPECT_FALSE(
          decryptor->decrypt(decrypted + offset, encrypted + offset, length));
      EXPECT_FALSE(memcmp(source + offset, decrypted + offset, length));
    }
  }
}

/* Aes_ctr tests */

TEST(Aes_ctr, BinlogIssue) { Binlog<Aes_ctr>(binlog_issue); }
TEST(Aes_ctr, BinlogIssueMax) { Binlog<Aes_ctr>(max); }

TEST(Aes_ctr, BasicEncryptAndDecrypt) {
  EncryptAndDecrypt<Aes_ctr>(aes_ctr_basic);
}
TEST(Aes_ctr, BasicSequentialEncryptAndDecrypt) {
  SequentialEncryptAndDecrypt<Aes_ctr>(aes_ctr_basic);
}
TEST(Aes_ctr, BasicMultiLengthEncryptAndDecrypt) {
  MultiLengthEncryptAndDecrypt<Aes_ctr>(aes_ctr_basic);
}
TEST(Aes_ctr, BasicSeekAndEncryptAndDecrypt) {
  SeekAndEncryptAndDecrypt<Aes_ctr>(aes_ctr_basic);
}

TEST(Aes_ctr, BasicEncryptAndDecryptKnownPassword) {
  EncryptAndDecrypt<Aes_ctr>(aes_ctr_password_basic);
}
TEST(Aes_ctr, BasicSequentialEncryptAndDecryptKnownPassword) {
  SequentialEncryptAndDecrypt<Aes_ctr>(aes_ctr_password_basic);
}
TEST(Aes_ctr, BasicMultiLengthEncryptAndDecryptKnownPassword) {
  MultiLengthEncryptAndDecrypt<Aes_ctr>(aes_ctr_password_basic);
}
TEST(Aes_ctr, BasicSeekAndEncryptAndDecryptKnownPassword) {
  SeekAndEncryptAndDecrypt<Aes_ctr>(aes_ctr_password_basic);
}

TEST(Aes_ctr, MaxEncryptAndDecrypt) { EncryptAndDecrypt<Aes_ctr>(max); }
TEST(Aes_ctr, MaxSequentialEncryptAndDecrypt) {
  SequentialEncryptAndDecrypt<Aes_ctr>(max);
}
TEST(Aes_ctr, MaxMultiLengthEncryptAndDecrypt) {
  MultiLengthEncryptAndDecrypt<Aes_ctr>(max);
}
TEST(Aes_ctr, MaxSeekAndEncryptAndDecrypt) {
  SeekAndEncryptAndDecrypt<Aes_ctr>(max);
}

}  // namespace stream_cipher_unittest
