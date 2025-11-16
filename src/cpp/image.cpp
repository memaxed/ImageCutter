#include "../headers/image.h"

using std::cerr;
using std::endl;

bool Image::load() {
    // read image (color, 3 channels)
    data = cv::imread(sourcePath.string(), cv::IMREAD_COLOR);

    if (data.empty()) {
        loaded = false;
    } else loaded = true;

    return loaded;
}

bool Image::save() const {
    if (!loaded) {
        return false;
    }

    // trying save file
    if (!cv::imwrite(outputPath.string(), data)) {
        return false;
    }

    return true;
}