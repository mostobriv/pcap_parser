#include "PcapLoader.h"
#include "logger.h"
#include <pqxx/pqxx>


int main(int argc, char** argv)
{
    try {
        pqxx::connection c ("host=localhost port=5432 user=pcap password=pcap_312b4a6b229587d831dd4a05fc83d4f7");

        // Start a transaction.  In libpqxx, you always work in one.
        pqxx::work w(c);

        // We'll just ask the database to return the number 1 to us.
        pqxx::result res = w.exec("SELECT 1");

        w.commit();

        logger::logger.info() << res[0][0].as<int>();
        return 0;

    } catch (const std::exception &e) {
        logger::logger.error() << e.what();
    }

    if (argc < 2) {
        logger::logger.error() << "Usage:" << argv[0] << "[*].pcap";
        return 1;
    }

    PcapLoader foo;
    try {
        foo.parse(argv[1]);

    } catch (const std::exception& e) {
        logger::logger.error() << e.what();
    }

    return 0;
}
