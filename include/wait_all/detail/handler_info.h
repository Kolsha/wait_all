// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/asio/spawn.hpp>
#include <type_traits>


namespace wait_all::detail {
template<typename F>
struct handler_info {
    inline static constexpr bool need_context = std::is_invocable_v<F, boost::asio::yield_context>;

    using return_type = typename std::conditional_t<need_context,
            std::invoke_result<F, boost::asio::yield_context>,
            std::invoke_result<F>
    >::type;
};


}
