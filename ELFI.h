#ifndef ELFI_ELFI_H
#define ELFI_ELFI_H

#include "Cosa/Activity.hh"
#include "Cosa/Driver/NEXA.hh"
#include "Cosa/Event.hh"
#include "Cosa/INET/DNS.hh"
#include "Cosa/INET/HTTP.hh"
#include "Cosa/INET/NTP.hh"
#include "Cosa/RTC.hh"
#include "Cosa/String.hh"
#include "Cosa/Socket/Driver/W5100.hh"

// Web server settings =========================================================
#define CRLF "\r\n"                         // HTML end of line
#define WEBSERVER_PORT 80                   // Web server port
// -----------------------------------------------------------------------------

// NEXA settings ===============================================================
// Maximum number of NEXA Switches to use.
// Set as low as posiblie to save dynamic memory.
#define NEXA_SWITCHES 16

// Maximim number of NEXA Activities to use.
// Set as low as posiblie to save dynamic memory.
#define NEXA_ACTIVITIES 5
// -----------------------------------------------------------------------------

// NTP settings ================================================================
// Time-zone to use. Specify offset hour from GMT
// 1 : GMT+1, Stockholm
#define NTP_TIME_ZONE 1

// NTP server tu use
#define NTP_SERVER "se.pool.ntp.org"
// -----------------------------------------------------------------------------

// Weekday alarm settings ======================================================
// Common days of the week to dispatch a alarm on. The alarm will be dispatched
// on all true days and not on all false days.
// The week begins on Monday.
static const bool ALLDAYS[7] __PROGMEM = {true, true, true, true, true, true, true};
static const bool BEFOREWORKDAY[7] __PROGMEM = { true, true, true, true, false, false, true };
static const bool WEEKDAYS[7] __PROGMEM = { true, true, true, true, true, false, false };
static const bool WEEKENDDAYS[7] __PROGMEM = { false, false, false, false, false, true, true };
// -----------------------------------------------------------------------------

class ELFI
{
  public:
    /**
     * Default constructor.
     */
    ELFI() :
      m_transmitter(NULL),
      m_ethernet(NULL),
      m_webserverflag(false),
      m_webserver(this)
    { initialize(); };
    
    /**
     * Start ElFi based on the settings provided.
     * @param[in] transmitter to use
     * @param[in] ethernet socket to use (optional)
     * @param[in] webserverflag true if HTTP acces to ElFi should be used; default false (optional)
     */
    bool begin(NEXA::Transmitter * transmitter, W5100 * ethernet = NULL, bool webserverflag = false);
    
    /**
     * ElFi loop function
     */
    int run();
    
    /**
     * Activates a NEXA switch. Returns true if successfull
     * activataion otherwise false. ElFi can control up to NEXA_SWITCHES
     * number of NEXA switches. If id >= NEXA_SWITCHES the activation will
     * fail and return false. Furthermore, the NEXA switch may not be activated
     * before and atempt to activate a previously activated switch will result in
     * failure and return false; a previously activated switch hase to be
     * deactivated prior to a new activation can take place.
     * @param[in] id number for NEXA switch (0-based)
     * @param[in] str NEXA switch name
     * @param[in] dimable flag (default is false)
     */
    bool activate_NEXA_Switch(uint8_t id, String str, bool dimable = false);
    
    /**
     * Activates a NEXA activity. Returns true if successfull
     * activataion otherwise false. ElFi can control up to NEXA_ACTIVITIES
     * number of NEXA activities. If id >= NEXA_ACTIVITIES the activation will
     * fail and return false. Furthermore, the NEXA activity may not be activated
     * before and atempt to activate a previously activated activity will result in
     * failure and return false; a previously activated activity hase to be
     * deactivated prior to a new activation can take place.
     * @param[in] id number for NEXA activity (0-based)
     * @param[in] str NEXA activity name
     * @param[in] d days to dispatch activity on
     * @param[in] h hour to dispatch activity on
     * @param[in] m minute to dispatch activity on
     * @param[in] mode to switch to at dispatch
     * @param[in] sid ID of NEXA Switch to be switched; if no value is provided, all NEXA Switches are switched to given mode
     */
    bool activate_NEXA_Activity(uint8_t id, String str, const bool (&d)[7], uint8_t h, uint8_t m, uint8_t mode, uint8_t sid = NEXA_SWITCHES);
    
    /**
     * Switch the power switch to given mode.
     * @section Reference
     * 1. See NEXA::Transmitter::send()
     * @param[in] id for the NEXA Switch
     * @param[in] mode to switch to: 0 for OFF, 1 for ON and (-15 ...-1) for dim level
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     */
    void switch_to(uint8_t id, int8_t mode);
    
    /**
     * Switch the power switch on.
     * @section Reference
     * 1. See NEXA::Transmitter::send()
     * @param[in] id for the NEXA Switch
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     */
    void switch_on(uint8_t id);
    
    /**
     * Switch the power switch off.
     * @section Reference
     * 1. See NEXA::Transmitter::send()
     * @param[in] id for the NEXA Switch
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     */
    void switch_off(uint8_t id);
    
