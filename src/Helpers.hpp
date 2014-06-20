/*
 * Helpers.hpp
 *
 *  Created on: May 7, 2014
 *      Author: Pimenta
 */

#ifndef HELPERS_HPP_
#define HELPERS_HPP_

// standard
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <sstream>

namespace helpers {

class StaticInitializer {
  public:
    StaticInitializer(std::function<void()> f);
};

class Timer {
  private:
    bool started;
    bool paused;
    uint32_t initialTime;
    uint32_t pauseTime;
  public:
    Timer();
    void start();
    void pause();
    void resume();
    void reset();
    uint32_t time();
    bool counting();
};

void openBrowser();
std::vector<char> readFile(FILE* fp);

template <typename T> std::string toString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

template <typename T> T fromString(const std::string& strValue) {
  std::stringstream ss;
  ss << strValue;
  T value;
  ss >> value;
  return value;
}

} // namespace helpers

#endif /* HELPERS_HPP_ */
