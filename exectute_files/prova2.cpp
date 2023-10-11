#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <tbb/tbb.h>
#include <thread>
#include <chrono>
#include <fstream>


// Define a global variable to store the desired number of threads
int desiredThreads = tbb::global_control::active_value(tbb::global_control::max_allowed_parallelism);
// Define a flag to signal the monitoring thread to exit
std::atomic<bool> monitorExitFlag(false);

// Function to calculate CPU usage as a percentage
double getCPULoad() {
    static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

    // Delay to calculate CPU usage over time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Read the current CPU statistics from /proc/stat
    std::ifstream statFile("/proc/stat");
    std::string line;
    std::getline(statFile, line);

    // Parse the CPU usage values
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;
    sscanf(line.c_str(), "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow, &totalSys, &totalIdle);

    // Calculate the CPU usage percentage
    unsigned long long total = totalUser - lastTotalUser + totalUserLow - lastTotalUserLow +
                               totalSys - lastTotalSys;
    unsigned long long totalIdleTime = totalIdle - lastTotalIdle;
    double cpuUsage = 100.0 * (1.0 - (totalIdleTime * 1.0 / total));

    // Update the last values for the next calculation
    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return cpuUsage;
}

// Implement your own logic to calculate the desired number of threads
int calculateDesiredThreads() {
    // Example: Adjust the number of threads based on CPU load
    // Here, we'll use a simple threshold (e.g., 70% CPU usage)
    double oldUse;
    oldUse= cpuUsage!= NULL ? cpuUsage, 100;
    double cpuUsage = getCPULoad(); // Implement this function to get CPU usage
    double delta= oldUse-cpuUsage;
    //se delta > 1 togli 3 thred e vai di 3 in 3 ad ogni delta maggiore di x
    std::cout << cpuUsage << std::endl;

    if (cpuUsage < 95) {
        // If CPU usage is high, reduce the number of threads
        return desiredThreads - 1;
    } else {
        // If CPU usage is low, increase the number of threads
        return desiredThreads + 1;
    }
}




void executeMOCO(const std::string& programPath, const std::string& jaspar, const std::string& genome, const std::string& bedFile) {
    // Construct the command arguments for ./MOCO
    std::vector<std::string> arguments = {
        programPath,
        "-b", bedFile,
        "-t", genome,
        "-j", jaspar
    };

    // Join the arguments into a space-separated command string
    std::string command;
    for (const std::string& arg : arguments) {
        command += arg + " ";
    }

    // Execute the command as a child process
    if (system(command.c_str()) == -1) {
        perror("system"); // Handle execution error if needed
    }
}

int main() {
    // Define the path to your program and common arguments
    std::string programPath = "./MOCO"; // Change to the actual path of your program
    std::string jaspar = "/home/loig/MocoLoco/Jaspar_2020/MA0809.2.jaspar"; // Common argument for all instances
    std::string genome = "/home/loig/MocoLoco/Genomes/hg38/hg38.2bit";

    // Define different sets of .bed file arguments for parallel execution
    std::vector<std::string> tf2s = {
        "JDP2", "MAFG", "ZNF23", "ZSCAN5A", "GRHL1", "MAFK", "IRF3", "ZNF675", "ZNF768",
  "MYOG", "MEF2C", "ZKSCAN8", "ZNF660", "ATF3", "GATA2", "CTCFL", "ZNF708", "ZNF425",
  "ZNF586", "ZIC5", "ZFP91", "TFAP4", "HHEX", "OSR2", "RREB1", "ZBTB14", "ZFP57",
  "ZNF440", "ZNF37A", "HNF1A", "BATF", "ZNF778", "ZNF506", "SREBF2", "ZSCAN22", "ZNF213",
  "KLF12", "KLF13", "MEF2B", "PAX8", "NFE2L2", "ZNF580", "BHLHE22", "ZNF257", "ZNF93",
  "ZNF274", "ZNF436", "KLF10", "KLF17", "SP1", "IRF9", "TBP", "MLX", "KLF5", "ADNP",
  "SP3", "ZNF692", "MYBL2", "SNAI2", "ZNF823", "ZNF674", "AR", "PPARG", "MAF", "SATB1",
  "ZNF264", "ESR2", "MNX1", "NFE2", "REST", "SP2", "ZNF12", "RUNX2", "GMEB2", "BCL11B",
  "ESR1", "E2F3", "IRF4", "NRF1", "ZNF524", "ONECUT2", "ZNF667", "ZNF223", "PLAG1", "ZNF664",
  "ZSCAN29", "TFAP2C", "ZNF343", "ZNF677", "POU4F2", "KLF3", "KLF7", "ZBTB33", "ZNF573",
  "ZNF350", "ZNF281", "EGR1", "ZNF518A", "ZNF891", "ZNF765", "ZNF770", "ZNF652", "ZNF596",
  "ZNF662", "GLIS2", "ZNF462", "ZNF701", "ZNF740", "HOXA9", "SCRT1", "ZNF148", "MEF2A",
  "FEZF1", "BACH2", "ZFP42", "BATF3", "PROX1", "SPI1", "ZNF467", "SOX10", "ATF7", "ZNF548",
  "BNC2", "ZNF597", "E2F1", "GATA4", "SREBF1", "MEF2D", "ZNF329", "ZNF783", "ZSCAN31", "LHX2",
  "TCFL5", "ZSCAN16", "ZNF750", "OVOL1", "IRF8", "ZNF333", "ZNF444", "ZNF543", "ZNF141",
  "HMBOX1", "FOS", "DLX6", "MYF5", "ZBTB6", "ZNF322", "ZNF549", "KLF14", "FOSL2", "SP4",
  "ZBED2", "ATF2", "ZNF554", "ZBTB7B", "ZNF182", "OLIG2", "KLF9", "CTCF", "HNF1B", "IRF5",
  "GRHL2", "ZNF19", "ZNF571", "SRF", "ZBTB21", "ZNF623", "ZNF101", "ZNF384", "ZNF490", "ZNF565",
  "KLF4", "ZNF574", "ZNF429", "TFAP2A", "ZNF10", "TCF21", "PATZ1", "KLF15", "RORB", "RARA",
  "ZNF79", "PAX5", "FOSB", "MYB", "EBF1", "THRB", "MSX2", "ZNF273", "KLF1"
    };

    std::vector<std::string> bedFiles;
    for (std::string tf2 : tf2s) {
        std::string bedFile = "/home/loig/MocoLoco/remap_hg_bed/TEAD4/TEAD4_" + tf2 + ".bed";
        bedFiles.push_back(bedFile);
    }

    // Create a thread-safe queue to hold file indices
    tbb::concurrent_bounded_queue<size_t> fileIndices;

    // Enqueue all the file indices you want to process
    for (size_t i = 0; i < bedFiles.size(); ++i) {
        fileIndices.push(i);
    }

    // Create a monitoring thread
    auto monitorAndAdjustThreads = [&]() {
        while (!monitorExitFlag.load(std::memory_order_acquire)) {
            // Calculate the desired number of threads
            int newThreads = calculateDesiredThreads();
            if (newThreads != desiredThreads) {
                desiredThreads = newThreads;
                std::cout << desiredThreads << std::endl;
                tbb::global_control max_allowed_parallelism_control(tbb::global_control::max_allowed_parallelism, desiredThreads);
                
            }

            // Sleep for a while before the next monitoring cycle
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    };
    std::thread monitorThread(monitorAndAdjustThreads);
    // Custom observer for monitoring and adjusting threads
    

    // Parallel task execution
    tbb::parallel_for(0, desiredThreads, [&](int threadId) {
        size_t fileIndex;
        while (fileIndices.try_pop(fileIndex)) {
            executeMOCO(programPath, jaspar, genome, bedFiles[fileIndex]);
        }
    });

    // Signal the monitor thread to exit
    monitorExitFlag.store(true, std::memory_order_release);
    //monitorObserver.cancel_wait();

    // Wait for the monitor thread to finish
    monitorThread.join();
    return 0;
}