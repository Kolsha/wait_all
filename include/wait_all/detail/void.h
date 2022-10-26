// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>

namespace wait_all::detail {

struct Void final {
};


template<typename F, typename ...Args, typename Result = std::invoke_result_t<F, Args...>>
requires (not std::is_void_v<Result>)

inline constexpr Result invoke_void(F &&f, Args &&...args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}


template<typename F, typename ...Args, typename Result = std::invoke_result_t<F, Args...>>
requires(std::is_void_v<Result>)
inline constexpr Void invoke_void(F &&f, Args &&...args) {
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    return Void{};
}

}