//
// Created by stephane bourque on 2021-08-26.
//
#pragma once

#include "framework/MicroService.h"
#include "StorageService.h"

namespace OpenWifi {

    class RESTAPI_managementRole_list_handler : public RESTAPIHandler {
    public:
        RESTAPI_managementRole_list_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            TransactionId,
            Internal),
          DB_(StorageService()->RolesDB()) {
        }
        static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/managementRole"}; };
    private:
        ManagementRoleDB    &DB_;
        void DoGet() final;
        void DoPost() final {};
        void DoPut() final {};
        void DoDelete() final {};
    };
}
