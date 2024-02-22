#include "time_stamp.h"
#include <iostream>

int main() {
    speed::mq::time_stamp time_stamp;
    std::cout << time_stamp.fotmat_string(3) << std::endl;
}