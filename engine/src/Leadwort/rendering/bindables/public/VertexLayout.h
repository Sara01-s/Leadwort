#pragma once

#include "IBindable.h"
#include <cstdint>
#include <glad/glad.h>
#include <utility>
#include <glm/glm.hpp>
#include <vector>

namespace Leadwort::Rendering::Bindables {

	enum class ElementType {
	    Position3D,
	    Normal3D,
	    TexCoord2D,
	    Tangent3D,
	    Bitangent3D,
	    ColorRGBA
	};

	struct VertexAttribute {
	    GLint location;
	    GLint count;
	    GLenum type;
	    GLboolean normalized;
	    std::uint32_t sizeInBytes;

	    void Apply(const int stride, const void* offset) const {
	        glEnableVertexAttribArray(location);

	        if (type == GL_INT || type == GL_UNSIGNED_INT) {
	            glVertexAttribIPointer(location, count, type, stride, offset);
	        }
	        else {
	            glVertexAttribPointer(location, count, type, normalized, stride, offset);
	        }
	    }
	};

	struct ElementTraits {
	    GLint count;
	    GLenum type;
	    GLboolean normalized;
	    std::uint32_t size;
	};

	inline ElementTraits GetTraits(const ElementType type) {
	    switch (type) {
	        case ElementType::Position3D:
	        case ElementType::Normal3D:
	        case ElementType::Tangent3D:
	        case ElementType::Bitangent3D: return {3, GL_FLOAT, GL_FALSE, sizeof(Vec3)};
	        case ElementType::TexCoord2D:  return {2, GL_FLOAT, GL_FALSE, sizeof(Vec2)};
	        case ElementType::ColorRGBA:   return {4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(glm::u8vec4)};
	        default:                       return {0, 0, GL_FALSE, 0};
	    }
	}

	class VertexLayout : public IBindable {
	public:
	    VertexLayout() = default;

	    // Copyable, unlike the rest of the bindables: a layout is a pure CPU description
	    // of the vertex format and owns no GPU object (its m_GpuID stays 0), so the base's
	    // deleted copy does not apply. MeshData holds one by value, which is what keeps a
	    // Mesh from outliving the layout it was built from.
	    VertexLayout(const VertexLayout& other)
	        : IBindable(), m_Attributes(other.m_Attributes), m_Stride(other.m_Stride), m_NextLocation(other.m_NextLocation) {}

	    VertexLayout(VertexLayout&& other) noexcept
	        : IBindable(), m_Attributes(std::move(other.m_Attributes)), m_Stride(other.m_Stride), m_NextLocation(other.m_NextLocation) {}

	    VertexLayout& operator=(const VertexLayout& other) {
	        m_Attributes = other.m_Attributes;
	        m_Stride = other.m_Stride;
	        m_NextLocation = other.m_NextLocation;

	        return *this;
	    }

	    VertexLayout& operator=(VertexLayout&& other) noexcept {
	        m_Attributes = std::move(other.m_Attributes);
	        m_Stride = other.m_Stride;
	        m_NextLocation = other.m_NextLocation;

	        return *this;
	    }

	    VertexLayout& Append(const ElementType type) {
	        const auto traits = GetTraits(type);

	        const VertexAttribute attribute {
	            m_NextLocation++,
	            traits.count,
	            traits.type,
	            traits.normalized,
	            traits.size
	        };

	        m_Attributes.push_back(attribute);
	        m_Stride += traits.size;

	        return *this;
	    }

	    void Bind() const noexcept override {
	        const auto stride = static_cast<GLsizei>(m_Stride);
	        std::uintptr_t offset = 0;

	        for (const auto& attribute : m_Attributes) {
	            attribute.Apply(stride, reinterpret_cast<const void*>(offset));
	            offset += attribute.sizeInBytes;
	        }
	    }

	    void Unbind() const noexcept override {
	        for (const auto& attribute : m_Attributes) {
	            glDisableVertexAttribArray(attribute.location);
	        }
	    }

	    [[nodiscard]] const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }
	    [[nodiscard]] std::uint32_t GetStride() const { return m_Stride; }

	private:
	    std::vector<VertexAttribute> m_Attributes;
	    std::uint32_t m_Stride{0};
	    GLint m_NextLocation{0};
	};

} // namespace Engine::Rendering::Bindables