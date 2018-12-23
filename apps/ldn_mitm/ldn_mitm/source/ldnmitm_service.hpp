#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <cstring>
#include "ldn_icommunication.hpp"
#include "debug.hpp"

enum LdnSrvCmd {
    LdnSrvCmd_CreateUserLocalCommunicationService = 0,
};
enum LdnMitmDebugCmd {
    LdnMitmDebugCmd_SaveLogToFile = 65000,
    LdnMitmDebugCmd_GetVersion = 65001,
};
struct LdnMitmVersion {
    char raw[32];
};

class LdnMitMService : public IMitmServiceObject {      
    public:
        LdnMitMService(std::shared_ptr<Service> s, u64 pid) : IMitmServiceObject(s, pid) {
            /* ... */
        }
        
        static bool ShouldMitm(u64 pid, u64 tid) {
            LogFormat("should_mitm pid: %" PRIu64 " tid: %" PRIx64, pid, tid);
            return true;
        }
        
        static void PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx) {};
    protected:
        /* Overridden commands. */
        Result CreateUserLocalCommunicationService(Out<std::shared_ptr<ICommunicationInterface>> out);
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<LdnSrvCmd_CreateUserLocalCommunicationService, &LdnMitMService::CreateUserLocalCommunicationService>(),
        };
};

class LdnMitmDebugService final : public IServiceObject {
    protected:
        Result SaveLogToFile() {
            bool rc = ::SaveLogToFile();
            return rc ? 0 : 0x1234;
        }
        Result GetVersion(Out<LdnMitmVersion> version) {
            std::strcpy(version.GetPointer()->raw, GITDESCVER);
            return 0;
        }
    public:
        DEFINE_SERVICE_DISPATCH_TABLE {
            MakeServiceCommandMeta<LdnMitmDebugCmd_SaveLogToFile, &LdnMitmDebugService::SaveLogToFile>(),
            MakeServiceCommandMeta<LdnMitmDebugCmd_GetVersion, &LdnMitmDebugService::GetVersion>(),
        };
};
