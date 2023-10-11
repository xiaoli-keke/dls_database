#include <iostream>
#include "../src/database.h"

int main(int argc, char **argv) {
    dls::DataBase db("/home/kell/workspace/ddatabase/data");
    db.put("10001", "test 10001_1", 12345);
    db.put("10002", "test 10002_1", 12345);
    db.put("10001", "test 10001_1", 12345);
    db.put("10001", "test 10001_2", 12346);
    db.put("10002", "test 10002_1_dd", 12345);
    db.put("10002", "test 10002_2", 12346);

    auto results = db.get("10002", 12340, 12345);
    for(auto& r : results) {
        std::cout << r.format() << std::endl;
    }
    db.exit();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    db.get("10002", 12340, 12345);
    return 0;
}