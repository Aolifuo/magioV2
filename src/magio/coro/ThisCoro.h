#pragma once

#include "magio/coro/Config.h"
#include "magio/execution/Execution.h"

namespace magio {

namespace this_coro {

namespace detail {

}

struct GetExecutor {
    bool await_ready() { return false; }

    template<typename PT>
    auto await_suspend(coroutine_handle<PT> prev_h) {
        executor = prev_h.promise().executor;
        prev_h.resume();
    }

    auto await_resume() {
        return executor;
    }

    AnyExecutor executor;
};



inline GetExecutor executor;

}

}