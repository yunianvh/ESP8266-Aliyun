# ESP8266-Aliyun-ESP8266+Arduino对接阿里云实现远程控制LED、一键配网和自动计算MQTT连接密码|玉念聿辉|吴明辉|奥捷迅科技
具体说明看下面文章：<br/>
阿里云Iot：https://dev.iot.aliyun.com/profile/502dal206acfbbd9b034d2d1d51b4a422e749656 <br/>
CSDN：https://blog.csdn.net/qq_35350654/article/details/96980688

### ESP8266一键配网：
<hr/>
（配网app：https://pan.baidu.com/s/1eSMpA2e?errno=0&errmsg=Auth%20Login%20Sucess&&bduss=&ssnerror=0&traceid=）

（app源码：https://github.com/Life1412378121/EsptouchDemo）

```
/**
 * 一键配网关键代码
 */
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  delay(2000);
  // 等待配网
  WiFi.beginSmartConfig();
 
 while (1)
  {
    Serial.print(".");
    delay(500);
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
```

### MQTT密码计算：
<hr/>
这里是cpp的代码，下面只是核心代码不能直接拿来使用；

```
void DFRobot_Aliyun :: init(String AliyunServer, String AliProductKey, 
                            String AliClientId, String AliDeviceName, 
                            String AliDeviceSecret, uint16_t AliPort)
                            {
    this->ALIYUN_SERVER = AliyunServer;
    this->ProductKey    = AliProductKey;
    this->ClientId      = AliClientId;
    this->DeviceName    = AliDeviceName;
    this->DeviceSecret  = AliDeviceSecret;
    this->Port          = AliPort;
    setConfig();
}

void DFRobot_Aliyun :: setConfig(){
    String tempSERVER = (this->ProductKey + "." + this->ALIYUN_SERVER);
    uint8_t len = tempSERVER.length();
    uint16_t timestamp = 49;
    if(this->mqtt_server == NULL){
        this->mqtt_server = (char *) malloc(len);
    }
    strcpy(this->mqtt_server,tempSERVER.c_str());
	
    String tempID = (this->ClientId + 
                   "|securemode=3"+
                   ",signmethod=" + "hmacsha1"+
                   ",timestamp="+(String)timestamp+"|");
    len = tempID.length();
	len +=1;
    if(this->client_id == NULL){
        this->client_id = (char *) malloc(len);
    }
    strcpy(this->client_id,tempID.c_str());

	
    String Data = ("clientId" + this->ClientId + 
                     "deviceName" + this->DeviceName + 
                     "productKey" + this->ProductKey + 
                     "timestamp" + (String)timestamp
                     );
    byte tempPassWord[20];
    char tempSecret[this->DeviceSecret.length()];
    char tempData[Data.length()];
    String tempName = (this->DeviceName + "&" + this->ProductKey);
    len = tempName.length();
    this->username = (char * )malloc(len);
    strcpy(this->username,tempName.c_str());
    
    strcpy(tempData,Data.c_str());
    strcpy(tempSecret,this->DeviceSecret.c_str());
    MyHmac_Sha1.HMAC_SHA1((byte * )tempData,Data.length(),(byte * )tempSecret,this->DeviceSecret.length(),tempPassWord);
    String tempPass = byteToHexStr(tempPassWord,sizeof(tempPassWord));
    if(this->password == NULL){
        this->password = (char *) malloc(tempPass.length());
    }
    strcpy(this->password,tempPass.c_str());
    Serial.print("userName=");
    Serial.println(this->username);
}
```

### 远程控制LED例子
<hr/>
代码实现：
阿里云创建产品和设备我就不用讲了，不会的看官方文档，网上也有很多教程。

```
#include <ESP8266WiFi.h>
/* 依赖 PubSubClient 2.4.0 */
#include <PubSubClient.h>
/* 依赖 ArduinoJson 5.13.4 */
#include <ArduinoJson.h>
#include <MD5_String.h>
#include "DFRobot_Aliyun.h"

#define SENSOR_PIN  13

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "";
const char * WIFI_PASSWORD = "";

/*配置设备证书信息*/
String ProductKey = "a1HA";
String ClientId = "12345";/*自定义ID*/
String DeviceName = "1vlGNJUNq";
String DeviceSecret = "T0PYh4A5DFHpV";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*
 *信息上报定义
 */
//#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
//#define ALINK_TOPIC_PROP_POST     "/sys/a158jzjhYxu/xB7cmv0tfFxhdoQOrvRS/thing/event/property/post"

/*需要操作的产品标识符*/
String Identifier = "smartLightSate";

/*需要上报和订阅的两个TOPIC*/
const char * subTopic = "you sub Topic";//****set
const char * pubTopic = "you pub Topic";//******post

unsigned long lastMs = 0;
DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient  client(espClient);

static void openLight(){
  digitalWrite(SENSOR_PIN, HIGH);
}

static void closeLight(){
  digitalWrite(SENSOR_PIN, LOW);
}

/**
 * 手动设置帐号密码给设备联网
 */
void connectWiFi(){
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Adderss: ");
  Serial.println(WiFi.localIP());
}

/**
 * 一键配网关键代码
 */
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  delay(2000);
  // 等待配网
  WiFi.beginSmartConfig();
 
 while (1)
  {
    Serial.print(".");
    delay(500);
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


/**
 * 连接阿里云物联网Iot
 */
void ConnectAliyun(){
  while(!client.connected()){
    Serial.println("Attempting MQTT connection...");
    /*根据自动计算的用户名和密码连接到Alinyun的设备，不需要更改*/
     Serial.println(myAliyun.client_id);
     Serial.println(myAliyun.username);
     Serial.println(myAliyun.password);
    if(client.connect(myAliyun.client_id,myAliyun.username,myAliyun.password)){
      Serial.println("connected");
      client.subscribe(subTopic);
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * 回调监听
 */
void callback(char *topic, byte *payload, unsigned int len)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
//    payload[length] = '\0';
//    Serial.println((char *)payload);

    for (int i = 0; i < len; i++){
      Serial.print((char)payload[i]);
    }
    Serial.println();
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char *)payload);
    if(!root.success()){
      Serial.println("parseObject() failed");
      return;
    }
    const uint16_t LightStatus = root["params"][Identifier];
    if(LightStatus == 1){
      openLight();
    }else{
      closeLight();
    }
    String tempMseg = "{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":"+(String)LightStatus+"},\"method\":\"thing.event.property.post\"}";
    char sendMseg[tempMseg.length()];
    strcpy(sendMseg,tempMseg.c_str());
    client.publish(pubTopic,sendMseg);
}
void setup()
{
  // Init the serial
  Serial.begin(115200);
  pinMode(SENSOR_PIN,OUTPUT);

 /*连接WIFI*/
  connectWiFi();
//  smartConfig()
  
  /*初始化Alinyun的配置，可自动计算用户名和密码*/
  myAliyun.init(ALIYUN_SERVER,ProductKey,ClientId,DeviceName,DeviceSecret);
  
  client.setServer(myAliyun.mqtt_server,PORT);
  
  /*设置回调函数，当收到订阅信息时会执行回调函数*/
  client.setCallback(callback);
  
  /*连接到Aliyun*/
  ConnectAliyun();

   /*开机先关灯*/
  closeLight();
  
  /*上报关灯信息*/
  client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":0},\"method\":\"thing.event.property.post\"}").c_str());
}

void loop()
{
    if(!client.connected()){
      ConnectAliyun();
    }
    client.loop();
}
```

<hr/>
作者：玉念聿辉
编辑：吴明辉
公司：奥捷迅科技

