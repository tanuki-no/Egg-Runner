/*!
 *	\file		runner.cpp
 *	\brief		Daemon library system process
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <syslog.h>
#include <unistd.h>

#include <cap-ng.h>

#include <cstring>  // std::memset

#include "common.h"

#include <egg/runner/credentials.hpp>
#include <egg/runner/runner.hpp>


namespace egg
{

// Helpers
namespace helper
{

struct EGG_PRIVATE child :
    public egg::signal::handler
{

child() noexcept
  :	egg::signal::handler(SIGCHLD, SA_RESTART)
{
}

virtual ~child() noexcept {}

void process(int the_id) noexcept
{}

void process(
	int         the_id,
	siginfo_t*  the_info,
	void*       the_context) noexcept
{}

};

} // End of sys::helper namespace

// Process itself
process::process(
    const std::string& the_name)
  : _environment(egg::environment::instance()),
    _signal(egg::signal::controller::instance()),
    __f_trace(0),
    __f_is_daemon(0),
    __f_req_user_change(0),
    __f_req_group_change(0),
    __f_req_cwd_change(0),
    __f_req_pid_file(0),
    __f_req_syslog(0),
    __f_req_cgroup(0),
    __f_switch_complete(0),
    _description("Default process"),
    _uid(getuid()),
    _user(credentials::user_id_to_name(getuid())),
    _gid(getgid()),
    _group(credentials::group_id_to_name(getgid())),
    _syslog_label("DMN")
{
  // Purify _name
  {
    std::string::size_type st = the_name.find_last_of('/');
    st = (std::string::npos == st ? 0 : ++st);
    _name = the_name.substr(st);
  }
}

process::~process() noexcept
{
  if (__f_req_syslog)
    ::closelog();
}

void
process::execute()
{
  // Open syslog
  if (__f_req_syslog)
  {
    ::openlog(
            _syslog_label.c_str(),
            LOG_CONS    |
            LOG_NDELAY  |
            LOG_PERROR  |
            LOG_PID,
            LOG_DAEMON);

    if (__f_trace)
    {
      ::syslog(LOG_INFO, "Start logging ...");
    }
  }

  // Check if service is up
  __is_service_up();

  // Build directories
  if (__f_req_pid_file)
  {
    std::string __path = _pid_path.substr(0, _pid_path.find_last_of('/'));

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Create directory \"%s\" if required ...", __path.c_str());
    }

    credentials::create_directory(__path, _uid, _gid);
  }

  // Call before
  before();

  // Configure capabilities
  __set_capabilities();

  // Set credentials
  __set_credentials();

  // Change working directory
  __cwd();

  // First fork
  if (__f_is_daemon)
  {
    if (__fork())
      return;

    __detach_terminal();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_INFO, "Starting new session ...");
    }

    if (::setsid() < 0)
    {
      throw std::system_error(errno, std::system_category(), "setsid() failed");
    }
  }

  // Complete environment preset
  __in_between();

  // Between
  between();

  // Second fork
  if (__f_is_daemon && __fork())
    return;

  // Initialize and set signal handlers not done since
  // the iheritated implementation could do it itself

  // Write PID file
  __write_pid();

  // Last call
  after();

  // Complete
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_INFO, "Initialization complete!");
  }

  // Done
  __f_switch_complete = 1;

  // Main cycle
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_INFO, "Starting main cycle ...");
  }

  // Main cycle
  run();

  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_INFO, "Main cycle complete!");
  }

  // Remove pid
  __remove_pid();
}

/* Properties */
void
process::enable(
    process::property	the_property)
  noexcept
{
  if (property::trace == the_property)
  {
    __f_trace = 1;
  }
  else if (property::daemon == the_property)
  {
    __f_is_daemon = 1;
  }
  else if(property::user == the_property)
  {
    __f_req_user_change = 1;
  }
  else if(property::group == the_property)
  {
    __f_req_group_change = 1;
  }
  else if(property::working_directory == the_property)
  {
    __f_req_cwd_change = 1;
  }
  else if(property::pid_file == the_property)
  {
    __f_req_pid_file = 1;
  }
  else if(property::syslog == the_property)
  {
    __f_req_syslog = 1;

    // Open syslog
    if (__f_req_syslog)
    {
      ::openlog(
	_syslog_label.c_str(),
	LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID,
	LOG_DAEMON);

      if (__f_trace)
      {
	::syslog(LOG_INFO, "Start logging ...");
      }
    }
  }
  else if(property::cgroup == the_property)
  {
    __f_req_cgroup = 1;
  }
}

