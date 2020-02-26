#include <unistd.h>
#include <syslog.h>

#include <iostream>
#include <egg/commandline/processor.hpp>

#include <egg/runner/runner.hpp>


namespace test
{

struct daemon : public egg::process
{

daemon(
    const std::string& argv0)
  : egg::process(argv0)
{
  std::cout << "Create console ..." << std::endl;

  set(
    process::property::syslog,
    "test::console");

  enable(
    process::property::syslog);
}

~daemon() noexcept
{
  // ::syslog(LOG_INFO, "Destroy daemon ...");
}

void before()
{
  ::syslog(LOG_INFO, "Call before() ...");
}

void between()
{
  ::syslog(LOG_INFO, "Call between() ...");
}

void after()
{
  ::syslog(LOG_INFO, "Call after() ...");
}

void run()
{
  ::syslog(LOG_INFO, "Call run() ...");
}

};

}

int
main(
  const int   argc,
  const char* argv[])
{
  using std::cout;
  using std::endl;
  using std::cerr;

  egg::commandline::description
  ds(
   "Sample console application",
   "0.0.1-patch-0",
   "GPLv3",
"This is the sample console application that demonstrates the process \
class in action. You are free to use this code sample for whatever you \
want to. Cheers!",
   "For more details see https://github.com/tanuki-no/Egg-Runner.");

  egg::commandline::options op;
  std::string path("sample.xml");
  op('h',
     "help",
     "Display help information on command line arguments")
    ('v',
     "version",
     "Display version")
    ('d',
     "daemon",
     "Run application as a daemon")
    ('t',
     "trace",
     "Enable tracing")
    ('c',
     "configuration",
     "Set configuration file to use",
     path)
    ('V',
     "validate",
     "Validate configuration file (don't run the service, only check configuration and reports the error found)");

  cout << "Checking system app with command line args" << endl;
  cout << "---------------------------------------------------------" << endl;

  try
  {
    egg::commandline::processor cmd(ds, op);

    try
    {
      egg::registry::value _reg;
      cmd.parse(argc, argv, _reg);

      bool __false_start = false;
      for (auto i : _reg["cmd"]['\t'])
      {
        const char __shortKey =
            i.second["short"].as<char>();

        const std::string __longKey(
              std::move(i.second["long"].as<std::string>()));

        if (__shortKey == 'v' || __longKey  == "version")
        {
          cout << ds.version << endl;
          __false_start = true;
        }

        if (__shortKey == 'h' || __longKey  == "help")
        {
          cout << cmd.help() << endl;
          __false_start = true;
        }
      }

      if (!__false_start)
      {
        test::daemon the_console(argv[0]);
	the_console.enable(egg::process::property::trace);
	the_console.enable(egg::process::property::daemon);
	the_console.set(egg::process::property::user, "daemon");
	the_console.set(egg::process::property::group, "daemon");
        the_console.execute();
      }
    }
    catch (const std::system_error& e)
    {
      cerr << e.what() << endl;
    }
    catch (const std::invalid_argument& e)
    {
      cerr << e.what() << endl << endl << cmd.help() << endl;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  cout  << "---------------------------------------------------------" << endl
        << "Done." << endl << endl;

  return 0;
}
