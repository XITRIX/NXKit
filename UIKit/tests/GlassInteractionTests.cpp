#include "GlassInteraction.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace NXKit;
using namespace NXKit::Internal;

namespace {
int failures = 0;
void expect(bool condition, const char* message) {
    if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
struct Event { double time; int phase; NXPoint delta; };
struct Frame { double time; std::array<double, 4> geometry; };
struct Reference {
    std::string name;
    NXSize size;
    std::array<double, 3> alignment{};
    std::vector<Event> events;
    std::vector<Frame> frames;
};

void replay(const Reference& reference, std::ostream* csv) {
    GlassInteraction motion;
    double time = 0, squareError = 0, maximumError = 0;
    size_t index = 0, count = 0;
    auto events = reference.events;
    for (auto& event : events) event.time += reference.alignment[event.phase == 0 ? 0 : event.phase == 1 ? 1 : 2];
    std::stable_sort(events.begin(), events.end(), [](auto a, auto b) { return a.time < b.time; });
    auto advanceTo = [&](double next) {
        motion.advance(std::max(0.0, next-time), reference.size);
        time = next;
    };
    for (const auto& frame : reference.frames) {
        while (index < events.size() && events[index].time <= frame.time) {
            const auto& event = events[index++];
            advanceTo(event.time);
            motion.pressed = event.phase == 0 || event.phase == 1;
            if (motion.pressed) motion.move(NXPoint(reference.size.width*.5f,reference.size.height*.5f) + event.delta, event.delta);
        }
        advanceTo(frame.time);
        const auto transform = motion.transform(reference.size);
        const std::array<double, 4> geometry = {
            transform.tX + reference.size.width * .5 * (transform.m11 - 1),
            transform.tY + reference.size.height * .5 * (transform.m22 - 1),
            reference.size.width * transform.m11, reference.size.height * transform.m22
        };
        for (size_t component = 0; component < 4; ++component) {
            const double error = geometry[component]-frame.geometry[component];
            squareError += error*error; maximumError = std::max(maximumError, std::abs(error)); ++count;
        }
        if (csv) {
            *csv << reference.name << ',' << frame.time;
            for (double value : frame.geometry) *csv << ',' << value;
            for (double value : geometry) *csv << ',' << value;
            *csv << '\n';
        }
    }
    const double rmse = std::sqrt(squareError/count);
    std::cout << reference.name << " RMSE " << rmse << " pt, max " << maximumError << " pt\n";
    expect(count > 100, "native reference contains a real sampled time series");
    expect(rmse < .12 && maximumError < 1.25, "recorded UIKit press/pull/release geometry stays within acquisition tolerance");
}

void invariants() {
    for (NXSize size : {NXSize(52,52), NXSize(80,80), NXSize(120,120), NXSize(160,52), NXSize(240,240)}) {
        GlassInteraction sixty, oneTwenty;
        sixty.pressed = oneTwenty.pressed = true;
        sixty.move({}, {160,80}); oneTwenty.move({}, {160,80});
        for (int i = 0; i < 30; ++i) sixty.advance(1.0/60,size);
        for (int i = 0; i < 60; ++i) oneTwenty.advance(1.0/120,size);
        expect(std::abs(sixty.inflation.value-oneTwenty.inflation.value)<1e-9
            && std::abs(sixty.dragX.value-oneTwenty.dragX.value)<1e-9, "native-calibrated motion is frame-rate independent");
        const double before = sixty.inflation.value;
        sixty.pressed = false;
        expect(sixty.inflation.value == before, "cancellation retains presentation and velocity");
        sixty.advance(.05,size); sixty.pressed = true; sixty.advance(.04,size);
        sixty.advance(.1,{240,52});
        expect(std::isfinite(sixty.transform({240,52}).m11), "resize during interrupted motion stays finite");
        sixty.pressed = false; sixty.advance(10,size);
        expect(sixty.atRest() && sixty.transform(size).isIdentity(), "release restores exact unmodified geometry");
    }
    GlassInteraction diagonal, horizontal;
    diagonal.pressed = horizontal.pressed = true;
    diagonal.move({}, {80,80}); horizontal.move({}, {80,0});
    diagonal.advance(2,{52,52}); horizontal.advance(2,{52,52});
    expect(diagonal.dragX.value == horizontal.dragX.value, "diagonal pull does not reduce either axis's resistance");
    expect(std::abs(diagonal.transform({52,52}).m11-diagonal.transform({52,52}).m22)<1e-9,
        "equal diagonal pulls preserve square inflation");
    const auto presentation = horizontal.transform({52,52});
    horizontal.move({}, {-80,0});
    expect(horizontal.transform({52,52}) == presentation, "reversing pull retains shape and translation before the next spring step");
    horizontal.advance(2,{52,52});
    expect(horizontal.dragX.value < 0 && horizontal.stretchX.value > 0,
        "reversed pull changes translation direction while preserving elongation axis");
}
}

int main(int argc, char** argv) {
    std::filesystem::path fixture = argc > 1 ? argv[1] : NXKIT_GLASS_INTERACTION_REFERENCE;
    if (argc == 1) {
        const auto bundled = std::filesystem::path(argv[0]).parent_path()/"uikit-reference.txt";
        if (std::filesystem::exists(bundled)) fixture = bundled;
    }
    std::ifstream input(fixture);
    if (!input) { std::cerr << "Cannot open native reference: " << fixture << '\n'; return 1; }
    std::ofstream csv;
    if (argc > 2) { csv.open(argv[2]); csv << "case,t,nativeX,nativeY,nativeW,nativeH,nxX,nxY,nxW,nxH\n"; }
    Reference reference;
    int cases = 0;
    for (std::string line; std::getline(input,line);) {
        std::istringstream stream(line); std::string kind; stream >> kind;
        if (kind == "case") { reference = {}; stream >> reference.name >> reference.size.width >> reference.size.height; }
        else if (kind == "alignment") for (auto& value : reference.alignment) stream >> value;
        else if (kind == "event") { Event event; stream >> event.time >> event.phase >> event.delta.x >> event.delta.y; reference.events.push_back(event); }
        else if (kind == "frame") { Frame frame; stream >> frame.time; for (auto& value : frame.geometry) stream >> value; reference.frames.push_back(frame); }
        else if (kind == "end") { replay(reference,csv ? &csv : nullptr); ++cases; }
    }
    expect(cases >= 30, "native replay covers size boundary, rectangles, diagonal and far pulls, and both themes");
    invariants();
    return failures ? 1 : 0;
}
