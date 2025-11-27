#include <optional>
#include <filesystem>
#include <thread>

using std::optional;
using namespace std::filesystem;

struct ProgramOptions {
    path srcPath;
    path dstPath;
    optional<unsigned int> threads;

    bool empty() {
        return srcPath.empty() || dstPath.empty();
    }

    bool threadsSpecified() {
        return threads.has_value();
    }

    unsigned int threadCount() {
        return threadsSpecified() ? threads.value() : std::thread::hardware_concurrency();
    }
};