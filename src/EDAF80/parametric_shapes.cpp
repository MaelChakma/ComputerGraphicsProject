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
		glm::vec3(0.0f,  0.0f,   0.0f), //Bottom left
		glm::vec3(width, 0.0f,   0.0f), //Bottom right
		glm::vec3(width, height, 0.0f), // Top rifht
		glm::vec3(0.0f,  height, 0.0f) // Top left
	};

	auto const index_sets = std::array<glm::uvec3, 2>{
		glm::uvec3(0u, 1u, 2u), //First triangle
		glm::uvec3(0u, 2u, 3u) // Second triangle
	};

	bonobo::mesh_data data;

	if (horizontal_split_count > 0u || vertical_split_count > 0u)
	{
		LogError("parametric_shapes::createQuad() does not support tesselation.");
		return data;
	}

	glGenVertexArrays(1, &data.vao);
	glBindVertexArray(data.vao);

	glGenBuffers(1, &data.bo);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3)),
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));

	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices),
	                      3,
	                      /* what is the type of each component? */GL_FLOAT,
	                      /* should it automatically normalise the values stored */GL_FALSE,
	                      /* once all components of a vertex have been read, how far away (in bytes) is the next vertex? */ sizeof(glm::vec3),
	                      /* how far away (in bytes) from the start of the buffer is the first vertex? */reinterpret_cast<GLvoid const*>(0x0));
	glGenBuffers(1, &data.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_sets.size() * sizeof(glm::uvec3),
	             /* where is the data stored on the CPU? */index_sets.data(),
	             /* inform OpenGL that the data is modified once, but used often */GL_STATIC_DRAW);

	data.indices_nb = index_sets.size() * 3u;

	// All the data has been recorded, we can unbind them.
	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
bonobo::mesh_data
parametric_shapes::createSphere(float const radius, 
								unsigned int const longitude_split_count, 
								unsigned int const latitude_split_count)
{
    bonobo::mesh_data data;

    // Number of vertices (for a UV sphere)
    unsigned int const vertex_count = (latitude_split_count + 1) * (longitude_split_count + 1);
    std::vector<glm::vec3> positions(vertex_count);
    std::vector<glm::vec3> normals(vertex_count);
    std::vector<glm::vec3> tangents(vertex_count);
    std::vector<glm::vec3> binormals(vertex_count);
    std::vector<glm::vec2> texcoords(vertex_count);

    float const d_theta = glm::pi<float>() / latitude_split_count;     // Step size in theta (polar angle)
    float const d_phi = glm::two_pi<float>() / longitude_split_count;  // Step size in phi

    // Generate vertices, normals, tangents, and texture coordinates
    for (unsigned int i = 0; i <= latitude_split_count; ++i) {
        for (unsigned int j = 0; j <= longitude_split_count; ++j) {
            float const theta = i * d_theta;
            float const phi = j * d_phi;

            glm::vec3 position = glm::vec3(
                radius * sin(theta) * cos(phi),  // x
                radius * cos(theta),             // y
                radius * sin(theta) * sin(phi)   // z
            );
            glm::vec3 normal = glm::normalize(position);  // Normalize the position for the normal
            glm::vec3 tangent = glm::vec3(-sin(phi), 0.0f, cos(phi));  // Tangent vector (partial derivative w.r.t. phi)
            glm::vec3 binormal = glm::cross(normal, tangent);          // Binormal vector (cross product of normal and tangent)

            positions[i * (longitude_split_count + 1) + j] = position;
            normals[i * (longitude_split_count + 1) + j] = normal;
            tangents[i * (longitude_split_count + 1) + j] = glm::normalize(tangent);
            binormals[i * (longitude_split_count + 1) + j] = glm::normalize(binormal);
            texcoords[i * (longitude_split_count + 1) + j] = glm::vec2(static_cast<float>(j) / longitude_split_count, static_cast<float>(i) / latitude_split_count);
        }
    }

    // Generate indices
    std::vector<glm::uvec3> indices;
    for (unsigned int i = 0; i < latitude_split_count; ++i) {
        for (unsigned int j = 0; j < longitude_split_count; ++j) {
            unsigned int first = i * (longitude_split_count + 1) + j;
            unsigned int second = first + longitude_split_count + 1;

            // First triangle
            indices.emplace_back(first, second, first + 1);
            // Second triangle
            indices.emplace_back(second, second + 1, first + 1);
        }
    }

    // Create and bind VAO
    glGenVertexArrays(1, &data.vao);
    glBindVertexArray(data.vao);

    // Create and bind VBO for positions
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for normals
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for tangents
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), tangents.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for binormals
    GLuint bbo;
    glGenBuffers(1, &bbo);
    glBindBuffer(GL_ARRAY_BUFFER, bbo);
    glBufferData(GL_ARRAY_BUFFER, binormals.size() * sizeof(glm::vec3), binormals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for texture coordinates
    GLuint tcb;
    glGenBuffers(1, &tcb);
    glBindBuffer(GL_ARRAY_BUFFER, tcb);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind EBO for indices
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);

    // Set the number of indices for rendering
    data.indices_nb = static_cast<GLsizei>(indices.size() * 3);

    // Unbind VAO, VBO, and EBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return data;
}

