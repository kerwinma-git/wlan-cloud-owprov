//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "storage_policies.h"
#include "Utils.h"
#include "OpenWifiTypes.h"
#include "RESTAPI_utils.h"
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi {

    static  ORM::FieldVec    PolicyDB_Fields{
        // object info
        ORM::Field{"id",64, true},
        ORM::Field{"name",ORM::FieldType::FT_TEXT},
        ORM::Field{"description",ORM::FieldType::FT_TEXT},
        ORM::Field{"notes",ORM::FieldType::FT_TEXT},
        ORM::Field{"created",ORM::FieldType::FT_BIGINT},
        ORM::Field{"modified",ORM::FieldType::FT_BIGINT},
        ORM::Field{"entries",ORM::FieldType::FT_TEXT},
        ORM::Field{"inUse",ORM::FieldType::FT_TEXT}
    };

    static  ORM::IndexVec    PolicyDB_Indexes{
        { std::string("policy_name_index"),
          ORM::IndexEntryVec{
            {std::string("name"),
             ORM::Indextype::ASC} } }
    };

    PolicyDB::PolicyDB( ORM::DBType T, Poco::Data::SessionPool & P, Poco::Logger &L) :
        DB(T, "policies", PolicyDB_Fields, PolicyDB_Indexes, P, L, "pol") {}

}

template<> void ORM::DB<    OpenWifi::PolicyDBRecordType, OpenWifi::ProvObjects::ManagementPolicy>::Convert(OpenWifi::PolicyDBRecordType &In, OpenWifi::ProvObjects::ManagementPolicy &Out) {
    Out.info.id = In.get<0>();
    Out.info.name = In.get<1>();
    Out.info.description = In.get<2>();
    Out.info.notes = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::SecurityObjects::NoteInfo>(In.get<3>());
    Out.info.created = In.get<4>();
    Out.info.modified = In.get<5>();
    Out.entries = OpenWifi::RESTAPI_utils::to_object_array<OpenWifi::ProvObjects::ManagementPolicyEntry>(In.get<6>());
    OpenWifi::Types::from_string(In.get<7>(), Out.inUse);
}


template<> void ORM::DB<    OpenWifi::PolicyDBRecordType, OpenWifi::ProvObjects::ManagementPolicy>::Convert(OpenWifi::ProvObjects::ManagementPolicy &In, OpenWifi::PolicyDBRecordType &Out) {
    Out.set<0>(In.info.id);
    Out.set<1>(In.info.name);
    Out.set<2>(In.info.description);
    Out.set<3>(OpenWifi::RESTAPI_utils::to_string(In.info.notes));
    Out.set<4>(In.info.created);
    Out.set<5>(In.info.modified);
    Out.set<6>(OpenWifi::RESTAPI_utils::to_string(In.entries));
    Out.set<7>(OpenWifi::Types::to_string(In.inUse));
}
