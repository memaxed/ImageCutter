#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>

using namespace std::filesystem;
using std::string;

class Image {
    path sourcePath;
    path outputPath;

    cv::Mat data;
    bool loaded = false;
public:
    Image(const path& source, const path& output) : sourcePath(source), outputPath(output) {};

    bool load();
    bool save() const;

    path source() const {return sourcePath;}
    path destination() const {return outputPath;}

    cv::Mat& mat() { return data; }
    const cv::Mat& mat() const { return data; }

    bool isLoaded() const { return loaded; }
};