#include "ELFI.h"

void
ELFI::initialize() {  
  // Initialize NEXA Switches
  {
    static const String str = "NEXA Switch";
    for (int id = 0; id < NEXA_SWITCHES; id++)
    {
      m_switch_id[id] = id;
      m_switch_name[id] = str;
      m_switch_dimable[id] = false;
      m_switch_activated[id] = false;
    }
  }
  
  // Initialize NEXA Activities
  {
    static const String str = "NEXA Activity";
    for (int id = 0; id < NEXA_ACTIVITIES; id++)
    {
      m_activities[id].m_parent = this;
      m_activities[id].m_id = id;
      m_activities[id].m_name = str;
    }
  }
}

bool
ELFI::begin(NEXA::Transmitter * transmitter, W5100 * ethernet, bool webserverflag)
{  
  m_transmitter = transmitter;
  m_ethernet = ethernet;
  m_webserverflag = webserverflag;
  
  if(m_ethernet != NULL)
  {
    // Initiate the ethernet controller using DHCP
    bool res;
    res = m_ethernet->begin_P(PSTR("ElFi"));
    if (!res) return (false);
    
    // Start the webserver if it shoudl be used.
    if(m_webserverflag)
    {
      res = m_webserver.begin(m_ethernet->socket(Socket::TCP, WEBSERVER_PORT));
      if (!res) return (false);
    }
    
    // Set the clock using a NTP
    time_t::epoch_year( NTP_EPOCH_YEAR );
    time_t::epoch_weekday = NTP_EPOCH_WEEKDAY;
    time_t::pivot_year = 37; // 1937..2036 range
    update_RTC();
  }
  
  if(NEXA_ACTIVITIES > 0)
  {
    m_scheduler.begin();
    
    for (int id = 0; id < NEXA_ACTIVITIES; id++)
    {
      if(m_activities[id].m_activated)
      {
        m_activities[id].enable();
      }
    }
  }
  
  return (true);
}

int
ELFI::run()
{
  // The standard event dispatcher
  Event event;
  while (Event::queue.dequeue( &event ))
    event.dispatch();
    
  // Service incoming requests
  int res = 0;
  if(m_webserverflag)
  {
    res = m_webserver.run(5L);
  }
  return res;
}

bool
ELFI::activate_NEXA_Switch(uint8_t id, String str, bool dimable)
{
  if (( id == 0 || id > 0 ) && (id < NEXA_SWITCHES))
  {
    if (!m_switch_activated[id])
    {
      m_switch_name[id] = str;
      m_switch_dimable[id] = dimable;
      m_switch_activated[id] = true;
      return (true);
    }
  }
  return (false);
}

bool
ELFI::activate_NEXA_Activity(uint8_t id, String str, const bool (&d)[7], uint8_t h, uint8_t m, uint8_t mode, uint8_t sid)
{
  if (( id == 0 || id > 0 ) && (id < NEXA_ACTIVITIES))
  {
    if (!m_activities[id].m_activated)
    {
      m_activities[id].m_name = str;
      for (int i = 0; i < 7; i++) { m_activities[id].m_days[i] = d[i]; }
      m_activities[id].m_hours = h;
      m_activities[id].m_minutes = m;
      m_activities[id].m_mode = mode;
      m_activities[id].m_switch = sid;
      m_activities[id].m_activated = true;
      return (true);
    }
  }
  return (false);
}

void
ELFI::switch_to(uint8_t id, int8_t mode)
{
  switch(mode)
  {
    case 0 :
      switch_off(id);
      break;
    case 1 :
      switch_on(id);
      break;
    default:
      switch_dim(id, mode);
      break;
  }
}

void
ELFI::switch_on(uint8_t id)
{
  m_transmitter->send(id, 1);
}

void
ELFI::switch_off(uint8_t id)
{
  m_transmitter->send(id, 0);
}

int
ELFI::switch_dim(uint8_t id, int8_t dim)
{ 
  if(m_switch_dimable[id]) {
    if((dim > -16) && (dim < 0)) {
      m_transmitter->send(id, dim);

      return (0);
    }
    return (-2);
  }
  return (-1);
}

