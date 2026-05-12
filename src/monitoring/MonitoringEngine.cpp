#include "../../include/monitoring/MonitoringEngine.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace autotierx {

void MonitoringEngine::monitorStorage(
    const StorageManager& manager
) {

    std::cout << std::endl;
    std::cout
        << "===== STORAGE MONITORING ====="
        << std::endl;

    for (const auto& tier : manager.getTiers()) {

        try {

            fs::space_info info =
                fs::space(tier.getPath());

            long totalGB =
                info.capacity / (1024 * 1024 * 1024);

            long freeGB =
                info.available / (1024 * 1024 * 1024);

            long usedGB =
                totalGB - freeGB;

            std::cout << std::endl;

            std::cout
                << "Tier: "
                << tier.getName()
                << std::endl;

            std::cout
                << "Path: "
                << tier.getPath()
                << std::endl;

            std::cout
                << "Used Space: "
                << usedGB
                << " GB"
                << std::endl;

            std::cout
                << "Free Space: "
                << freeGB
                << " GB"
                << std::endl;

            std::cout
                << "Total Capacity: "
                << totalGB
                << " GB"
                << std::endl;

            /*
            ======================================
            ALERTS
            ======================================
            */

            if (freeGB < 5) {

                std::cout
                    << "[WARNING] LOW STORAGE SPACE"
                    << std::endl;
            }

        } catch (const std::exception& e) {

            std::cout
                << "Monitoring Error: "
                << e.what()
                << std::endl;
        }
    }
}
}