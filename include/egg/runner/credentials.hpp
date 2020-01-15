/*!
 *	\file		credentials.hpp
 *	\brief		Declares credentials utilities
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#ifndef EGG_RUNNER_CREDENTIALS
#define EGG_RUNNER_CREDENTIALS

#include <string>
#include <vector>
#include <system_error>

#include <egg/common.hpp>


namespace egg
{

struct EGG_PRIVATE credentials
{
  static const std::string
  user_id_to_name(
    const uid_t);

  static const uid_t
  name_to_user_id(
    const std::string&);

  static const std::string
  group_id_to_name(
    const gid_t);

  static const gid_t
  name_to_group_id(
    const std::string&);

  static const std::string
  working_directory();

  static void
  create_directory(
    const std::string the_path,
    const uid_t       the_uid,
    const gid_t       the_gid);
};

} // End of egg namespace

#endif  // EGG_RUNNER_CREDENTIALS

/* End of file */
