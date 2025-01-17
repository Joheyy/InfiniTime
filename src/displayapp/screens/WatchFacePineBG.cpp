#include "displayapp/screens/WatchFacePineBG.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/Colors.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/settings/Settings.h"
using namespace Pinetime::Applications::Screens;

LV_IMG_DECLARE(bg_pines);

WatchFacePineBG::WatchFacePineBG(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Battery& batteryController,
                                   Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificatioManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::TimerController& timerController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    timerController {timerController} {
  settingsController.SetClockFace(3);

  bg_obj = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_height(bg_obj, LV_VER_RES);
  lv_obj_set_width(bg_obj, LV_HOR_RES);
  lv_obj_align(bg_obj, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(bg_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));

  bg_clock_img = lv_img_create(bg_obj, nullptr);
  lv_img_set_src(bg_clock_img, &bg_pines);
  lv_obj_align(bg_clock_img, nullptr, LV_ALIGN_CENTER, 0, 0);

  label_time_hrs = lv_label_create(lv_scr_act(), nullptr);
  label_time_min = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time_hrs, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &carlito_bold_120);
  lv_obj_set_style_local_text_font(label_time_min, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &carlito_bold_120);
  lv_label_set_text_fmt(label_time_hrs, "%02d", 0);
  lv_label_set_text_fmt(label_time_min, "%02d", 0);
  lv_obj_align(label_time_hrs, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);
  lv_obj_align(label_time_min, lv_scr_act(), LV_ALIGN_CENTER, 0, 50);

  batteryIcon.Create(lv_scr_act());
  batteryIcon.SetColor(LV_COLOR_WHITE);
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_CENTER, -85, -20);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_CENTER, -85, 20);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_MID, 0, 5);

  label_timer = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_timer, "00:00");
  lv_obj_align(label_timer, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
  lv_label_set_text(label_timer, "");

  label_date_weekday = lv_label_create(lv_scr_act(), nullptr);
  label_date_day = lv_label_create(lv_scr_act(), nullptr);
  label_date_month = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date_weekday, lv_scr_act(), LV_ALIGN_CENTER, 95, -20);
  lv_obj_align(label_date_day, lv_scr_act(), LV_ALIGN_CENTER, 95, 0);
  lv_obj_align(label_date_month, lv_scr_act(), LV_ALIGN_CENTER, 95, 20);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePineBG::~WatchFacePineBG() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFacePineBG::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(batteryIcon.GetObject());
  lv_obj_realign(batteryPlug);
  lv_obj_realign(bleIcon);

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  if (timerController.IsRunning()) {
    uint32_t seconds = timerController.GetTimeRemaining() / 1000;
    uint32_t minutesDisplay = seconds / 60;
    uint32_t secondsDisplay = seconds % 60;
    lv_label_set_text_fmt(label_timer, "%02d:%02d", minutesDisplay, secondsDisplay);
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = static_cast<int>(yearMonthDay.year());
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    uint8_t hour = time.hours().count();
    uint8_t minute = time.minutes().count();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[3] = "AM";
        if (hour == 0) {
          hour = 12;
        } else if (hour == 12) {
          ampmChar[0] = 'P';
        } else if (hour > 12) {
          hour = hour - 12;
          ampmChar[0] = 'P';
        }
        lv_label_set_text(label_time_ampm, ampmChar);
      }
      
      lv_label_set_text_fmt(label_time_hrs, "%02d", hour);
      lv_label_set_text_fmt(label_time_min, "%02d", minute);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(label_date_weekday, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(label_date_day, "%d", day);
      lv_label_set_text_fmt(label_date_month, "%s", dateTimeController.MonthShortToString());

      lv_obj_realign(label_date_weekday);
      lv_obj_realign(label_date_day);
      lv_obj_realign(label_date_month);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

}