void
process::disable(
    process::property	the_property)
  noexcept
{
  if (property::trace == the_property)
  {
    __f_trace = 0;
  }
  else if (property::daemon == the_property)
  {
    __f_is_daemon = 0;
  }
  else if (property::user == the_property)
  {
    __f_req_user_change = 0;
  }
  else if (property::group == the_property)
  {
    __f_req_group_change = 0;
  }
  else if (property::working_directory == the_property)
  {
    __f_req_cwd_change = 0;
  }
  else if (property::pid_file == the_property)
  {
    __f_req_pid_file = 0;
  }
  else if (property::syslog == the_property)
  {
    __f_req_syslog = 0;
    ::closelog();
  }
  else if (property::cgroup == the_property)
  {
    __f_req_cgroup = 0;
  }
}

bool
process::status(
    process::property	the_property)
  const noexcept
{
  if (property::trace == the_property)
  {
    return (__f_trace ? true : false);
  }
  else if (property::daemon == the_property)
  {
    return (__f_is_daemon ? true : false);
  }
  else if (property::user == the_property)
  {
    return (__f_req_user_change ? true : false);
  }
  else if (property::group == the_property)
  {
    return (__f_req_group_change ? true : false);
  }
  else if (property::working_directory == the_property)
  {
    return (__f_req_cwd_change ? true : false);
  }
  else if (property::pid_file == the_property)
  {
    return (__f_req_pid_file ? true : false);
  }
  else if (property::syslog == the_property)
  {
    return (__f_req_syslog ? true : false);
  }
  else if (property::cgroup == the_property)
  {
    return (__f_req_cgroup ? true : false);
  }

  return false;
}

void
process::set(
    process::property	the_property,
    const egg::variable& the_value)
{
  if (property::description == the_property)
  {
    _description = the_value.as_string();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Set name to \"%s\"", _description.c_str());
    }
  }
  else if (property::user == the_property)
  {
    _uid  = credentials::name_to_user_id(the_value.as_string());
    _user = the_value.as_string();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Set user name to \"%s\" (id: %u)", _user.c_str(), _uid);
    }
  }
  else if (property::group == the_property)
  {
    _gid   = credentials::name_to_group_id(the_value.as_string());
    _group = the_value.as_string();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Set group name to \"%s\" (id: %u)", _group.c_str(), _gid);
    }
  }
  else if (property::working_directory == the_property)
  {
    _working_directory = the_value.as_string();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Set working directory to \"%s\"", _working_directory.c_str());
    }
  }
  else if (property::pid_file == the_property)
    {
      _pid_path = the_value.as_string();

      if (__f_req_syslog && __f_trace)
      {
	::syslog(LOG_DEBUG, "Set PID file name to \"%s\"", _pid_path.c_str());
      }
    }
  else if (property::syslog == the_property)
  {
    _syslog_label = the_value.as_string();

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(LOG_DEBUG, "Change label to \"%s\"", _syslog_label.c_str());
    }
  }
}

egg::variable
process::get(
    process::property the_property) const
{
  if (property::description == the_property)
  {
    return _description;
  }
  else if (property::user == the_property)
  {
    return _user;
  }
  else if (property::group == the_property)
  {
    return _group;
  }
  else if (property::working_directory == the_property)
  {
    return _working_directory;
  }
  else if (property::pid_file == the_property)
  {
    return _pid_path;
  }
  else if (property::syslog == the_property)
  {
    return _syslog_label;
  }
  else
  {
    return std::move(egg::variable());
  }
}

bool
process::is_final_instance() const noexcept
{
  return __f_switch_complete;
}

