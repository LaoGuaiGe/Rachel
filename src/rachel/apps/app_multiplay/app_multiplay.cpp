/**
 * @file app_multiplay.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.4
 * @date 2024-01-01
 *
 */
#include "app_multiplay.h"
#include "spdlog/spdlog.h"
#include "../../hal/hal.h"
#include "../assets/theme/theme.h"

using namespace MOONCAKE::APPS;

static const char* g_main_menu_items[] = {
    "Create Room",
    "Join Room",
    "Back"
};

void AppMultiplay::onResume()
{
    spdlog::info("{} onResume", getAppName());
    _data.state = State_t::MainMenu;
    _data.waitButtonReleased = false;
    _data.selectedIndex = 0;
    _data.animValue = 0;
    _data.scanlineY = 0;
}

void AppMultiplay::_drawGridBackground()
{
    auto* canvas = HAL::GetCanvas();
    canvas->drawRect(15, 25, 210, 190, (uint32_t)(0x2a2a2a));
    for (int x = 15; x <= 225; x += 20) {
        canvas->drawLine(x, 25, x, 215, (uint32_t)(0x1a1a1a));
    }
    for (int y = 25; y <= 215; y += 20) {
        canvas->drawLine(15, y, 225, y, (uint32_t)(0x1a1a1a));
    }
}

void AppMultiplay::_drawScanlines()
{
    auto* canvas = HAL::GetCanvas();
    canvas->fillRect(0, _data.scanlineY, 240, 2, (uint32_t)(0x111111));
    _data.scanlineY += 4;
    if (_data.scanlineY >= 240) _data.scanlineY = 0;
}

void AppMultiplay::_drawGlowTitle(const char* title, int v)
{
    auto* canvas = HAL::GetCanvas();
    uint32_t colors[] = {0x4a9eff, 0x00d4ff, 0x4aff9e};
    uint32_t glowColor = colors[(v / 50) % 3];

    int titleW = canvas->textWidth(title, nullptr) + 20;
    int titleX = 120 - titleW / 2;
    int titleY = 8;

    int titleH = 32;
    int cornerSize = 8;

    canvas->fillRect(titleX + cornerSize, titleY, titleW - cornerSize * 2, titleH, (uint32_t)(0x1a1a1a));
    canvas->fillTriangle(titleX, titleY, titleX + cornerSize, titleY, titleX, titleY + cornerSize, (uint32_t)(0x1a1a1a));
    canvas->fillTriangle(titleX + titleW - 1, titleY, titleX + titleW - cornerSize, titleY, titleX + titleW - 1, titleY + cornerSize, (uint32_t)(0x1a1a1a));
    canvas->fillTriangle(titleX, titleY + titleH - 1, titleX, titleY + titleH - cornerSize - 1, titleX + cornerSize, titleY + titleH - 1, (uint32_t)(0x1a1a1a));
    canvas->fillTriangle(titleX + titleW - 1, titleY + titleH - 1, titleX + titleW - 1, titleY + titleH - cornerSize - 1, titleX + titleW - cornerSize, titleY + titleH - 1, (uint32_t)(0x1a1a1a));

    canvas->drawLine(titleX + cornerSize, titleY, titleX + titleW - cornerSize, titleY, glowColor);
    canvas->drawLine(titleX + cornerSize, titleY + titleH - 1, titleX + titleW - cornerSize, titleY + titleH - 1, glowColor);
    canvas->drawTriangle(titleX, titleY, titleX + cornerSize, titleY, titleX, titleY + cornerSize, glowColor);
    canvas->drawTriangle(titleX + titleW - 1, titleY, titleX + titleW - cornerSize, titleY, titleX + titleW - 1, titleY + cornerSize, glowColor);
    canvas->drawTriangle(titleX, titleY + titleH - 1, titleX, titleY + titleH - cornerSize - 1, titleX + cornerSize, titleY + titleH - 1, glowColor);
    canvas->drawTriangle(titleX + titleW - 1, titleY + titleH - 1, titleX + titleW - 1, titleY + titleH - cornerSize - 1, titleX + titleW - cornerSize, titleY + titleH - 1, glowColor);

    for (int i = 2; i > 0; i--) {
        canvas->setTextColor(glowColor, (uint32_t)(0x1a1a1a));
        canvas->setTextSize(1);
        canvas->drawCenterString(title, 120, titleY + 3);
    }
    canvas->setTextColor((uint32_t)0xffffff, (uint32_t)(0x1a1a1a));
    canvas->setTextSize(1);
    canvas->drawCenterString(title, 120, titleY + 3);
}

