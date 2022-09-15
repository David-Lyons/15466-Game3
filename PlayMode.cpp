#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	}
	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	for (int i = 0; i < 10; i++) {
		rounds.push_back(Sound::Sample(data_path("Test Sound.wav")));
	}
	answers.push_back(Guess::LEFT);
	answers.push_back(Guess::RIGHT);
	answers.push_back(Guess::CENTER);
	answers.push_back(Guess::LEFT);
	answers.push_back(Guess::RIGHT);
	answers.push_back(Guess::CENTER);
	answers.push_back(Guess::LEFT);
	answers.push_back(Guess::RIGHT);
	answers.push_back(Guess::CENTER);
	answers.push_back(Guess::CENTER);

	round_number = 0;
	num_correct = 0; 
	current_sound = Sound::play(rounds[0]);
	current_guess = Guess::NONE;
	round_timer = 30.0f;
	made_guess = false;
	game_status = Status::NOTHING;
	end_game = false;
}

PlayMode::~PlayMode() {
}

void PlayMode::next_round() {
	if (current_guess == answers[round_number]) {
		num_correct++;
		game_status = Status::CORRECT;
	} else {
		game_status = Status::INCORRECT;
	}
	current_guess = Guess::NONE;
	round_number++;
	current_sound = Sound::play(rounds[round_number]);
	round_timer = 30.0f;
	made_guess = false;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (end_game || made_guess) {
		return false;
	}
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			made_guess = true;
			current_guess = Guess::LEFT;
			printf("You guessed left.\n");
			printf("The answer is %d.\n", answers[round_number]);
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			made_guess = true;
			current_guess = Guess::RIGHT;
			printf("You guessed right.\n");
			printf("The answer is %d.\n", answers[round_number]);
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_DOWN) {
			made_guess = true;
			current_guess = Guess::CENTER;
			printf("You guessed center.\n");
			printf("The answer is %d.\n", answers[round_number]);
			return true;
		}
	}
	return false;
}

void PlayMode::update(float elapsed) {
	if (end_game) {
		return;
	}

	// Decrease the round timer, and go to the next round if this one is over
	if (made_guess) {
		current_sound->stop();			
		if (round_number == 9) {
			end_game = true;
		} else {
			next_round();
		}
	} else {
		round_timer -= elapsed;
		if (round_timer <= 0.0f) {
			if (round_number == 9) {
				end_game = true;
			} else {
				next_round();
			}
		}
	}

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.2f;
		float offset = 50.0f / drawable_size.y;
		switch (game_status) {
		case NOTHING:
			break;
		case CORRECT:
			lines.draw_text("That was correct!",
				glm::vec3(-aspect + 0.1f * H + offset, -1.0 + +0.1f * H + offset, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			break;
		case INCORRECT:
			lines.draw_text("Sorry, that was incorrect...",
				glm::vec3(-aspect + 0.1f * H + offset, -1.0 + +0.1f * H + offset, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
			break;
		}

		lines.draw_text("Number Correct: " + std::to_string(num_correct),
			glm::vec3(-aspect + 0.1f * H + offset, -1.0 + +0.1f * H + 1200.0f / drawable_size.y, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		if (end_game) {
			lines.draw_text("That was the last one!",
				glm::vec3(-aspect + 0.1f * H + 1200.0f / drawable_size.y, -1.0 + +0.1f * H + 1000.0f / drawable_size.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		}
	}
	GL_ERRORS();
}
