//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Util/Application.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/JSON/Parser.h"

#include "StorageService.h"
#include "Daemon.h"
#include "Utils.h"
#include "OpenAPIRequest.h"

namespace OpenWifi {

	class Storage *Storage::instance_ = nullptr;

    int Storage::Start() {
		SubMutexGuard		Guard(Mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = Daemon()->ConfigGetString("storage.type");

        if (DBType == "sqlite") {
            DBType_ = ORM::DBType::sqlite;
            Setup_SQLite();
        } else if (DBType == "postgresql") {
            DBType_ = ORM::DBType::postgresql;
            Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            DBType_ = ORM::DBType::mysql;
            Setup_MySQL();
        }

        EntityDB_ = std::make_unique<OpenWifi::EntityDB>(DBType_,*Pool_, Logger_);
        PolicyDB_ = std::make_unique<OpenWifi::PolicyDB>(DBType_, *Pool_, Logger_);
        VenueDB_ = std::make_unique<OpenWifi::VenueDB>(DBType_, *Pool_, Logger_);
        LocationDB_ = std::make_unique<OpenWifi::LocationDB>(DBType_, *Pool_, Logger_);
        ContactDB_ = std::make_unique<OpenWifi::ContactDB>(DBType_, *Pool_, Logger_);
        InventoryDB_ = std::make_unique<OpenWifi::InventoryDB>(DBType_, *Pool_, Logger_);
        RolesDB_ = std::make_unique<OpenWifi::ManagementRoleDB>(DBType_, *Pool_, Logger_);
        ConfigurationDB_ = std::make_unique<OpenWifi::ConfigurationDB>(DBType_, *Pool_, Logger_);

        EntityDB_->Create();
        PolicyDB_->Create();
        VenueDB_->Create();
        LocationDB_->Create();
        ContactDB_->Create();
        InventoryDB_->Create();
        RolesDB_->Create();
        ConfigurationDB_->Create();

        ExistFunc_[EntityDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return EntityDB_->Exists(F,V); };
        ExistFunc_[PolicyDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return PolicyDB_->Exists(F,V); };
        ExistFunc_[VenueDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return VenueDB_->Exists(F,V); };
        ExistFunc_[ContactDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return ContactDB_->Exists(F,V); };
        ExistFunc_[InventoryDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return InventoryDB_->Exists(F,V); };
        ExistFunc_[ConfigurationDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return ConfigurationDB_->Exists(F,V); };
        ExistFunc_[LocationDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return LocationDB_->Exists(F,V); };
        ExistFunc_[RolesDB_->Prefix()] = [=](const char *F, std::string &V) -> bool { return RolesDB_->Exists(F,V); };

        EntityDB_->CheckForRoot();
        Updater_.start(*this);

        return 0;
    }

    void Storage::run() {
	    Running_ = true ;
	    bool FirstRun=true;
	    long Retry = 2000;
	    while(Running_) {
	        if(!FirstRun)
	            Poco::Thread::trySleep(Retry);
	        if(!Running_)
	            break;
	        if(UpdateDeviceTypes()) {
	            FirstRun = false;
	            Logger_.information("Updated existing DeviceType list from FMS.");
	            Retry = 60 * 5 * 1000 ; // 5 minutes
	        } else {
	            Retry = 2000;
	        }
	    }
	}


    /*  Get the device types... /api/v1/firmwares?deviceSet=true
     {
          "deviceTypes": [
            "cig_wf160d",
            "cig_wf188",
            "cig_wf194c",
            "edgecore_eap101",
            "edgecore_eap102",
            "edgecore_ecs4100-12ph",
            "edgecore_ecw5211",
            "edgecore_ecw5410",
            "edgecore_oap100",
            "edgecore_spw2ac1200",
            "edgecore_ssw2ac2600",
            "indio_um-305ac",
            "linksys_e8450-ubi",
            "linksys_ea8300",
            "mikrotik_nand",
            "mikrotik_nand-large",
            "tplink_cpe210_v3",
            "tplink_cpe510_v3",
            "tplink_eap225_outdoor_v1",
            "tplink_ec420",
            "tplink_ex227",
            "tplink_ex228",
            "tplink_ex447",
            "wallys_dr40x9"
          ]
        }
     */
	bool Storage::UpdateDeviceTypes() {

	    try {
	        Types::StringPairVec QueryData;

	        QueryData.push_back(std::make_pair("deviceSet","true"));
	        OpenAPIRequestGet	Req(    uSERVICE_FIRMWARE,
                                     "/api/v1/firmwares",
                                     QueryData,
                                     5000);

	        Poco::JSON::Object::Ptr Response;
	        auto StatusCode = Req.Do(Response);
	        if( StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
	            if(Response->isArray("deviceTypes")) {
	                SubMutexGuard G(Mutex_);
	                DeviceTypes_.clear();
	                auto Array = Response->getArray("deviceTypes");
	                for(const auto &i:*Array) {
	                    DeviceTypes_.insert(i.toString());
	                }
	                return true;
	            }
	        } else {
	        }
	    } catch (const Poco::Exception &E) {
	        Logger_.log(E);
	    }
	    return false;
	}

    void Storage::Stop() {
	    Running_=false;
	    Updater_.wakeUp();
	    Updater_.join();
        Logger_.notice("Stopping.");
    }

    bool Storage::Validate(const Poco::URI::QueryParameters &P, std::string &Error) {
	    for(const auto &i:P) {
	        auto uuid_parts = Utils::Split(i.second,':');
	        if(uuid_parts.size()==2) {
	            auto F = ExistFunc_.find(uuid_parts[0]);
	            if(F!=ExistFunc_.end()) {
	                if(!F->second("id", uuid_parts[1])) {
	                    Error = "Unknown " + F->first + " UUID:" + uuid_parts[1] ;
	                    break;
	                }
	            }
	        }
	    }

	    if(Error.empty())
	        return true;
	    return false;

	}

    bool Storage::Validate(const Types::StringVec &P, std::string &Error) {
        for(const auto &i:P) {
            auto uuid_parts = Utils::Split(i,':');
            if(uuid_parts.size()==2) {
                auto F = ExistFunc_.find(uuid_parts[0]);
                if(F!=ExistFunc_.end()) {
                    if(!F->second("id", uuid_parts[1])) {
                        Error = "Unknown " + F->first + " UUID:" + uuid_parts[1] ;
                        break;
                    }
                }
            }
        }
        if(Error.empty())
            return true;
        return false;
	}

}

// namespace