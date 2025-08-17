/*
 * LinuxDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_DISPLAY_H
#define LLGL_LINUX_DISPLAY_H

#include <LLGL/Display.h>

namespace LLGL {

class LinuxDisplay : public Display {

    public:

        virtual bool SetCursorPositionInternal(const Offset2D& position) = 0;

        virtual Offset2D GetCursorPositionInternal() = 0;
    
    private:

};

} // /namespace LLGL

#endif // LLGL_LINUX_DISPLAY_H