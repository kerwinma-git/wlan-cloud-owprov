//
// Created by stephane bourque on 2023-09-27.
//

#include "RESTAPI_radius_endpoint_handler.h"

namespace OpenWifi {
    static inline bool ValidEndpointTypes(const std::string &T) {
        return T=="radius" || T=="radsec";
    }

    static inline bool ValidPoolStrategy(const std::string &T) {
        return T=="none" || T=="tandom" || T=="weighted";
    }

    void RESTAPI_radius_endpoint_handler::DoGet() {
        auto id = GetBinding("id");
        if(id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
        }

        RecordType Record;
        if(DB_.GetRecord("id",id,Record)) {
            return ReturnObject(Record);
        }

        return NotFound();
    }

    void RESTAPI_radius_endpoint_handler::DoDelete() {
        auto id = GetBinding("id");
        if(id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
        }
        RecordType Record;
        if(DB_.GetRecord("id",id,Record)) {
            DB_.DeleteRecord("id",id);
            return OK();
        }
        return NotFound();
    }

    void RESTAPI_radius_endpoint_handler::DoPost() {
        auto id = GetBinding("id");
        if(id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
        }

        const auto &RawObject = ParsedBody_;
        RecordType     NewRecord;
        if(!NewRecord.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        if(!ValidEndpointTypes(NewRecord.Type)) {
            return BadRequest(RESTAPI::Errors::InvalidRadiusTypeEndpoint);
        }
        if(!ValidPoolStrategy(NewRecord.PoolStrategy)) {
            return BadRequest(RESTAPI::Errors::InvalidRadiusEndpointPoolStrategy);
        }
        if(!NewRecord.RadiusServers.empty() && !NewRecord.RadsecServers.empty()) {
            return BadRequest(RESTAPI::Errors::EndpointMustHaveOneTypeOfServers);
        }
        if(NewRecord.Index.empty()) {
            return BadRequest(RESTAPI::Errors::RadiusEndpointIndexInvalid);
        }

        ProvObjects::CreateObjectInfo(RawObject,UserInfo_.userinfo,NewRecord.info);
        if(DB_.CreateRecord(NewRecord)) {
            RecordType  AddedRecord;
            DB_.GetRecord("id", NewRecord.info.id, AddedRecord);
            return ReturnObject(AddedRecord);
        }
        return BadRequest(RESTAPI::Errors::RecordNotCreated);
    }

    void RESTAPI_radius_endpoint_handler::DoPut() {
        auto id = GetBinding("id");
        if(id.empty()) {
            return BadRequest(RESTAPI::Errors::MissingAuthenticationInformation);
        }

        const auto &RawObject = ParsedBody_;
        RecordType     ModifiedRecord;
        if(!ModifiedRecord.from_json(RawObject)) {
            return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
        }

        RecordType     Existing;
        if(!DB_.GetRecord("id",id,Existing)) {
            return NotFound();
        }

        ProvObjects::UpdateObjectInfo(RawObject, UserInfo_.userinfo, Existing.info);
        if(DB_.UpdateRecord("id", Existing.info.id, Existing)) {
            RecordType  AddedRecord;
            DB_.GetRecord("id", Existing.info.id, AddedRecord);
            return ReturnObject(AddedRecord);
        }

        return BadRequest(RESTAPI::Errors::NotImplemented);
    }

} // OpenWifi