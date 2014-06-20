/*
 * Folder.hpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

#ifndef FOLDER_HPP_
#define FOLDER_HPP_

// standard
#include <map>
#include <string>

// local
#include "File.hpp"

struct Folder {
  public:
    std::map<std::string, Folder> subfolders;
    std::map<std::string, File> files;
    int getTotalFiles();
    int getTotalSize();
};

#endif /* FOLDER_HPP_ */
