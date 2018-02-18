#pragma once

#include "util.hh"
#include "sync.hh"

namespace nix {

class ScriptMaster
{
private:

    const std::string script;
    const int logFD;

    struct State
    {
        Pid scriptMaster;
        std::unique_ptr<AutoDelete> tmpDir;
    };

    Sync<State> state_;

public:

    ScriptMaster(const std::string & script, int logFD = -1);

    struct Connection
    {
        Pid scriptPid;
        AutoCloseFD out, in;
    };

    std::unique_ptr<Connection> startCommand();

    Path startMaster();
};

}
