#include "parser.h"
#include "logger.h"



int main(int argc, char** argv) {

    logger::Logger log ("main", logger::LVL_DEBUG);
    logger::Logger l2  ("other", logger::LVL_DEBUG);

    log.info() << "starting";
    l2.info() << "start2";
    log.debug("info!");
    l2.debug("info2!");
    log.debug() << "bug?";
    l2.debug() << __PRETTY_FUNCTION__;


    PcapLoader foo;
    try {
        foo.parse(argv[1]);

    } catch (const std::exception& e) {
        log.error() << e.what();
    }

    return 0;
}