void
ELFI::switch_on()
{
  m_transmitter->broadcast(0, 1);
  m_transmitter->broadcast(1, 1);
  m_transmitter->broadcast(2, 1);
  m_transmitter->broadcast(3, 1);
}

void
ELFI::switch_off()
{
  m_transmitter->broadcast(0, 0);
  m_transmitter->broadcast(1, 0);
  m_transmitter->broadcast(2, 0);
  m_transmitter->broadcast(3, 0);
}

void
ELFI::update_RTC()
{
  // Update the RTC
  RTC::time(get_NTP_time());

  // Match time in Alarm with RTC
  Alarm::set_time(RTC::time());
}

clock_t
ELFI::get_NTP_time()
{
  uint8_t server[4];

  // Use DNS to get the NTP server network address
  DNS dns;
  m_ethernet->get_dns_addr(server);
  if (!dns.begin(m_ethernet->socket(Socket::UDP), server)) return 0L;
  if (dns.gethostbyname_P(PSTR(NTP_SERVER), server) != 0) return 0L;

  // Connect to the NTP server using given socket
  NTP ntp(m_ethernet->socket(Socket::UDP), server, NTP_TIME_ZONE);

  // Get current time. Allow a number of retries
  const uint8_t RETRY_MAX = 20;
  clock_t clock;
  for (uint8_t retry = 0; retry < RETRY_MAX; retry++)
    if ((clock = ntp.time()) != 0L) break;
  if(clock == 0L) return 0L;

  return clock;
}

void
ELFI::NEXAActivity::enable()
{
  // Set time of the activity
  time_t start = RTC::time();
  start.hours = m_hours;
  start.minutes = m_minutes;
  start.seconds = 0;
  
  // Pass first time (in seconds after epoch time)
  // for the first occurence of the activity.
  // The activity will be repeated each 1440 s = 24 h.
  clock_t start_clock = start;
  set_time(start_clock, 1, 1440);
  
  // Enable the activity
  Activity::enable();
}

void
ELFI::NEXAActivity::run()
{  
  if (m_activated)
  {
    time_t time = RTC::time();
    uint8_t d = time.day;
    if (m_days[d])
    {
      if (m_switch != NEXA_SWITCHES)
      {
        m_parent->m_transmitter->send(m_switch, m_mode);
      }
      else
      {
        m_parent->m_transmitter->broadcast(0, m_mode);
        m_parent->m_transmitter->broadcast(1, m_mode);
        m_parent->m_transmitter->broadcast(2, m_mode);
        m_parent->m_transmitter->broadcast(3, m_mode);
      }
    }
  }
}

/**
 * The HTML page provided on request is Apple Web Application compatible. It
 * uses a simple jQuery script to pass background GET queries triggerd by the
 * interface buttons. The interface design is controlled via a header defined
 * style (CSS).
 *
 * @section Acknowledgements
 * Kudos to Mikael Patel for the the idea of putting large static peices of text
 * in the program memory; this approach reduced the usage of dynamic memory
 * substantionally. See references and WebServer.h for more details.
 *
 * @section References
 * 1. CosaPinWebServer.ino example file.
 * https://github.com/mikaelpatel/Cosa/tree/master/examples/Ethernet/CosaPinWebServer
 */
