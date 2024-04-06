// CraftConnect_ESP8266.cpp
#include "CraftConnect_ESP8266.h"
///////////////////////////////////////////////////////////////////////CraftConnect
CraftConnect::CraftConnect(const char* ssid, const char* password, int serverPort, String passwordApp) : _server(serverPort){ 
  Serial.begin(115200);
  Serial.println();
  _ssid = ssid;
  _password = password;
  _serverPort = serverPort;
  _passwordApp = passwordApp;
}
String CraftConnect::getLocalIP(){
  return WiFi.localIP().toString();
}
String CraftConnect::getGlobalIP(){
  HTTPClient http;
  WiFiClient httpWifi;
  http.begin(httpWifi,"http://api.ipify.org/");
  http.GET();
  String payload = http.getString();
  http.end();
  return payload;
}
void CraftConnect::init(CCScene& scene){
  _scene = &scene;
  WiFi.begin(_ssid, _password);
  Serial.print("Подключение");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println();
  _server.begin();
  Serial.print("Server local IP: ");
  Serial.println(WiFi.localIP());
  HTTPClient http;
  WiFiClient httpWifi;
  http.begin(httpWifi,"http://api.ipify.org/");
  http.GET();
  String payload = http.getString();
  http.end();
  Serial.print("Server global IP: ");
  Serial.println(payload);
  Serial.print("Port: ");
  Serial.println(_serverPort);
  // _scene->_init();

  // DynamicJsonDocument json(1024);
  // JsonObject jsonT1 jsonT1.to<JsonObject>();
  // jsonT1["status"]
}
void CraftConnect::run(){
  WiFiClient client = _server.accept();
  if (client) {
    _clients.push_back(client);
    Serial.println("new client");
  }
  for (int i = 0; i < _clients.size(); i++) {
    WiFiClient& client = _clients[i];
    if (client.connected()) {
      while (client.available()) {
        String request = client.readStringUntil('\n');
        request.trim();
        DynamicJsonDocument json(1024);
        DeserializationError error = deserializeJson(json, request);
        if (error) {
          Serial.print("Ошибка разбора JSON: ");
          Serial.println(error.c_str());
        } else {
          if(json["type"] == 1) _packageT1(json, client);
          else if (json["type"] == 2) _packageT2(client, 2);
          else if (json["type"] == 3) _packageT2(client, 3);
          else if (json["type"] == 4) _packageT4(json, client);
        }
      }
    } else {
      Serial.println("close");
      client.stop();
      _clients.erase(_clients.begin() + i);
      i--;
    }
  }
}

void CraftConnect::_packageT1(DynamicJsonDocument json, WiFiClient& client){
  String password = json["password"];
  DynamicJsonDocument returnJson(300);
  returnJson["type"] = 1;
  if (password == _passwordApp){
    returnJson["status"] = "success";
    _sendJson(returnJson, client);
  }
  else {
    returnJson["status"] = "failure";
    returnJson["debugMs"] = "invalid password";
    _sendJson(returnJson, client);
    Serial.println("invalid password");
    client.stop();
  }
}
void CraftConnect::_packageT2(WiFiClient& client, int t){
  DynamicJsonDocument returnJson(_scene->_sceneSize);
  returnJson["type"] = t;
  returnJson["simple"] = true;
  JsonArray dataScene = returnJson.createNestedArray("dataScene");
  JsonArray array = _scene->_getDataScene();
  for (auto element : array) {
    dataScene.add(element);
  }
  _sendJson(returnJson, client);
}

