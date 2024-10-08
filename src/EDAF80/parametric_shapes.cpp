#include "parametric_shapes.hpp"
#include "core/Log.h"

#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

bonobo::mesh_data
parametric_shapes::createQuad(float const width, float const depth,
							  unsigned int const horizontal_split_count,
							  unsigned int const vertical_split_count)
{
	bonobo::mesh_data data;

	// Total number of vertices based on splits
	unsigned int num_vertices_x = horizontal_split_count + 1;
	unsigned int num_vertices_z = vertical_split_count + 1;

	// Calculate the step size for x and z directions
	float step_x = width / horizontal_split_count;
	float step_z = depth / vertical_split_count;

	// Vectors to store vertices and texture coordinates
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::uvec3> indices;

	// Generate vertices and texture coordinates
	for (unsigned int z = 0; z <= vertical_split_count; ++z)
	{
		for (unsigned int x = 0; x <= horizontal_split_count; ++x)
		{
			// Position in x-z plane
			float pos_x = x * step_x;
			float pos_z = z * step_z;
			vertices.emplace_back(pos_x, 0.0f, pos_z); // Vertex in the x-z plane

			// Texture coordinates (normalized from 0 to 1)
			float tex_u = static_cast<float>(x) / horizontal_split_count;
			float tex_v = static_cast<float>(z) / vertical_split_count;
			texcoords.emplace_back(tex_u, tex_v);
		}
	}

	// Generate indices for two triangles per quad
	for (unsigned int z = 0; z < vertical_split_count; ++z)
	{
		for (unsigned int x = 0; x < horizontal_split_count; ++x)
		{
			unsigned int topLeft = z * num_vertices_x + x;
			unsigned int topRight = topLeft + 1;
			unsigned int bottomLeft = (z + 1) * num_vertices_x + x;
			unsigned int bottomRight = bottomLeft + 1;

			// First triangle (top-left, bottom-left, bottom-right)
			indices.emplace_back(topLeft, bottomLeft, bottomRight);

			// Second triangle (top-left, bottom-right, top-right)
			indices.emplace_back(topLeft, bottomRight, topRight);
		}
	}

	// Create and bind VAO
	glGenVertexArrays(1, &data.vao);
	glBindVertexArray(data.vao);

	// Create and bind VBO for vertex positions
	glGenBuffers(1, &data.bo);
	glBindBuffer(GL_ARRAY_BUFFER, data.bo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	// Set vertex attribute pointer for position
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::vertices));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::vertices),
						  3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), reinterpret_cast<GLvoid const *>(0x0));

	// Create and bind VBO for texture coordinates
	GLuint tex_vbo;
	glGenBuffers(1, &tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);

	// Set vertex attribute pointer for texture coordinates
	glEnableVertexAttribArray(static_cast<unsigned int>(bonobo::shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<unsigned int>(bonobo::shader_bindings::texcoords),
						  2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), reinterpret_cast<GLvoid const *>(0x0));

	// Create and bind EBO for indices
	glGenBuffers(1, &data.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), indices.data(), GL_STATIC_DRAW);

	// Set the number of indices
	data.indices_nb = indices.size() * 3u;

	// Unbind VAO, VBO, and EBO
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
	auto const vertice_count = (longitude_split_count + 2) * (latitude_split_count + 2);

	auto vertices = std::vector<glm::vec3>(vertice_count);
	auto normals = std::vector<glm::vec3>(vertice_count);
	auto texcoords = std::vector<glm::vec3>(vertice_count);
	auto tangents = std::vector<glm::vec3>(vertice_count);
	auto binormals = std::vector<glm::vec3>(vertice_count);

	float const d_theta = glm::two_pi<float>() / (static_cast<float>(longitude_split_count + 1));
	float const d_phi = glm::pi<float>() / (static_cast<float>(latitude_split_count + 1));

	// generate vertices iteratively
	size_t index = 0u;
	float theta = 0.0f;

	for (unsigned int i = 0u; i < longitude_split_count + 2; ++i)
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
										 static_cast<float>(j) / (static_cast<float>(longitude_split_count)),
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
	auto index_sets = std::vector<glm::uvec3>(2u * (longitude_split_count + 1) * (latitude_split_count + 1));

	// generate indices iteratively
	index = 0u;
	for (unsigned int i = 0u; i < longitude_split_count + 1; ++i)
	{
		for (unsigned int j = 0u; j < latitude_split_count + 1; ++j)
		{
			index_sets[index] = glm::uvec3((longitude_split_count + 2) * (i + 0u) + (j + 0u),
										   (longitude_split_count + 2) * (i + 0u) + (j + 1u),
										   (longitude_split_count + 2) * (i + 1u) + (j + 1u));
			++index;

			index_sets[index] = glm::uvec3((longitude_split_count + 2) * (i + 0u) + (j + 0u),
										   (longitude_split_count + 2) * (i + 1u) + (j + 1u),
										   (longitude_split_count + 2) * (i + 1u) + (j + 0u));
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
	for (unsigned int i = 0; i <= major_split_count; ++i)
	{
		for (unsigned int j = 0; j <= minor_split_count; ++j)
		{
			float const u = i * d_major;
			float const v = j * d_minor;

			// Compute the vertex position
			glm::vec3 position = glm::vec3(
				(major_radius + minor_radius * cos(v)) * cos(u), // x-coordinate
				(major_radius + minor_radius * cos(v)) * sin(u), // y-coordinate
				minor_radius * sin(v)							 // z-coordinate
			);

			// Compute the normal vector
			glm::vec3 normal = glm::normalize(glm::vec3(
				cos(v) * cos(u),
				cos(v) * sin(u),
				sin(v)));

			// Compute the tangent vector (partial derivative with respect to u)
			glm::vec3 tangent = glm::normalize(glm::vec3(
				-sin(u), cos(u), 0.0f));

			// Compute the binormal vector (partial derivative with respect to v)
			glm::vec3 binormal = glm::normalize(glm::cross(normal, tangent));

			// Compute the texture coordinates
			glm::vec2 texcoord = glm::vec2(
				static_cast<float>(i) / major_split_count, // u-coordinate
				static_cast<float>(j) / minor_split_count  // v-coordinate
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
	for (unsigned int i = 0; i < major_split_count; ++i)
	{
		for (unsigned int j = 0; j < minor_split_count; ++j)
		{
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
