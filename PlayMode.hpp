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

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;

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

	// My audio
	std::vector<Sound::Sample> rounds;
	std::vector<Guess> answers;
	std::shared_ptr<Sound::PlayingSample> current_sound;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
