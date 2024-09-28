#ifndef network_h
#define network_h

#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>

#define TIMEOUT 20 * 1000 * 1000 // 20s
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 16
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR 1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

extern WiFiUDP udpClient;

struct NetworkState {
  bool connected;
  String mac;
  String ip;
  String dns;
  String subnet;
  String gateway;
};

struct WifiState : NetworkState {
  float rssi;
};

struct EthState : NetworkState {
  bool fullDuplex;
  uint8_t linkSpeed;
};

class Network {
  public:
    static void setupTask();
    static void ethEventStatic(WiFiEvent_t event);

    void task();
    void ethEvent(WiFiEvent_t event);
    bool setupComplete;

    WifiState wifiState() const;
    EthState ethState() const;
    bool hasTime() const;
    String getFormattedTime() const;
    time_t getEpochTime() const;
    uint64_t getEpochMs() const;
    uint16_t getOffset() const;

    void reconfigureWifi();
  private:
    static constexpr const char* APssid = "PDU-SETUP";
    static constexpr const char* APpass = "123456789";

    bool wifiReconfigured = false;
    bool ap = false;
    bool wifi = false;
    bool eth = false;
    uint64_t lastConnected = 0;
    uint64_t nextWifi = 0;

    void setupWifi();
    void setupETH();
    void setupAP();
    void disconnected();
};

extern Network network;

#endif
