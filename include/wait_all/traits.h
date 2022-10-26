// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <wait_all/error_code.h>
#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>


namespace wait_all::traits {

struct Boost {
    template<typename... Args>
    using StaticContainer = std::tuple<Args...>;

    template<typename... Args>
    using DynamicContainer = std::vector<Args...>;

    using ErrorCode = ::boost::system::error_code;
    using SystemError = ::boost::system::system_error;

    static ErrorCode getUnexpectedErrorCode(const std::string_view = {}) {
        // TODO: you can shadow this method and add extra logging
        return {static_cast<int>(::wait_all::error_code::exception), ::wait_all::wait_all_error_category()};
    }

    template<typename Result>
    struct CompletionSignature {
        using type = void(ErrorCode, Result);
    };

    template<>
    struct CompletionSignature<void> {
        using type = void(ErrorCode);
    };

    template<typename Result>
    using CompletionSignatureT = typename CompletionSignature<Result>::type;
};

}
