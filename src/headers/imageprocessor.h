#include "image.h"
#include <vector>
#include <thread>
#include <atomic>

using std::vector;
using std::thread;
using std::atomic;

class ImageProcessor {
    unsigned int maxThreads;

public:
    ImageProcessor(unsigned int maxThreads) : maxThreads(maxThreads) {}

    unsigned int processAll(vector<Image>& imgs);
    void processImage(Image& img);
    
    void cropSide(Image& img); 
    
    void cropCenter(Image& img);
    void expandImage(Image& img);


};