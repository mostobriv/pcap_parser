#include "parser.h"

#include <iostream>



int main(int argc, char** argv) {

    PcapLoader foo;
    try {
        foo.parse(argv[1]);

    } catch (const std::exception& e) {
        const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
        if (st) {
            std::cerr << *st << '\n';
        }
    }

    return 0;
}