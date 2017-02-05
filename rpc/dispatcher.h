// Copyright 2017 <codefever@github.com>
#pragma once

#include <functional>
#include <memory>

#include "rpc/pack.h"

typedef std::function<void(
    std::shared_ptr<RawMessage> request,
    std::shared_ptr<Respondor> respondor)> DispatcherCall;
