#include "image.h"
#include "programoptions.h"
#include "programstatus.h"
#include <vector>
#include <thread>
#include <atomic>

using std::vector;
using std::thread;
using std::atomic;

class ImageProcessor {
    unsigned int maxThreads;
    ProgramOptions& options;

public:
    ImageProcessor(unsigned int maxThreads, ProgramOptions& options) : maxThreads(maxThreads), options(options) {}

    void processAll(vector<Image>& imgs, ProcessingResult& res);
    void processImage(Image& img, UnitProcessingResult& ures);
    
    void cropSide(cv::Mat& mat); 
    
    void cropCenter(cv::Mat& mat);
    void expandImage(cv::Mat& mat);

};