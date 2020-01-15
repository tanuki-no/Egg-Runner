/*!
 *	\file		signal.hpp
 *	\brief		Declares Linux signals
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#ifndef EGG_SIGNAL
#define EGG_SIGNAL

#include <signal.h>

#include <system_error>

#include <egg/common.hpp>


namespace egg
{
namespace signal
{

enum class policy
{
  /// Default action is to terminate the process.
  terminate_process,

  /// Default action is to ignore the signal.
  ignore,

  /// Default action is to terminate the process and dump core (see core(5)).
  core_dump,

  /// Default action is to stop the process.
  stop_process,

  /// Default action is to continue the process if it is currently stopped.
  continue_process
};

// Structure that describes
struct EGG_PUBLIC handler
{
  typedef handler*           pointer;
  typedef const handler*     const_pointer;
  typedef handler&           reference;
  typedef const handler&     const_reference;
  typedef struct sigaction   type;

  // See man sigaction for flags details but aware that SA_SIGINFO is set
  handler(
      const int   /* signal_number */,
      const int   /* the_flags*/ = 0,
      policy      /* the_policy*/ = policy::continue_process) noexcept;
  virtual ~handler() noexcept = 0;

  handler() noexcept = delete;

  handler(const handler&) noexcept = delete;
  handler& operator=(const handler&) noexcept = delete;

  handler(handler&&) noexcept = delete;
  handler& operator=(handler&&) noexcept = delete;

  virtual void process(int) noexcept = 0;
  virtual void process(int, siginfo_t*, void*) noexcept = 0;

  const int id() const noexcept;
  const int flags() const noexcept;
  policy    get_policy() const noexcept;

  type* get_handle() noexcept;
  const type* get_handle() const noexcept;

protected:

  int		_id;
  int		_flags;
  policy	_policy;
  type		_old_action;
};

inline handler::~handler() noexcept {}

// Statistics
struct EGG_PUBLIC stat
{
  stat() noexcept;

  stat(const stat&) noexcept;
  stat& operator=(const stat&) noexcept;

  stat(stat&&) noexcept;
  stat& operator=(stat&&) noexcept;

 ~stat() noexcept;

  unsigned long call_count;
  unsigned long error_count;
};

/*
 * Use: acquire instance:
 *
 * Example: signal::controller& c = signal::controller::instance();
 *
 * Block/unblock signals using toggle
 *
 * Example: c.toggle();
 */
union EGG_PUBLIC controller
{
  enum { count = NSIG };

  controller(const controller&) = delete;
  controller& operator=(const controller&) = delete;

  controller(controller&&) = delete;
  controller& operator=(controller&&) = delete;

  static controller& instance() noexcept;

  // Signal manipulation
  void lock();
  void lock(const int);

  void release();
  void release(const int);

  // Append/remove handler
  void enable(handler::pointer);
  void disable(const int) noexcept;

  // Stat support
  const stat& get_stat(const int) noexcept;

protected:

  controller() noexcept;
 ~controller() noexcept;

private:

  EGG_PRIVATE void __lock(bool);

  EGG_PRIVATE void __lock(bool, const int);
};

} // End of egg::signal namespace
} // End of egg namespace

namespace egg
{
namespace signal
{

inline const int
handler::id() const noexcept
{
  return _id;
}

inline const int
handler::flags() const noexcept
{
  return _flags;
}

inline policy
handler::get_policy() const noexcept
{
  return _policy;
}

inline handler::type*
handler::get_handle() noexcept
{
  return &_old_action;
}

inline const handler::type*
handler::get_handle() const noexcept
{
  return &_old_action;
}

} // End of egg::signal namespace
} // End of egg namespace

#endif  // EGG_SIGNAL

/* End of file */
