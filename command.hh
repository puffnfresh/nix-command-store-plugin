#pragma once

#include "util.hh"
#include "sync.hh"

namespace nix {

class CommandMaster
{
private:

    const std::string command;
    const bool useMaster;
    const int logFD;

    struct State
    {
        Pid commandMaster;
        std::unique_ptr<AutoDelete> tmpDir;
    };

    Sync<State> state_;

public:

    CommandMaster(const std::string & command, bool useMaster, int logFD = -1);

    struct Connection
    {
        Pid commandPid;
        AutoCloseFD out, in;
    };

    std::unique_ptr<Connection> startCommand();

    Path startMaster();
};

}
