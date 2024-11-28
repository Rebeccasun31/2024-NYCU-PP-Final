#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <bits/stdc++.h>

#include "./header/Object.h"
#include "./header/stb_image.h"
#include "./header/point.h"
#include "./header/nbody.h"
#include "./header/camera.h"

#define ANGEL_TO_RADIAN(x) (float)((x)*M_PI / 180.0f)


static point vertices_1[POINT_CNT];
static point vertices_2[POINT_CNT];
point *points1 = vertices_1, *points2 = vertices_2;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int createShader(const string &filename, const string &type);
unsigned int createProgram(unsigned int vertexShader, unsigned int fragmentShader);
unsigned int modelVAO(Object &model);
unsigned int loadTexture(const string *tFileName);
glm::vec3 HSV2RGB(glm::vec3 hsv); 
void init();
void nextdrawCircle(glm::quat rota, glm::vec3 *rotated, glm::vec3 *beforerotate);
void drawCircle(float px, float py, float pz, float R);

// settings
int SCR_WIDTH = 1600;
int SCR_HEIGHT = 900;

// Shader
unsigned int vertexShader, fragmentShader, shaderProgram;

// Texture
unsigned int earthTexture;

// VAO, VBO
unsigned int earthVAO, cubeVAO;

// Objects to display
Object *earthObject, *cubeObject;

// Constants you may need
const int rotateEarthSpeed = 30;

// You can use these parameters
float rotateEarthDegree = 0;

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PP-Final-Group11", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initialize Object, Shader, Texture, VAO, VBO
    init();

    // Enable depth test, face culling
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Set viewport
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // render loop variables
    double dt;
    double lastTime = glfwGetTime();
    double currentTime;

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    GLint textureLoc = glGetUniformLocation(shaderProgram, "ourTexture");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "ourColor");

    // camera
	Camera camera(glm::vec3(0.0f, 100.0f, 180.0f));
	camera.initialize(static_cast<float>((float)SCR_WIDTH) / (float)SCR_HEIGHT);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        point* tmp;
        // render
        glClearColor(0 / 255.0, 0 / 255.0, 0 / 255.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		camera.move(window);
        // glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 100.0f, 180.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

        glm::mat4 base(1.0f), earthModel(1.0f), cubeModel(1.0f);

        // earth
        earthModel = glm::rotate(earthModel, glm::radians(rotateEarthDegree), glm::vec3(0.0f, 1.0f, 0.0f));
        earthModel = glm::scale(earthModel, glm::vec3(10.0f, 10.0f, 10.0f));

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(earthModel));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.getViewMatrix());
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.getProjectionMatrix());

        glUniform1i(textureLoc, 1);
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));

        glBindVertexArray(earthVAO);
        glDrawArrays(GL_TRIANGLES, 0, earthObject->positions.size() / 3);
        glBindVertexArray(0);

        glUseProgram(0);

        // nbody
        nBodyCalculateSerial(points1, points2, dt * DELTA_TIME_MUL);
        tmp = points1;
		points1 = points2;
		points2 = tmp;

        glUseProgram(shaderProgram);
		for (int i = 0; i < POINT_CNT; i++) {
            // std::cout << points1[i]._x << points1[i]._y << points1[i]._z <<'\n';
            cubeModel = glm::translate(base, glm::vec3(points1[i]._x, points1[i]._y, points1[i]._z));
            cubeModel = glm::scale(cubeModel, glm::vec3(points1[i]._size, points1[i]._size, points1[i]._size));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera.getViewMatrix());
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, camera.getProjectionMatrix());

            glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(points1[i]._r, points1[i]._g, points1[i]._b)));

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, cubeObject->positions.size() / 3);
            glBindVertexArray(0);
		}
        glUseProgram(0);

        // Status update
        currentTime = glfwGetTime();
        dt = currentTime - lastTime;
        lastTime = currentTime;

        rotateEarthDegree += (float)rotateEarthSpeed * dt;
        if (rotateEarthDegree >= 360) {
            rotateEarthDegree -= 360;
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}


