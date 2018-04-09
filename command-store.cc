#include "store-api.hh"
#include "remote-store.hh"
#include "remote-fs-accessor.hh"
#include "archive.hh"
#include "worker-protocol.hh"
#include "pool.hh"
#include "command.hh"

namespace nix {

static std::string uriScheme = "command://";

class CommandStore : public RemoteStore
{
public:

    CommandStore(const std::string & command, const Params & params)
        : Store(params)
        , RemoteStore(params)
        , command(command)
        , master(command, connections->capacity() > 1)
    {
    }

    std::string getUri() override
    {
        return uriScheme + command;
    }

    void narFromPath(const Path & path, Sink & sink) override;

    ref<FSAccessor> getFSAccessor() override;

private:

    struct Connection : RemoteStore::Connection
    {
        std::unique_ptr<CommandMaster::Connection> commandConn;
    };

    ref<RemoteStore::Connection> openConnection() override;

    std::string command;

    CommandMaster master;

    void setOptions(RemoteStore::Connection & conn) override
    {
    };
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

void CommandStore::narFromPath(const Path & path, Sink & sink)
{
    auto conn(connections->get());
    conn->to << wopNarFromPath << path;
    conn->processStderr();
    ParseSink ps;
    auto fwd = ForwardSource(conn->from, sink);
    parseDump(ps, fwd);
}

ref<FSAccessor> CommandStore::getFSAccessor()
{
    return make_ref<RemoteFSAccessor>(ref<Store>(shared_from_this()));
}

ref<RemoteStore::Connection> CommandStore::openConnection()
{
    auto conn = make_ref<Connection>();
    conn->commandConn = master.startCommand();
    conn->to = FdSink(conn->commandConn->in.get());
    conn->from = FdSource(conn->commandConn->out.get());
    initConnection(*conn);
    return conn;
}

__attribute__((used))
static RegisterStoreImplementation regStore([](
    const std::string & uri, const Store::Params & params)
    -> std::shared_ptr<Store>
{
    if (std::string(uri, 0, uriScheme.size()) != uriScheme) return 0;
    return std::make_shared<CommandStore>(std::string(uri, uriScheme.size()), params);
});

}
