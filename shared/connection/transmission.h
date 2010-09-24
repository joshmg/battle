// File: transmission.h
// Written by Joshua Green

#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include <string>
#include "connection.h"

const int TRANSMISSION_SIZE = 1024;     // TRANSMISSION_SIZE should be a factor of connection's BUFF_SIZE

class transmission {
  private:
    std::string _data;
    std::string _server_code;
    mutable std::string _formatted; // cashed resized value
    mutable bool _format_valid;
    
    void _format() const;
  public:
    transmission();
    transmission(const std::string&);

    void set_server_code(const std::string&);

    void clear();
    void set(const std::string&);
    
    std::string value(bool unpadded=false) const;
    
    transmission& add(const std::string&);
    transmission& add(const transmission&);
    
    transmission operator+(const std::string&) const;
    transmission operator+(const transmission&) const;
    transmission& operator+=(const std::string&);
    transmission& operator+=(const transmission&);
};

#endif
