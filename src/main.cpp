#include <exception>
#include <iostream>

#include "cli/cli.hpp"

int main(int argc, char* argv[]) {
    try {
        run_cli(argc, argv);
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}