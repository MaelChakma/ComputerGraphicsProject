#include "assignment5.hpp"
#include "parametric_shapes.cpp"
#include "random"
#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <clocale>
#include <stdexcept>

edaf80::Assignment5::Assignment5(WindowManager &windowManager) : mCamera(0.5f * glm::half_pi<float>(),
																		 static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
																		 0.01f, 1000.0f),
																 inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr)
	{
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}

bool checkCollision(glm::vec3 pos1, glm::vec3 pos2, float rad1, float rad2)

{
	bool colision = distance(pos1, pos2) < rad1 + rad2;
	return colision;
}

void edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();
	float elapsed_time_s = 0.0f;

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
											 {{ShaderType::vertex, "common/fallback.vert"},
											  {ShaderType::fragment, "common/fallback.frag"}},
											 fallback_shader);
	if (fallback_shader == 0u)
	{
		LogError("Failed to load fallback shader");
		return;
	}
	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
											 {{ShaderType::vertex, "EDAF80/phong.vert"},
											  {ShaderType::fragment, "EDAF80/phong.frag"}},
											 phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load phong shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
											 {{ShaderType::vertex, "EDAF80/skybox.vert"},
											  {ShaderType::fragment, "EDAF80/skybox.frag"}},
											 skybox_shader);

	auto skybox_shape = parametric_shapes::createSphere(75.0f, 1000u, 1000u);
	auto astroid_ = parametric_shapes::createSphere(1.0f, 100u, 100u);
	auto spaceship_inside_ = parametric_shapes::createSphere(0.50f, 100u, 100u);
	auto spaceship_outside_ = parametric_shapes::createTorus(0.50f, 0.20f, 100u, 100u);
	glm::vec3 camera_offset = glm::vec3(0.0f, -1.50, -6.0);

	bool use_normal_mapping = true;
	auto light_position = glm::vec3(200.0f, 200.0f, -200.0f);

	// Random number generation setup for asteroid positions
	std::random_device rd;	// Seed generator
	std::mt19937 gen(rd()); // Random number generator
	std::uniform_real_distribution<float> dis(-50.0f, 50.0f);
	std::uniform_real_distribution<float> dis_distance(-1.0f, 1.0f); // Range for asteroid placement within skybox

	// Create multiple asteroids with random positions
	const int asteroid_count = 250; // Number of asteroids
	std::vector<Node> asteroids;
	std::vector<glm::vec3> asteroid_positions;
	std::vector<glm::vec3> asteroid_velocities; // Store velocities for each asteroid
	std::vector<glm::vec3> asteroid_directions;

	float asteroid_speed = 0.1f; // Speed factor for asteroid movement

	for (int i = 0; i < asteroid_count; ++i)
	{
		Node asteroid;
		asteroid.set_geometry(astroid_); // Assuming `astroid_` is your asteroid model

		// Generate random position within the skybox
		glm::vec3 position = glm::vec3(dis(gen), dis(gen), dis(gen));
		asteroid_positions.push_back(position);
		asteroid.get_transform().SetTranslate(position);

		// Initial velocity is set to zero, will be updated to point towards the spaceship
		asteroid_velocities.push_back(glm::vec3(0.0f));

		// Add asteroid node to vector
		asteroids.push_back(asteroid);
		glm::vec3 direction = glm::vec3(dis_distance(gen), dis_distance(gen), dis_distance(gen));
		asteroid_directions.push_back(direction);
	}

	// Load textures for the asteroids
	GLuint diffuse_texture_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_ao_1k.jpg"));
	GLuint specular_map_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_color_1k.jpg"));
	GLuint normal_map_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_normal_opengl_1k.png"));

	auto const set_uniforms = [&light_position](GLuint program)
	{
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};

	auto const phong_set_uniforms = [&use_normal_mapping, &light_position, &camera_position, &astroid_](GLuint program)
	{
		glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	if (skybox_shape.vao == 0u)
	{
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	GLuint cubemap = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/Space/right.png"),
		config::resources_path("cubemaps/Space/left.png"),
		config::resources_path("cubemaps/Space/top.png"),
		config::resources_path("cubemaps/Space/bottom.png"),
		config::resources_path("cubemaps/Space/front.png"),
		config::resources_path("cubemaps/Space/back.png"));

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&skybox_shader, set_uniforms);
	skybox.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);

	for (auto &asteroid : asteroids)
	{
		asteroid.add_texture("diffuse_texture", diffuse_texture_astroid, GL_TEXTURE_2D);
		asteroid.add_texture("specular_map", specular_map_astroid, GL_TEXTURE_2D);
		asteroid.add_texture("normal_map", normal_map_astroid, GL_TEXTURE_2D);
		asteroid.set_program(&phong_shader, phong_set_uniforms); // Apply Phong shader
	}

	GLuint diffuse_texture_inside = bonobo::loadTexture2D(config::resources_path("spaceship/Alien_Metal_002_DISP.png"));
	GLuint specular_map_inside = bonobo::loadTexture2D(config::resources_path("spaceship/Alien_Metal_002_COLOR.jpg"));
	GLuint normal_map_inside = bonobo::loadTexture2D(config::resources_path("spaceship/Alien_Metal_002_NORM.jpg"));

	Node spaceship_inside;
	spaceship_inside.set_geometry(spaceship_inside_);
	spaceship_inside.add_texture("diffuse_texture", diffuse_texture_inside, GL_TEXTURE_2D);
	spaceship_inside.add_texture("specular_map", specular_map_inside, GL_TEXTURE_2D);
	spaceship_inside.add_texture("normal_map", normal_map_inside, GL_TEXTURE_2D);
	spaceship_inside.set_program(&phong_shader, phong_set_uniforms);

	GLuint diffuse_texture_outside = bonobo::loadTexture2D(config::resources_path("spaceship/everytexture.com-stock-metal-texture-00139-diffuse.jpg"));
	GLuint specular_map_outside = bonobo::loadTexture2D(config::resources_path("spaceship/everytexture.com-stock-metal-texture-00139-bump.jpg"));
	GLuint normal_map_outside = bonobo::loadTexture2D(config::resources_path("spaceship/everytexture.com-stock-metal-texture-00139-normal.jpg"));

	Node spaceship_outside;
	spaceship_outside.set_geometry(spaceship_outside_);
	spaceship_outside.add_texture("diffuse_texture", diffuse_texture_outside, GL_TEXTURE_2D);
	spaceship_outside.add_texture("specular_map", specular_map_outside, GL_TEXTURE_2D);
	spaceship_outside.add_texture("normal_map", normal_map_outside, GL_TEXTURE_2D);
	spaceship_outside.set_program(&phong_shader, phong_set_uniforms);

	spaceship_inside.add_child(&spaceship_outside);

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	auto lastTime = std::chrono::high_resolution_clock::now();

	bool use_orbit_camera = false;
	std::int32_t skybox_program_index = 0;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	changeCullMode(cull_mode);
	enum State
	{
		NEW_GAME,
		PLAY_GAME,
		END_GAME,
	};
	State current_state = NEW_GAME;
	float spaceship_health = 1.0f;
	while (!glfwWindowShouldClose(window))
	{
		switch (current_state)
		{
		case NEW_GAME:
		{
			for (int i = 0; i < asteroid_count; ++i)
			{
				asteroid_positions[i] = glm::vec3(dis(gen), dis(gen), dis(gen));
				asteroids[i].get_transform().SetTranslate(asteroid_positions[i]);
			}
			spaceship_health = 1.0f;
			current_state = PLAY_GAME;
			break;
		}
		case PLAY_GAME:
		{
			// Set the clamped position back to the camera
			// mCamera.mWorld.SetTranslate(camera_position);
			auto const nowTime = std::chrono::high_resolution_clock::now();
			auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
			lastTime = nowTime;

			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
			camera_position = mCamera.mWorld.GetTranslation();
			float min_boundary = -75.0f / 2.0f;
			float max_boundary = 75.0f / 2.0f;

			// Clamp camera position to stay inside the skybox
			camera_position.x = glm::clamp(camera_position.x, min_boundary, max_boundary);
			camera_position.y = glm::clamp(camera_position.y, min_boundary, max_boundary);
			camera_position.z = glm::clamp(camera_position.z, min_boundary, max_boundary);

			spaceship_inside.get_transform().SetTranslate(camera_position + camera_offset);
			spaceship_outside.get_transform().SetTranslate(camera_position + camera_offset);
			glm::vec3 spaceship_position = spaceship_inside.get_transform().GetTranslation();

			auto &io = ImGui::GetIO();
			inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

			glfwPollEvents();
			inputHandler.Advance();
			mCamera.Update(deltaTimeUs, inputHandler);
			// glm::mat4 projViewMatrix = mCamera.mWorld.mar * mCamera.mWorld.GetMatrix();
			// Frustum frustum = extractFrustum(projViewMatrix);

			if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED)
			{
				shader_reload_failed = !program_manager.ReloadAllPrograms();
				if (shader_reload_failed)
					tinyfd_notifyPopup("Shader Program Reload Error",
									   "An error occurred while reloading shader programs; see the logs for details.\n"
									   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
									   "error");
			}
			if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
				show_logs = !show_logs;
			if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
				show_gui = !show_gui;
			if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
				mWindowManager.ToggleFullscreenStatusForWindow(window);

			// std::cout << spaceship_position;
			// std::cout << camera_position;

			// Update asteroid velocities and positions towards the spaceship
			for (int i = 0; i < asteroid_count; ++i)
			{

				if (!checkCollision(asteroid_positions[i], skybox.get_transform().GetTranslation(), 1.0f, 75.0f))
				{
					glm::vec3 n = normalize(asteroid_positions[i] - skybox.get_transform().GetTranslation());
					asteroid_directions[i] = reflect(asteroid_directions[i], -n);
				}

				for (int j = 0; j < asteroid_count; ++j)
				{
					if (i == j)
					{
						continue;
					}
					if (checkCollision(asteroid_positions[j], asteroid_positions[i], 1.0, 1.0))
					{
						glm::vec3 n = normalize(asteroid_positions[j] - asteroid_positions[i]);
						glm::vec3 direction_j = reflect(asteroid_directions[j], n);
						asteroid_velocities[j] = direction_j * asteroid_speed;
						asteroid_directions[i] = reflect(asteroid_directions[i], -n);
					}
				}
				asteroid_speed = elapsed_time_s / 100;
				asteroid_velocities[i] = asteroid_directions[i] * asteroid_speed;

				// Update the position of the asteroid based on velocity
				asteroid_positions[i] += asteroid_velocities[i];
				asteroids[i].get_transform().SetTranslate(asteroid_positions[i]);
				if (checkCollision(spaceship_position, asteroid_positions[i], 0.5f + 0.2f, 1.0))
				{
					asteroid_positions[i] = glm::vec3(dis(gen), dis(gen), dis(gen));
					asteroids[i].get_transform().SetTranslate(asteroid_positions[i]);
					spaceship_health -= 0.25f;
					// std::cout<< (spaceship_position);
					// std::cout << (camera_position);
					spaceship_health = glm::clamp(spaceship_health, 0.0f, 1.0f);
				}

				if (spaceship_health <= 0)
				{
					current_state = END_GAME;
				}
			}

			int N = 1;
			int framebuffer_width, framebuffer_height;
			glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			mWindowManager.NewImGuiFrame();

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			if (!shader_reload_failed)
			{
				skybox.render(mCamera.GetWorldToClipMatrix());

				for (int i = 0; i < asteroid_count; i++)
				{

					asteroids[i].render(mCamera.GetWorldToClipMatrix());
				}
				spaceship_inside.render(mCamera.GetWorldToClipMatrix());
				spaceship_outside.render(mCamera.GetWorldToClipMatrix());
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//
			// Todo: If you want a custom ImGUI window, you can set it up
			//       here
			//
			// char* score = str(deltaTimeUs);
			ImGui::Begin("Spaceship Health and Score");
			ImGui::Text("Health");
			ImGui::ProgressBar(spaceship_health, ImVec2(0.0f, 0.0f)); // Render a bar with current health
			ImGui::Text("Score");
			ImGui::End();
			ImGui::Render();
			mWindowManager.RenderImGuiFrame(show_gui);
			glfwSwapBuffers(window);
			break;
		}
		case END_GAME:
		{
			ImGui::NewFrame();
			ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_None);
			ImGui::Text("Game Over! You were hit by an asteroid.");
			ImGui::Separator();
			if (ImGui::Button("Restart Game"))
			{
				current_state = NEW_GAME;
			}
			if (ImGui::Button("Quit Game"))
			{
				glfwSetWindowShouldClose(window, true);
			}
			ImGui::End();
			ImGui::Render();
			mWindowManager.RenderImGuiFrame(show_gui);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		}
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try
	{
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	}
	catch (std::runtime_error const &e)
	{
		LogError(e.what());
	}
}
