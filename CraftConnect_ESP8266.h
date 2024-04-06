// CraftConnect_ESP8266.h
#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Arduino.h>
#include <map>
#include <AESLib.h>
#include "config.h"
/* изменить:
частота Serial
выделение памяти пакетов
*/


class ViewInfo {
public:
  ViewInfo(int type, String id);
  int _type;
  String _id;
};

template<typename T>
class CCcontainer{
public:
T& setHead(String text){
  _head = text;
  return static_cast<T&>(*this);
}
T& setHeadColor(int a, int r, int g, int b){
    _headColor[0] = a;
    _headColor[1] = r;
    _headColor[2] = g;
    _headColor[3] = b;
  return static_cast<T&>(*this);
} 
T& setHeadBackgroundColor(int a, int r,int g, int b){
    _headBackgroundColor[0] = a;
    _headBackgroundColor[1] = r;
    _headBackgroundColor[2] = g;
    _headBackgroundColor[3] = b;
  return static_cast<T&>(*this);
}
protected:
  String _head = "name";
  int _headColor[4] = { 255, 255, 255, 255 };
  int _headBackgroundColor[4] = { 255, 103, 58, 183 };
};

class CCStringWrite : public CCcontainer<CCStringWrite>{
  public:
    CCStringWrite(int number);
    CCStringWrite& setHint(String hint);
    CCStringWrite& setBackgruondColor(int a, int r, int g, int b);
    JsonObject _getJson();
    bool isUpdated();
    String getValue();
    void _setValue(String value);
  protected:
    int _number;
    String _hint = "hint";
    int _backgroundColor[4] = { 255, 207, 207, 207 };
    DynamicJsonDocument _json;
    bool _updated = false;
    String _value = "";
};

class CCStringView : public CCcontainer<CCStringView>{
public: 
  CCStringView& setText(String text); 
  CCStringView& setTextColor(int a, int r, int g, int b);
  CCStringView& setBackgroundColor(int a, int r, int g, int b);
  CCStringView(int number);
  JsonObject _getJson();
  //JsonArray _getUpdate();
protected:
  // void _addUpdate(int type, int array[]);
  // void _addUpdate(int type, String text);
  DynamicJsonDocument _json;
  int _number;
  String _text = "text";
  int _textColor[4] = { 255, 0, 0, 0 };
  int _textBackgroundColor[4] = { 255, 207, 207, 207 };
  //std::map<int, NodeJsonArray> _mapUpdate;
};

class CCScene {
public:
  CCScene(int scene, int update);
  //int[4] getColor();
  // void _init();
  JsonArray _getDataScene();
  void addStringView(String id);
  CCStringView& getStringView(String id);
  void addStringWrite(String id);
  CCStringWrite& getStringWrite(String id);
  std::map<int, ViewInfo> _mapNum;
  std::map<String, CCStringView> _mapStringView;
  std::map<String, CCStringWrite> _mapStringWrite;
  DynamicJsonDocument _jsonScene;
  DynamicJsonDocument _jsonUpdate;
  int _sceneSize;
  int _updateSize;
};

class CraftConnect {
public:
  CraftConnect(const char* ssid, const char* password, int serverPort, String passwordApp);
  void init(CCScene& scene);
  String getLocalIP();
  String getGlobalIP();
  void run();
  void initEncrypt(String key);
protected:
  bool _enc = false;
  AESLib _aes;
  byte _key[16];
  std::vector<WiFiClient> _clients;
  String _passwordApp;
  WiFiServer _server;
  WiFiClient _client;
  CCScene* _scene;
  const char* _ssid;
  const char* _password;
  int _serverPort;
  void _packageT1(DynamicJsonDocument json, WiFiClient& client);
  void _packageT2(WiFiClient& client, int t);
  // void _packageT3(WiFiClient& client);
  void _packageT4(DynamicJsonDocument json, WiFiClient& client);
  void _sendJson(DynamicJsonDocument json, WiFiClient& client);
};
