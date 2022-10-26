// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <wait_all/detail/storage.h>
#include <wait_all/detail/guard.h>
#include <wait_all/detail/run_all.h>


namespace wait_all::detail {

template<typename Result, typename... Fns>
inline constexpr void check_constraints() {
    constexpr bool is_any_void = (... ||  std::is_void_v<typename handler_info<Fns>::return_type>);
    static_assert(
            std::is_void_v<Result> ||
            (!std::is_void_v<Result> && !is_any_void),
            "Either all functions should return meaningful value, or none of them should."
    );
}

template<
        typename Result, typename Traits,
        typename CompletionSignature=typename Traits::template CompletionSignatureT<Result>,
        boost::asio::completion_token_for<CompletionSignature> CompletionToken, typename... Fns
>
requires (sizeof...(Fns) > 1)
inline auto waitAll(const boost::coroutines::attributes &attr, boost::asio::io_context &io,
                    CompletionToken &&token, Fns &&...functions) {
    detail::check_constraints<Result, Fns...>();

    using Container = typename Traits::template StaticContainer<typename detail::handler_info<Fns>::return_type ...>;
    using Data = typename std::conditional_t<std::is_void_v<Result>, void, Container>;
    using Storage = detail::StaticStorage<Result, typename Traits::ErrorCode, Data>;

    auto init = [&io, attr = attr](auto completion_handler, auto &&... functions) {
        using CompletionHandler = std::decay_t<decltype(completion_handler)>;
        using Guard = detail::Guard<CompletionHandler, Storage>;

        auto guard = std::make_shared<Guard>(std::move(completion_handler));

        detail::runAll<Traits>(guard, attr, io, std::index_sequence_for<Fns...>{},
                               std::forward<Fns>(functions)...);

        guard.reset();
    };

    return boost::asio::async_initiate<CompletionToken, CompletionSignature>(
            init, token, std::forward<Fns>(functions)...
    );
}


template<
        typename Result, typename Traits,
        typename CompletionSignature=typename Traits::template CompletionSignatureT<Result>,
        boost::asio::completion_token_for<CompletionSignature> CompletionToken,
        typename FuncsContainer, typename FuncT = std::decay_t<typename FuncsContainer::value_type>
>
inline auto waitAll(const boost::coroutines::attributes &attr, boost::asio::io_context &io,
                    CompletionToken &&token, FuncsContainer functions) {
    detail::check_constraints<Result, FuncT>();

    using Container = typename Traits::template DynamicContainer<typename detail::handler_info<FuncT>::return_type>;
    using Data = typename std::conditional_t<std::is_void_v<Result>, void, Container>;
    using Storage = detail::DynamicStorage<Result, typename Traits::ErrorCode, Data>;

    auto init = [&io, attr = attr](auto completion_handler, auto functions) {
        using CompletionHandler = std::decay_t<decltype(completion_handler)>;
        using Guard = detail::Guard<CompletionHandler, Storage>;

        auto guard = std::make_shared<Guard>(std::move(completion_handler), functions.size());

        detail::runAll<Traits>(guard, attr, io, std::move(functions));

        guard.reset();
    };

    return boost::asio::async_initiate<CompletionToken, CompletionSignature>(
            init, token, std::move(functions)
    );
}

}
