// GLSL version 4.50, which aligns with OpenGL 4.5
// GLSL 4.50 版本，与 OpenGL 4.5 对应
#version 450

// Input 0: Fragment color
// 输入位置 0：片元颜色
layout(location = 0) in vec3 fragColor;

// Input 0: Output Color
// 输出位置 0：颜色输出
layout(location = 0) out vec4 outColor;

// Shader main function
// 着色器主程序
void main() {
    // Output Color is simply set to be the fragment color
    // 片元颜色被简单设置成输入的颜色
    outColor = vec4(fragColor, 1.0);
}