unsigned int createShader(const string &filename, const string &type) {
    // create a new shader object
    GLuint shader;
    if (type == "vert") {   // vertex shader
        shader = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (type == "frag") {  // fragment shader
        shader = glCreateShader(GL_FRAGMENT_SHADER);
    }
    else {
        cerr << "shader type not found.\n";
        exit(1);
    }

    // read the shader file and set the source code
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "shader file not found.\n";
        exit(1);
    }
    string sourceCode((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    vector<const GLchar*> sourceCodeGL = {sourceCode.c_str()};
    glShaderSource(shader, 1, sourceCodeGL.data(), NULL);

    // compile
    glCompileShader(shader);
    return shader;
}


unsigned int createProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    // create a new program object
    GLuint program =  glCreateProgram();

    // attach shader objects to the program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // link the program
    glLinkProgram(program);

    // detach the shaders
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    return program;
}


unsigned int modelVAO(Object &model) {
    // create VAO and bind it
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // create VBO and bind it
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // refactor the vertex data to enhance readbility.
    struct VertexAttribute {
        GLfloat position[3];
        GLfloat normal[3];
        GLfloat texcoord[2];
    };
    int numVertices = model.positions.size() / 3;
    vector<VertexAttribute> vertices(numVertices);
    for (int i = 0; i < numVertices ; i++) {
        for (int j = 0; j < 3; j++) {
            vertices[i].position[j] = model.positions[i * 3 + j];
            vertices[i].normal[j] = model.normals[i * 3 + j];
        }
        for (int j = 0; j < 2; j++) {
            vertices[i].texcoord[j] = model.texcoords[i * 2 + j];
        }
    }
    // copy the data into VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * numVertices, vertices.data(), GL_STATIC_DRAW);

    // link the VBO with the vertex shader input
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, texcoord)));
    glEnableVertexAttribArray(2);

    // unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // unbind VAO
    glBindVertexArray(0);
    return VAO;
}


unsigned int loadTexture(const string &filename) {
    // enable texture
    glEnable(GL_TEXTURE_2D);

    // use different texture unit to save more than one textures
    if (filename.substr(filename.length() - 12) == "airplane.jpg") {
        glActiveTexture(GL_TEXTURE0);
    }
    else if (filename.substr(filename.length() - 9) == "earth.jpg") {
        glActiveTexture(GL_TEXTURE1);
    }

    // create a new texture object and bind it to 2D image
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the parameters to handle the scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load the texture image
    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (data == nullptr) {
        cerr << "Failed to load image: " << stbi_failure_reason() << '\n';
        exit(1);
    }

    // generate the texture by the image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    return texture;
}

glm::vec3 HSV2RGB(glm::vec3 hsv) {
    float c = hsv.z * hsv.y;
    float x = c * (1.0 - abs(fmod(hsv.x / 60.0, 2.0) - 1.0));
    float m = hsv.z - c;

    glm::vec3 rgb = glm::vec3(0.0f, 0.0f, 0.0f);
    if (hsv.x < 60.0) {
        rgb = glm::vec3(c, x, 0.0);
    } else if (hsv.x < 120.0) {
        rgb = glm::vec3(x, c, 0.0);
    } else if (hsv.x < 180.0) {
        rgb = glm::vec3(0.0, c, x);
    } else if (hsv.x < 240.0) {
        rgb = glm::vec3(0.0, x, c);
    } else if (hsv.x < 300.0) {
        rgb = glm::vec3(x, 0.0, c);
    } else {
        rgb = glm::vec3(c, 0.0, x);
    }

    return rgb + m;
}


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void init() {
#if defined(__linux__) || defined(__APPLE__)
    string dirShader = "../../source/src/shaders/";
    string dirAsset = "../../source/src/asset/obj/";
    string dirTexture = "../../source/src/asset/texture/";
#else
    string dirShader = "..\\..\\source\\src\\shaders\\";
    string dirAsset = "..\\..\\source\\src\\asset\\obj\\";
    string dirTexture = "..\\..\\source\\src\\asset\\texture\\";
#endif

    // Object
    earthObject = new Object(dirAsset + "earth.obj");
    cubeObject = new Object(dirAsset + "cube.obj");

    // Shader
    vertexShader = createShader(dirShader + "vertexShader.vert", "vert");
    fragmentShader = createShader(dirShader + "fragmentShader.frag", "frag");
    shaderProgram = createProgram(vertexShader, fragmentShader);
    glUseProgram(shaderProgram);

    // Texture
    earthTexture = loadTexture(dirTexture + "earth.jpg");

    // VAO, VBO
    earthVAO = modelVAO(*earthObject);
    cubeVAO = modelVAO(*cubeObject);

    vertices_1[0] = point(0, 0, 0, 255, 255, 255, 20, POINT_MASS_MAX * 100, 0, 0, 0);
    vertices_2[0] = vertices_1[0];

    vertices_1[1] = point(-100, 0, 0, 255, 255, 255, 2, 10, 0, sqrt(GRAVITATIONAL_G * POINT_MASS_MAX * 100 / 100), 0);
    vertices_2[1] = vertices_1[1];

    // for (int i = 1; i < POINT_CNT; ++i) {
	// 	unsigned int seed = SEED;
	// 	vertices_1[i] = point(seed);
	// 	vertices_2[i] = vertices_1[i];
	// }
}