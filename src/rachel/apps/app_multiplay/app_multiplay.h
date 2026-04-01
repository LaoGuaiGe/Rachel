/**
 * @file app_multiplay.h
 * @author Forairaaaaa
 * @brief
 * @version 0.4
 * @date 2024-01-01
 *
 */
#pragma once
#include <mooncake.h>
#include "../assets/icons/icons.h"

namespace MOONCAKE::APPS
{
    class AppMultiplay : public APP_BASE
    {
    private:
        enum class State_t : uint8_t
        {
            MainMenu,
            CreateRoom,
            JoinRoom,
            Waiting,
            Connected
        };
        struct Data_t
        {
            State_t state = State_t::MainMenu;
            bool waitButtonReleased = false;
            int selectedIndex = 0;
            int animValue = 0;
            int scanlineY = 0;
        };
        Data_t _data;

        void _drawGridBackground();
        void _drawScanlines();
        void _drawGlowTitle(const char* title, int v);
        void _drawMenuItem(int index, const char* text, bool selected, int v);
        void _drawWifiSignal(int x, int y, int strength, uint32_t color);
        void _drawPulsingWifi(int x, int y, int v);
        void _drawConnectedAnim(int x, int y, int v);
        void _drawProgressBar(int v);
        void _drawHint(const char* text, int v);
        void _drawControlsHint(int v);
        void _drawFrame();

    public:
        void onResume() override;
        void onRunning() override;
        void onDestroy() override;
    };

    class AppMultiplay_Packer : public APP_PACKER_BASE
    {
        std::string getAppName() override { return "联机"; }
        void* getAppIcon() override { return (void*)image_data_icon_app_default; }
        void* newApp() override { return new AppMultiplay; }
        void deleteApp(void* app) override { delete (AppMultiplay*)app; }
    };
} // namespace MOONCAKE::APPS
