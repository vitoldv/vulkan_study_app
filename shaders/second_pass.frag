#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;        // Color ouput from subpass 1
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;        // Depth ouput from subpass 1

layout(location = 0) out vec4 color;

void main()
{
    // We can only grab the input data at the current fragment position
    // for example, if current fragment is [2, 4], we can only take pixel (fragment) at [2, 4] position from any input image
    // subpassLoad([input_image]).rgba - is a way to grab this color
    //color = subpassLoad(inputColor).rgba;   
    
    int xHalf = 1920 / 2;
    if(gl_FragCoord.x > xHalf)
    {
        float lowerBound = 0.99; 
        float upperBound = 1;
        float depth = subpassLoad(inputDepth).r;
        float depthColorScaled = 1.0f - ((depth - lowerBound) / (upperBound - lowerBound));
        color = vec4(subpassLoad(inputColor).rgb * depthColorScaled, 1.0f);
    }
    else
    {
        color = subpassLoad(inputColor).rgba;   
    }
}
