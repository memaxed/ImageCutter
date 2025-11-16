#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <optional>
#include <atomic>
#include <semaphore>
#include "headers/imageprocessor.h"
#include "headers/timer.h"

#define log(msg) std::cout << "[INF] " << msg << std::endl;
#define warn(msg) std::cout << "[WARN] " << msg << std::endl;
#define err(msg) std::cerr << "[ERR] " << msg << std::endl;

using std::atomic;
using std::counting_semaphore;
using std::string;
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
        return threadsSpecified() ? threads.value() : thread::hardware_concurrency();
    }
};

bool readOptions(int argc, char* argv[], ProgramOptions& options);
bool isValidOptions(const ProgramOptions& options);
vector<Image> collectImages(const ProgramOptions& options);
unsigned int loadImages(vector<Image>& images, unsigned int maxThreads);

//

int main(int argc, char* argv[]) {

    unsigned int maxThreads;
    ProgramOptions options;

    if(!readOptions(argc, argv, options)) {
        return 1;
    }

    if(!isValidOptions(options)) {
        return 1;
    }

    maxThreads = options.threadCount();
    ImageProcessor ip(maxThreads);

    log("Selected input folder: " + options.srcPath.string());
    log("Selected output folder: " + options.dstPath.string());
    log("Threads for working: " + std::to_string(maxThreads));
    log("Finding supported images...");

    Timer t;

    vector<Image> images = collectImages(options);
    if(images.size() == 0) {
        warn("There isn't any image in source folder");
        warn("Exiting program");
        return 0;
    }
    string count = std::to_string(images.size());
    
    log("Found " + count + " images in " + std::to_string(t.ms()) + " ms");
    log("Trying load " + count + " images...");
    t.reset();
    unsigned int loaded = loadImages(images, maxThreads);
    log("Successfully loaded " + std::to_string(loaded) + " images in " + std::to_string(t.ms()) + " ms");
    log("Starting process loaded images...");
    t.reset();
    unsigned int processed = ip.processAll(images);
    log("Successfully processed " + std::to_string(processed) + " images in " + std::to_string(t.ms()) + " ms");
    t.reset();

    return 0;
}

//

bool readOptions(int argc, char* argv[], ProgramOptions& options) {
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        if(arg == "--src" && (i+1) < argc) {
            options.srcPath = argv[++i];
        } else if(arg == "--dst" && (i+1) < argc) {
            options.dstPath = argv[++i];
        } else if(arg == "--threads" && (i+1) < argc) {
            int threads; 
            try {
                threads = std::stoi(argv[++i]);
            } catch(const std::invalid_argument& e) {
                err("Invalid --threads argument");
                return false;
            }
            if(threads <= 0) {
                err("Thread count must be > 0");
                return false;
            }
            options.threads = std::make_optional(threads);
        } else {
            err("Unknown options format");
            err("Usage: imgcut --src <src-folder> --dst <destination-folder>");
            return false;
        }
    }

    if (options.empty()) {
        err("Source and destination folders must be provided");
        err("Usage: imgcut --src <src-folder> --dst <destination-folder>");
        return false;
    }

    return true;
}

bool isValidOptions(const ProgramOptions& options) {
    const path& src = options.srcPath;
    const path& dst = options.dstPath;
    
    if(!exists(src)) {
        err("Source path " + src.string() + " does not exist");
        return false;
    }

    if(!exists(dst)) {
        err("Destination path " + dst.string() + " does not exist");
        return false;
    }

    if(!is_directory(src)) {
        err("Source path must be directory");
        return false;
    }

    if(!is_directory(dst)) {
        err("Destination path must be directory");
        return false;
    }

    return true;
}

vector<Image> collectImages(const ProgramOptions& options) {
    vector<Image> images;

    const vector<string> supported = {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp"};

    for(const auto& file : directory_iterator(options.srcPath)) {
        if(!file.is_regular_file()) continue;

        path filename = file.path().filename();
        path dst = options.dstPath / filename;

        string extension = file.path().extension().string();
        for (char& c : extension)
            c = std::tolower(static_cast<unsigned char>(c));

        // add to processing if file supported
        if(find(supported.begin(), supported.end(), extension) != supported.end()) {
            images.emplace_back(file.path(), dst);
        }
    }

    return images;
}

std::mutex mtx;
// multi-thread images load
unsigned int loadImages(vector<Image>& images, unsigned int maxThreads) {
    vector<thread> pool;
    counting_semaphore sem(maxThreads);
    atomic<unsigned int> loaded = 0;

    vector<Image*> failedImages;
    
    for(Image& img : images) {
        sem.acquire();

        pool.emplace_back(thread(
            [&img, &loaded, &sem, &failedImages](){
                if(!img.load()) {
                    std::lock_guard<std::mutex> guard = std::lock_guard(mtx);
                    warn("Failed to load image: " + img.source().string() + ". Is image corrupted?");
                    failedImages.push_back(&img);
                } else {
                    loaded.fetch_add(1, std::memory_order_relaxed);
                }
                sem.release();
            }
        ));
    }

    for(thread& t : pool) {
        if(t.joinable()) t.join();
    }

    for (Image* imgPtr : failedImages) {
        images.erase(std::remove_if(images.begin(), images.end(),
                                    [&](const Image& im){ return &im == imgPtr; }),
                     images.end());
    }

    return loaded.load(std::memory_order_relaxed);
}