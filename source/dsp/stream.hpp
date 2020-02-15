/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace DSP {

template <typename T>
class ReadStream {
public:
  virtual ~ReadStream() {};

  virtual auto Read() -> T = 0;
};

template <typename T>
class WriteStream {
public:
  virtual ~WriteStream() {};
  
  virtual void Write(T const& value) = 0;
};

template <typename T>
class Stream : public ReadStream<T>,
               public WriteStream<T>
{ };
  
}