    /**
     * Dim the power switch. Return zero if successful command otherwise
     * negative error code; -1 if not dimable, -2 if dim level is out of scope.
     * @section Reference
     * 1. See NEXA::Transmitter::send()
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     * @param[in] id for the NEXA Switch
     * @param[in] dim level (-15 ...-1)
     * @return zero or negative error code
     */
    int switch_dim(uint8_t id, int8_t dim);
    
    /**
     * Switch all the power switches on.
     * 1. See NEXA::Transmitter::bradcast()
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     */
    void switch_on();

    /**
     * Switch all the power switches off.
     * 1. See NEXA::Transmitter::bradcast()
     * https://github.com/mikaelpatel/Cosa/blob/master/cores/cosa/Cosa/Driver/NEXA.hh
     */
    void switch_off();
  
  private:
    /**
     * The NEXA activity transmitts a given command at a given time at specified days
     * of the week.
     * @section Acknowledgements
     * The class is based on the Cosa Activity example related to the Cosa
     * library by Mikael Patel. See references for more details.
     * @section References
     * 1. CosaActivity.ino example file for implementing the Cosa Activity class.
     * https://github.com/mikaelpatel/Cosa/tree/master/examples/Time/CosaActivity
     */
    class NEXAActivity : public Activity
    {
      friend class ELFI;
      
      public:      
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
        
      protected:
        NEXAActivity() :
          m_parent(NULL),
          m_id(NEXA_ACTIVITIES),
          m_switch(NEXA_SWITCHES),
          m_activated(false),
          Activity()
        {
          set_run_period(1);
        };
      
      private:
        ELFI *    m_parent;     //<! NEXA activity parent.
        uint8_t   m_id;         //<! NEXA activity id number.
        String    m_name;       //<! NEXA activity name.
        bool      m_days[7];    //<! Days to dispatch activity.
        uint8_t   m_hours;      //<! Hour to dispatch activity.
        uint8_t   m_minutes;    //<! Minute to dispatch activity.
        uint8_t   m_mode;       //<! Mode to switch to on dispath.
        uint8_t   m_switch;     //<! NEXA Switch to switch mode for on dispatch
        bool      m_activated;  //<! If activated.
    };
    
    /**
     * Subclass of HTTP::Server, a server request handler class. Implements project
     * specific on_request() function to produce response to HTTP requests.
     *
     * @section Acknowledgements
     * The WebServer header is basically a copy of the Cosa Pin Web Server example
     * related to the Cosa library by Mikael Patel. See references for more details.
     *
     * @section References
     * 1. CosaPinWebServer.ino example file.
     * https://github.com/mikaelpatel/Cosa/tree/master/examples/Ethernet/CosaPinWebServer
     */
    class WebServer : public HTTP::Server
    {
      public:
        /**
         * Default constructor.
         * @param[in] parent object
         */
        WebServer(ELFI * parent) :
          m_parent(parent)
        {};
    
        /**
         * Override of the HTTP::Server:on_request() member function. Displays
         * the ElFi web application controll and communicats with the rest of the
         * controll system.
         * @param[in] page iostream for response.
         * @param[in] method http request method string.
         * @param[in] path resource path string.
         * @param[in] query possible query string.
         */
        virtual void on_request(IOStream& page, char* method, char* path, char* query);
        
        /**
         * Handel queries. Parse the query char and reacts.
         * @param[in] query possible query string.
         */
        void handle_query(char* query);
        
        ELFI * m_parent;  //<! Parnet object.
    };
    
    /**
     * Change from default initialized values to correct values for all
     * object members.
     */
    void initialize();
    
    /**
     * Update the Real Time Clock on the Arduino. Also updates the time in all the
     * alarms and activities.
     */
    void update_RTC();
    
    /**
     * Get the current time from a NTP. Returns the clock is successful
     * otherwise 0L.
     * @section Acknowledgements
     * The function is based on the Cosa NTP example related to the Cosa
     * library by Mikael Patel. See references for more details.
     * @section References
     * 1. CosaNTP.ino example file.
     * https://github.com/mikaelpatel/Cosa/tree/master/examples/Time/CosaNTP
     * @return clock or 0L if error
     */
    clock_t get_NTP_time();
  
    NEXA::Transmitter * m_transmitter;
    W5100 *             m_ethernet;
    bool                m_webserverflag;
    WebServer           m_webserver;
    Alarm::Scheduler    m_scheduler;

    // NEXA switches
    uint8_t             m_switch_id[NEXA_SWITCHES];           //<! NEXA switch id number.
    String              m_switch_name[NEXA_SWITCHES];         //<! NEXA switch name.
    bool                m_switch_dimable[NEXA_SWITCHES];      //<! NEXA switch is dimable or not
    bool                m_switch_activated[NEXA_SWITCHES];    //<! NEXA switch is used or not, i.e. if it has been activated or not
    
    // NEXA activities
    NEXAActivity        m_activities[NEXA_ACTIVITIES];        //<! NEXA activities.
};

#endif
