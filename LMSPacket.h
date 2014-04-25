
#include <string.h>
#include <cstdlib>


using namespace std;
class LMSPacket
{
  private:
    int intValue;
    std::string stringValue;
    bool boolValue;
    int responseCode;

  public:
    LMSPacket() {
        boolValue = 0;
        responseCode = 0;
        stringValue = "";
        intValue = 0;
    }
    ~LMSPacket() {
    }
    void setIntValue(int newInt){
    this->intValue = newInt;
    }
    int getIntValue(){
    return this->intValue;
    }
    void setStringValue(const char* newString){
        this->stringValue = newString;
    }
    const char* getStringValue() {
        return this->stringValue.c_str();
    }
    void setBoolValue(bool newBool) {
        this->boolValue= newBool;
    }
    bool getBoolValue(){
        return this->boolValue;
    }
    void setResponseCode(int responseCode){
        this->responseCode = responseCode;
    }
    int getResponseCode(){
        return this->responseCode;
    }
};
