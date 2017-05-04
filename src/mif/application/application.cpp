//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     03.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

// C
#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <signal.h>
#endif

// STD
#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

// BOOST
#include <boost/filesystem.hpp>
#include <boost/program_options/parsers.hpp>

// MIF
#include "mif/application/id/config.h"
#include "mif/application/id/service.h"
#include "mif/application/application.h"
#include "mif/common/log.h"
#include "mif/common/log_init.h"
#include "mif/common/static_string.h"
#include "mif/common/version.h"
#include "mif/service/root_locator.h"
#include "mif/service/create.h"

namespace Mif
{
    namespace Application
    {

        namespace Option
        {

            MIF_DECLARE_SRTING_PROVIDER(Help, "help")
            MIF_DECLARE_SRTING_PROVIDER(Version, "version")
            MIF_DECLARE_SRTING_PROVIDER(Daemon, "daemon")
            MIF_DECLARE_SRTING_PROVIDER(Config, "config")
            MIF_DECLARE_SRTING_PROVIDER(ConfigFormat, "configformat")
            MIF_DECLARE_SRTING_PROVIDER(LogDir, "logdir")
            MIF_DECLARE_SRTING_PROVIDER(LogPattern, "logpattern")
            MIF_DECLARE_SRTING_PROVIDER(LogLevel, "loglevel")

        }   // namespace Option

        namespace
        {

            MIF_DECLARE_SRTING_PROVIDER(ProjectPageLink, "https://github.com/tdv/mif")

        }   // namespace

        class Application::Daemon final
        {
        public:
        #if defined(__linux__) || defined(__unix__)
            Daemon(std::function<void ()> onStart, std::function<void ()> onStop)
            {
                if (daemon(0, 0))
                    throw std::runtime_error{"[Mif::Application::Daemon] The application could not be started in daemon mode."};

                MIF_LOG(Info) << "[Mif::Application::Daemon] Starting application in a daemon mode...";

                try
                {
                    onStart();
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Fatal) << "[Mif::Application::Daemon] Failed to start application. Error: " << e.what();
                }
                catch (...)
                {
                    MIF_LOG(Fatal) << "[Mif::Application::Daemon] Failed to start application. Error: unknown error.";
                }

                signal(SIGINT, ShutdownSignal);
                signal(SIGQUIT, ShutdownSignal);
                signal(SIGTERM, ShutdownSignal);

                signal(SIGPIPE, SIG_IGN);

                m_mutex.reset(new std::mutex);
                m_cv.reset(new std::condition_variable);

                std::unique_lock<std::mutex> lock{*m_mutex};

                MIF_LOG(Info) << "[Mif::Application::Daemon] The application is started in daemon mode.";

                m_cv->wait(lock);

                MIF_LOG(Info) << "[Mif::Application::Daemon] Stopping daemon ...";

                m_cv.reset();
                m_mutex.reset();

                try
                {
                    onStop();
                }
                catch (std::exception const &e)
                {
                    MIF_LOG(Warning) << "[Mif::Application::Daemon] Failed to correct stop application. Error: " << e.what();
                }
                catch (...)
                {
                    MIF_LOG(Warning) << "[Mif::Application::Daemon] Failed to correct stop application. Error: unknown error.";
                }

                MIF_LOG(Info) << "[Mif::Application::Daemon] Stopped daemon.";
            }
        private:
            static std::unique_ptr<std::mutex> m_mutex;
            static std::unique_ptr<std::condition_variable> m_cv;

            static void ShutdownSignal(int)
            {
                if (m_cv)
                    m_cv->notify_all();
            }
        #endif
        };

        #if defined(__linux__) || defined(__unix__)
        std::unique_ptr<std::mutex> Application::Daemon::m_mutex;
        std::unique_ptr<std::condition_variable> Application::Daemon::m_cv;
        #endif

