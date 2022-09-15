#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	// Helper functions
	void next_round();

	//----- game state -----

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	// Jump animations
	Scene::Transform *left_transform = nullptr;
	Scene::Transform *right_transform = nullptr;
	Scene::Transform *center_transform = nullptr;
	float left_origin;
	float right_origin;
	float center_origin;
	float right_velocity = 0.0f;
	float left_velocity = 0.0f;
	float center_velocity = 0.0f;
	float acceleration = 10.0f;
	bool left_jump = false;
	bool right_jump = false;
	bool center_jump = false;
	bool jump_in_progress = false;

	enum Status {
		NOTHING,
		CORRECT,
		INCORRECT
	} game_status;

	enum Guess {
		LEFT,
		RIGHT,
		CENTER,
		NONE
	} current_guess;

	int round_number;
	int num_correct;
	float round_timer;
	bool made_guess;
	bool end_game;
	float total_time = 0;

	// My audio
	std::vector<Sound::Sample> rounds;
	std::vector<Guess> answers;
	std::shared_ptr<Sound::PlayingSample> current_sound;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
