#ifndef AFINA_NETWORK_MT_BLOCKING_SERVER_H
#define AFINA_NETWORK_MT_BLOCKING_SERVER_H

#include <algorithm>
#include <atomic>
#include <thread>

#include <afina/network/Server.h>
#include <condition_variable>
#include <mutex>
#include <netdb.h>
#include <queue>
#include <vector>

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace MTblocking {

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl);
    ~ServerImpl();

    // See Server.h
    void Start(uint16_t port, uint32_t, uint32_t) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

    void _StartWorker(int client_socket, int index);

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void OnRun();

private:
    // Inner methods

    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;

    std::vector<std::thread> _waiting_workers;
    std::queue<int> _free_thread_index;
    std::mutex _list_mutex;
    uint32_t _count_of_workers = 0;

    // Server socket to accept connections on
    int _server_socket;

    // cond variable for wake up on join
    std::condition_variable _ready_join;

    // Thread to run network on
    std::thread _thread;
};

} // namespace MTblocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_BLOCKING_SERVER_H