void AppMultiplay::_drawMenuItem(int index, const char* text, bool selected, int v)
{
    auto* canvas = HAL::GetCanvas();
    int baseY = 65 + index * 40;

    if (selected) {
        int bounce = (v % 20) > 10 ? -1 : 1;
        canvas->fillRoundRect(35, baseY + bounce, 170, 32, 8, (uint32_t)(0x4a9eff));
        canvas->fillRoundRect(37, baseY + 2 + bounce, 166, 28, 6, (uint32_t)(0x2d7dd9));

        canvas->setTextColor((uint32_t)0xffffff, (uint32_t)(0x2d7dd9));
        canvas->setTextSize(1);
        canvas->drawCenterString(text, 120, baseY + 5 + bounce);
    } else {
        canvas->setTextColor((uint32_t)(0x888888), THEME_COLOR_DARK);
        canvas->setTextSize(1);
        canvas->drawCenterString(text, 120, baseY + 5);
    }
}

void AppMultiplay::_drawWifiSignal(int x, int y, int strength, uint32_t color)
{
    auto* canvas = HAL::GetCanvas();
    if (strength >= 1) canvas->drawArc(x, y, 8, 200, 340, color);
    if (strength >= 2) canvas->drawArc(x, y, 14, 210, 330, color);
    if (strength >= 3) canvas->drawArc(x, y, 20, 220, 320, color);
    if (strength >= 4) canvas->drawArc(x, y, 26, 230, 310, color);
    canvas->fillCircle(x, y + 2, 3, color);
}

void AppMultiplay::_drawPulsingWifi(int x, int y, int v)
{
    auto* canvas = HAL::GetCanvas();
    uint32_t colors[] = {0x4a9eff, 0x00d4ff, 0x4aff9e};

    for (int i = 0; i < 4; i++) {
        int strength = ((v / 10) + i) % 4 + 1;
        _drawWifiSignal(x, y, strength, colors[i % 3]);
    }
}

void AppMultiplay::_drawConnectedAnim(int x, int y, int v)
{
    auto* canvas = HAL::GetCanvas();
    int pulse = (v % 20);
    canvas->drawCircle(x, y, 30 + pulse / 2, (uint32_t)(0x00ff00));
    canvas->drawCircle(x, y, 25, (uint32_t)(0x00ff00));
    canvas->fillCircle(x, y, 3, (uint32_t)(0x00ff00));
}

void AppMultiplay::_drawProgressBar(int v)
{
    auto* canvas = HAL::GetCanvas();
    int barWidth = 180;
    int barX = 30;
    int barY = 165;

    canvas->fillRoundRect(barX, barY, barWidth, 8, 4, (uint32_t)(0x333333));

    int progress = (v % 100);
    uint32_t colors[] = {0x4a9eff, 0x00d4ff, 0x4aff9e};
    uint32_t barColor = colors[(v / 50) % 3];
    canvas->fillRoundRect(barX, barY, barWidth * progress / 100, 8, 4, barColor);
}

void AppMultiplay::_drawHint(const char* text, int v)
{
    auto* canvas = HAL::GetCanvas();
    canvas->fillRect(0, 215, 240, 25, (uint32_t)(0xffffff));
    canvas->setTextColor((uint32_t)(0x000000), (uint32_t)(0xffffff));
    canvas->setTextSize(1);
    canvas->drawCenterString(text, 120, 215);
}

void AppMultiplay::_drawControlsHint(int v)
{
    auto* canvas = HAL::GetCanvas();
    canvas->fillRect(0, 215, 240, 25, (uint32_t)(0xffffff));
    canvas->setTextSize(1);

    if (_data.state == State_t::MainMenu) {
        canvas->setTextColor((uint32_t)(0x000000), (uint32_t)(0xffffff));
        canvas->drawCenterString("↑↓ Sel  A OK  B Back", 120, 215);
    }
    else {
        canvas->setTextColor((uint32_t)(0x000000), (uint32_t)(0xffffff));
        canvas->drawCenterString("B Back", 120, 215);
    }
}

