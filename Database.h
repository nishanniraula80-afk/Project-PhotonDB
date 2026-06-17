#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_map>
#include <string>

class Database {
private:
    std::unordered_map<std::string, std::string> data;

public:
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    void del(const std::string& key);

    std::unordered_map<std::string, std::string>& getAllData();
};

#endif