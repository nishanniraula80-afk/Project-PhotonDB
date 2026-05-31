#include "Database.h"

void Database::set(const std::string& key, const std::string& value) {
    data[key] = value;
}

std::string Database::get(const std::string& key) {
    if (data.find(key) == data.end()) return "";
    return data[key];
}

void Database::del(const std::string& key) {
    data.erase(key);
}

std::unordered_map<std::string, std::string>& Database::getAllData() {
    return data;
}