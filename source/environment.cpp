/*!
 *	\file		environment.cpp
 *	\brief		Implements environment utilities
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#include <egg/runner/environment.hpp>


extern char **environ;

namespace egg
{

egg::registry::value environment::_env;

void
environment::__load(
      const char** data)
{
  if (_env.size())
    _env.clear();

  for (auto e = data; e != nullptr && *e != nullptr; ++e)
  {
    const char* p = *e;
    std::string key, value;

    while (p && *p && *p != '=')
      key.push_back(*p++);

    ++p;

    while (p && *p)
      value.push_back(*p++);

    _env[key] = value;
  }
}

environment::environment() noexcept
{
  __load(const_cast<const char **>(environ));
}

environment::~environment() noexcept
{}

environment&
environment::instance()
{
  static environment _instance;

  return _instance;
}

static variable _empty;

variable&
environment::operator[](
    std::string& key) noexcept
{
  return (_env.count(key) ? _env[key] : _empty);
}

const variable&
environment::operator[](
    const std::string& key) const noexcept
{
  return (_env.count(key) ? _env[key] : _empty);
}

} // End of egg namespace

/* End of file */
