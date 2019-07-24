#ifndef __DFROBOT_ALIYUN__
#define __DFROBOT_ALIYUN__

#include "Arduino.h"

class DFRobot_Aliyun{
  public:
    DFRobot_Aliyun(void);
    ~DFRobot_Aliyun(void);
    void init(String AliyunServer, String AliProductKey, 
              String AliClientId, String AliDeviceName, 
              String AliDeviceSecret, uint16_t AliPort = 1883);
    void setConfig();
    String ALIYUN_SERVER;
    String ProductKey;
    String ClientId;
    String DeviceName;
    String DeviceSecret;
    uint16_t Port;
    char * mqtt_server;
    char * client_id;
    char * username;
    char * password;
  private:
    
};

#endif
