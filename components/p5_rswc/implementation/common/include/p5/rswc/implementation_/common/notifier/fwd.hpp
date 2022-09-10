#pragma once

#include <functional>


namespace p5::rswc::implementation_::common {
namespace notifier {
namespace subscription {
    
    struct Class;

} // namespace subscription

    struct Class;

    using Handler = ::std::function<void(void)>;
    using Subscription = subscription::Class;

} // namespace notifier

    using Notifier = notifier::Class;

} // p5::rswc::implementation_::common
