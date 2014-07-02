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
  MTYPE_SYNC = 0,
  MTYPE_CREATE_FOLDER,
  MTYPE_UPDATE_FOLDER,
  MTYPE_DELETE_FOLDER,
  MTYPE_CREATE_FILE,
  MTYPE_UPDATE_FILE,
  MTYPE_DELETE_FILE,
  MTYPE_COMMANDS,
  MTYPE_FILE,
  MTYPE_ACK,
  MTYPE_CMD_DUPLICATION,
  MTYPE_CMD_BALANCING
};

} // namespace fd8protocol

#endif /* FD8PROTOCOL_HPP_ */
