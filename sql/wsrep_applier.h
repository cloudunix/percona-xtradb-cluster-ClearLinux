/* Copyright 2013-2019 Codership Oy <http://www.codership.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#ifndef WSREP_APPLIER_H
#define WSREP_APPLIER_H

#include "sql_class.h"  // THD class
#include "rpl_rli.h"   // Relay_log_info
#include "log_event.h" // Format_description_log_event

int wsrep_apply_events(THD *thd, Relay_log_info *rli, const void *events_buf,
                       size_t buf_len);

/* Applier error codes, when nothing better is available. */
#define WSREP_RET_SUCCESS 0     // Success
#define WSREP_ERR_GENERIC 1     // When in doubt (MySQL default error code)
#define WSREP_ERR_BAD_EVENT 2   // Can't parse event
#define WSREP_ERR_NOT_FOUND 3   // Key. table, schema not found
#define WSREP_ERR_EXISTS 4      // Key, table, schema already exists
#define WSREP_ERR_WRONG_TYPE 5  // Incompatible data type
#define WSREP_ERR_FAILED 6      // Operation failed for some internal reason
#define WSREP_ERR_ABORTED 7     // Operation was aborted externally

class wsrep_apply_error {
 public:
  wsrep_apply_error() : str_(NULL), len_(0){}
  ~wsrep_apply_error() { ::free(str_); }
  /* stores the current THD error info from the diagnostic area. Works only
   * once, subsequent invocations are ignored in order to preserve the original
   * condition. */
  void store(const THD *thd);
  const char *c_str() const { return str_; }
  size_t length() const { return len_; }
  bool is_null() const { return (c_str() == NULL && length() == 0); }
  wsrep_buf_t get_buf() const {
    wsrep_buf_t ret = {c_str(), length()};
    return ret;
  }

 private:
  char *str_;
  size_t len_;
};

void wsrep_store_error(const THD* thd, wsrep::mutable_buffer& buf);
class Format_description_log_event;
void wsrep_set_apply_format(THD *, Format_description_log_event *);
Format_description_log_event *wsrep_get_apply_format(THD *thd);

#endif /* WSREP_APPLIER_H */