        Application::Application(int argc, char const **argv)
            : m_argc{argc}
            , m_argv{argv}
            , m_logLevel{Common::Log::Level::Trace}
            , m_optionsDescr{"Allowed options"}
            , m_version{"snapshot"}
            , m_name{boost::filesystem::path{argv[0]}.filename().c_str()}
            , m_description{"MIF application"}
        {
            m_optionsDescr.add_options()
                    (Option::Help::GetString(), "Show help.")
                    (Option::Version::GetString(), "Show program version.")
            #if defined(__linux__) || defined(__unix__)
                    (Option::Daemon::GetString(), "Run as daemon.")
            #endif
                    (Option::Config::GetString(), boost::program_options::value<std::string>(&m_configFileName), "Config file name (full path).")
                    (Option::ConfigFormat::GetString(), boost::program_options::value<std::string>(&m_configFileFormat)->default_value(
                            "json"), "Config file format (available formats: json, xml).")
                    (Option::LogDir::GetString(), boost::program_options::value<std::string>(&m_logDirName), "Log directory name.")
                    (Option::LogPattern::GetString(), boost::program_options::value<std::string>(&m_logPattern), "Log file pattern.")
                    (Option::LogLevel::GetString(), boost::program_options::value<std::uint32_t>(&m_logLevel), "Log level.");
        }

        Application::~Application()
        {
        }

        void Application::OnStart()
        {
        }

        void Application::OnStop()
        {
        }

        int Application::GetArgc() const
        {
            return m_argc;
        }

        char const** Application::GetArgv() const
        {
            return m_argv;
        }

        void Application::AddCustomOptions(boost::program_options::options_description const &options)
        {
            m_optionsDescr.add(options);
        }

        boost::program_options::variables_map const& Application::GetOptions() const
        {
            return m_options;
        }

        void Application::SetName(std::string const &name)
        {
            m_name = name;
        }

        std::string const& Application::GetName() const
        {
            return m_name;
        }

        IConfigPtr Application::GetConfig() const
        {
            return m_config;
        }

        void Application::SetVersion(std::string const &version)
        {
            m_version = version;
        }

        std::string const& Application::GetVersion() const
        {
            return m_version;
        }

        void Application::SetDescription(std::string const &description)
        {
            m_description = description;
        }

        std::string const& Application::GetDescription() const
        {
            return m_description;
        }

        int Application::Run()
        {
            try
            {
                boost::program_options::store(
                        boost::program_options::parse_command_line(m_argc, m_argv, m_optionsDescr),
                        m_options
                    );

                boost::program_options::notify(m_options);

                if (m_options.count(Option::Help::GetString()))
                {
                    std::cout << m_optionsDescr << std::endl;
                    return EXIT_SUCCESS;
                }

                if (m_options.count(Option::Version::GetString()))
                {
                    std::cout << "Application: " << m_name << std::endl
                              << "Veraion: " << m_version << std::endl
                              << "Description: " << m_description << std::endl
                              << "MIF " << Common::Version::GetAsString() << " " << ProjectPageLink::GetString() << std::endl;
                    return EXIT_SUCCESS;
                }

                bool pathFromConfig = false;
                bool levelFromConfig = false;

                IConfigPtr components;

                if (m_options.count(Option::Config::GetString()))
                {
                    auto config = LoadConfig();
                    if (config)
                    {
                        if (config->Exists("data"))
                            m_config = config->GetConfig("data");

                        if (config->Exists("common"))
                        {
                            auto common = config->GetConfig("common");

                            if (!m_options.count(Option::Daemon::GetString()) && common->Exists("daemon"))
                                m_runAsDaemon = common->GetValue<bool>("daemon");

                            if (common->Exists("log"))
                            {
                                auto log = common->GetConfig("log");
                                if (!m_options.count(Option::LogDir::GetString()) && log->Exists("dir"))
                                {
                                    m_logDirName = log->GetValue("dir");
                                    pathFromConfig = true;
                                }
                                if (!m_options.count(Option::LogPattern::GetString()) && log->Exists("pattern"))
                                {
                                    m_logPattern = log->GetValue("pattern");
                                    pathFromConfig = true;
                                }
                                if (!m_options.count(Option::LogLevel::GetString()) && log->Exists("level"))
                                {
                                    m_logLevel = static_cast<Common::Log::Level>(log->GetValue<std::uint32_t>("level"));
                                    levelFromConfig = true;
                                }
                            }

                            if (config->Exists("components"))
                                components = config->GetConfig("components");
                        }
                    }
                }

                InitLog(pathFromConfig, levelFromConfig);

                Start(components);
            }
            catch (std::exception const &e)
            {
                MIF_LOG(Fatal) << "[Mif::Application::Application::Run] Failed to start application. Error: " << e.what();
            }
            return EXIT_FAILURE;
        }

