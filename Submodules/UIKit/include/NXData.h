#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace NXKit {

class NXData {
public:
    std::size_t count() const;
    const std::byte* data() const;

    NXData(const std::byte* bytes, std::size_t count, bool freeSource = false);
    ~NXData();

   static std::shared_ptr<NXData> fromPath(const std::string& path);
private:
    std::vector<std::byte> _data;
};

}
