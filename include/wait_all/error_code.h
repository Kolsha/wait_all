// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include <boost/system/error_code.hpp>

namespace wait_all {

enum class error_code {
    success = 0,

    exception,
};

class wait_all_err_cat : public boost::system::error_category {
public:

    const char *name() const noexcept override {
        return "wait_all";
    }

    std::string message(int ev) const override {
        if (ev) {
            return "unexpected exception";
        }
        return {};
    }
};

inline boost::system::error_category const &wait_all_error_category() {
    static const wait_all_err_cat instance;
    return instance;
}
}

namespace boost::system {

template<>
struct is_error_code_enum<::wait_all::error_code> : std::true_type {
};

}
