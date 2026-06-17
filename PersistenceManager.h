#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

#include "database.h"
#include <string>

class PersistenceManager {
private:
    std::string snapshotFile;
    std::string aofFile;

public:
    PersistenceManager(const std::string& snapshotFile,
                       const std::string& aofFile);

    // Snapshot
    void saveSnapshot(Database& db);
    void loadSnapshot(Database& db);

    // AOF (Append Only File)
    void appendLog(const std::string& command);
    void replayLog(Database& db);

    // Full recovery
    void recover(Database& db);
};

#endif