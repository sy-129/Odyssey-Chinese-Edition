#pragma once
#include "GameData.hpp"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
class Engine {
 std::atomic<bool> running_{false};std::thread worker_;mutable std::mutex mutex_;
 ts::Frame frame_;std::string status_="尚未启动";
 double lastReport_=0;int diagnosticPID_=0;std::uint64_t diagnosticModule_=0;bool coordinatesVerified_=false;unsigned confirmedLocations_=0;
 void run();void status(std::string);void publish();
public:
 ~Engine(){stop();}
 void start();void stop();
 ts::Frame frame()const;std::string status()const;
};
