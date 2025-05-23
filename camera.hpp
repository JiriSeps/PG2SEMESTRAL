class Camera
{
public:

    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right; 
    glm::vec3 Up; // camera local UP vector

    GLfloat Yaw = -90.0f;
    GLfloat Pitch =  0.0f;;
    GLfloat Roll = 0.0f;
    
    // Camera options
    GLfloat MovementSpeed = 5.0f;
    GLfloat MouseSensitivity = 0.25f;

    Camera(glm::vec3 position):Position(position)
    {
        this->Up = glm::vec3(0.0f,1.0f,0.0f);
        // initialization of the camera reference system
        this->updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    glm::vec3 ProcessInput(GLFWwindow* window, GLfloat deltaTime)
    {
        glm::vec3 direction{ 0 };

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            direction += glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            direction -= glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            direction -= Right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            direction += Right;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            direction += Up;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            direction -= Up;

        if (glm::length(direction) > 0.0f)
            return glm::normalize(direction) * MovementSpeed * deltaTime;
        else
            return glm::vec3(0.0f); //  return no movement if idle
    }


    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        if (constraintPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        this->updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));

        this->Front = glm::normalize(front);
        this->Right = glm::normalize(glm::cross(this->Front, glm::vec3(0.0f,1.0f,0.0f)));
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
