#include "command.hh"

namespace nix {

CommandMaster::CommandMaster(const std::string & command, bool useMaster, int logFD)
    : command(command)
    , useMaster(useMaster)
    , logFD(logFD)
{
    if (command == "" || hasPrefix(command, "-"))
        throw Error("invalid command '%s'", command);
}

std::unique_ptr<CommandMaster::Connection> CommandMaster::startCommand()
{
    Path tmpDir = startMaster();

    Pipe in, out;
    in.create();
    out.create();

    auto conn = std::make_unique<Connection>();
    conn->commandPid = startProcess([&]() {
        restoreSignals();

        close(in.writeSide.get());
        close(out.readSide.get());

        if (dup2(in.readSide.get(), STDIN_FILENO) == -1)
            throw SysError("duping over stdin");
        if (dup2(out.writeSide.get(), STDOUT_FILENO) == -1)
            throw SysError("duping over stdout");
        if (logFD != -1 && dup2(logFD, STDERR_FILENO) == -1)
            throw SysError("duping over stderr");

        Strings args = { command, "start", tmpDir };
        execvp(args.begin()->c_str(), stringsToCharPtrs(args).data());

        throw SysError("executing '%s'", command);
    });


    in.readSide = -1;
    out.writeSide = -1;

    conn->out = std::move(out.readSide);
    conn->in = std::move(in.writeSide);

    return conn;
}

Path CommandMaster::startMaster()
{
    if (!useMaster) return "";

    auto state(state_.lock());

    if (state->commandMaster != -1) return *state->tmpDir;

    Path tmpDir = createTempDir("", "nix", true, true, 0700);
    state->tmpDir = std::make_unique<AutoDelete>(tmpDir);

    Pipe out;
    out.create();

    state->commandMaster = startProcess([&]() {
        restoreSignals();

        close(out.readSide.get());

        if (dup2(out.writeSide.get(), STDOUT_FILENO) == -1)
            throw SysError("duping over stdout");

        Strings args = { command, "master", tmpDir };
        execvp(args.begin()->c_str(), stringsToCharPtrs(args).data());

        throw SysError("starting command master");
    });

    out.writeSide = -1;

    std::string reply;
    try {
        reply = readLine(out.readSide.get());
    } catch (EndOfFile & e) { }

    if (reply != "started")
        throw Error("failed to start command '%s'", command);

    return tmpDir;
}

}
