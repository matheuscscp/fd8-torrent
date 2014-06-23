/*
 * FD8Protocol.hpp
 *
 *  Created on: Jun 19, 2014
 *      Author: Pimenta
 */

#ifndef FD8PROTOCOL_HPP_
#define FD8PROTOCOL_HPP_

namespace fd8protocol {

enum {
  MTYPE_GET_USERS = 0,
  MTYPE_CREATE_FOLDER,
  MTYPE_UPDATE_FOLDER,
  MTYPE_DELETE_FOLDER
};

} // namespace fd8protocol

#endif /* FD8PROTOCOL_HPP_ */
