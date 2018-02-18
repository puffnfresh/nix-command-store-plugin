#include "store-api.hh"
#include "remote-store.hh"
#include "remote-fs-accessor.hh"
#include "archive.hh"
#include "worker-protocol.hh"
#include "pool.hh"
#include "script.hh"

namespace nix {

static std::string uriScheme = "script://";

class ScriptStore : public RemoteStore
{
public:

    ScriptStore(const std::string & script, const Params & params)
        : Store(params)
        , RemoteStore(params)
        , script(script)
        , master(script)
    {
    }

    std::string getUri() override
    {
        return uriScheme + script;
    }

    void narFromPath(const Path & path, Sink & sink) override;

    ref<FSAccessor> getFSAccessor() override;

private:

    struct Connection : RemoteStore::Connection
    {
        std::unique_ptr<ScriptMaster::Connection> scriptConn;
    };

    ref<RemoteStore::Connection> openConnection() override;

    std::string script;

    ScriptMaster master;
};


class ForwardSource : public Source
{
    Source & readSource;
    Sink & writeSink;
public:
    ForwardSource(Source & readSource, Sink & writeSink) : readSource(readSource), writeSink(writeSink) {}
    size_t read(unsigned char * data, size_t len) override
    {
        auto n = readSource.read(data, len);
        writeSink(data, n);
        return n;
    }
};

void ScriptStore::narFromPath(const Path & path, Sink & sink)
{
    auto conn(connections->get());
    conn->to << wopNarFromPath << path;
    conn->processStderr();
    ParseSink ps;
    auto fwd = ForwardSource(conn->from, sink);
    parseDump(ps, fwd);
}

ref<FSAccessor> ScriptStore::getFSAccessor()
{
    return make_ref<RemoteFSAccessor>(ref<Store>(shared_from_this()));
}

ref<RemoteStore::Connection> ScriptStore::openConnection()
{
    auto conn = make_ref<Connection>();
    conn->scriptConn = master.startCommand();
    conn->to = FdSink(conn->scriptConn->in.get());
    conn->from = FdSource(conn->scriptConn->out.get());
    initConnection(*conn);
    return conn;
}

static RegisterStoreImplementation regStore([](
    const std::string & uri, const Store::Params & params)
    -> std::shared_ptr<Store>
{
    if (std::string(uri, 0, uriScheme.size()) != uriScheme) return 0;
    return std::make_shared<ScriptStore>(std::string(uri, uriScheme.size()), params);
});

}
