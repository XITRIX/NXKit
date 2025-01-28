#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace NXKit {

#define DEFAULT_ROMFS_REGISTRATION                          \
NXData::resourceProvider = [](const std::string& path) {    \
auto res = romfs::get(path);                                \
return new_shared<NXData>(res.data(), res.size());          \
};

class NXData {
public:
    [[nodiscard]] std::size_t count() const;
    [[nodiscard]] const std::byte* data() const;

    NXData(const std::byte* bytes, std::size_t count, bool freeSource = false);
    ~NXData();

    static std::shared_ptr<NXData> fromPath(const std::string& path);
    static std::shared_ptr<NXData> fromRes(const std::string& path);
    static std::function<std::shared_ptr<NXData>(std::string)> resourceProvider;
private:
    std::vector<std::byte> _data;
};

}
