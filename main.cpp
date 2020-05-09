#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "gsoap\new\add\CustomBinding_USCOREResourceFactoryInterface.nsmap"
#include "gsoap\new\add\soapH.h"

// #include "gsoap\add\CustomBinding_USCOREResourceFactoryInterface.nsmap"
// #include "gsoap\add\soapH.h"

int main(int argc, char* argv[])
{
	int result = Catch::Session().run(argc, argv);

	struct soap soap_;
	memset(soap_.buf, 0, sizeof(soap_.buf));
	_WSRT__Create *create = soap_new_req__WSRT__Create(&soap_);

	struct wsa5__EndpointReferenceType response;
	int res = soap_call___WSRT2__Create(&soap_, "http://ADX0000CC12E0:16000/WSRT-TF", NULL, create, response);

	getchar();
	return (result < 0xff ? result : 0xff);
}
