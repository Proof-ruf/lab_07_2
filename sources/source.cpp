// Copyright 2018 Your Name <your_email>

#include <header.hpp>
talk_to_client::talk_to_client() : sock_(context) {}

sock &talk_to_client::my_socket() { return sock_; }

std::string &talk_to_client::username() { return username_; }

bool &talk_to_client:: list_chaned() {return client_list_chaned; }

void talk_to_client::write(const std::string &msg) {
    boost::asio::streambuf buffer{};
    std::ostream out(&buffer);
    out << msg;
    boost::asio::write(my_socket(), buffer);
}

void talk_to_client::read_answer() {
    boost::asio::streambuf buffer{};
    boost::asio::read_until(my_socket(), buffer, '\n');

    std::string answer(std::istreambuf_iterator<char>{&buffer},
            std::istreambuf_iterator<char>{});
        answ = answer;
    }

void talk_to_client::login(std::string msg) {
    msg = msg.substr(5, msg.size());
    std::istringstream in(msg);
    in >> username_;
    write(username_ + LOGIN_OK);
    BOOST_LOG_TRIVIAL(trace) << CONNECTED << username_  << std::endl;
    }

void talk_to_client::ping() {
    last_ping =
    boost::posix_time::microsec_clock::local_time();
    if (client_list_chaned == true) {
        write(PING_OK);
    } else {
        write(LIST_CHANED);
        client_list_chaned = true;
    }
}

void talk_to_client::client_list() {
    std::string msg;
    msg = "List for " + username_ + ": ";
    for (auto &client : clients) {
        msg += client->username() + " ";
    }
    msg += "\n";
    write(msg);
}

void talk_to_client::process_request() {
    if (answ.find(LOGIN) == 0) login(answ);
    if (answ.find(PING) == 0) ping();
    if (answ.find(CLIENTS) == 0) client_list();
}

bool talk_to_client::timed_out() const{
    boost::posix_time::ptime now =
    boost::posix_time::microsec_clock::local_time();
    int64_t ms = (now - last_ping).total_milliseconds();
    return ms > TIME_OUT;
}

void talk_to_client::stop() {
    sock_.close();
}


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
                    if ((*client)->timed_out()){
                        (*client)->stop();
                        std::cout << (*client)->username()
                            << " " << e.what() << std::endl;
                        clients.erase(client);
                        continue;
                    }
                    ++client;
                    continue;
                }
                (*client)->stop();
                std::cout << (*client)->username()
                    << " " << e.what() << std::endl;
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
            boost::log::keywords::format = OUTPUT_FORMAT);
    boost::log::add_console_log
            (
                    std::cout,
                    boost::log::keywords::format = OUTPUT_FORMAT);
    boost::log::add_common_attributes();
}

int main() {
        init();
        std::thread th1(accept_thread);
        std::thread th2(handle_clients_thread);
        th1.join();
        th2.join();
    return 0;
}