// Implementation
void
process::__is_service_up()
{
  if (__f_req_pid_file)
  {
    int fd = open(_pid_path.c_str(), O_RDONLY | O_CLOEXEC);

    // If pid file exists
    if (fd >= 0)
    {
      const std::size_t Size = 64;
      char txt[Size];
      std::memset(txt, '\0', Size);
      ssize_t count = read(fd, txt, Size);

      // Not readable. No credentials?
      if (count < 0)
      {
        std::error_code ec(errno, std::system_category());

        if (__f_req_syslog && __f_trace)
	{
          ::syslog(
            LOG_ERR,
            "read(%d, %s, %ld): %s, %d",
            fd, txt, Size, ec.message().c_str(), ec.value());
	}

        throw std::system_error(ec, "Pid file " );
      }

      // Check if process exists
      pid_t __pid = std::strtol(txt, nullptr, 10);

      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_DEBUG, "Querying PID %d ...", __pid);
      }

      if (kill(__pid, 0))
      {
        std::error_code ec(errno, std::generic_category());

        if (ec != std::errc::no_such_process)
        {
          if (__f_req_syslog)
            ::syslog(
              LOG_ERR,
              "kill(%d, 0): %s, %d",
              __pid, ec.message().c_str(), ec.value());

          throw std::system_error(ec, "Process check");
        }
      }
      else
      {
        std::string msg("Process ");
        msg.append(txt);
        msg.append(" exists");

        if (__f_req_syslog)
	{
          ::syslog(LOG_ALERT, "%s", msg.c_str());
	}

        throw std::system_error(
          std::make_error_code(std::errc::device_or_resource_busy), msg);
      }
    }
  }

  // Process check based on non pid
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(
        LOG_DEBUG,
        "Check if the process with the name %s does exist",
        _name.c_str());
  }

  pid_t __existsing_process = exists(_name);
  if (__existsing_process != -1)
  {
    std::string msg("Identical process ");
    msg.append(std::to_string(__existsing_process));
    msg.append(" exists. Please, stop it first");

    if (__f_req_syslog)
    {
      ::syslog(LOG_ALERT, "%s", msg.c_str());
    }

    throw std::system_error(
      std::make_error_code(std::errc::device_or_resource_busy),
      msg);
  }
}

// Capabilities
void
process::__set_capabilities() noexcept
{
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_DEBUG, "Seting up capabilities ...");
  }

  try
  {
    if (capng_get_caps_process())
    {
      throw std::system_error(
	std::make_error_code(std::errc::invalid_argument),
	"Unable to get capabilities");
    }

    capng_clear(CAPNG_SELECT_BOTH);

    if (capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE, CAP_SETUID, CAP_SETGID, -1) < 0)
    {
      throw std::system_error(
	errno,
	std::system_category(),
	"capng_updatev(.., CAPNG_EFFECTIVE)");
    }

    if (capng_updatev(CAPNG_ADD, CAPNG_PERMITTED, CAP_SETUID, CAP_SETGID, -1) < 0)
    {
      throw std::system_error(
	errno,
	std::system_category(),
	"capng_updatev(.., CAPNG_PERMITTED)");
    }

    if (capng_apply(CAPNG_SELECT_BOTH) < 0)
    {
      throw std::system_error(
	errno,
	std::system_category(),
	"capng_apply(CAPNG_SELECT_BOTH)");
    }
  }
  catch (const std::system_error& ec)
  {
    if (__f_req_syslog)
    {
      ::syslog(LOG_ERR, "%s", ec.what());
    }

    capng_clear(CAPNG_SELECT_BOTH);
  }

  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_DEBUG, "Capabilities set.");
  }
}

