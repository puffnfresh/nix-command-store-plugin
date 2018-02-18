#include "script.hh"

namespace nix {

ScriptMaster::ScriptMaster(const std::string & script, int logFD)
    : script(script)
    , logFD(logFD)
{
    if (script == "" || hasPrefix(script, "-"))
        throw Error("invalid script '%s'", script);
}

std::unique_ptr<ScriptMaster::Connection> ScriptMaster::startCommand()
{
    Path tmpDir = startMaster();

    Pipe in, out;
    in.create();
    out.create();

    auto conn = std::make_unique<Connection>();
    conn->scriptPid = startProcess([&]() {
        restoreSignals();

        close(in.writeSide.get());
        close(out.readSide.get());

        if (dup2(in.readSide.get(), STDIN_FILENO) == -1)
            throw SysError("duping over stdin");
        if (dup2(out.writeSide.get(), STDOUT_FILENO) == -1)
            throw SysError("duping over stdout");
        if (logFD != -1 && dup2(logFD, STDERR_FILENO) == -1)
            throw SysError("duping over stderr");

        Strings args = { script, "start", tmpDir };
        execvp(args.begin()->c_str(), stringsToCharPtrs(args).data());

        throw SysError("executing '%s'", script);
    });


    in.readSide = -1;
    out.writeSide = -1;

    conn->out = std::move(out.readSide);
    conn->in = std::move(in.writeSide);

    return conn;
}

Path ScriptMaster::startMaster()
{
    auto state(state_.lock());

    if (state->scriptMaster != -1) return *state->tmpDir;

    Path tmpDir = createTempDir("", "nix", true, true, 0700);
    state->tmpDir = std::make_unique<AutoDelete>(tmpDir);

    Pipe out;
    out.create();

    state->scriptMaster = startProcess([&]() {
        restoreSignals();

        close(out.readSide.get());

        if (dup2(out.writeSide.get(), STDOUT_FILENO) == -1)
            throw SysError("duping over stdout");

        Strings args = { script, "master", tmpDir };
        execvp(args.begin()->c_str(), stringsToCharPtrs(args).data());

        throw SysError("starting script master");
    });

    out.writeSide = -1;

    std::string reply;
    try {
        reply = readLine(out.readSide.get());
    } catch (EndOfFile & e) { }

    if (reply != "started")
        throw Error("failed to start script '%s'", script);

    return tmpDir;
}

}
