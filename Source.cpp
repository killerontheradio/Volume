#ifdef _WIN32
#include <windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif


/**
 * INCLUDES
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <shader.h>
#include <stb_image.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <vector>
#include "arcball.h"
#include "fileread.h"


/**
* MISC
*/
using std::cout;
using std::cerr;
using std::endl;
using std::vector;


/**
 * FUNCTION DECLARATIONS
 */
void process_input(GLFWwindow* window, float delta_time, Arcball& camera);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


/**
 * CONSTANTS
 */
constexpr int SCR_WIDTH{ 800 };
constexpr int SCR_HEIGHT{ 450 };


/**
 * MAIN
 */
int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window{ glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Volume", NULL, NULL) };
    if (window == NULL) {
        cout << "Failed to create GLFW window.\n" << endl;
        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD.\n" << endl;

        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader_program("vertex.glsl", "fragment.glsl");

    float cube_vertices[] = {
        // Positions (Local UVW Space)

        // Front face (Z = 1.0)
        0.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        // Back face (Z = 0.0)
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f,

        // Left face (X = 0.0)
        0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, 1.0f,

        // Right face (X = 1.0)
        1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 1.0f,

        // Bottom face (Y = 0.0)
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.0f,

        // Top face (Y = 1.0)
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f
    };

    unsigned int cubeVAO{};
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    unsigned int cubeVBO{};
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);


    constexpr int dim_x{ 256 };
    constexpr int dim_y{ 256 };
    constexpr int dim_z{ 256 };
    const size_t total_voxels = static_cast<size_t>(dim_x) * dim_y * dim_z;

    vector<uint8_t> volume_data = load_raw_volume("foot.raw", total_voxels);

    if (volume_data.empty()) {
        std::cerr << "ERROR::RAW_FILE::READING.\n" << endl;
    }

    unsigned int vol_tex{}; // volume texture
    glGenTextures(1, &vol_tex);
    glBindTexture(GL_TEXTURE_3D, vol_tex);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, dim_x, dim_y, dim_z, 0, GL_RED, GL_UNSIGNED_BYTE, volume_data.data());

    
    vector<glm::vec4> transfer_function(256);

    for (int i = 0; i < 256; i++) {
        float intensity = i / 255.0f;

        if (intensity < 0.15f) {
            transfer_function[i] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        else if (intensity < 0.45f) {
            transfer_function[i] = glm::vec4(0.8f, 0.3f, 0.2f, 0.02f);
        }
        else {
            transfer_function[i] = glm::vec4(0.9f, 0.9f, 0.8f, 0.5f);
        }
    }

    unsigned int tf_texture{};
    glGenTextures(1, &tf_texture);
    glBindTexture(GL_TEXTURE_1D, tf_texture);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 256, 0, GL_RGBA, GL_FLOAT, transfer_function.data());

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


    /**
     * ARCBALL CAMERA
     */
    Arcball camera{};
    float last_frame{};

    /**
     * RENDER LOOP
     */
    while (!glfwWindowShouldClose(window)) {
        float current_time{ static_cast<float>(glfwGetTime()) };
        float delta_time{ current_time - last_frame };
        last_frame = current_time;

        process_input(window, delta_time, camera);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Projection Matrix (Perspective)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // 2. View Matrix (Place camera back at Z = 3.0 looking at the origin)
        glm::mat4 view = glm::lookAt(
            glm::vec3(1.5f, 1.5f, 3.0f), // Camera position in world space
            glm::vec3(0.0f, 0.0f, 0.0f), // Target position to look at
            glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
        );

        // 3. Model Matrix (Center the cube's rotation pivot)
        // Since vertices are (0 to 1), shifting by (-0.5, -0.5, -0.5) puts the cube center at (0,0,0)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));

        // 4. Send matrices to shader uniforms
        glUniformMatrix4fv(glGetUniformLocation(shader_program.program_id, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader_program.program_id, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program.program_id, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

        glm::vec3 world_cam_pos = glm::vec3(1.5f, 1.5f, 3.0f);
        glm::mat4 inv_model = glm::inverse(model);
        glm::vec3 local_cam_pos = glm::vec3(inv_model * glm::vec4(world_cam_pos, 1.0f));

        glUniform3fv(glGetUniformLocation(shader_program.program_id, "u_local_cam_pos"), 1, glm::value_ptr(local_cam_pos));
        glUniform1i(glGetUniformLocation(shader_program.program_id, "u_max_steps"), 1024);
        glUniform1f(glGetUniformLocation(shader_program.program_id, "u_step_size"), 0.008f);

        view = camera.m_view_matrix;
        glUniformMatrix4fv(glGetUniformLocation(shader_program.program_id, "u_view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::vec3 cam_pos = camera.m_position;
        glUniform3fv(glGetUniformLocation(shader_program.program_id, "u_ray_origin"), 1, glm::value_ptr(cam_pos));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, vol_tex);
        glUniform1i(glGetUniformLocation(shader_program.program_id, "u_volume"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, tf_texture);
        glUniform1i(glGetUniformLocation(shader_program.program_id, "u_transfer_func"), 1);

        shader_program.use_shader_program();
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteShader(shader_program.program_id);

    glfwTerminate();

    return 0;
}


void process_input(GLFWwindow* window, float delta_time, Arcball& camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
    }

    camera.m_rotate_speed = 1.5f * delta_time;
    camera.m_zoom_speed = 2.0f * delta_time;

    // Orbit (WASD)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.m_yaw_theta += camera.m_rotate_speed; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.m_yaw_theta -= camera.m_rotate_speed; }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.m_pitch_phi -= camera.m_rotate_speed; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.m_pitch_phi += camera.m_rotate_speed; }

    camera.m_pitch_phi = glm::clamp(camera.m_pitch_phi, -glm::radians(89.0f), glm::radians(89.0f));

    // Zoom (up/down)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { camera.m_radius -= camera.m_zoom_speed; }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { camera.m_radius += camera.m_zoom_speed; }

    camera.m_radius = glm::clamp(camera.m_radius, 0.5f, 10.0f);

    camera.update_camera_vectors();

    camera.m_view_matrix = glm::lookAt(camera.m_position, camera.m_target, glm::vec3(0.0f, 1.0f, 0.0f));

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
