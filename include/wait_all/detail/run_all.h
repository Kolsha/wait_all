// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <wait_all/detail/run_one.h>

namespace wait_all::detail {


template<typename Traits, typename Guard, std::size_t... Is, typename... Fns>
inline void runAll(std::shared_ptr<Guard> &guard, const boost::coroutines::attributes &attr,
                   boost::asio::io_context &io, std::index_sequence<Is...>, Fns &&...functions) {
    (runOne<Traits>(guard->template getAccess<Is>(), attr, io, std::forward<Fns>(functions)), ...);
}


template<typename Traits, typename Guard, typename FuncsContainer>
inline void runAll(std::shared_ptr<Guard> &guard, const boost::coroutines::attributes &attr,
                   boost::asio::io_context &io, FuncsContainer functions) {
    for (size_t i = 0; i < functions.size(); i++) {
        detail::runOne<Traits>(guard->getAccess(i), attr, io, std::move(functions[i]));
    }
}

}
