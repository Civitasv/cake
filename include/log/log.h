#ifndef CAKE_LOG_H_
#define CAKE_LOG_H_

#include <memory>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>

#include "term/color.h"
#include "term/style.h"
#include "term/text.h"

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
	}
	Logger(const std::string &logpath)
		: mask_(0xFFFFFFFF)
	{
	}

	~Logger()
	{
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
		exit(errno);
	}

	template <typename... T> inline void Fatal(const T &...msg)
	{
		Log(FATAL, msg...);
		exit(errno);
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
			std::string level = LevelToString(l);

			std::stringstream messages;
			Helper(messages, msg...);
			
			Text text(level + " " + messages.str(), LevelToStyle(l));
			
			std::cout << text << std::endl;
		}
	}

private:
	std::string LevelToString(Level l)
	{
		switch (l) {
		case VERBOSE:
			return "[CAKE][Verbose]";
		case DEBUG:
			return "[CAKE][Debug]";
		case INFO:
			return "[CAKE][Info]";
		case WARNING:
			return "[CAKE][Warning]";
		case ERROR:
			return "[CAKE][Error]";
		case FATAL:
			return "[CAKE][Fatal]";
		default:
			return "[Unknown]";
		}
	}

	Style LevelToStyle(Level l)
	{
		Style style;
		switch (l) {
		case VERBOSE:
			style.fg(Foreground::From(Color::GREEN));
			break;
		case DEBUG:
			style.fg(Foreground::From(RGB(255, 144, 188)));
			break;
		case INFO:
			style.fg(Foreground::From(Color::YELLOW));
			break;
		case WARNING:
			style.fg(Foreground::From(Color::YELLOW)).AddDecoration(Decoration::From(Decoration::BOLD));
			break;
		case ERROR:
			style.fg(Foreground::From(Color::RED)).AddDecoration(Decoration::From(Decoration::BOLD));
			break;
		case FATAL:
			style.fg(Foreground::From(Color::RED)).AddDecoration(Decoration::From(Decoration::BOLD));
			break;
		default:
			style.fg(Foreground::From(Color::GREEN));
			break;
		}

		return style;
	}



private:
	std::mutex mt_;

	uint32_t mask_; //< 用于更细粒度的控制输出级别
};

#endif // !CAKE_LOG_H_
