/*
 * Folder.cpp
 *
 *  Created on: Jun 20, 2014
 *      Author: Pimenta
 */

// this
#include "Folder.hpp"

int Folder::getTotalFiles() {
  int totalFiles = files.size();
  for (auto& kv : subfolders)
    totalFiles += kv.second.getTotalFiles();
  return totalFiles;
}

int Folder::getTotalSize() {
  int totalSize = 0;
  for (auto& kv : files)
    totalSize += kv.second.size;
  for (auto& kv : subfolders)
    totalSize += kv.second.getTotalSize();
  return totalSize;
}
