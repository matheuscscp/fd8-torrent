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
#include <cstring>
#include <list>

namespace helpers {

// classes

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

class ByteQueue {
  private:
    std::vector<uint8_t> buf;
  public:
    ByteQueue(size_t size = 0);
    size_t size() const;
    void resize(size_t size);
    void* ptr() const;
    
    ByteQueue& push(const void* data, size_t maxlen);
    ByteQueue& push(const std::string& data, bool withoutNullTermination = false);
    template <typename T> ByteQueue& push(T data) {
      buf.insert(buf.end(), (uint8_t*)&data, ((uint8_t*)&data) + sizeof(T));
      return *this;
    }
    
    size_t pop(void* data, size_t maxlen);
    template <typename T> T pop() {
      T data = 0;
      size_t total = sizeof(T) <= buf.size() ? sizeof(T) : buf.size();
      if (!total)
        return data;
      memcpy((void*)&data, (const void*)&buf[0], total);
      buf.erase(buf.begin(), buf.begin() + total);
      return data;
    }
};

template <> inline ByteQueue& ByteQueue::push<std::string>(std::string data) {
  buf.insert(buf.end(), (uint8_t*)data.c_str(), ((uint8_t*)data.c_str()) + data.size());
  buf.push_back(uint8_t(0));
  return *this;
}

template <> inline std::string ByteQueue::pop<std::string>() {
  std::string data;
  char c;
  while ((c = pop<char>()) != '\0')
    data += c;
  return data;
}

// functions

void openBrowser();
std::list<std::string> explode(const std::string& str, char delim);
std::pair<std::string, std::string> divideFirst(const std::string& str, char delim);
std::pair<std::string, std::string> divideLast(const std::string& str, char delim);

// template functions

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
