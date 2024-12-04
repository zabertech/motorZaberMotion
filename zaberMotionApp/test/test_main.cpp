#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main (int argc, char* argv[]) {
    try {
        Catch::Session session;

        auto exitCode = session.applyCommandLine(argc, argv);
        if (exitCode != 0) {
            return exitCode;
        }

        return session.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
