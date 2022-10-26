// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <wait_all/detail/impl.h>
#include <wait_all/traits.h>


namespace wait_all {

template<typename Result, typename CompletionToken, typename... Fns>
inline auto waitAll(const boost::coroutines::attributes &attr, boost::asio::io_context &io, CompletionToken &&token,
                    Fns &&...functions) {
    return wait_all::detail::waitAll<Result, wait_all::traits::Boost>(attr, io, std::forward<CompletionToken>(token),
                                                                      std::forward<Fns>(functions)...);
}


template<typename Result, typename CompletionToken, typename... Fns>
inline auto waitAll(boost::asio::io_context &io, CompletionToken &&token, Fns &&...functions) {
    return waitAll<Result>(boost::coroutines::attributes{}, io, std::forward<CompletionToken>(token),
                           std::forward<Fns>(functions)...);
}

}

namespace boost::asio {
using wait_all::waitAll;
}
