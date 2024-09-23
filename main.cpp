/**
* Author: Jemima Datus
* Assignment: Simple 2D Scene
* Date due: 2024-09-28, 11:58pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION //need for textures

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"                // 4x4 Matrix
#include "glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "ShaderProgram.h"               // We'll talk about these later in the course
//include for loading textures
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

// Our window dimensions
constexpr int WINDOW_WIDTH = 640,
              WINDOW_HEIGHT = 480;

// Background color components //unit rgb
constexpr float BG_RED = 0.612f,
BG_BLUE = 0.902f,
BG_GREEN = 0.949f,
BG_OPACITY = 1.0f;

// Our viewport—or our "camera"'s—position and dimensions
constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture Shaders
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//character sprites
constexpr char kimPossible[] = "kimPossible.png";
constexpr char shego[] = "shego.png";
constexpr char ronStoppable[] = "ronStoppable.png";
constexpr char drDrakken[] = "drDrakken.png";

//texture ID variables for all sprites
GLuint g_kimTextureID, g_shegoTextureID, g_ronTextureID, g_drDrakkenTextureID;

//scale variables
constexpr glm::vec3 g_kimScale = glm::vec3(3.56f, 3.56f, 0.0f);
constexpr glm::vec3 g_shegoScale = glm::vec3(2.58f, 2.58f, 0.0f);
constexpr glm::vec3 g_ronScale = glm::vec3(2.86f, 2.86f, 0.0f);
constexpr glm::vec3 g_drScale = glm::vec3(2.69f, 2.69f, 0.0f);

//inital position variables
constexpr glm::vec3 posKim = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 posShego = glm::vec3(-1.0f, 0.0f, 0.0f);
constexpr glm::vec3 posRon = glm::vec3(3.5f, 0.0f, 0.0f);
constexpr glm::vec3 posDr = glm::vec3(-3.5f, 0.0f, 0.0f);

//variable needed for delta time
float g_prevTick = 0.0f;

//variables need for translation and rotations
glm::vec3 kimTranslate = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ronTranslate = glm::vec3(0.0f, 0.0f, 0.0f);
float shegoRotation = 0.0f;
float theta = 0.0f;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;

ShaderProgram g_shader_program;

//create matrices
glm::mat4 g_view_matrix, g_kimMatrix, g_shegoMatrix, g_ronMatrix, g_drDrakkenMatrix, g_projection_matrix;

//Variables for load_texture
constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;


GLuint loadTexture(const char* filePath) { 

    int width, height, numberOfComponents;
    //loading the image file 
    unsigned char* image = stbi_load(filePath, &width, &height, &numberOfComponents, STBI_rgb_alpha);

    //show error message if the image was not loaded
    if (image == NULL) {
        std::cerr << "Unable to load image. Make sure the path is correct.";
        assert(false);
    }

    //generating a textureID that we will use for our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    //binding the ID to a 2D texture
    glBindTexture(GL_TEXTURE_2D, textureID); 
    //binding the textureID to the raw image data
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //set texture filers linear vs nearest
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //free up memory allocated for image file data
    stbi_image_free(image);
    //return textureID
    return textureID;

}

//drawObject is used to make each of our textured objects appear on the screen
void drawObject(glm::mat4& modelMatrix, GLuint& textureID) {
    g_shader_program.set_model_matrix(modelMatrix);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Kim Possible!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    if (g_display_window == nullptr)
    {
        std::cerr << "ERROR: SDL Window could not be created.\n";
        g_app_status = TERMINATED;

        SDL_Quit();
        exit(1);
    }

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // Initialise our camera
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Load up our shaders
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    //Load textures for all characters
    g_kimTextureID = loadTexture(kimPossible);
    g_shegoTextureID = loadTexture(shego);
    g_ronTextureID = loadTexture(ronStoppable);
    g_drDrakkenTextureID = loadTexture(drDrakken);

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialise our view, model, and projection matrices
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera

    //Initialise all character matrices
    g_kimMatrix = glm::mat4(1.0f); 
    g_shegoMatrix = glm::mat4(1.0f);
    g_ronMatrix = glm::mat4(1.0f);
    g_drDrakkenMatrix = glm::mat4(1.0f);

    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);


    // Each object has its own unique ID
    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}

void update() { 
    //1.translate, 2.rotate, 3.scale
   
    //delta time calculations
    float newTick = (float) SDL_GetTicks() / 1000.0f;
    float deltaTime = newTick - g_prevTick;
    g_prevTick = newTick;

    //reset model matrices
    g_kimMatrix = glm::mat4(1.0f);
    g_shegoMatrix = glm::mat4(1.0f);
    g_ronMatrix = glm::mat4(1.0f);
    g_drDrakkenMatrix = glm::mat4(1.0f);

    //put models in original positions
    g_kimMatrix = glm::translate(g_kimMatrix, posKim);
    g_shegoMatrix = glm::translate(g_shegoMatrix, posShego);
    g_ronMatrix = glm::translate(g_ronMatrix, posRon);
    g_drDrakkenMatrix = glm::translate(g_drDrakkenMatrix, posDr);
    
    //translate kim in a very small circle
    kimTranslate.x += deltaTime * 2 * glm::cos(theta / 200.0f);
    kimTranslate.y += deltaTime * 2 * glm::sin(theta / 200.0f);
    g_kimMatrix = glm::translate(g_kimMatrix, kimTranslate);
   
    //translate ron in relation to kim, so he is always right next to her
    ronTranslate.x = 2.0f; //don't need to multiply by delta because his movement will depend on kim
    g_ronMatrix = glm::translate(g_kimMatrix, ronTranslate);

    //rotate shego
    shegoRotation += 2.0f * deltaTime;
    g_shegoMatrix = glm::rotate(g_shegoMatrix, shegoRotation, glm::vec3(0.0f, shegoRotation, 0.f));

    //scale all characters 
    g_kimMatrix = glm::scale(g_kimMatrix, g_kimScale);
    g_shegoMatrix = glm::scale(g_shegoMatrix, g_shegoScale);
    g_drDrakkenMatrix = glm::scale(g_drDrakkenMatrix, g_drScale);
    g_ronMatrix = glm::scale(g_ronMatrix, g_ronScale);

    //scale drDraken like a heartbeat
    float drDrakkenScale = 1.0f + (0.1f * glm::cos(theta / 200.0f)); //Base scale + (max_amplitude * cos(theta / pulseSpeed)
    g_drDrakkenMatrix = glm::scale(g_drDrakkenMatrix, glm::vec3(drDrakkenScale, drDrakkenScale, 0.0f));

    //increase theta
    theta++;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    //vertices
    float vertices[] = {
    -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    //textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    //draw all of the characters 
    drawObject(g_kimMatrix, g_kimTextureID);
    drawObject(g_shegoMatrix, g_shegoTextureID);
    drawObject(g_ronMatrix, g_ronTextureID);
    drawObject(g_drDrakkenMatrix, g_drDrakkenTextureID);

    //disable attribute arrays 
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    // Initialise our program—whatever that means
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();  // If the player did anything—press a button, move the joystick—process it
        update();         // Using the game's previous state, and whatever new input we have, update the game's state
        render();         // Once updated, render those changes onto the screen
    }

    shutdown();  // The game is over, so let's perform any shutdown protocols
    return 0;
}