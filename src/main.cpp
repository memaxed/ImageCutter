#include <opencv2/opencv.hpp>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <semaphore>
#include "headers/imageprocessor.h"
#include "headers/timer.h"
#include "headers/loggers.h"

using std::atomic;
using std::counting_semaphore;
using std::string;
using namespace std::filesystem;

bool readOptions(int argc, char* argv[], ProgramOptions& options);
bool isValidOptions(const ProgramOptions& options);
void collectImages(vector<Image>& images, const ProgramOptions& options);
void printUsage();
void report(const ProcessingResult& res);

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
    ImageProcessor ip(maxThreads, options);

    log("Selected input folder: " + options.srcPath.string());
    log("Selected output folder: " + options.dstPath.string());
    log("Threads for working: " + std::to_string(maxThreads));
    log("Finding supported images...");

    Timer t;

    vector<Image> images;
    collectImages(images, options);
    if(images.size() == 0) {
        warn("There isn't any image in source folder");
        warn("Exiting program");
        return 0;
    }
    string count = std::to_string(images.size());
    
    log("Found " + count + " images in " + std::to_string(t.ms()) + " ms");
    log("Starting process loaded images...");
    t.reset();
    ProcessingResult res;
    ip.processAll(images, res);
    log("Successfully processed " + std::to_string(res.oks()) + " images in " + std::to_string(t.ms()) + " ms");
    t.reset();

    report(res);

    return 0;
}

//

bool readOptions(int argc, char* argv[], ProgramOptions& options) {
    if (argc == 1) {
        printUsage();
        return false;
    }

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
                printUsage();
                return false;
            }
            if(threads <= 0) {
                err("Thread count must be > 0");
                printUsage();
                return false;
            }
            options.threads = std::make_optional(threads);
        } else {
            err("Unknown options format");
            printUsage();
            return false;
        }
    }

    if (options.empty()) {
        err("Source and destination folders must be provided");
        printUsage();
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

void collectImages(vector<Image>& images, const ProgramOptions& options) {

    const vector<string> supported = {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif", ".webp"};

    for(const auto& file : directory_iterator(options.srcPath)) {
        if(!file.is_regular_file()) continue;

        string extension = file.path().extension().string();
        for (char& c : extension)
            c = std::tolower(static_cast<unsigned char>(c));

        // add to processing if file supported
        if(find(supported.begin(), supported.end(), extension) != supported.end()) {
            images.emplace_back(file.path());
        }
    }
}

void printUsage() {
    println("Usage:");
    println("  imgcut --src <src-folder> --dst <destination-folder> [--threads N]");
    println("");
    println("Options:");
    println("  --src       Source folder with input images");
    println("  --dst       Destination folder where processed images are saved");
    println("  --threads   Number of threads (optional, default: all CPU cores)");
}

void report(const ProcessingResult& res) {
    log("-------------------------------------------------------");
    log("Processing summary: ");
    log("\tLoad errors: " + std::to_string(res.loadErrs()));
    log("\tSave errors: " + std::to_string(res.saveErrs()));
    log("\tProcessing errors: " + std::to_string(res.procErrs()));

    if(!res.result.empty()) {
        log("Detailed errors:");
        for (const auto& ures : res.result) {
            if (ures.status != ProcessingStatus::Ok) {
                logg("\tFile: " + *ures.name + " - ");
                switch (ures.status) {
                    case ProcessingStatus::LoadErr: print("Load error. Maybe file corrupted."); break;
                    case ProcessingStatus::SaveErr: print("Save error. Maybe you don't have permissions to save files to destination folder."); break;
                    case ProcessingStatus::ProcessErr: print("Processing error:"); break;
                    default: break;
                }
                if (ures.errtext.has_value()) {
                    print(" (" + *ures.errtext + ")");
                }
                println("");
            }
        }
    }
}
