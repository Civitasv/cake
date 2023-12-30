#ifndef CAKE_LOG_H_
#define CAKE_LOG_H_

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>

#define BIT(n) 1 << n

std::ostream &operator<<(std::ostream &os, const std::vector<std::string> &args);
class Logger {
    public:
	enum Level {
		VERBOSE = BIT(0),
		DEBUG = BIT(1),
		INFO = BIT(2),
		WARNING = BIT(3),
		ERROR = BIT(4),
		FATAL = BIT(5),
		NO_LOG = BIT(6)
	};

    public:
	Logger()
		: mask_(0xFFFFFFFF)
	{
		logfile_ = stdout;
	}
	Logger(const std::string &logpath)
		: mask_(0xFFFFFFFF)
	{
		logfile_ = fopen(logpath.c_str(), "w");
	}

	~Logger()
	{
		if (logfile_ != stdout)
			fclose(logfile_);
	}

	/**
   * @brief 创建日志实例，输出到文件
   *
   * @param logpath 日志输出文件
   * @return std::shared_ptr<Logger> 该日志实例可以在多个线程中共享，
   *         引用数量为0时会被自动回收
   */
	static std::shared_ptr<Logger> Create(const std::string &logpath)
	{
		return std::make_shared<Logger>(logpath);
	}

	/**
   * @brief 创建日志实例，输出到标准输出
   *
   * @return std::shared_ptr<Logger> 该日志实例可以在多个线程中共享，
   *         引用数量为0时会被自动回收
   */
	static std::shared_ptr<Logger> Create()
	{
		return std::make_shared<Logger>();
	}

	template <typename... T> inline void Verbose(const T &...msg)
	{
		Log(VERBOSE, msg...);
	}

	template <typename... T> inline void Debug(const T &...msg)
	{
		Log(DEBUG, msg...);
	}

	template <typename... T> inline void Info(const T &...msg)
	{
		Log(INFO, msg...);
	}

	template <typename... T> inline void Warning(const T &...msg)
	{
		Log(WARNING, msg...);
	}

	template <typename... T> inline void Error(const T &...msg)
	{
		Log(ERROR, msg...);
	}

	template <typename... T> inline void Fatal(const T &...msg)
	{
		Log(FATAL, msg...);
	}

	template <typename T>
	friend std::shared_ptr<Logger>
	operator<<(std::shared_ptr<Logger> logger, const T &data)
	{
		std::stringstream ss;
		logger->Helper(ss, data);

		fprintf(logger->logfile_, "%s", ss.str().c_str());

		return logger;
	}

    public:
	void set_mask(uint32_t mask)
	{
		mask_ = mask;
	}

    private:
	template <typename T0, typename... T>
	void Helper(std::stringstream &ss, const T0 &t0, const T &...msg)
	{
		ss << t0;
		if constexpr (sizeof...(msg) > 0)
			Helper(ss, msg...);
	}

	template <typename... T> void Log(Level l, const T &...msg)
	{
		bool check = ((l & mask_) != 0);
		if (check) {
			std::lock_guard<std::mutex> lk(mt_);
			auto level = LevelToString(l);

			std::stringstream ss;
			Helper(ss, msg...);

			fprintf(logfile_, "%s ", level.c_str());
			fprintf(logfile_, "%s", ss.str().c_str());
			fprintf(logfile_, "\n");
		}
	}

    private:
	std::string LevelToString(Level l)
	{
		switch (l) {
		case VERBOSE:
			return "[Verbose]";
		case DEBUG:
			return "[Debug]";
		case INFO:
			return "[Info]";
		case WARNING:
			return "[Warning]";
		case ERROR:
			return "[Error]";
		case FATAL:
			return "[Fatal]";
		default:
			return "[Unknown]";
		}
	}

    private:
	FILE *logfile_;
	std::mutex mt_;

	uint32_t mask_; //< 用于更细粒度的控制输出级别
};

#endif // !CAKE_LOG_H_
