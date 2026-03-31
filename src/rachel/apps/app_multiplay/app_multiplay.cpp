/**
 * @file app_multiplay.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-11-04
 *
 */
#include "app_multiplay.h"
#include "spdlog/spdlog.h"
#include "../../hal/hal.h"
#include "../assets/theme/theme.h"

using namespace MOONCAKE::APPS;

static const char* g_main_menu_items[] = {
    "创建房间",
    "加入房间",
    "返回"
};

static const char* g_state_titles[] = {
    "",
    "创建房间",
    "加入房间",
    "等待连接...",
    "已连接"
};

void AppMultiplay::onResume()
{
    spdlog::info("{} onResume", getAppName());
    _data.state = State_t::MainMenu;
    _data.waitButtonReleased = false;
}

void AppMultiplay::onRunning()
{
    int selectedIndex = 0;
    bool needRedraw = true;

    while (true)
    {
        if (needRedraw)
        {
            HAL::GetCanvas()->fillScreen(THEME_COLOR_DARK);
            HAL::GetCanvas()->setTextColor(THEME_COLOR_LIGHT, THEME_COLOR_DARK);

            switch (_data.state)
            {
            case State_t::MainMenu:
                HAL::GetCanvas()->setTextSize(2);
                HAL::GetCanvas()->drawCenterString("联机", 120, 20);
                HAL::GetCanvas()->setTextSize(1);
                for (int i = 0; i < 3; i++)
                {
                    int yPos = 80 + i * 35;
                    if (i == selectedIndex)
                    {
                        HAL::GetCanvas()->fillRect(40, yPos, 160, 28, THEME_COLOR_LIGHT);
                        HAL::GetCanvas()->setTextColor(THEME_COLOR_DARK, THEME_COLOR_LIGHT);
                        HAL::GetCanvas()->drawCenterString(g_main_menu_items[i], 120, yPos + 6);
                        HAL::GetCanvas()->setTextColor(THEME_COLOR_LIGHT, THEME_COLOR_DARK);
                    }
                    else
                    {
                        HAL::GetCanvas()->drawCenterString(g_main_menu_items[i], 120, yPos + 6);
                    }
                }
                break;

            case State_t::CreateRoom:
                HAL::GetCanvas()->setTextSize(2);
                HAL::GetCanvas()->drawCenterString("创建房间", 120, 20);
                HAL::GetCanvas()->setTextSize(1);
                HAL::GetCanvas()->drawCenterString("房间号: 123456", 120, 80);
                HAL::GetCanvas()->drawCenterString("等待玩家加入...", 120, 120);
                HAL::GetCanvas()->drawCenterString("按 B 返回", 120, 200);
                break;

            case State_t::JoinRoom:
                HAL::GetCanvas()->setTextSize(2);
                HAL::GetCanvas()->drawCenterString("加入房间", 120, 20);
                HAL::GetCanvas()->setTextSize(1);
                HAL::GetCanvas()->drawCenterString("请输入房间号", 120, 80);
                HAL::GetCanvas()->drawCenterString("按 B 返回", 120, 200);
                break;

            case State_t::Waiting:
                HAL::GetCanvas()->setTextSize(2);
                HAL::GetCanvas()->drawCenterString("等待连接...", 120, 80);
                HAL::GetCanvas()->setTextSize(1);
                HAL::GetCanvas()->drawCenterString("按 B 返回", 120, 200);
                break;

            case State_t::Connected:
                HAL::GetCanvas()->setTextSize(2);
                HAL::GetCanvas()->drawCenterString("已连接!", 120, 80);
                HAL::GetCanvas()->setTextSize(1);
                HAL::GetCanvas()->drawCenterString("按 B 返回", 120, 200);
                break;
            }

            HAL::CanvasUpdate();
            needRedraw = false;
        }

        if (HAL::GetButton(GAMEPAD::BTN_UP))
        {
            if (!_data.waitButtonReleased)
            {
                if (_data.state == State_t::MainMenu)
                {
                    selectedIndex = (selectedIndex - 1 + 3) % 3;
                    needRedraw = true;
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_DOWN))
        {
            if (!_data.waitButtonReleased)
            {
                if (_data.state == State_t::MainMenu)
                {
                    selectedIndex = (selectedIndex + 1) % 3;
                    needRedraw = true;
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_A))
        {
            if (!_data.waitButtonReleased)
            {
                if (_data.state == State_t::MainMenu)
                {
                    if (selectedIndex == 0)
                    {
                        _data.state = State_t::CreateRoom;
                        needRedraw = true;
                    }
                    else if (selectedIndex == 1)
                    {
                        _data.state = State_t::JoinRoom;
                        needRedraw = true;
                    }
                    else if (selectedIndex == 2)
                    {
                        destroyApp();
                        return;
                    }
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_B))
        {
            if (!_data.waitButtonReleased)
            {
                if (_data.state != State_t::MainMenu)
                {
                    _data.state = State_t::MainMenu;
                    selectedIndex = 0;
                    needRedraw = true;
                }
                _data.waitButtonReleased = true;
            }
        }
        else
        {
            _data.waitButtonReleased = false;
        }

        HAL::Delay(50);
    }
}

void AppMultiplay::onDestroy()
{
    spdlog::info("{} onDestroy", getAppName());
}