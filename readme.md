# wait_all

This is C++20 library for `boost::asio` which waits for all `coroutines/awaitables/handlers` to complete.


### Motivating Example #0
```cpp
#include "wait_all/wait_all.hpp"

Response handleRequest(const Context& ctx, boost::asio::yield_context yield) {

    struct Responses {
        ServiceResponse1 s1;
        ServiceResponse2 s2;
        // ..
        ServiceResponseN sN;
    };

    const auto responses = wait_all::waitAll<Responses>(
        ctx.io, yield,
        makreRequestService1,
        makreRequestService2,
        // ...
        makreRequestServiceN,
    );

    // compose and return Response based on responses from N services
}
```