// Credentials
void
process::__set_credentials()
{
  // No user change required
  if (!__f_req_user_change)
  {
    if (__f_req_syslog)
    {
      ::syslog(
	LOG_INFO,
	"User switch disabled!");
    }

    return;
  }

  const uid_t euid    = geteuid();
  std::string euser   = credentials::user_id_to_name(euid);
  const gid_t egid    = getegid();
  std::string egroup  = credentials::group_id_to_name(egid);

  if (__f_req_syslog && __f_trace)
  {
    ::syslog(
      LOG_DEBUG,
      "Switch user EUID: %s(%d), UID: %s(%d), EGID: %s(%d), GID: %s(%d).",
      _user.c_str(),    _uid,
      euser.c_str(),    euid,
      _group.c_str(),   _gid,
      egroup.c_str(),   egid);
  }

  // Same uid
  if (_uid == euid)
  {
    if (__f_req_syslog)
    {
      ::syslog(
        LOG_INFO,
        "Trying to switch to the same user. Credentials are kept as is.");
    }

    return;
  }

  // No enough credentials
  if (0 != getuid() &&
        !capng_have_capability(CAPNG_EFFECTIVE, CAP_SETUID))
  {
    if (__f_req_syslog)
    {
      ::syslog(
	LOG_INFO,
	"Not enough credentials to switch to user \"%s\" and group \"%s\"",
	_user.c_str(),
	_group.c_str());
    }

    return;
  }

  // Retain privilleges over the uid/gid switch using Linux capabilities
  if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SETUID))
  {
    try
    {
      capng_clear(CAPNG_SELECT_BOTH);

      if (capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE, CAP_SETUID, CAP_SETGID, -1))
      {
        throw std::system_error(
            errno,
            std::system_category(),
            "capng_updatev(.., CAPNG_EFFECTIVE)");
      }

      if (capng_updatev(CAPNG_ADD, CAPNG_PERMITTED, CAP_SETUID, CAP_SETGID, -1) < 0)
      {
        throw std::system_error(
            errno,
            std::system_category(),
            "capng_updatev(.., CAPNG_PERMITTED)");
      }

      if (capng_change_id(_uid, _gid, CAPNG_DROP_SUPP_GRP))
      {
        throw std::system_error(
            errno,
            std::system_category(),
            "capng_change_id(.., CAPNG_DROP_SUPP_GRP)");
      }
    }
    catch (const std::system_error& ec)
    {
      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_ERR, "%s", ec.what());
      }

      capng_clear(CAPNG_SELECT_BOTH);

      throw ec;
    }

#if HAVE_INITGROUPS

    errno = 0;
    if (::initgroups(_user.c_str(), _gid) < 0)
    {
      throw std::system_error(
          errno,
          std::system_category(),
          "initgroups()");
    }

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(
          LOG_INFO,
          "Initializing all groups for the user %s",
          _user.c_str());
    }

#endif

    if (__f_req_syslog && __f_trace)
    {
      ::syslog(
        LOG_INFO,
        "User/ID: %s (%d), effective user/UID: %s (%d), group/ID: %s (%d), effective group/ID: %s (%d)",
        _user.c_str(),	_uid,
        euser.c_str(),	euid,
        _group.c_str(),	_gid,
        egroup.c_str(),	egid);
    }
  }

  // Traditional setup
  else
  {
    // Setting group and affective group
    if (0 == getgid())
    {
      errno = 0;

      if (0 > setgid(_gid))
      {
        throw std::system_error(errno, std::system_category(), "setgid()");
      }

      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_INFO, "Setting up group/ID: %s (%d)", _group.c_str(), _gid);
      }

#if HAVE_INITGROUPS

      errno = 0;

      if (0 > ::initgroups(_user.c_str(), _gid))
      {
        throw std::system_error(errno, std::system_category(), "initgroups()");
      }

      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_INFO, "Initializing all groups for the user %s", _user.c_str());
      }