void CraftConnect::_packageT4(DynamicJsonDocument json, WiFiClient& client){
  int number = json["num"];
  String value = json["value"];
  auto it = _scene->_mapNum.find(number);
  ViewInfo* viewInfo = &it->second;
  int type = viewInfo->_type;
  String id = viewInfo->_id;
  if (type == TYPE_STRING_WRITE) {
    _scene->getStringWrite(id)._setValue(value);
  }
}
// void CraftConnect::_packageT3(WiFiClient& client){
//   DynamicJsonDocument newJson(_scene->_updateSize);
//   newJson["type"] = 3;
//   JsonArray updateArray = newJson.createNestedArray("update");
//   for (int i = 0;i < _scene->_mapNum.size();i++){
//     auto it = _scene->_mapNum.find(i);
//     ViewInfo* viewInfo = &it->second;
//     int type = viewInfo->_type;
//     String id = viewInfo->_id;
//     if (type == TYPE_STRING_VIEW){
//       JsonArray update = _scene->getStringView(id)._getUpdate();
//       for (int j = 0; j < update.size(); j++){
      
//         updateArray.add(update[j].as<JsonArray>(;))
//       }
//     }
//   }
//   _sendJson(newJson, client);
// }
void CraftConnect::initEncrypt(String key){
  key.toCharArray(reinterpret_cast<char*>(_key), 16);
  _enc = true;
}
void CraftConnect::_sendJson(DynamicJsonDocument json, WiFiClient& client){
  //char response[2000];
  String out;
  serializeJson(json, out);
  if (_enc) {
    // byte outB[16];
    // _aes.encrypt((byte*)out.c_str(), outB, _key);
    // out = String((char*)out);
  }
  //else serializeJson(json, response);
  client.println(out);
  // Serial.println("send");
  // Serial.println(response);
}
///////////////////////////////////////////////////////////////////////ViewInfo
ViewInfo::ViewInfo(int type, String id){
  _type = type;
  _id = id;
}
///////////////////////////////////////////////////////////////////////CCStringWrite

CCStringWrite::CCStringWrite(int number) : _json(STRING_VIEW_BYTE_DATA){
  _number = number;
}

void CCStringWrite::_setValue(String value){
  _value = value;
  _updated = true;
}

bool CCStringWrite::isUpdated(){
  return _updated;
}

String CCStringWrite::getValue(){
  _updated = false;
  return _value;
}

CCStringWrite& CCStringWrite::setHint(String hint){
  _hint = hint;
  return *this;
}
CCStringWrite& CCStringWrite::setBackgroundColor(int a, int r, int g, int b){
  _backgroundColor[0] = a;
  _backgroundColor[1] = r;
  _backgroundColor[2] = g;
  _backgroundColor[3] = b;
  return *this;
}
JsonObject CCStringWrite::_getJson(){
  JsonObject widget = _json.to<JsonObject>();
  widget["num"] = _number;
  widget[String(UPDATE_SET_HEAD)] = _head;
  JsonArray color = widget.createNestedArray(String(UPDATE_SET_HEAD_COLOR));
  for (int i = 0;i<4;i++) color.add(_headColor[i]);
  color = widget.createNestedArray(String(UPDATE_SET_HEAD_BACKGROUND_COLOR));
  for (int i = 0;i<4;i++) color.add(_headBackgroundColor[i]);
  widget["type"] = TYPE_STRING_WRITE;
  widget[String(UPDATE_SET_HINT)] = _hint;
  color = widget.createNestedArray(String(UPDATE_SET_BACKGROUND_COLOR));
  for (int i = 0;i<4;i++) color.add(_backgroundColor[i]);
  // String abc;
  // serializeJson(widget, abc);
  // Serial.println(abc);
  return widget;
}
///////////////////////////////////////////////////////////////////////CCStringView
CCStringView::CCStringView(int number) : _json(STRING_VIEW_BYTE_DATA){
  _number = number;
  
}

CCStringView& CCStringView::setText(String text){
  _text = text;
  // if (_update) _addUpdate(UPDATE_SET_TEXT, _text);
  return *this;
}

