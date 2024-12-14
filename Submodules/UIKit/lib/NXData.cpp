#include <NXData.h>
//#include <Tools/Tools.hpp>
#include <SDL2/SDL.h>

//#ifdef USE_LIBROMFS
#include <romfs/romfs.hpp>
//#endif

namespace NXKit {

NXData::NXData(uint8_t bytes[], int count, bool freeSource) {
    for (int i = 0; i < count; i++)
        _data.push_back(bytes[i]);

    if (freeSource)
        delete[] bytes;
}

NXData::~NXData() = default;

int NXData::count() const {
    return (int) _data.size();
}

uint8_t* NXData::data() const {
    return (uint8_t*) _data.data();
}

std::optional<NXData> NXData::fromPath(const std::string& path) {
//#ifdef USE_LIBROMFS
//    auto file = romfs::get(path);
//    auto fileReader = SDL_RWFromConstMem(file.data(), (int) file.size());
//#else
   auto fileReader = SDL_RWFromFile(path.c_str(), "r");
//#endif

   auto fileSize = int(fileReader->size(fileReader));

   auto buffer = new uint8_t[fileSize];

   auto bytesRead = int(fileReader->read(fileReader, buffer, 1, fileSize));

   fileReader->close(fileReader);

   if (bytesRead == fileSize) {
       return NXData(buffer, fileSize, true);
   } else {
       delete[] buffer;
       return std::nullopt;
   }
}

}
