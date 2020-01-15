/*!
 *	\file		utility.cpp
 *	\brief		Implements command line options and description
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <cstring>
#include <cctype>
#include <limits>

#include "common.h"

#include <egg/runner/credentials.hpp>


namespace egg
{

// Get name by UID
const std::string
credentials::user_id_to_name(
    const uid_t the_id)
{
  struct passwd* _password = nullptr;

#ifdef HAVE_PWUID_R

  const long the_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  if (-1 == the_size)
  {
    throw std::system_error(
      std::make_error_code(std::errc::result_out_of_range),
      "Failed call to sysconf(_SC_GETPW_R_SIZE_MAX)");
  }

  char buffer[the_size];
  std::memset(buffer, 0, the_size);
  struct passwd _password_r;

#endif

  errno = 0;

#ifdef HAVE_PWUID_R

  if (getpwuid_r(
        the_id,
        &_password_r,
        buffer,
        the_size,
        &_password))
  {
    throw std::system_error(
      errno,
      std::system_category(),
      "Failed call to getpwuid_r()");
  }

#else

  _password = getpwuid(the_id);

#endif

  if (NULL == _password)
  {
    throw std::system_error(
      std::make_error_code(std::errc::invalid_argument),
      "User id " + std::to_string(the_id) + " not found");
  }

  return _password->pw_name;
}

// Get UID by name
const uid_t
credentials::name_to_user_id(
    const std::string& the_name)
{
  struct passwd* _password = NULL;

#ifdef HAVE_PWNAM_R

  const long the_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  if (-1 == the_size)
  {
    throw std::system_error(
      std::make_error_code(std::errc::result_out_of_range),
      "Failed call to sysconf(_SC_GETPW_R_SIZE_MAX)");
  }

  char buffer[the_size];
  std::memset(buffer, 0, the_size);
  struct passwd _password_r;

#endif

  errno = 0;

#ifdef HAVE_PWNAM_R

  if (getpwnam_r(
        the_name.c_str(),
        &_password_r,
        buffer,
        the_size,
        &_password))
  {
    throw std::system_error(
            errno,
            std::system_category(),
            "Failed call to getpwnam_r()");
  }

#else

  _password = getpwnam(the_name.c_str());

#endif

  if (NULL == _password)
  {
    throw std::system_error(
            std::make_error_code(std::errc::invalid_argument),
            "User \"" + the_name + "\" not found");
  }

  return _password->pw_uid;
}

// Get name by GID
const std::string
credentials::group_id_to_name(
    const gid_t the_id)
{
  struct group* _group = NULL;

#ifdef HAVE_GRGID_R

  const long the_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  if (-1 == the_size)
  {
    throw std::system_error(
      std::make_error_code(std::errc::result_out_of_range),
      "Failed call to sysconf(_SC_GETPW_R_SIZE_MAX)");
  }
  char buffer[the_size];
  std::memset(buffer, 0, the_size);
  struct group _group_r;

#endif

  errno = 0;

#ifdef HAVE_GRGID_R

  if (getgrgid_r(
        the_id,
        &_group_r,
        buffer,
        the_size,
        &_group))
  {
    throw std::system_error(
            errno,
            std::system_category(),
            "Failed call to getgrgid_r()");
  }

#else

  _group = getgrgid(the_id);

#endif

  if (_group == NULL)
  {
    throw std::system_error(
            std::make_error_code(std::errc::invalid_argument),
            "Group id " + std::to_string(the_id) + " not found");
  }

  return _group->gr_name;
}

// Get GID by name
const gid_t
credentials::name_to_group_id(
    const std::string& the_name)
{
  struct group* _group = NULL;

#ifdef HAVE_GRNAM_R

  const long the_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  if (-1 == the_size)
  {
    throw std::system_error(
      std::make_error_code(std::errc::result_out_of_range),
      "Failed call to sysconf(_SC_GETPW_R_SIZE_MAX)");
  }
  char buffer[the_size];
  std::memset(buffer, 0, the_size);
  struct group _group_r;

#endif

  errno = 0;

#ifdef HAVE_GRNAM_R

  if (getgrnam_r(the_name.c_str(), &_group_r, buffer, the_size, &_group))
  {
    throw std::system_error(
            errno,
            std::system_category(),
            "Failed call to getgrnam_r()");
  }

#else

  _group = getgrnam(the_name.c_str());

#endif

  if (_group == NULL)
  {
    throw std::system_error(
            std::make_error_code(std::errc::invalid_argument),
            "Group \"" + the_name + "\" not found");
  }

  return _group->gr_gid;
}

// Get current working directory
const std::string
credentials::working_directory()
{
  std::size_t size = 1024;
  char* buffer = new char[size];

  while (getcwd(buffer, size) == NULL)
  {
    std::error_code ec(errno, std::system_category());
    if (ec == std::errc::result_out_of_range)
    {
      size *= 2;
      delete [] buffer;
      char* buffer = new char[size];
    }
    else
    {
      delete [] buffer;
      throw std::system_error(ec, "getcwd() failed");
    }
  }

  std::string result(buffer);
  delete [] buffer;
  return result;
}

// Create directory
void
credentials::create_directory(
    const std::string the_path,
    const uid_t       the_uid,
    const gid_t       the_gid)
{
  struct stat d_info;
  DIR* d = ::opendir(the_path.c_str());

  // Exists?
  if (NULL == d)
  {
    if (::mkdir(the_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
    {
      std::error_code ec(errno, std::system_category());

      std::string msg("mkdir(");
      msg.append(the_path);
      msg.append(", 0755) failed");

      throw std::system_error(ec, msg);
    }

    if (::stat(the_path.c_str(), &d_info))
    {
      std::error_code ec(errno, std::system_category());

      std::string msg("stat(");
      msg.append(the_path);
      msg.append(", ...) failed");

      throw std::system_error(ec, msg);
    }
  }
  else
  {
    int result = ::fstat(::dirfd(d), &d_info);
    ::closedir(d);

    if (result)
    {
      std::error_code ec(errno, std::system_category());

      std::string msg("fstat(");
      msg.append(the_path);
      msg.append(", ...) failed");

      throw std::system_error(ec, msg);
    }
  }

  // Change ownership
  if ((d_info.st_uid != the_uid || d_info.st_gid != the_gid) &&
       ::chown(the_path.c_str(), the_uid, the_gid))
  {
    std::error_code ec(errno, std::system_category());

    std::string msg("chown(");
    msg.append(the_path);
    msg.push_back(',');
    msg.append(std::to_string(the_uid));
    msg.push_back(',');
    msg.append(std::to_string(the_gid));
    msg.append(") failed");

    throw std::system_error(ec, msg);
  }
}

} // End of egg namespace

/* End of file */
