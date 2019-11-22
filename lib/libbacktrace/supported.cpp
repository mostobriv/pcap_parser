#include <iostream>
#include "libbacktrace/backtrace-supported.h"


int main()
{
    if (BACKTRACE_SUPPORTED)
    {
        std::cout << "backtrace supported\n";
    }
    else
    {
        std::cout << "backtrace not supported\n";
        return 1;
    }

    if (BACKTRACE_USES_MALLOC)
    {
        std::cout << "uses malloc\n";
    }

    if (BACKTRACE_SUPPORTS_THREADS)
    {
        std::cout << "supports threads\n";
    }
}
