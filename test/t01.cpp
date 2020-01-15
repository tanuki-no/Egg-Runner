#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <system_error>


pid_t
exists(
  const std::string& name) 
{
  pid_t result = -1;
  DIR* __process_dir = ::opendir("/proc");

  if (!__process_dir)
    throw std::system_error(
      errno,
      std::system_category(),
      "Failed to open /proc");
    
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

int
main(
  const int   argc,
  const char* argv[])
{
  using std::cout;
  using std::endl;
  using std::cerr;

  cout << "Checking proc" << endl;
  cout << "---------------------------------------------------------" << endl;
  {
    cout << "Self PID: " << exists(argv[0])
         << ", getpid(): " << getpid()
         << endl;
  }
  cout  << "---------------------------------------------------------" << endl
        << "Done." << endl << endl;

  return 0;
}
