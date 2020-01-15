/*!
 *	\file		filesystem.cpp
 *	\brief		Implements file utilities
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <egg/runner/filesystem.hpp>


namespace egg
{

void
file::__stat()
{
  using std::chrono::seconds;
  using std::chrono::nanoseconds;

  struct stat buffer;

  if (::stat(_name.c_str(), &buffer) < 0)
    throw std::system_error(errno, std::system_category());

  // iNode
  _inode = buffer.st_ino;

  // Type
  if (S_ISREG(buffer.st_mode))
    _type = type::is_regular;
  else if (S_ISDIR(buffer.st_mode))
    _type = type::is_directory;
  else if (S_ISCHR(buffer.st_mode))
    _type = type::is_char_device;
  else if (S_ISBLK(buffer.st_mode))
    _type = type::is_block_device;
  else if (S_ISFIFO(buffer.st_mode))
    _type = type::is_fifo;
  else if (S_ISLNK(buffer.st_mode))
    _type = type::is_link;
  else if (S_ISSOCK(buffer.st_mode))
    _type = type::is_socket;
  else
    _type = type::is_unknown;

  // Mode
  if (buffer.st_mode & S_ISUID)
    _mode.set_user_id = 1;

  if (buffer.st_mode & S_ISGID)
    _mode.set_group_id = 1;

  if (buffer.st_mode & S_ISVTX)
    _mode.sticky_bit = 1;

  if (buffer.st_mode & S_IRUSR)
    _mode.user.read = 1;

  if (buffer.st_mode & S_IWUSR)
    _mode.user.write = 1;

  if (buffer.st_mode & S_IXUSR)
    _mode.user.execute = 1;

  if (buffer.st_mode & S_IRGRP)
    _mode.group.read = 1;

  if (buffer.st_mode & S_IWGRP)
    _mode.group.write = 1;

  if (buffer.st_mode & S_IXGRP)
    _mode.group.execute = 1;

  if (buffer.st_mode & S_IROTH)
    _mode.other.read = 1;

  if (buffer.st_mode & S_IWOTH)
    _mode.other.write = 1;

  if (buffer.st_mode & S_IXOTH)
    _mode.other.execute = 1;

  _link_count   = buffer.st_nlink;
  _uid          = buffer.st_uid;
  _gid          = buffer.st_gid;
  _dev          = buffer.st_dev;
  _rdev         = buffer.st_rdev;

  _size         = buffer.st_size;
  _block_size   = buffer.st_blksize;
  _block_count  = buffer.st_blocks;


  _access.sec   = seconds(buffer.st_atim.tv_sec);
  _access.nsec  = nanoseconds(buffer.st_atim.tv_nsec);

  _modification.sec   = seconds(buffer.st_mtim.tv_sec);
  _modification.nsec  = nanoseconds(buffer.st_mtim.tv_nsec);

  _status_change.sec  = seconds(buffer.st_ctim.tv_sec);
  _status_change.nsec = nanoseconds(buffer.st_ctim.tv_nsec);
}

file::file()
  : _inode(-1),
    _type(type::is_unknown),
    _link_count(0),
    _uid(-1),
    _gid(-1),
    _dev(-1),
    _rdev(-1),
    _size(0),
    _block_size(0),
    _block_count(0)
{
}

file::~file() noexcept
{}

file::file(
    const file& other)
  : _name(other._name),
    _inode(other._inode),
    _mode(other._mode),
    _type(other._type),
    _link_count(other._link_count),
    _uid(other._uid),
    _gid(other._gid),
    _dev(other._dev),
    _rdev(other._rdev),
    _size(other._size),
    _block_size(other._block_size),
    _block_count(other._block_count)
{}

file&
file::operator=(
    const file& other)
{
  if (this != &other)
  {
    _name = other._name;
    _inode = other._inode;
    _mode = other._mode;
    _type = other._type;
    _link_count = other._link_count;
    _uid = other._uid;
    _gid = other._gid;
    _dev = other._dev;
    _rdev = other._rdev;
    _size = other._size;
    _block_size = other._block_size;
    _block_count = other._block_count;
  }

  return *this;
}

file::file(
    file&& other)
  : _name(other._name),
    _inode(other._inode),
    _mode(other._mode),
    _type(other._type),
    _link_count(other._link_count),
    _uid(other._uid),
    _gid(other._gid),
    _dev(other._dev),
    _rdev(other._rdev),
    _size(other._size),
    _block_size(other._block_size),
    _block_count(other._block_count)
{
  other._type = type::is_unknown;
}

file&
file::operator=(
    file&&  other)
{
  if (this != &other)
  {
    _name = other._name;
    _inode = other._inode;
    _mode = other._mode;
    _type = other._type;
    _link_count = other._link_count;
    _uid = other._uid;
    _gid = other._gid;
    _dev = other._dev;
    _rdev = other._rdev;
    _size = other._size;
    _block_size = other._block_size;
    _block_count = other._block_count;

    other._type = type::is_unknown;
  }

  return *this;
}

file::file(
    const std::string& the_file_name)
  : _name(the_file_name),
    _inode(-1),
    _type(type::is_unknown),
    _link_count(0),
    _uid(-1),
    _gid(-1),
    _dev(-1),
    _rdev(-1),
    _size(0),
    _block_size(0),
    _block_count(0)
{
  __stat();
}

void
file::load(const std::string& the_file_name)
{
  _name = the_file_name;
  __stat();
}

bool
file::exist(
      const std::string& the_file_name) noexcept
{
  struct stat buffer;

  if (::stat(the_file_name.c_str(), &buffer) < 0)
    return false;

  return true;
}

} // End of egg namespace

/* End of file */
