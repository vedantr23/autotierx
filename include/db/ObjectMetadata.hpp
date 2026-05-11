#ifndef OBJECT_METADATA_HPP
#define OBJECT_METADATA_HPP

#include <string>

namespace autotierx {

class ObjectMetadata {
private:
    std::string objectId;
    std::string filename;
    std::string path;
    std::string tier;
    long sizeBytes;
    int accessCount;
    std::string createdAt;
    std::string lastAccessed;
    std::string checksum;

public:
    ObjectMetadata(
        const std::string& objectId,
        const std::string& filename,
        const std::string& path,
        const std::string& tier,
        long sizeBytes,
        int accessCount,
        const std::string& createdAt,
        const std::string& lastAccessed,
        const std::string& checksum
    );

    void printMetadata() const;
};

}

#endif