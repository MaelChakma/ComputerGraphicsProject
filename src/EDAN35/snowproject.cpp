#include "snowproject.hpp"
#include "EDAF80/parametric_shapes.hpp"
#include "ParticleSystem.hpp"

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

// ------------------------------------------------------ Constructor of the project
edan35::SnowProject::SnowProject(WindowManager& windowManager) : mCamera(0.5f * glm::half_pi<float>(),
	static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("edan35::SnowProject", window_datum, config::msaa_rate);
	if (window == nullptr)
	{
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

// ------------------------------------------------------ Destructor of the project
edan35::SnowProject::~SnowProject()
{
	bonobo::deinit();
}


// ------------------------------------------------------ Runs the project
void edan35::SnowProject::run()
{
	// --------------------- Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(-40.0f, 14.0f, 6.0f));
	mCamera.mWorld.LookAt(glm::vec3(0.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(5.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// ---------------------- Create the shader programs
	ShaderProgramManager program_manager;

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ {ShaderType::vertex, "EDAF80/skybox.vert"},
		 {ShaderType::fragment, "EDAF80/skybox.frag"} },
		skybox_shader);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("water",
		{ {ShaderType::vertex, "EDAF80/water.vert"},
		 {ShaderType::fragment, "EDAF80/water.frag"} },
		water_shader);
	if (water_shader == 0u)
		LogError("Failed to load water shader");


	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program)
		{
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		};

	float elapsed_time_s = 0.0f;
	float wave_speed = 1.0f;
	float wave_amplitude = 0.50f;

	auto const water_uniforms = [&elapsed_time_s, &wave_speed, &wave_amplitude, &light_position, &camera_position](GLuint program)
		{
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			glUniform1f(glGetUniformLocation(program, "wave_speed"), wave_speed);
			glUniform1f(glGetUniformLocation(program, "wave_amplitude"), wave_amplitude);
			glUniform1f(glGetUniformLocation(program, "elapsed_time"), elapsed_time_s);
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));

		};

	auto quad_shape = parametric_shapes::createQuad(100.0f, 100.0f, 1000u, 1000u);
	if (quad_shape.vao == 0u)
	{
		LogError("Failed to retrieve the mesh for the demo sphere");
		return;
	}
	auto skybox_shape = parametric_shapes::createSphere(100.0f, 100u, 100u);
	if (skybox_shape.vao == 0u)
	{
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	GLuint cubemap = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/NissiBeach2/posx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negx.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negy.jpg"),
		config::resources_path("cubemaps/NissiBeach2/posz.jpg"),
		config::resources_path("cubemaps/NissiBeach2/negz.jpg"));

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.set_program(&skybox_shader, set_uniforms);
	skybox.add_texture("cubemap", cubemap, GL_TEXTURE_CUBE_MAP);

	GLuint normal_map = bonobo::loadTexture2D(config::resources_path("textures/waves.png"));

	Node quad;
	quad.set_geometry(quad_shape);
	// quad.set_program(&fallback_shader, set_uniforms);
	quad.set_program(&water_shader, water_uniforms);
	//quad.set_program(&particle_shader, set_uniforms);
	quad.add_texture("normal_map", normal_map, GL_TEXTURE_2D);
	quad.add_texture("water_texture", cubemap, GL_TEXTURE_CUBE_MAP);


	// ----------------------------------- Setup particles

	// Creating the particle shader program
	GLuint particleGPT_shader = 0u;
	program_manager.CreateAndRegisterProgram("ParticleGPT",
		{ {ShaderType::vertex, "EDAN35/particleGPT.vert"},
		  {ShaderType::fragment, "EDAN35/particleGPT.frag"}
		},
		particleGPT_shader);
	if (particleGPT_shader == 0u)
		LogError("Failed to load particleGPT shader");

	// Setup the feedbacks varyings
	const char* feedbackVaryings[] = { "positionOut", "velocityOut", "lifetimeOut" };
	glTransformFeedbackVaryings(particleGPT_shader, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(particleGPT_shader);


	// Setup of the vertex buffer object to store 1 particle
	
	const int MAX_PARTICLES = 1;
	std::vector<Particle> particles(MAX_PARTICLES);

	// Initialize particles with random data
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		particles[i].position[0] = (rand() % 200 - 100) / 100.0f;
		particles[i].position[1] = (rand() % 200 - 100) / 100.0f;
		particles[i].position[2] = 0.0f;

		particles[i].velocity[0] = 1.0f;
		particles[i].velocity[1] = 0.0f;
		particles[i].velocity[2] = 0.0f;

		particles[i].lifetime = 1.0f;

		particles[i].color = vec4(1.0f);
	}

	// Create a buffer for particles
	GLuint particleBuffers[2];
	glGenBuffers(2, particleBuffers);

	// Creating the transform feedback
	GLuint transformFeedback[2];
	glGenTransformFeedbacks(2, transformFeedback);

	// Binding buffers and transform feedbacks
	for (int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffers[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * MAX_PARTICLES, particles.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffers[i]);
	}


	// Setting a buffer for the geometry of the particle

	float particleGeometryVertices[] = {
		-0.1f, -0.1f, 0.0f,
		 0.1f, -0.1f, 0.0f,
		-0.1f,  0.1f, 0.0f,
		 0.1f,  0.1f, 0.0f,
	};
	GLuint particleGeometryVBO, particleGeometryVAO;
	
	glGenVertexArrays(1, &particleGeometryVAO);
	glBindVertexArray(particleGeometryVAO);

	glGenBuffers(1, &particleGeometryVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particleGeometryVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleGeometryVertices), particleGeometryVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
		

	// Set up instancing

	glBindVertexArray(particleGeometryVAO);
	for (int i = 0; i < 2; i++) {
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffers[i]);
		// Position (attribute 1)
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);

		// Velocity (attribute 2)
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, velocity));
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1);

		// Lifetime (attribute 3)
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, lifetime));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);

		// Color (attribute 4)
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);

	}

	glBindVertexArray(0u);

	glUseProgram(0u);

	// ----------------------------------- End off setup particles
	

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	auto lastTime = std::chrono::high_resolution_clock::now();

	bool pause_animation = true;
	bool use_orbit_camera = false;
	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false;
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	changeCullMode(cull_mode);

	int iter = 0;
	while (!glfwWindowShouldClose(window))
	{
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation)
		{
			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
		}

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		if (use_orbit_camera)
		{
			mCamera.mWorld.LookAt(glm::vec3(0.0f));
		}
		camera_position = mCamera.mWorld.GetTranslation();

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
		bonobo::changePolygonMode(polygon_mode);

		if (!shader_reload_failed)
		{
			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
			skybox.render(mCamera.GetWorldToClipMatrix());
			quad.render(mCamera.GetWorldToClipMatrix());

			// ---------------------------- Rendering particles

			// Use the shader
			glUseProgram(particleGPT_shader);

			// Update the uniforms of the vertex shader
			GLuint deltaTimeLoc = glGetUniformLocation(particleGPT_shader, "deltaTime");
			glUniform1f(deltaTimeLoc, std::chrono::duration<float>(deltaTimeUs).count());

			glBindVertexArray(particleGeometryVAO);

			int n = iter %2;
			glBindBuffer(GL_ARRAY_BUFFER, particleBuffers[0]);
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[1]);
			iter++;

			glBeginTransformFeedback(GL_TRIANGLES);
			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES); // Render all instances
			glEndTransformFeedback();

			glBindVertexArray(0u);
			glUseProgram(0u);

		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		

		// Scene controls
		bool opened = ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_None);
		if (opened)
		{

			ImGui::SliderFloat("Wave Speed", &wave_speed, 0.1f, 5.0f);
			ImGui::SliderFloat("Wave Amplitude", &wave_amplitude, 0.1f, 5.0f);
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Use orbit camera", &use_orbit_camera);
			ImGui::Separator();
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed)
			{
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
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
	}
}



// ----------------------------------------------------- Main
int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try
	{
		edan35::SnowProject snowproject(framework.GetWindowManager());
		snowproject.run();
	}
	catch (std::runtime_error const& e)
	{
		LogError(e.what());
	}
}
