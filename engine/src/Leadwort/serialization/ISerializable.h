#pragma once

#include <nlohmann/json.hpp>

namespace Leadwort {
	using Json = nlohmann::json;
}

namespace Leadwort::Serialization {

	class ISerializable {
	public:
		virtual ~ISerializable() = default;
		virtual void Serialize(Json& out) const = 0;
		virtual void Deserialize(const Json& in) = 0;
		virtual std::string_view GetTypeName() const = 0;
	};

}