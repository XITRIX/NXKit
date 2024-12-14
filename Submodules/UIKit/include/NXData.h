#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace NXKit {

class NXData {
public:
    int count() const;
    uint8_t* data() const;

    NXData(uint8_t* bytes, int count, bool freeSource = false);
    ~NXData();

   static std::optional<NXData> fromPath(const std::string& path);
private:
    std::vector<uint8_t> _data;
};

}
