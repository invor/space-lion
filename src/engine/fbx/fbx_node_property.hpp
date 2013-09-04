#ifndef _FBX_NODE_PROPERTY_H
#define _FBX_NODE_PROPERTY_H _FBX_NODE_PROPERTY_H

#include "fbx_types.hpp"
#include "fbx_value.hpp"

#include <iostream>

namespace FBX {
	class NodeProperty {
	public:
		enum Type {
			FLOAT,
			DOUBLE,
			SINT64,
			SINT32,
			SINT16,
			BOOLEAN,
			STRING,
			RAW
		};

	private:
		Type m_type;
		bool m_isArray;
		Value m_value;

	public:
		/* default to FALSE */
		explicit NodeProperty() : m_type(BOOLEAN), m_isArray(false), m_value(Value::make<bool>(false)) { }

		explicit NodeProperty(float f) : m_type(FLOAT), m_isArray(false), m_value(Value::make<float>(f)) { }
		explicit NodeProperty(const std::vector<float>& f) : m_type(FLOAT), m_isArray(true), m_value(Value::make<std::vector<float> >(f)) { }
		explicit NodeProperty(double d) : m_type(DOUBLE), m_isArray(false), m_value(Value::make<double>(d)) { }
		explicit NodeProperty(const std::vector<double>& d) : m_type(DOUBLE), m_isArray(true), m_value(Value::make<std::vector<double> >(d)) { }
		explicit NodeProperty(int64_t i) : m_type(SINT64), m_isArray(false), m_value(Value::make<int64_t>(i)) { }
		explicit NodeProperty(const std::vector<int64_t>& i) : m_type(SINT64), m_isArray(true), m_value(Value::make<std::vector<int64_t> >(i)) { }
		explicit NodeProperty(int32_t i) : m_type(SINT32), m_isArray(false), m_value(Value::make<int32_t>(i)) { }
		explicit NodeProperty(const std::vector<int32_t>& i) : m_type(SINT32), m_isArray(true), m_value(Value::make<std::vector<int32_t> >(i)) { }
		explicit NodeProperty(int16_t i) : m_type(SINT16), m_isArray(false), m_value(Value::make<int16_t>(i)) { }
		explicit NodeProperty(const std::vector<int16_t>& i) : m_type(SINT16), m_isArray(true), m_value(Value::make<std::vector<int16_t> >(i)) { }
		explicit NodeProperty(bool b) : m_type(BOOLEAN), m_isArray(false), m_value(Value::make<bool>(b)) { }
		explicit NodeProperty(const std::vector<bool>& b) : m_type(BOOLEAN), m_isArray(true), m_value(Value::make<std::vector<bool> >(b)) { }

		explicit NodeProperty(std::string s) : m_type(STRING), m_isArray(false), m_value(Value::make<std::string>(s)) { }
		explicit NodeProperty(ByteVector r) : m_type(RAW), m_isArray(false), m_value(Value::make<ByteVector>(r)) { }

		Type type() const { return m_type; }
		bool isArray() const { return m_isArray; }
		Value value() const { return m_value; }

		float                 getFloat()   const { return m_value.get<float>(); }
		std::vector<float>&   getFloats()  const { return m_value.get<std::vector<float> >(); }
		double                getDouble()  const { return m_value.get<double>(); }
		std::vector<double>&  getDoubles() const { return m_value.get<std::vector<double> >(); }
		int64_t               getInt64()   const { return m_value.get<int64_t>(); }
		std::vector<int64_t>& getInt64s()  const { return m_value.get<std::vector<int64_t> >(); }
		int32_t               getInt32()   const { return m_value.get<int32_t>(); }
		std::vector<int32_t>& getInt32s()  const { return m_value.get<std::vector<int32_t> >(); }
		int16_t               getInt16()   const { return m_value.get<int16_t>(); }
		std::vector<int16_t>& getInt16s()  const { return m_value.get<std::vector<int16_t> >(); }
		bool                  getBool()    const { return m_value.get<bool>(); }
		std::vector<bool>&    getBools()   const { return m_value.get<std::vector<bool> >(); }
		std::string&          getString()  const { return m_value.get<std::string>(); }
		ByteVector&           getRaw()     const { return m_value.get<ByteVector>(); }
	};

	inline std::ostream& operator<<(std::ostream& os, const NodeProperty &prop) {
		prop.value().print(os, -1);
		os << " :: " << prop.value().name();
		return os;
	}
}

#endif
