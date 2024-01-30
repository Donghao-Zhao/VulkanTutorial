// GLSL version 4.50, which aligns with OpenGL 4.5
// GLSL 4.50 版本，与 OpenGL 4.5 对应
#version 450

// Input 0: 2D coordinate
// 输入位置 0：2D 坐标
layout(location = 0) in vec2 inPosition;

// Input 1: R G B color
// 输入位置 1：R G B 颜色
layout(location = 1) in vec3 inColor;

// Input 0: Fragment color
// 输出位置 0：片元颜色
layout(location = 0) out vec3 fragColor;

// Shader main function
// 着色器主程序
void main() {
    // Built-in variable gl_Position represent the position of the current vertex in normalized device coordinates
    // 内置变量 gl_Position 表示当前顶点在标准化设备坐标中的位置
    gl_Position = vec4(inPosition, 0.0, 1.0);

    // Fragment color is simply set to be the input color
    // 片元颜色被简单设置成输入的颜色
    fragColor = inColor;
}