/*
 * Test_Container.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/Strings.h>
#include <iostream>
#include <sstream>
#include <vector>


static const int g_timerCount = 16;
static int g_currentTimer;
static std::unique_ptr<LLGL::Timer> g_timer[g_timerCount];

static LLGL::Timer* GetNextTimer()
{
    g_currentTimer = (g_currentTimer + 1) % g_timerCount;
    if (!g_timer[g_currentTimer])
        g_timer[g_currentTimer] = LLGL::Timer::Create();
    return g_timer[g_currentTimer].get();
}

class StopWatchScope
{

    public:

        StopWatchScope(const char* name) :
            name_  { name           },
            timer_ { GetNextTimer() }
        {
            timer_->Start();
        }

        ~StopWatchScope()
        {
            auto ticks = timer_->Stop();
            auto elapsedTime = (static_cast<double>(ticks) / static_cast<double>(timer_->GetFrequency())) * 1000.0;
            printf("%s: %fms\n", name_, elapsedTime);
        }

    private:

        const char*     name_;
        LLGL::Timer*    timer_;

};

int main()
{
    try
    {
        LLGL::UTF8String sa, sb, sc, sd;

        std::wstring sc_orig = L"\u3053\u3093\u306B\u3061\u306F\u4E16\u754C\u3002";
        sc = sc_orig.c_str();
        LLGL::SmallVector<wchar_t> sc_array = sc.ToWCharArray();
        std::wstring sc_back = sc_array.data();

        sa = "Hello";
        sb = L"World";
        sd = sa + " " + sb + "\n" + sc;

        std::string s = sd.c_str();

        LLGL::SmallVector<wchar_t> wsd = sd.ToWCharArray();
        std::wstring ws = wsd.data();

        std::cout << s.c_str() << std::endl;
        std::wcout << ws.c_str() << std::endl;

        struct CustomGrowth
        {
            static inline std::size_t capacity(std::size_t size)
            {
                return size + size / 2;
            }
        };

        for (int n = 0; n < 10; ++n)
        {
            {
                StopWatchScope scope{ "LLGL::ArrayList<int>::push_back(0 .. 10000000)" };

                LLGL::SmallVector<int> l1;
                l1.reserve(10000000);
                for (int i = 0; i < 10000000; ++i)
                    l1.push_back(i);
            }

            {
                StopWatchScope scope{ "std::vector<int>::push_back(0 .. 10000000)" };

                std::vector<int> l2;
                l2.push_back(1);
                l2.reserve(10000000);
                for (int i = 0; i < 10000000; ++i)
                    l2.push_back(i);
            }
        }


        #ifdef _WIN32
        system("pause");
        #endif
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
