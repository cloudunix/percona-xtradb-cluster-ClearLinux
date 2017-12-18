/*
 * Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

// MySQL DB access module, for use by plugins and others
// For the module that implements interactive DB functionality see mod_db

#ifndef X_CLIENT_MYSQLXCLIENT_XERROR_H_
#define X_CLIENT_MYSQLXCLIENT_XERROR_H_

#include <string>

#include "mysqlxclient/mysqlxclient_error.h"


namespace xcl {

/**
  MySQL error holder.

  The class can holds error codes from:

  * MySQL Server, errors beginning with ER_ prefix
  * client side, errors beginning with CR_ prefix
  * X Protocol, errors beginning with ER_X_ prefix
  * X Client, errors beginning with CR_X_ prefix

  Object created with default constructor sets the error
  code to "0", which means no error.
*/
class XError {
 public:
  XError()
  : m_error(0) {
  }

  explicit XError(const int err, const std::string &message = "")
  : m_message(message),
    m_error(err) {
  }

  /** Check if an error occurred */
  operator bool() const { return 0 != m_error; }

  /** Get error code. */
  int error() const { return m_error; }

  /** Get error description. */
  const char *what() const { return m_message.c_str(); }

 private:
  std::string m_message;
  int m_error;
};

}  // namespace xcl

#endif  // X_CLIENT_MYSQLXCLIENT_XERROR_H_