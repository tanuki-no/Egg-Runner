/*!
 *	\file		runner.hpp
 *	\brief		Declares process
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#ifndef EGG_RUNNER
#define EGG_RUNNER

#include <list>

#include <egg/common.hpp>
#include <egg/variable.hpp>
#include <egg/runner/environment.hpp>
#include <egg/runner/signal.hpp>


namespace egg
{

class EGG_PUBLIC process
{

public:

  /**********************************************
   * Properties
   **********************************************/
  enum class property : std::uint32_t
  {
    description,
    daemon,
    user,
    group,
    working_directory,
    pid_file,
    syslog,
    cgroup
  };

  /**********************************************
   * Construct/destruct
   **********************************************/

  // Create the process
  process(
      const std::string& /*argv[0]*/);
  virtual ~process() noexcept;

  // Lock default
  process() = delete;

  // Lock copy
  process(const process&) = delete;
  process& operator=(const process&) = delete;

  // Lock move
  process(process&&) = delete;
  process& operator=(process&&) = delete;

public:

  /**********************************************
   * Run-time
   **********************************************/

  /* Switch to background and run the sequence
     before(), between(), after() and then run() */
  void execute();

public:

  /**********************************************
   * Properties
   **********************************************/
  void enable(property) noexcept;
  void disable(property) noexcept;
  void toggle(property, bool) noexcept;
  bool status(property) const noexcept;

  void set(property, const egg::variable&);
  egg::variable get(property) const;

public:

  // Get idea if this is the instance of successfuly initialized
  // background process
  bool is_final_instance() const noexcept;

protected:

  // What to do before the service switch to the background
  virtual void before() = 0;

  // What to do between the service states switch
  virtual void between() = 0;

  // What to do after the switch
  virtual void after() = 0;

  // Main cycle after the switch
  virtual void run() = 0;

protected:

  // Environment
  egg::environment&         _environment;

  // Signal
  egg::signal::controller&  _signal;

private:

  // Flags
  std::uint32_t			__f_is_daemon		: 1;
  std::uint32_t			__f_req_user_change	: 1;
  std::uint32_t			__f_req_group_change	: 1;
  std::uint32_t			__f_req_cwd_change	: 1;
  std::uint32_t			__f_req_pid_file	: 1;
  std::uint32_t			__f_req_syslog		: 1;
  std::uint32_t			__f_req_cgroup		: 1;
  std::uint32_t			__f_switch_complete	: 1;
  std::uint32_t			__f_unused		: 24;

  // Program name
  std::string                   _name;
  std::string                   _description;

  // Credentials to switch to
  uid_t				_uid;
  std::string			_user;
  gid_t				_gid;
  std::string			_group;
  std::list<std::string>	_group_list;

  // Syslog
  std::string			_syslog_label;

  // Current working directory and PID file path
  std::string			_working_directory;
  std::string			_pid_path;

private:

  // Checkers
  EGG_PRIVATE void __is_service_up();

  // Capabilities
  EGG_PRIVATE void __set_capabilities()
    noexcept;

  // Credentials
  EGG_PRIVATE void __set_credentials();

  // Fork
  EGG_PRIVATE bool __fork();

  EGG_PRIVATE void __detach_terminal();

  EGG_PRIVATE void __in_between();

  // Working directory
  EGG_PRIVATE void __cwd();

  // PID utils
  EGG_PRIVATE void __write_pid();

  EGG_PRIVATE void __remove_pid()
    noexcept;

  EGG_PRIVATE const pid_t
  exists(
      const std::string /*name*/);

  EGG_PRIVATE void
  exists(
      const std::string   /*name*/,
      std::list<pid_t>&   /*result*/);
};

inline void
process::toggle(
    property p, bool the_value) noexcept
{
  if (the_value)
    enable(p);
  else
    disable(p);
}

} // End of egg namespace

#endif  // EGG_RUNNER

/* End of file */
