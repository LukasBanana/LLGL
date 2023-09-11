/*
 * Display.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DISPLAY_H
#define LLGL_DISPLAY_H


#include <LLGL/Interface.h>
#include <LLGL/DisplayFlags.h>
#include <LLGL/Container/Strings.h>
#include <vector>


namespace LLGL
{


/**
\brief Display interface to query the attributes of all connected displays/monitors.
\remarks Here is an example to print the attributes of all displays:
\code
for (Display* const * myDisplayList = LLGL::Display::GetList(); Display* myDisplay = *myDisplayList; ++myDisplayList) {
    auto myDisplayOffset = myDisplay->GetOffset();
    auto myDisplayMode   = myDisplay->GetDisplayMode();
    std::wcout << L"Display: \""  << myDisplay->GetDeviceName() << L"\"" << std::endl;
    std::cout << "|-Primary = " << std::boolalpha << myDisplay->IsPrimary() << std::endl;
    std::cout << "|-X       = " << myDisplayOffset.x << std::endl;
    std::cout << "|-Y       = " << myDisplayOffset.y << std::endl;
    std::cout << "|-Width   = " << myDisplayMode.resolution.width << std::endl;
    std::cout << "|-Height  = " << myDisplayMode.resolution.height << std::endl;
    std::cout << "`-Hz      = " << myDisplayMode.refreshRate << std::endl;
}
\endcode
*/
class LLGL_EXPORT Display : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Display );

    public:

        /**
        \brief Returns the number of available displays.
        \remarks This function always checks for updates in the display list.
        \see Get
        */
        static std::size_t Count();

        /**
        \brief Returns a null-terminated array of all displays.
        \return Pointer to an array of Display pointers with Count()+1 elements and the last element being null.
        \remarks This function always checks for updates in the display list.
        \see Count
        */
        static Display* const * GetList();

        /**
        \brief Returns the specified display or null if the index is out of bounds.
        \param[in] index Specifies the zero-based index for the display to be returned.
        This should be in the half-open range [0, Count) or null will be returned.
        \remarks This function always checks for updates in the display list.
        \see Count
        \see GetPrimary
        */
        static Display* Get(std::size_t index);

        /**
        \brief Returns the primary display or null if no display can be found.
        \see Get
        */
        static Display* GetPrimary();

        /**
        \brief Shows or hides the cursor for the running application from all displays.
        \param[in] show Specifies whether to show or hide the cursor.
        \remarks In contrast to the Win32 API, this function only shows or hides the cursor,
        while the Win32 API function with the same name either increments or decrements an internal visibility counter for the cursor.
        \return True on success, otherwise cursor visibility changes are not supported.
        \see IsCursorShown
        */
        static bool ShowCursor(bool show);

        /**
        \brief Returns true if the cursor is currently being shown on any display.
        \see ShowCursor
        */
        static bool IsCursorShown();

        //! Sets the cursor to the specified screen coordinate and returns true on success. Otherwise, cursor relocation is not supported.
        static bool SetCursorPosition(const Offset2D& position);

        //! Returns the screen coordiante of the cursor.
        static Offset2D GetCursorPosition();

    public:

        //! Returns true if this is the primary display, as configured by the host system.
        virtual bool IsPrimary() const = 0;

        //! Returns the device name of this display in UTF-8 encoding. This may also be empty, if the platform does not support display names.
        virtual UTF8String GetDeviceName() const = 0;

        /**
        \brief Returns the 2D offset relative to the primary display.
        \remarks This can be used to position your windows accordingly to your displays.
        \see Window::SetPosition
        */
        virtual Offset2D GetOffset() const = 0;

        /**
        \brief Returns the scale factor for this display.
        \remarks This value is used to convert between screen resolution coordinates (see SwapChainDescriptor::resolution) and window coordinates (see WindowDescriptor::size).
        \remarks For high resolution displays, this may have a value of 3 or 2. Otherwise, 1 is the most common value if no extra scaling is necessariy.
        \see SwapChainDescriptor::resolution
        \see WindowDescriptor::size
        */
        virtual float GetScale() const = 0;

        /**
        \brief Resets the display mode to its default value depending on the host system configuration.
        \see SetDisplayMode
        */
        virtual bool ResetDisplayMode() = 0;

        /**
        \brief Sets the new display mode for this display.
        \param[in] displayModeDesc Specifies the descriptor of the new display mode.
        \return True on success, otherwise the specified display mode is not supported by this display and the function has no effect.
        \see GetDisplayMode
        */
        virtual bool SetDisplayMode(const DisplayModeDescriptor& displayModeDesc) = 0;

        /**
        \brief Returns the current display mode of this display.
        \see SetDisplayMode
        */
        virtual DisplayModeDescriptor GetDisplayMode() const = 0;

        /**
        \brief Returns a list of all modes that are supported by this display.
        \remarks This list is always sorted in the following manner:
        The first sorting criterion is the number of pixels (resolution width times resolution height) in ascending order,
        and the second sorting criterion is the refresh rate in ascending order.
        To get only the currently active display mode, use GetDisplayMode.
        \see GetDisplayMode
        */
        virtual std::vector<DisplayModeDescriptor> GetSupportedDisplayModes() const = 0;

    protected:

        /**
        \brief Sorts the specified list of display modes as described in the GetSupportedDisplayModes function, and removes duplicate entries.
        \see GetSupportedDisplayModes
        */
        static void FinalizeDisplayModes(std::vector<DisplayModeDescriptor>& displayModeDescs);

};


} // /namespace LLGL


#endif



// ================================================================================
