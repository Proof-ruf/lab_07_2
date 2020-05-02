// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_
#include <iostream>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <vector>
#include <mutex>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <cstring>

#define SIZE_FILE 10 * 1024 * 1024
#define TIME_OUT 5000
#define PORT_NUM 8001
#define SECONDS 1

const char PWD[] = "/home/darioshka/Desktop/my_file%N.log";
const char ATTR_NAME[] = "Severity";
const char type_exeption[] = "read_until: Resource temporarily unavailable";
const char START[] = "server started...";
const char STR[] = "127.0.0.1";
const char CONNECTED[] = "client connected ";
const char PING_OK[] = "ping ok\n";
const char LIST_CHANED[] = "client list chaned\n";
const char LOGIN_OK[] = "login ok\n";
const char LOGIN[] = "login";
const char PING[] = "ping";
const char CLIENTS[] = "clients";
const char OUTPUT_FORMAT[] = "[%TimeStamp%] [%Severity%] %Message%";

using sock = boost::asio::ip::tcp::socket;
using endpoint = boost::asio::ip::tcp::endpoint;
using acceptor = boost::asio::ip::tcp::acceptor;
using io_context = boost::asio::io_context;
using input_ip = boost::asio::ip::address;
using int64_t;
io_context context;
std::mutex mutex;
struct talk_to_client{
private:
    std::string answ;
    sock sock_;
    std::string username_;
    boost::posix_time::ptime last_ping;
    bool client_list_chaned = true;

public:
    talk_to_client();
    sock &my_socket();
    std::string &username();
    bool &list_chaned();
    void write(const std::string &msg);
    void read_answer();
    void login(std::string msg);
    void ping();
    void client_list();
    void process_request();
    bool timed_out() const;
    void stop();
};
std::vector<std::shared_ptr<talk_to_client>> clients;

#endif // INCLUDE_HEADER_HPP_
