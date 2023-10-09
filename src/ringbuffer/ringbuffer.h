#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <string.h>

class Data {
 public:
  Data() = default;

  Data(const char* data, size_t size) {
    data_ = new char[size];
    memcpy(data_, data, size);
    size_ = size;
  }

  Data(const Data& obj) {
    data_ = new char[obj.size_];
    memcpy(data_, obj.data_, obj.size_);
    size_ = obj.size_;
  }

  Data& operator=(const Data& obj) {
    data_ = new char[obj.size_];
    memcpy(data_, obj.data_, obj.size_);
    size_ = obj.size_;
    return *this;
  }

  ~Data() {
    if (data_) {
      delete data_;
      data_ = nullptr;
    }
    size_ = 0;
  }

  size_t size() const { return size_; }
  char* data() const { return data_; }

 public:
  char* data_ = nullptr;
  size_t size_ = 0;
};

template <typename T>
class RingBuffer {
 public:
  RingBuffer(unsigned size = 128) : m_size(size), m_front(0), m_rear(0) {
    m_data = new T[size];
  }

  ~RingBuffer() {
    delete[] m_data;
    m_data = nullptr;
  }

  inline bool isEmpty() const { return m_front == m_rear; }

  inline bool isFull() const { return m_front == (m_rear + 1) % m_size; }

  bool push(const T& value) {
    if (isFull()) {
      return false;
    }
    if (!m_data) {
      return false;
    }
    m_data[m_rear] = value;
    m_rear = (m_rear + 1) % m_size;
    return true;
  }

  bool push(const T* value) {
    if (isFull()) {
      return false;
    }
    if (!m_data) {
      return false;
    }
    m_data[m_rear] = *value;
    m_rear = (m_rear + 1) % m_size;
    return true;
  }

  inline bool pop(T& value) {
    if (isEmpty()) {
      return false;
    }
    value = m_data[m_front];
    m_front = (m_front + 1) % m_size;
    return true;
  }

  inline unsigned int front() const { return m_front; }

  inline unsigned int rear() const { return m_rear; }

  inline unsigned int size() const { return m_size; }

 private:
  unsigned int m_size;
  int m_front;
  int m_rear;
  T* m_data;
};

#endif