/*!
 *	\file		filesystem.hpp
 *	\brief		Declares file utilities
 *	\author		Vladislav "Tanuki" Mikhailikov \<vmikhailikov\@gmail.com\>
 *	\copyright	GNU GPL v3
 *	\date		14/01/2020
 *	\version	1.0
 */

#ifndef EGG_FILE
#define EGG_FILE

#include <chrono>
#include <string>
#include <system_error>

#include <egg/common.hpp>


namespace egg
{

// Time structure
struct timestamp
{
  inline timestamp() noexcept : sec(0), nsec(0) {}
  inline ~timestamp() noexcept {}

  std::chrono::seconds      sec;
  std::chrono::nanoseconds  nsec;
};

enum class type
{
  // Unknown
  is_unknown,

  // is it a regular file?
  is_regular,

  // directory?
  is_directory,

  // character device?
  is_char_device,

  // block device?
  is_block_device,

  // FIFO (named pipe)?
  is_fifo,

  // symbolic link?  (Not in POSIX.1-1996.)
  is_link,

  // socket?  (Not in POSIX.1-1996.)
  is_socket
};

struct permission
{
  inline permission() noexcept : read(0), write(0), execute(0) {}
  inline permission(const permission& other) noexcept
    : read(other.read), write(other.write), execute(other.execute) {}
  inline permission& operator=(const permission& other) noexcept
    {
      if (this != &other)
      {
        read = other.read;
        write = other.write;
        execute = other.execute;
      }

      return *this;
    }
  inline ~permission() noexcept {}

  std::uint8_t  read:     1;
  std::uint8_t  write:    1;
  std::uint8_t  execute:  1;
};

struct mode
{
  inline mode() noexcept : set_user_id(0), set_group_id(0), sticky_bit(0) {}
  inline mode(const mode& o) noexcept
    : set_user_id(o.set_user_id),
      set_group_id(o.set_group_id),
      sticky_bit(o.sticky_bit),
      user(o.user),
      group(o.group),
      other(o.other)
  {}
  inline mode& operator=(const mode& o) noexcept
  {
    if (this != &o)
    {
      set_user_id = o.set_user_id;
      set_group_id = o.set_group_id;
      sticky_bit = o.sticky_bit;
      user = o.user;
      group = o.group;
      other = o.other;
    }

    return *this;
  }
  inline ~mode() noexcept {}

  std::uint32_t     set_user_id: 1;
  std::uint32_t     set_group_id: 1;
  std::uint32_t     sticky_bit: 1;

  permission        user;
  permission        group;
  permission        other;
};

/*
 * File utilities
 */
struct EGG_PUBLIC file
{
  file();
 ~file() noexcept;

  file(const file&);
  file& operator=(const file&);

  file(file&&);
  file& operator=(file&&);

  file(const std::string& e);

  void load(const std::string& e);

  // Existence
  static bool exist(
      const std::string& /*the_file_name*/) noexcept;

  bool exist() noexcept;
  bool is_valid() const noexcept;

  // Accessors
  const std::string& get_name() const noexcept;
  const ino_t       get_inode() const noexcept;
  const type        get_type() const noexcept;
  const mode        get_mode() const noexcept;
  const nlink_t     get_nlink() const noexcept;

  // Permisions
  const permission  get_user() const noexcept;
  const permission  get_group() const noexcept;
  const permission  get_other() const noexcept;

  const uid_t       get_user_id() const noexcept;
  const gid_t       get_group_id() const noexcept;
  const dev_t       get_device_id() const noexcept;
  const dev_t       get_real_device_id() const noexcept;

  // Sizes
  const off_t       get_size() const noexcept;
  const blksize_t   get_block_size() const noexcept;
  const blkcnt_t    get_block_count() const noexcept;

  // Time
  const timestamp   get_access_time() const noexcept;
  const timestamp   get_modification_time() const noexcept;
  const timestamp   get_status_change_time() const noexcept;

private:

  EGG_PRIVATE void __stat();

private:

  std::string _name;

  ino_t       _inode;         // inode number
  type        _type;          // Type
  mode        _mode;          // Mode
  nlink_t     _link_count;    // Number of hard links

  uid_t       _uid;           // user ID of owner
  gid_t       _gid;           // group ID of owner
  dev_t       _dev;           // ID of device containing file
  dev_t       _rdev;          // device ID (if special file)

  off_t       _size;          // total size, in bytes
  blksize_t   _block_size;    // blocksize for filesystem I/O
  blkcnt_t    _block_count;   // number of 512B blocks allocated

  timestamp   _access;
  timestamp   _modification;
  timestamp   _status_change;
};

// Inlines
inline bool
file::is_valid() const noexcept
{
  return (type::is_unknown != _type);
}

inline bool
file::exist() noexcept
{
  return (type::is_unknown != _type);
}

// Accessors
inline const std::string&
file::get_name() const noexcept
{
  return _name;
}

inline const ino_t
file::get_inode() const noexcept
{
  return _inode;
}

inline const type
file::get_type() const noexcept
{
  return _type;
}

inline const mode
file::get_mode() const noexcept
{
  return _mode;
}

inline const nlink_t
file::get_nlink() const noexcept
{
  return _link_count;
}

// Permisions
inline const permission
file::get_user() const noexcept
{
  return _mode.user;
}

inline const permission
file::get_group() const noexcept
{
  return _mode.group;
}

inline const permission
file::get_other() const noexcept
{
  return _mode.other;
}

inline const uid_t
file::get_user_id() const noexcept
{
  return _uid;
}

inline const gid_t
file::get_group_id() const noexcept
{
  return _gid;
}

inline const dev_t
file::get_device_id() const noexcept
{
  return _dev;
}

inline const dev_t
file::get_real_device_id() const noexcept
{
  return _rdev;
}

// Sizes
inline const off_t
file::get_size() const noexcept
{
  return _size;
}

inline const blksize_t
file::get_block_size() const noexcept
{
  return _block_size;
}

inline const blkcnt_t
file::get_block_count() const noexcept
{
  return _block_count;
}

// Time
inline const timestamp
file::get_access_time() const noexcept
{
  return _access;
}

inline const timestamp
file::get_modification_time() const noexcept
{
  return _modification;
}

inline const timestamp
file::get_status_change_time() const noexcept
{
  return _status_change;
}

} // End of egg namespace

#endif  // EGG_FILE

/* End of file */
