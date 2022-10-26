// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace wait_all::detail {

template<typename CompletionHandler, typename Storage>
struct Guard final : public Storage {

    template<class... Args>
    explicit
    Guard(CompletionHandler h, Args &&... args) : Storage{std::forward<Args>(args)...}, handler{std::move(h)} {}

    ~Guard() {
        complete();
    }

    Guard(const Guard &) = delete;

    Guard(Guard &&) = delete;

    Guard &operator=(const Guard &other) = delete;

    Guard &operator=(Guard &&other) = delete;

private:
    CompletionHandler handler;

    void complete() {
        using ErrorCode = std::decay_t<decltype(Storage::getError())>;
        using Result = std::decay_t<decltype(Storage::getResult())>;

        if constexpr (not std::is_same_v<Result, Void> and std::is_invocable_v<CompletionHandler, ErrorCode, Result>) {
            handler(Storage::getError(), Storage::getResult());
        } else {
            handler(Storage::getError());
        }
    }
};

}
