/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba {

template<typename T>
struct ReadStream {
  virtual ~ReadStream() = default;

  virtual auto Read() -> T = 0;
};

template<typename T>
struct WriteStream {
  virtual ~WriteStream() = default;
  
  virtual void Write(T const& value) = 0;
};

template<typename T>
struct Stream : ReadStream<T>, WriteStream<T> { };
  
} // namespace nba
