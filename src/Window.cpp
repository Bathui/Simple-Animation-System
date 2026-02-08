#include "Window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "skin.h"

int selectedJointIdx = 0;
int selectedDOF = 0; // 0 for X, 1 for Y, 2 for Z

// Window Properties
int Window::width;
int Window::height;
const char* Window::windowTitle = "CSE 169 Project 1";


Skin* Window::skin = nullptr;

// Objects to render
Cube* Window::cube;
Skeleton* Window::skeleton = nullptr;

// Camera Properties
Camera* Cam;

// Interaction Variables
bool LeftDown, RightDown;
int MouseX, MouseY;

// The shader program id
GLuint Window::shaderProgram;
GLuint Window::skinShaderProgram;
// Constructors and desctructors
bool Window::initializeProgram() {
    // Create a shader program with a vertex shader and a fragment shader.
    PrintInstructions();
    shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag");
    skinShaderProgram = LoadShaders("shaders/skin.vert", "shaders/skin.frag");
    // Check the shader program.
    if (!shaderProgram) {
        std::cerr << "Failed to initialize shader program" << std::endl;
        return false;
    }

    // Enable Depth Test
    glEnable(GL_DEPTH_TEST);
    // Enable Face Culling
    glEnable(GL_CULL_FACE);

    return true;
}

bool Window::initializeObjects() {
    // Create a cube
    cube = new Cube();
    // cube = new Cube(glm::vec3(-1, 0, -2), glm::vec3(1, 1, 1));
    return true;
}

void Window::cleanUp() {
    // Deallcoate the objects.
    delete cube;
    if (skeleton) delete skeleton;
    if (animation) delete animation;

    // Delete the shader program.
    glDeleteProgram(shaderProgram);
}

// Initializing static members
Animation* Window::animation = nullptr;
float Window::time = 0.0f;
bool Window::isPlaying = true;

std::string Window::lastLoadedFile = "";
void Window::LoadSkeleton(const char* filename) {
    std::string fn(filename);
    if(fn.find(".skel") != std::string::npos) {
        if (skeleton) delete skeleton;
        skeleton = new Skeleton();
        if (!skeleton->Load(filename)) {
            std::cerr << "Failed to load skeleton: " << filename << std::endl;
        }
    } 
    else if(fn.find(".skin") != std::string::npos) {
        if (skin) delete skin;
        skin = new Skin();
        if (!skin->Load(filename)) {
            std::cerr << "Failed to load skin: " << filename << std::endl;
        }
    }
    else if(fn.find(".anim") != std::string::npos) {
        if (animation) delete animation;
        animation = new Animation();
        if (!animation->Load(filename)) {
            std::cerr << "Failed to load animation: " << filename << std::endl;
        } else {
             time = animation->GetStartTime();
        }
    }
}

// for the Window
GLFWwindow* Window::createWindow(int width, int height) {
    // Initialize GLFW.
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return NULL;
    }

    // 4x antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create the GLFW window.
    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, NULL, NULL);

    // Check if the window could not be created.
    if (!window) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        return NULL;
    }

    // Make the context of the window.
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Set swap interval to 1.
    glfwSwapInterval(0);

    // set up the camera
    Cam = new Camera();
    Cam->SetAspect(float(width) / float(height));

    // initialize the interaction variables
    LeftDown = RightDown = false;
    MouseX = MouseY = 0;

    // Call the resize callback to make sure things get drawn immediately.
    Window::resizeCallback(window, width, height);

    return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    // Set the viewport size.
    glViewport(0, 0, width, height);

    Cam->SetAspect(float(width) / float(height));
}

// update and draw functions
void Window::idleCallback() {
    // Perform any updates as necessary.
    Cam->Update();

    // cube->update(); // Don't spin the default cube
    if (skeleton) skeleton->Update();
}

