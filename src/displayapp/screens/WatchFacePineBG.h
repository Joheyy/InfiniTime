#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/datetime/DateTimeController.h"
#include "components/timer/TimerController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
  }

  namespace Applications {
    namespace Screens {

      class WatchFacePineBG : public Screen {
      public:
        WatchFacePineBG(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::TimerController& timerController);
        ~WatchFacePineBG() override;

        void Refresh() override;

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> powerPresent {};
        DirtyValue<bool> bleState {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> notificationState {};

        lv_obj_t* bg_obj;
        lv_obj_t* bg_clock_img;
        lv_obj_t* label_time_hrs;
        lv_obj_t* label_time_min;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date_weekday;
        lv_obj_t* label_date_day;
        lv_obj_t* label_date_month;
        lv_obj_t* label_timer;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* notificationIcon;

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::TimerController& timerController;

        lv_task_t* taskRefresh;
      };
    }
  }
}
