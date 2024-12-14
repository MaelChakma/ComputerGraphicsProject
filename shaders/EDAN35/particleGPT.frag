#version 330 core
out vec4 FragColor;
  
in vec4 colorOut; // the input variable from the vertex shader (same name and same type)  

void main()
{
    FragColor = colorOut;
} 
