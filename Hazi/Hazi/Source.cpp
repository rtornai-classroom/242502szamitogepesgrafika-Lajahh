#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

std::string readShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

GLuint createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSrc = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);                                           // Shader fileok betöltése és program létrehozása
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSrc = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

float distBetween(const glm::vec2& a, const glm::vec2& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;                                       // Két pont közti táv kiszámolása
    return std::sqrt(dx * dx + dy * dy);
}

bool metszesCheck(const glm::vec2& circPos, float radius,
    const glm::vec2& lineStart, const glm::vec2& lineEnd) {
    glm::vec2 lineDir = lineEnd - lineStart;
    float lineLength = std::sqrt(lineDir.x * lineDir.x + lineDir.y * lineDir.y);
    glm::vec2 lineNorm = lineDir / lineLength;

    glm::vec2 toCircle = circPos - lineStart;
    float proj = toCircle.x * lineNorm.x + toCircle.y * lineNorm.y;               // Metszés ellenőrzésre szolgáló kód

    glm::vec2 closest;
    if (proj <= 0) closest = lineStart;
    else if (proj >= lineLength) closest = lineEnd;
    else closest = lineStart + lineNorm * proj;

    return distBetween(circPos, closest) <= radius;
}

glm::vec2 circCenter(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
float circRadius = 50.0f;
glm::vec3 innerColor(0.8f, 0.0f, 0.0f);
glm::vec3 outerColor(0.0f, 0.8f, 0.0f);
glm::vec3 lineColor(0.0f, 0.0f, 1.0f);

float lineY = 0.0f;
const float lineMove = 0.01f;

glm::vec2 veloc(1.0f, 0.0f);     // Kör alap mozgása
bool isMoving = true;
const float initSpeed = 1.0f;                   // Kör mozgásához és irányához szükséges paraméterek
const float angle = glm::radians(25.0f);

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_UP: lineY = std::min(lineY + lineMove, 1.0f); break;
        case GLFW_KEY_DOWN: lineY = std::max(lineY - lineMove, -1.0f); break;
        case GLFW_KEY_S:
            if (isMoving) {
                veloc.x = initSpeed * std::cos(angle);                          // Billentyű események kezelése
                veloc.y = initSpeed * std::sin(angle);
            }
            else {
                isMoving = true;
                veloc.x = initSpeed * std::cos(angle);
                veloc.y = initSpeed * std::sin(angle);
            }
            break;
        }
    }
}

int main() {
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pattogó Kör", nullptr, nullptr);
    if (!window) {
        glfwTerminate();                                                                                        // Inicializálás
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return EXIT_FAILURE;
    }

    glfwSetKeyCallback(window, keyCallback);

    GLuint circShader = createShaderProgram(readShaderFile("vertex_shader.glsl"),readShaderFile("fragment_shader.glsl"));
    GLuint lineShader = createShaderProgram(readShaderFile("line_vertex_shader.glsl"),readShaderFile("line_fragment_shader.glsl"));

    GLuint circVAO, circVBO;
    glGenVertexArrays(1, &circVAO);           // Kör VAO és VBO-ja
    glGenBuffers(1, &circVBO);

    std::vector<glm::vec2> circVert = {
        {-1.0f, -1.0f},
        {1.0f, -1.0f},                         // Kör határai
        {-1.0f, 1.0f},
        {1.0f, 1.0f}
    };

    glBindVertexArray(circVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circVBO);
    glBufferData(GL_ARRAY_BUFFER, circVert.size() * sizeof(glm::vec2), circVert.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);                 // Vonal VAO és VBO-ja
    glGenBuffers(1, &lineVBO);

    std::vector<glm::vec2> lineVert = {
        {-0.33f, 0.0f},                             // Vonal hossza
        {0.33f, 0.0f}
    };

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineVert.size() * sizeof(glm::vec2), lineVert.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 0.7f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (isMoving) {
            circCenter += veloc;

            if (circCenter.x - circRadius < 0) {
                circCenter.x = circRadius;
                veloc.x = std::abs(veloc.x);
            }
            else if (circCenter.x + circRadius > WINDOW_WIDTH) {           // Kör mozgatására és visszapattanására felelős kód
                circCenter.x = WINDOW_WIDTH - circRadius;
                veloc.x = -std::abs(veloc.x);
            }

            if (circCenter.y - circRadius < 0) {
                circCenter.y = circRadius;
                veloc.y = std::abs(veloc.y);
            }
            else if (circCenter.y + circRadius > WINDOW_HEIGHT) {
                circCenter.y = WINDOW_HEIGHT - circRadius;
                veloc.y = -std::abs(veloc.y);
            }
        }

        glm::vec2 lineStart(WINDOW_WIDTH / 2.0f - WINDOW_WIDTH / 6.0f,lineY * WINDOW_HEIGHT / 2.0f + WINDOW_HEIGHT / 2.0f);
        glm::vec2 lineEnd(WINDOW_WIDTH / 2.0f + WINDOW_WIDTH / 6.0f,lineY * WINDOW_HEIGHT / 2.0f + WINDOW_HEIGHT / 2.0f);

        bool metszes = metszesCheck(circCenter, circRadius, lineStart, lineEnd);
        if (metszes) {
            innerColor = glm::vec3(0.8f, 0.0f, 0.0f);
            outerColor = glm::vec3(0.0f, 0.8f, 0.0f);                       // Metszés ellenőrzése és a színek beállítása
        }
        else {
            innerColor = glm::vec3(0.0f, 0.8f, 0.0f);
            outerColor = glm::vec3(0.8f, 0.0f, 0.0f);
        }

        glUseProgram(circShader);
        glUniform2f(glGetUniformLocation(circShader, "circCenter"), circCenter.x, circCenter.y);
        glUniform1f(glGetUniformLocation(circShader, "circRadius"), circRadius);                       // Kör kirajzolása
        glUniform3fv(glGetUniformLocation(circShader, "innerColor"), 1, glm::value_ptr(innerColor));
        glUniform3fv(glGetUniformLocation(circShader, "outerColor"), 1, glm::value_ptr(outerColor));

        glBindVertexArray(circVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        glUseProgram(lineShader);
        glUniform3fv(glGetUniformLocation(lineShader, "lineColor"), 1, glm::value_ptr(lineColor));          // Vonal kirajzolása és frissítése

        std::vector<glm::vec2> newLineVert = {
            {-0.33f, lineY},
            {0.33f, lineY}
        };
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, newLineVert.size() * sizeof(glm::vec2), newLineVert.data(), GL_STATIC_DRAW);

        glBindVertexArray(lineVAO);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &circVAO);
    glDeleteBuffers(1, &circVBO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteProgram(circShader);
    glDeleteProgram(lineShader);
    glfwTerminate();

    return EXIT_SUCCESS;
}