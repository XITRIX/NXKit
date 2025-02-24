//
// Created by Daniil Vinogradov on 12/01/2024.
//

#pragma once

namespace NXKit {

class UIScrollViewDecelerationRate {
public:
    enum Value {
        normal,
        fast
    };

    UIScrollViewDecelerationRate() = default;
    constexpr UIScrollViewDecelerationRate(Value value) : value(value) { }

    // Allow switch and comparisons.
    constexpr operator Value() const { return value; }

    // Prevent usage: if(fruit)
    explicit operator bool() const = delete;

    [[nodiscard]] constexpr float rawValue() const {
        switch (value) {
            case normal: {
                return 0.998f;
            }
            case fast: {
                return 0.99f;
            }
        }
    }

private:
    Value value;
};
}