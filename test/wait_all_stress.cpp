// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "common.h"

namespace {

using namespace testing;
using boost::asio::waitAll;
using boost::asio::yield_context;


template<size_t, class T>
using T_ = T;

template<size_t>
constexpr auto V_(auto V) {
    return V;
}

template<class T, size_t... Is>
auto gen(std::index_sequence<Is...>, T def = {}) { return std::tuple<T_<Is, T>...>{V_<Is>(def)...}; }

template<class T, size_t N>
auto genTuple(T def = {}) { return gen<T>(std::make_index_sequence<N>{}, def); }


MATCHER_P2(IsBetween, a, b,
           (std::string(negation ? "isn't" : "is") + " between " + PrintToString(a) + " and " + PrintToString(b))) {
    return a <= arg && arg <= b;
}

std::chrono::milliseconds getElapsed(const auto &start) {
    return (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start));
}

struct WaitAllTestStress : BaseTest {
    static constexpr size_t RUN_COUNT = 16;
    static constexpr size_t CALL_COUNT = 2;

    auto lightweight(auto arg, long timeout = 10) {
        return [=](boost::asio::yield_context yield) {
            boost::asio::deadline_timer t(coro::getIoContext(yield));
            t.expires_from_now(boost::posix_time::milliseconds(timeout));
            t.async_wait(yield);
            return arg;
        };
    }

    template<size_t RunCount = 1, size_t CallCount = 1, long TimeoutMs = 10, typename expected_t>
    void runTestOnYieldAndCallback(const auto &preparedWait, const expected_t &expected) {
        const auto timer = []() {
            return std::shared_ptr<std::nullptr_t>(nullptr, [start = std::chrono::high_resolution_clock::now()](auto) {
                EXPECT_THAT(getElapsed(start).count(), IsBetween(TimeoutMs, 2 * TimeoutMs));
            });
        };

        std::atomic_size_t callCount = 0;
        const auto token = [&]() {
            return [&, t = timer()](boost::system::error_code ec, expected_t actual) mutable {
                t.reset();
                ++callCount;
                EXPECT_EQ(expected, actual);
                EXPECT_FALSE(ec);
            };
        };

        for (size_t run = 0; run < RunCount; run++) {
            for (size_t call = 0; call < CallCount; call++) {
                spawn([&](yield_context yield) {
                    auto t = timer();
                    const auto actual = preparedWait(yield);
                    t.reset();
                    EXPECT_EQ(expected, actual);
                    ++callCount;
                });
                spawn([&](auto) { preparedWait(token()); });
            }
            runIO();
            EXPECT_EQ(2U * CallCount, callCount);
            callCount = 0;
        }

    }
};


TEST_P(WaitAllTestStress, variadicOfLightweight) {
    constexpr long timeout_ms = 100;
    constexpr size_t requests_count = 128;
    const std::tuple expected = genTuple<int, requests_count>(42);
    using expected_t = std::remove_cv_t<decltype(expected)>;

    const auto preparedWait = [&](auto &&ct) {
        return std::apply(
                [&](auto &&... args) {
                    return waitAll<expected_t>(
                            io, std::forward<decltype(ct)>(ct),
                            lightweight(args, timeout_ms)...
                    );
                },
                expected
        );
    };

    runTestOnYieldAndCallback<RUN_COUNT, CALL_COUNT, timeout_ms>(preparedWait, expected);
}


TEST_P(WaitAllTestStress, vectorOfLightweight) {
    constexpr long timeout_ms = 150;
    constexpr size_t requests_count = 128;
    const std::vector<int> expected{requests_count, 42};
    using expected_t = std::remove_cv_t<decltype(expected)>;

    std::vector<std::function<int(yield_context)>> functors;
    functors.reserve(expected.size());
    for (int e: expected) {
        functors.emplace_back(lightweight(e, timeout_ms));
    }

    const auto preparedWait = [&](auto &&ct) {
        return waitAll<expected_t>(
                io, std::forward<decltype(ct)>(ct),
                functors
        );
    };

    runTestOnYieldAndCallback<RUN_COUNT, CALL_COUNT, timeout_ms>(preparedWait, expected);
}


TEST_P(WaitAllTestStress, multipleWaitAllInsideWaitAll) {
    constexpr long timeout_ms = 90;
    constexpr size_t requests_count = 64;
    const std::tuple oneExpected = genTuple<std::string, requests_count>(std::string{42, 'y'});
    using one_expected_t = std::remove_cv_t<decltype(oneExpected)>;

    const auto oneWaitAll = [&](yield_context yield) {
        return std::apply(
                [&](auto &&... args) {
                    return waitAll<one_expected_t>(
                            io, yield,
                            lightweight(args, timeout_ms)...
                    );
                },
                oneExpected
        );
    };

    const std::tuple expected = genTuple<one_expected_t, 5>(oneExpected);
    using expected_t = std::remove_cv_t<decltype(expected)>;

    const auto preparedWait = [&](auto &&ct) {
        return waitAll<expected_t>(
                io, std::forward<decltype(ct)>(ct),
                oneWaitAll,
                oneWaitAll,
                oneWaitAll,
                oneWaitAll,
                oneWaitAll
        );
    };

    runTestOnYieldAndCallback<RUN_COUNT, CALL_COUNT, timeout_ms>(preparedWait, expected);
}


INSTANTIATE_TEST_SUITE_P(
        WaitAllTestsStress,
        WaitAllTestStress,
        ::testing::Values(
                1, 2, 3, 4
        ));

}
