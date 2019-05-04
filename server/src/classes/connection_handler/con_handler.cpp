//
// Created by Augens on 03/05/2019.
//

#include "con_handler.h"


using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;

class con_handler: public boost::enable_shared_from_this<con_handler> {
private:
    tcp::socket sock;
    std::string message;
    enum { max_length = 1024 };
    char data[max_length];


public:
    typedef boost::shared_ptr<con_handler> pointer;
    con_handler(boost::asio::io_service& io_service): sock(io_service){}
// creating the pointer
    static pointer create(boost::asio::io_service& io_service)
    {
        return pointer(new con_handler(io_service));
    }
//socket creation
    tcp::socket& socket()
    {

        return sock;
    }

    void start() //gestione della connessione in entrata
    {
//      Handshake di inizio conversazione, aspetto allineamento client

//            sock.async_read_some(
//                    boost::asio::buffer(data, max_length),
//                    boost::bind(&con_handler::handle_read,
//                                shared_from_this(),
//                                boost::asio::placeholders::error,
//                                boost::asio::placeholders::bytes_transferred));



        // Set up a timed select call, so we can handle timeout cases.
        fd_set fileDescriptorSet;
        struct timeval timeStruct;

        // set the timeout to 30 seconds
        timeStruct.tv_sec = 10;
        timeStruct.tv_usec = 0;
        FD_ZERO(&fileDescriptorSet);

        // We'll need to get the underlying native socket for this select call, in order
        // to add a simple timeout on the read:

        int nativeSocket = sock.native_handle();
        FD_SET(nativeSocket,&fileDescriptorSet);

        select(nativeSocket+1,&fileDescriptorSet,NULL,NULL,&timeStruct);

        if(!FD_ISSET(nativeSocket,&fileDescriptorSet)){ // timeout reached
            cout << "Client muto\n";
        }

        sock.receive(buffer(data,max_length));

        for(int i=0,j=0;i<(int)max_length;i++){
            if (data[i-1] == '\n')
                j = 1;
            if (j == 1)
                data[i] = '\0';
        }

        cout << data;


        cout << "R: ";
        std::getline(std::cin,message);
        message += "\n\r";

        sock.async_write_some(
                boost::asio::buffer(message, message.size()),
                boost::bind(&con_handler::handle_write,
                            shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));


    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) {
            cout << data << endl;
        } else {
            std::cerr << "error: " << err.message() << std::endl;
            sock.close();
        }
    }
    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) {
            cout << "Message sendt"<< endl;
        } else {
            std::cerr << "error: " << err.message() << endl;
            sock.close();
        }
    }


//    Potrebbe essere utile per evitare attese
    static void configureSocketTimeouts(boost::asio::ip::tcp::socket& socket)
    {
#if defined OS_WINDOWS
        int32_t timeout = 15000;
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec  = 15;
        tv.tv_usec = 0;
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    }
};