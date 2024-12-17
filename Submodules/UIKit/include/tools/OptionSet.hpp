#pragma once

namespace NXKit {

template<typename T>
struct OptionSet {
public:
    OptionSet(): OptionSet(0) {}
    OptionSet(int initValue): _value(initValue) {}
    void include(T value) { _value |= value; }
    void exclude(T value) { _value &= ~value; }
    bool contains(T value) { return _value & value; }
private:
    int _value;
};

}
