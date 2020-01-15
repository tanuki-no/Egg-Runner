/*!
 *	\file		signal.cpp
 *	\brief		Implements Linux signals
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#include <cstring>

#include <egg/runner/signal.hpp>


namespace egg
{
namespace signal
{

// Entity
handler::handler(
    const int signal_number,
    const int the_flags,
    policy    the_policy) noexcept
  : _id(signal_number),
    _flags(the_flags),
    _policy(the_policy)
{
}

//handler::~handler() noexcept
//{}

// Stat
stat::stat() noexcept
  : call_count(0),
    error_count(0)
{}

stat::stat(
    const stat& other) noexcept
  : call_count(other.call_count),
    error_count(other.error_count)
{}

stat&
stat::operator=(
    const stat& other) noexcept
{
  if (this != &other)
  {
    call_count = other.call_count;
    error_count = other.error_count;
  }

  return *this;
}

stat::stat(
    stat&& other) noexcept
  : call_count(other.call_count),
    error_count(other.error_count)
{
  other.call_count = other.error_count = 0;
}

stat&
stat::operator=(stat&& other) noexcept
{
  if (this != &other)
  {
    call_count = other.call_count;
    error_count = other.error_count;
    other.call_count = other.error_count = 0;
  }

  return *this;
}

stat::~stat() noexcept
{}

// Controller
static handler*	_s_handler[controller::count];
static stat	_s_stat[controller::count];

static void
__signal_callback(
    int         the_id,
    siginfo_t*  the_info,
    void*       the_context)
{
  if (_s_handler[the_id] != nullptr)
  {
    _s_handler[the_id]->process(the_id, the_info, the_context);
    _s_stat[the_id].call_count++;
  }
  else
    _s_stat[the_id].error_count++;
}

// Controller
controller::controller() noexcept
{
  for (auto i = 0; i < count; ++i)
    _s_handler[i] = nullptr;
}

controller::~controller() noexcept
{
  for (auto i = 0; i < count; ++i)
    disable(i);
}

controller&
controller::instance() noexcept
{
  static controller _instance;
  return _instance;
}

void
controller::__lock(
    bool is_lock_required)
{
  // Set action
  const int __action = (is_lock_required ? SIG_BLOCK : SIG_UNBLOCK);

  // Fetch current mask
  sigset_t mask;

  // Get mask
  if (sigfillset(&mask))
  {
    throw std::system_error(
          errno,
          std::system_category(),
          "Failed to call sigfillset");
  }

  // Append
  if (sigprocmask(__action, &mask, nullptr))
  {
    throw std::system_error(
          errno,
          std::system_category(),
          "Failed to call sigprocmask for all signals");
  }
}

void
controller::__lock(
    bool      is_lock_required,
    const int the_id)
{
  if (the_id < count)
  {
      // Set action
      const int __action = (is_lock_required ? SIG_BLOCK : SIG_UNBLOCK);

      // Fetch current mask
      sigset_t mask;

      // Get mask
      if (sigprocmask(__action, nullptr, &mask))
      {
        std::error_code ec(errno, std::system_category());

        std::string msg("Failed to call sigprocmask for signal ");
        msg.append(std::to_string(the_id));

        throw std::system_error(ec, msg);
      }

      // Append
      if (sigaddset(&mask, the_id))
      {
        std::error_code ec(errno, std::system_category());

        std::string msg("Wrong signal ");
        msg.append(std::to_string(the_id));

        throw std::system_error(ec, msg);
      }

      // Append
      if (sigprocmask(__action, &mask, nullptr))
      {
        std::error_code ec(errno, std::system_category());

        std::string msg("Failed to call sigprocmask for signal ");
        msg.append(std::to_string(the_id));

        throw std::system_error(ec, msg);
      }
  }
  else
  {
    std::error_code ec = std::make_error_code(
          std::errc::invalid_argument);

    std::string msg("Wrong signal code ");
    msg.append(std::to_string(the_id));
    msg.append(". The max signal value is ");
    msg.append(std::to_string(count - 1));

    throw std::system_error(ec, msg);
  }
}

void
controller::lock()
{
  __lock(true);
}

void
controller::lock(
    const int id)
{
  __lock(true, id);
}

void
controller::release()
{
  __lock(false);
}

void
controller::release(
    const int id)
{
  __lock(false, id);
}

// Enable new handler
void
controller::enable(
    handler::pointer the_handle)
{
  if (the_handle == nullptr)
  {
    std::error_code ec = std::make_error_code(
          std::errc::invalid_argument);

    std::string msg("Null pointer");

    throw std::system_error(ec, msg);
  }

  const int id = the_handle->id();

  if (id >= count)
  {
    std::error_code ec = std::make_error_code(
          std::errc::invalid_argument);

    std::string msg("Wrong signal code ");
    msg.append(std::to_string(id));
    msg.append(". The max signal value is ");
    msg.append(std::to_string(count - 1));

    throw std::system_error(ec, msg);
  }

  // Clean up
  if (_s_handler[id] != nullptr)
  {
    delete _s_handler[id];
    _s_handler[id] =  nullptr;
  }

  // Move to handler
  _s_handler[id] = std::move(the_handle);

  // Fetch current mask
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);

  // Fill the set
  for (auto i = 0; i < count; ++i)
  {
    if (_s_handler[i] != nullptr &&
        sigaddset(&sa.sa_mask, i))
    {
      std::error_code ec(errno, std::system_category());

      std::string msg("Wrong signal ");
      msg.append(std::to_string(i));

      throw std::system_error(ec, msg);
    }
  }

  // Append
  sa.sa_flags = SA_SIGINFO | _s_handler[id]->flags();
  sa.sa_sigaction = &__signal_callback;

  // Set action
  if (sigaction(id, &sa, _s_handler[id]->get_handle()))
  {
    std::error_code ec(errno, std::system_category());
    std::string msg("Unable to set up signal handler for ");
    msg.append(std::to_string(id));
    throw std::system_error(ec, msg);
  }
}

void
controller::disable(
    const int id) noexcept
{
  if (_s_handler[id] == nullptr)
    return;

  ::sigaction(id, _s_handler[id]->get_handle(), nullptr);

  delete _s_handler[id];
  _s_handler[id] = nullptr;
}

const stat&
controller::get_stat(const int the_id) noexcept
{
  return _s_stat[the_id];
}

} // End of egg::signal namespace
} // End of egg namespace

/* End of file */
