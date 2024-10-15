#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const height,
							  unsigned int const horizontal_split_count,
							  unsigned int const vertical_split_count)
{
	auto const vertices = std::array<glm::vec3, 4>{
		glm::vec3(0.0f, 0.0f, 0.0f),	// Bottom left
		glm::vec3(width, 0.0f, 0.0f),	// Bottom right
		glm::vec3(width, 0.0f, height), // Top rifht
		glm::vec3(0.0f, 0.0f, height)	// Top left
	};

	auto const index_sets = std::array<glm::uvec3, 2>{
		glm::uvec3(0u, 1u, 2u), // First triangle
		glm::uvec3(0u, 2u, 3u)	// Second triangle
	};

	bonobo::mesh_data data;

	if (horizontal_split_count > 0u || vertical_split_count > 0u)
	{
		auto const vertice_count = (horizontal_split_count + 2) * (vertical_split_count + 2);

		auto vertices = std::vector<glm::vec3>(vertice_count);
		auto texcoords = std::vector<glm::vec3>(vertice_count);

		float const d_horizontal = width / (static_cast<float>(horizontal_split_count + 1));
		float const d_vertical = height / (static_cast<float>(vertical_split_count + 1));

		// generate vertices iteratively
		size_t index = 0u;
		float hor = 0.0f;

		for (unsigned int i = 0u; i < horizontal_split_count + 2; ++i)
		{
			float ver = 0.0f;
			for (unsigned int j = 0u; j < vertical_split_count + 2; ++j)
			{
				// vertex
				vertices[index] = glm::vec3(hor, 0.0f, ver);

				// texture coordinates
				texcoords[index] = glm::vec3(static_cast<float>(i) / (static_cast<float>(horizontal_split_count)),
											 0.0f,
											 static_cast<float>(j) / (static_cast<float>(vertical_split_count)));

				++index;
				ver += d_vertical;
			}
			hor += d_horizontal;
		}

		// create index array
		auto index_sets = std::vector<glm::uvec3>(2u * (horizontal_split_count + 1) * (vertical_split_count + 1));

		// generate indices iteratively
		index = 0u;
		for (unsigned int i = 0u; i < horizontal_split_count + 1; ++i)
		{
			for (unsigned int j = 0u; j < vertical_split_count + 1; ++j)
			{
				index_sets[index] = glm::uvec3((horizontal_split_count + 2) * (i + 0u) + (j + 0u),
											   (horizontal_split_count + 2) * (i + 0u) + (j + 1u),
											   (horizontal_split_count + 2) * (i + 1u) + (j + 1u));
				++index;

				index_sets[index] = glm::uvec3((horizontal_split_count + 2) * (i + 0u) + (j + 0u),
											   (horizontal_split_count + 2) * (i + 1u) + (j + 1u),
											   (horizontal_split_count + 2) * (i + 1u) + (j + 0u));
				++index;
			}
		}

		bonobo::mesh_data data;
		glGenVertexArrays(1, &data.vao);
		assert(data.vao != 0u);
		glBindVertexArray(data.vao);

		auto const vertices_offset = 0u;
		auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
		auto const texcoords_offset = vertices_size;
		auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
		auto const bo_size = static_cast<GLsizeiptr>(vertices_size + texcoords_size);
		glGenBuffers(1, &data.bo);
		assert(data.bo != 0u);
		glBindBuffer(GL_ARRAY_BUFFER, data.bo);
		glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const *>(vertices.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

		glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const *>(texcoords.data()));
		glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
		glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(texcoords_offset));

		glBindBuffer(GL_ARRAY_BUFFER, 0u);

		data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
		glGenBuffers(1, &data.ibo);
		assert(data.ibo != 0u);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const *>(index_sets.data()), GL_STATIC_DRAW);

		glBindVertexArray(0u);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

		return data;
	}

	glGenVertexArrays(1, &data.vao);
	glBindVertexArray(data.vao);

	glGenBuffers(1, &data.bo);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));

	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices),
						  3,
						  /* what is the type of each component? */ GL_FLOAT,
						  /* should it automatically normalise the values stored */ GL_FALSE,
						  /* once all components of a vertex have been read, how far away (in bytes) is the next vertex? */ sizeof(glm::vec3),
						  /* how far away (in bytes) from the start of the buffer is the first vertex? */ reinterpret_cast<GLvoid const *>(0x0));
	glGenBuffers(1, &data.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_sets.size() * sizeof(glm::uvec3),
				 /* where is the data stored on the CPU? */ index_sets.data(),
				 /* inform OpenGL that the data is modified once, but used often */ GL_STATIC_DRAW);

	data.indices_nb = index_sets.size() * 3u;

	// All the data has been recorded, we can unbind them.
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data
parametric_shapes::createSphere(float const radius,
								unsigned int const minor_split_count,
								unsigned int const latitude_split_count)
{
	auto const vertice_count = (minor_split_count + 2) * (latitude_split_count + 2);

	auto vertices = std::vector<glm::vec3>(vertice_count);
	auto normals = std::vector<glm::vec3>(vertice_count);
	auto texcoords = std::vector<glm::vec3>(vertice_count);
	auto tangents = std::vector<glm::vec3>(vertice_count);
	auto binormals = std::vector<glm::vec3>(vertice_count);

	float const d_theta = glm::two_pi<float>() / (static_cast<float>(minor_split_count + 1));
	float const d_phi = glm::pi<float>() / (static_cast<float>(latitude_split_count + 1));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;

	for (unsigned int i = 0u; i < minor_split_count + 2; ++i)
	{
		float phi = 0.0f;
		for (unsigned int j = 0u; j < latitude_split_count + 2; ++j)
		{
			// vertex
			vertices[index] = glm::vec3(radius * std::sin(theta) * std::sin(phi),
										-radius * std::cos(phi),
										radius * std::cos(theta) * std::sin(phi));

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(i) / (static_cast<float>(latitude_split_count)),
										 static_cast<float>(j) / (static_cast<float>(minor_split_count)),
										 0.0f);

			// tangent
			auto const t = glm::vec3(radius * cos(theta) /** sin(phi)*/,
									 0,
									 -radius * sin(theta) /** sin(phi)*/);
			tangents[index] = normalize(t);

			// binormal
			auto const b = glm::vec3(radius * sin(theta) * cos(phi),
									 radius * sin(phi),
									 radius * cos(theta) * cos(phi));
			binormals[index] = glm::normalize(b);

			// normal
			auto const n = glm::normalize(glm::cross(t, b));
			normals[index] = n;

			++index;
			phi += d_phi;
		}
		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * (minor_split_count + 1) * (latitude_split_count + 1));

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < minor_split_count + 1; ++i)
	{
		for (unsigned int j = 0u; j < latitude_split_count + 1; ++j)
		{
			index_sets[index] = glm::uvec3((minor_split_count + 2) * (i + 0u) + (j + 0u),
										   (minor_split_count + 2) * (i + 0u) + (j + 1u),
										   (minor_split_count + 2) * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3((minor_split_count + 2) * (i + 0u) + (j + 0u),
										   (minor_split_count + 2) * (i + 1u) + (j + 1u),
										   (minor_split_count + 2) * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size + tangents_size + binormals_size);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const *>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const *>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const *>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const *>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const *>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const *>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}

bonobo::mesh_data 
parametric_shapes::createTorus(float const major_radius, float const minor_radius, unsigned int const major_split_count, unsigned int const minor_split_count)
{
    // Number of vertices to generate
    auto const vertice_count = (major_split_count + 1) * (minor_split_count + 1);

    // Initialize vertex-related buffers
    auto vertices = std::vector<glm::vec3>(vertice_count);
    auto normals = std::vector<glm::vec3>(vertice_count);
    auto texcoords = std::vector<glm::vec2>(vertice_count);
    auto tangents = std::vector<glm::vec3>(vertice_count);
    auto binormals = std::vector<glm::vec3>(vertice_count);

    // Step sizes for parametric coordinates
    float const d_theta = glm::two_pi<float>() / static_cast<float>(major_split_count);
    float const d_phi = glm::two_pi<float>() / static_cast<float>(minor_split_count);

    // Generate vertices iteratively
    size_t index = 0u;
    for (unsigned int i = 0u; i <= major_split_count; ++i) {
        float theta = i * d_theta;  // Angle around the major circle
        glm::vec3 major_center = glm::vec3(major_radius * cos(theta), 0.0f, major_radius * sin(theta));

        for (unsigned int j = 0u; j <= minor_split_count; ++j) {
            float phi = j * d_phi;  // Angle around the minor circle

            // Vertex position in 3D space
            glm::vec3 normal = glm::vec3(cos(theta) * cos(phi), sin(phi), sin(theta) * cos(phi));
            vertices[index] = major_center + normal * minor_radius;

            // Normal is the direction from the center of the tube
            normals[index] = glm::normalize(normal);

            // Tangent is the direction along the major circle
            tangents[index] = glm::normalize(glm::vec3(-sin(theta), 0.0f, cos(theta)));

            // Binormal is the direction along the minor circle
            binormals[index] = glm::normalize(glm::cross(tangents[index], normals[index]));

            // Texture coordinates
            texcoords[index] = glm::vec2(static_cast<float>(i) / static_cast<float>(major_split_count),
                                         static_cast<float>(j) / static_cast<float>(minor_split_count));

            ++index;
        }
    }

    // Generate indices for the torus
    auto index_sets = std::vector<glm::uvec3>(2u * major_split_count * minor_split_count);
    index = 0u;
    for (unsigned int i = 0u; i < major_split_count; ++i) {
        for (unsigned int j = 0u; j < minor_split_count; ++j) {
            unsigned int first = i * (minor_split_count + 1) + j;
            unsigned int second = first + minor_split_count + 1;

            // First triangle of the quad
            index_sets[index] = glm::uvec3(first, second, first + 1);
            ++index;

            // Second triangle of the quad
            index_sets[index] = glm::uvec3(first + 1, second, second + 1);
            ++index;
        }
    }

    // Create and bind VAO
    bonobo::mesh_data data;
    glGenVertexArrays(1, &data.vao);
    assert(data.vao != 0u);
    glBindVertexArray(data.vao);

    // Calculate buffer sizes
    auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
    auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
    auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec2));
    auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
    auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
    auto const bo_size = vertices_size + normals_size + texcoords_size + tangents_size + binormals_size;

    // Create and bind buffer objects
    glGenBuffers(1, &data.bo);
    assert(data.bo != 0u);
    glBindBuffer(GL_ARRAY_BUFFER, data.bo);
    glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

    // Upload vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices_size, vertices.data());
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));

    // Upload normal data
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size, normals_size, normals.data());
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(vertices_size));

    // Upload texture coordinate data
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size, texcoords_size, texcoords.data());
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(vertices_size + normals_size));

    // Upload tangent data
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size + texcoords_size, tangents_size, tangents.data());
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(vertices_size + normals_size + texcoords_size));

    // Upload binormal data
    glBufferSubData(GL_ARRAY_BUFFER, vertices_size + normals_size + texcoords_size + tangents_size, binormals_size, binormals.data());
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(vertices_size + normals_size + texcoords_size + tangents_size));

    glBindBuffer(GL_ARRAY_BUFFER, 0u);

    // Create and bind index buffer
    glGenBuffers(1, &data.ibo);
    assert(data.ibo != 0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), index_sets.data(), GL_STATIC_DRAW);

    glBindVertexArray(0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

    data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);

    return data;
}


