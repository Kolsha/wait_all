// Copyright (c) 2022 Nikolai Ovchinnikov (kolsha.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>

#include <wait_all/detail/void.h>


namespace wait_all::detail {


template<typename OwnerPtr, typename ResultFn, typename ErrorFn>
struct StorageAccess final {
    StorageAccess(OwnerPtr ownPtr, ResultFn resFn, ErrorFn errFn)
            : ownerPtr{std::move(ownPtr)}, resultFn{std::move(resFn)}, errorFn{std::move(errFn)} {}

    void setResult(auto res) {
        resultFn(std::move(res));
    }

    void setError(auto err) {
        errorFn(std::move(err));
    }

private:
    OwnerPtr ownerPtr;
    ResultFn resultFn;
    ErrorFn errorFn;
};


template<typename ErrorCode>
struct BaseStorage : public std::enable_shared_from_this<BaseStorage<ErrorCode> > {
    BaseStorage() = default;

protected:
    ErrorCode getError() {
        errorSet = false;
        return std::move(errorCode);
    }

    auto getBaseAccess(auto resultHandler) {
        return StorageAccess{this->shared_from_this(), std::move(resultHandler), this->errorHandler()};
    }

private:
    void setErrorCode(ErrorCode ec) {
        if (errorSet.exchange(true)) {
            return;
        }
        errorCode = std::move(ec);
    }

    auto errorHandler() {
        return [this](ErrorCode v) mutable {
            this->setErrorCode(std::move(v));
        };
    }

    ErrorCode errorCode;
    std::atomic_bool errorSet{false};
};


template<typename Result, typename ErrorCode, typename Data>
struct StaticStorage : public BaseStorage<ErrorCode> {
    StaticStorage() = default;

    template<size_t Idx>
    auto getAccess() {
        return this->getBaseAccess(this->resultHandler<Idx>());
    }

protected:
    Result getResult() {
        if constexpr (std::is_convertible_v<Data, Result>) {
            return std::move(data);
        }

        return std::apply(
                [](auto &&... args) {
                    static_assert(requires{ Result{std::move(args)...}; },
                            "couldn't construct Result from return types of functions");
                    return Result{std::move(args)...};
                },
                std::move(data)
        );
    }

private:
    Data data;

    template<size_t Idx>
    auto resultHandler() {
        return [this](auto v) mutable {
            std::get<Idx>(this->data) = std::move(v);
        };
    }
};


template<typename Result, typename ErrorCode, typename Data>
struct DynamicStorage : public BaseStorage<ErrorCode> {
    using Base = BaseStorage<ErrorCode>;

    explicit DynamicStorage(size_t size) : Base() {
        data.resize(size);
    }

    auto getAccess(size_t i) {
        return this->getBaseAccess(this->resultHandler(i));
    }

protected:
    Result getResult() {
        if constexpr (std::is_convertible_v<Data, Result>) {
            return std::move(data);
        }

        Result res;
        if constexpr (requires{ res.reserve(data.size()); }) {
            res.reserve(data.size());
        }
        for (auto &&s: data) {
            res.emplace_back(std::move(s));
        }
        return res;
    }

private:
    Data data;

    auto resultHandler(size_t i) {
        return [i, this](auto v) mutable {
            auto it = this->data.begin();
            std::advance(it, i);
            *it = std::move(v);
        };
    }
};


template<typename ErrorCode>
struct VoidStorage : public BaseStorage<ErrorCode> {
    VoidStorage() = default;

    template<size_t= 0>
    auto getAccess(size_t= 0) {
        return this->getBaseAccess([](auto) {});
    }

protected:
    Void getResult() {
        return {};
    }
};


template<typename ErrorCode, typename Data>
struct StaticStorage<void, ErrorCode, Data> : public VoidStorage<ErrorCode> {
};


template<typename ErrorCode, typename Data>
struct DynamicStorage<void, ErrorCode, Data> : public VoidStorage<ErrorCode> {
    using Base = VoidStorage<ErrorCode>;

    explicit DynamicStorage(size_t) : Base() {}
};

}
