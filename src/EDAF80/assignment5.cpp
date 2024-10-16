#include "assignment5.hpp"
#include "parametric_shapes.cpp"

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
	bool colX = abs(pos1.x - pos2.x) < rad1 + rad2;
	bool colY = abs(pos1.y - pos2.y) < rad1 + rad2;
	bool colZ = abs(pos1.z - pos2.z) < rad1 + rad2;
	return colX && colY && colZ;
}

void edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

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

	auto skybox_shape = parametric_shapes::createSphere(500.0f, 1000u, 1000u);
	auto astroid_ = parametric_shapes::createSphere(1.0f, 100u, 100u);
	auto spaceship_inside_ = parametric_shapes::createSphere(0.50f, 100u, 100u);
	auto spaceship_outside_ = parametric_shapes::createTorus(0.50f, 0.20f, 100u, 100u);

	bool use_normal_mapping = true;
	auto light_position = glm::vec3(250.0f, 250.0f, -250.0f);

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

	GLuint diffuse_texture_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_ao_1k.jpg"));
	GLuint specular_map_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_color_1k.jpg"));
	GLuint normal_map_astroid = bonobo::loadTexture2D(config::resources_path("astriods/ground_0010_normal_opengl_1k.png"));

	Node astroid;
	astroid.set_geometry(astroid_);
	astroid.add_texture("diffuse_texture", diffuse_texture_astroid, GL_TEXTURE_2D);
	astroid.add_texture("specular_map", specular_map_astroid, GL_TEXTURE_2D);
	astroid.add_texture("normal_map", normal_map_astroid, GL_TEXTURE_2D);
	astroid.set_program(&phong_shader, phong_set_uniforms);

	Node spaceship_inside;
	spaceship_inside.set_geometry(spaceship_inside_);
	spaceship_inside.set_program(&fallback_shader, set_uniforms);
	spaceship_inside.add_texture("fallback", fallback_shader, GL_TEXTURE0);

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
	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//

	//
	// Todo: Load your geometry
	//

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
	while (!glfwWindowShouldClose(window))
	{
		switch (current_state)
		{
		case NEW_GAME:
		{
			current_state = PLAY_GAME;
			break;
		}
		case PLAY_GAME:
		{
			glm::vec3 camera_position = mCamera.mWorld.GetTranslation();
			float min_boundary = -500.0f / 2.0f;
			float max_boundary = 500.0f / 2.0f;

			// Clamp camera position to stay inside the skybox
			camera_position.x = glm::clamp(camera_position.x, min_boundary, max_boundary);
			camera_position.y = glm::clamp(camera_position.y, min_boundary, max_boundary);
			camera_position.z = glm::clamp(camera_position.z, min_boundary, max_boundary);

			// Set the clamped position back to the camera
			mCamera.mWorld.SetTranslate(camera_position);
			auto const nowTime = std::chrono::high_resolution_clock::now();
			auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
			lastTime = nowTime;

			auto &io = ImGui::GetIO();
			inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

			glfwPollEvents();
			inputHandler.Advance();
			mCamera.Update(deltaTimeUs, inputHandler);

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

			int N = 1;
			for (int i = 0; i < N; i++)
			{
				if (checkCollision(glm::vec3(spaceship_inside.get_transform().GetMatrix()), astroid.get_transform().GetMatrix(), 0.5f + 0.2f, 1.0))
				{
					current_state = END_GAME;
				}
			}
			// Retrieve the actual framebuffer size: for HiDPI monitors,
			// you might end up with a framebuffer larger than what you
			// actually asked for. For example, if you ask for a 1920x1080
			// framebuffer, you might get a 3840x2160 one instead.
			// Also it might change as the user drags the window between
			// monitors with different DPIs, or if the fullscreen status is
			// being toggled.
			int framebuffer_width, framebuffer_height;
			glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
			glViewport(0, 0, framebuffer_width, framebuffer_height);

			//
			// Todo: If you need to handle inputs, you can do it here
			//

			mWindowManager.NewImGuiFrame();

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			if (!shader_reload_failed)
			{
				skybox.render(mCamera.GetWorldToClipMatrix());
				glm::mat4 transformastr = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));

				astroid.render(mCamera.GetWorldToClipMatrix(), transformastr);

				std::cout << spaceship_inside.get_transform().GetMatrix();
				// std::cout << (astroid.get_transform().GetTranslation());
				glm::mat4 transform = mCamera.mWorld.GetMatrix();
				transform = glm::translate(transform, glm::vec3(0.0f, -1.0f, -5.0f));
				spaceship_inside.render(mCamera.GetWorldToClipMatrix(), transform);
				spaceship_outside.render(mCamera.GetWorldToClipMatrix(), transform);
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			//
			// Todo: If you want a custom ImGUI window, you can set it up
			//       here
			//
			bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
			if (opened)
			{
				auto sky_box_selection_result = program_manager.SelectProgram("skybox", skybox_program_index);
				if (sky_box_selection_result.was_selection_changed)
				{
					skybox.set_program(sky_box_selection_result.program, set_uniforms);
				}
				ImGui::Separator();
				// ImGui::SliderFloat3("Light Position", glm::value_ptr(light_position), -20.0f, 20.0f);
				ImGui::Separator();
				ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
				ImGui::Separator();
				ImGui::Checkbox("Show basis", &show_basis);
				ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
				ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
			}
			ImGui::End();
			if (show_basis)
				bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
			if (show_logs)
				Log::View::Render();
			mWindowManager.RenderImGuiFrame(show_gui);

			glfwSwapBuffers(window);
			break;
		}
		case END_GAME:
		{
			// printf("dead");
			// break;
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
