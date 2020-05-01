// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_
#include <iostream>
#include <string>
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
#define TIME_OUT 10000
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
using sock = boost::asio::ip::tcp::socket;
using endpoint = boost::asio::ip::tcp::endpoint;
using acceptor = boost::asio::ip::tcp::acceptor;
using io_context = boost::asio::io_context;
using input_ip = boost::asio::ip::address;
io_context context;
std::mutex mutex;
struct talk_to_client;
std::vector<std::shared_ptr<talk_to_client>> clients;
struct talk_to_client {
private:
    std::string answ;
    sock sock_;
    std::string username_;
    boost::posix_time::ptime last_ping;
    bool client_list_chaned = true;

public:
    talk_to_client() : sock_(context) {}
    sock &my_socket() { return sock_; }
    std::string &username() { return username_; }
    bool &list_chaned() {return client_list_chaned; }

    void write(const std::string &msg) {
        boost::asio::streambuf buffer{};
        std::ostream out(&buffer);
        out << msg;
        boost::asio::write(my_socket(), buffer);
    }

    void read_answer() {
        boost::asio::streambuf buffer{};
        boost::asio::read_until(my_socket(), buffer, '\n');

        std::string answer(std::istreambuf_iterator<char>{&buffer},
                           std::istreambuf_iterator<char>{}); //(c) MoraPresence
        answ = answer;
    }

    void login(std::string msg) {
        msg = msg.substr(5, msg.size());
        std::istringstream in(msg);
        in >> username_;
        write(username_ + LOGIN_OK);
        BOOST_LOG_TRIVIAL(trace) << CONNECTED << username_  << std::endl;
    }

    void ping() {
        last_ping = boost::posix_time::microsec_clock::local_time();
        if (client_list_chaned == true) {
            write(PING_OK);
        } else {
            write(LIST_CHANED);
            client_list_chaned = true;
        }
    }

    void client_list() {
        std::string msg;
        msg = "For " + username_ + " list: ";
        for (auto &client : clients) {
            msg += client->username() + " ";
        }
        msg += "\n";
        write(msg);
    }

    void process_request() {
        if (answ.find("login") == 0) login(answ);
        if (answ.find("ping") == 0) ping();
        if (answ.find("clients") == 0) client_list();
    }

    bool timed_out() const {
        boost::posix_time::ptime now = 
        boost::posix_time::microsec_clock::local_time();
        long long ms = (now - last_ping).total_milliseconds();
        return ms > TIME_OUT;
        }

    void stop() {
        boost::system::error_code err;
        sock_.close(err);
    }
};

void accept_thread() {
    endpoint ep(input_ip::from_string(STR), PORT_NUM);
    acceptor acc(context, ep);
    std::cout << START << std::endl;
    while (true) {
        auto client = std::make_shared<talk_to_client>();
        acc.accept(client->my_socket());
        client->my_socket().non_blocking(true);
        mutex.lock();
        for (auto &elem : clients) {
            elem->list_chaned() = false;
        }
        clients.push_back(client);
        mutex.unlock();
    }
}

void handle_clients_thread() {
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
    while (true) {
        sleep(SECONDS);
        mutex.lock();
        for (auto client = clients.begin(); client != clients.end();) {
            try {
                (*client)->read_answer();
                (*client)->process_request();
            } catch (boost::system::system_error &e) {
                if (!strcmp(type_exeption, e.what())) {
                    if ((*client)->timed_out()) {
                        (*client)->stop();
                        std::cout << (*client)->username() << " " << e.what() << std::endl;
                        ++client;
                        continue;
                    }
                    ++client;
                    continue;
                }
                (*client)->stop();
                std::cout << (*client)->username() <<
                    " " << e.what() << std::endl;
                for (auto &elem : clients) {
                    elem->list_chaned() = false;
                }
                clients.erase(client);
                continue;
            }
            ++client;
        }
        mutex.unlock();
    }
}

void init() {
    boost::log::register_simple_formatter_factory
            <boost::log::trivial::severity_level, char>(ATTR_NAME);
    boost::log::add_file_log
            (
                    boost::log::keywords::file_name = PWD ,
                    boost::log::keywords::rotation_size = SIZE_FILE ,
                    boost::log::keywords::time_based_rotation =
                        boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
                    boost::log::keywords::format =
                            "[%TimeStamp%] [%Severity%] %Message%");
    boost::log::add_console_log
            (
                    std::cout,
                    boost::log::keywords::format =
                            "[%TimeStamp%] [%Severity%] %Message%");
    boost::log::add_common_attributes();
}
#endif // INCLUDE_HEADER_HPP_
