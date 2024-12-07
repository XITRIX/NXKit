#pragma once

typedef double NXFloat;

struct NXSize {
    NXFloat width;
    NXFloat height;

    NXSize();
    NXSize(NXFloat width, NXFloat height);
};
