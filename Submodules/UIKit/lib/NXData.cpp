#include <NXData.h>
#include <tools/Tools.hpp>
#include <SDL2/SDL.h>

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
//    auto fileReader = SDL_RWFromConstMem(file.data(), (int) file.size());
//#else
   auto fileReader = SDL_RWFromFile(path.c_str(), "r");
//#endif

   auto fileSize = int(fileReader->size(fileReader));

   auto buffer = new std::byte[fileSize];

   auto bytesRead = int(fileReader->read(fileReader, buffer, 1, fileSize));

   fileReader->close(fileReader);

   if (bytesRead == fileSize) {
       return new_shared<NXData>(buffer, fileSize, true);
   } else {
       delete[] buffer;
       return nullptr;
   }
}

std::shared_ptr<NXData> NXData::fromRes(const std::string& path) {
    return resourceProvider(path);
}

}
