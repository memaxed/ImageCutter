#include <opencv2/opencv.hpp>
#include <semaphore>
#include "../headers/imageprocessor.h"
#include "../headers/loggers.h"

using std::counting_semaphore;

void ImageProcessor::cropSide(cv::Mat& mat) {
    const int targetSize = 512;

    int width = mat.cols;
    int height = mat.rows;

    if(width > targetSize || height > targetSize) {

        int cropW = std::min(width, targetSize);
        int cropH = std::min(height, targetSize);

        int x = (width - cropW) / 2;
        int y = (height - cropH) / 2;

        cv::Rect roi(x, y, cropW, cropH);
        mat = mat(roi).clone();

    }
}

void ImageProcessor::cropCenter(cv::Mat& mat) {
    const int targetSize = 512;
    int width = mat.cols;
    int height = mat.rows;

    // calculate rectangle's start coordinates
    int x = (width - targetSize) / 2;
    int y = (height - targetSize) / 2;

    // construct rectangle, select Region Of Interest
    // and mutate matrix in source image (then clone it)
    cv::Rect roi(x, y, targetSize, targetSize);
    mat = mat(roi).clone(); 
}

void ImageProcessor::expandImage(cv::Mat& mat) {
    int targetSize = 512;

    // additions (in pixels) to target size
    int top = 0, bottom = 0, left = 0, right = 0;

    // if first side > 512 and second side < 512, cropping biggest side to 512
    cropSide(mat);

    int width = mat.cols;
    int height = mat.rows;

    // targetSize = height + top + bottom
    // targetSize = width + left + right

    if (height < targetSize) {
        top = (targetSize - height) / 2;
        bottom = targetSize - height - top;
    }

    if (width < targetSize) {
        left = (targetSize - width) / 2;
        right = targetSize - width - left;
    }

    // expanding image using copyMakeBorder
    if (top > 0 || bottom > 0 || left > 0 || right > 0) {
        cv::copyMakeBorder(mat, mat, top, bottom, left, right,
                           cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255)); 
    }
}

void ImageProcessor::processImage(Image& img, UnitProcessingResult& ures) {

    string filename = img.source().filename().string();

    // loading
    cv::Mat mat = cv::imread(img.source().string(), cv::IMREAD_COLOR);
    if(mat.empty()) {
        ures.fill(filename, ProcessingStatus::LoadErr);
        return;
    }

    // processing
    try {
        int targetSize = 512;

        int width = mat.cols;
        int height = mat.rows;

        bool biggerThanTarget = width >= targetSize && height >= targetSize;
    
        if(biggerThanTarget)
            cropCenter(mat);
        else expandImage(mat);
    } catch(std::exception& ex) {
        ures.fillError(filename, ProcessingStatus::ProcessErr, ex.what());
        return;
    }
     
    // saving
    path dst = options.dstPath / filename;
    bool status = cv::imwrite(dst.string(), mat);
    if(!status) {
        ures.fill(filename, ProcessingStatus::SaveErr);
        return;
    }

    ures.fill(filename, ProcessingStatus::Ok);
}

void ImageProcessor::processAll(vector<Image>& imgs, ProcessingResult& res) {
    vector<thread> pool;
    counting_semaphore sem(maxThreads);

    // for all images do multi-thread logic (using maximum threads)
    for(Image& img : imgs) {
        sem.acquire();

        pool.emplace_back([&img, &sem, &res, this]() mutable {
            UnitProcessingResult ures;
            processImage(img, ures);
            res.add(ures);
            sem.release();
        });
    }

    for(thread& t : pool)
        if(t.joinable()) t.join();

}