CCStringView& CCStringView::setTextColor(int a, int r, int g, int b){
  _textColor[0] = a;
  _textColor[1] = r;
  _textColor[2] = g;
  _textColor[3] = b;
  // if (_update) _addUpdate(UPDATE_SET_TEXT_COLOR, _textColor);
  return *this;
}
CCStringView& CCStringView::setBackgroundColor(int a, int r, int g, int b){
  _textBackgroundColor[0] = a;
  _textBackgroundColor[1] = r;
  _textBackgroundColor[2] = g;
  _textBackgroundColor[3] = b;
  // if (_update) _addUpdate(UPDATE_SET_TEXT_BACKGROUND_COLOR, _textBackgroundColor);
  return *this;
}
JsonObject CCStringView::_getJson(){
  JsonObject widget = _json.to<JsonObject>();
  widget["num"] = _number;
  widget[String(UPDATE_SET_HEAD)] = _head;
  JsonArray color = widget.createNestedArray(String(UPDATE_SET_HEAD_COLOR));
  for (int i = 0;i<4;i++) color.add(_headColor[i]);
  color = widget.createNestedArray(String(UPDATE_SET_HEAD_BACKGROUND_COLOR));
  for (int i = 0;i<4;i++) color.add(_headBackgroundColor[i]);
  widget["type"] = TYPE_STRING_VIEW;
  widget[String(UPDATE_SET_TEXT)] = _text;
  color = widget.createNestedArray(String(UPDATE_SET_TEXT_COLOR));
  for (int i = 0;i<4;i++) color.add(_textColor[i]);
  color = widget.createNestedArray(String(UPDATE_SET_TEXT_BACKGROUND_COLOR));
  for (int i = 0;i<4;i++) color.add(_textBackgroundColor[i]);
  // String abc;
  // serializeJson(widget, abc);
  // Serial.println(abc);
  return widget;
}


///////////////////////////////////////////////////////////////////////CCScene
CCScene::CCScene(int scene, int update) : _jsonScene(scene), _jsonUpdate(update){
  _sceneSize = scene;
  _updateSize = update;
  
}
// void CCScene::_init(){
//   for (int i = 0;i < _mapNum.size();i++){
//     auto it = _mapNum.find(i);
//     ViewInfo* viewInfo = &it->second;
//     int type = viewInfo->_type;
//     String id = viewInfo->_id;
//     if (type == TYPE_STRING_VIEW) getStringView(id)._update = true;
//   }
// }
void CCScene::addStringView(String id){
  int number = _mapNum.size();
  ViewInfo info(TYPE_STRING_VIEW,id);
  CCStringView view(number);
  _mapNum.emplace(number, info);
  _mapStringView.emplace(id,view);
}

CCStringView& CCScene::getStringView(String id){
  auto it = _mapStringView.find(id);
  return it->second;
}

void CCScene::addStringWrite(String id){
  int number = _mapNum.size();
  ViewInfo info(TYPE_STRING_WRITE,id);
  CCStringWrite write(number);
  _mapNum.emplace(number, info);
  _mapStringWrite.emplace(id,write);
}

CCStringWrite& CCScene::getStringWrite(String id){
  auto it = _mapStringWrite.find(id);
  return it->second;
}

JsonArray CCScene::_getDataScene(){
  DynamicJsonDocument newJson(_sceneSize);
  _jsonScene = newJson;
  JsonArray dataScene = _jsonScene.createNestedArray("dataScene");
  for (int i = 0;i < _mapNum.size();i++){
    auto it = _mapNum.find(i);
    ViewInfo* viewInfo = &it->second;
    int type = viewInfo->_type;
    String id = viewInfo->_id;
    if (type == TYPE_STRING_VIEW){
      JsonObject widget = getStringView(id)._getJson();
      dataScene.add(widget);
    }
    else if (type == TYPE_STRING_WRITE) {
      JsonObject widget = getStringWrite(id)._getJson();
      dataScene.add(widget);
    }
  }
  return _jsonScene["dataScene"].as<JsonArray>();
}

