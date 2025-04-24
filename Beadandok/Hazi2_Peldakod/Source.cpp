enum eVertexArrayObject {
    VAOCurveData,
    VAOCount
};
enum eVertexBufferObject {
    VBOBezierData,
    BOCount
};
enum eProgram {
    CurveTesselationProgram,
    QuadScreenProgram,
    ProgramCount
};
enum eTexture {
    NoTexture,
    TextureCount
};

#include "common.cpp"

#define BEZIER_BERNSTEIN    3
#define MAX_CONTROL_POINTS  32

GLchar  windowTitle[] = "Bézier-görbe";
vector<vec3> controlPoints = {
    vec3(-0.7f, -0.5f, 0.0f),
    vec3(-0.3f, 0.5f, 0.0f),
    vec3(0.3f, 0.5f, 0.0f),                         // Előre inicializált kontrollpontok
    vec3(0.7f, -0.5f, 0.0f)
};

GLuint locationTessMatProjection, locationTessMatModelView, locationCurveType, locationControlPointsNumber;
GLuint locationCurveColor, locationLineColor, locationPointColor;
GLuint curveType = BEZIER_BERNSTEIN;
GLint selPoint = -1;
bool drag = false;

vec3 curveColor = vec3(0.8f, 0.4f, 0.5f);
vec3 lineColor = vec3(0.3f, 0.0f, 0.5f);            // Színek beállítása
vec3 pointColor = vec3(1.0f, 1.0f, 0.0f);

void updateControlPoints() {
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOBezierData]);
    glBufferData(GL_ARRAY_BUFFER, MAX_CONTROL_POINTS * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);
    if (!controlPoints.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, controlPoints.size() * sizeof(vec3), controlPoints.data());     // Kontrolpontok tömbjének frissítése
    }
    glBindVertexArray(VAO[VAOCurveData]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

void initTesselationShader() {
    ShaderInfo shader_info[] = {
        { GL_FRAGMENT_SHADER,          "./CurveFragShader.glsl" },
        { GL_TESS_CONTROL_SHADER,      "./CurveTessContShader.glsl" },
        { GL_TESS_EVALUATION_SHADER,   "./CurveTessEvalShader.glsl" },
        { GL_VERTEX_SHADER,            "./CurveVertShader.glsl" },
        { GL_NONE,                     nullptr }
    };
    program[CurveTesselationProgram] = LoadShaders(shader_info);                                        // Shader fileok betöltése és inicializálása
    glBindVertexArray(VAO[VAOCurveData]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOBezierData]);
    glBufferData(GL_ARRAY_BUFFER, MAX_CONTROL_POINTS * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);
    updateControlPoints();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    locationCurveType = glGetUniformLocation(program[CurveTesselationProgram], "curveType");
    locationControlPointsNumber = glGetUniformLocation(program[CurveTesselationProgram], "controlPointsNumber");
    locationTessMatProjection = glGetUniformLocation(program[CurveTesselationProgram], "matProjection");
    locationTessMatModelView = glGetUniformLocation(program[CurveTesselationProgram], "matModelView");
    locationCurveColor = glGetUniformLocation(program[CurveTesselationProgram], "curveColor");

    glUseProgram(program[CurveTesselationProgram]);
    glUniform1i(locationCurveType, curveType);
    glUniform1i(locationControlPointsNumber, controlPoints.size());
    glUniform3fv(locationCurveColor, 1, value_ptr(curveColor));
}

void initShaderProgram() {
    ShaderInfo shader_info[] = {
        { GL_FRAGMENT_SHADER,          "./QuadScreenFragShader.glsl" },
        { GL_VERTEX_SHADER,            "./QuadScreenVertShader.glsl" },
        { GL_NONE,                     nullptr }
    };
    program[QuadScreenProgram] = LoadShaders(shader_info);
    locationMatProjection = glGetUniformLocation(program[QuadScreenProgram], "matProjection");          // Shader program inicializálása
    locationMatModelView = glGetUniformLocation(program[QuadScreenProgram], "matModelView");
    locationLineColor = glGetUniformLocation(program[QuadScreenProgram], "lineColor");
    locationPointColor = glGetUniformLocation(program[QuadScreenProgram], "lineColor");
}

