#include "Logging.h"

class AClass {
public:
	AClass(int a) {
		this->a = a;
	}
	int val() {
		return a;
	}
 private:
	int a;

};


class ASink : public tuya::LogSink {
public:
	  void OnLogMessage(const std::string& message) override {
		  printf("ONLOG ==>%s", message.c_str());
	  }

};
#include <unistd.h>

int main(void) {
	ASink sink;
	tuya::LogMessage::LogThreads(true);
	tuya::LogMessage::LogTimestamps(true);
	tuya::LogMessage::SetLogToStderr(false);
	tuya::LogMessage::LogToDebug(tuya::LoggingSeverity::LS_VERBOSE);
	tuya::LogMessage::AddLogToStream(&sink, tuya::LoggingSeverity::LS_VERBOSE);
	int val = 100;
	char* ptr = (char*)malloc(10);
	memset(ptr, 'c', 10);
	LOGE  << "AA" ;
	AClass *a  = new AClass(10);
	printf("ptr %p\n", ptr);
	LOGV  << "AA" << " BB " << 1000 << " cc " << val << " dd " << ptr << " ee " << a << " val " << a->val();
	std::string name = "黄敬佩";
	LOGV << "name = " << name;

	errno = EAGAIN;
	LOGS<< "system error!";

	return 0;
}