void 
ELFI::WebServer::on_request(IOStream& page, char* method, char* path, char* query)
{
  if (query != NULL)
  {
    handle_query(query);
  }
  else
  {
    static const char header[] __PROGMEM = 
      "HTTP/1.1 200 OK" CRLF
      "Content-Type: text/html" CRLF
      "Connection: close" CRLF CRLF
      "<!DOCTYPE HTML>" CRLF
      "<html>" CRLF
      "<head>" CRLF
      "<meta charset=\"UTF-8\">"
      "<meta name='apple-mobile-web-app-capable' content='yes' />" CRLF
      "<meta name='apple-mobile-web-app-status-bar-style' content='black' />" CRLF
      "<meta name='apple-mobile-web-app-title' content='Home Automation System' />" CRLF
      "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable = no'>" CRLF
      "<script src=\"//ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js\"></script>" CRLF
      "<script>" CRLF
      "  function deviceControll(url) {$.ajax(url);}" CRLF
      "</script>" CRLF
      "<script>" CRLF
      "  $(document).ready(function(){" CRLF
      "    $('.toggle').click(function(){" CRLF
      "      $('div div').toggle();" CRLF
      "      $('div div:first-child').show();" CRLF
      "    });" CRLF
      "  });" CRLF
      "</script>" CRLF
      "<style>" CRLF
      "body{margin:0; font-family:Helvetica,Arial,Sans-Serif; font-size:14px; background:#CCC; box-sizing:border-box;}" CRLF
      "*, *:before, *:after {box-sizing: inherit;}" CRLF
      "h1,h2,h3{display: block; padding:6px; margin:0;}" CRLF
      "h1{background:#67D66F; color: white;}" CRLF
      "h2, h3{padding-left:0px;}" CRLF
      "h3{font-size:15px}" CRLF
      ".button{display:table-cell; width:20%; height:inherit; vertical-align:middle; text-align:center;}" CRLF
      ".devices .device:first-child{background:#9CEF9F; color: white; margin-bottom:0;}" CRLF
      ".group {border-bottom:2px solid #67D66F}" CRLF
      ".group H2, .group H3{display:table-cell; width:60%; height:inherit; vertical-align:middle;}" CRLF
      ".group-header, .group-item{display:table; width:100%; height:20px; background:white; padding:6px; margin-bottom:1px}" CRLF
      ".group-header{background:#9CEF9F; color: white; margin-bottom:0;}"CRLF
      ".group-item{display:table; width:100%; height:20px; background:white; padding:6px; margin-bottom:1px}" CRLF
      ".group-item:last-child{margin-bottom:0px}" CRLF
      ".group-item span{vertical-align:middle; display:table-cell;}" CRLF
      ".group button{background:#67D66F; padding:5px 10px; margin: auto; border:hidden; -webkit-border-radius:3px; -moz-border-radius:3px; border-radius:3px; color:white; vertical-align:middle; min-width:95%;}" CRLF
      "div#time{color:gray; padding:8px; font-size:10px}" CRLF
      "</style>" CRLF
      "<title>ElFI - Home Automation System</title>" CRLF
      "</head>" CRLF 
      "<body>" CRLF;
      
    static const char body[] __PROGMEM = 
      "<h1>ElFi</h1>" CRLF;
      
    // Construct the NEXA Switches part of body
    static const char body_NEXASwitch0[] __PROGMEM =
      "<div class=\"group\">" CRLF
      "<div class=\"group-header\">" CRLF
      "<h2>NEXA Switches</h2>" CRLF
      "<div class=\"button\">" CRLF
      "<button onclick=\"deviceControll('http://10.0.1.190/?switch_all=1');\">All on</button>" CRLF
      "</div>" CRLF
      "<div class=\"button\">" CRLF
      "<button onclick=\"deviceControll('http://10.0.1.190/?switch_all=0');\">All off</button>" CRLF
      "</div>" CRLF
      "</div>" CRLF
      "<div class=\"group-items\">" CRLF;
    
    static const char body_NEXASwitch1[] __PROGMEM = 
      "<div class=\"group-item\">" CRLF
      "<h3>";
      
    static const char body_NEXASwitch2[] __PROGMEM = 
      "</h3>" CRLF
      "<div class=\"button\">" CRLF
      "<button onclick=\"deviceControll('http://10.0.1.190/?switch=";
      
    static const char body_NEXASwitch3[] __PROGMEM = 
      ",1');\">On</button>" CRLF
      "</div>" CRLF
      "<div class=\"button\">" CRLF
      "<button onclick=\"deviceControll('http://10.0.1.190/?switch=";
      
    static const char body_NEXASwitch4[] __PROGMEM = 
      ",0');\">Off</button>" CRLF
      "</div>" CRLF
      "</div>" CRLF;
      
    // Construct the NEXA Switches part of body
    static const char body_NEXAActivity0[] __PROGMEM =
      "<div class=\"group\">" CRLF
      "<div class=\"group-header\">" CRLF
      "<h2>NEXA Activities</h2>" CRLF
      "</div>" CRLF
      "<div class=\"group-items\">" CRLF;
    
    static const char body_NEXAActivity1[] __PROGMEM = 
      "<div class=\"group-item\">" CRLF
      "<h3>";
      
    static const char body_NEXAActivity2[] __PROGMEM = 
      "</h3>" CRLF
      "<span>" CRLF;
      
    static const char body_NEXAActivity3[] __PROGMEM = 
      "</span>" CRLF
      "</div>" CRLF;
    
    // Cnstruct end div
    static const char body_enddiv[] __PROGMEM = 
      "</div>";
      
    static const char groupscript[] __PROGMEM = 
    "<script>" CRLF
    "  function handler( event ) {" CRLF
    "    var target = $( event.target );" CRLF
    "    target.parent().parent().find( '.group-items' ).toggle();" CRLF
    "  }" CRLF
    "  $( '.group-header' ).click( handler ).parent().find('.group-items').hide();" CRLF
    "  $( '.group' ).first().find('.group-items').show();" CRLF
    "</script>" CRLF;
    
    // Construct footer
    static const char footer[] __PROGMEM = 
      "</body>" CRLF 
      "</html>";
    
    // Print the header and start of body
    page << (str_P) header;
    page << (str_P) body;
    
    // Print NEXA Switches
    page << (str_P) body_NEXASwitch0;
    
    for (int id = 0; id < NEXA_SWITCHES; id++)
    {    
      if(m_parent->m_switch_activated[id]) {
        page << (str_P) body_NEXASwitch1
             << m_parent->m_switch_name[id].c_str()
             << (str_P) body_NEXASwitch2
             << id
             << (str_P) body_NEXASwitch3
             << id
             << (str_P) body_NEXASwitch4;
      }
    }
    
    page << (str_P) body_enddiv << endl << (str_P) body_enddiv << endl;
    
    // Print NEXA Activities
    page << (str_P) body_NEXAActivity0;
    
    for (int id = 0; id < NEXA_ACTIVITIES; id++)
    {    
      if(m_parent->m_activities[id].m_activated) {
        page << (str_P) body_NEXAActivity1
             << m_parent->m_activities[id].m_name.c_str()
             << (str_P) body_NEXAActivity2
             << m_parent->m_activities[id].m_hours << PSTR(":")
             << m_parent->m_activities[id].m_minutes
             << (str_P) body_NEXAActivity3;
      }
    }
    
    page << (str_P) body_enddiv << endl << (str_P) body_enddiv << endl;
    
    // Print time
    time_t time = RTC::time();
    page << PSTR("<div id =\"time\">Time: ") << time << PSTR("</div>") << endl;
    
    // Print group script
    page << (str_P) groupscript;
    
    // Print footer
    page << (str_P) footer;
  }
}

