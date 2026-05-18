#include "../../include/migration/BackgroundTieringDaemon.hpp"
#include "../../include/storage/StorageManager.hpp"

#include <thread>
#include <chrono>
#include <iostream>

namespace autotierx {

void BackgroundTieringDaemon::start(

    StorageManager& manager,

    ObjectManager& objectManager,

    AutoTieringEngine& autoTieringEngine
) {

    std::thread daemonThread(

        [&manager, &objectManager, &autoTieringEngine]() {

        while (true) {

            std::cout << std::endl;

            std::cout
                << "[BACKGROUND DAEMON RUNNING]"
                << std::endl;

            manager.refreshTierHealth();

            autoTieringEngine.evaluateAndMigrate(

                objectManager.getObjects()
            );

            /*
            =========================================
            WAIT 15 SECONDS
            =========================================
            */

            std::this_thread::sleep_for(

                std::chrono::seconds(15)
            );
        }
    });

    daemonThread.detach();
}

}
