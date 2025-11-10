#pragma once
#include <chrono>
#include <ctime>

namespace TradingEngine {
    inline bool is_market_open_now() {
	return true;
        const int start_hour = 9;
        const int start_min = 30;
        const int end_hour = 16;
        const int end_min = 0;

        auto now_utc = std::chrono::system_clock::now();
        std::time_t t_utc = std::chrono::system_clock::to_time_t(now_utc);
        
        std::tm* tm_utc = std::gmtime(&t_utc);

        int hour_ny = tm_utc->tm_hour - 4; // Assuming EDT for summer/fall
        
        int wday_ny = tm_utc->tm_wday;
        if (hour_ny < 0) {
            hour_ny += 24;
            wday_ny = (wday_ny == 0) ? 6 : wday_ny - 1;
        }
        
        if (wday_ny == 0 || wday_ny == 6) {
            return false;
        }

        int current_time_in_minutes = hour_ny * 60 + tm_utc->tm_min;
        int start_time_in_minutes = start_hour * 60 + start_min;
        int end_time_in_minutes = end_hour * 60 + end_min;
        
        return current_time_in_minutes >= start_time_in_minutes && 
               current_time_in_minutes < end_time_in_minutes;
    }
}
