#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "pipeline.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;


static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
                                                                                    \n\
out vec4 Color;                                                                     \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    Color = vec4(clamp(Position, 0.0, 1.0), 1.0);                                   \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec4 Color;                                                                      \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                                              \n\
}";

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;

    Scale += 0.1f;

    Pipeline p;
    p.Rotate(0.0f, Scale, 0.0f); // Задаем вращение
    p.WorldPos(0.0f, 0.0f, 3.0f);//Задаем позицию
    Vector3f CameraPos(0.0f, 0.0f, -3.0f); // позиция камеры
    Vector3f CameraTarget(0.0f, 0.0f, 2.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);
    p.SetCamera(CameraPos, CameraTarget, CameraUp); // устанавливаем камеру
    p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f); // Перспектива

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glEnableVertexAttribArray(0); // Координаты вершин, используемые в буфере, рассматриваются как атрибут вершины с индексом 0
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Параметр GL_ARRAY_BUFFER означает, что буфер будет хранить массив вершин для отрисовки
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // Вызов говорит конвейеру как воспринимать данные внутри буфера
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); //связывает буфер вершин(Index Buffer Object) с указанным типом буфера

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0); //рисованиt примитивов на основе индексов вершин, хранящихся в буфере

    glDisableVertexAttribArray(0); //Отключает каждый атрибут вершины, как только отпадает необходимость в нем

    glutSwapBuffers(); // Меняет местами буферы текущего окна при двойной буферизации
}


static void InitializeGlutCallbacks() //Просит GLUT поменять фоновый буфер и буфер кадра местами
{
    glutDisplayFunc(RenderSceneCB); // Задает отображение обратного вызова для текущего окна
    glutIdleFunc(RenderSceneCB); // создание графических пользовательских интерфейсов и управления окнами и событиями
}

static void CreateVertexBuffer()
{
    // Массив из экземпляров структуры Vector3f заголовка math_3d.h
    Vector3f Vertices[4];
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.5773f);
    Vertices[1] = Vector3f(0.0f, -1.0f, -1.15475);
    Vertices[2] = Vector3f(1.0f, -1.0f, 0.5773f);
    Vertices[3] = Vector3f(0.0f, 1.0f, 0.0f);

    glGenBuffers(1, &VBO); //генерации буфера вершин (Vertex Buffer Object - VBO) и выделения памяти для него на видеокарте
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //функция для генерации объектов переменных типов
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW); //заполнение буфера вершин (Vertex Buffer Object - VBO) данными вершин.
}

static void CreateIndexBuffer() //
{
    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               0, 2, 1 };

    glGenBuffers(1, &IBO);  //генерации буфера вершин 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); //используется для связывания буфера вершин с типом буфера 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); //заполнение буфера вершин
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

                        // создание шейдера
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);

                        // Компилируем шейдер
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj); // проверяем программу
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram(); //создание программного объекта

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER); // создаем шейдер
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER); // создаем шейдер

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram); // создаем испольняемую программу и связываем шейдеры
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram); // проверка корректности шейдерной программы
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram); // устанавливает шейдерную программу для отрисовки

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);  // инициализация GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //(отрисовка будет происходить в фоновый буфер, в то время как другой буфер отображается)
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); // размер окна
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 13");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// (красный, зелёный, синий, альфа-канал)

    CreateVertexBuffer(); // Создает буфер вершин
    CreateIndexBuffer(); // Создает буфер индексов

    CompileShaders(); // шейдер

    glutMainLoop(); // Передаёт контроль GLUT'у

    return 0;
}