#endif

      errno = 0;

      if (0 > setegid(_gid))
      {
        throw std::system_error(errno, std::system_category(), "setegid()");
      }

      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_INFO, "Setting up effective group/ID: %s (%d)", _group.c_str(), _gid);
      }
    }

    // Setting user and effective user
    if (0 == getuid())
    {
      errno = 0;

      if (0 > setuid(_uid))
      {
        throw std::system_error(errno,  std::system_category(), "setuid()");
      }

      if (__f_req_syslog && __f_trace)
      {
        ::syslog(LOG_INFO, "Setting up user/ID: %s (%d)", _user.c_str(), _uid);
      }

      errno = 0;

      if (0 > seteuid(_uid))
      {
        throw std::system_error(errno, std::system_category(), "seteuid()");
      }

      if (__f_req_syslog)
      {
        ::syslog(LOG_INFO, "Setting up effective user/ID: %s (%d)", _user.c_str(), _uid);
      }
    }
  }

  // Set up environment
  {
    struct passwd* pw;

#ifdef HAVE_PWUID_R

    struct passwd p;
    const std::size_t __size = 1024 * 4;
    char __buffer[__size];
    std::memset(__buffer, 0, __size);

    if (getpwuid_r(_uid, &p, __buffer, __size, &pw))
    {
      throw std::system_error(errno, std::system_category(), "getpwuid_r()");
    }

#else

    if ((pw = getpwuid(_uid)) == NULL)
    {
      throw std::system_error(errno, std::system_category(), "getpwuid()");
    }

#endif

    ::setenv("USER",	_user.c_str(),	1);
    ::setenv("LOGNAME",	_user.c_str(),	1);
    ::setenv("HOME",	pw->pw_dir,	1);
  }

  // Done
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(
        LOG_DEBUG,
        "Successfully retaining privilleges over UID switch");
  }
}

// Change working directory
void
process::__cwd()
{
  if (!__f_req_cwd_change)
    return;

  if (::chdir(_working_directory.c_str()))
  {
    if (::chdir("/"))
    {
      std::error_code ec(errno, std::system_category());

      std::string msg("Both chdir(\"/\") and chdir(\"");
      msg.append(_working_directory);
      msg.append("\") failed");

      throw std::system_error(ec, msg);
    }
  }
}

// Daemonization
bool
process::__fork()
{
  // Lock everything
  _signal.lock();

  // Install child signal
  _signal.enable(new helper::child());

  // Enable child signal
  _signal.release(SIGCLD);

  pid_t pid = ::fork();

  // Failed
  if (pid < 0)
  {
    std::error_code ec(errno, std::system_category());

    // Restore signals
    _signal.disable(SIGCHLD);
    _signal.release();

    throw std::system_error(ec, "fork() failed");
  }
  // Parent
  else if (pid != 0)
  {
    int wait_status = 0;

    // Wait failed
    if (::waitpid(pid, &wait_status, WUNTRACED | WNOHANG | WCONTINUED) < 0)
    {
      std::error_code ec(errno, std::system_category());

      // Restore signals
      _signal.disable(SIGCHLD);
      _signal.release();

      throw std::system_error(ec, "waitpid() failed");
    }

    // Restore
    _signal.disable(SIGCHLD);
    _signal.release();

    return true;
  }

  // Restore
  _signal.disable(SIGCHLD);
  _signal.release();

  return false;
}

void
process::__detach_terminal()
{
  // Handy reopener
  auto null_open = [](
      int the_attributes,
      int the_fd)
  {
    // Open
    const int the_null_fd = ::open("/dev/null", the_attributes);
    if (the_null_fd < 0)
    {
      throw std::system_error(
              errno,
              std::system_category(),
              "Failed to open \"/dev/null\"");
    }

    // Duplicate
    errno = 0;
    if (0 > ::dup2(the_null_fd, the_fd) < 0)
    {
      std::error_code ec(errno, std::system_category());
      std::string msg("Failed to duplicate descriptor ");
      msg.append(std::to_string(the_fd));
      throw std::system_error(ec, msg);
    }

    // Close
    ::close(the_null_fd);
  };

  // Rebind descriptors
  null_open(O_RDONLY, 0);
  null_open(O_WRONLY, 1);
  null_open(O_WRONLY, 2);

  // Close all open file descriptors other than stdin, stdout, stderr
  int descriptor_count = ::getdtablesize();
  for (int i = 3; i < descriptor_count; ++i)
  {
    ::close(i);
  }
}

void
process::__in_between()
{
  // Setting mask
  if (__f_req_syslog && __f_trace)
  {
    ::syslog(LOG_INFO, "Setting mask 0077");
  }

  ::umask(0077);

  // Make sure we have a relatively sane environment
  if (NULL == getenv("IFS"))
  {
    setenv("IFS", " \t\n", 1);
  }

  if (NULL == getenv("PATH"))
  {
    setenv("PATH","/usr/local/sbin:/sbin:/bin:/usr/sbin:/usr/bin", 1);
  }
}

