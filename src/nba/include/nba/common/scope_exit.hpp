/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba {

template<typename Functor>
struct ScopeExit {
  ScopeExit(Functor&& fn) : fn{fn} {}

  ScopeExit(ScopeExit&& other) = delete;
  ScopeExit(ScopeExit const& other) = delete;

 ~ScopeExit() { fn.operator()(); }

  Functor fn;
};

} // namespace nba