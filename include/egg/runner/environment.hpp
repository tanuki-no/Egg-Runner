/*!
 *	\file		environment.hpp
 *	\brief		Declares environment utilities
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#ifndef EGG_ENVIRONMENT
#define EGG_ENVIRONMENT

#include <string>

#include <egg/common.hpp>
#include <egg/variable.hpp>
#include <egg/registry/value.hpp>


namespace egg
{

/*
 * Environment singleton
 *
 * Environment variables are stored in registry under ["env"] tag with environement
 * variable name after it.
 *
 * Example:
 *
 * instance()["env"]["PATH"] = "/Some/Path";
*/
struct EGG_PUBLIC environment
{
  static environment& instance();

  environment(const environment&) = delete;
  environment& operator=(const environment&) = delete;

  environment(environment&&) = delete;
  environment& operator=(environment&&) = delete;

  variable& operator[](std::string& /*key*/) noexcept;
  const variable& operator[](const std::string& /*key*/) const noexcept;

protected:

  environment() noexcept;
 ~environment() noexcept;

private:

  EGG_PRIVATE void __load(
      const char** /*data*/);

  static egg::registry::value _env;
};

} // End of egg namespace

#endif  // EGG_ENVIRONMENT

/* End of file */