void
process::__write_pid()
{
  if (!__f_req_pid_file || _pid_path.empty())
  {
    if (__f_req_syslog)
    {
      ::syslog(
	LOG_WARNING,
	"PID file not set. Bypassing ...");
    }

    return;
  }

  if (__f_req_syslog && __f_trace)
  {
    ::syslog(
	LOG_INFO,
	"Writing PID to %s ...",
	_pid_path.c_str());
  }

  // Writing
  FILE* f = NULL;
  if (NULL == (f = ::fopen (_pid_path.c_str(), "w")))
  {
    std::error_code ec(errno, std::system_category());
    std::string msg("fopen(");
    msg.append(_pid_path);
    msg.append(", w) failed");

    if (__f_req_syslog)
    {
      ::syslog(LOG_ERR, "%s: %s, %d", msg.c_str(), ec.message().c_str(), ec.value());
    }

    throw std::system_error(ec, msg);
  }

  // Write PID
  if (0 > ::fprintf(f, "%d", getpid()))
  {
    std::error_code ec(errno, std::system_category());
    std::string msg("fprintf() failed");

    if (__f_req_syslog)
    {
      ::syslog(LOG_ERR, "%s: %s, %d", msg.c_str(), ec.message().c_str(), ec.value());
    }

    throw std::system_error(ec, msg);
  }

  // Done
  ::fclose(f);
}

void
process::__remove_pid() noexcept
{
  remove(_pid_path.c_str());
}

// Process utilities
const pid_t
process::exists(
  const std::string  name)
{
  pid_t result = -1;
  DIR* __process_dir = ::opendir("/proc");

  if (!__process_dir)
  {
    throw std::system_error(errno, std::system_category(), "Failed to open /proc");
  }

  const pid_t self_pid = getpid();

  // Iterate the /proc/<pid>
  bool __is_matched = false;
  char __buffer[512];
  for (struct dirent* entry  = ::readdir(__process_dir);
                      NULL  != entry;
                      entry  = ::readdir(__process_dir))
  {
    try
    {
      long __pid = std::stol(entry->d_name);
      if (__pid == self_pid)
        continue;

      std::string __path("/proc/");
      __path.append(entry->d_name);
      __path.append("/cmdline");

      FILE* __file = ::fopen(__path.c_str(), "r");
      if (__file != NULL &&
         ::fgets(__buffer, 512, __file) != NULL)
      {
        std::string __cmd__line(__buffer);
        if (__cmd__line.find(name) != std::string::npos)
        {
          __is_matched = true;
          result = __pid;
        }
      }
      ::fclose(__file);

      if (__is_matched)
        break;
    }
    catch (const std::exception& e)
    {
      // Do nothing
    }
  }

  ::closedir(__process_dir);

  return result;
}

void
process::exists(
  const std::string    name,
  std::list<pid_t>&  result)
{
  DIR* __process_dir = ::opendir("/proc");

  if (!__process_dir)
  {
    throw std::system_error(errno, std::system_category(), "Failed to open /proc");
  }

  // Iterate the /proc/<pid>
  bool __is_matched = false;
  char __buffer[512];
  for (struct dirent* entry  = ::readdir(__process_dir);
                      entry != NULL;
                      entry  = ::readdir(__process_dir))
  {
    try
    {
      long __pid = std::stol(entry->d_name);

      std::string __path("/proc/");
      __path.append(entry->d_name);
      __path.append("/cmdline");

      FILE* __file = ::fopen(__path.c_str(), "r");
      if (__file != NULL &&
         ::fgets(__buffer, 512, __file) != NULL)
      {
        std::string __cmd__line(__buffer);
        if (__cmd__line.find(name) != std::string::npos)
          result.push_back(__pid);
      }
      ::fclose(__file);
    }
    catch (const std::exception& e)
    {
      // Do nothing
    }
  }

  ::closedir(__process_dir);
}

} // End of sys namespace

// End of file
