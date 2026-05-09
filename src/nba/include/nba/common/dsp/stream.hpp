/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
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
