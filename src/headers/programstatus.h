#include <atomic>
#include <vector>
#include <string>
#include <mutex>
#include <optional>
#include <memory>

using std::optional;
using std::atomic;
using std::vector;
using std::string;
using std::shared_ptr;

constexpr auto mrel = std::memory_order_relaxed;

enum class ProcessingStatus {
    Ok,
    SaveErr,
    LoadErr,
    ProcessErr
};

// result for one image
struct UnitProcessingResult {
    shared_ptr<string> name{};
    optional<string> errtext{};
    ProcessingStatus status{};

    void fill(const string& n, ProcessingStatus st) {
        name = std::make_shared<string>(n);
        status = st;
        errtext.reset();
    }

    void fillError(const string& n, ProcessingStatus st, const string& err) {
        name = std::make_shared<string>(n);
        status = st;
        errtext = err;
    }
};

// result for whole program
struct ProcessingResult {

    atomic<unsigned int> ok{0};
    atomic<unsigned int> saveErr{0};
    atomic<unsigned int> loadErr{0};
    atomic<unsigned int> procErr{0};

    std::mutex mtx;

    vector<UnitProcessingResult> result;

    void add(const UnitProcessingResult& res) {
        switch (res.status) {
            case ProcessingStatus::Ok: ok.fetch_add(1, mrel); break;
            case ProcessingStatus::LoadErr: loadErr.fetch_add(1, mrel); break;
            case ProcessingStatus::SaveErr: saveErr.fetch_add(1, mrel); break;
            case ProcessingStatus::ProcessErr: procErr.fetch_add(1, mrel); break;
        }

        std::lock_guard lock(mtx);
        result.emplace_back(res);
    }

    unsigned int oks() const {
        return ok.load(mrel);
    }

    unsigned int saveErrs() const {
        return saveErr.load(mrel);
    }

    unsigned int loadErrs() const {
        return loadErr.load(mrel);
    }
    
    unsigned int procErrs() const {
        return procErr.load(mrel);
    }

};