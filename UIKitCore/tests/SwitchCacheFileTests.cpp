#include "platforms/switch/SwitchCacheFile.hpp"
#include <cstdlib>
#include <iostream>
#include <map>

namespace {
struct Files {
    std::map<std::string, std::string> data{{"cache", "old"}, {"cache.tmp", "new"}};
    bool replace = false, installFails = false, rollbackFails = false, removeFails = false;
    bool exists(const std::string& name) { return data.contains(name); }
    bool remove(const std::string& name) {
        if (removeFails) return false;
        data.erase(name);
        return true;
    }
    bool rename(const std::string& from, const std::string& to) {
        if (!exists(from) || (!replace && exists(to)) ||
            (installFails && from == "cache.tmp") ||
            (rollbackFails && from == "cache.previous")) return false;
        data[to] = data.at(from);
        data.erase(from);
        return true;
    }
};
void expect(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
bool install(Files& files) {
    return NXKit::Internal::replaceSwitchCacheFile(files, "cache.tmp", "cache");
}
}

int main() {
    Files sd;
    expect(install(sd) && sd.data.at("cache") == "new" && sd.data.size() == 1,
           "replace existing file when rename cannot overwrite");
    Files posix;
    posix.replace = true;
    expect(install(posix) && posix.data.at("cache") == "new", "POSIX replacement");
    Files failed;
    failed.installFails = true;
    expect(!install(failed) && failed.data.at("cache") == "old", "failed install rolls back");
    Files interrupted;
    interrupted.installFails = interrupted.rollbackFails = true;
    expect(!install(interrupted) && interrupted.data.at("cache.previous") == "old",
           "failed rollback preserves recovery file");
    expect(!install(interrupted) && interrupted.data.at("cache.previous") == "old",
           "failed retry without destination preserves recovery file");
    Files removeFailure;
    removeFailure.removeFails = true;
    expect(!install(removeFailure) && removeFailure.data.at("cache") == "old",
           "failed backup cleanup preserves current file");
    Files staleBackup;
    staleBackup.data["cache.previous"] = "older";
    expect(install(staleBackup) && staleBackup.data.at("cache") == "new" &&
           !staleBackup.exists("cache.previous"), "replaces stale backup safely");
    std::cout << "PASS: SD/POSIX replacement, rollback, recovery and cleanup failures\n";
}
