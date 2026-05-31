#include "PersistenceManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

PersistenceManager::PersistenceManager(const std::string& snapshotFile,
                                       const std::string& aofFile)
    : snapshotFile(snapshotFile), aofFile(aofFile) {}


// ---------------- SNAPSHOT SAVE ----------------
void PersistenceManager::saveSnapshot(Database& db) {
    std::ofstream file(snapshotFile);

    if (!file.is_open()) {
        std::cerr << "Failed to open snapshot file\n";
        return;
    }

    auto& data = db.getAllData();

    for (auto& pair : data) {
        file << pair.first << "=" << pair.second << "\n";
    }

    file.close();
}


// ---------------- SNAPSHOT LOAD ----------------
void PersistenceManager::loadSnapshot(Database& db) {
    std::ifstream file(snapshotFile);

    if (!file.is_open()) return;

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string key, value;

        if (std::getline(ss, key, '=') &&
            std::getline(ss, value)) {
            db.set(key, value);
        }
    }

    file.close();
}


// ---------------- AOF APPEND ----------------
void PersistenceManager::appendLog(const std::string& command) {
    std::ofstream file(aofFile, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Failed to open AOF file\n";
        return;
    }

    file << command << "\n";
    file.close();
}


// ---------------- REPLAY LOG ----------------
void PersistenceManager::replayLog(Database& db) {
    std::ifstream file(aofFile);

    if (!file.is_open()) return;

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string cmd, key, value;
        ss >> cmd >> key;

        if (cmd == "SET") {
            ss >> value;
            db.set(key, value);
        }
        else if (cmd == "DEL") {
            db.del(key);
        }
    }

    file.close();
}


// ---------------- FULL RECOVERY ----------------
void PersistenceManager::recover(Database& db) {
    loadSnapshot(db);
    replayLog(db);
}