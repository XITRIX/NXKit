#pragma once
#include <UIApplication.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <cmath>
#include <fstream>
#include <optional>
#include <string>

// Persistence belongs to the application. UIKitCore only resolves the override.
namespace DemoGlassSettings {
inline std::string path() {
    auto directory=SDL_GetPrefPath("XITRIX", "NXKitDemo");
    if(!directory) return {};
    const auto result=std::string(directory)+"glass-appearance.txt";
    SDL_free(directory);
    return result;
}
inline void restore() {
    const auto file=path();
    if(file.empty()) return;
    std::ifstream input(file);
    float amount;
    if(input>>amount && (input>>std::ws).eof() && std::isfinite(amount) && amount>=0 && amount<=1)
        NXKit::UIApplication::shared->setOverrideGlassTintAmount(amount);
}
inline void set(std::optional<NXKit::NXFloat> amount) {
    NXKit::UIApplication::shared->setOverrideGlassTintAmount(amount);
    const auto file=path();
    if(file.empty()) return;
    std::ofstream output(file,std::ios::trunc);
    if(amount) output<<*amount<<'\n';
    else output<<"system\n";
}
}
