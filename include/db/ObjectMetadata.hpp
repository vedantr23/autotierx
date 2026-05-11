#pragma once

#include <string>
#include <iostream>

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

    /*
    =====================================
    PRINT
    =====================================
    */

    void printMetadata() const;

    /*
    =====================================
    GETTERS
    =====================================
    */

    std::string getObjectId() const;

    std::string getFilename() const;

    std::string getPath() const;

    std::string getTier() const;

    long getSizeBytes() const;

    int getAccessCount() const;

    std::string getCreatedAt() const;

    std::string getLastAccessed() const;

    std::string getChecksum() const;

    /*
    =====================================
    SETTERS
    =====================================
    */

    void setTier(const std::string& newTier);

    void incrementAccessCount();
};

}