bonobo::mesh_data
parametric_shapes::createTorus(float const major_radius,
                               float const minor_radius,
                               unsigned int const major_split_count,
                               unsigned int const minor_split_count)
{
    bonobo::mesh_data data;

    // Number of vertices
    unsigned int const vertex_count = (major_split_count + 1) * (minor_split_count + 1);
    std::vector<glm::vec3> positions(vertex_count);
    std::vector<glm::vec3> normals(vertex_count);
    std::vector<glm::vec3> tangents(vertex_count);
    std::vector<glm::vec3> binormals(vertex_count);
    std::vector<glm::vec2> texcoords(vertex_count);

    // Step in angles
    float const d_major = glm::two_pi<float>() / major_split_count; // Step size in u (around the major axis)
    float const d_minor = glm::two_pi<float>() / minor_split_count; // Step size in v (around the minor circle)

    // Generate vertices, normals, tangents, and texture coordinates
    for (unsigned int i = 0; i <= major_split_count; ++i) {
        for (unsigned int j = 0; j <= minor_split_count; ++j) {
            float const u = i * d_major;
            float const v = j * d_minor;

            // Compute the vertex position
            glm::vec3 position = glm::vec3(
                (major_radius + minor_radius * cos(v)) * cos(u),  // x-coordinate
                (major_radius + minor_radius * cos(v)) * sin(u),  // y-coordinate
                minor_radius * sin(v)                             // z-coordinate
            );

            // Compute the normal vector
            glm::vec3 normal = glm::normalize(glm::vec3(
                cos(v) * cos(u),
                cos(v) * sin(u),
                sin(v)
            ));

            // Compute the tangent vector (partial derivative with respect to u)
            glm::vec3 tangent = glm::normalize(glm::vec3(
                -sin(u), cos(u), 0.0f
            ));

            // Compute the binormal vector (partial derivative with respect to v)
            glm::vec3 binormal = glm::normalize(glm::cross(normal, tangent));

            // Compute the texture coordinates
            glm::vec2 texcoord = glm::vec2(
                static_cast<float>(i) / major_split_count,   // u-coordinate
                static_cast<float>(j) / minor_split_count    // v-coordinate
            );

            positions[i * (minor_split_count + 1) + j] = position;
            normals[i * (minor_split_count + 1) + j] = normal;
            tangents[i * (minor_split_count + 1) + j] = tangent;
            binormals[i * (minor_split_count + 1) + j] = binormal;
            texcoords[i * (minor_split_count + 1) + j] = texcoord;
        }
    }

    // Generate indices
    std::vector<glm::uvec3> indices;
    for (unsigned int i = 0; i < major_split_count; ++i) {
        for (unsigned int j = 0; j < minor_split_count; ++j) {
            unsigned int first = i * (minor_split_count + 1) + j;
            unsigned int second = first + minor_split_count + 1;

            // First triangle
            indices.emplace_back(first, second, first + 1);
            // Second triangle
            indices.emplace_back(second, second + 1, first + 1);
        }
    }

    // Create and bind VAO
    glGenVertexArrays(1, &data.vao);
    glBindVertexArray(data.vao);

    // Create and bind VBO for positions
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for normals
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for tangents
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), tangents.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for binormals
    GLuint bbo;
    glGenBuffers(1, &bbo);
    glBindBuffer(GL_ARRAY_BUFFER, bbo);
    glBufferData(GL_ARRAY_BUFFER, binormals.size() * sizeof(glm::vec3), binormals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind VBO for texture coordinates
    GLuint tcb;
    glGenBuffers(1, &tcb);
    glBindBuffer(GL_ARRAY_BUFFER, tcb);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
    glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Create and bind EBO for indices
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);

    // Set the number of indices for rendering
    data.indices_nb = static_cast<GLsizei>(indices.size() * 3);

    // Unbind VAO, VBO, and EBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

	auto vertices  = std::vector<glm::vec3>(vertices_nb);
	auto normals   = std::vector<glm::vec3>(vertices_nb);
	auto texcoords = std::vector<glm::vec3>(vertices_nb);
	auto tangents  = std::vector<glm::vec3>(vertices_nb);
	auto binormals = std::vector<glm::vec3>(vertices_nb);

	float const spread_start = radius - 0.5f * spread_length;
	float const d_theta = glm::two_pi<float>() / (static_cast<float>(circle_slice_edges_count));
	float const d_spread = spread_length / (static_cast<float>(spread_slice_edges_count));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;
	for (unsigned int i = 0u; i < circle_slice_vertices_count; ++i) {
		float const cos_theta = std::cos(theta);
		float const sin_theta = std::sin(theta);

		float distance_to_centre = spread_start;
		for (unsigned int j = 0u; j < spread_slice_vertices_count; ++j) {
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
	auto const bo_size = static_cast<GLsizeiptr>(vertices_size
	                                            +normals_size
	                                            +texcoords_size
	                                            +tangents_size
	                                            +binormals_size
	                                            );
	glGenBuffers(1, &data.bo);
	assert(data.bo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, bo_size, nullptr, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, vertices_offset, vertices_size, static_cast<GLvoid const*>(vertices.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBufferSubData(GL_ARRAY_BUFFER, normals_offset, normals_size, static_cast<GLvoid const*>(normals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::normals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::normals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(normals_offset));

	glBufferSubData(GL_ARRAY_BUFFER, texcoords_offset, texcoords_size, static_cast<GLvoid const*>(texcoords.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(texcoords_offset));

	glBufferSubData(GL_ARRAY_BUFFER, tangents_offset, tangents_size, static_cast<GLvoid const*>(tangents.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::tangents));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::tangents), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(tangents_offset));

	glBufferSubData(GL_ARRAY_BUFFER, binormals_offset, binormals_size, static_cast<GLvoid const*>(binormals.data()));
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::binormals));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::binormals), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(binormals_offset));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	data.indices_nb = static_cast<GLsizei>(index_sets.size() * 3u);
	glGenBuffers(1, &data.ibo);
	assert(data.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_sets.size() * sizeof(glm::uvec3)), reinterpret_cast<GLvoid const*>(index_sets.data()), GL_STATIC_DRAW);

	glBindVertexArray(0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return data;
}
