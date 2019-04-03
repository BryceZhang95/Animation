#pragma once
#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/perpendicular.hpp>

#include "Shader.h"
#include "model.h"

#include <map>
#include <vector>

GLfloat g = -9.8f;
GLfloat energy_res_floor = 0.8f;
GLfloat energy_res_wall = 0.7f;
GLfloat MaxSpeed = 5.0f;
class Physics_Rigid {
public:
	Model model;
	glm::vec3 a;
	glm::vec3 speed;
	glm::vec3 position;
	GLint id;
	GLfloat mass;
	GLfloat radius;			
	GLfloat size;
	glm::quat pose;			
	glm::vec3 r_axis;		
	GLfloat angularspeed;

	Physics_Rigid(){}

	Physics_Rigid(Model model) {
		this->model = model;
	}
	void Draw(Shader shader) {
		model.Draw(shader);
	}
	void set_paras(int id, glm::vec3 position, GLfloat mass, GLfloat size, glm::quat pose = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3 speed = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 a = glm::vec3(0.0f, 0.0f, 0.0f)) {
		this->id = id;
		this->position = position;
		this->mass = mass;
		this->size = size;
		this->radius = 95.0f * size;
		this->pose = pose;
		this->speed = speed;
		this->a = a;	
	}
 
	/*
		update position and pose
		X = v*t + 1/2 * a * t2
		v = at

	*/
	void update_position (GLfloat t_period) {
		position = position + speed * t_period + 0.5f * a * t_period * t_period;
		speed = speed + a * t_period;
		glm::vec3 p1(0.0f, 0.0f, 1.0f);
		if (glm::length(speed) > MaxSpeed) {
			speed = MaxSpeed * glm::normalize(speed);
		}
		
	
	}
	
	void boid(glm::vec3 target, Physics_Rigid flocks[], int flocks_size) {
		GLfloat distance_to_target , distance_to_flock;
		GLfloat attarct_factor = 50.0f;
		GLfloat repulsive_factor = 8000.0f;
		glm::vec3 target_vector, flock_vector;
		glm::vec3 force_target , force_flock , force_flock_attract, force_flock_repulsive;

		
		// with target
		// Alignment
		target_vector = target - this->position;
		distance_to_target = glm::length(target_vector);
		if (distance_to_target < 2.0f)
			force_target = glm::vec3(0.0f, 0.0f, 0.0f);
		else
			force_target = glm::normalize(target_vector) * distance_to_target * distance_to_target * attarct_factor;

		// with flocks
		for (int i = 0; i < flocks->size; i++)
		{
			if (flocks[i].id != this->id) {
				flock_vector = flocks[i].position - this->position;
				distance_to_flock = glm::length(flock_vector);
				if (distance_to_flock > 0)
				{
					// Separation
					force_flock_repulsive = - glm::normalize(flock_vector) / distance_to_flock / distance_to_flock * repulsive_factor;
					// Cohesion
					force_flock_attract = glm::normalize(flock_vector) * distance_to_flock * distance_to_flock * attarct_factor;
					force_flock = force_flock + force_flock_attract + force_flock_repulsive;

				}
			}
		}
		this->a = (force_flock + force_target) / this->mass;
	}

	

};