#ifndef _FBX_EXCEPTION_H
#define _FBX_EXCEPTION_H _FBX_EXCEPTION_H

#include <exception>
#include <string>

namespace FBX {

	class BaseException : public std::exception {
		std::string m_store;
		const char *m_what_arg;
	protected:
		explicit BaseException(const char *what_arg) : m_what_arg(what_arg) { }
		explicit BaseException(const std::string &what_arg) : m_store(what_arg), m_what_arg(m_store.c_str()) { }
	public:
		virtual const char* what() const throw() { return m_what_arg; }
	};

	class Exception : public BaseException {
	public:
		explicit Exception(const char *what_arg) : BaseException(what_arg) { }
		explicit Exception(const std::string &what_arg) : BaseException(what_arg) { }
	};

	class InflateException : public BaseException {
	public:
		explicit InflateException(const char *what_arg) : BaseException(what_arg) { }
		explicit InflateException(const std::string &what_arg) : BaseException(what_arg) { }
	};

	class ReaderException : public BaseException {
	public:
		explicit ReaderException(const char *what_arg) : BaseException(what_arg) { }
		explicit ReaderException(const std::string &what_arg) : BaseException(what_arg) { }
	};

	class ValueBadCastException : public BaseException {
	public:
		explicit ValueBadCastException(const char *what_arg) : BaseException(what_arg) { }
		explicit ValueBadCastException(const std::string &what_arg) : BaseException(what_arg) { }
	};

}

#endif
