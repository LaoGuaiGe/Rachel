/**
 * @file network_manager.cpp
 * @author Forairaaaaa
 * @brief Network manager for multiplay
 * @version 0.1
 * @date 2024-01-01
 */
#include "network_manager.h"
#include <stdio.h>

namespace MULTIPLAY
{
    NetworkManager::NetworkManager()
    {
    }

    NetworkManager::~NetworkManager()
    {
        deinit();
    }

    void NetworkManager::init(const NetworkConfig_t& config)
    {
        _config = config;
        _mode = NetworkMode_t::None;
        _status = NetworkStatus_t::Idle;
        printf("NetworkManager init\n");
    }

    void NetworkManager::deinit()
    {
        stopAP();
        stopSTA();
        _mode = NetworkMode_t::None;
        _status = NetworkStatus_t::Idle;
        printf("NetworkManager deinit\n");
    }

    bool NetworkManager::startAP()
    {
        if (_mode == NetworkMode_t::AP)
        {
            return true;
        }

        stopSTA();

        printf("Start AP: %s\n", _config.ap_ssid);

        // Start DNS server
        if (_dns_server == nullptr)
        {
            _dns_server = new DNSServer();
        }

        // Start AP
        WiFi.mode(WIFI_AP);
        if (!WiFi.softAP(_config.ap_ssid, _config.ap_password))
        {
            printf("Failed to start AP\n");
            if (_dns_server)
            {
                delete _dns_server;
                _dns_server = nullptr;
            }
            return false;
        }

        // Configure AP IP
        IPAddress ap_ip(192, 168, 4, 1);
        IPAddress ap_gateway(192, 168, 4, 1);
        IPAddress ap_subnet(255, 255, 255, 0);
        WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet);

        // Start DNS server
        _dns_server->start(53, "*", ap_ip);

        // Start server
        if (_server == nullptr)
        {
            _server = new WiFiServer(_config.server_port);
        }
        _server->begin();
        _server->setNoDelay(true);

        _mode = NetworkMode_t::AP;
        _status = NetworkStatus_t::Connected;
        _last_activity = millis();

        printf("AP started, IP: %s\n", getAPIP().c_str());

