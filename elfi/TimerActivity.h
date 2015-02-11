/*
 * @flie TimerActivity.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2015,      Alexander Sehlström
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Arduino ElFi project.
 */
 
#ifndef TIMER_ACTIVITY_H
#define TIMER_ACTIVITY_H

#include "Cosa/Activity.hh"
#include "Cosa/Event.hh"
#include "Cosa/RTC.hh"

/**
 * The device timer transmitts a given commad at a given time at specified days
 * of the week.
 *
 * @section References
 * 1. CosaActivity.ino example file for implementing the Cosa Activity class.
 * https://github.com/mikaelpatel/Cosa/tree/master/examples/Time/CosaActivity
 */
class DeviceTimer : public Activity {
  public:
    /**
     * Construct a device activity that transmitts a given command at a given time
     * each specified day.
     * time (h:m:s). The activity repeats each day.
     * @param[in] d[7] array with 0 or 1 values; 0 for not active days,
     *            1 for active days. Sunday is  day 1, Monday 2, ... and Saturday 7.
     * @param[in] h hour to start activity.
     * @param[in] m minute to start activity.
     * @param[in] s second to start activity.
     * @param[in] device NEXA device to communicate with (0...15) or, for
     *            broadcast to all devices, -1
     * @param[in] mode NEXA mode to send; 0 for off, 1 for on.
     */
    DeviceTimer(const uint8_t (&d)[7], const uint8_t h, const uint8_t m, const uint8_t s, const uint8_t device, const uint8_t mode) :
      m_day(d),
      m_hours(h),
      m_minutes(m),
      m_seconds(s),
      m_device(device),
      m_mode(mode),
      Activity()
    {
      set_run_period(1);
    };
    
    /**
     * Run is called when the activity is dispatched. Starts the function
     * related to the DailyActivity.
     */
    virtual void run();
    
    /**
     * Overrides the Activity::enable() function. Used to relate the
     * activity clock to the RTC clock.
     */
    void enable();
    
  private:
    const uint8_t (&m_day)[7];  //!< Which days should the alarm be activated on?
    const uint8_t m_hours;      //!< Start hour
    const uint8_t m_minutes;    //!< Start monute
    const uint8_t m_seconds;    //!< Start second
    const uint8_t m_device;     //!< Device
    const uint8_t m_mode;       //!< Mode
};

/**
 * The function timer runs a specified function at a given time at specified days
 * of the week.
 *
 * @section References
 * 1. CosaActivity.ino example file for implementing the Cosa Activity class.
 * https://github.com/mikaelpatel/Cosa/tree/master/examples/Time/CosaActivity
 */
class FunctionTimer : public Activity {
  typedef void (*function_type)();
  
  public:
    /**
     * Construct a function activity that runs a a function (*f)() at
     * a given time each specified day.
     * @param[in] d[7] array with 0 or 1 values; 0 for not active days,
     *            1 for active days. Sunday is day 1, Monday 2, ... and Saturday 7.
     * @param[in] h hour to start activity.
     * @param[in] m minute to start activity.
     * @param[in] s second to start activity.
     * @param[in] (*f)() function to run when activity is dispatched. 
     */
    FunctionTimer(const uint8_t (&d)[7], const uint8_t h, const uint8_t m, const uint8_t s, void (*f)()) :
      m_day(d),
      m_hours(h),
      m_minutes(m),
      m_seconds(s),
      m_function((*f)),
      Activity()
    {
      set_run_period(1);
    };
    
    /**
     * Run is called when the activity is dispatched. Starts the function
     * related to the FunctionTimer.
     */
    virtual void run();
    
    /**
     * Overrides the Activity::enable() function. Used to relate the
     * activity clock to the RTC clock.
     */
    void enable();
    
  private:
    const uint8_t (&m_day)[7];  //!< Which days should the alarm be activated on?
    const uint8_t m_hours;      //!< Start hour
    const uint8_t m_minutes;    //!< Start monute
    const uint8_t m_seconds;    //!< Start second
    function_type m_function;   //!< Function to run when activity is dispatched
};

#endif
