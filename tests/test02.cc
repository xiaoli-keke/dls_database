#include <iostream>
#include "../src/logger.h"
#include "../src/database.h"
// 20K
// 2023-10-07 09:43:16: save begin
// 2023-10-07 09:49:50: save end
// 2023-10-07 09:50:00: search begin
// 2023-10-07 09:50:00: search end

void print_time(const char* fmt) {
    time_t t = ::time(nullptr);
    tm *curTime = localtime(&t);  // NOLINT
    char time_str[32];            // FIXME
    ::strftime(time_str, 32, "%Y-%m-%d %H:%M:%S", curTime);
    std::cout << time_str << ": " << fmt << std::endl;
}

int main(int argc, char **argv) {
    dls::DataBase db("/home/kell/workspace/ddatabase/data");
    int start_key = 1001;
    print_time("save begin");
    for(int i = 1; i <= 1000000; i++) {
        // if (i % 1000000 == 0) {
        //     std::this_thread::sleep_for(std::chrono::seconds(5));
        // }
        for(int j = 0 ; j < 100; j++) {
             db.put(std::to_string(start_key + j),
                std::to_string(start_key + j) + "_" + std::to_string(i), 
                i);
        }
    }
    db.exit();
    print_time("save end");
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "aaaaaaaaaaaaaaaaaaaaaa" << std::endl;
    print_time("search begin");
    auto results = db.get("1008", 100, 1000);
    print_time("search end");
    // for(auto& r : results) {
    //     std::cout << r.format() << std::endl;
    // }
    db.exit();
    return 0;
}