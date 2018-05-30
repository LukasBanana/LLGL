/*
 * Display.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DISPLAY_H
#define LLGL_DISPLAY_H


#include "NonCopyable.h"
#include "DisplayFlags.h"
#include <vector>
#include <memory>


namespace LLGL
{


/**
\brief Display interface to query the attributes of all connected displays/monitors.
\remarks Here is an example to print the attributes of all displays:
\code
auto myDisplayList = LLGL::Display::QueryList();
for (const auto& myDisplay : myDisplayList) {
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
class LLGL_EXPORT Display : public NonCopyable
{

    public:

        //! Queries the list of all connected displays.
        static std::vector<std::unique_ptr<Display>> QueryList();

        /**
        \brief Queries the primary display.
        \return Unique pointer to a Display instance that represents the primary display, or null on failure.
        */
        static std::unique_ptr<Display> QueryPrimary();

        //! Returns true if this is the primary display, as configured by the host system.
        virtual bool IsPrimary() const = 0;

        //! Returns the device name of this display. This may also be empty, if the platform does not support display names.
        virtual std::wstring GetDeviceName() const = 0;

        /**
        \brief Returns the 2D offset relative to the primary display.
        \remarks This can be used to position your windows accordingly to your displays.
        \see Window::SetPosition
        */
        virtual Offset2D GetOffset() const = 0;

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
        virtual std::vector<DisplayModeDescriptor> QuerySupportedDisplayModes() const = 0;

    protected:

        /**
        \brief Sorts the specified list of display modes as described in the QuerySupportedDisplayModes function, and removes duplicate entries.
        \see QuerySupportedDisplayModes
        */
        static void FinalizeDisplayModes(std::vector<DisplayModeDescriptor>& displayModeDescs);

};


} // /namespace LLGL


#endif



// ================================================================================
