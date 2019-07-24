#include "DFRobot_Aliyun.h"
#include "HMAC_SHA1.h"

CHMAC_SHA1 MyHmac_Sha1;
String byteToHexStr(unsigned char byte_arr[], int arr_len)  
{  
    String hexstr;  
    for (int i=0;i<arr_len;i++)  
    {  
        char hex1;  
        char hex2;  
        int value=byte_arr[i]; 
        int v1=value/16;  
        int v2=value % 16;  
  
        if (v1>=0&&v1<=9)  
            hex1=(char)(48+v1);  
        else  
            hex1=(char)(55+v1);  
  
        if (v2>=0&&v2<=9)  
            hex2=(char)(48+v2);  
        else  
            hex2=(char)(55+v2);  
  
        hexstr=hexstr+hex1+hex2;  
    }  
    return hexstr;  
} 

DFRobot_Aliyun :: DFRobot_Aliyun(void){
    
}

DFRobot_Aliyun :: ~DFRobot_Aliyun(void){
    
}

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
