/*
 * SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
 * SPDX-License-Identifier: GPL-3.0-or-later
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
