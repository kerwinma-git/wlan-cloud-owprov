//
// Created by stephane bourque on 2021-08-12.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
		RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, uint64_t TransactionId, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{	Poco::Net::HTTPRequest::HTTP_GET,
												  	Poco::Net::HTTPRequest::HTTP_OPTIONS},
													Server, TransactionId, Internal,false) {}
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws"};}
		void DoGet() final;
		void DoDelete() final {};
		void DoPost() final {};
		void DoPut() final {};
	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done);
		std::string GoogleGeoCodeCall(const std::string &A);
		bool        GeoCodeEnabled_=false;
		std::string GoogleApiKey_;

	};
}
