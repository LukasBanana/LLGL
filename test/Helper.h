/*
 * Helper.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEST_HELPER_H__
#define __LLGL_TEST_HELPER_H__


#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <fstream>


inline std::string ReadFileContent(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.good())
        throw std::runtime_error("failed to open file: \"" + filename + "\"");

    std::string content(
        ( std::istreambuf_iterator<char>(file) ),
        ( std::istreambuf_iterator<char>() )
    );

    return content;
}

class TestDebugger : public LLGL::RenderingDebugger
{

    public:

        TestDebugger(std::size_t messageLimit = 1) :
            messageLimit_( messageLimit )
        {
        }

    private:

        void OnError(LLGL::ErrorType type, Message& message) override
        {
            std::cerr << "ERROR: " << message.GetSource() << ": " << message.GetText() << std::endl;
            message.BlockAfter(messageLimit_);
        }

        void OnWarning(LLGL::WarningType type, Message& message) override
        {
            std::cerr << "WARNING: " << message.GetSource() << ": " << message.GetText() << std::endl;
            message.BlockAfter(messageLimit_);
        }

        std::size_t messageLimit_ = 1;

};


#endif

