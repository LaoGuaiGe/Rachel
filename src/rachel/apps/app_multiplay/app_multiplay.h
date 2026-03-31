/**
 * @file app_multiplay.h
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-11-04
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
        };
        Data_t _data;

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