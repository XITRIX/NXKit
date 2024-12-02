//
// Created by Даниил Виноградов on 17.11.2024.
//

#pragma once


#include <tools/sk_app/Application.h>
#include "tools/sk_app/Window.h"
#include "tools/sk_app/CommandSet.h"
#include "include/core/SkFontMgr.h"

class UIApplication: public sk_app::Application, sk_app::Window::Layer {
public:
    UIApplication(int argc, char** argv, void* platformData);
    ~UIApplication();

    void onIdle() override;
    void onBackendCreated() override;
    void onResize(int width, int height) override;
    void onPaint(SkSurface *surface) override;

private:
    sk_app::Window* fWindow = nullptr;
    sk_sp<SkFontMgr> mgr;
    sk_sp<SkTypeface> typeface;

    float fRotationAngle = 0;
};