void display(GLFWwindow* window, double currentTime) {
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);                                                         // Kontrolpont kör alakjáért felelős parancsok
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glClear(GL_COLOR_BUFFER_BIT);

    if (controlPoints.size() > 1) {
        glUseProgram(program[CurveTesselationProgram]);
        glUniform1i(locationControlPointsNumber, controlPoints.size());         // Bézier görbe kirajzolása
        glPatchParameteri(GL_PATCH_VERTICES, controlPoints.size());             
        glDrawArrays(GL_PATCHES, 0, controlPoints.size());
    }

    if (!controlPoints.empty()) {
        glUseProgram(program[QuadScreenProgram]);

        if (controlPoints.size() > 1) {
            glUniform3fv(locationLineColor, 1, value_ptr(lineColor));                   // Kontrollpoligon kirajzolása
            glLineWidth(2.0f);
            glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());
        }

        glUniform3fv(locationPointColor, 1, value_ptr(pointColor));
        glPointSize(8.0);                                                               // Kontrollpontokkirajzolása
        glDrawArrays(GL_POINTS, 0, controlPoints.size());
    }

    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = glm::max(width, 1);
    windowHeight = glm::max(height, 1);

    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glViewport(0, 0, windowWidth, windowHeight);

    if (windowWidth < windowHeight)
        matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
    else
        matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);

    matModel = mat4(1.0);
    matView = lookAt(
        vec3(0.0f, 0.0f, 9.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f));
    matModelView = matView * matModel;

    glUseProgram(program[QuadScreenProgram]);
    glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, glm::value_ptr(matModelView));
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, glm::value_ptr(matProjection));

    glUseProgram(program[CurveTesselationProgram]);
    glUniformMatrix4fv(locationTessMatModelView, 1, GL_FALSE, glm::value_ptr(matModelView));
    glUniformMatrix4fv(locationTessMatProjection, 1, GL_FALSE, glm::value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS)
        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE)
        keyboard[key] = GL_FALSE;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    if (drag && selPoint != -1 && selPoint < controlPoints.size()) {
        float worldX = ((2.0f * xPos) / windowWidth - 1.0f) * (windowWidth > windowHeight ? worldSize * (float)windowWidth / windowHeight : worldSize);
        float worldY = (1.0f - (2.0f * yPos) / windowHeight) * (windowWidth > windowHeight ? worldSize : worldSize * (float)windowHeight / windowWidth);            // Görbe valós koordinátái

        controlPoints[selPoint] = vec3(worldX, worldY, 0.0f);
        updateControlPoints();
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    float worldX = ((2.0f * x) / windowWidth - 1.0f) * (windowWidth > windowHeight ? worldSize * (float)windowWidth / windowHeight : worldSize);
    float worldY = (1.0f - (2.0f * y) / windowHeight) * (windowWidth > windowHeight ? worldSize : worldSize * (float)windowHeight / windowWidth);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        float minDist = 0.1f;
        selPoint = -1;

        for (int i = 0; i < controlPoints.size(); i++) {
            float dx = controlPoints[i].x - worldX;
            float dy = controlPoints[i].y - worldY;
            float dist = sqrt(dx * dx + dy * dy);                                           // Kurzor távolsága egy kontrollponttól

            if (dist < minDist) {
                minDist = dist;
                selPoint = i;
            }
        }

        drag = true;

        if (selPoint == -1 && controlPoints.size() < MAX_CONTROL_POINTS) {                     // Egér mozgatási események kezelése
            controlPoints.push_back(vec3(worldX, worldY, 0.0f));
            updateControlPoints();
            glUseProgram(program[CurveTesselationProgram]);
            glUniform1i(locationControlPointsNumber, controlPoints.size());
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        drag = false;
        selPoint = -1;                                                                     // Kontrollpont mozgatása és görbe újrarajzolása
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        float minDist = 0.1f;
        selPoint = -1;

        for (int i = 0; i < controlPoints.size(); i++) {
            float dx = controlPoints[i].x - worldX;
            float dy = controlPoints[i].y - worldY;
            float dist = sqrt(dx * dx + dy * dy);                                       // Kontrollpontok közötti távolság kiszámítása

            if (dist < minDist) {
                minDist = dist;
                selPoint = i;
            }
        }

        if (selPoint != -1) {
            controlPoints.erase(controlPoints.begin() + selPoint);
            updateControlPoints();                                                            // Kontrollpont törlése
            glUseProgram(program[CurveTesselationProgram]);
            glUniform1i(locationControlPointsNumber, controlPoints.size());
        }
    }
}

int main(void) {
    init(4, 0, GLFW_OPENGL_COMPAT_PROFILE);
    initTesselationShader();
    initShaderProgram();

    setlocale(LC_ALL, "");

    framebufferSizeCallback(window, windowWidth, windowHeight);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}