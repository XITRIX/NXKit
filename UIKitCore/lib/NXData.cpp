#include <NXData.h>
#include <tools/Tools.hpp>
#include <SDL3/SDL.h>

#include <limits>

//#ifdef USE_LIBROMFS
#include <romfs/romfs.hpp>
//#endif

namespace NXKit {

std::function<std::shared_ptr<NXData>(std::string)> NXData::resourceProvider;

NXData::NXData(const std::byte bytes[], std::size_t count, bool freeSource) {
    for (int i = 0; i < count; i++)
        _data.push_back(bytes[i]);

    if (freeSource)
        delete[] bytes;
}

NXData::~NXData() {
//    _data.clear();
}

std::size_t NXData::count() const {
    return _data.size();
}

const std::byte* NXData::data() const {
    return _data.data();
}

std::shared_ptr<NXData> NXData::fromPath(const std::string& path) {
//#ifdef USE_LIBROMFS
//    auto file = romfs::get(path);
//    auto fileReader = SDL_IOFromConstMem(file.data(), file.size());
//#else
    auto fileReader = SDL_IOFromFile(path.c_str(), "rb");
//#endif
    if (!fileReader) {
        return nullptr;
    }

    const Sint64 fileSize = SDL_GetIOSize(fileReader);
    if (fileSize < 0
        || static_cast<Uint64>(fileSize) > std::numeric_limits<std::size_t>::max()) {
        SDL_CloseIO(fileReader);
        return nullptr;
    }

    const auto byteCount = static_cast<std::size_t>(fileSize);
    auto buffer = new std::byte[byteCount];
    const auto bytesRead = SDL_ReadIO(fileReader, buffer, byteCount);
    SDL_CloseIO(fileReader);

    if (bytesRead == byteCount) {
        return new_shared<NXData>(buffer, byteCount, true);
    }

    delete[] buffer;
    return nullptr;
}

std::shared_ptr<NXData> NXData::fromRes(const std::string& path) {
    return resourceProvider(path);
}

}
