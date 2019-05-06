//
// Created by Klaus on 05/05/2019.
//

#ifndef CLIENT_KKCLIENT_H
#define CLIENT_KKCLIENT_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout;
using std::string;
using std::endl;

class KKClient: public boost::enable_shared_from_this<KKClient> {
private:
    enum { max_length = 1024 };
    /**
     * Memorizza i messaggi del server.
     */
    char _server_msg[max_length];

    /**
     * Riferimento al socket aperto.
     */
    tcp::socket _socket;

    /**
     * Memorizza l'errore in caso di write fallita.
     */
    error_code _send_error;

    /**
     * Costruttore privato.
     * @param io_service stream di input.
     */
    KKClient(boost::asio::io_service& io_service);

public:
    typedef boost::shared_ptr<KKClient> pointer;

    /// Ritorna l'istanza del client.
    /// \param io_service
    /// \return
    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new KKClient(io_service));
    }

    /// Invia al server il messaggio passato per paretro.
    /// \param msg
    /// \return
    bool send(string msg);

    /// Rimane in ascolto di messaggi provenienti dal server.
    void listen();

    void handle_accept(const boost::system::error_code& error);

    /// Gestore della lettura asincrona.
    /// @param err
    /// @param bytes_transferred
    void handle_read(const error_code& err, size_t bytes_transferred);

    /// Chiude la connessione.
    void close();
};


#endif //CLIENT_KKCLIENT_H
