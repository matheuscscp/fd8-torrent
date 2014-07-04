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
  SYNC = 0,
  CREATE_FOLDER,
  UPDATE_FOLDER,
  DELETE_FOLDER,
  CREATE_FILE,
  UPDATE_FILE,
  DELETE_FILE,
  COMMANDS,
  FILE
};

} // namespace fd8protocol

#endif /* FD8PROTOCOL_HPP_ */
