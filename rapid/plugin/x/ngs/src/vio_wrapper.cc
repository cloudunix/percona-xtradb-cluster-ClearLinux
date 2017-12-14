/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

#include "plugin/x/ngs/include/ngs/vio_wrapper.h"

namespace ngs {

Vio_wrapper::Vio_wrapper(Vio *vio)
  : m_vio(vio)
{}


ssize_t Vio_wrapper::read(uchar* buffer, ssize_t bytes_to_send) {
  return vio_read(m_vio, buffer, bytes_to_send);
}

ssize_t Vio_wrapper::write(const uchar* buffer, ssize_t bytes_to_send) {
  return vio_write(m_vio, buffer, bytes_to_send);
}

void Vio_wrapper::set_timeout(const Direction direction,
                              const uint32_t timeout) {
  vio_timeout(m_vio, static_cast<uint32_t>(direction), timeout);
}

void Vio_wrapper::set_state(const PSI_socket_state state) {
  MYSQL_SOCKET_SET_STATE(m_vio->mysql_socket, state);
}

void Vio_wrapper::set_thread_owner() {
  mysql_socket_set_thread_owner(m_vio->mysql_socket);

#ifdef USE_PPOLL_IN_VIO
  m_vio->thread_id = my_thread_self();
#endif
}

my_socket Vio_wrapper::get_fd() {
  return vio_fd(m_vio);
}

enum_vio_type Vio_wrapper::get_type() {
  return vio_type(m_vio);
}

sockaddr_storage *Vio_wrapper::peer_addr(std::string &address, uint16 &port) {
  address.resize(256);
  char *buffer = &address[0];

  buffer[0] = 0;

  if (vio_peer_addr(m_vio, buffer, &port, address.capacity()))
    return nullptr;

  address.resize(strlen(buffer));

  return &m_vio->remote;
}

int Vio_wrapper::shutdown() {
  return vio_shutdown(m_vio);
}

Vio_wrapper::~Vio_wrapper() {
  if (m_vio)
    vio_delete(m_vio);
}

} // namespace ngs