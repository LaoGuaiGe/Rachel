/**
 * @file network_manager.h
 * @author Forairaaaaa
 * @brief Network manager for multiplay
 * @version 0.1
 * @date 2024-01-01
 */
#pragma once

#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <queue>
#include <vector>
#include <cstdint>

namespace MULTIPLAY
{
    enum class NetworkMode_t : uint8_t
    {
        None,
        AP,
        STA,
        Connected
    };

    enum class NetworkStatus_t : uint8_t
    {
        Idle,
        Scanning,
        Connecting,
        Connected,
        Failed,
        Disconnected
    };

    struct NetworkConfig_t
    {
        const char* ap_ssid = "Rachel_001";
        const char* ap_password = "12345678";
        uint32_t server_port = 8888;
        uint32_t max_clients = 4;
        uint32_t connect_timeout = 10000;
        uint32_t keep_alive = 30000;
    };

    class NetworkManager
    {
    private:
        NetworkMode_t _mode = NetworkMode_t::None;
        NetworkStatus_t _status = NetworkStatus_t::Idle;
        NetworkConfig_t _config;

        WiFiServer* _server = nullptr;
        mutable std::vector<WiFiClient> _clients;
        
        WiFiClient* _client = nullptr;
        IPAddress _server_ip;

        DNSServer* _dns_server = nullptr;

        std::queue<String> _message_queue;
        uint32_t _last_activity = 0;

    public:
        NetworkManager();
        ~NetworkManager();

        void init(const NetworkConfig_t& config);
        void deinit();

        bool startAP();
        bool stopAP();
        bool startSTA();
        bool startSTA(const char* ssid, const char* password);
        bool stopSTA();
        
        bool isConnected() const;
        NetworkMode_t getMode() const;
        NetworkStatus_t getStatus() const;
        
        int getConnectedClientCount() const;
        void broadcast(const String& data);
        void broadcast(const uint8_t* data, size_t len);
        String readFromClient(size_t client_idx);
        bool writeToFileClient(size_t client_idx, const String& data);
        bool writeToFileClient(size_t client_idx, const uint8_t* data, size_t len);

        bool connectToServer(const String& ip, uint32_t port);
        void disconnectFromServer();
        String readFromServer();
        bool writeToServer(const String& data);
        bool writeToServer(const uint8_t* data, size_t len);

        bool hasMessage();
        String popMessage();
        void pushMessage(const String& msg);

        String getLocalIP() const;
        String getAPIP() const;
        int getSignalStrength() const;
        void update();
    };
} // namespace MULTIPLAY
