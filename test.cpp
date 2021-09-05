#include "Logging.h"
#include "string.h"

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


class ASink : public utils::LogSink {
public:
	  void OnLogMessage(const std::string& message) override {
		  printf("ONLOG ==>%s", message.c_str());
	  }

};
#include <unistd.h>

int main(void) {
	ASink sink;
//	utils::LogMessage::LogThreads(true);
//	utils::LogMessage::LogTimestamps(true);
	utils::LogMessage::SetLogToStderr(false);
//	utils::LogMessage::LogToDebug(utils::LoggingSeverity::LS_VERBOSE);
	utils::LogMessage::AddLogToStream(&sink, utils::LoggingSeverity::LS_VERBOSE);
	utils::LogMessage::ConfigureLogging("tstamp thread sensitive");
	int val = 100;
	char* ptr = (char*)malloc(10);
	memset(ptr, 'c', 10);
	LOGE  << "AA" ;
	AClass *a  = new AClass(10);
	printf("ptr %p\n", ptr);
	LOGV << "AA" << " BB " << 1000 << " cc " << val << " dd " << ptr << " ee " << a << " val " << a->val();
	std::string name = "黄敬佩";
	LOGV << "name = " << name;

	errno = EAGAIN;
	LOGS<< "system error!";

	return 0;
}
