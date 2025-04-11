#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

std::vector<glm::vec2> controlPoints;
bool drag = false;
int selectedPoint = -1;

std::string readShaderFile(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    return shaderStream.str();
}

GLuint createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSrc = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);                                                // Shader fileok betöltése és program létrehozása
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

float sqrDistance(const glm::vec2& p1, const glm::vec2& p2) {
    float sx = p1.x - p2.x;                                                 // Távolság négyzete két pont között
    float sy = p1.y - p2.y;
    return sx * sx + sy * sy;
}

int actPoint(std::vector<glm::vec2>& points, float sens, const glm::vec2& mousePos) {
    float sqrSens = sens * sens;
    for (int i = 0; i < points.size(); i++) {                                                           // Aktív pont keresése
        if (sqrDistance(points[i], mousePos) < sqrSens) {
            return i;
        }
    }
    return -1;
}

int binom(int n, int k) {
    int result = 1;
    for (int i = 1; i <= k; i++) {                // Binomiális együttható
        result = result * (n - k + i) / i;
    }
    return result;
}

glm::vec2 bezierCalc(float t, std::vector<glm::vec2>& points) {
    glm::vec2 p(0.0f);
    int n = points.size() - 1;
    for (int i = 0; i <= n; i++) {                                      // Bézier-görbe kiszámítása
        int bc = binom(n, i);
        float b = bc * std::pow(t, i) * std::pow(1 - t, n - i);
        p += points[i] * b;
    }
    return p;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    if (drag && selectedPoint != -1) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);                           // Kurzor pozíciója
        glm::vec2 mousePos(
            (float)xPos / width * 2.0f - 1.0f,
            1.0f - (float)yPos / height * 2.0f
        );
        controlPoints[selectedPoint] = mousePos;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glm::vec2 mousePos(
            (float)xPos / width * 2.0f - 1.0f,
            1.0f - (float)yPos / height * 2.0f
        );

        if (action == GLFW_PRESS) {
            selectedPoint = actPoint(controlPoints, 0.1f, mousePos);
            if (selectedPoint == -1) {
                controlPoints.push_back(mousePos);
                selectedPoint = controlPoints.size() - 1;
            }
            drag = true;                                                            // Egérgomb eseményekre felelős kód
        }
        else if (action == GLFW_RELEASE) {
            drag = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glm::vec2 mousePos(
            (float)xPos / width * 2.0f - 1.0f,
            1.0f - (float)yPos / height * 2.0f
        );

        int removePoint = actPoint(controlPoints, 0.1f, mousePos);
        if (removePoint != -1) {
            controlPoints.erase(controlPoints.begin() + removePoint);
        }
    }
}


int main() {
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bézier-görbe", nullptr, nullptr);
    if (!window) {
        glfwTerminate();                                                                                                // Inicializálás
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return EXIT_FAILURE;
    }

    GLuint shaderProgram = createShaderProgram(readShaderFile("vertex_shader.glsl"),readShaderFile("fragment_shader.glsl"));

    controlPoints = {
        glm::vec2(-0.7f, -0.5f),
        glm::vec2(-0.3f, 0.3f),             // Előre inicializált kontrollpontok
        glm::vec2(0.3f, 0.3f),
        glm::vec2(0.7f, -0.5f)
    };


    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);             // VAO és VBO inicializálása
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        if (controlPoints.size() >= 2) {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.3f, 0.0f, 0.5f);
            glBindVertexArray(VAO);                                                                                                 // Kontrollpoligon
            glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec2), controlPoints.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());
        }


        if (controlPoints.size() >= 2) {
            std::vector<glm::vec2> bezierPoints;
            for (float point = 0; point <= 1; point += 0.01f) {
                bezierPoints.push_back(bezierCalc(point, controlPoints));
            }                                                                                                                       // Bézier görbe
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.8f, 0.4f, 0.5f);
            glBufferData(GL_ARRAY_BUFFER, bezierPoints.size() * sizeof(glm::vec2), bezierPoints.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINE_STRIP, 0, bezierPoints.size());
        }


        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec2), controlPoints.data(), GL_DYNAMIC_DRAW);
        glPointSize(8.0f);                                                                                                         // Kontrollpontok
        glEnable(GL_POINT_SMOOTH);  // Pont kinézetért felelős parancs
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_POINTS, 0, controlPoints.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return EXIT_SUCCESS;
}