// File: connection.h
// Written by Joshua Green

#ifndef CONNECTION_H
#define CONNECTION_H

#pragma comment(lib, "../shared/connection/connection.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <string>

const int DEFAULT_PORT = 4444;                                    // default port
const int BUFF_SIZE = 1024;                                       // transmission buffer size
const int LISTEN_TIMEOUT = 60;                                    // wait time in seconds before a listen attempt times out...
                                                                  //   a value of -1 never times out

class transmission;

class p2p {
  private:
    SOCKET _listen;
    SOCKET _transmit;
    sockaddr_in _listen_addr;
    sockaddr_in _transmit_addr;
    int PORT;
    std::string _machine_ip;
    int _id;
    bool _is_linked;
    bool _is_receiving;
    bool _halt_receiving;
    bool _abort_listening;
    void (*_recv_function)(void*);              // pointer to a void function (having parameter void*) which specifies the function called by the recieve loop
                                                // the void* points to an instance of std::string
                                                //   WHICH MUST BE CASTED TO std::string AND DELETED BEFORE GOING OUT OF SCOPE
                                                //   recommended usage:
                                                //       string transmission_stream = (*(string*)ptr);
                                                //       delete (string*)ptr;
                                                // note: the function pointed to by this is executed within its own thread after data has been received

    void (*_disc_function)(void*);              // pointer to a void function which specifies the function called upon disconnection
                                                // the void* points to an int containing the value set by connection_id()
                                                //   WHICH MUST BE CASTED TO int AND DELETED BEFORE GOING OUT OF SCOPE
                                                //   recommended usage:
                                                //       int connection_id = (*(int*)ptr);
                                                //       delete (int*)ptr;
                                                // note: the function pointed to by this is executed within its own thread after data has been received

    void _initialize();
    static void _recv_branch(void*);
    static void _close_branch(void*);
    void _recv_loop();

  public:
    p2p();
    p2p(int);                                             // p2p(port)
    ~p2p();
    void set_port(int);                                   // sets the port used for all connections
                                                          // does not disconnect or reestablish connections

    void close(bool now=false);                           // close(bool=false)
                                                          // halts transmissions with the peer and closes the link with the peer
                                                          // if now is set to true, close will not wait for pending transmissions to clear

    void set_recv_func(void (*)(void*));                  // set_recv_func(receive_function_pointer)
                                                          // sets the function the receive process calls with the raw incoming data
                                                          // if no function is defined, "do_nothing" is called (which does nothing)
                                                          
    void set_disc_func(void (*)(void*));                  // set_disc_func(receive_function_pointer)
                                                          // sets the function the disconnect process
                                                          // if no function is defined, "do_nothing" is called (which does nothing)

    void enable_receive();                                // enables the receiving process
    void disable_receive();                               // temporarily disables receiving until the process is re-enabled

    bool link(const std::string& connect_ip="0.0.0.0");   // link(connect_ip)
                                                          // connect_ip is the string representation of the the requested ip
                                                          // link establishes a connection between a peer:
                                                          //   a connection request is first sent out, 
                                                          //   if request fails, connection enters a listen state and waits for a connection
                                                          //   if no connection was made, link() returns false

    void abort_link();                                    // aborts the link() process

    bool transmit(const std::string&);                    // transmit(data)
                                                          // transmits the data if a link has been established to peer
    bool transmit(const transmission&);

    std::string get_local_ip();                           // returns the ip address of the machine
    std::string get_remote_ip();                          // returns the ip address of the remotely connected machine
    bool is_linked();                                     // returns true if a connection has been established
    
    void id(int);                                         // gives the connection and ID which is passed to disc_function (as a void*) upon disconnect
    int id();                                             // returns the connection ID set by id(int)
                                                          // default value is 0
};

#endif
