#if defined(SDKCONFIG_TEST_STATE) && (true == SDKCONFIG_TEST_STATE)
#include <p5/rswc/implementation.hpp>
#include <p5_rswc.h>
extern "C" void p5_rswc_main(void) { ::p5::rswc::implementation(); }
#else
#error "unsupported project configuration (sdkconfig)"
#endif
