#include <filesystem>

class Image {
    std::filesystem::path sourcePath;
public:
    Image(const std::filesystem::path& source) : sourcePath(source) {};

    std::filesystem::path source() const {return sourcePath;}

};