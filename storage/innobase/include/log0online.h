/*****************************************************************************

Copyright (c) 2011-2012, Percona Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA

*****************************************************************************/

/** @file include/log0online.h
Online database log parsing for changed page tracking */

#ifndef log0online_h
#define log0online_h

#include "log0log.h"
#include "os0file.h"
#include "univ.i"

/** Single bitmap file information */
typedef struct log_online_bitmap_file_struct log_online_bitmap_file_t;

/** A set of bitmap files containing some LSN range */
typedef struct log_online_bitmap_file_range_struct
    log_online_bitmap_file_range_t;

/** An iterator over changed page info */
typedef struct log_bitmap_iterator_struct log_bitmap_iterator_t;

/** Initialize the constant part of the log tracking subsystem */
void log_online_init(void) noexcept;

/** Initialize the dynamic part of the log tracking subsystem */
void log_online_read_init(void);

/** Shut down the dynamic part of the log tracking subsystem */
void log_online_read_shutdown(void) noexcept;

/** Shut down the constant part of the log tracking subsystem */
void log_online_shutdown(void) noexcept;

/** Reads and parses the redo log up to last checkpoint LSN to build the
changed page bitmap which is then written to disk.

@return true if log tracking succeeded, false if bitmap write I/O error */
MY_NODISCARD
bool log_online_follow_redo_log_one_pass(void);

/** Read and parse redo log for thr FLUSH CHANGED_PAGE_BITMAPS command.
Make sure that the checkpoint LSN measured at the beginning of the command
is tracked.

@return true if log tracking succeeded, false if bitmap write I/O error */
bool log_online_follow_redo_log(void);

/** Delete all the bitmap files for data less than the specified LSN.
If called with lsn == 0 (i.e. set by RESET request) or LSN_MAX, restart the
bitmap file sequence, otherwise continue it.

@return false to indicate success, true for failure. */
bool log_online_purge_changed_page_bitmaps(
    lsn_t lsn); /*!<in: LSN to purge files up to */

class MetadataRecover;
/** A read-only MetadataRecover instance to support log record parsing */
extern MetadataRecover *log_online_metadata_recover;

#define LOG_BITMAP_ITERATOR_START_LSN(i) ((i).start_lsn)
#define LOG_BITMAP_ITERATOR_END_LSN(i) ((i).end_lsn)
#define LOG_BITMAP_ITERATOR_SPACE_ID(i) ((i).space_id)
#define LOG_BITMAP_ITERATOR_PAGE_NUM(i) ((i).first_page_id + (i).bit_offset)
#define LOG_BITMAP_ITERATOR_PAGE_CHANGED(i) ((i).changed)

/** Initializes log bitmap iterator.  The minimum LSN is used for finding the
correct starting file with records and it there may be records returned by
the iterator that have LSN less than start_lsn.

@return true if the iterator is initialized OK, false otherwise. */
bool log_online_bitmap_iterator_init(
    log_bitmap_iterator_t *i, /*!<in/out:  iterator */
    lsn_t min_lsn,            /*!<in: start LSN for the
                                          iterator */
    lsn_t max_lsn);           /*!<in: end LSN for the
                          iterator */

/** Releases log bitmap iterator. */
void log_online_bitmap_iterator_release(
    log_bitmap_iterator_t *i) noexcept; /*!<in/out:  iterator */

/** Iterates through bits of saved bitmap blocks.
Sequentially reads blocks from bitmap file(s) and interates through
their bits. Ignores blocks with wrong checksum.
@return true if iteration is successful, false if all bits are iterated. */
bool log_online_bitmap_iterator_next(
    log_bitmap_iterator_t *i); /*!<in/out: iterator */

/** Struct for single bitmap file information */
struct log_online_bitmap_file_struct {
  /** Name with full path
      61 is a nice magic constant for the extra space needed for the sprintf
      template in the cc file
  */
  char name[FN_REFLEN + 61];
  pfs_os_file_t file; /*!< Handle to opened file */
  uint64_t size;      /*!< Size of the file */
  os_offset_t offset; /*!< Offset of the next read,
                                        or count of already-read bytes
                                        */
};

/** Struct for a set of bitmap files containing some LSN range */
struct log_online_bitmap_file_range_struct {
  size_t count; /*!< Number of files */
  /*!< Dynamically-allocated array of info about individual files */
  struct files_t {
    char name[FN_REFLEN]; /*!< Name of a file */
    lsn_t start_lsn;      /*!< Starting LSN of data in
                                   this file */
    ulong seq_num;        /*!< Sequence number of	this
                                         file */
  } * files;
};

/** Struct for an iterator through all bits of changed pages bitmap blocks */
struct log_bitmap_iterator_struct {
  lsn_t max_lsn;                           /*!< End LSN of the
                                             range */
  bool failed;                             /*!< Has the iteration
                                                   stopped prematurely */
  log_online_bitmap_file_range_t in_files; /*!< The bitmap files
                                             for this iterator */
  size_t in_i;                             /*!< Currently read
                                                 file index in in_files
                                                 */
  log_online_bitmap_file_t in;             /*!< Currently read
                                               file */
  uint32_t bit_offset;                     /*!< bit offset inside
                                             the current bitmap
                                             block */
  lsn_t start_lsn;                         /*!< Start LSN of the
                                             current bitmap block */
  lsn_t end_lsn;                           /*!< End LSN of the
                                             current bitmap block */
  uint32_t space_id;                       /*!< Current block
                                             space id */
  uint32_t first_page_id;                  /*!< Id of the first
                                             page in the current
                                             block */
  bool last_page_in_run;                   /*!< "Last page in
                                          run" flag value for the
                                          current block */
  bool changed;                            /*!< true if current
                                          page was changed */
  byte *page;                              /*!< Bitmap block */
};

#endif