// Helper function to recursively draw Joint UI
// Local helper function in Window.cpp
void DrawJointUI(Joint* joint) {
    if (!joint) return;

    // Create a unique ID for ImGui to handle joints with the same name
    ImGui::PushID(joint);

    // Use a tree node for the hierarchy
    if (ImGui::TreeNode(joint->GetName().c_str())) {
        float* posePtr = joint->GetPosePtr();
        
        // Retrieve limits (which are parsed from the .skel file)
        glm::vec2 limX = joint->GetRotXLimit();
        glm::vec2 limY = joint->GetRotYLimit();
        glm::vec2 limZ = joint->GetRotZLimit();

        // SliderAngle: user sees Degrees, variable stores Radians
        if (ImGui::SliderAngle("Rotate X", &posePtr[0], glm::degrees(limX.x), glm::degrees(limX.y))) {
            printf("Joint: %s | DOF: Rotate X | Value: %.3f degrees\n", joint->GetName().c_str(), glm::degrees(posePtr[0]));
        }
        if (ImGui::SliderAngle("Rotate Y", &posePtr[1], glm::degrees(limY.x), glm::degrees(limY.y))) {
            printf("Joint: %s | DOF: Rotate Y | Value: %.3f degrees\n", joint->GetName().c_str(), glm::degrees(posePtr[1]));
        }
        if (ImGui::SliderAngle("Rotate Z", &posePtr[2], glm::degrees(limZ.x), glm::degrees(limZ.y))) {
            printf("Joint: %s | DOF: Rotate Z | Value: %.3f degrees\n", joint->GetName().c_str(), glm::degrees(posePtr[2]));
        }
        // Recursively draw children
        for (Joint* child : joint->GetChildren()) {
            DrawJointUI(child);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void Window::displayCallback(GLFWwindow* window) {
    // Clear the color and depth buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the object.
    // cube->draw(Camera::getViewProjMtx(), Window::shaderProgram);
    if (skin && skeleton) {
        // Compute matrices based on current skeleton pose
        skin->Update(skeleton);
        
        // Draw Mesh
        skeleton->Draw(Cam->GetViewProjectMtx(), Window::shaderProgram);
        skin->Draw(Cam->GetViewProjectMtx(), Window::skinShaderProgram);
    }
    else if (skeleton) skeleton->Draw(Cam->GetViewProjectMtx(), Window::shaderProgram);

    else if (skin) {
    // Draw skin in bind pose if no skeleton is loaded
        skin->Update(nullptr);
        skin->Draw(Cam->GetViewProjectMtx(), Window::skinShaderProgram);
    }
    
    // Animation Update
    if (animation && skeleton) {
        if (isPlaying) {
            // Simple delta time
            static double lastTime = glfwGetTime();
            double currentTime = glfwGetTime();
            float deltaTime = float(currentTime - lastTime);
            lastTime = currentTime;
            
            time += deltaTime;
            
        } else {
             // Update lastTime to avoid huge jump when resuming
             static double lastTime = glfwGetTime();
             lastTime = glfwGetTime();
        }
        
        animation->Evaluate(time, skeleton);
    }


    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::Begin("Animation Controls");
    if (animation) {
         if (ImGui::Button(isPlaying ? "Pause" : "Play")) {
             isPlaying = !isPlaying;
         }
         ImGui::SameLine();
         ImGui::SliderFloat("Time", &time, animation->GetStartTime(), animation->GetEndTime());
         
         ImGui::Text("Time: %.3f", time);
    } else {
         ImGui::Text("No animation loaded");
    }
    ImGui::End();

    ImGui::Begin("Skeleton Editor");
    if (Window::skeleton && Window::skeleton->GetRoot()) {
        DrawJointUI(Window::skeleton->GetRoot());
    } else {
        ImGui::Text("No skeleton loaded.");
    }
    ImGui::End();
    // ImGui::ShowDemoWindow();
    
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


    // Gets events, including input such as keyboard and mouse or window resizing.
    glfwPollEvents();
    // Swap buffers.
    glfwSwapBuffers(window);
}

// helper to reset the camera
void Window::resetCamera() {
    Cam->Reset();
    Cam->SetAspect(float(Window::width) / float(Window::height));
}

// callbacks - for Interaction
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*
     * TODO: Modify below to add your key callbacks.
     */

    // Check for a key press.
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        
        Joint* currentJoint = skeleton->jointList[selectedJointIdx];
        float* posePtr = currentJoint->GetPosePtr();

        switch (key) {
            case GLFW_KEY_ESCAPE:
                // Close the window. This causes the program to also terminate.
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;

            case GLFW_KEY_R:
                resetCamera();
                break;
            
            case GLFW_KEY_0:
                // Refresh the skeleton by reloading the last file
                if (!lastLoadedFile.empty()) {
                    LoadSkeleton(lastLoadedFile.c_str());
                }
                break;
            case GLFW_KEY_UP:
                selectedJointIdx = (selectedJointIdx - 1 + skeleton->jointList.size()) % skeleton->jointList.size();
                break;
            case GLFW_KEY_DOWN:
                selectedJointIdx = (selectedJointIdx + 1) % skeleton->jointList.size();
                break;
            case GLFW_KEY_LEFT:
                selectedDOF = (selectedDOF - 1 + 3) % 3;
                break;
            case GLFW_KEY_RIGHT:
                selectedDOF = (selectedDOF + 1) % 3;
                break;
            case GLFW_KEY_EQUAL: // The '+' key (without shift)
                posePtr[selectedDOF] += 0.05f;
                break;
            case GLFW_KEY_MINUS:
                posePtr[selectedDOF] -= 0.05f;
                break;

            default:
                break;
        }
        const char* dofNames[] = {"Rotate X", "Rotate Y", "Rotate Z"};
        printf("Selected Joint: %s | DOF: %s | Value: %.3f degrees\n", 
               currentJoint->GetName().c_str(), dofNames[selectedDOF], glm::degrees(posePtr[selectedDOF]));
    }
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (ImGui::GetIO().WantCaptureMouse) {
        LeftDown = false;
        RightDown = false;
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        LeftDown = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        RightDown = (action == GLFW_PRESS);
    }
}

void Window::cursor_callback(GLFWwindow* window, double currX, double currY) {
    
    int maxDelta = 100;
    int dx = glm::clamp((int)currX - MouseX, -maxDelta, maxDelta);
    int dy = glm::clamp(-((int)currY - MouseY), -maxDelta, maxDelta);

    MouseX = (int)currX;
    MouseY = (int)currY;

    // Move camera
    // NOTE: this should really be part of Camera::Update()
    if (LeftDown) {
        const float rate = 1.0f;
        Cam->SetAzimuth(Cam->GetAzimuth() + dx * rate);
        Cam->SetIncline(glm::clamp(Cam->GetIncline() - dy * rate, -90.0f, 90.0f));
    }
    if (RightDown) {
        const float rate = 0.005f;
        float dist = glm::clamp(Cam->GetDistance() * (1.0f - dx * rate), 0.01f, 1000.0f);
        Cam->SetDistance(dist);
    }
}

void Window::PrintInstructions() {
    printf("------------------------------------------------------------------------\n");
    printf("|CSE 169 Project User Instructions                                      |\n");
    printf("------------------------------------------------------------------------\n");
    printf("SKELETON SELECTION:\n");
    printf("  [UP / DOWN ARROW]    : Cycle through Joints\n");
    printf("  [LEFT / RIGHT ARROW] : Cycle through DOFs (Rotate X, Y, Z)\n");
    printf("\nDOF MANIPULATION:\n");
    printf("  [+ / =]              : Increase current DOF value\n");
    printf("  [-]                  : Decrease current DOF value\n");
    printf("\nCAMERA & SYSTEM:\n");
    printf("  [R]                  : Reset Camera position\n");
    printf("  [0]                  : Reload the current skeleton/skin file\n");
    printf("  [ESC]                : Exit application\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Interactive GUI is also available in the 'Skeleton Editor' window.\n");
    printf("--------------------------------------------------------------------------------\n\n");
}