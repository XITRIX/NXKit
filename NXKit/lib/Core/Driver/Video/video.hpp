/*
    Copyright 2021 natinusala

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <nanovg.h>
#include <Core/UITraitCollection/UITraitCollection.hpp>
#include <Core/Utils/SharedBase/SharedBase.hpp>

namespace NXKit {

// A VideoContext is responsible for providing a nanovg context for the app
// (so by extension it manages all the graphics state as well as the window / context).
// The VideoContext implementation must also provide the nanovg implementation. As such, there
// can only be one VideoContext linked at any time in the binary.
// Context creation and teardown can be done in the constructor and destructor.
class VideoContext: public enable_shared_from_base<VideoContext>
{
public:
    virtual ~VideoContext() {};
    
    /**
     * Called at the beginning of every frame to clear the window
     */
    virtual void clear(NVGcolor color) = 0;
    
    /**
     * Called at the beginning of every frame to begin it.
     */
    virtual void beginFrame() = 0;
    
    /**
     * Called at the end of every frame to end it (swap buffers...).
     */
    virtual void endFrame() = 0;
    
    virtual void disableScreenDimming(bool disable) = 0;
    
    virtual void getContextPixels(int x, int y, int w, int h, unsigned char** pixels) = 0;
    
    virtual float getScaleFactor() = 0;
    
    virtual bool mainLoopInteraction() = 0;

    virtual UIUserInterfaceStyle getUserInterfaceStyle() { return UIUserInterfaceStyle::unspecified; }
    
    /**
     * Can be called by the application to reset the graphics
     * state, in case there is a need to use the graphics API
     * directly (for instance direct OpenGL calls).
     */
    virtual void resetState() = 0;
    
    virtual NVGcontext* getNVGContext() = 0;
};

}