bonobo::mesh_data
parametric_shapes::createCircleRing(float const radius,
									float const spread_length,
									unsigned int const circle_split_count,
									unsigned int const spread_split_count)
{
	auto const circle_slice_edges_count = circle_split_count + 1u;
	auto const spread_slice_edges_count = spread_split_count + 1u;
	auto const circle_slice_vertices_count = circle_slice_edges_count + 1u;
	auto const spread_slice_vertices_count = spread_slice_edges_count + 1u;
	auto const vertices_nb = circle_slice_vertices_count * spread_slice_vertices_count;

	auto vertices = std::vector<glm::vec3>(vertices_nb);
	auto normals = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i)
	{
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j)
		{
			// vertex
			vertices[index] = glm::vec3(distance_to_centre * cos_theta,
										distance_to_centre * sin_theta,
										0.0f);

			// texture coordinates
			texcoords[index] = glm::vec3(static_cast<float>(j) / (static_cast<float>(spread_slice_vertices_count)),
										 static_cast<float>(i) / (static_cast<float>(circle_slice_vertices_count)),
										 0.0f);

			// tangent
			auto const t = glm::vec3(cos_theta, sin_theta, 0.0f);
			tangents[index] = t;

			// binormal
			auto const b = glm::vec3(-sin_theta, cos_theta, 0.0f);
			binormals[index] = b;

			// normal
			auto const n = glm::cross(t, b);
			normals[index] = n;

			distance_to_centre += d_spread;
			++index;
		}

		theta += d_theta;
	}

	// create index array
	auto index_sets = std::vector<glm::uvec3>(2u * circle_slice_edges_count * spread_slice_edges_count);

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < circle_slice_edges_count; ++i)
	{
		for (unsigned int j = 0u; j < spread_slice_edges_count; ++j)
		{
			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
										   spread_slice_vertices_count * (i + 0u) + (j + 1u),
										   spread_slice_vertices_count * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3(spread_slice_vertices_count * (i + 0u) + (j + 0u),
										   spread_slice_vertices_count * (i + 1u) + (j + 1u),
										   spread_slice_vertices_count * (i + 1u) + (j + 0u));
			++index;
		}
	}

	bonobo::mesh_data data;
	glGenVertexArrays(1, &data.vao);
	assert(data.vao != 0u);
	glBindVertexArray(data.vao);

	auto const vertices_offset = 0u;
	auto const vertices_size = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec3));
	auto const normals_offset = vertices_size;
	auto const normals_size = static_cast<GLsizeiptr>(normals.size() * sizeof(glm::vec3));
	auto const texcoords_offset = normals_offset + normals_size;
	auto const texcoords_size = static_cast<GLsizeiptr>(texcoords.size() * sizeof(glm::vec3));
	auto const tangents_offset = texcoords_offset + texcoords_size;
	auto const tangents_size = static_cast<GLsizeiptr>(tangents.size() * sizeof(glm::vec3));
	auto const binormals_offset = tangents_offset + tangents_size;
	auto const binormals_size = static_cast<GLsizeiptr>(binormals.size() * sizeof(glm::vec3));
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size + normals_size + texcoords_size + tangents_size + binormals_size);
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const *>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const *>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const *>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const *>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const *>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const *>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const *>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