        return true;
    }

    bool NetworkManager::stopAP()
    {
        if (_mode != NetworkMode_t::AP)
        {
            return true;
        }

        printf("Stop AP\n");

        // Stop server
        if (_server)
        {
            _server->stop();
            delete _server;
            _server = nullptr;
        }

        // Stop DNS server
        if (_dns_server)
        {
            _dns_server->stop();
            delete _dns_server;
            _dns_server = nullptr;
        }

        // Clear clients
        _clients.clear();

        // Stop AP mode
        WiFi.mode(WIFI_STA);

        _mode = NetworkMode_t::None;
        _status = NetworkStatus_t::Idle;

        printf("AP stopped\n");

        return true;
    }

    bool NetworkManager::startSTA()
    {
        if (_mode == NetworkMode_t::STA)
        {
            return true;
        }

        stopAP();

        printf("Start STA: %s\n", _config.ap_ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(_config.ap_ssid, _config.ap_password);

        _mode = NetworkMode_t::STA;
        _status = NetworkStatus_t::Connecting;
        _last_activity = millis();

        return true;
    }

    bool NetworkManager::startSTA(const char* ssid, const char* password)
    {
        if (_mode == NetworkMode_t::STA)
        {
            return true;
        }

        stopAP();

        printf("Start STA: %s\n", ssid);

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        _mode = NetworkMode_t::STA;
        _status = NetworkStatus_t::Connecting;
        _last_activity = millis();

        return true;
    }

    bool NetworkManager::stopSTA()
    {
        if (_mode != NetworkMode_t::STA && _mode != NetworkMode_t::Connected)
        {
            return true;
        }

        printf("Stop STA\n");

        disconnectFromServer();

        WiFi.disconnect();

        _mode = NetworkMode_t::None;
        _status = NetworkStatus_t::Idle;

        printf("STA stopped\n");

        return true;
    }

    bool NetworkManager::isConnected() const
    {
        return (_mode == NetworkMode_t::AP && _server != nullptr) ||
               (_mode == NetworkMode_t::Connected && _client != nullptr && _client->connected());
    }

    NetworkMode_t NetworkManager::getMode() const
    {
        return _mode;
    }

    NetworkStatus_t NetworkManager::getStatus() const
    {
        return _status;
    }

    int NetworkManager::getConnectedClientCount() const
    {
        if (_mode != NetworkMode_t::AP || _server == nullptr)
        {
            return 0;
        }

        int count = 0;
        for (size_t i = 0; i < _clients.size(); i++)
        {
            if (_clients[i].connected())
            {
                count++;
            }
        }
        return count;
    }

    void NetworkManager::broadcast(const String& data)
    {
        broadcast((const uint8_t*)data.c_str(), data.length());
    }

    void NetworkManager::broadcast(const uint8_t* data, size_t len)
    {
        if (_mode != NetworkMode_t::AP)
        {
            return;
        }

        for (size_t i = 0; i < _clients.size(); i++)
        {
            if (_clients[i].connected())
            {
                _clients[i].write(data, len);
            }
        }
        _last_activity = millis();
    }

    String NetworkManager::readFromClient(size_t client_idx)
    {
        if (_mode != NetworkMode_t::AP || client_idx >= _clients.size())
        {
            return "";
        }

        WiFiClient& client = _clients[client_idx];
        String data = "";
        while (client.available())
        {
            data += (char)client.read();
        }
        if (data.length() > 0)
        {
            _last_activity = millis();
        }
        return data;
    }

    bool NetworkManager::writeToFileClient(size_t client_idx, const String& data)
    {
        return writeToFileClient(client_idx, (const uint8_t*)data.c_str(), data.length());
    }

    bool NetworkManager::writeToFileClient(size_t client_idx, const uint8_t* data, size_t len)
    {
        if (_mode != NetworkMode_t::AP || client_idx >= _clients.size())
        {
            return false;
        }

        WiFiClient& client = _clients[client_idx];
        if (!client.connected())
        {
            return false;
        }

        size_t written = client.write(data, len);
        _last_activity = millis();
        return written == len;
    }

    bool NetworkManager::connectToServer(const String& ip, uint32_t port)
    {
        if (_mode != NetworkMode_t::STA)
        {
            return false;
        }

        printf("Connect to server: %s:%d\n", ip.c_str(), port);

        if (_client == nullptr)
        {
            _client = new WiFiClient();
        }

        unsigned long start_time = millis();
        while (!_client->connect(ip.c_str(), port))
        {
            if (millis() - start_time > _config.connect_timeout)
            {
                printf("Connect timeout\n");
                if (_client)
                {
                    delete _client;
                    _client = nullptr;
                }
                _status = NetworkStatus_t::Failed;
                return false;
            }
            delay(100);
        }

        _client->setNoDelay(true);
        _server_ip = IPAddress((uint8_t)0);
        _mode = NetworkMode_t::Connected;
        _status = NetworkStatus_t::Connected;
        _last_activity = millis();

        printf("Connected to server\n");

        return true;
    }

    void NetworkManager::disconnectFromServer()
    {
        if (_client)
        {
            if (_client->connected())
            {
                _client->stop();
            }
            delete _client;
            _client = nullptr;
        }
        _server_ip = IPAddress(0, 0, 0, 0);
        _mode = NetworkMode_t::STA;
        _status = NetworkStatus_t::Idle;
    }

    String NetworkManager::readFromServer()
    {
        if (_mode != NetworkMode_t::Connected || _client == nullptr || !_client->connected())
        {
            return "";
        }

        String data = "";
        while (_client->available())
        {
            data += (char)_client->read();
        }
        if (data.length() > 0)
        {
            _last_activity = millis();
        }
        return data;
    }

    bool NetworkManager::writeToServer(const String& data)
    {
        return writeToServer((const uint8_t*)data.c_str(), data.length());
    }

    bool NetworkManager::writeToServer(const uint8_t* data, size_t len)
    {
        if (_mode != NetworkMode_t::Connected || _client == nullptr || !_client->connected())
        {
            return false;
        }

        size_t written = _client->write(data, len);
        _last_activity = millis();
        return written == len;
    }

    bool NetworkManager::hasMessage()
    {
        return !_message_queue.empty();
    }

    String NetworkManager::popMessage()
    {
        if (_message_queue.empty())
        {
            return "";
        }
        String msg = _message_queue.front();
        _message_queue.pop();
        return msg;
    }

    void NetworkManager::pushMessage(const String& msg)
    {
        _message_queue.push(msg);
    }

    String NetworkManager::getLocalIP() const
    {
        if (_mode == NetworkMode_t::AP)
        {
            return WiFi.softAPIP().toString();
        }
        else if (_mode == NetworkMode_t::STA || _mode == NetworkMode_t::Connected)
        {
            return WiFi.localIP().toString();
        }
        return "0.0.0.0";
    }

    String NetworkManager::getAPIP() const
    {
        if (_mode == NetworkMode_t::AP)
        {
            return WiFi.softAPIP().toString();
        }
        return "0.0.0.0";
    }

    int NetworkManager::getSignalStrength() const
    {
        if (_mode == NetworkMode_t::STA || _mode == NetworkMode_t::Connected)
        {
            return WiFi.RSSI();
        }
        return 0;
    }

    void NetworkManager::update()
    {
        // Handle AP mode
        if (_mode == NetworkMode_t::AP)
        {
            // Accept new connections
            if (_server)
            {
                WiFiClient new_client = _server->available();
                if (new_client)
                {
                    if (_clients.size() < _config.max_clients)
                    {
                        new_client.setNoDelay(true);
                        _clients.push_back(new_client);
                        printf("New client connected, total: %d\n", (int)_clients.size());
                        _last_activity = millis();
                    }
                    else
                    {
                        printf("Server full, reject new client\n");
                    }
                }
            }

            // Check client connections
            for (size_t i = 0; i < _clients.size();)
            {
                if (!_clients[i].connected())
                {
                    printf("Client disconnected\n");
                    _clients.erase(_clients.begin() + i);
                }
                else
                {
                    i++;
                }
            }

            // Check timeout
            if (millis() - _last_activity > _config.keep_alive)
            {
                _status = NetworkStatus_t::Disconnected;
            }
        }

        // Handle STA mode
        if (_mode == NetworkMode_t::STA)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                _status = NetworkStatus_t::Connected;
                printf("STA connected, IP: %s\n", getLocalIP().c_str());
            }
            else if (WiFi.status() == WL_CONNECT_FAILED)
            {
                _status = NetworkStatus_t::Failed;
                printf("STA connect failed\n");
                stopSTA();
            }
        }

        // Handle Connected mode
        if (_mode == NetworkMode_t::Connected)
        {
            if (_client && !_client->connected())
            {
                printf("Server disconnected\n");
                disconnectFromServer();
            }

            // Check timeout
            if (millis() - _last_activity > _config.keep_alive)
            {
                // Keep alive
                if (_client && _client->connected())
                {
                    _client->write("\0", 1);
                }
                _last_activity = millis();
            }
        }
    }
} // namespace MULTIPLAY
