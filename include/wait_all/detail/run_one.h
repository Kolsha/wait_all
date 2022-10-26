// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include <wait_all/detail/handler_info.h>


namespace wait_all::detail {


template<typename Traits, typename F, typename... Args>
inline void runOne(StorageAccess<Args...> storage, const boost::coroutines::attributes &attr,
                   boost::asio::io_context &io, F f) {

    auto worker = [storage = std::move(storage), task = std::move(f)]<typename... Ts>(Ts &&...ts) mutable -> void {
        try {
            storage.setResult(invoke_void(task, std::forward<Ts>(ts)...));
        } catch (const boost::coroutines::detail::forced_unwind &) {
            storage.setError(Traits::getUnexpectedErrorCode(
                    "waitAll::runOne got 'boost::coroutines::detail::forced_unwind'"));
            throw;
        } catch (const typename Traits::SystemError &e) {
            storage.setError(e.code());
        } catch (const std::exception &e) {
            storage.setError(Traits::getUnexpectedErrorCode(e.what()));
        } catch (...) {
            storage.setError(Traits::getUnexpectedErrorCode());
        }
    };

    if constexpr (handler_info<F>::need_context) {
        boost::asio::spawn(io, std::move(worker), attr);
    } else {
        io.post(std::move(worker));
    }
};

}