void
ELFI::WebServer::handle_query(char* query)
{
  String toparse = query;
  String keyval, key, val;
  
  while (toparse.length() > 0)
  {
    if (toparse.indexOf('&') > 0) {
      keyval = toparse.substring(0, toparse.indexOf('&'));
    } else {
      keyval = toparse;
    }
    
    key = keyval.substring(0, keyval.indexOf('='));
    val = keyval.substring(keyval.indexOf('=')+1);
    
    if (key.equals("switch"))
    {
      int id = val.substring(0, val.indexOf(',')).toInt();
      if (id >= 0 && id < NEXA_SWITCHES)
      {
        int mode = val.substring(val.indexOf(',')+1).toInt();
        if (mode == 0)
        {
          m_parent->switch_off(id);
        }
        else if (mode == 1)
        {
          m_parent->switch_on(id);
        }
      }
    }
    if (key.equals("switch_all"))
    {
      int mode = val.toInt();
      if (mode == 0)
      {
        m_parent->switch_off();
      }
      else if (mode == 1)
      {
        m_parent->switch_on();
      }
    }
    
    //TODO: Implement parsing for NEXASwitch::switch_dim(int_t dim)
    
    if (toparse.indexOf('&') > 0) {
      toparse = toparse.substring(toparse.indexOf('&')+1);
    } else {
      toparse = "";
    }
  }
}
