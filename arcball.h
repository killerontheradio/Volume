#pragma once

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <algorithm>

class Arcball {
public:
	Arcball(glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), float distance = 2.5f) :
		m_target{ target },
		m_radius{ distance },
		m_yaw_theta{ 0.0f },
		m_pitch_phi{ 0.0f },
		m_rotate_speed{ 1.5f },
		m_zoom_speed{ 2.0f },
		m_min_radius{0.5f},
		m_max_radius{10.0f}
	{}

	glm::vec3 m_target{};
	glm::vec3 m_position{};
	glm::mat4 m_view_matrix{};

	float m_radius{};
	float m_yaw_theta{};
	float m_pitch_phi{};

	float m_rotate_speed{};
	float m_zoom_speed{};
	float m_min_radius{};
	float m_max_radius{};

	void update_camera_vectors() {
		m_position.x = m_target.x + m_radius * cos(m_pitch_phi) * sin(m_yaw_theta);
		m_position.y = m_target.y + m_radius * sin(m_pitch_phi);
		m_position.z = m_target.z + m_radius * cos(m_pitch_phi) * cos(m_yaw_theta);

		m_view_matrix = glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
	}
};