        void Application::InitLog(bool pathFromConfig, bool levelFromConfig)
        {
            if (m_options.count(Option::LogLevel::GetString()) || levelFromConfig)
            {
                if (m_logLevel > Common::Log::Level::Trace)
                {
                    throw std::invalid_argument{"[Mif::Application::Application::Run] "
                            "Invalid log level valie \"" + std::to_string(m_logLevel) + "\". "
                            "The value of the logging level should not exceed \"" + std::to_string(Common::Log::Level::Trace) + "\"."};
                }
            }

            if (m_options.count(Option::LogDir::GetString()) || m_options.count(Option::LogPattern::GetString()) || pathFromConfig)
            {
                if (!boost::filesystem::exists(m_logDirName))
                {
                    if (!boost::filesystem::create_directories(m_logDirName))
                    {
                        throw std::runtime_error{"[Mif::Application::Application::Run] "
                                "Failed to create log directory \"" + m_logDirName + "\""};
                    }
                }
                Common::InitFileLog(static_cast<Common::Log::Level>(m_logLevel), m_logDirName, m_logPattern);
            }
            else if (m_options.count(Option::LogLevel::GetString()) || levelFromConfig)
            {
                Common::InitConsoleLog(static_cast<Common::Log::Level>(m_logLevel));
            }
        }

        void Application::Start(IConfigPtr components)
        {
            auto locator = Service::RootLocator::Get();

            try
            {
            #if defined(__linux__) || defined(__unix__)
                if (m_options.count(Option::Daemon::GetString()))
                    m_runAsDaemon = true;
            #endif
            }
            catch (std::exception const &e)
            {
                locator->Clear();
                MIF_LOG(Fatal) << "[Mif::Application::Application::Start] Failed to start application. Error: " << e.what();
                throw;
            }

            try
            {
                if (!m_runAsDaemon)
                    RunInThisProcess(components);
                else
                    RunAsDaemon(components);
            }
            catch (std::exception const &e)
            {
                locator->Clear();
                MIF_LOG(Fatal) << "[Mif::Application::Application::Start] Failed to call OnStart. Error: " << e.what();
                throw;
            }
        }

        void Application::Stop()
        {
            try
            {
                OnStop();
            }
            catch (std::exception const &e)
            {
                Service::RootLocator::Get()->Clear();
                MIF_LOG(Warning) << "[Mif::Application::Application::Stop] Failed to call OnStop. Error: " << e.what();
            }
        }

        IConfigPtr Application::LoadConfig() const
        {
            if (!boost::filesystem::exists(m_configFileName))
            {
                throw std::invalid_argument{"[Mif::Application::Application::LoadConfig] "
                    "Failed to load config from file \"" + m_configFileName + "\". File not found."};
            }
            if (!boost::filesystem::is_regular_file(m_configFileName))
            {
                throw std::invalid_argument{"[Mif::Application::Application::LoadConfig] "
                        "Failed to load config file. "
                        "The path \"" + m_configFileName + "\" is not a regular file."};
            }

            auto file = std::make_shared<std::ifstream>(m_configFileName, std::ios_base::in);
            if (!file->is_open())
            {
                throw std::invalid_argument{"[Mif::Application::Application::LoadConfig] "
                        "Failed to open config file \"" + m_configFileName + "\""};
            }

            if (m_configFileFormat == "json")
                return Service::Create<Id::Service::Config::Json, IConfig>(file);
            else if (m_configFileFormat == "xml")
                return Service::Create<Id::Service::Config::Xml, IConfig>(file);

            throw std::invalid_argument{"[Mif::Application::Application::LoadConfig] Unsupported format \"" + m_configFileFormat + "\""};
        }

        void Application::RunAsDaemon(IConfigPtr components)
        {
            #if defined(__linux__) || defined(__unix__)

            if (!m_daemon)
            {
                m_daemon.reset(new Daemon{
                        [this, components]
                        {
                            if (components)
                                Service::RootLocator::Get()->Put<Id::Service::ComponentsFactory>(components);
                            OnStart();
                        },
                        [this] { Stop(); }
                        }
                    );
            }

            #endif
        }

        void Application::RunInThisProcess(IConfigPtr components)
        {
            std::exception_ptr exception{};

            std::thread t{
                    [this, &components, &exception]
                    {
                        try
                        {
                            if (components)
                                Service::RootLocator::Get()->Put<Id::Service::ComponentsFactory>(components);
                            OnStart();
                            std::cout << "Press 'Enter' for quit." << std::endl;
                            std::cin.get();
                            Stop();
                        }
                        catch (...)
                        {
                            exception = std::current_exception();
                        }
                    }
                };

            t.join();

            if (exception)
                std::rethrow_exception(exception);
        }

    }   // namespace Application
}   // namespace Mif
