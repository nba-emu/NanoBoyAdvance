/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
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