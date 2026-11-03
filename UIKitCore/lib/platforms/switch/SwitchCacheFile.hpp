#pragma once

#include <string>

namespace NXKit::Internal {

// Ops returns booleans and treats a missing remove target as success. Keep the
// replacement transaction independently testable against fsdev rename semantics.
template<class Ops>
bool replaceSwitchCacheFile(Ops& ops, const std::string& temporary, const std::string& path) {
    const auto previous = path + ".previous";
    if (ops.rename(temporary, path)) {
        ops.remove(previous);
        return true;
    }
    if (!ops.exists(path) || !ops.remove(previous) || !ops.rename(path, previous)) return false;
    if (!ops.rename(temporary, path)) {
        // If rollback also fails, readFile can recover the preserved .previous.
        ops.rename(previous, path);
        return false;
    }
    ops.remove(previous);
    return true;
}

} // namespace NXKit::Internal
