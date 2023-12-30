#ifndef CAKE_TERM_TEXT_H_
#define CAKE_TERM_TEXT_H_

#include "style.h"

class Text {
public:
	Text()
	{
	}
	Text(std::string text)
		: text_(text)
	{
	}
	Text(std::string text, Style style)
		: text_(text)
		, style_(style)
	{
	}

	Text &style(Style style)
	{
		style_ = style;
		return *this;
	}

	Text &text(std::string text)
	{
		text_ = text;
		return *this;
	}

	std::string text()
	{
		return text_;
	}

	friend std::ostream &operator<<(std::ostream &os, const Text &val)
	{
		os << val.style_ << val.text_
		   << Decoration::From(Decoration::RESET);
		return os;
	}

private:
	std::string text_;
	Style style_;
};
#endif // CAKE_TERM_TEXT_H_
