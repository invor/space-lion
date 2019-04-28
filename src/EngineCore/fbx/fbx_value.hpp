#ifndef _FBX_VALUE_H
#define _FBX_VALUE_H _FBX_VALUE_H

#include "fbx_types.hpp"

#include <memory>

/* you have to know the exact type to retrieve a value */

namespace FBX {
	class Value {
	private:
		class BaseContent {
		public:
			virtual ~BaseContent();
			virtual void print(std::ostream &os, uint32_t limit) = 0;
			virtual const char* name() = 0;
		protected:
			BaseContent() { }
		private:
			/* not copyable */
			BaseContent(const BaseContent& other);
			BaseContent& operator=(const BaseContent& other);
		};
		template<typename T>
		class Content : public BaseContent {
		public:
			T data;
			Content(const T& data) : data(data) { }
			virtual ~Content() { }
			virtual void print(std::ostream &os, uint32_t limit) { Print<T>::print(os, data, limit); }
			virtual const char* name() { return Print<T>::name(); };
		};

		std::shared_ptr<BaseContent> content;
	public:
		template<typename T>
		static Value make(const T& data) {
			Value v;
			v.set(data);
			return v;
		}

		template<typename T>
		void set(const T& data) {
			content.reset(new Content<T>(data));
		}

		template<typename T>
		T* tryGet() const {
			std::shared_ptr< Content<T> > c(std::dynamic_pointer_cast< Content<T> >(content));
			if (!c) return nullptr;
			return &c->data;
		}

		template<typename T>
		T& get() const {
			T* data = tryGet<T>();
			if (nullptr == data) throw ValueBadCastException("type mismatch");
			return *data;
		}

		void print(std::ostream &os, uint32_t limit) {
			if (!content) os << "null"; else content->print(os, limit);
		}

		const char *name() {
			if (!content) return "null"; else return content->name();
		}
	};
	inline std::ostream& operator<<(std::ostream& os, Value value) {
		value.print(os, -1);
		return os;
	}

	class SharedValue {
	private:
		class BaseContent {
		public:
			virtual ~BaseContent();
			virtual void print(std::ostream &os, uint32_t limit) = 0;
			virtual const char* name() = 0;
		protected:
			BaseContent() { }
		private:
			/* not copyable */
			BaseContent(const BaseContent& other);
			BaseContent& operator=(const BaseContent& other);
		};
		template<typename T>
		class Content : public BaseContent {
		public:
			typedef std::shared_ptr<T> Ptr;
			Ptr data;
			Content(const Ptr& data) : data(data) { }
			virtual void print(std::ostream &os, uint32_t limit) { Print<T>::print(os, *data, limit); }
			virtual const char* name() { return Print<T>::name(); };
		};

		std::shared_ptr<BaseContent> content;
	public:
		template<typename T>
		static SharedValue make(const std::shared_ptr<T>& data) {
			SharedValue v;
			v.set(data);
			return v;
		}

		template<typename T>
		void set(const std::shared_ptr<T>& data) {
			content.reset(new Content<T>(data));
		}

		template<typename T>
		std::shared_ptr<T> tryGet() {
			std::shared_ptr< Content<T> > c(std::dynamic_pointer_cast< Content<T> >(content));
			if (!c) return std::shared_ptr<T>();
			return c->data;
		}

		template<typename T>
		std::shared_ptr<T> get() {
			std::shared_ptr<T> data = tryGet<T>();
			if (!data) throw ValueBadCastException("type mismatch");
			return data;
		}

		void print(std::ostream &os, uint32_t limit) {
			if (!content) os << "null"; else content->print(os, limit);
		}

		const char *name() {
			if (!content) return "null"; else return content->name();
		}
	};
	inline std::ostream& operator<<(std::ostream& os, SharedValue value) {
		value.print(os, -1);
		return os;
	}
}

#endif