void AppMultiplay::_drawFrame()
{
    auto* canvas = HAL::GetCanvas();
    canvas->fillScreen(THEME_COLOR_DARK);
    _drawGridBackground();
    _drawScanlines();

    int v = _data.animValue;

    switch (_data.state)
    {
    case State_t::MainMenu:
        _drawGlowTitle("Multiplay", v);
        for (int i = 0; i < 3; i++) {
            _drawMenuItem(i, g_main_menu_items[i], i == _data.selectedIndex, v);
        }
        _drawControlsHint(v);
        break;

    case State_t::CreateRoom:
        _drawGlowTitle("Create Room", v);

        canvas->setTextSize(0);
        canvas->setTextColor((uint32_t)(0x888888), THEME_COLOR_DARK);
        canvas->drawCenterString("Room: RACHEL_001", 120, 42);

        canvas->setTextColor((uint32_t)(0x666666), THEME_COLOR_DARK);
        canvas->drawCenterString("Players:", 120, 66);

        {
            uint32_t userColors[] = {0x00ff00, 0x4a9eff, 0xffd700, 0xff6b6b};
            const char* userNames[] = {"P1", "P2", "P3", "P4"};

            for (int i = 0; i < 4; i++) {
                int y = 93 + i * 24;
                uint32_t color = userColors[i];
                bool connected = (i == 0) || ((v / 20 + i) % 4 == 0);

                if (connected) {
                    // canvas->fillCircle(45, y, 4, color);
                    canvas->setTextColor(color, THEME_COLOR_DARK);
                    canvas->drawString(userNames[i], 58, y - 3);

                    if (i == 0) {
                        canvas->setTextColor((uint32_t)(0x00aa00), THEME_COLOR_DARK);
                        canvas->drawString(" <-You", 85, y - 3);
                        for (int j = 0; j < 3; j++) {
                            canvas->drawRect(j, j, 240 - j * 2, 215 - j * 2, (uint32_t)(0x00ff00));
                        }
                    }
                } else {
                    // canvas->drawCircle(45, y, 4, (uint32_t)(0x444444));
                    canvas->setTextColor((uint32_t)(0x444444), THEME_COLOR_DARK);
                    canvas->drawString(userNames[i], 58, y - 3);
                }
            }
        }

        canvas->setTextSize(0);
        _drawControlsHint(v);
        break;

    case State_t::JoinRoom:
        {
        _drawGlowTitle("Join Room", v);
        canvas->setTextSize(0);
        canvas->setTextColor((uint32_t)(0xaaaaaa), THEME_COLOR_DARK);
        canvas->drawCenterString("Scanning...", 120, 55);
        int strength = (v / 20) % 4 + 1;
        _drawWifiSignal(120, 95, strength, (uint32_t)(0x4a9eff));
        canvas->setTextColor((uint32_t)(0x666666), THEME_COLOR_DARK);
        canvas->drawCenterString("No room found", 120, 140);
        _drawControlsHint(v);
        }
        break;

    case State_t::Waiting:
        _drawGlowTitle("Waiting", v);
        canvas->setTextSize(0);
        canvas->setTextColor((uint32_t)(0xaaaaaa), THEME_COLOR_DARK);
        canvas->drawCenterString("Waiting for players...", 120, 55);
        _drawPulsingWifi(120, 95, v);
        _drawControlsHint(v);
        break;

    case State_t::Connected:
        _drawGlowTitle("Connected", v);
        canvas->setTextSize(0);
        canvas->setTextColor((uint32_t)(0x00ff00), THEME_COLOR_DARK);
        canvas->drawCenterString("Connected!", 120, 55);
        _drawConnectedAnim(120, 100, v);
        canvas->setTextColor((uint32_t)(0x888888), THEME_COLOR_DARK);
        canvas->drawCenterString("Ready to play", 120, 150);
        _drawControlsHint(v);
        break;
    }
}

void AppMultiplay::onRunning()
{
    _data.selectedIndex = 0;
    _data.animValue = 0;
    _data.scanlineY = 0;
    uint32_t lastUpdate = HAL::Millis();

    while (true)
    {
        uint32_t currentTime = HAL::Millis();
        if (currentTime - lastUpdate >= 50) {
            lastUpdate = currentTime;
            _data.animValue++;
            _drawFrame();
            HAL::CanvasUpdate();
        }

        if (HAL::GetButton(GAMEPAD::BTN_UP)) {
            if (!_data.waitButtonReleased) {
                if (_data.state == State_t::MainMenu) {
                    _data.selectedIndex = (_data.selectedIndex - 1 + 3) % 3;
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_DOWN)) {
            if (!_data.waitButtonReleased) {
                if (_data.state == State_t::MainMenu) {
                    _data.selectedIndex = (_data.selectedIndex + 1) % 3;
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_A)) {
            if (!_data.waitButtonReleased) {
                if (_data.state == State_t::MainMenu) {
                    if (_data.selectedIndex == 0) {
                        _data.state = State_t::CreateRoom;
                    }
                    else if (_data.selectedIndex == 1) {
                        _data.state = State_t::JoinRoom;
                    }
                    else if (_data.selectedIndex == 2) {
                        destroyApp();
                        return;
                    }
                }
                _data.waitButtonReleased = true;
            }
        }
        else if (HAL::GetButton(GAMEPAD::BTN_B)) {
            if (!_data.waitButtonReleased) {
                if (_data.state == State_t::MainMenu) {
                    destroyApp();
                    return;
                }
                else {
                    _data.state = State_t::MainMenu;
                    _data.selectedIndex = 0;
                }
                _data.waitButtonReleased = true;
            }
        }
        else {
            _data.waitButtonReleased = false;
        }

        HAL::Delay(10);
    }
}

void AppMultiplay::onDestroy()
{
    spdlog::info("{} onDestroy", getAppName());
}
