#include <iostream>
#include "Tsp.hpp"
using namespace std;

int main(int argc, char* argv[]) {

    Tsp tsp;
    tsp.read(argc, argv);
    if (tsp.hasError()) {
        return 1;
    }

    tsp.openTime(false);
    tsp.findSolution();
    // tsp.outputResult();
    // tsp.printData();

    return 0;
};