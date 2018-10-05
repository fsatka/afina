#ifndef AFINA_NETWORK_MT_BLOCKING_SERVER_H
#define AFINA_NETWORK_MT_BLOCKING_SERVER_H


#include <atomic>
#include <thread>
#include <algorithm>

#include <afina/network/Server.h>
#include <netdb.h>
#include <queue>
#include <mutex>
#include <condition_variable>



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

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void OnRun();

private:
    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bounds
    std::atomic<bool> running;
    std::queue<std::thread*> _waiting_workers;

    std::mutex _queue_mutex;
    std::mutex _is_storage;
    std::atomic<bool> _complete_storage;
    uint32_t _count_of_workers = 0;
    std::condition_variable _shutdown_worker; 
    // Server socket to accept connections on
    int _server_socket;

    // Thread to run network on
    std::thread _thread;

    //Inner methods
    void _start_worker(std::thread* current_worker, int client_socket, Afina::Storage* pStorage);
};

} // namespace MTblocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_BLOCKING_SERVER_H
